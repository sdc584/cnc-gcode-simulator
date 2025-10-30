#include "pch.h"
#include "CNCSimulator.h"
#include "CNCSimulatorDoc.h"
#include "GraphicsView.h"
#include "CodeEditView.h"
#include "InfoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CGraphicsView

IMPLEMENT_DYNCREATE(CGraphicsView, CView)

CGraphicsView::CGraphicsView()
{
	// 初始化视图参数
	m_dScale = 5.0;             // 初始缩放比例 (5像素/毫米)
	m_ptOffset = CPoint(50, 50); // 初始偏移
	m_rectWorkArea = CRect(0, 0, 300, 200); // 工作区域 300x200mm
	
	// 创建画笔和画刷
	m_penRapid.CreatePen(PS_DASH, 1, RGB(255, 0, 0));      // 红色虚线
	m_penFeed.CreatePen(PS_SOLID, 2, RGB(0, 0, 255));      // 蓝色实线
	m_penAxis.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));        // 黑色实线
	m_penGrid.CreatePen(PS_DOT, 1, RGB(200, 200, 200));    // 灰色点线
	m_brushTool.CreateSolidBrush(RGB(0, 255, 0));          // 绿色
}

CGraphicsView::~CGraphicsView()
{
}

BEGIN_MESSAGE_MAP(CGraphicsView, CView)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CGraphicsView 绘制

void CGraphicsView::OnDraw(CDC* pDC)
{
	CCNCSimulatorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// 创建内存DC进行双缓冲绘制
	CRect rect;
	GetClientRect(&rect);
	
	CDC memDC;
	CBitmap memBitmap;
	memDC.CreateCompatibleDC(pDC);
	memBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);
	
	// 填充背景 - 专业CNC深色背景
	memDC.FillSolidRect(&rect, RGB(40, 40, 40));
	
	// 绘制各个元素
	DrawGrid(&memDC);
	DrawCoordinateSystem(&memDC);
	DrawWorkArea(&memDC);
	DrawToolPath(&memDC);
	DrawCurrentTool(&memDC);
	
	// 复制到屏幕
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

#ifdef _DEBUG
	// 在调试模式下显示性能信息
	static DWORD dwLastTime = 0;
	static int nFrameCount = 0;
	static double dFPS = 0.0;

	DWORD dwCurrentTime = GetTickCount();
	nFrameCount++;

	if (dwCurrentTime - dwLastTime >= 1000) // 每秒更新一次
	{
		dFPS = nFrameCount * 1000.0 / (dwCurrentTime - dwLastTime);
		dwLastTime = dwCurrentTime;
		nFrameCount = 0;
	}

	CString strFPS;
	strFPS.Format(_T("FPS: %.1f"), dFPS);
	pDC->SetTextColor(RGB(255, 255, 0));
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(rect.right - 80, 10, strFPS);
#endif

	// 清理
	memDC.SelectObject(pOldBitmap);
}

CCNCSimulatorDoc* CGraphicsView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)));
	return (CCNCSimulatorDoc*)m_pDocument;
}

CPoint CGraphicsView::WorldToScreen(double x, double y)
{
	// 获取客户区大小
	CRect rect;
	GetClientRect(&rect);

	// 将(0,0)点放在屏幕中心
	int centerX = rect.Width() / 2;
	int centerY = rect.Height() / 2;

	int screenX = (int)(x * m_dScale) + centerX;
	int screenY = (int)(-y * m_dScale) + centerY; // Y轴翻转，向上为正
	return CPoint(screenX, screenY);
}

void CGraphicsView::ScreenToWorld(CPoint pt, double& x, double& y)
{
	// 获取客户区大小
	CRect rect;
	GetClientRect(&rect);

	// 将屏幕坐标转换为世界坐标，(0,0)在屏幕中心
	int centerX = rect.Width() / 2;
	int centerY = rect.Height() / 2;

	x = (pt.x - centerX) / m_dScale;
	y = -(pt.y - centerY) / m_dScale; // Y轴翻转，向上为正
}

