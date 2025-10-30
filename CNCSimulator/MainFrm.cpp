#include "pch.h"
#include "framework.h"
#include "CNCSimulator.h"
#include "CNCSimulatorDoc.h"
#include "GraphicsView.h"
#include "CodeEditView.h"
#include "InfoView.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	// G代码和仿真命令
	ON_COMMAND(ID_FILE_OPEN_GCODE, &CMainFrame::OnFileOpenGcode)
	ON_COMMAND(ID_SIMULATION_START, &CMainFrame::OnSimulationStart)
	ON_COMMAND(ID_SIMULATION_PAUSE, &CMainFrame::OnSimulationPause)
	ON_COMMAND(ID_SIMULATION_STOP, &CMainFrame::OnSimulationStop)
	ON_COMMAND(ID_SIMULATION_RESET, &CMainFrame::OnSimulationReset)
	// 工具栏显示控制
	ON_COMMAND(ID_VIEW_TOOLBAR, &CMainFrame::OnViewToolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, &CMainFrame::OnUpdateViewToolbar)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame 构造/析构

CMainFrame::CMainFrame() noexcept
{
	// TODO: 在此添加成员初始化代码
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
	m_pControlPanel = nullptr;
	m_pSimpleControlPanel = nullptr;
}

CMainFrame::~CMainFrame()
{
	if (m_pControlPanel)
	{
		delete m_pControlPanel;
		m_pControlPanel = nullptr;
	}
	if (m_pSimpleControlPanel)
	{
		delete m_pSimpleControlPanel;
		m_pSimpleControlPanel = nullptr;
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // 其他可用样式...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // 设置为 FALSE 会将关闭按钮放置在选项卡区域的右侧
	mdiTabParams.m_bTabIcons = FALSE;    // 设置为 TRUE 将在 MDI 选项卡上启用文档图标
	mdiTabParams.m_bAutoColor = TRUE;    // 设置为 FALSE 将禁用 MDI 选项卡的自动着色
	mdiTabParams.m_bDocumentMenu = TRUE; // 在选项卡区域的右边缘启用文档菜单
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("未能创建菜单栏\n");
		return -1;      // 未能创建
	}

	// 设置菜单栏样式，确保它始终可见
	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_FIXED | CBRS_TOOLTIPS | CBRS_FLYBY);

	// 防止菜单栏在激活时获得焦点
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	// 确保菜单栏不能被隐藏 - 注释掉不支持的方法
	// m_wndMenuBar.SetBarStyle(m_wndMenuBar.GetBarStyle() & ~CBRS_HIDE_INPLACE);

	// 创建工具栏（不加载位图资源）
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC))
	{
		TRACE0("未能创建工具栏\n");
		AfxMessageBox(_T("工具栏创建失败！"));
		return -1;      // 未能创建
	}

	// 手动添加工具栏按钮（使用系统图标）
	CreateToolbarButtons();

	TRACE0("工具栏创建成功\n");

	m_wndToolBar.SetWindowText(_T("CNC Simulator"));
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, _T("Customize..."));

	// 获取工具栏按钮数量进行调试
	int buttonCount = m_wndToolBar.GetCount();
	TRACE1("工具栏按钮数量: %d\n", buttonCount);

	// 允许用户定义的工具栏操作:
	InitUserToolbars(nullptr, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// 菜单栏固定在标题栏下方，不允许移动
	m_wndMenuBar.EnableDocking(0); // 禁用停靠，固定位置
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);

	// 菜单栏固定在顶部，不可移动
	DockPane(&m_wndMenuBar);
	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() & ~CBRS_GRIPPER); // 移除抓手

	// 工具栏停靠在菜单栏下方
	DockPane(&m_wndToolBar);

	// 强制显示菜单栏和工具栏
	m_wndMenuBar.ShowPane(TRUE, FALSE, TRUE);
	m_wndToolBar.ShowPane(TRUE, FALSE, TRUE);

	// 确保工具栏可见并正确停靠
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// 强制工具栏可见
	m_wndToolBar.ShowWindow(SW_SHOW);
	m_wndToolBar.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);

	TRACE0("工具栏显示设置完成\n");

	// 启用 Visual Studio 2005 样式停靠窗口行为
	CDockingManager::SetDockingMode(DT_SMART);
	// 启用 Visual Studio 2005 样式停靠窗口自动隐藏行为
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// 基于持久值设置视觉管理器和样式
	OnApplicationLook(theApp.m_nAppLook);

	// 创建完整功能控制面板（基于普通窗口，避免对话框复杂性）
	m_pSimpleControlPanel = new CSimpleControlPanel();
	if (m_pSimpleControlPanel)
	{
		CRect rect;
		GetClientRect(&rect);
		CRect panelRect(0, rect.bottom - 140, rect.Width(), rect.bottom);

		if (!m_pSimpleControlPanel->Create(this, panelRect))
		{
			delete m_pSimpleControlPanel;
			m_pSimpleControlPanel = nullptr;
			TRACE0("Failed to create simple control panel\n");
		}
	}

	// 启动仿真定时器 (50ms间隔，20FPS)
	SetTimer(SIMULATION_TIMER_ID, 50, nullptr);

	// 暂时注释掉工具栏按钮，使用菜单代替
	// 添加工具栏按钮 - 使用正确的构造函数参数
	//CMFCToolBarButton btnOpen(ID_FILE_OPEN_GCODE, 0, _T("打开G代码"));
	//CMFCToolBarButton btnStart(ID_SIMULATION_START, 1, _T("开始仿真"));
	//CMFCToolBarButton btnPause(ID_SIMULATION_PAUSE, 2, _T("暂停仿真"));
	//CMFCToolBarButton btnStop(ID_SIMULATION_STOP, 3, _T("停止仿真"));
	//CMFCToolBarButton btnReset(ID_SIMULATION_RESET, 4, _T("复位"));
	//
	//m_wndToolBar.InsertButton(btnOpen);
	//m_wndToolBar.InsertButton(btnStart);
	//m_wndToolBar.InsertButton(btnPause);
	//m_wndToolBar.InsertButton(btnStop);
	//m_wndToolBar.InsertButton(btnReset);

	// 强制显示菜单栏和工具栏 - 使用正确的方法
	m_wndMenuBar.ShowPane(TRUE, FALSE, TRUE);
	m_wndToolBar.ShowPane(TRUE, FALSE, TRUE);

	// 重新计算布局
	RecalcLayout();

	// 延迟显示工具栏
	SetTimer(1, 100, NULL);  // 100ms后显示工具栏

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return TRUE;
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// 基类将执行真正的工作

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	// 确保菜单栏可见 - 使用正确的方法
	m_wndMenuBar.ShowPane(TRUE, FALSE, TRUE);

	// 为所有用户工具栏启用自定义按钮
	CString strCustomize = _T("Customize...");

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != nullptr)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	// 暂时注释掉基类调用，避免断言失败
	// CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG

