#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "CNCSimulator.h"
#endif

#include "CNCSimulatorDoc.h"
#include "CNCSimulatorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCNCSimulatorView

IMPLEMENT_DYNCREATE(CCNCSimulatorView, CView)

BEGIN_MESSAGE_MAP(CCNCSimulatorView, CView)
END_MESSAGE_MAP()

// CCNCSimulatorView 构造/析构

CCNCSimulatorView::CCNCSimulatorView() noexcept
{
	// TODO: 在此处添加构造代码
}

CCNCSimulatorView::~CCNCSimulatorView()
{
}

BOOL CCNCSimulatorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CCNCSimulatorView 绘制

void CCNCSimulatorView::OnDraw(CDC* pDC)
{
	CCNCSimulatorDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	CRect rect;
	GetClientRect(&rect);

	// 绘制背景
	pDC->FillSolidRect(&rect, RGB(255, 255, 255));

	// 创建字体
	CFont font;
	font.CreatePointFont(90, _T("Consolas"));
	CFont* pOldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);

	// 调试信息
	TRACE(_T("OnDraw: G代码块数量 = %d\n"), pDoc->m_arrGCodeBlocks.GetSize());

	// 如果有G代码数据，显示G代码内容
	if (pDoc->m_arrGCodeBlocks.GetSize() > 0)
	{
		int y = 10;
		int lineHeight = 16;

		for (int i = 0; i < pDoc->m_arrGCodeBlocks.GetSize() && y < rect.bottom - 20; i++)
		{
			const GCodeBlock& block = pDoc->m_arrGCodeBlocks[i];
			CString strLine;
			strLine.Format(_T("%3d: %s"), block.nLineNumber, block.strLine);

			// 设置不同颜色：注释为绿色，G代码为黑色
			if (block.strLine.GetLength() > 0 && block.strLine[0] == ';')
			{
				pDC->SetTextColor(RGB(0, 128, 0)); // 绿色注释
			}
			else
			{
				pDC->SetTextColor(RGB(0, 0, 0)); // 黑色G代码
			}

			pDC->TextOut(10, y, strLine);
			y += lineHeight;
		}
	}
	else
	{
		// 显示提示信息
		pDC->DrawText(_T("CNC Simulator - 请使用 File -> Open 打开G代码文件"),
			&rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	pDC->SelectObject(pOldFont);
}

// CCNCSimulatorView 诊断

#ifdef _DEBUG
void CCNCSimulatorView::AssertValid() const
{
	CView::AssertValid();
}

void CCNCSimulatorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCNCSimulatorDoc* CCNCSimulatorView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)));
	return (CCNCSimulatorDoc*)m_pDocument;
}
#endif //_DEBUG

// CCNCSimulatorView 消息处理程序
