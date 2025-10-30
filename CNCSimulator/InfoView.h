#pragma once

class CCNCSimulatorDoc;

class CInfoView : public CView
{
	DECLARE_DYNCREATE(CInfoView)

protected:
	CInfoView();           // 动态创建所使用的受保护的构造函数
	virtual ~CInfoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	CCNCSimulatorDoc* GetDocument() const;

	// 更新显示
	void UpdateDisplay();

protected:
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnInitialUpdate();

	DECLARE_MESSAGE_MAP()
};
