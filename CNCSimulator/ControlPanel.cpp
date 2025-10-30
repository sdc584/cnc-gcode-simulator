#include "pch.h"
#include "CNCSimulator.h"
#include "CNCSimulatorDoc.h"
#include "ControlPanel.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CControlPanel 对话框

IMPLEMENT_DYNAMIC(CControlPanel, CDialogEx)

CControlPanel::CControlPanel(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CONTROL_PANEL, pParent)
{
	m_pDoc = nullptr;
	m_dMoveStep = 1.0;          // 默认1mm步长
	m_bContinuousMove = FALSE;  // 默认点动模式
	m_bHandwheelMode = FALSE;   // 默认非手轮模式
	m_dSimSpeed = 1.0;          // 默认仿真速度100%
}

CControlPanel::~CControlPanel()
{
}

void CControlPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	// 只关联简单的按钮控件
	DDX_Control(pDX, IDC_BTN_START, m_btnStart);
	DDX_Control(pDX, IDC_BTN_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BTN_RESET, m_btnReset);
	DDX_Control(pDX, IDC_BTN_OPEN_FILE, m_btnOpenFile);
	DDX_Control(pDX, IDC_BTN_EMERGENCY_STOP, m_btnEmergencyStop);
}

BEGIN_MESSAGE_MAP(CControlPanel, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_START, &CControlPanel::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CControlPanel::OnBnClickedPause)
	ON_BN_CLICKED(IDC_BTN_STOP, &CControlPanel::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BTN_RESET, &CControlPanel::OnBnClickedReset)
	ON_BN_CLICKED(IDC_BTN_OPEN_FILE, &CControlPanel::OnBnClickedOpenFile)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_X_PLUS, &CControlPanel::OnBnClickedXPlus)
	ON_BN_CLICKED(IDC_BTN_X_MINUS, &CControlPanel::OnBnClickedXMinus)
	ON_BN_CLICKED(IDC_BTN_Y_PLUS, &CControlPanel::OnBnClickedYPlus)
	ON_BN_CLICKED(IDC_BTN_Y_MINUS, &CControlPanel::OnBnClickedYMinus)
	ON_BN_CLICKED(IDC_BTN_Z_PLUS, &CControlPanel::OnBnClickedZPlus)
	ON_BN_CLICKED(IDC_BTN_Z_MINUS, &CControlPanel::OnBnClickedZMinus)
	ON_BN_CLICKED(IDC_BTN_EMERGENCY_STOP, &CControlPanel::OnBnClickedEmergencyStop)
	ON_BN_CLICKED(IDC_BTN_HOME, &CControlPanel::OnBnClickedHome)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MODE, &CControlPanel::OnBnClickedManualMode)
	ON_BN_CLICKED(IDC_BTN_SINGLE_STEP, &CControlPanel::OnBnClickedSingleStep)
	ON_BN_CLICKED(IDC_BTN_NEXT_STEP, &CControlPanel::OnBnClickedNextStep)
	ON_BN_CLICKED(IDC_BTN_CLEAR_ALARM, &CControlPanel::OnBnClickedClearAlarm)
	ON_BN_CLICKED(IDC_CHECK_CONTINUOUS, &CControlPanel::OnBnClickedContinuous)
	ON_BN_CLICKED(IDC_CHECK_HANDWHEEL, &CControlPanel::OnBnClickedHandwheel)
	ON_CBN_SELCHANGE(IDC_COMBO_MOVE_STEP, &CControlPanel::OnCbnSelchangeMoveStep)
END_MESSAGE_MAP()

// CControlPanel 消息处理程序

