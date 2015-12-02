// CTRLDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "SLDR.h"
#include "CtrlDialog.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "SLDRView.h"
#include "FaceIdentification.h"
#include "SLDRDoc.h"
#include "BodyDecomposition.h"
#include "FaceClassification.h"
#include "CDepthEstimation.h"
#include "Reconstruction.h"

// CCTRLDialog 对话框

IMPLEMENT_DYNAMIC(CCtrlDialog, CDialogEx)

	CCtrlDialog::CCtrlDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCtrlDialog::IDD, pParent)
{

}

CCtrlDialog::~CCtrlDialog()
{
}

void CCtrlDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BODYTREE, m_bodyEleTree);
	DDX_Control(pDX, IDC_MFCBUTTON_FACEIDENT, m_btnFaceIdent);
}


BEGIN_MESSAGE_MAP(CCtrlDialog, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TVN_SELCHANGED, IDC_BODYTREE, &CCtrlDialog::OnTvnSelchangedTree)
	ON_BN_CLICKED(IDC_MFCBUTTON_FACEIDENT, &CCtrlDialog::OnBnClickedMfcbuttonFaceident)
	ON_BN_CLICKED(IDC_MFCBUTTON_CUT, &CCtrlDialog::OnBnClickedMfcbuttonCut)
	ON_BN_CLICKED(IDC_MFCBUTTON_FACECLS, &CCtrlDialog::OnBnClickedMfcbuttonFacecls)
	ON_BN_CLICKED(IDC_MFCBUTTON_RECONSTRUCTION, &CCtrlDialog::OnBnClickedMfcbuttonReconstruction)
	ON_BN_CLICKED(IDC_MFCBUTTON_DEPTHESTIM, &CCtrlDialog::OnBnClickedMfcbuttonDepthestim)
	ON_BN_CLICKED(IDC_MFCBUTTON_OBTAINUCS, &CCtrlDialog::OnBnClickedMfcbuttonObtainucs)
END_MESSAGE_MAP()


// CCTRLDialog 消息处理程序


HBRUSH CCtrlDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	pDC-> SetBkMode(TRANSPARENT);
	hbr   =   CreateSolidBrush(RGB(240,240,240));
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CCtrlDialog::SetTreeItems(CP_AssembledBody *pAsmbBody) {
	m_pAsmbBody = pAsmbBody;
	CString cstr;
	m_bodyEleTree.DeleteAllItems();

	for (int bodyIndex = 0; bodyIndex < pAsmbBody->GetBodyNumber(); ++bodyIndex)
	{
		CP_Body *pBody = m_pAsmbBody->GetBody(bodyIndex);
		cstr.Format("Body: %d", bodyIndex);
		HTREEITEM hBody = m_bodyEleTree.InsertItem(cstr);
		cstr.Format("FACES : %d",pBody->GetFaceNumber());
		HTREEITEM hParentFace = m_bodyEleTree.InsertItem(cstr,hBody);
		cstr.Format("VERTICES : %d",pBody->GetVertexNumber());
		HTREEITEM hParentVert = m_bodyEleTree.InsertItem(cstr,hBody);
		cstr.Format("EDGES : %d",pBody->GetEdgeNumber());
		HTREEITEM hParentEdge = m_bodyEleTree.InsertItem(cstr,hBody);

		m_bodyEleTree.Expand(hBody,TVE_EXPAND);

		// Vertices
		for (int i = 0; i < pBody->GetVertexNumber(); i++)
		{
			CP_Vertex *pVert = pBody->GetVertex(i);
			cstr.Format("Vertex: %d",i);
			HTREEITEM hVert = m_bodyEleTree.InsertItem(cstr,hParentVert);
			cstr.Format("x: %f",pVert->m_pPoint->m_x);
			HTREEITEM hVert_x = m_bodyEleTree.InsertItem(cstr,hVert);
			cstr.Format("y: %f",pVert->m_pPoint->m_y);
			HTREEITEM hVert_y = m_bodyEleTree.InsertItem(cstr,hVert);
			cstr.Format("z: %f",pVert->m_pPoint->m_z);
			HTREEITEM hVert_z = m_bodyEleTree.InsertItem(cstr,hVert);
		}

		// Edges
		for (int i = 0; i < pBody->GetEdgeNumber(); i++)
		{
			CP_Edge *pEdge = pBody->GetEdge(i);
			cstr.Format("Edge: %d",i);
			HTREEITEM hEdge = m_bodyEleTree.InsertItem(cstr,hParentEdge);
			cstr.Format("StrVert: %d",pBody->FindVertexIndex(pEdge->m_pStartVt));
			HTREEITEM hEdge_str = m_bodyEleTree.InsertItem(cstr,hEdge);
			cstr.Format("EndVert: %d",pBody->FindVertexIndex(pEdge->m_pEndVt));
			HTREEITEM hEdge_end = m_bodyEleTree.InsertItem(cstr,hEdge);
		}

		// Faces
		for (int i = 0; i < pBody->GetFaceNumber(); i++)
		{
			CP_Face *pFace = pBody->GetFace(i);
			cstr.Format("Face: %d",i);
			HTREEITEM hEdge = m_bodyEleTree.InsertItem(cstr,hParentFace);
		}
	}

}


