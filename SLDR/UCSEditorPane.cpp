#include "StdAfx.h"
#include "UCSEditorPane.h"
#include "MainFrm.h"

IMPLEMENT_DYNAMIC(CUCSEditorPane, CDockablePane)

CUCSEditorPane::CUCSEditorPane(void)
{
}


CUCSEditorPane::~CUCSEditorPane(void)
{
}

BEGIN_MESSAGE_MAP(CUCSEditorPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

int CUCSEditorPane::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_dialog.Create(IDD_DIALOG_UCSEDIT, this)) {
		MessageBox(_T("未能初始化窗口"), _T("ERROR"));
		return -1;
	}
	m_dialog.ShowWindow(SW_SHOW);

	return 0;
}

void CUCSEditorPane::OnSize(UINT nType, int cx, int cy) {
	CDockablePane::OnSize(nType, cx, cy);

	m_dialog.SetWindowPos(this, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	m_dialog.ShowWindow(SW_SHOW);
}

void CUCSEditorPane::OnDestroy() {
	CDockablePane::OnDestroy();
	m_dialog.DestroyWindow();
}

void CUCSEditorPane::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CWnd *pWnd = pMain->GetCtrlPane()->GetDialog()->GetDlgItem(IDC_MFCBUTTON_DEPTHESTIM);
	pWnd->EnableWindow(TRUE);
	CDockablePane::OnClose();
}
