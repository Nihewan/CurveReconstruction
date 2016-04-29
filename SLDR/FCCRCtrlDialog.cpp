// FCCRCtrlDialog.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SLDR.h"
#include "MainFrm.h"
#include "SLDRView.h"
#include "SLDRDoc.h"
#include "FCCRCtrlDialog.h"
#include "afxdialogex.h"
#include "CP_FlowComplex.h"
// FCCRCtrlDialog �Ի���

//#define WM_RESULT WM_USER+500

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
	sel2cell=-1;
	seltriangle=-1;
	selpoly=-1;
	showvoids=false;
	showcircum=false;
	shownongabriel=false;
	showvoronoi=false;
	mTrans=1.0;
	play=0;
	pos=0;
	pause=false;
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
ON_BN_CLICKED(IDC_CHECK4, &FCCRCtrlDialog::OnBnClickedCheck4)
ON_BN_CLICKED(IDC_CHECK_PATCH, &FCCRCtrlDialog::OnBnClickedCheckPatch)
ON_BN_CLICKED(IDC_CHECK_CIRCUM, &FCCRCtrlDialog::OnBnClickedCheckCircum)
ON_BN_CLICKED(IDC_CHECK_NONGABRIEL, &FCCRCtrlDialog::OnBnClickedCheckNongabriel)
ON_BN_CLICKED(IDC_CHECK_VORONOI, &FCCRCtrlDialog::OnBnClickedCheckVoronoi)
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

	for(unsigned int i=0;i<m_2cell->m_triangle.size();i++)
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

	for(unsigned int i=0;i<pPatch->m_2cells.size();i++)
	{
		cstr.Format("2cell:%d",fccr.m_FlowComplex->m_2cells[pPatch->m_2cells[i]]->index);//2cell��m_2cell�е�index
		m_treeCtrl.InsertItem(cstr);
	}
}

void FCCRCtrlDialog::SetTreeCreator()
{
	CString cstr;
	m_treeCtrl.DeleteAllItems(); 
	for(unsigned int i=fccr.m_FlowComplex->desN;i<fccr.m_FlowComplex->m_2cells.size();i++)
	{
		cstr.Format("2cell:%d",i);//2cell��m_2cell�е�index
		m_treeCtrl.InsertItem(cstr);
	}//creators
}

void FCCRCtrlDialog::SetTree3cells()
{
	CString cstr;
	m_treeCtrl.DeleteAllItems(); 
	for(unsigned int i=0;i<fccr.m_FlowComplex->m_3cells.size();i++)
	{
		cstr.Format("3cell:%d",i);//2cell��m_2cell�е�index
		HTREEITEM h3cell=m_treeCtrl.InsertItem(cstr);
		CP_3cell *p3cell=fccr.m_FlowComplex->m_3cells[i];
		for(unsigned int j=0;j<p3cell->m_2cells.size();j++)
		{
			CP_2cell *p2cell=fccr.m_FlowComplex->m_2cells[p3cell->m_2cells[j]];
			cstr.Format("2cell:%d",p2cell->index);//2cell��m_2cell�е�index
			HTREEITEM h2cell=m_treeCtrl.InsertItem(cstr,h3cell);
			for(unsigned int k=0;k<p2cell->m_triangle.size();k++)
			{
				cstr.Format("triangle:%d",p2cell->m_triangle[k]);
				HTREEITEM htri = m_treeCtrl.InsertItem(cstr,h2cell);
			}//k
		}
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
	// ˢ��״̬����մ�����
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
	sel2cell=-1;
	seltriangle=-1;
	selpoly=-1;
	showvoids=false;
	showcircum=false;
	shownongabriel=false;
	showvoronoi=false;
	mTrans=1.0;
	play=0;
	pos=0;
	pause=false;

	fccr.ReSet();
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->ReSet();

	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_SELECT_TRIANGLE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_PATCH))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_TRI))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_2CELL))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK4))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_SHOWEDGE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_POLYLINE))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_VORONOI))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_CREATOR))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_VOIDS))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_NONGABRIEL))->SetCheck(BST_UNCHECKED);
	GetDlgItem(IDC_EDIT_EPSILON)->SetWindowText("0.055");
	GetDlgItem(IDC_EDIT_FEASA)->SetWindowText("0.7");
	GetDlgItem(IDC_EDIT_VOIDS)->SetWindowText("0");
	pView->Invalidate();
}


BOOL FCCRCtrlDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	GetDlgItem(IDC_EDIT_EPSILON)->SetWindowText("0.055");
	GetDlgItem(IDC_EDIT_FEASA)->SetWindowText("0.7");
	GetDlgItem(IDC_EDIT_VOIDS)->SetWindowText("0");
	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(BST_CHECKED);
	CSliderCtrl *pSlidCtrl=(CSliderCtrl*)GetDlgItem(IDC_SLIDER_TANSPARENT);
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

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());

	CString strText; // ���ڵ�ı�ǩ�ı��ַ���   
	CString seltype;
	// ��ȡ��ǰѡ�нڵ�ľ��   
	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();  
	// ��ȡѡ�нڵ�ı�ǩ�ı��ַ���   
	strText = m_treeCtrl.GetItemText(hItem); 
	seltype=strText.Left(strText.Find(":"));
	
	if(showcreators)
	{
		int _2cell=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));
		selcreator=_2cell;
	}
	if(showvoids)
	{
		if(seltype=="3cell"){
			sel3cell=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));

			sel2cell=-1;
			seltriangle=-1;
		}
		else if(seltype=="2cell"){
			sel2cell=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));

			HTREEITEM h3cell=m_treeCtrl.GetParentItem(hItem);
			strText = m_treeCtrl.GetItemText(h3cell); 
			sel3cell=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));

			seltriangle=-1;
		}else if(seltype=="triangle")
		{
			seltriangle=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));
			HTREEITEM h2cell=m_treeCtrl.GetParentItem(hItem);
			strText = m_treeCtrl.GetItemText(h2cell); 
			sel2cell=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));

			HTREEITEM h3cell=m_treeCtrl.GetParentItem(h2cell);
			strText = m_treeCtrl.GetItemText(h3cell); 
			sel3cell=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));
		}
	}
	
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


