#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

// G代码程序段结构
struct GCodeBlock
{
	CString strLine;        // 原始G代码行
	int nLineNumber;        // 行号
	CArray<int> arrGCodes;  // G代码数组 (支持多个G代码在同一行)
	int nMCode;             // M代码 (如 M03, M05等)
	double dX, dY, dZ;      // 坐标值
	double dI, dJ, dK;      // 圆弧中心偏移量
	double dR;              // 圆弧半径
	double dF;              // 进给速度
	double dS;              // 主轴转速
	BOOL bHasX, bHasY, bHasZ; // 是否包含坐标
	BOOL bHasI, bHasJ, bHasK; // 是否包含圆弧参数
	BOOL bHasR;             // 是否包含半径
	BOOL bHasF, bHasS;      // 是否包含F、S值

	GCodeBlock()
	{
		nLineNumber = 0;
		nMCode = -1;
		dX = dY = dZ = 0.0;
		dI = dJ = dK = 0.0;
		dR = 0.0;
		dF = dS = 0.0;
		bHasX = bHasY = bHasZ = FALSE;
		bHasI = bHasJ = bHasK = FALSE;
		bHasR = FALSE;
		bHasF = bHasS = FALSE;
	}

	// 拷贝构造函数
	GCodeBlock(const GCodeBlock& other)
	{
		*this = other;
	}

	// 赋值操作符
	GCodeBlock& operator=(const GCodeBlock& other)
	{
		if (this != &other)
		{
			strLine = other.strLine;
			nLineNumber = other.nLineNumber;
			nMCode = other.nMCode;
			dX = other.dX; dY = other.dY; dZ = other.dZ;
			dI = other.dI; dJ = other.dJ; dK = other.dK;
			dR = other.dR;
			dF = other.dF; dS = other.dS;
			bHasX = other.bHasX; bHasY = other.bHasY; bHasZ = other.bHasZ;
			bHasI = other.bHasI; bHasJ = other.bHasJ; bHasK = other.bHasK;
			bHasR = other.bHasR;
			bHasF = other.bHasF; bHasS = other.bHasS;

			// 复制G代码数组
			arrGCodes.RemoveAll();
			for (int i = 0; i < other.arrGCodes.GetSize(); i++)
			{
				arrGCodes.Add(other.arrGCodes[i]);
			}
		}
		return *this;
	}

	// 获取主要的G代码（用于移动类型判断）
	int GetMainGCode() const
	{
		// 返回移动相关的G代码 (G00, G01, G02, G03)
		for (int i = 0; i < arrGCodes.GetSize(); i++)
		{
			int gcode = arrGCodes[i];
			if (gcode >= 0 && gcode <= 3)
				return gcode;
		}
		// 如果没有移动G代码，返回第一个G代码
		return arrGCodes.GetSize() > 0 ? arrGCodes[0] : -1;
	}

	// 检查是否包含特定G代码
	BOOL HasGCode(int gcode) const
	{
		for (int i = 0; i < arrGCodes.GetSize(); i++)
		{
			if (arrGCodes[i] == gcode)
				return TRUE;
		}
		return FALSE;
	}
};

// 刀具路径点结构
struct ToolPathPoint
{
	double x, y, z;         // 坐标
	int nBlockIndex;        // 对应的程序段索引
	int nMoveType;          // 移动类型: 0=快速移动, 1=直线插补, 2=圆弧插补
	
	ToolPathPoint(double _x = 0, double _y = 0, double _z = 0, int _block = -1, int _type = 0)
		: x(_x), y(_y), z(_z), nBlockIndex(_block), nMoveType(_type) {}
};

// 系统状态枚举
enum SystemState
{
	ST_IDLE = 0,        // 空闲
	ST_RUNNING,         // 运行中
	ST_PAUSED,          // 暂停
	ST_STOPPED,         // 停止
	ST_ERROR,           // 错误状态
	ST_EMERGENCY,       // 急停状态
	ST_HOMING,          // 回零状态
	ST_MANUAL           // 手动模式
};

class CCNCSimulatorDoc : public CDocument
{
protected: // 仅从序列化创建
	CCNCSimulatorDoc() noexcept;
	DECLARE_DYNCREATE(CCNCSimulatorDoc)

// 特性
public:
	// G代码数据
	CArray<GCodeBlock> m_arrGCodeBlocks;    // G代码程序段数组
	CArray<ToolPathPoint> m_arrToolPath;    // 刀具路径点数组
	
	// 系统状态
	SystemState m_nSystemState;             // 当前系统状态
	int m_nCurrentBlock;                    // 当前执行的程序段
	double m_dCurrentX, m_dCurrentY, m_dCurrentZ; // 当前刀具位置
	double m_dFeedRate;                     // 当前进给速度
	double m_dSpindleSpeed;                 // 当前主轴转速