// CMainFrame 消息处理程序

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* 扫描菜单 */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, _T("Customize..."));
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == SIMULATION_TIMER_ID)
	{
		UpdateSimulation();
	}
	else if (nIDEvent == 1) // 工具栏显示定时器
	{
		KillTimer(1);
		// 强制显示工具栏
		m_wndToolBar.ShowPane(TRUE, FALSE, TRUE);
		m_wndToolBar.ShowWindow(SW_SHOW);
		RecalcLayout();
		TRACE0("定时器强制显示工具栏\n");
	}
	else
	{
		CMDIFrameWndEx::OnTimer(nIDEvent);
	}
}

void CMainFrame::OnDestroy()
{
	KillTimer(SIMULATION_TIMER_ID);
	CMDIFrameWndEx::OnDestroy();
}

void CMainFrame::UpdateSimulation()
{
	// 获取活动文档
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild)
	{
		CDocument* pDoc = pActiveChild->GetActiveDocument();
		if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)))
		{
			CCNCSimulatorDoc* pCNCDoc = static_cast<CCNCSimulatorDoc*>(pDoc);
			pCNCDoc->UpdateSimulation();

			// 更新控制面板
			if (m_pControlPanel)
			{
				m_pControlPanel->SetDocument(pCNCDoc);
				m_pControlPanel->UpdateControls();
			}
			if (m_pSimpleControlPanel)
			{
				m_pSimpleControlPanel->SetDocument(pCNCDoc);
				m_pSimpleControlPanel->UpdateControls();
			}
		}
	}
}

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
END_MESSAGE_MAP()

// CChildFrame 构造/析构

CChildFrame::CChildFrame() noexcept
{
	// TODO: 在此添加成员初始化代码
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// 创建专业CNC界面布局
	// 主分割窗口：左侧图形显示区域，右侧信息和控制区域
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to create main splitter\n");
		return FALSE;
	}

	// 左侧：图形显示视图 (占70%宽度)
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CGraphicsView), CSize(700, 500), pContext))
	{
		TRACE0("Failed to create graphics view\n");
		return FALSE;
	}

	// 右侧分割窗口：上下分割
	if (!m_wndSplitter2.CreateStatic(&m_wndSplitter, 2, 1, WS_CHILD | WS_VISIBLE, m_wndSplitter.IdFromRowCol(0, 1)))
	{
		TRACE0("Failed to create right splitter\n");
		return FALSE;
	}

	// 右上：信息显示视图
	if (!m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CInfoView), CSize(300, 250), pContext))
	{
		TRACE0("Failed to create info view\n");
		return FALSE;
	}

	// 右下：代码编辑视图
	if (!m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CCodeEditView), CSize(300, 250), pContext))
	{
		TRACE0("Failed to create code edit view\n");
		return FALSE;
	}

	// 设置分割窗口的初始大小
	m_wndSplitter.SetColumnInfo(0, 700, 50);  // 左侧最小50像素
	m_wndSplitter.SetColumnInfo(1, 300, 50);  // 右侧最小50像素

	m_wndSplitter2.SetRowInfo(0, 250, 50);    // 右上最小50像素
	m_wndSplitter2.SetRowInfo(1, 250, 50);    // 右下最小50像素

	return TRUE;
}

