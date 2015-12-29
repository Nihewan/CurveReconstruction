// FCCRCtrlDialog.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SLDR.h"
#include "MainFrm.h"
#include "SLDRView.h"
#include "SLDRDoc.h"
#include "FCCRCtrlDialog.h"
#include "afxdialogex.h"
// FCCRCtrlDialog �Ի���

IMPLEMENT_DYNAMIC(FCCRCtrlDialog, CDialogEx)

FCCRCtrlDialog::FCCRCtrlDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(FCCRCtrlDialog::IDD, pParent)
{
	select2cellEnable=false;
	selectpatchEnable=false;
	randomColor=true;
	colorByDis=false;
	triboundary=false;
	_2cellboundary=false;
	patchboundary=false;
	edgeByDegree=false;
	showpoly=false;
	showcreators=false;
	selcreator=-1;
	sel3cell=-1;
	showvoids=false;
	mTrans=0.7;
}

FCCRCtrlDialog::~FCCRCtrlDialog()
{
}

void FCCRCtrlDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFOTREE, m_treeCtrl);
}


BEGIN_MESSAGE_MAP(FCCRCtrlDialog, CDialogEx)
	ON_BN_CLICKED(IDC_SELECT_TRIANGLE, &FCCRCtrlDialog::OnBnClickedSelect2cell)
	ON_BN_CLICKED(IDC_CHECK1, &FCCRCtrlDialog::OnBnClickedRandomColor)
	ON_BN_CLICKED(IDC_CHECK2, &FCCRCtrlDialog::OnBnClickedColorByDistance)
	ON_BN_CLICKED(IDC_CHECK_TRI, &FCCRCtrlDialog::OnBnClickedCheckTri)
	ON_BN_CLICKED(IDC_CHECK_2CELL, &FCCRCtrlDialog::OnBnClickedCheck2cell)
	ON_BN_CLICKED(IDC_CLEAR, &FCCRCtrlDialog::OnBnClickedClear)
	ON_EN_CHANGE(IDC_EDIT_VOIDS, &FCCRCtrlDialog::OnEnChangeEditVoids)
	ON_EN_CHANGE(IDC_EDIT_EPSILON, &FCCRCtrlDialog::OnEnChangeEditEpsilon)
	ON_EN_CHANGE(IDC_EDIT_FEASA, &FCCRCtrlDialog::OnEnChangeEditFeasa)
	ON_BN_CLICKED(IDC_CHECK_POLYLINE, &FCCRCtrlDialog::OnBnClickedCheckPolyline)
	ON_BN_CLICKED(IDC_CHECK_CREATOR, &FCCRCtrlDialog::OnBnClickedCheckCreator)
ON_NOTIFY(TVN_SELCHANGED, IDC_INFOTREE, &FCCRCtrlDialog::OnTvnSelchangedInfotree)
ON_BN_CLICKED(IDC_CHECK_VOIDS, &FCCRCtrlDialog::OnBnClickedCheckVoids)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TANSPARENT, &FCCRCtrlDialog::OnNMCustomdrawSliderTansparent)
ON_BN_CLICKED(IDC_CHECK4, &FCCRCtrlDialog::OnBnClickedCheck4)
ON_BN_CLICKED(IDC_CHECK_PATCH, &FCCRCtrlDialog::OnBnClickedCheckPatch)
END_MESSAGE_MAP()


// FCCRCtrlDialog ��Ϣ�������



void FCCRCtrlDialog::SetItems(CP_FlowComplex *FlowComplex,vector<CP_PolyLine3D> *VT_PolyLine)
{
	fccr.m_FlowComplex=FlowComplex;
	fccr.m_VT_PolyLine=VT_PolyLine;
}

void FCCRCtrlDialog::SetTreeItems(int _2cell)
{
	CP_2cell *m_2cell=fccr.m_FlowComplex->m_2cells[fccr.m_FlowComplex->Locate2cell(_2cell)];
	m_treeCtrl.DeleteAllItems(); 
	CString cstr;

	cstr.Format("2cell:%d",_2cell);//2cell��m_2cell�е�index
	HTREEITEM h2cell=m_treeCtrl.InsertItem(cstr);

	for(int i=0;i<m_2cell->m_triangle.size();i++)
	{
		cstr.Format("triangle:%d",m_2cell->m_triangle[i]);
		HTREEITEM htri = m_treeCtrl.InsertItem(cstr,h2cell);
		//CP_Triganle3D *ptrTri=fccr.m_FlowComplex->tricells[m_2cell->m_triangle[i]];
		//for(int j=0;j<3;j++)
		//{
		//	cstr.Format("vertices : %d",ptrTri->m_points[j]);
		//	HTREEITEM hpoint = m_treeCtrl.InsertItem(cstr,htri);

		//	cstr.Format("x: %f",fccr.m_FlowComplex->m_0cells[ptrTri->m_points[j]].m_x);
		//	HTREEITEM hVert_x = m_treeCtrl.InsertItem(cstr,hpoint);
		//	cstr.Format("y: %f",fccr.m_FlowComplex->m_0cells[ptrTri->m_points[j]].m_y);
		//	HTREEITEM hVert_y = m_treeCtrl.InsertItem(cstr,hpoint);
		//	cstr.Format("z: %f",fccr.m_FlowComplex->m_0cells[ptrTri->m_points[j]].m_z);
		//	HTREEITEM hVert_z = m_treeCtrl.InsertItem(cstr,hpoint);
		//}//j
	}//i
	m_treeCtrl.Expand(h2cell,TVE_EXPAND);
}