void FCCRCtrlDialog::OnBnClickedCheckCircum()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	showcircum=!showcircum;
	if(showcircum)
		mTrans=0.7;
	else
		mTrans=1.0;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


//void FCCRCtrlDialog::OnBnClickedCheckNonGabriel()
//{
//	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	shownongabriel=!shownongabriel;
//	if(shownongabriel)
//		cout<<"aaaaaaaaaa"<<endl;
//	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
//	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
//	pView->Invalidate();
//}


void FCCRCtrlDialog::OnBnClickedCheckNongabriel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	shownongabriel=!shownongabriel;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckVoronoi()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	showvoronoi=!showvoronoi;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnButtonReconstruction()
{
	// TODO: �ڴ���������������
	thread=AfxBeginThread(ThreadFunc,this);  
}

void FCCRCtrlDialog::OnImprovedFCReconstruction()
{
	thread=AfxBeginThread(ThreadImrovedFC,this);
}

void FCCRCtrlDialog::OnPlayFC()
{
	pFCthread=AfxBeginThread(ThreadPlayFC,this);
}

UINT  ThreadPlayFC(LPVOID pParam) 
{  
	FCCRCtrlDialog *dlg=(FCCRCtrlDialog*)pParam;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	CSLDRDoc* pDoc = pView->GetDocument();
	int num=0;
	//cout<<dlg->selpoly<<endl;
	if(dlg->selpoly!=-1)
	    num=static_cast<int>(dlg->fccr.m_FlowComplex->m_PolyLine[dlg->selpoly].cycle.size());
	
	for(int i=0;i<num;i++)
	{
		dlg->play++;
		dlg->pos=(dlg->play*100)/num;
		//delay
		::PostMessage(pMain->m_hWnd,WM_RESULT_FCPLAY,0,0);
		Sleep(1000);
	}
	dlg->play=0;//��ʾ�����ʾFC������ʾ����
	return 0;
}

UINT  ThreadFunc(LPVOID pParam) 
{  
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	clock_t start,end;
	//polyline
	FCCRCtrlDialog *fc=(FCCRCtrlDialog*)pParam;
	start = clock();
	fc->fccr.ToPolyLine();
	end = clock();
	cout<<"time for polyline: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//delauny and voronoi
	start = clock();
	fc->fccr.OnDelaunyTriangulation();
	end = clock();
	cout<<"time for Delauny: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//flow complex
	//pMain->m_ctrlPaneFCCR->m_dialog.fccr.filename=pDoc->filename;
	start = clock();
	fc->fccr.ToFlowcomplex();
	end = clock();
	cout<<"time for compute Flow Complex: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	// topological reconstruction
	start = clock();
	fc->fccr.OnCollapse();
	end = clock();
	cout<<"time for collapse: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//thicken
	start = clock();
	fc->fccr.OnThicken();
	end = clock();
	cout<<"time for Thicken and Pruning: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	
	fc->fccr.showResult=true;
	fc->fccr.IsProcess=true;
	::PostMessage(pMain->m_hWnd,WM_RESULT,0,0);
	return 0;
}  

UINT  ThreadImrovedFC(LPVOID pParam) 
{  
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	clock_t start,end;
	//polyline
	FCCRCtrlDialog *fc=(FCCRCtrlDialog*)pParam;
	fc->fccr.IsProcess=true;
	start = clock();
	fc->fccr.IFCPolyline();//step0 �Ƕȷ���
	//::PostMessage(pMain->m_hWnd,WM_RESULT_REFRESH,0,0);//ˢ�½������
	//fc->thread->SuspendThread();

	//fc->fccr.ShortestCycle();//step1 ��С��
	//::PostMessage(pMain->m_hWnd,WM_RESULT_REFRESH,0,0);//ˢ�½������
	//fc->thread->SuspendThread();

	//fc->fccr.ConfirmClassification();//step2 ��С��Ϊ˫�ı߲�Ϊ������
	//fc->fccr.SetSymmetricCurveTagTrue();
	cout<<fc->fccr.m_FlowComplex->m_PolyLine.size()<<endl;
	end = clock();
	cout<<"time for polyline: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//delauny and voronoi
	//start = clock();
	//fc->fccr.OnDelaunyTriangulation();
	//end = clock();
	//cout<<"time for Delauny: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	////flow complex
	////pMain->m_ctrlPaneFCCR->m_dialog.fccr.filename=pDoc->filename;
	//start = clock();
	//fc->fccr.ToFlowcomplex();
	//end = clock();
	//cout<<"time for compute Flow Complex: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//// topological reconstruction
	//start = clock();
	//fc->fccr.OnCollapse();
	//end = clock();
	//cout<<"time for collapse: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	////thicken
	//start = clock();
	//fc->fccr.OnThicken();
	//end = clock();
	//cout<<"time for Thicken and Pruning: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;

	//fc->fccr.showResult=true;
	::PostMessage(pMain->m_hWnd,WM_RESULT,0,0);//ˢ�½������
	return 0;
}

void FCCRCtrlDialog::OnPlayFCPause()
{
	if(!pause){
		pause=true;
		pFCthread->SuspendThread();
	}
	else{
		pause=false;
		pFCthread->ResumeThread();
	}
}

void FCCRCtrlDialog::OnIFCMethodNext()
{
	thread->ResumeThread();
}