BOOL CControlPanel::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置专业CNC界面风格
	SetBackgroundColor(RGB(80, 80, 80));  // 深灰色背景

	// 初始化滑块控件
	m_sliderFeedOverride.SetRange(10, 200);     // 10% - 200%
	m_sliderFeedOverride.SetPos(100);           // 默认100%

	m_sliderSpindleOverride.SetRange(10, 200);  // 10% - 200%
	m_sliderSpindleOverride.SetPos(100);        // 默认100%

	// 初始化仿真速度滑块
	m_sliderSimSpeed.SetRange(10, 500);         // 10% - 500%
	m_sliderSimSpeed.SetPos(100);               // 默认100%

	// 初始化移动步长下拉框
	m_comboMoveStep.AddString(_T("0.01"));
	m_comboMoveStep.AddString(_T("0.1"));
	m_comboMoveStep.AddString(_T("1.0"));
	m_comboMoveStep.AddString(_T("10.0"));
	m_comboMoveStep.SetCurSel(2);               // 默认选择1.0mm

	// 初始化复选框
	m_checkContinuous.SetCheck(BST_UNCHECKED);  // 默认点动模式
	m_checkHandwheel.SetCheck(BST_UNCHECKED);   // 默认非手轮模式

	// 初始化进度条
	m_progressBar.SetRange(0, 100);
	m_progressBar.SetPos(0);

	// 设置按钮样式为专业CNC风格
	SetButtonStyle(&m_btnStart, RGB(0, 255, 0));      // 绿色启动按钮
	SetButtonStyle(&m_btnPause, RGB(255, 255, 0));    // 黄色暂停按钮
	SetButtonStyle(&m_btnStop, RGB(255, 0, 0));       // 红色停止按钮
	SetButtonStyle(&m_btnReset, RGB(0, 150, 255));    // 蓝色复位按钮
	SetButtonStyle(&m_btnEmergencyStop, RGB(255, 0, 0)); // 红色急停按钮

	UpdateControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CControlPanel::SetButtonStyle(CButton* pButton, COLORREF color)
{
	// 简单的按钮样式设置 - 在实际项目中可以使用更复杂的自绘按钮
	if (pButton && pButton->GetSafeHwnd())
	{
		// 设置按钮为所有者绘制样式
		pButton->ModifyStyle(0, BS_OWNERDRAW);
	}
}

void CControlPanel::UpdateMoveStep()
{
	// 从下拉框获取当前选择的移动步长
	int sel = m_comboMoveStep.GetCurSel();
	if (sel != CB_ERR)
	{
		CString strStep;
		m_comboMoveStep.GetLBText(sel, strStep);
		m_dMoveStep = _ttof(strStep);
	}
	else
	{
		m_dMoveStep = 1.0;  // 默认1mm步长
	}
}

