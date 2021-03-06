// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "SLDR.h"

#include "MainFrm.h"
#include "CLog.h"
#include "CCopierRegister.h"
#include "SLDRDoc.h"
#include "SLDRView.h"
#include "CP_FlowComplex.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_COMMAND(ID_FILE_PRINT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnUpdateFilePrintPreview)
	ON_WM_SHOWWINDOW()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_BUTTON_SAVECNW, &CMainFrame::OnButtonSavecnw)
	ON_MESSAGE(WM_RESULT,&CMainFrame::ShowResult)
	ON_MESSAGE(WM_RESULT_POLYLINE,&CMainFrame::ShowPolyline)
	ON_MESSAGE(WM_RESULT_FLOWCOMPLEX,&CMainFrame::ShowFlowComplex)
	ON_MESSAGE(WM_RESULT_MERGE,&CMainFrame::ShowMerge)
	ON_MESSAGE(WM_RESULT_CYCLES,&CMainFrame::ShowCycles)
	ON_MESSAGE(WM_RESULT_PRUNING,&CMainFrame::ShowPruning)
	ON_MESSAGE(WM_RESULT_REFRESH,&CMainFrame::ResultRefresh)
END_MESSAGE_MAP()

// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	// TODO: 在此添加成员初始化代码
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLACK);

	CLog::initConsole();
}

CMainFrame::~CMainFrame()
{
	CLog::freeConsole();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// _CrtSetBreakAlloc(29693);
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;
	// 基于持久值设置视觉管理器和样式
	OnApplicationLook(theApp.m_nAppLook);

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}

	CString strTitlePane1;
	CString strTitlePane2;
	bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
	ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, strTitlePane1, TRUE), strTitlePane1);
	m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, strTitlePane2, TRUE), strTitlePane2);
	//材质
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_SPECULAR));
	pSlider->SetPos(25);
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_Diffuse));
	pSlider->SetPos(80);
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_SHININESS));
	pSlider->SetPos(45);
	//光源0
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_AMBIENT0));
	pSlider->SetPos(25);
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_DIFFUSE0));
	pSlider->SetPos(70);
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_SPECULAR0));
	pSlider->SetPos(23);
	//光源1
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_AMBIENT1));
	pSlider->SetPos(0);
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_DIFFUSE1));
	pSlider->SetPos(21);
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_SPECULAR1));
	pSlider->SetPos(70);
	//光源2
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_CUTOFF2));
	pSlider->SetPos(10);
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_SPECULAR2));
	pSlider->SetPos(40);
	pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_SLIDER_EXPONENT2));
	pSlider->SetPos(30);
	// 启用 Visual Studio 2005 样式停靠窗口行为
	CDockingManager::SetDockingMode(DT_SMART);
	// 启用 Visual Studio 2005 样式停靠窗口自动隐藏行为
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Register copier
	func_reg::RegisterCopier();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(TRUE);
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
		m_wndRibbonBar.SetWindows7Look(FALSE);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnFilePrint()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_PRINT);
	}
}

void CMainFrame::OnFilePrintPreview()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);  // 强制关闭“打印预览”模式
	}
}

void CMainFrame::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsPrintPreview());
}


void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus)
{
//	m_ctrlPane.ShowPane(TRUE,FALSE,TRUE);
	CFrameWndEx::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
}

int CMainFrame::ShowUCSEditor() {
	m_ucsEditor.ShowPane(TRUE, TRUE, TRUE);
	
	m_ucsEditor.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_ucsEditor);
	return 1;
}

void CMainFrame::SetCursor(CursorType type) {
	m_cursorType = type;
}

CursorType CMainFrame::GetCursor() {
	return m_cursorType;
}


BOOL CMainFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_cursorType == NORMAL)
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	else if (m_cursorType == SEL)
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
	else if (m_cursorType == FOUND)
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
	else
		return CFrameWndEx::OnSetCursor(pWnd, nHitTest, message);
	return TRUE;
}


void CMainFrame::OnButtonSavecnw()
{
	// TODO: 在此添加命令处理程序代码
	CSLDRDoc* pDoc = dynamic_cast<CSLDRDoc*>(GetActiveDocument());
	pDoc->OutputCurveNetwork();
}

class CSLDRDoc;
LRESULT  CMainFrame::ShowResult(WPARAM wParam,LPARAM lParam)
{
	CSLDRView * pView = (CSLDRView *)(GetActiveView());
	pView->fcEnable=true;
	pView->ifcEnable=true;
	pView->showPruningEnable=true;//select patch
	pView->showPreprocessEnable=true;
	pView->Invalidate();
	return 0;
}

LRESULT  CMainFrame::ShowPolyline(WPARAM wParam,LPARAM lParam)
{
	CSLDRView * pView = (CSLDRView *)(GetActiveView());
	pView->showFCEnable=true;
	pView->showInputP=true;
	pView->Invalidate();
	return 0;
}

LRESULT  CMainFrame::ShowFlowComplex(WPARAM wParam,LPARAM lParam)
{
	CSLDRView * pView = (CSLDRView *)(GetActiveView());
	pView->showMergeEnable=true;
	pView->showInputP=false;
	pView->showFC=true;
	pView->Invalidate();
	return 0;
}

LRESULT  CMainFrame::ShowMerge(WPARAM wParam,LPARAM lParam)
{
	CSLDRView * pView = (CSLDRView *)(GetActiveView());
	pView->showCyclesEnable=true;
	pView->showFC=false;
	pView->showTop=true;
	pView->Invalidate();
	return 0;
}

LRESULT  CMainFrame::ShowCycles(WPARAM wParam,LPARAM lParam)
{
	CSLDRView * pView = (CSLDRView *)(GetActiveView());
	pView->showPruningEnable=true;
	pView->showTop=false;
	pView->showCycles=true;
	pView->Invalidate();
	return 0;
}

LRESULT  CMainFrame::ShowPruning(WPARAM wParam,LPARAM lParam)
{
	CSLDRView * pView = (CSLDRView *)(GetActiveView());
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	pView->showCycles=false;

	pView->showPolyEnable=true;
	pView->showFCEnable=true;
	pView->showMergeEnable=true;
	pView->showCyclesEnable=true;
	pView->showPruningEnable=true;
	pView->fcEnable=true;
	pView->ifcEnable=true;
	pView->showPreprocessEnable=true;
	pView->Invalidate();
	return 0;
}

LRESULT  CMainFrame::ResultRefresh(WPARAM wParam,LPARAM  lParam)
{
	CSLDRView * pView = (CSLDRView *)(GetActiveView());
	pView->Invalidate();
	return 0;
}


