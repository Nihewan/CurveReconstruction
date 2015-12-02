#include "StdAfx.h"
#include "SLDR.h"
#include "MainFrm.h"
#include "FCCRCtrlPane.h"
class CSLDRDoc;
class CSLDRView;

IMPLEMENT_DYNAMIC(FCCRCtrlPane, CDockablePane)

FCCRCtrlPane::FCCRCtrlPane(void)
{
}


FCCRCtrlPane::~FCCRCtrlPane(void)
{
}

BEGIN_MESSAGE_MAP(FCCRCtrlPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int FCCRCtrlPane::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_dialog.Create(IDD_DIALOG_CTRL_FCCR, this)) {
		MessageBox(_T("未能初始化窗口"), _T("ERROR"));
		return -1;
	}
	m_dialog.ShowWindow(SW_SHOW);
	
	return 0;
}

void FCCRCtrlPane::OnSize(UINT nType, int cx, int cy) {
	CDockablePane::OnSize(nType, cx, cy);

	m_dialog.SetWindowPos(this, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	m_dialog.ShowWindow(SW_SHOW);
}

void FCCRCtrlPane::OnDestroy() {
	CDockablePane::OnDestroy();
	m_dialog.DestroyWindow();
}