void CGraphicsView::DrawCoordinateSystem(CDC* pDC)
{
	CPen penAxis(PS_SOLID, 2, RGB(255, 255, 255));  // 白色粗线 - 适应深色背景
	CPen* pOldPen = pDC->SelectObject(&penAxis);

	// 绘制X轴
	CPoint origin = WorldToScreen(0, 0);
	CPoint xEnd = WorldToScreen(40, 0);
	pDC->MoveTo(origin);
	pDC->LineTo(xEnd);

	// X轴箭头
	pDC->MoveTo(xEnd.x - 8, xEnd.y - 4);
	pDC->LineTo(xEnd);
	pDC->LineTo(xEnd.x - 8, xEnd.y + 4);

	// 绘制Y轴
	CPoint yEnd = WorldToScreen(0, 40);
	pDC->MoveTo(origin);
	pDC->LineTo(yEnd);

	// Y轴箭头
	pDC->MoveTo(yEnd.x - 4, yEnd.y + 8);
	pDC->LineTo(yEnd);
	pDC->LineTo(yEnd.x + 4, yEnd.y + 8);

	// 绘制原点标记
	CBrush brushOrigin(RGB(255, 0, 0));
	CBrush* pOldBrush = pDC->SelectObject(&brushOrigin);
	pDC->Ellipse(origin.x - 3, origin.y - 3, origin.x + 3, origin.y + 3);
	pDC->SelectObject(pOldBrush);

	// 绘制标签
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255, 255, 255));  // 白色文字 - 适应深色背景
	CFont font;
	font.CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	CFont* pOldFont = pDC->SelectObject(&font);

	pDC->TextOut(xEnd.x + 8, xEnd.y - 8, _T("X"));
	pDC->TextOut(yEnd.x - 20, yEnd.y - 5, _T("Y"));
	pDC->TextOut(origin.x + 8, origin.y + 8, _T("(0,0)"));

	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
}

void CGraphicsView::DrawGrid(CDC* pDC)
{
	CPen penGrid(PS_SOLID, 1, RGB(80, 80, 80));      // 深灰色细线 - 适应深色背景
	CPen penMajorGrid(PS_SOLID, 1, RGB(120, 120, 120)); // 浅一点的灰色 - 主网格
	CPen* pOldPen = pDC->SelectObject(&penGrid);

	CRect rect;
	GetClientRect(&rect);

	// 绘制细网格线 (每5mm一条线)
	double gridSize = 5.0; // 5mm
	double majorGridSize = 10.0; // 10mm主网格

	// 垂直线
	for (double x = -50; x <= 350; x += gridSize)
	{
		CPoint pt1 = WorldToScreen(x, -50);
		CPoint pt2 = WorldToScreen(x, 250);
		if (pt1.x >= rect.left && pt1.x <= rect.right)
		{
			// 主网格线用深色
			if (fmod(x, majorGridSize) == 0.0)
				pDC->SelectObject(&penMajorGrid);
			else
				pDC->SelectObject(&penGrid);

			pDC->MoveTo(pt1.x, rect.top);
			pDC->LineTo(pt1.x, rect.bottom);
		}
	}

	// 水平线
	for (double y = -50; y <= 250; y += gridSize)
	{
		CPoint pt1 = WorldToScreen(-50, y);
		CPoint pt2 = WorldToScreen(350, y);
		if (pt1.y >= rect.top && pt1.y <= rect.bottom)
		{
			// 主网格线用深色
			if (fmod(y, majorGridSize) == 0.0)
				pDC->SelectObject(&penMajorGrid);
			else
				pDC->SelectObject(&penGrid);

			pDC->MoveTo(rect.left, pt1.y);
			pDC->LineTo(rect.right, pt1.y);
		}
	}

	pDC->SelectObject(pOldPen);
}