void FCCRCtrlDialog::SetTreePatch(int _patch)
{
	CP_Patch *pPatch=fccr.m_FlowComplex->m_patches[_patch];
	m_treeCtrl.DeleteAllItems(); 
	CString cstr;

	for(int i=0;i<pPatch->m_2cells.size();i++)
	{
		cstr.Format("2cell:%d",fccr.m_FlowComplex->m_2cells[pPatch->m_2cells[i]]->index);//2cell��m_2cell�е�index
		m_treeCtrl.InsertItem(cstr);
	}
}

void FCCRCtrlDialog::SetTreeCreator()
{
	CString cstr;
	m_treeCtrl.DeleteAllItems(); 
	for(int i=fccr.m_FlowComplex->desN;i<fccr.m_FlowComplex->m_2cells.size();i++)
	{
		cstr.Format("2cell:%d",i);//2cell��m_2cell�е�index
		m_treeCtrl.InsertItem(cstr);
	}//creators
}

void FCCRCtrlDialog::SetTree3cells()
{
	CString cstr;
	m_treeCtrl.DeleteAllItems(); 
	for(int i=0;i<fccr.m_FlowComplex->m_3cells.size();i++)
	{
		cstr.Format("3cell:%d",i);//2cell��m_2cell�е�index
		m_treeCtrl.InsertItem(cstr);
	}//creators
}

void FCCRCtrlDialog::OnBnClickedSelect2cell()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	select2cellEnable=!select2cellEnable;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	if(!select2cellEnable)
		pView->selected2cell=-1;
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedRandomColor()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	randomColor=!randomColor;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedColorByDistance()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	edgeByDegree=!edgeByDegree;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckTri()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	triboundary=!triboundary;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheck2cell()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	_2cellboundary=!_2cellboundary;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedClear()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	selcreator=-1;
	sel3cell=-1;
	fccr.m_FlowComplex->clearAll();
	fccr.maxhd=0;
	fccr.T.clear();

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->IsProcess=false;
	pView->Invalidate();
}


BOOL FCCRCtrlDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	GetDlgItem(IDC_EDIT_EPSILON)->SetWindowText("0.055");
	GetDlgItem(IDC_EDIT_FEASA)->SetWindowText("0.05");
	GetDlgItem(IDC_EDIT_VOIDS)->SetWindowText("0");
	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(BST_CHECKED);
	CSliderCtrl *pSlidCtrl=(CSliderCtrl*)GetDlgItem(IDC_SLIDER_TANSPARENT);
	pSlidCtrl->SetPos(mTrans*100);
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void FCCRCtrlDialog::OnEnChangeEditVoids()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;   
	GetDlgItemText(IDC_EDIT_VOIDS,str);
	fccr.voids=_ttoi(str);
}


void FCCRCtrlDialog::OnEnChangeEditEpsilon()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;   
	GetDlgItemText(IDC_EDIT_EPSILON,str);
	fccr.epsilon=atof(str);
}

void FCCRCtrlDialog::OnEnChangeEditFeasa()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;   
	GetDlgItemText(IDC_EDIT_FEASA,str);
	fccr.feasa=atof(str);
}


void FCCRCtrlDialog::OnBnClickedCheckPolyline()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	showpoly=!showpoly;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckCreator()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(!showcreators)
	{
		showvoids=false;
		((CButton*)GetDlgItem(IDC_CHECK_VOIDS))->SetCheck(BST_UNCHECKED);
		showcreators=true;
		SetTreeCreator();
	}else{
		showcreators=false;
	}
	selcreator=0;
	sel3cell=0;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}

void FCCRCtrlDialog::OnTvnSelchangedInfotree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	CString strText; // ���ڵ�ı�ǩ�ı��ַ���   
	CString seltype;
	// ��ȡ��ǰѡ�нڵ�ľ��   
	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();  
	// ��ȡѡ�нڵ�ı�ǩ�ı��ַ���   
	strText = m_treeCtrl.GetItemText(hItem); 

	if(showcreators)
	{
		int _2cell=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));
		selcreator=_2cell;
	}
	if(showvoids)
	{
		sel3cell=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));
	}
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckVoids()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(!showvoids)
	{
		showcreators=false;
		((CButton*)GetDlgItem(IDC_CHECK_CREATOR))->SetCheck(BST_UNCHECKED);
		showvoids=true;
		SetTree3cells();
	}else
		showvoids=false;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnNMCustomdrawSliderTansparent(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	CSliderCtrl *pSlidCtrl=(CSliderCtrl*)GetDlgItem(IDC_SLIDER_TANSPARENT);
	int nPos = pSlidCtrl->GetPos(); //��û���ĵ�ǰλ��
	mTrans=nPos/100.0;

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheck4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	patchboundary=!patchboundary;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckPatch()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	selectpatchEnable=!selectpatchEnable;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	if(!selectpatchEnable)
		pView->selectedpatch=-1;
	pView->Invalidate();
}
