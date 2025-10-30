#pragma once

class CCNCSimulatorDoc;

// CControlPanel 对话框

class CControlPanel : public CDialogEx
{
	DECLARE_DYNAMIC(CControlPanel)

public:
	CControlPanel(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CControlPanel();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONTROL_PANEL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	// 控件变量
	CButton m_btnStart;
	CButton m_btnPause;
	CButton m_btnStop;
	CButton m_btnReset;
	CButton m_btnOpenFile;
	CButton m_btnEmergencyStop;
	CButton m_btnHome;
	CButton m_btnManualMode;
	CButton m_btnSingleStep;
	CButton m_btnNextStep;
	CSliderCtrl m_sliderFeedOverride;
	CSliderCtrl m_sliderSpindleOverride;
	CStatic m_staticFeedOverride;
	CStatic m_staticSpindleOverride;

	// 新增速度控制
	CSliderCtrl m_sliderSimSpeed;
	CStatic m_staticSimSpeed;

	// 手动控制按钮
	CButton m_btnXPlus;
	CButton m_btnXMinus;
	CButton m_btnYPlus;
	CButton m_btnYMinus;
	CButton m_btnZPlus;
	CButton m_btnZMinus;

	// 手轮模拟和移动控制
	CSliderCtrl m_sliderHandwheel;
	CStatic m_staticHandwheelValue;
	CComboBox m_comboMoveStep;      // 移动步长选择
	CButton m_checkContinuous;      // 连续移动复选框
	CButton m_checkHandwheel;       // 手轮模式复选框

	// 系统控制
	CButton m_btnClearAlarm;        // 清除报警按钮

	// 状态显示控件
	CStatic m_staticSystemState;    // 系统状态
	CStatic m_staticPosX;           // X坐标
	CStatic m_staticPosY;           // Y坐标
	CStatic m_staticPosZ;           // Z坐标
	CStatic m_staticRunTime;        // 运行时间
	CStatic m_staticProgress;       // 进度百分比
	CProgressCtrl m_progressBar;    // 进度条
	CStatic m_staticModalInfo;      // 模态信息
	CStatic m_staticCurrentGCode;   // 当前G代码
	CStatic m_staticAlarmMsg;       // 报警信息
	
	// 文档指针
	CCNCSimulatorDoc* m_pDoc;

	// 手动控制状态
	double m_dMoveStep;             // 当前移动步长
	BOOL m_bContinuousMove;         // 连续移动模式
	BOOL m_bHandwheelMode;          // 手轮模式
	double m_dSimSpeed;             // 仿真速度倍率
	
	// 操作
	void SetDocument(CCNCSimulatorDoc* pDoc) { m_pDoc = pDoc; }
	void UpdateControls();
	void SetButtonStyle(CButton* pButton, COLORREF color);
	void UpdateMoveStep();                  // 更新移动步长

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedReset();
	afx_msg void OnBnClickedOpenFile();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedXPlus();
	afx_msg void OnBnClickedXMinus();
	afx_msg void OnBnClickedYPlus();
	afx_msg void OnBnClickedYMinus();
	afx_msg void OnBnClickedZPlus();
	afx_msg void OnBnClickedZMinus();
	afx_msg void OnBnClickedEmergencyStop();
	afx_msg void OnBnClickedHome();
	afx_msg void OnBnClickedManualMode();
	afx_msg void OnBnClickedSingleStep();
	afx_msg void OnBnClickedNextStep();
	afx_msg void OnBnClickedClearAlarm();
	afx_msg void OnBnClickedContinuous();
	afx_msg void OnBnClickedHandwheel();
	afx_msg void OnCbnSelchangeMoveStep();
};