void CGraphicsView::DrawWorkArea(CDC* pDC)
{
	CPen penWorkArea(PS_SOLID, 2, RGB(0, 0, 255));  // 蓝色边界
	CPen* pOldPen = pDC->SelectObject(&penWorkArea);

	// 绘制工作区域边界 (根据红色阶梯路径调整范围)
	CPoint pt1 = WorldToScreen(0, 0);
	CPoint pt2 = WorldToScreen(35, 0);
	CPoint pt3 = WorldToScreen(35, 25);
	CPoint pt4 = WorldToScreen(0, 25);

	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);
	pDC->LineTo(pt3);
	pDC->LineTo(pt4);
	pDC->LineTo(pt1);

	// 绘制工作区域标签
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(100, 149, 237));  // 浅蓝色文字 - 适应深色背景
	CFont font;
	font.CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	CFont* pOldFont = pDC->SelectObject(&font);

	pDC->TextOut(pt3.x + 5, pt3.y - 15, _T("工作区域"));

	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
}

void CGraphicsView::DrawToolPath(CDC* pDC)
{
	CCNCSimulatorDoc* pDoc = GetDocument();
	if (!pDoc || pDoc->m_arrToolPath.IsEmpty())
		return;

	// 创建专业CNC界面风格的画笔
	CPen penRapid(PS_DOT, 1, RGB(128, 128, 128));     // 灰色虚线 - 快速移动
	CPen penFeed(PS_SOLID, 3, RGB(255, 0, 0));        // 红色实线 - 进给移动（阶梯路径）
	CPen penArc(PS_SOLID, 2, RGB(0, 128, 255));       // 蓝色实线 - 圆弧
	CPen penExecuted(PS_SOLID, 4, RGB(0, 255, 0));    // 绿色粗线 - 已执行路径
	CPen penCurrent(PS_SOLID, 5, RGB(255, 255, 0));   // 黄色粗线 - 当前执行路径

	CPoint lastPoint;
	BOOL bFirst = TRUE;
	int lastMoveType = -1;
	int currentPathIndex = pDoc->m_nCurrentPathIndex;

	for (int i = 0; i < pDoc->m_arrToolPath.GetSize(); i++)
	{
		const ToolPathPoint& pt = pDoc->m_arrToolPath[i];
		CPoint screenPt = WorldToScreen(pt.x, pt.y);

		if (!bFirst)
		{
			// 检查是否应该绘制连接线
			BOOL bShouldDrawLine = TRUE;

			// 跳过从原点(0,0)到第一个实际加工点的连接线
			if (i == 1 && pDoc->m_arrToolPath[0].x == 0.0 && pDoc->m_arrToolPath[0].y == 0.0)
			{
				bShouldDrawLine = FALSE;
			}

			// 跳过从最后一个加工点回到原点(0,0)的连接线
			if (i == pDoc->m_arrToolPath.GetSize() - 1 && pt.x == 0.0 && pt.y == 0.0)
			{
				bShouldDrawLine = FALSE;
			}

			// 跳过Z轴抬刀时的连接线（Z > 0表示抬刀状态）
			if (pt.z > 0.1 || (i > 0 && pDoc->m_arrToolPath[i-1].z > 0.1))
			{
				bShouldDrawLine = FALSE;
			}

			if (bShouldDrawLine)
			{
				// 根据执行状态和移动类型选择画笔
				CPen* pPen = nullptr;

				if (i <= currentPathIndex)
				{
					// 已执行的路径 - 绿色
					pPen = &penExecuted;
				}
				else if (i == currentPathIndex + 1)
				{
					// 当前执行的路径 - 黄色
					pPen = &penCurrent;
				}
				else
				{
					// 未执行的路径 - 根据类型选择
					switch (pt.nMoveType)
					{
					case 0: // G00 快速移动
						pPen = &penRapid;
						break;
					case 1: // G01 直线插补
						pPen = &penFeed;
						break;
					case 2: // G02/G03 圆弧插补
						pPen = &penArc;
						break;
					default:
						pPen = &penFeed;
						break;
					}
				}

				CPen* pOldPen = pDC->SelectObject(pPen);

				pDC->MoveTo(lastPoint);
				pDC->LineTo(screenPt);

				pDC->SelectObject(pOldPen);
			}
		}

		// 绘制关键路径点标记（转折点）- 但不包括原点
		if ((pt.x != 0.0 || pt.y != 0.0) && // 不在原点
			(i == 0 || i == pDoc->m_arrToolPath.GetSize() - 1 ||
			(i > 0 && pt.nMoveType != pDoc->m_arrToolPath[i-1].nMoveType)))
		{
			CBrush brushPoint(RGB(255, 0, 0)); // 红色点标记关键点
			CBrush* pOldBrush = pDC->SelectObject(&brushPoint);
			CPen penPoint(PS_SOLID, 1, RGB(0, 0, 0));
			CPen* pOldPointPen = pDC->SelectObject(&penPoint);

			pDC->Ellipse(screenPt.x - 3, screenPt.y - 3, screenPt.x + 3, screenPt.y + 3);

			pDC->SelectObject(pOldPointPen);
			pDC->SelectObject(pOldBrush);
		}

		lastPoint = screenPt;
		lastMoveType = pt.nMoveType;
		bFirst = FALSE;
	}

	// 绘制起点和终点标记（跳过原点）
	if (!pDoc->m_arrToolPath.IsEmpty())
	{
		// 找到第一个非原点的加工起点
		int startIndex = -1;
		for (int i = 0; i < pDoc->m_arrToolPath.GetSize(); i++)
		{
			const ToolPathPoint& pt = pDoc->m_arrToolPath[i];
			if (pt.x != 0.0 || pt.y != 0.0) // 不是原点
			{
				startIndex = i;
				break;
			}
		}

		// 找到最后一个非原点的加工终点
		int endIndex = -1;
		for (int i = pDoc->m_arrToolPath.GetSize() - 1; i >= 0; i--)
		{
			const ToolPathPoint& pt = pDoc->m_arrToolPath[i];
			if (pt.x != 0.0 || pt.y != 0.0) // 不是原点
			{
				endIndex = i;
				break;
			}
		}

		// 绘制加工起点 - 绿色圆圈带标签
		if (startIndex >= 0)
		{
			const ToolPathPoint& startPt = pDoc->m_arrToolPath[startIndex];
			CPoint startScreen = WorldToScreen(startPt.x, startPt.y);
			CBrush brushStart(RGB(0, 255, 0));
			CBrush* pOldBrush = pDC->SelectObject(&brushStart);
			CPen penStart(PS_SOLID, 2, RGB(0, 128, 0));
			CPen* pOldStartPen = pDC->SelectObject(&penStart);
			pDC->Ellipse(startScreen.x - 6, startScreen.y - 6, startScreen.x + 6, startScreen.y + 6);

			// 起点标签
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(0, 128, 0));
			pDC->TextOut(startScreen.x + 10, startScreen.y - 5, _T("起点"));

			pDC->SelectObject(pOldStartPen);
			pDC->SelectObject(pOldBrush);
		}

		// 绘制加工终点 - 红色方块带标签
		if (endIndex >= 0 && endIndex != startIndex)
		{
			const ToolPathPoint& endPt = pDoc->m_arrToolPath[endIndex];
			CPoint endScreen = WorldToScreen(endPt.x, endPt.y);
			CBrush brushEnd(RGB(255, 0, 0));
			CBrush* pOldBrush = pDC->SelectObject(&brushEnd);
			CPen penEnd(PS_SOLID, 2, RGB(128, 0, 0));
			CPen* pOldEndPen = pDC->SelectObject(&penEnd);
			pDC->Rectangle(endScreen.x - 6, endScreen.y - 6, endScreen.x + 6, endScreen.y + 6);

			// 终点标签
			pDC->SetTextColor(RGB(128, 0, 0));
			pDC->TextOut(endScreen.x + 10, endScreen.y - 5, _T("终点"));

			pDC->SelectObject(pOldEndPen);
			pDC->SelectObject(pOldBrush);
		}
	}
}

