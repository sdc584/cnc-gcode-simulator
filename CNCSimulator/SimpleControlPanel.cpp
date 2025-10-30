#include "pch.h"
#include "SimpleControlPanel.h"
#include "CNCSimulatorDoc.h"

BEGIN_MESSAGE_MAP(CSimpleControlPanel, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_COMMAND(ID_BTN_START, OnBtnStart)
    ON_COMMAND(ID_BTN_PAUSE, OnBtnPause)
    ON_COMMAND(ID_BTN_STOP, OnBtnStop)
    ON_COMMAND(ID_BTN_RESET, OnBtnReset)
    ON_COMMAND(ID_BTN_OPEN_FILE, OnBtnOpenFile)
    ON_COMMAND(ID_BTN_EMERGENCY_STOP, OnBtnEmergencyStop)
    ON_COMMAND(ID_BTN_SINGLE_STEP, OnBtnSingleStep)
    ON_COMMAND(ID_BTN_NEXT_STEP, OnBtnNextStep)
    ON_COMMAND(ID_BTN_MANUAL_MODE, OnBtnManualMode)
    ON_COMMAND(ID_BTN_HOME, OnBtnHome)
    ON_COMMAND(ID_BTN_X_PLUS, OnBtnXPlus)
    ON_COMMAND(ID_BTN_X_MINUS, OnBtnXMinus)
    ON_COMMAND(ID_BTN_Y_PLUS, OnBtnYPlus)
    ON_COMMAND(ID_BTN_Y_MINUS, OnBtnYMinus)
    ON_COMMAND(ID_BTN_Z_PLUS, OnBtnZPlus)
    ON_COMMAND(ID_BTN_Z_MINUS, OnBtnZMinus)
    ON_CBN_SELCHANGE(ID_COMBO_STEP, OnCbnSelchangeMoveStep)
    ON_WM_HSCROLL()
END_MESSAGE_MAP()

CSimpleControlPanel::CSimpleControlPanel()
{
    m_pDoc = nullptr;
    m_bSingleStepMode = FALSE;
    m_bManualMode = FALSE;
    m_dMoveStep = 1.0;
}

CSimpleControlPanel::~CSimpleControlPanel()
{
}

BOOL CSimpleControlPanel::Create(CWnd* pParentWnd, const RECT& rect)
{
    // 注册窗口类
    CString strWndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(COLOR_3DFACE + 1),
        NULL);

    // 创建窗口
    return CWnd::Create(strWndClass, _T("CNC Control Panel"), 
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        rect, pParentWnd, 0);
}

int CSimpleControlPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    CreateControls();
    return 0;
}

