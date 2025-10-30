#pragma once

// 简单控制面板类 - 基于普通窗口，避免对话框复杂性
class CCNCSimulatorDoc;

class CSimpleControlPanel : public CWnd
{
public:
    CSimpleControlPanel();
    virtual ~CSimpleControlPanel();

    // 创建控制面板
    BOOL Create(CWnd* pParentWnd, const RECT& rect);
    
    // 设置文档
    void SetDocument(CCNCSimulatorDoc* pDoc) { m_pDoc = pDoc; }
    
    // 更新控制状态
    void UpdateControls();

protected:
    CCNCSimulatorDoc* m_pDoc;
    
    // 控件ID定义
    enum {
        ID_BTN_START = 1001,
        ID_BTN_PAUSE = 1002,
        ID_BTN_STOP = 1003,
        ID_BTN_RESET = 1004,
        ID_BTN_OPEN_FILE = 1005,
        ID_BTN_EMERGENCY_STOP = 1006,
        ID_BTN_SINGLE_STEP = 1007,
        ID_BTN_NEXT_STEP = 1008,
        ID_BTN_MANUAL_MODE = 1009,
        ID_BTN_HOME = 1010,
        ID_BTN_X_PLUS = 1011,
        ID_BTN_X_MINUS = 1012,
        ID_BTN_Y_PLUS = 1013,
        ID_BTN_Y_MINUS = 1014,
        ID_BTN_Z_PLUS = 1015,
        ID_BTN_Z_MINUS = 1016,
        ID_SLIDER_FEED = 1017,
        ID_SLIDER_SPINDLE = 1018,
        ID_COMBO_STEP = 1019
    };
    
    // 控件变量
    CButton m_btnStart;
    CButton m_btnPause;
    CButton m_btnStop;
    CButton m_btnReset;
    CButton m_btnOpenFile;
    CButton m_btnEmergencyStop;
    CButton m_btnSingleStep;
    CButton m_btnNextStep;
    CButton m_btnManualMode;
    CButton m_btnHome;
    CButton m_btnXPlus;
    CButton m_btnXMinus;
    CButton m_btnYPlus;
    CButton m_btnYMinus;
    CButton m_btnZPlus;
    CButton m_btnZMinus;
    CSliderCtrl m_sliderFeed;
    CSliderCtrl m_sliderSpindle;
    CComboBox m_comboStep;
    CStatic m_staticFeedLabel;
    CStatic m_staticSpindleLabel;
    CStatic m_staticStepLabel;
    
    // 状态变量
    BOOL m_bSingleStepMode;
    BOOL m_bManualMode;
    double m_dMoveStep;
    
    // 创建控件
    void CreateControls();
    
    // 消息处理
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnBtnStart();
    afx_msg void OnBtnPause();
    afx_msg void OnBtnStop();
    afx_msg void OnBtnReset();
    afx_msg void OnBtnOpenFile();
    afx_msg void OnBtnEmergencyStop();
    afx_msg void OnBtnSingleStep();
    afx_msg void OnBtnNextStep();
    afx_msg void OnBtnManualMode();
    afx_msg void OnBtnHome();
    afx_msg void OnBtnXPlus();
    afx_msg void OnBtnXMinus();
    afx_msg void OnBtnYPlus();
    afx_msg void OnBtnYMinus();
    afx_msg void OnBtnZPlus();
    afx_msg void OnBtnZMinus();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnCbnSelchangeMoveStep();
    
    DECLARE_MESSAGE_MAP()
};