void CGraphicsView::DrawCurrentTool(CDC* pDC)
{
	CCNCSimulatorDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	// 绘制当前刀具位置
	double x, y, z;
	pDoc->GetCurrentPosition(x, y, z);

	CPoint toolPos = WorldToScreen(x, y);

	// 绘制刀具 - 专业CNC风格
	CBrush brushTool(RGB(255, 255, 0));  // 黄色刀具
	CBrush* pOldBrush = pDC->SelectObject(&brushTool);
	CPen penTool(PS_SOLID, 2, RGB(255, 165, 0));  // 橙色边框
	CPen* pOldPen = pDC->SelectObject(&penTool);

	int radius = 5;
	pDC->Ellipse(toolPos.x - radius, toolPos.y - radius,
				 toolPos.x + radius, toolPos.y + radius);

	// 绘制刀具中心十字线
	CPen penCross(PS_SOLID, 1, RGB(0, 0, 0));
	pDC->SelectObject(&penCross);
	pDC->MoveTo(toolPos.x - 3, toolPos.y);
	pDC->LineTo(toolPos.x + 3, toolPos.y);
	pDC->MoveTo(toolPos.x, toolPos.y - 3);
	pDC->LineTo(toolPos.x, toolPos.y + 3);

	// 显示当前坐标
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255, 165, 0));
	CString strPos;
	strPos.Format(_T("(%.1f, %.1f)"), x, y);
	pDC->TextOut(toolPos.x + 8, toolPos.y - 15, strPos);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CGraphicsView::ZoomIn()
{
	m_dScale *= 1.2;
	Invalidate();
}

