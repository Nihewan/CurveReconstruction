// UCSEditor.cpp : 实现文件
//

#include "stdafx.h"
#include "SLDR.h"
#include "UCSEditor.h"
#include "afxdialogex.h"
#include "CLog.h"
#include "MainFrm.h"
#include "cubiccorner.h"
#include "topo_geometricoper.h"


// UCSEditor 对话框

IMPLEMENT_DYNAMIC(CUCSEditor, CDialogEx)

CUCSEditor::CUCSEditor(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUCSEditor::IDD, pParent)
{
	m_alignAxisIndex = -1;
}

CUCSEditor::~CUCSEditor()
{
}

void CUCSEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUCSEditor, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_MFCBUTTON_ALIGNX, &CUCSEditor::OnBnClickedMfcbuttonAlignx)
	ON_BN_CLICKED(IDC_MFCBUTTON_ALIGNZ, &CUCSEditor::OnBnClickedMfcbuttonAlignz)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_MFCBUTTON_ALIGNOK, &CUCSEditor::OnBnClickedMfcbuttonAlignok)
END_MESSAGE_MAP()


// UCSEditor 消息处理程序


void CUCSEditor::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CLog::log("UCS editor closed.");

	CDialogEx::OnClose();
}


int CUCSEditor::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	

	return 0;
}


HBRUSH CUCSEditor::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}


void CUCSEditor::OnBnClickedCommand1()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CUCSEditor::OnBnClickedMfcbuttonAlignx()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_MFCBUTTON_ALIGNX)->EnableWindow(FALSE);
	GetDlgItem(IDC_MFCBUTTON_ALIGNZ)->EnableWindow(FALSE);
	GetDlgItem(IDC_MFCBUTTON_ALIGNOK)->EnableWindow(FALSE);
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	pMain->SetCursor(SEL);
	m_alignAxisIndex = 0;
}

void CUCSEditor::CancelAlign() {
	GetDlgItem(IDC_MFCBUTTON_ALIGNX)->EnableWindow(TRUE);
	GetDlgItem(IDC_MFCBUTTON_ALIGNZ)->EnableWindow(TRUE);
	GetDlgItem(IDC_MFCBUTTON_ALIGNOK)->EnableWindow(TRUE);
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	pMain->SetCursor(NORMAL);
	m_alignAxisIndex = -1;
}

void CUCSEditor::AlignWith(CP_AssembledBody *pAsmbBody, int bodyIndex, int edgeIndex)
{
	CP_BodyExt *pBody = dynamic_cast<CP_BodyExt *>(pAsmbBody->GetBody(bodyIndex));
	CP_Edge *pEdge = pBody->GetEdge(edgeIndex);
	CP_BodyExt *pOriBody = dynamic_cast<CP_BodyExt *>(pAsmbBody->GetOriginalBody());
	CP_Vector3D* uscVecs[3] = {&pOriBody->m_ucs.m_axisX,
		&pOriBody->m_ucs.m_axisZ, &pOriBody->m_ucs.m_axisY};

	const int secSize = 5;
	double secArr[secSize] = {0.1, 0.3, 0.5, 0.7, 0.9};
	CP_Vector3D sum(0, 0, 0);
	for (int i = 0; i < secSize; ++i) {
		sum += pEdge->m_pCurve3D->GetTangent(secArr[i]);
	}
	sum.Normalize();
	*uscVecs[m_alignAxisIndex] = sum;
	cubic_corner::NormalizeCubicCornerVecs(uscVecs);
	cubic_corner::CalcCubicCorner(uscVecs, 2, 1);
	CancelAlign();
}

void CUCSEditor::OnBnClickedMfcbuttonAlignz()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_MFCBUTTON_ALIGNX)->EnableWindow(FALSE);
	GetDlgItem(IDC_MFCBUTTON_ALIGNZ)->EnableWindow(FALSE);
	GetDlgItem(IDC_MFCBUTTON_ALIGNOK)->EnableWindow(FALSE);
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	pMain->SetCursor(SEL);
	m_alignAxisIndex = 1;
}


BOOL CUCSEditor::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}


void CUCSEditor::OnBnClickedMfcbuttonAlignok()
{
	// TODO: 在此添加控件通知处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CDockablePane *pPane = pMain->GetUCSEditorPane();
	pPane->ShowPane(FALSE, FALSE, FALSE);
	pMain->RemovePaneFromDockManager(pPane,TRUE,TRUE,TRUE,NULL);
	pMain->RecalcLayout();

	// Really close the window so the docking manager also doesn't know of it anymore.
	pPane->PostMessage(WM_CLOSE);
}
