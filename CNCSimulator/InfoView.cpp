#include "pch.h"
#include "CNCSimulator.h"
#include "CNCSimulatorDoc.h"
#include "InfoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CInfoView

IMPLEMENT_DYNCREATE(CInfoView, CView)

CInfoView::CInfoView()
{
}

CInfoView::~CInfoView()
{
}

BEGIN_MESSAGE_MAP(CInfoView, CView)
END_MESSAGE_MAP()

// CInfoView 诊断

#ifdef _DEBUG
void CInfoView::AssertValid() const
{
	CView::AssertValid();
}

void CInfoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

CCNCSimulatorDoc* CInfoView::GetDocument() const
{
	if (m_pDocument && m_pDocument->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)))
		return (CCNCSimulatorDoc*)m_pDocument;
	return nullptr;
}

void CInfoView::OnDraw(CDC* pDC)
{
	CCNCSimulatorDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	CRect rect;
	GetClientRect(&rect);

	// 设置专业CNC界面背景色
	pDC->FillSolidRect(&rect, RGB(60, 60, 60));

	// 设置字体
	CFont font;
	font.CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	CFont* pOldFont = pDC->SelectObject(&font);

	// 设置文字颜色为绿色（经典CNC显示颜色）
	pDC->SetTextColor(RGB(0, 255, 0));
	pDC->SetBkMode(TRANSPARENT);

	// 显示专业CNC信息
	int y = 15;
	CString str;

	// 绘制标题栏
	CBrush brushTitle(RGB(0, 100, 0));
	CRect titleRect(5, 5, rect.Width() - 5, 35);
	pDC->FillRect(&titleRect, &brushTitle);
	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->TextOut(10, 12, _T("CNC 监控面板"));
	y = 45;

	// 绝对坐标显示
	pDC->SetTextColor(RGB(255, 255, 0));  // 黄色标题
	pDC->TextOut(10, y, _T("绝对坐标"));
	y += 25;

	double x, yPos, z;
	pDoc->GetCurrentPosition(x, yPos, z);
	pDC->SetTextColor(RGB(0, 255, 0));    // 绿色数值
	str.Format(_T("X  %8.3f"), x);
	pDC->TextOut(20, y, str);
	y += 22;

	str.Format(_T("Y  %8.3f"), yPos);
	pDC->TextOut(20, y, str);
	y += 22;

	str.Format(_T("Z  %8.3f"), z);
	pDC->TextOut(20, y, str);
	y += 35;

	// 系统状态
	pDC->SetTextColor(RGB(255, 255, 0));  // 黄色标题
	pDC->TextOut(10, y, _T("系统状态"));
	y += 25;

	pDC->SetTextColor(RGB(0, 255, 0));    // 绿色数值
	str.Format(_T("状态: %s"), pDoc->GetStateString());
	pDC->TextOut(20, y, str);
	y += 22;

	str.Format(_T("当前行: %d"), pDoc->GetCurrentBlockIndex() + 1);
	pDC->TextOut(20, y, str);
	y += 22;

	str.Format(_T("进给: %.0f mm/min"), pDoc->m_dFeedRate);
	pDC->TextOut(20, y, str);
	y += 22;

	str.Format(_T("主轴: %.0f RPM"), pDoc->m_dSpindleSpeed);
	pDC->TextOut(20, y, str);
	y += 35;

	// 进度信息
	pDC->SetTextColor(RGB(255, 255, 0));  // 黄色标题
	pDC->TextOut(10, y, _T("进度信息"));
	y += 25;

	pDC->SetTextColor(RGB(0, 255, 0));    // 绿色数值
	str.Format(_T("进度: %.1f%%"), pDoc->GetProgress());
	pDC->TextOut(20, y, str);
	y += 22;

	DWORD runTime = pDoc->GetRunTime();
	int hours = runTime / 3600;
	int minutes = (runTime % 3600) / 60;
	int seconds = runTime % 60;
	str.Format(_T("时间: %02d:%02d:%02d"), hours, minutes, seconds);
	pDC->TextOut(20, y, str);
	y += 35;

	// 模态信息
	pDC->SetTextColor(RGB(255, 255, 0));  // 黄色标题
	pDC->TextOut(10, y, _T("模态信息"));
	y += 25;

	pDC->SetTextColor(RGB(0, 255, 0));    // 绿色数值
	str.Format(_T("G90 G17 G94"));  // 显示当前模态
	pDC->TextOut(20, y, str);
	y += 22;

	// 倍率信息
	str.Format(_T("进给倍率: %.0f%%"), pDoc->m_dFeedOverride);
	pDC->TextOut(20, y, str);
	y += 22;

	str.Format(_T("主轴倍率: %.0f%%"), pDoc->m_dSpindleOverride);
	pDC->TextOut(20, y, str);

	pDC->SelectObject(pOldFont);
}

void CInfoView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类
	UpdateDisplay();
}

void CInfoView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UpdateDisplay();
	CView::OnUpdate(pSender, lHint, pHint);
}

void CInfoView::UpdateDisplay()
{
	// 简单地重绘视图
	Invalidate();
}

// CInfoView 消息处理程序