	// 仿真控制
	int m_nCurrentPathIndex;                // 当前路径点索引
	double m_dSimulationSpeed;              // 仿真速度倍率 (1.0 = 正常速度)
	BOOL m_bSimulationRunning;              // 仿真是否正在运行
	DWORD m_dwLastUpdateTime;               // 上次更新时间
	double m_dPathProgress;                 // 当前路径段的进度 (0.0-1.0)
	BOOL m_bSingleStepMode;                 // 单段执行模式
	BOOL m_bWaitingForStep;                 // 等待单段执行
	DWORD m_dwStartTime;                    // 仿真开始时间
	DWORD m_dwTotalRunTime;                 // 总运行时间（毫秒）

	// 动画插值
	double m_dTargetX, m_dTargetY, m_dTargetZ; // 目标位置
	double m_dStartX, m_dStartY, m_dStartZ;    // 起始位置

	// 报警和监控
	CString m_strLastError;                     // 最后的错误信息
	CString m_strStatusMessage;                 // 状态消息
	BOOL m_bHasAlarm;                          // 是否有报警
	
	// 模态状态
	BOOL m_bAbsoluteMode;                   // 绝对坐标模式 (G90/G91)
	int m_nPlaneMode;                       // 平面选择 (G17/G18/G19)
	
	// 倍率设置
	double m_dFeedOverride;                 // 进给倍率 (%)
	double m_dSpindleOverride;              // 主轴倍率 (%)

// 操作
public:
	// G代码解析
	BOOL ParseGCodeFile(const CString& strFilePath);
	BOOL ParseGCodeLine(const CString& strLine, GCodeBlock& block);
	BOOL ParseGCodeParameters(const CString& str, GCodeBlock& block);
	
	// 路径计算
	void CalculateToolPath();
	void CalculateArcPath(double startX, double startY, double endX, double endY,
		double centerOffsetX, double centerOffsetY, BOOL bClockwise, int blockIndex);
	
	// 仿真控制
	void StartSimulation();
	void PauseSimulation();
	void StopSimulation();
	void ResetSimulation();
	void UpdateSimulation();                // 更新仿真状态
	void SetSimulationSpeed(double dSpeed); // 设置仿真速度
	void SetSingleStepMode(BOOL bEnable);   // 设置单段执行模式
	void ExecuteNextStep();                 // 执行下一段
	BOOL IsSingleStepMode() const { return m_bSingleStepMode; }
	BOOL IsWaitingForStep() const { return m_bWaitingForStep; }
	DWORD GetRunTime() const;               // 获取运行时间（秒）
	double GetProgress() const;             // 获取总进度百分比

	// 动画控制
	void StartPathSegment(int nPathIndex);  // 开始新的路径段
	void UpdateToolPosition();              // 更新刀具位置
	BOOL IsSimulationComplete() const;      // 检查仿真是否完成

	// 状态机控制
	BOOL CanTransitionTo(SystemState newState) const;  // 检查状态转换是否合法
	void SetSystemState(SystemState newState);         // 设置系统状态
	CString GetStateString() const;                     // 获取状态字符串

	// 手动控制
	void ManualMove(double dX, double dY, double dZ);   // 手动移动
	void EmergencyStop();                               // 急停
	void HomeAxes();                                    // 回零

	// 报警和监控
	void SetError(const CString& strError);             // 设置错误信息
	void ClearError();                                  // 清除错误
	void SetStatusMessage(const CString& strMessage);   // 设置状态消息
	CString GetLastError() const { return m_strLastError; }
	CString GetStatusMessage() const { return m_strStatusMessage; }
	BOOL HasAlarm() const { return m_bHasAlarm; }
	void CheckLimits();                                 // 检查限位
	
	// 状态获取
	SystemState GetSystemState() const { return m_nSystemState; }
	void GetCurrentPosition(double& x, double& y, double& z) const;
	int GetCurrentBlockIndex() const { return m_nCurrentBlock; }
	
	// 倍率设置
	void SetFeedOverride(double dOverride) { m_dFeedOverride = dOverride; }
	void SetSpindleOverride(double dOverride) { m_dSpindleOverride = dOverride; }

	// 状态信息获取
	CString GetModalInfo() const;
	CString GetCurrentGCodeLine() const;

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// 命令处理
public:
	afx_msg void OnFileOpenGcode();
	afx_msg void OnSimulationStart();
	afx_msg void OnSimulationPause();
	afx_msg void OnSimulationStop();
	afx_msg void OnSimulationReset();

	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CCNCSimulatorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