void CGraphicsView::ZoomOut()
{
	m_dScale /= 1.2;
	if (m_dScale < 0.1)
		m_dScale = 0.1;
	Invalidate();
}

void CGraphicsView::ZoomFit()
{
	CCNCSimulatorDoc* pDoc = GetDocument();
	if (!pDoc || pDoc->m_arrToolPath.IsEmpty())
		return;

	// 计算路径边界
	double minX = 1e6, maxX = -1e6, minY = 1e6, maxY = -1e6;
	for (int i = 0; i < pDoc->m_arrToolPath.GetSize(); i++)
	{
		const ToolPathPoint& pt = pDoc->m_arrToolPath[i];
		if (pt.x < minX) minX = pt.x;
		if (pt.x > maxX) maxX = pt.x;
		if (pt.y < minY) minY = pt.y;
		if (pt.y > maxY) maxY = pt.y;
	}

	CRect rect;
	GetClientRect(&rect);

	double rangeX = maxX - minX;
	double rangeY = maxY - minY;

	if (rangeX > 0 && rangeY > 0)
	{
		double scaleX = (rect.Width() - 100) / rangeX;
		double scaleY = (rect.Height() - 100) / rangeY;
		m_dScale = min(scaleX, scaleY);

		m_ptOffset.x = 50 - (int)(minX * m_dScale);
		m_ptOffset.y = 50 + (int)(maxY * m_dScale);
	}

	Invalidate();
}

void CGraphicsView::ResetView()
{
	m_dScale = 5.0;
	m_ptOffset = CPoint(50, 50);
	Invalidate();
}

// CGraphicsView 诊断

#ifdef _DEBUG
void CGraphicsView::AssertValid() const
{
	CView::AssertValid();
}

void CGraphicsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

// CGraphicsView 消息处理程序

void CGraphicsView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	double x, y;
	ScreenToWorld(point, x, y);

	CString str;
	str.Format(_T("点击位置: X=%.2f, Y=%.2f"), x, y);
	AfxMessageBox(str);

	CView::OnLButtonDown(nFlags, point);
}

void CGraphicsView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CView::OnMouseMove(nFlags, point);
}

BOOL CGraphicsView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (zDelta > 0)
		ZoomIn();
	else
		ZoomOut();

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CGraphicsView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}