void CControlPanel::UpdateControls()
{
	if (!m_pDoc)
		return;

	// 更新按钮状态
	SystemState state = m_pDoc->GetSystemState();

	// 仿真控制按钮
	m_btnStart.EnableWindow(state == ST_IDLE || state == ST_PAUSED);
	m_btnPause.EnableWindow(state == ST_RUNNING);
	m_btnStop.EnableWindow(state == ST_RUNNING || state == ST_PAUSED);
	m_btnReset.EnableWindow(state != ST_RUNNING && state != ST_EMERGENCY);

	// 系统控制按钮
	m_btnEmergencyStop.EnableWindow(state != ST_EMERGENCY);

	// 更新倍率显示
	CString str;
	str.Format(_T("进给倍率 F%%: %d%%"), m_sliderFeedOverride.GetPos());
	m_staticFeedOverride.SetWindowText(str);

	str.Format(_T("主轴倍率 S%%: %d%%"), m_sliderSpindleOverride.GetPos());
	m_staticSpindleOverride.SetWindowText(str);

	str.Format(_T("仿真速度: %d%%"), m_sliderSimSpeed.GetPos());
	m_staticSimSpeed.SetWindowText(str);

	// 更新系统状态显示
	CString strState;
	switch (state)
	{
	case ST_IDLE: strState = _T("空闲"); break;
	case ST_RUNNING: strState = _T("运行中"); break;
	case ST_PAUSED: strState = _T("暂停"); break;
	case ST_MANUAL: strState = _T("手动模式"); break;
	case ST_EMERGENCY: strState = _T("急停"); break;
	default: strState = _T("未知"); break;
	}
	m_staticSystemState.SetWindowText(strState);

	// 更新坐标显示
	double x, y, z;
	m_pDoc->GetCurrentPosition(x, y, z);
	str.Format(_T("X: %.3f"), x);
	m_staticPosX.SetWindowText(str);
	str.Format(_T("Y: %.3f"), y);
	m_staticPosY.SetWindowText(str);
	str.Format(_T("Z: %.3f"), z);
	m_staticPosZ.SetWindowText(str);

	// 更新运行时间
	DWORD runTime = m_pDoc->GetRunTime();
	str.Format(_T("%02d:%02d:%02d"), runTime / 3600, (runTime % 3600) / 60, runTime % 60);
	m_staticRunTime.SetWindowText(str);

	// 更新进度
	double progress = m_pDoc->GetProgress();
	m_progressBar.SetPos((int)progress);
	str.Format(_T("%.1f%%"), progress);
	m_staticProgress.SetWindowText(str);

	// 更新模态信息
	CString strModal = m_pDoc->GetModalInfo();
	m_staticModalInfo.SetWindowText(strModal);

	// 更新当前G代码
	CString strCurrentGCode = m_pDoc->GetCurrentGCodeLine();
	m_staticCurrentGCode.SetWindowText(strCurrentGCode);

	// 更新报警信息
	if (m_pDoc->HasAlarm())
	{
		CString strAlarm = m_pDoc->GetLastError();
		m_staticAlarmMsg.SetWindowText(strAlarm);
	}
	else
	{
		m_staticAlarmMsg.SetWindowText(_T(""));
	}

	// 更新手动模式按钮文本
	if (state == ST_MANUAL)
	{
		m_btnManualMode.SetWindowText(_T("退出手动"));
	}
	else
	{
		m_btnManualMode.SetWindowText(_T("手动模式"));
	}

	// 更新单段执行按钮
	if (m_pDoc->IsSingleStepMode())
	{
		m_btnSingleStep.SetWindowText(_T("连续执行"));
		// 下一步按钮：在单段模式下且系统正在等待下一步时启用
		BOOL bNextStepEnabled = (state == ST_PAUSED || state == ST_IDLE ||
		                        (state == ST_RUNNING && m_pDoc->IsWaitingForStep()));
		m_btnNextStep.EnableWindow(bNextStepEnabled);
	}
	else
	{
		m_btnSingleStep.SetWindowText(_T("单段执行"));
		m_btnNextStep.EnableWindow(FALSE);
	}

	// 更新急停按钮状态
	if (state == ST_EMERGENCY)
	{
		m_btnEmergencyStop.SetWindowText(_T("复位急停"));
	}
	else
	{
		m_btnEmergencyStop.SetWindowText(_T("急停"));
	}
}

void CControlPanel::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		m_pDoc->StartSimulation();
		UpdateControls();
	}
	AfxMessageBox(_T("Start button clicked!"));
}

void CControlPanel::OnBnClickedPause()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		m_pDoc->PauseSimulation();
		UpdateControls();
	}
}

void CControlPanel::OnBnClickedStop()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		m_pDoc->StopSimulation();
		UpdateControls();
	}
}

void CControlPanel::OnBnClickedReset()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		// 如果有报警，先清除错误
		if (m_pDoc->HasAlarm())
		{
			m_pDoc->ClearError();
		}

		m_pDoc->ResetSimulation();
		UpdateControls();
	}
}

void CControlPanel::OnBnClickedOpenFile()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, _T("nc"), nullptr, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("G代码文件 (*.nc;*.cnc;*.txt)|*.nc;*.cnc;*.txt|所有文件 (*.*)|*.*||"));
	
	if (dlg.DoModal() == IDOK && m_pDoc)
	{
		if (m_pDoc->ParseGCodeFile(dlg.GetPathName()))
		{
			AfxMessageBox(_T("G代码文件加载成功！"));
			UpdateControls();
		}
		else
		{
			AfxMessageBox(_T("G代码文件加载失败！"));
		}
	}
}

