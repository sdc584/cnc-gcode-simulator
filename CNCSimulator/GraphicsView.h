#pragma once

class CCNCSimulatorDoc;

class CGraphicsView : public CView
{
	DECLARE_DYNCREATE(CGraphicsView)

protected:
	CGraphicsView();           // 动态创建所使用的受保护的构造函数
	virtual ~CGraphicsView();

public:
	virtual void OnDraw(CDC* pDC);      // 重写以绘制该视图
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// 视图参数
	double m_dScale;            // 缩放比例
	CPoint m_ptOffset;          // 偏移量
	CRect m_rectWorkArea;       // 工作区域
	
	// 绘制参数
	CPen m_penRapid;           // 快速移动画笔 (红色虚线)
	CPen m_penFeed;            // 进给移动画笔 (蓝色实线)
	CPen m_penAxis;            // 坐标轴画笔 (黑色)
	CPen m_penGrid;            // 网格画笔 (灰色)
	CBrush m_brushTool;        // 刀具画刷 (绿色)

public:
	CCNCSimulatorDoc* GetDocument() const;
	
	// 坐标转换
	CPoint WorldToScreen(double x, double y);
	void ScreenToWorld(CPoint pt, double& x, double& y);
	
	// 视图控制
	void ZoomIn();
	void ZoomOut();
	void ZoomFit();
	void ResetView();

protected:
	// 绘制函数
	void DrawCoordinateSystem(CDC* pDC);
	void DrawGrid(CDC* pDC);
	void DrawToolPath(CDC* pDC);
	void DrawCurrentTool(CDC* pDC);
	void DrawWorkArea(CDC* pDC);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