void CCtrlDialog::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	CSLDRDoc* pDoc = pView->GetDocument();

	HTREEITEM iTem = m_bodyEleTree.GetSelectedItem();
	if(iTem == NULL) return;
	CString strSelItem = m_bodyEleTree.GetItemText(iTem);

	HTREEITEM rootItem, tempItem = iTem;
	do {
		rootItem = tempItem;
		tempItem = m_bodyEleTree.GetParentItem(tempItem);
	} while(tempItem != NULL);

	// Set selected item's information
	pDoc->m_selectedItem.m_strSelectedItem = strSelItem;
	CString rootStr = m_bodyEleTree.GetItemText(rootItem);
	rootStr = rootStr.Mid(rootStr.Find(' ')+1);
	pDoc->m_selectedItem.m_bodyIndex = atoi(rootStr.GetBuffer(rootStr.GetLength()));

	pView->Invalidate();
}

void CCtrlDialog::SetCurrentStep(int step) {
	if (step == STEP_FACE_IDENTIFICATION) {
		m_btnFaceIdent.EnableWindow(TRUE);
	}
}

void CCtrlDialog::OnBnClickedMfcbuttonFaceident()
{
	CFaceIdentification faceIdent(m_pAsmbBody);
	faceIdent.FaceIdentify();
	this->SetTreeItems(m_pAsmbBody);

	// Invalidate button state
	CWnd *pWnd = GetDlgItem(IDC_MFCBUTTON_FACEIDENT);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_MFCBUTTON_CUT);
	pWnd->EnableWindow(TRUE);
}


void CCtrlDialog::OnBnClickedMfcbuttonCut()
{
	CBodyDecomposition bodyDec(m_pAsmbBody);
	bodyDec.Decompose();
	this->SetTreeItems(m_pAsmbBody);

	// Invalidate button state
	CWnd *pWnd = GetDlgItem(IDC_MFCBUTTON_CUT);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_MFCBUTTON_FACECLS);
	pWnd->EnableWindow(TRUE);
}


void CCtrlDialog::OnBnClickedMfcbuttonFacecls()
{
	CFaceClassification faceCls(m_pAsmbBody);
	faceCls.ClassifyFace();
	this->SetTreeItems(m_pAsmbBody);

	CWnd *pWnd = GetDlgItem(IDC_MFCBUTTON_FACECLS);
	pWnd->EnableWindow(FALSE);
	pWnd = GetDlgItem(IDC_MFCBUTTON_OBTAINUCS);
	pWnd->EnableWindow(TRUE);
}


void CCtrlDialog::OnBnClickedMfcbuttonDepthestim()
{
	// TODO: 在此添加控件通知处理程序代码
	CDepthEstimation estimator(m_pAsmbBody);
	estimator.Estimate();

	CWnd *pWnd = GetDlgItem(IDC_MFCBUTTON_DEPTHESTIM);
	pWnd->EnableWindow(FALSE);

	pWnd = GetDlgItem(IDC_MFCBUTTON_RECONSTRUCTION);
	pWnd->EnableWindow(TRUE);

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate(); 
}

void CCtrlDialog::OnBnClickedMfcbuttonReconstruction()
{
	// TODO: 在此添加控件通知处理程序代码
	vector<int> vWeight;
	vWeight.push_back(20);
	vWeight.push_back(1);

	CReconstruction rec(m_pAsmbBody);
	rec.Reconstruction(vWeight);
	this->SetTreeItems(m_pAsmbBody);

	CWnd *pWnd = GetDlgItem(IDC_MFCBUTTON_RECONSTRUCTION);
	pWnd->EnableWindow(FALSE);

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}



void CCtrlDialog::OnBnClickedMfcbuttonObtainucs()
{
	// TODO: 在此添加控件通知处理程序代码
	CDepthEstimation estimator(m_pAsmbBody);
	estimator.EstimateUCS();

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	pMain->ShowUCSEditor();

	CWnd *pWnd = GetDlgItem(IDC_MFCBUTTON_OBTAINUCS);
	pWnd->EnableWindow(FALSE);
// 	pWnd = GetDlgItem(IDC_MFCBUTTON_DEPTHESTIM);
// 	pWnd->EnableWindow(TRUE);
}