void CControlPanel::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_pDoc)
	{
		if (pScrollBar == (CScrollBar*)&m_sliderFeedOverride)
		{
			double feedOverride = m_sliderFeedOverride.GetPos() / 100.0;
			m_pDoc->SetFeedOverride(feedOverride);
		}
		else if (pScrollBar == (CScrollBar*)&m_sliderSpindleOverride)
		{
			double spindleOverride = m_sliderSpindleOverride.GetPos() / 100.0;
			m_pDoc->SetSpindleOverride(spindleOverride);
		}
		else if (pScrollBar == (CScrollBar*)&m_sliderSimSpeed)
		{
			m_dSimSpeed = m_sliderSimSpeed.GetPos() / 100.0;
			// 设置仿真速度倍率
			m_pDoc->SetSimulationSpeed(m_dSimSpeed);
		}

		UpdateControls();
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CControlPanel::OnBnClickedXPlus()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		UpdateMoveStep();
		m_pDoc->ManualMove(m_dMoveStep, 0.0, 0.0);
	}
}

void CControlPanel::OnBnClickedXMinus()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		UpdateMoveStep();
		m_pDoc->ManualMove(-m_dMoveStep, 0.0, 0.0);
	}
}

void CControlPanel::OnBnClickedYPlus()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		UpdateMoveStep();
		m_pDoc->ManualMove(0.0, m_dMoveStep, 0.0);
	}
}

void CControlPanel::OnBnClickedYMinus()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		UpdateMoveStep();
		m_pDoc->ManualMove(0.0, -m_dMoveStep, 0.0);
	}
}

void CControlPanel::OnBnClickedZPlus()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		UpdateMoveStep();
		m_pDoc->ManualMove(0.0, 0.0, m_dMoveStep);
	}
}

void CControlPanel::OnBnClickedZMinus()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		UpdateMoveStep();
		m_pDoc->ManualMove(0.0, 0.0, -m_dMoveStep);
	}
}

void CControlPanel::OnBnClickedEmergencyStop()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		m_pDoc->EmergencyStop();
		UpdateControls();
	}
}

void CControlPanel::OnBnClickedHome()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		m_pDoc->HomeAxes();
		UpdateControls();
	}
}

void CControlPanel::OnBnClickedManualMode()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		SystemState currentState = m_pDoc->GetSystemState();
		if (currentState == ST_MANUAL)
		{
			m_pDoc->SetSystemState(ST_IDLE);
		}
		else if (currentState == ST_IDLE)
		{
			m_pDoc->SetSystemState(ST_MANUAL);
		}
		UpdateControls();
	}
}

void CControlPanel::OnBnClickedSingleStep()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		// 切换单段执行模式
		BOOL bCurrentMode = m_pDoc->IsSingleStepMode();
		m_pDoc->SetSingleStepMode(!bCurrentMode);
		UpdateControls();
	}
}

void CControlPanel::OnBnClickedNextStep()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_pDoc)
	{
		m_pDoc->ExecuteNextStep();
		UpdateControls();
	}
}

void CControlPanel::OnBnClickedClearAlarm()
{
	// 清除报警
	if (m_pDoc)
	{
		m_pDoc->ClearError();
		UpdateControls();
	}
}

void CControlPanel::OnBnClickedContinuous()
{
	// 切换连续移动模式
	m_bContinuousMove = m_checkContinuous.GetCheck() == BST_CHECKED;
}

void CControlPanel::OnBnClickedHandwheel()
{
	// 切换手轮模式
	m_bHandwheelMode = m_checkHandwheel.GetCheck() == BST_CHECKED;

	// 手轮模式下禁用连续移动
	if (m_bHandwheelMode)
	{
		m_checkContinuous.SetCheck(BST_UNCHECKED);
		m_bContinuousMove = FALSE;
		m_checkContinuous.EnableWindow(FALSE);
	}
	else
	{
		m_checkContinuous.EnableWindow(TRUE);
	}
}

void CControlPanel::OnCbnSelchangeMoveStep()
{
	// 更新移动步长
	UpdateMoveStep();
}