void CSimpleControlPanel::CreateControls()
{
    // 第一行：循环控制 - 更宽松的间距 (y=20-45)
    m_btnStart.Create(_T("▶ 开始"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(20, 20, 110, 45), this, ID_BTN_START);
    m_btnPause.Create(_T("⏸ 暂停"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(120, 20, 210, 45), this, ID_BTN_PAUSE);
    m_btnStop.Create(_T("⏹ 停止"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(220, 20, 310, 45), this, ID_BTN_STOP);
    m_btnReset.Create(_T("🔄 复位"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(320, 20, 410, 45), this, ID_BTN_RESET);

    // 单段执行控制
    m_btnSingleStep.Create(_T("📝 单段"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(430, 20, 520, 45), this, ID_BTN_SINGLE_STEP);
    m_btnNextStep.Create(_T("➡ 下一步"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(530, 20, 630, 45), this, ID_BTN_NEXT_STEP);

    // 文件操作和急停
    m_btnOpenFile.Create(_T("📁 打开"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(650, 20, 740, 45), this, ID_BTN_OPEN_FILE);
    m_btnEmergencyStop.Create(_T("🛑 急停"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(760, 20, 850, 45), this, ID_BTN_EMERGENCY_STOP);

    // 第二行：速度调节 - 更长的滑块 (y=55-85)
    m_staticFeedLabel.Create(_T("进给倍率: 100%"), WS_CHILD | WS_VISIBLE,
        CRect(20, 55, 150, 70), this, -1);
    m_sliderFeed.Create(WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_BOTH | TBS_NOTICKS,
        CRect(20, 70, 220, 85), this, ID_SLIDER_FEED);
    m_sliderFeed.SetRange(10, 200);
    m_sliderFeed.SetPos(100);

    m_staticSpindleLabel.Create(_T("主轴倍率: 100%"), WS_CHILD | WS_VISIBLE,
        CRect(250, 55, 380, 70), this, -1);
    m_sliderSpindle.Create(WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_BOTH | TBS_NOTICKS,
        CRect(250, 70, 450, 85), this, ID_SLIDER_SPINDLE);
    m_sliderSpindle.SetRange(10, 200);
    m_sliderSpindle.SetPos(100);

    // 第三行：手动控制 - 更大的按钮 (y=95-125)
    m_btnManualMode.Create(_T("🔧 手动"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(20, 95, 120, 125), this, ID_BTN_MANUAL_MODE);
    m_btnHome.Create(_T("🏠 回零"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(140, 95, 230, 125), this, ID_BTN_HOME);

    // 手轮模拟 - 轴向控制按钮 (更大更易点击)
    m_btnXPlus.Create(_T("X+"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(260, 95, 310, 125), this, ID_BTN_X_PLUS);
    m_btnXMinus.Create(_T("X-"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(320, 95, 370, 125), this, ID_BTN_X_MINUS);

    m_btnYPlus.Create(_T("Y+"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(390, 95, 440, 125), this, ID_BTN_Y_PLUS);
    m_btnYMinus.Create(_T("Y-"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(450, 95, 500, 125), this, ID_BTN_Y_MINUS);

    m_btnZPlus.Create(_T("Z+"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(520, 95, 570, 125), this, ID_BTN_Z_PLUS);
    m_btnZMinus.Create(_T("Z-"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(580, 95, 630, 125), this, ID_BTN_Z_MINUS);

    // 移动步长选择 - 更宽的下拉框
    m_staticStepLabel.Create(_T("步长:"), WS_CHILD | WS_VISIBLE,
        CRect(660, 95, 710, 110), this, -1);
    m_comboStep.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        CRect(660, 110, 730, 155), this, ID_COMBO_STEP);
    m_comboStep.AddString(_T("0.01"));
    m_comboStep.AddString(_T("0.1"));
    m_comboStep.AddString(_T("1.0"));
    m_comboStep.AddString(_T("10.0"));
    m_comboStep.SetCurSel(2); // 默认1.0mm
}

void CSimpleControlPanel::OnPaint()
{
    CPaintDC dc(this);

    // 设置背景色为专业的深灰色
    CRect rect;
    GetClientRect(&rect);
    dc.FillSolidRect(&rect, RGB(64, 64, 64));

    // 绘制分组框
    CPen pen(PS_SOLID, 1, RGB(128, 128, 128));
    CPen* pOldPen = dc.SelectObject(&pen);

    // 设置文字颜色为白色
    dc.SetTextColor(RGB(255, 255, 255));
    dc.SetBkMode(TRANSPARENT);

    // 第一行：循环控制 - 扩展到更宽
    dc.Rectangle(15, 15, 860, 50);
    dc.TextOut(20, 10, _T("🎮 循环控制 & 单段执行"));

    // 第二行：速度调节 - 适应更长的滑块
    dc.Rectangle(15, 50, 460, 90);
    dc.TextOut(20, 45, _T("⚡ 速度调节"));

    // 第三行：手轮模拟 - 扩展到包含所有控件
    dc.Rectangle(15, 90, 740, 130);
    dc.TextOut(20, 85, _T("🔧 手轮模拟 & 手动控制"));

    dc.SelectObject(pOldPen);
}

void CSimpleControlPanel::OnBtnStart()
{
    if (m_pDoc)
    {
        m_pDoc->StartSimulation();
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnPause()
{
    if (m_pDoc)
    {
        m_pDoc->PauseSimulation();
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnStop()
{
    if (m_pDoc)
    {
        m_pDoc->StopSimulation();
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnReset()
{
    if (m_pDoc)
    {
        m_pDoc->ResetSimulation();
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnOpenFile()
{
    if (m_pDoc)
    {
        CFileDialog dlg(TRUE, _T("nc"), nullptr,
            OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
            _T("G代码文件 (*.nc;*.cnc;*.txt)|*.nc;*.cnc;*.txt|所有文件 (*.*)|*.*||"));
        if (dlg.DoModal() == IDOK)
        {
            if (m_pDoc->ParseGCodeFile(dlg.GetPathName()))
            {
                AfxMessageBox(_T("G代码文件加载成功！"));
            }
            else
            {
                AfxMessageBox(_T("G代码文件加载失败！"));
            }
        }
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnEmergencyStop()
{
    if (m_pDoc)
    {
        m_pDoc->EmergencyStop();
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnSingleStep()
{
    if (m_pDoc)
    {
        m_bSingleStepMode = !m_bSingleStepMode;
        m_pDoc->SetSingleStepMode(m_bSingleStepMode);
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnNextStep()
{
    if (m_pDoc && m_bSingleStepMode)
    {
        m_pDoc->ExecuteNextStep();
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnManualMode()
{
    if (m_pDoc)
    {
        m_bManualMode = !m_bManualMode;
        if (m_bManualMode)
            m_pDoc->SetSystemState(ST_MANUAL);
        else
            m_pDoc->SetSystemState(ST_IDLE);
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnHome()
{
    if (m_pDoc)
    {
        m_pDoc->HomeAxes();
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnXPlus()
{
    if (m_pDoc)
    {
        m_pDoc->ManualMove(m_dMoveStep, 0.0, 0.0);
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnXMinus()
{
    if (m_pDoc)
    {
        m_pDoc->ManualMove(-m_dMoveStep, 0.0, 0.0);
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnYPlus()
{
    if (m_pDoc)
    {
        m_pDoc->ManualMove(0.0, m_dMoveStep, 0.0);
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnYMinus()
{
    if (m_pDoc)
    {
        m_pDoc->ManualMove(0.0, -m_dMoveStep, 0.0);
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnZPlus()
{
    if (m_pDoc)
    {
        m_pDoc->ManualMove(0.0, 0.0, m_dMoveStep);
        UpdateControls();
    }
}

void CSimpleControlPanel::OnBtnZMinus()
{
    if (m_pDoc)
    {
        m_pDoc->ManualMove(0.0, 0.0, -m_dMoveStep);
        UpdateControls();
    }
}

void CSimpleControlPanel::OnCbnSelchangeMoveStep()
{
    // 更新移动步长
    int sel = m_comboStep.GetCurSel();
    if (sel != CB_ERR)
    {
        CString strStep;
        m_comboStep.GetLBText(sel, strStep);
        m_dMoveStep = _ttof(strStep);
    }
}

void CSimpleControlPanel::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if (!m_pDoc) return;

    if (pScrollBar == (CScrollBar*)&m_sliderFeed)
    {
        int pos = m_sliderFeed.GetPos();
        m_pDoc->SetFeedOverride(pos / 100.0);
        CString str;
        str.Format(_T("进给倍率: %d%%"), pos);
        m_staticFeedLabel.SetWindowText(str);
    }
    else if (pScrollBar == (CScrollBar*)&m_sliderSpindle)
    {
        int pos = m_sliderSpindle.GetPos();
        m_pDoc->SetSpindleOverride(pos / 100.0);
        CString str;
        str.Format(_T("主轴倍率: %d%%"), pos);
        m_staticSpindleLabel.SetWindowText(str);
    }
}

void CSimpleControlPanel::UpdateControls()
{
    if (!m_pDoc) return;

    SystemState state = m_pDoc->GetSystemState();

    // 更新按钮状态
    m_btnStart.EnableWindow(state == ST_IDLE || state == ST_PAUSED);
    m_btnPause.EnableWindow(state == ST_RUNNING);
    m_btnStop.EnableWindow(state == ST_RUNNING || state == ST_PAUSED);
    m_btnReset.EnableWindow(state != ST_RUNNING && state != ST_EMERGENCY);
    m_btnEmergencyStop.EnableWindow(state != ST_EMERGENCY);

    // 手动控制按钮
    BOOL bManualEnabled = (state == ST_MANUAL || state == ST_IDLE);
    m_btnXPlus.EnableWindow(bManualEnabled);
    m_btnXMinus.EnableWindow(bManualEnabled);
    m_btnYPlus.EnableWindow(bManualEnabled);
    m_btnYMinus.EnableWindow(bManualEnabled);
    m_btnZPlus.EnableWindow(bManualEnabled);
    m_btnZMinus.EnableWindow(bManualEnabled);
    m_btnHome.EnableWindow(state == ST_IDLE);

    // 更新按钮文本
    m_btnSingleStep.SetWindowText(m_bSingleStepMode ? _T("连续执行") : _T("单段执行"));
    m_btnManualMode.SetWindowText(m_bManualMode ? _T("自动模式") : _T("手动模式"));

    // 下一步按钮：在单段模式下且系统正在等待下一步时启用
    BOOL bNextStepEnabled = m_bSingleStepMode &&
                           (state == ST_PAUSED || state == ST_IDLE ||
                           (state == ST_RUNNING && m_pDoc && m_pDoc->IsWaitingForStep()));
    m_btnNextStep.EnableWindow(bNextStepEnabled);

    // 更新移动步长
    int sel = m_comboStep.GetCurSel();
    if (sel != CB_ERR)
    {
        CString strStep;
        m_comboStep.GetLBText(sel, strStep);
        m_dMoveStep = _ttof(strStep);
    }
}
