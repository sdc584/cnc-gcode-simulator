#include "pch.h"
#include "CNCSimulator.h"
#include "CNCSimulatorDoc.h"
#include "CodeEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCodeEditView

IMPLEMENT_DYNCREATE(CCodeEditView, CEditView)

CCodeEditView::CCodeEditView()
{
	m_nHighlightLine = -1;
	m_brushHighlight.CreateSolidBrush(RGB(0, 100, 200)); // 蓝色高亮 - 专业CNC风格
}

CCodeEditView::~CCodeEditView()
{
}

BEGIN_MESSAGE_MAP(CCodeEditView, CEditView)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CCodeEditView 绘制

void CCodeEditView::OnDraw(CDC* pDC)
{
	// 先调用基类绘制
	CEditView::OnDraw(pDC);
	
	// 绘制高亮行
	if (m_nHighlightLine >= 0)
	{
		CEdit& edit = GetEditCtrl();
		int nLineIndex = edit.LineIndex(m_nHighlightLine);
		if (nLineIndex >= 0)
		{
			CPoint pt = edit.PosFromChar(nLineIndex);
			CRect rect;
			GetClientRect(&rect);
			
			// 计算行高
			TEXTMETRIC tm;
			pDC->GetTextMetrics(&tm);
			int nLineHeight = tm.tmHeight + tm.tmExternalLeading;
			
			// 绘制高亮背景
			CRect highlightRect(0, pt.y, rect.Width(), pt.y + nLineHeight);
			pDC->FillRect(&highlightRect, &m_brushHighlight);
		}
	}
}

CCNCSimulatorDoc* CCodeEditView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)));
	return (CCNCSimulatorDoc*)m_pDocument;
}

void CCodeEditView::UpdateGCodeDisplay()
{
	CCNCSimulatorDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	CString strContent;
	for (int i = 0; i < pDoc->m_arrGCodeBlocks.GetSize(); i++)
	{
		const GCodeBlock& block = pDoc->m_arrGCodeBlocks[i];

		// 添加行号和格式化的G代码
		CString strLine;
		strLine.Format(_T("%4d: %s\r\n"), block.nLineNumber, block.strLine);
		strContent += strLine;
	}

	// 如果没有G代码，显示提示信息
	if (pDoc->m_arrGCodeBlocks.IsEmpty())
	{
		strContent = _T("没有加载G代码文件。\r\n请使用菜单 File -> Open G-Code 或控制面板的\"打开文件\"按钮加载G代码文件。");
	}

	SetWindowText(strContent);

	// 设置专业CNC字体 - 使用支持中文的字体
	CFont font;
	font.CreatePointFont(90, _T("Consolas"));  // 等宽字体，支持中文显示
	GetEditCtrl().SetFont(&font);

	// 设置只读模式
	GetEditCtrl().SetReadOnly(TRUE);
}

void CCodeEditView::HighlightLine(int nLine)
{
	m_nHighlightLine = nLine;
	Invalidate();
	
	// 滚动到指定行
	if (nLine >= 0)
	{
		CEdit& edit = GetEditCtrl();
		edit.LineScroll(nLine - edit.GetFirstVisibleLine());
	}
}

void CCodeEditView::ClearHighlight()
{
	m_nHighlightLine = -1;
	Invalidate();
}

void CCodeEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// 更新G代码显示
	UpdateGCodeDisplay();
	
	CEditView::OnUpdate(pSender, lHint, pHint);
}

// CCodeEditView 诊断

#ifdef _DEBUG
void CCodeEditView::AssertValid() const
{
	CEditView::AssertValid();
}

void CCodeEditView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

// CCodeEditView 消息处理程序

void CCodeEditView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	// 获取点击的行号
	CEdit& edit = GetEditCtrl();
	int nChar = edit.CharFromPos(point);
	int nLine = edit.LineFromChar(nChar);

	// 显示行信息
	CCNCSimulatorDoc* pDoc = GetDocument();
	if (pDoc && nLine < pDoc->m_arrGCodeBlocks.GetSize())
	{
		const GCodeBlock& block = pDoc->m_arrGCodeBlocks[nLine];
		CString str;
		str.Format(_T("行 %d: %s"), block.nLineNumber, block.strLine);
		AfxMessageBox(str);
	}

	CEditView::OnLButtonDown(nFlags, point);
}

BOOL CCodeEditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// 计算滚动行数 - 每次滚动3行，提供流畅的滚动体验
	int nLinesToScroll = 3;

	// 根据滚轮方向确定滚动方向
	if (zDelta > 0)
	{
		// 向上滚动（显示前面的内容）
		nLinesToScroll = -nLinesToScroll;
	}

	// 执行滚动
	CEdit& edit = GetEditCtrl();
	edit.LineScroll(nLinesToScroll);

	// 更新显示
	Invalidate();

	return TRUE; // 表示消息已处理
}
