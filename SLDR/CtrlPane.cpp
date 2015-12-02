#include "StdAfx.h"
#include "CtrlPane.h"
#include "SLDR.h"
#include "MainFrm.h"
class CSLDRDoc;
class CSLDRView;

IMPLEMENT_DYNAMIC(CCtrlPane, CDockablePane)

CCtrlPane::CCtrlPane(void)
{
}


CCtrlPane::~CCtrlPane(void)
{
}

BEGIN_MESSAGE_MAP(CCtrlPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CCtrlPane::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_dialog.Create(IDD_DIALOG_CTRL, this)) {
		MessageBox(_T("未能初始化窗口"), _T("ERROR"));
		return -1;
	}
	m_dialog.ShowWindow(SW_SHOW);
	
	return 0;
}

void CCtrlPane::OnSize(UINT nType, int cx, int cy) {
	CDockablePane::OnSize(nType, cx, cy);

	m_dialog.SetWindowPos(this, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	m_dialog.ShowWindow(SW_SHOW);
}

void CCtrlPane::OnDestroy() {
	CDockablePane::OnDestroy();
	m_dialog.DestroyWindow();
}