// CChildFrame 诊断

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	// 暂时注释掉基类调用，避免断言失败
	// CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame 消息处理程序

// G代码和仿真命令处理

void CMainFrame::OnFileOpenGcode()
{
	// 获取活动文档
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild)
	{
		CDocument* pDoc = pActiveChild->GetActiveDocument();
		if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)))
		{
			CCNCSimulatorDoc* pCNCDoc = static_cast<CCNCSimulatorDoc*>(pDoc);
			pCNCDoc->OnFileOpenGcode();
		}
	}
}

void CMainFrame::OnSimulationStart()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild)
	{
		CDocument* pDoc = pActiveChild->GetActiveDocument();
		if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)))
		{
			CCNCSimulatorDoc* pCNCDoc = static_cast<CCNCSimulatorDoc*>(pDoc);
			pCNCDoc->OnSimulationStart();
		}
	}
}

void CMainFrame::OnSimulationPause()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild)
	{
		CDocument* pDoc = pActiveChild->GetActiveDocument();
		if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)))
		{
			CCNCSimulatorDoc* pCNCDoc = static_cast<CCNCSimulatorDoc*>(pDoc);
			pCNCDoc->OnSimulationPause();
		}
	}
}

void CMainFrame::OnSimulationStop()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild)
	{
		CDocument* pDoc = pActiveChild->GetActiveDocument();
		if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)))
		{
			CCNCSimulatorDoc* pCNCDoc = static_cast<CCNCSimulatorDoc*>(pDoc);
			pCNCDoc->OnSimulationStop();
		}
	}
}

void CMainFrame::OnSimulationReset()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild)
	{
		CDocument* pDoc = pActiveChild->GetActiveDocument();
		if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CCNCSimulatorDoc)))
		{
			CCNCSimulatorDoc* pCNCDoc = static_cast<CCNCSimulatorDoc*>(pDoc);
			pCNCDoc->OnSimulationReset();
		}
	}
}

void CMainFrame::OnViewToolbar()
{
	BOOL bVisible = m_wndToolBar.IsVisible();
	m_wndToolBar.ShowPane(!bVisible, FALSE, TRUE);
	RecalcLayout();
	TRACE1("工具栏显示状态切换: %s\n", !bVisible ? "显示" : "隐藏");
}

void CMainFrame::OnUpdateViewToolbar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBar.IsVisible());
}

void CMainFrame::CreateToolbarButtons()
{
	// 简化方案：使用文本按钮，不依赖图标资源

	// 清空现有按钮
	while (m_wndToolBar.GetCount() > 0)
	{
		m_wndToolBar.RemoveButton(0);
	}

	// 添加文本按钮
	m_wndToolBar.InsertButton(CMFCToolBarButton(ID_FILE_NEW, -1, _T("新建")));
	m_wndToolBar.InsertButton(CMFCToolBarButton(ID_FILE_OPEN, -1, _T("打开")));
	m_wndToolBar.InsertButton(CMFCToolBarButton(ID_FILE_OPEN_GCODE, -1, _T("G代码")));
	m_wndToolBar.InsertButton(CMFCToolBarButton(ID_FILE_SAVE, -1, _T("保存")));

	// 添加分隔符
	m_wndToolBar.InsertSeparator();

	// 仿真控制按钮
	m_wndToolBar.InsertButton(CMFCToolBarButton(ID_SIMULATION_START, -1, _T("▶开始")));
	m_wndToolBar.InsertButton(CMFCToolBarButton(ID_SIMULATION_PAUSE, -1, _T("⏸暂停")));
	m_wndToolBar.InsertButton(CMFCToolBarButton(ID_SIMULATION_STOP, -1, _T("⏹停止")));
	m_wndToolBar.InsertButton(CMFCToolBarButton(ID_SIMULATION_RESET, -1, _T("🔄复位")));

	// 调整工具栏大小
	m_wndToolBar.AdjustLayout();

	TRACE0("工具栏按钮创建完成\n");
}
