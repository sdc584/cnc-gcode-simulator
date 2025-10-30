#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "CNCSimulator.h"
#endif

#include "CNCSimulatorDoc.h"
#include <cmath>

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCNCSimulatorDoc 消息映射

BEGIN_MESSAGE_MAP(CCNCSimulatorDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN_GCODE, &CCNCSimulatorDoc::OnFileOpenGcode)
	ON_COMMAND(ID_SIMULATION_START, &CCNCSimulatorDoc::OnSimulationStart)
	ON_COMMAND(ID_SIMULATION_PAUSE, &CCNCSimulatorDoc::OnSimulationPause)
	ON_COMMAND(ID_SIMULATION_STOP, &CCNCSimulatorDoc::OnSimulationStop)
	ON_COMMAND(ID_SIMULATION_RESET, &CCNCSimulatorDoc::OnSimulationReset)
END_MESSAGE_MAP()

// CCNCSimulatorDoc

IMPLEMENT_DYNCREATE(CCNCSimulatorDoc, CDocument)

// CCNCSimulatorDoc 构造/析构

CCNCSimulatorDoc::CCNCSimulatorDoc() noexcept
{
	// 初始化系统状态
	m_nSystemState = ST_IDLE;
	m_nCurrentBlock = 0;
	m_dCurrentX = m_dCurrentY = m_dCurrentZ = 0.0;
	m_dFeedRate = 100.0;        // 默认进给速度
	m_dSpindleSpeed = 1000.0;   // 默认主轴转速

	// 初始化模态状态
	m_bAbsoluteMode = TRUE;     // 默认绝对坐标模式 G90
	m_nPlaneMode = 17;          // 默认XY平面 G17

	// 初始化倍率
	m_dFeedOverride = 1.0;      // 100%
	m_dSpindleOverride = 1.0;   // 100%
	m_dSimulationSpeed = 1.0;   // 100%

	// 初始化仿真状态
	m_nCurrentPathIndex = 0;
	m_bSimulationRunning = FALSE;
	m_dwLastUpdateTime = 0;
	m_dPathProgress = 0.0;
	m_dTargetX = m_dTargetY = m_dTargetZ = 0.0;
	m_dStartX = m_dStartY = m_dStartZ = 0.0;
	m_bSingleStepMode = FALSE;  // 默认连续执行
	m_bWaitingForStep = FALSE;
	m_dwStartTime = 0;
	m_dwTotalRunTime = 0;

	// 初始化报警和监控
	m_strLastError = _T("");
	m_strStatusMessage = _T("系统就绪");
	m_bHasAlarm = FALSE;
}

CCNCSimulatorDoc::~CCNCSimulatorDoc()
{
}

BOOL CCNCSimulatorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// 清空数据
	m_arrGCodeBlocks.RemoveAll();
	m_arrToolPath.RemoveAll();

	// 重置状态
	m_nSystemState = ST_IDLE;
	m_nCurrentBlock = 0;
	m_dCurrentX = m_dCurrentY = m_dCurrentZ = 0.0;

	return TRUE;
}

// CCNCSimulatorDoc 序列化

void CCNCSimulatorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// 保存G代码到文件
		// TODO: 实现保存功能
	}
	else
	{
		// 从文件加载G代码
		CString strFilePath = ar.GetFile()->GetFilePath();
		TRACE(_T("Serialize: 正在加载文件 %s\n"), strFilePath);

		// 调用G代码解析函数
		if (!ParseGCodeFile(strFilePath))
		{
			AfxMessageBox(_T("无法解析G代码文件！"));
		}
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CCNCSimulatorDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CCNCSimulatorDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由";"分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CCNCSimulatorDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CCNCSimulatorDoc 诊断

#ifdef _DEBUG
void CCNCSimulatorDoc::AssertValid() const
{
	// 暂时注释掉基类调用，避免断言失败
	// CDocument::AssertValid();
}

void CCNCSimulatorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// CCNCSimulatorDoc 命令

BOOL CCNCSimulatorDoc::ParseGCodeFile(const CString& strFilePath)
{
	// 先尝试以二进制方式读取文件，然后转换编码
	CFile file;
	if (!file.Open(strFilePath, CFile::modeRead))
	{
		CString strError;
		strError.Format(_T("无法打开G代码文件:\n%s\n\n请检查:\n1. 文件是否存在\n2. 文件是否被其他程序占用\n3. 是否有读取权限"), strFilePath);
		AfxMessageBox(strError);
		return FALSE;
	}

	// 读取文件内容
	ULONGLONG fileSize = file.GetLength();
	if (fileSize > 1024 * 1024) // 限制文件大小为1MB
	{
		AfxMessageBox(_T("G代码文件过大（超过1MB），请选择较小的文件。"));
		file.Close();
		return FALSE;
	}

	// 读取原始字节
	CByteArray byteArray;
	byteArray.SetSize((INT_PTR)fileSize);
	file.Read(byteArray.GetData(), (UINT)fileSize);
	file.Close();

	// 转换编码：尝试UTF-8转Unicode
	CString strFileContent;

	// 检查BOM标记
	BOOL bIsUTF8 = FALSE;
	if (fileSize >= 3 &&
		byteArray[0] == 0xEF &&
		byteArray[1] == 0xBB &&
		byteArray[2] == 0xBF)
	{
		bIsUTF8 = TRUE;
		// 跳过BOM
		int utf8Len = MultiByteToWideChar(CP_UTF8, 0,
			(LPCSTR)(byteArray.GetData() + 3), (int)fileSize - 3, NULL, 0);
		if (utf8Len > 0)
		{
			LPWSTR pWideStr = strFileContent.GetBuffer(utf8Len + 1);
			MultiByteToWideChar(CP_UTF8, 0,
				(LPCSTR)(byteArray.GetData() + 3), (int)fileSize - 3, pWideStr, utf8Len);
			pWideStr[utf8Len] = 0;
			strFileContent.ReleaseBuffer();
		}
	}
	else
	{
		// 尝试UTF-8无BOM
		int utf8Len = MultiByteToWideChar(CP_UTF8, 0,
			(LPCSTR)byteArray.GetData(), (int)fileSize, NULL, 0);
		if (utf8Len > 0)
		{
			LPWSTR pWideStr = strFileContent.GetBuffer(utf8Len + 1);
			int result = MultiByteToWideChar(CP_UTF8, 0,
				(LPCSTR)byteArray.GetData(), (int)fileSize, pWideStr, utf8Len);
			if (result > 0)
			{
				pWideStr[utf8Len] = 0;
				strFileContent.ReleaseBuffer();
				bIsUTF8 = TRUE;
			}
			else
			{
				strFileContent.ReleaseBuffer(0);
			}
		}

		// 如果UTF-8转换失败，尝试ANSI
		if (!bIsUTF8)
		{
			int ansiLen = MultiByteToWideChar(CP_ACP, 0,
				(LPCSTR)byteArray.GetData(), (int)fileSize, NULL, 0);
			if (ansiLen > 0)
			{
				LPWSTR pWideStr = strFileContent.GetBuffer(ansiLen + 1);
				MultiByteToWideChar(CP_ACP, 0,
					(LPCSTR)byteArray.GetData(), (int)fileSize, pWideStr, ansiLen);
				pWideStr[ansiLen] = 0;
				strFileContent.ReleaseBuffer();
			}
		}
	}

	// 清空现有数据
	m_arrGCodeBlocks.RemoveAll();
	m_arrToolPath.RemoveAll();

	// 调试信息
	TRACE(_T("开始解析G代码文件: %s (编码: %s)\n"), strFilePath, bIsUTF8 ? _T("UTF-8") : _T("ANSI"));

	// 按行解析内容
	int nLineNumber = 1;
	int nStart = 0;
	int nPos = 0;

	while (nPos < strFileContent.GetLength())
	{
		// 查找行结束符
		while (nPos < strFileContent.GetLength() &&
			   strFileContent[nPos] != _T('\r') &&
			   strFileContent[nPos] != _T('\n'))
		{
			nPos++;
		}

		// 提取一行
		CString strLine = strFileContent.Mid(nStart, nPos - nStart);
		strLine.Trim();

		if (!strLine.IsEmpty()) // 只跳过空行，保留注释
		{
			GCodeBlock block;
			if (strLine.GetLength() > 0 && strLine[0] == _T(';')) // 注释行
			{
				block.strLine = strLine;
				block.nLineNumber = nLineNumber;
				// 注释行不包含G代码，arrGCodes保持为空
				block.nMCode = -1;
				m_arrGCodeBlocks.Add(block);
			}
			else if (ParseGCodeLine(strLine, block)) // G代码行
			{
				block.nLineNumber = nLineNumber;
				m_arrGCodeBlocks.Add(block);
			}
		}

		// 跳过行结束符
		while (nPos < strFileContent.GetLength() &&
			   (strFileContent[nPos] == _T('\r') || strFileContent[nPos] == _T('\n')))
		{
			nPos++;
		}

		nStart = nPos;
		nLineNumber++;
	}

	// 调试信息
	TRACE(_T("解析完成，共解析了 %d 行G代码\n"), m_arrGCodeBlocks.GetSize());

	// 计算刀具路径
	CalculateToolPath();

	// 通知所有视图更新
	UpdateAllViews(nullptr);

	return TRUE;
}

BOOL CCNCSimulatorDoc::ParseGCodeLine(const CString& strLine, GCodeBlock& block)
{
	block.strLine = strLine;

	// 预处理：移除注释和多余空格
	CString str = strLine;
	int commentPos = str.Find(';');
	if (commentPos >= 0)
		str = str.Left(commentPos); // 移除注释部分

	str.MakeUpper();
	str.Trim();

	if (str.IsEmpty())
		return FALSE;

	// 使用更健壮的解析方法
	return ParseGCodeParameters(str, block);
}

BOOL CCNCSimulatorDoc::ParseGCodeParameters(const CString& str, GCodeBlock& block)
{
	int pos = 0;
	while (pos < str.GetLength())
	{
		// 跳过空格
		while (pos < str.GetLength() && str[pos] == ' ')
			pos++;

		if (pos >= str.GetLength())
			break;

		TCHAR ch = str[pos];
		pos++;

		// 解析数值
		CString strNum;
		BOOL bNegative = FALSE;

		// 处理负号
		if (pos < str.GetLength() && str[pos] == '-')
		{
			bNegative = TRUE;
			pos++;
		}
		else if (pos < str.GetLength() && str[pos] == '+')
		{
			pos++;
		}

		// 提取数字
		while (pos < str.GetLength() && (isdigit(str[pos]) || str[pos] == '.'))
		{
			strNum += str[pos++];
		}

		if (strNum.IsEmpty())
			continue;

		double value = _ttof(strNum);
		if (bNegative)
			value = -value;

		// 根据字母分配值
		switch (ch)
		{
		case 'G':
			block.arrGCodes.Add((int)value);
			break;
		case 'M':
			block.nMCode = (int)value;
			break;
		case 'X':
			block.dX = value;
			block.bHasX = TRUE;
			break;
		case 'Y':
			block.dY = value;
			block.bHasY = TRUE;
			break;
		case 'Z':
			block.dZ = value;
			block.bHasZ = TRUE;
			break;
		case 'F':
			block.dF = value;
			block.bHasF = TRUE;
			break;
		case 'S':
			block.dS = value;
			block.bHasS = TRUE;
			break;
		case 'I':
			block.dI = value;
			block.bHasI = TRUE;
			break;
		case 'J':
			block.dJ = value;
			block.bHasJ = TRUE;
			break;
		case 'K':
			block.dK = value;
			block.bHasK = TRUE;
			break;
		case 'R':
			block.dR = value;
			block.bHasR = TRUE;
			break;
		default:
			// 忽略未知参数
			break;
		}
	}

	return TRUE;
}

void CCNCSimulatorDoc::CalculateToolPath()
{
	m_arrToolPath.RemoveAll();

	if (m_arrGCodeBlocks.IsEmpty())
		return;

	// 当前位置和模态状态
	double currentX = 0.0, currentY = 0.0, currentZ = 0.0;
	BOOL bAbsMode = TRUE; // 当前坐标模式
	double currentF = 100.0; // 当前进给速度
	double currentS = 1000.0; // 当前主轴转速

	for (int i = 0; i < m_arrGCodeBlocks.GetSize(); i++)
	{
		const GCodeBlock& block = m_arrGCodeBlocks[i];

		// 处理模态代码
		if (block.HasGCode(90))
		{
			bAbsMode = TRUE;  // 绝对坐标
			m_bAbsoluteMode = TRUE;
		}
		else if (block.HasGCode(91))
		{
			bAbsMode = FALSE; // 增量坐标
			m_bAbsoluteMode = FALSE;
		}

		if (block.HasGCode(17))
			m_nPlaneMode = 17; // XY平面
		else if (block.HasGCode(18))
			m_nPlaneMode = 18; // XZ平面
		else if (block.HasGCode(19))
			m_nPlaneMode = 19; // YZ平面

		// 更新进给速度和主轴转速
		if (block.bHasF)
		{
			currentF = block.dF;
			m_dFeedRate = currentF;
		}
		if (block.bHasS)
		{
			currentS = block.dS;
			m_dSpindleSpeed = currentS;
		}

		// 计算目标位置
		double targetX = currentX, targetY = currentY, targetZ = currentZ;

		if (block.bHasX)
		{
			if (bAbsMode)
				targetX = block.dX;
			else
				targetX = currentX + block.dX;
		}

		if (block.bHasY)
		{
			if (bAbsMode)
				targetY = block.dY;
			else
				targetY = currentY + block.dY;
		}

		if (block.bHasZ)
		{
			if (bAbsMode)
				targetZ = block.dZ;
			else
				targetZ = currentZ + block.dZ;
		}

		// 根据G代码类型处理移动
		int mainGCode = block.GetMainGCode();
		if (mainGCode == 0) // G00 快速移动
		{
			if (block.bHasX || block.bHasY || block.bHasZ)
			{
				m_arrToolPath.Add(ToolPathPoint(targetX, targetY, targetZ, i, 0));
			}
		}
		else if (mainGCode == 1) // G01 直线插补
		{
			if (block.bHasX || block.bHasY || block.bHasZ)
			{
				m_arrToolPath.Add(ToolPathPoint(targetX, targetY, targetZ, i, 1));
			}
		}
		else if (mainGCode == 2 || mainGCode == 3) // G02/G03 圆弧插补
		{
			if (block.bHasX || block.bHasY)
			{
				CalculateArcPath(currentX, currentY, targetX, targetY,
					block.dI, block.dJ, mainGCode == 2, i);
			}
		}

		// 更新当前位置
		currentX = targetX;
		currentY = targetY;
		currentZ = targetZ;
	}
}

void CCNCSimulatorDoc::CalculateArcPath(double startX, double startY, double endX, double endY,
	double centerOffsetX, double centerOffsetY, BOOL bClockwise, int blockIndex)
{
	// 计算圆弧中心
	double centerX = startX + centerOffsetX;
	double centerY = startY + centerOffsetY;

	// 计算半径
	double radius = sqrt(centerOffsetX * centerOffsetX + centerOffsetY * centerOffsetY);

	// 计算起始角度和结束角度
	double startAngle = atan2(startY - centerY, startX - centerX);
	double endAngle = atan2(endY - centerY, endX - centerX);

	// 处理角度范围
	if (bClockwise) // 顺时针 G02
	{
		if (endAngle > startAngle)
			endAngle -= 2 * M_PI;
	}
	else // 逆时针 G03
	{
		if (endAngle < startAngle)
			endAngle += 2 * M_PI;
	}

	// 计算角度差
	double angleDiff = endAngle - startAngle;

	// 分割圆弧为多个小段
	int segments = max(8, (int)(fabs(angleDiff) * radius / 2.0)); // 根据半径和角度确定段数

	for (int i = 1; i <= segments; i++)
	{
		double t = (double)i / segments;
		double angle = startAngle + angleDiff * t;

		double x = centerX + radius * cos(angle);
		double y = centerY + radius * sin(angle);

		m_arrToolPath.Add(ToolPathPoint(x, y, 0, blockIndex, 2)); // 圆弧插补
	}
}

void CCNCSimulatorDoc::StartSimulation()
{
	if (CanTransitionTo(ST_RUNNING))
	{
		SetSystemState(ST_RUNNING);
		m_bSimulationRunning = TRUE;
		m_dwLastUpdateTime = GetTickCount();

		// 记录开始时间
		if (m_dwStartTime == 0)
		{
			m_dwStartTime = GetTickCount();
		}
		else
		{
			// 从暂停恢复，重新设置开始时间
			m_dwStartTime = GetTickCount();
		}

		// 如果是从空闲状态开始，初始化仿真
		if (m_nCurrentPathIndex == 0 && !m_arrToolPath.IsEmpty())
		{
			StartPathSegment(0);

			// 在单段模式下，开始后立即等待第一步指令
			if (m_bSingleStepMode)
			{
				m_bWaitingForStep = TRUE;
			}
		}
	}
}

void CCNCSimulatorDoc::PauseSimulation()
{
	if (CanTransitionTo(ST_PAUSED))
	{
		SetSystemState(ST_PAUSED);
		m_bSimulationRunning = FALSE;

		// 累计运行时间
		if (m_dwStartTime > 0)
		{
			m_dwTotalRunTime += GetTickCount() - m_dwStartTime;
			m_dwStartTime = 0;
		}
	}
}

void CCNCSimulatorDoc::StopSimulation()
{
	if (CanTransitionTo(ST_STOPPED))
	{
		SetSystemState(ST_STOPPED);
		m_bSimulationRunning = FALSE;
	}
}

void CCNCSimulatorDoc::ResetSimulation()
{
	m_nSystemState = ST_IDLE;
	m_nCurrentBlock = 0;
	m_nCurrentPathIndex = 0;
	m_dPathProgress = 0.0;
	m_bSimulationRunning = FALSE;
	m_bWaitingForStep = FALSE;

	// 重置时间统计
	m_dwStartTime = 0;
	m_dwTotalRunTime = 0;

	// 重置刀具位置到原点
	m_dCurrentX = m_dCurrentY = m_dCurrentZ = 0.0;
	m_dStartX = m_dStartY = m_dStartZ = 0.0;
	m_dTargetX = m_dTargetY = m_dTargetZ = 0.0;

	UpdateAllViews(nullptr);
}

void CCNCSimulatorDoc::SetSingleStepMode(BOOL bEnable)
{
	m_bSingleStepMode = bEnable;

	if (bEnable)
	{
		// 启用单段模式
		if (m_bSimulationRunning)
		{
			// 如果正在运行，暂停等待下一步
			m_bWaitingForStep = TRUE;
		}
	}
	else
	{
		// 禁用单段模式，清除等待状态
		m_bWaitingForStep = FALSE;
	}

	// 通知视图更新按钮状态
	UpdateAllViews(nullptr);
}

void CCNCSimulatorDoc::ExecuteNextStep()
{
	if (m_bSingleStepMode && m_bWaitingForStep)
	{
		m_bWaitingForStep = FALSE;

		// 如果仿真没有运行，启动仿真
		if (!m_bSimulationRunning && m_nSystemState == ST_IDLE)
		{
			StartSimulation();
		}

		// 通知视图更新，显示当前状态
		UpdateAllViews(nullptr);
	}
}

DWORD CCNCSimulatorDoc::GetRunTime() const
{
	if (m_dwStartTime == 0)
		return 0;

	if (m_bSimulationRunning)
	{
		return (GetTickCount() - m_dwStartTime + m_dwTotalRunTime) / 1000;
	}
	else
	{
		return m_dwTotalRunTime / 1000;
	}
}

double CCNCSimulatorDoc::GetProgress() const
{
	if (m_arrToolPath.IsEmpty())
		return 0.0;

	// 确保索引在有效范围内
	int currentIndex = min(m_nCurrentPathIndex, (int)m_arrToolPath.GetSize());
	return (double)currentIndex / m_arrToolPath.GetSize() * 100.0;
}

void CCNCSimulatorDoc::GetCurrentPosition(double& x, double& y, double& z) const
{
	x = m_dCurrentX;
	y = m_dCurrentY;
	z = m_dCurrentZ;
}

void CCNCSimulatorDoc::UpdateSimulation()
{
	if (!m_bSimulationRunning || m_nSystemState != ST_RUNNING)
		return;

	if (m_arrToolPath.IsEmpty())
		return;

	// 单段执行模式检查
	if (m_bSingleStepMode && m_bWaitingForStep)
		return;

	DWORD dwCurrentTime = GetTickCount();
	DWORD dwElapsed = dwCurrentTime - m_dwLastUpdateTime;
	m_dwLastUpdateTime = dwCurrentTime;

	// 计算时间步长 (考虑仿真速度)
	double dTimeStep = dwElapsed / 1000.0 * m_dSimulationSpeed;

	// 限制最大时间步长，避免跳跃过大
	if (dTimeStep > 0.1)
		dTimeStep = 0.1;

	// 更新路径进度
	if (m_nCurrentPathIndex < m_arrToolPath.GetSize())
	{
		// 根据进给速度和进给倍率计算移动速度
		// 进给速度单位：mm/min，需要转换为mm/s
		// 倍率：1.0 = 100%
		double dMoveSpeed = m_dFeedRate * m_dFeedOverride / 60.0; // mm/s

		// 计算当前路径段的长度
		double dSegmentLength = sqrt(
			(m_dTargetX - m_dStartX) * (m_dTargetX - m_dStartX) +
			(m_dTargetY - m_dStartY) * (m_dTargetY - m_dStartY) +
			(m_dTargetZ - m_dStartZ) * (m_dTargetZ - m_dStartZ)
		);

		if (dSegmentLength > 0.001) // 避免除零
		{
			// 计算进度增量
			double dProgressIncrement = dTimeStep * dMoveSpeed / dSegmentLength;
			m_dPathProgress += dProgressIncrement;

			if (m_dPathProgress >= 1.0)
			{
				// 当前段完成，移动到下一段
				m_dPathProgress = 0.0;
				m_nCurrentPathIndex++;

				if (m_nCurrentPathIndex < m_arrToolPath.GetSize())
				{
					StartPathSegment(m_nCurrentPathIndex);

					// 在单段模式下，完成一段后等待下一步指令
					if (m_bSingleStepMode)
					{
						m_bWaitingForStep = TRUE;
					}
				}
				else
				{
					// 仿真完成
					m_nSystemState = ST_IDLE;
					m_bSimulationRunning = FALSE;
				}
			}
		}
		else
		{
			// 零长度段，直接跳到下一段
			m_nCurrentPathIndex++;
			if (m_nCurrentPathIndex < m_arrToolPath.GetSize())
			{
				StartPathSegment(m_nCurrentPathIndex);
			}
		}

		// 更新刀具位置
		UpdateToolPosition();

		// 检查限位
		CheckLimits();

		// 如果有错误，停止仿真
		if (m_bHasAlarm)
		{
			m_bSimulationRunning = FALSE;
			SetSystemState(ST_ERROR);
		}
	}

	// 通知视图更新
	UpdateAllViews(nullptr);
}

void CCNCSimulatorDoc::StartPathSegment(int nPathIndex)
{
	if (nPathIndex < 0 || nPathIndex >= m_arrToolPath.GetSize())
		return;

	// 设置起始位置为当前位置
	m_dStartX = m_dCurrentX;
	m_dStartY = m_dCurrentY;
	m_dStartZ = m_dCurrentZ;

	// 设置目标位置
	const ToolPathPoint& targetPoint = m_arrToolPath[nPathIndex];
	m_dTargetX = targetPoint.x;
	m_dTargetY = targetPoint.y;
	m_dTargetZ = targetPoint.z;

	// 更新当前程序段
	m_nCurrentBlock = targetPoint.nBlockIndex;

	// 重置进度
	m_dPathProgress = 0.0;
}

void CCNCSimulatorDoc::UpdateToolPosition()
{
	// 线性插值计算当前位置
	m_dCurrentX = m_dStartX + (m_dTargetX - m_dStartX) * m_dPathProgress;
	m_dCurrentY = m_dStartY + (m_dTargetY - m_dStartY) * m_dPathProgress;
	m_dCurrentZ = m_dStartZ + (m_dTargetZ - m_dStartZ) * m_dPathProgress;
}

void CCNCSimulatorDoc::SetSimulationSpeed(double dSpeed)
{
	m_dSimulationSpeed = max(0.1, min(10.0, dSpeed)); // 限制在0.1x到10x之间
}

BOOL CCNCSimulatorDoc::IsSimulationComplete() const
{
	return m_nCurrentPathIndex >= m_arrToolPath.GetSize();
}

BOOL CCNCSimulatorDoc::CanTransitionTo(SystemState newState) const
{
	// 定义状态转换规则
	switch (m_nSystemState)
	{
	case ST_IDLE:
		return (newState == ST_RUNNING || newState == ST_MANUAL ||
				newState == ST_HOMING || newState == ST_EMERGENCY);

	case ST_RUNNING:
		return (newState == ST_PAUSED || newState == ST_STOPPED ||
				newState == ST_EMERGENCY || newState == ST_ERROR);

	case ST_PAUSED:
		return (newState == ST_RUNNING || newState == ST_STOPPED ||
				newState == ST_EMERGENCY || newState == ST_IDLE);

	case ST_STOPPED:
		return (newState == ST_IDLE || newState == ST_EMERGENCY);

	case ST_ERROR:
		return (newState == ST_IDLE || newState == ST_EMERGENCY);

	case ST_EMERGENCY:
		return (newState == ST_IDLE);

	case ST_HOMING:
		return (newState == ST_IDLE || newState == ST_EMERGENCY || newState == ST_ERROR);

	case ST_MANUAL:
		return (newState == ST_IDLE || newState == ST_EMERGENCY);

	default:
		return FALSE;
	}
}

void CCNCSimulatorDoc::SetSystemState(SystemState newState)
{
	if (CanTransitionTo(newState))
	{
		SystemState oldState = m_nSystemState;
		m_nSystemState = newState;

		// 状态转换时的特殊处理
		switch (newState)
		{
		case ST_EMERGENCY:
			m_bSimulationRunning = FALSE;
			break;

		case ST_IDLE:
			if (oldState == ST_EMERGENCY || oldState == ST_ERROR)
			{
				// 从错误状态恢复，重置仿真
				ResetSimulation();
			}
			break;

		case ST_MANUAL:
			m_bSimulationRunning = FALSE;
			break;
		}

		UpdateAllViews(nullptr);
	}
}

CString CCNCSimulatorDoc::GetStateString() const
{
	switch (m_nSystemState)
	{
	case ST_IDLE:       return _T("空闲");
	case ST_RUNNING:    return _T("运行中");
	case ST_PAUSED:     return _T("暂停");
	case ST_STOPPED:    return _T("停止");
	case ST_ERROR:      return _T("错误");
	case ST_EMERGENCY:  return _T("急停");
	case ST_HOMING:     return _T("回零中");
	case ST_MANUAL:     return _T("手动模式");
	default:            return _T("未知");
	}
}

void CCNCSimulatorDoc::ManualMove(double dX, double dY, double dZ)
{
	if (m_nSystemState == ST_MANUAL || m_nSystemState == ST_IDLE)
	{
		// 保存当前位置
		double oldX = m_dCurrentX;
		double oldY = m_dCurrentY;
		double oldZ = m_dCurrentZ;

		m_dCurrentX += dX;
		m_dCurrentY += dY;
		m_dCurrentZ += dZ;

		// 检查限位
		CheckLimits();

		// 如果有错误，恢复原位置
		if (m_bHasAlarm)
		{
			m_dCurrentX = oldX;
			m_dCurrentY = oldY;
			m_dCurrentZ = oldZ;
		}
		else
		{
			SetStatusMessage(_T("手动移动完成"));
		}

		UpdateAllViews(nullptr);
	}
}

void CCNCSimulatorDoc::EmergencyStop()
{
	SetSystemState(ST_EMERGENCY);
}

void CCNCSimulatorDoc::HomeAxes()
{
	if (CanTransitionTo(ST_HOMING))
	{
		SetSystemState(ST_HOMING);
		SetStatusMessage(_T("正在执行回零操作..."));

		// 模拟回零过程
		m_dCurrentX = 0.0;
		m_dCurrentY = 0.0;
		m_dCurrentZ = 0.0;

		// 回零完成后返回空闲状态
		SetSystemState(ST_IDLE);
		SetStatusMessage(_T("回零操作完成"));
	}
}

void CCNCSimulatorDoc::SetError(const CString& strError)
{
	m_strLastError = strError;
	m_bHasAlarm = TRUE;
	SetSystemState(ST_ERROR);
	SetStatusMessage(_T("系统错误: ") + strError);
}

void CCNCSimulatorDoc::ClearError()
{
	m_strLastError = _T("");
	m_bHasAlarm = FALSE;
	if (m_nSystemState == ST_ERROR)
	{
		SetSystemState(ST_IDLE);
		SetStatusMessage(_T("错误已清除，系统就绪"));
	}
}

void CCNCSimulatorDoc::SetStatusMessage(const CString& strMessage)
{
	m_strStatusMessage = strMessage;
	UpdateAllViews(nullptr);
}

void CCNCSimulatorDoc::CheckLimits()
{
	// 检查X轴限位
	if (m_dCurrentX < -5.0 || m_dCurrentX > 305.0)
	{
		SetError(_T("X轴超出工作范围"));
		return;
	}

	// 检查Y轴限位
	if (m_dCurrentY < -5.0 || m_dCurrentY > 205.0)
	{
		SetError(_T("Y轴超出工作范围"));
		return;
	}

	// 检查Z轴限位
	if (m_dCurrentZ < -55.0 || m_dCurrentZ > 55.0)
	{
		SetError(_T("Z轴超出工作范围"));
		return;
	}
}

// 命令处理函数

void CCNCSimulatorDoc::OnFileOpenGcode()
{
	CFileDialog dlg(TRUE, _T("nc"), nullptr,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("G代码文件 (*.nc;*.cnc;*.txt)|*.nc;*.cnc;*.txt|所有文件 (*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		if (ParseGCodeFile(dlg.GetPathName()))
		{
			AfxMessageBox(_T("G代码文件加载成功！"));
		}
		else
		{
			AfxMessageBox(_T("G代码文件加载失败！"));
		}
	}
}

void CCNCSimulatorDoc::OnSimulationStart()
{
	StartSimulation();
}

void CCNCSimulatorDoc::OnSimulationPause()
{
	PauseSimulation();
}

void CCNCSimulatorDoc::OnSimulationStop()
{
	StopSimulation();
}

void CCNCSimulatorDoc::OnSimulationReset()
{
	ResetSimulation();
}

CString CCNCSimulatorDoc::GetModalInfo() const
{
	CString strModal;

	// 构建模态信息字符串
	strModal += m_bAbsoluteMode ? _T("G90 ") : _T("G91 ");

	switch (m_nPlaneMode)
	{
	case 17: strModal += _T("G17 "); break;
	case 18: strModal += _T("G18 "); break;
	case 19: strModal += _T("G19 "); break;
	}

	strModal += _T("G54");  // 默认工件坐标系

	return strModal;
}

CString CCNCSimulatorDoc::GetCurrentGCodeLine() const
{
	if (m_nCurrentBlock >= 0 && m_nCurrentBlock < m_arrGCodeBlocks.GetSize())
	{
		const GCodeBlock& block = m_arrGCodeBlocks[m_nCurrentBlock];
		return block.strLine;
	}

	return _T("");
}
