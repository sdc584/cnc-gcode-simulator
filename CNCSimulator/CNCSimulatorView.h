#pragma once

class CCNCSimulatorDoc;

class CCNCSimulatorView : public CView
{
protected: // 仅从序列化创建
	CCNCSimulatorView() noexcept;
	DECLARE_DYNCREATE(CCNCSimulatorView)

// 特性
public:
	CCNCSimulatorDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 实现
public:
	virtual ~CCNCSimulatorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // CNCSimulatorView.cpp 中的调试版本
inline CCNCSimulatorDoc* CCNCSimulatorView::GetDocument() const
   { return reinterpret_cast<CCNCSimulatorDoc*>(m_pDocument); }
#endif
