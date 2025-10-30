#pragma once

class CCNCSimulatorDoc;

class CCodeEditView : public CEditView
{
	DECLARE_DYNCREATE(CCodeEditView)

protected:
	CCodeEditView();           // 动态创建所使用的受保护的构造函数
	virtual ~CCodeEditView();

public:
	virtual void OnDraw(CDC* pDC);      // 重写以绘制该视图
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	int m_nHighlightLine;       // 当前高亮显示的行号
	CBrush m_brushHighlight;    // 高亮画刷

public:
	CCNCSimulatorDoc* GetDocument() const;
	
	// 更新显示
	void UpdateGCodeDisplay();
	void HighlightLine(int nLine);
	void ClearHighlight();

protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};
