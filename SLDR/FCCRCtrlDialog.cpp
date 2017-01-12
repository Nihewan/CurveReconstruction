// FCCRCtrlDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "SLDR.h"
#include "MainFrm.h"
#include "SLDRView.h"
#include "SLDRDoc.h"
#include "FCCRCtrlDialog.h"
#include "afxdialogex.h"
#include "CP_FlowComplex.h"
#include "ParaDialog.h"
// FCCRCtrlDialog 对话框

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
	selpatch=-1;
	showDelaunay=false;
	showvoids=false;
	showcircum=false;
	showbystep=false;
	showdarts=false;
	showinteriorpatches=false;
	showRMF=false;

	mTrans=1.0;
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
	ON_BN_CLICKED(IDC_CHECK_POLYLINE, &FCCRCtrlDialog::OnBnClickedCheckPolyline)
	ON_BN_CLICKED(IDC_CHECK_CREATOR, &FCCRCtrlDialog::OnBnClickedCheckCreator)
ON_NOTIFY(TVN_SELCHANGED, IDC_INFOTREE, &FCCRCtrlDialog::OnTvnSelchangedInfotree)
ON_BN_CLICKED(IDC_CHECK_VOIDS, &FCCRCtrlDialog::OnBnClickedCheckVoids)
ON_BN_CLICKED(IDC_CHECK4, &FCCRCtrlDialog::OnBnClickedCheck4)
ON_BN_CLICKED(IDC_CHECK_PATCH, &FCCRCtrlDialog::OnBnClickedCheckPatch)
ON_BN_CLICKED(IDC_CHECK_NONGABRIEL, &FCCRCtrlDialog::OnBnClickedCheckShowByStep)
ON_BN_CLICKED(IDC_SHOW_DARTS, &FCCRCtrlDialog::OnBnClickedShowDarts)
ON_BN_CLICKED(IDC_BUTTON_Para, &FCCRCtrlDialog::OnBnClickedButtonPara)
ON_BN_CLICKED(IDC_CHECK5, &FCCRCtrlDialog::OnBnClickedCheckShowInterior)
ON_BN_CLICKED(IDC_CHECK6, &FCCRCtrlDialog::OnBnClickedCheckShowRMF)
ON_BN_CLICKED(IDC_CHECK_DELAUNAY, &FCCRCtrlDialog::OnBnClickedShowDelaunay)
END_MESSAGE_MAP()


// FCCRCtrlDialog 消息处理程序

void FCCRCtrlDialog::SetItems(CP_FlowComplex *FlowComplex,vector<CP_PolyLine3D> *VT_PolyLine)
{
	fccr.m_FlowComplex=FlowComplex;
	fccr.m_FlowComplex->inputCurves=VT_PolyLine->size();
	fccr.m_VT_PolyLine=VT_PolyLine;
}

void FCCRCtrlDialog::SetTreeItems(int _2cell)
{
	CP_2cell *m_2cell=fccr.m_FlowComplex->m_2cells[fccr.m_FlowComplex->Locate2cell(_2cell)];
	m_treeCtrl.DeleteAllItems(); 
	CString cstr;

	cstr.Format("2cell:%d",_2cell);//2cell在m_2cell中的index
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
		CP_2cell *p2cell=fccr.m_FlowComplex->m_2cells[pPatch->m_2cells[i]];
		cstr.Format("2cell:%d",p2cell->index);//2cell在m_2cell中的index
		HTREEITEM h2cell=m_treeCtrl.InsertItem(cstr);
		for(unsigned int k=0;k<p2cell->m_triangle.size();k++)
		{
			cstr.Format("triangle:%d",p2cell->m_triangle[k]);
			HTREEITEM htri = m_treeCtrl.InsertItem(cstr,h2cell);
		}//k
	}
}

void FCCRCtrlDialog::SetTreeCreator()
{
	CString cstr;
	m_treeCtrl.DeleteAllItems(); 
	for(unsigned int i=fccr.m_FlowComplex->desN;i<fccr.m_FlowComplex->m_2cells.size();i++)
	{
		cstr.Format("2cell:%d",i);//2cell在m_2cell中的index
		m_treeCtrl.InsertItem(cstr);
	}//creators
}

void FCCRCtrlDialog::SetTree3cells()
{
	CString cstr;
	m_treeCtrl.DeleteAllItems(); 
	for(unsigned int i=0;i<fccr.m_FlowComplex->m_3cells.size();i++)
	{
		cstr.Format("3cell:%d",i);//2cell在m_2cell中的index
		HTREEITEM h3cell=m_treeCtrl.InsertItem(cstr);
		CP_3cell *p3cell=fccr.m_FlowComplex->m_3cells[i];
		for(unsigned int j=0;j<p3cell->m_2cells.size();j++)
		{
			CP_2cell *p2cell=fccr.m_FlowComplex->m_2cells[p3cell->m_2cells[j]];
			cstr.Format("2cell:%d",p2cell->index);//2cell在m_2cell中的index
			HTREEITEM h2cell=m_treeCtrl.InsertItem(cstr,h3cell);
			for(unsigned int k=0;k<p2cell->m_triangle.size();k++)
			{
				cstr.Format("triangle:%d",p2cell->m_triangle[k]);
				HTREEITEM htri = m_treeCtrl.InsertItem(cstr,h2cell);
			}//k
		}
	}//creators
}

void FCCRCtrlDialog::SetTreePatches()
{
	CString cstr;
	m_treeCtrl.DeleteAllItems(); 
	for(unsigned int i=0;i<fccr.m_FlowComplex->m_patches.size();i++)
	{
		if(fccr.m_FlowComplex->m_patches[i]->flag){
			cstr.Format("patch:%d",i);//2cell在m_2cell中的index
			HTREEITEM hpatch=m_treeCtrl.InsertItem(cstr);
			CP_Patch *pPatch=fccr.m_FlowComplex->m_patches[i];
			for(unsigned int j=0;j<pPatch->m_2cells.size();j++)
			{
				CP_2cell *p2cell=fccr.m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
				cstr.Format("2cell:%d",p2cell->index);//2cell在m_2cell中的index
				HTREEITEM h2cell=m_treeCtrl.InsertItem(cstr,hpatch);
				for(unsigned int k=0;k<p2cell->m_triangle.size();k++)
				{
					cstr.Format("triangle:%d",p2cell->m_triangle[k]);
					HTREEITEM htri = m_treeCtrl.InsertItem(cstr,h2cell);
				}//k
			}
		}
	}//creators
}

void FCCRCtrlDialog::OnBnClickedSelect2cell()
{
	// TODO: 在此添加控件通知处理程序代码
	select2cellEnable=!select2cellEnable;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	if(!select2cellEnable)
		pView->selected2cell=-1;
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedRandomColor()
{
	// TODO: 在此添加控件通知处理程序代码
	randomColor=!randomColor;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedColorByDistance()
{
	// TODO: 在此添加控件通知处理程序代码
	edgeByDegree=!edgeByDegree;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckTri()
{
	// TODO: 在此添加控件通知处理程序代码
	triboundary=!triboundary;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheck2cell()
{
	// TODO: 在此添加控件通知处理程序代码
	_2cellboundary=!_2cellboundary;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}

void FCCRCtrlDialog::OnBnClickedClear()
{
	// 刷新状态、清空处理结果
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
	selpatch=-1;
	showDelaunay=false;
	showvoids=false;
	showcircum=false;
	showbystep=false;
	showdarts=false;
	showinteriorpatches=false;
	showRMF=false;
	mTrans=1.0;

	m_treeCtrl.DeleteAllItems();
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
	((CButton*)GetDlgItem(IDC_CHECK_CREATOR))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_VOIDS))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_NONGABRIEL))->SetCheck(BST_UNCHECKED);
	pView->Invalidate();
}


BOOL FCCRCtrlDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(BST_CHECKED);
	CSliderCtrl *pSlidCtrl=(CSliderCtrl*)GetDlgItem(IDC_SLIDER_TANSPARENT);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void FCCRCtrlDialog::OnBnClickedCheckPolyline()
{
	// TODO: 在此添加控件通知处理程序代码
	showpoly=!showpoly;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckCreator()
{
	// TODO: 在此添加控件通知处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	if(!pView->showFC){
		MessageBox("请先选择FlowComplex");
		((CButton*)GetDlgItem(IDC_CHECK_CREATOR))->SetCheck(BST_UNCHECKED);
	}
	else{
		if(!showcreators)
		{
			showvoids=false;
			showDelaunay=false;
			((CButton*)GetDlgItem(IDC_CHECK_VOIDS))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_CHECK_DELAUNAY))->SetCheck(BST_UNCHECKED);
			showcreators=true;
			SetTreeCreator();
		}else{
			showcreators=false;
		}
		selcreator=0;
		sel3cell=0;
	}
	pView->Invalidate();
}

void FCCRCtrlDialog::OnTvnSelchangedInfotree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());

	CString strText; // 树节点的标签文本字符串   
	CString seltype;
	// 获取当前选中节点的句柄   
	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();  
	// 获取选中节点的标签文本字符串   
	strText = m_treeCtrl.GetItemText(hItem); 
	seltype=strText.Left(strText.Find(":"));
	
	if(showcreators)
	{
		selcreator=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));
		pView->Invalidate();
		return;
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
		pView->Invalidate();
		return;
	}

	if (fccr.showResult)
	{
		if(seltype=="patch"){
			selpatch=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));
		}else if(seltype=="2cell"){
			sel2cell=atoi(strText.Right(strText.GetLength()-strText.Find(":")-1));
		}
		pView->Invalidate();
		return;
	}

	pView->Invalidate();
	return;
}


void FCCRCtrlDialog::OnBnClickedCheckVoids()
{
	// TODO: 在此添加控件通知处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	if(!pView->showFC){
		MessageBox("请先选择FlowComplex");
		((CButton*)GetDlgItem(IDC_CHECK_VOIDS))->SetCheck(BST_UNCHECKED);
	}
	else{
		if(!showvoids)
		{
			showcreators=false;
			showDelaunay=false;
			((CButton*)GetDlgItem(IDC_CHECK_CREATOR))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_CHECK_DELAUNAY))->SetCheck(BST_UNCHECKED);
			showvoids=true;
			SetTree3cells();
		}else
			showvoids=false;
	}
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheck4()
{
	// TODO: 在此添加控件通知处理程序代码
	patchboundary=!patchboundary;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckPatch()
{
	// TODO: 在此添加控件通知处理程序代码
	selectpatchEnable=!selectpatchEnable;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	if(!selectpatchEnable)
		pView->selectedpatch=-1;
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckCircum()
{
	// TODO: 在此添加控件通知处理程序代码
	showcircum=!showcircum;
	if(showcircum)
		mTrans=0.7;
	else
		mTrans=1.0;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}

void FCCRCtrlDialog::OnBnClickedCheckShowByStep()
{
	// TODO: 在此添加控件通知处理程序代码
	showbystep=!showbystep;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}

void FCCRCtrlDialog::OnButtonReconstruction()
{
	// TODO: 在此添加命令处理程序代码
	thread=AfxBeginThread(ThreadFunc,this);  
}

void FCCRCtrlDialog::OnImprovedFCReconstruction()
{
	thread=AfxBeginThread(ThreadImrovedFC,this);
}

void FCCRCtrlDialog::OnImprovedFCPolyline()
{
	thread=AfxBeginThread(ThreadImrovedFCPoyline,this);
}

void FCCRCtrlDialog::OnImprovedFCFlowComplex()
{
	thread=AfxBeginThread(ThreadImrovedFCFlowComplex,this);
}

void FCCRCtrlDialog::OnImprovedFCSpreadMerge()
{
	thread=AfxBeginThread(ThreadImrovedFCSpreadMerge,this);
}

void FCCRCtrlDialog::OnImprovedFCFindingCycles()
{
	thread=AfxBeginThread(ThreadImrovedFCFindingCycles,this);
}

void FCCRCtrlDialog::OnImprovedFCPruning()
{
	thread=AfxBeginThread(ThreadImrovedFCPruningAndComplete,this);
}

UINT  ThreadFunc(LPVOID pParam) 
{  
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	clock_t start,end;
	clock_t starttotal,endtotal;
	cout<<"********************FC Method***********************"<<endl;
	//polyline
	FCCRCtrlDialog *fc=(FCCRCtrlDialog*)pParam;
	start = clock();
	starttotal=clock();
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
	endtotal=clock();
	cout<<"time for Thicken and Pruning: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	cout<<"total time:"<<(double)(endtotal-starttotal)/CLOCKS_PER_SEC<<endl;
	cout<<"*************************End************************"<<endl;

	fc->fccr.showResult=true;
	fc->fccr.IsProcess=true;
	fc->SetTreePatches();
	::PostMessage(pMain->m_hWnd,WM_RESULT,0,0);
	return 0;
}  

UINT  ThreadImrovedFC(LPVOID pParam) 
{  
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	clock_t start,end;
	clock_t starttotal,endtotal;
	clock_t starttop,endtop;
	cout<<"********************IFC Method**********************"<<endl;
	//polyline
	FCCRCtrlDialog *fc=(FCCRCtrlDialog*)pParam;
	start = clock();
	starttotal=clock();
	fc->fccr.ImprovedPolyline();
	end = clock();
	cout<<"time for polyline: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//delauny and voronoi
	start = clock();
	fc->fccr.OnDelaunyTriangulation();
	end = clock();
	cout<<"time for Delauny: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//flow complex
	start = clock();
	fc->fccr.ImprovedFlowcomplex();
	end = clock();
	cout<<"time for compute Flow Complex: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//thicken
	start = clock();
	starttop=clock();
	fc->fccr.OnCollapse();
	fc->fccr.ImprovedThicken();
	end = clock();
	cout<<"time for Spread And Merge: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//findcycyles
	start = clock();
	fc->fccr.ImprovedFindCyclesForAllPatches();
	end = clock();
	cout<<"time for find cycles for wrong patches: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	//pruning and complete topology
	start = clock();
	fc->fccr.ImprovedPruningAndTopoComplete();
	end = clock();
	cout<<"time for pruning and complete topology: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;

	endtotal=clock();
	endtop=clock();
	cout<<"total time:"<<(double)(endtotal-starttotal)/CLOCKS_PER_SEC<<endl;
	cout<<"topological time:"<<(double)(endtop-starttop)/CLOCKS_PER_SEC<<endl;
	fc->fccr.m_FlowComplex->NonmanifoldCurves();
	cout<<"**************************End***********************"<<endl;
	fc->fccr.showResult=true;
	fc->fccr.IsProcess=true;
	fc->SetTreePatches();
	::PostMessage(pMain->m_hWnd,WM_RESULT_PRUNING,0,0);
	return 0;
}

UINT  ThreadImrovedFCPoyline(LPVOID pParam) 
{  
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	clock_t start,end;
	cout<<"********************IFC Method**********************"<<endl;
	//polyline
	FCCRCtrlDialog *fc=(FCCRCtrlDialog*)pParam;
	start = clock();
	fc->fccr.ImprovedPolyline();
	end = clock();
	cout<<"time for polyline: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	::PostMessage(pMain->m_hWnd,WM_RESULT_POLYLINE,0,0);
	return 0;
}

UINT  ThreadImrovedFCFlowComplex(LPVOID pParam) 
{  
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	clock_t start,end;
	//flow complex
	FCCRCtrlDialog *fc=(FCCRCtrlDialog*)pParam;
	start = clock();
	fc->fccr.OnDelaunyTriangulation();
	fc->fccr.ImprovedFlowcomplex();
	end = clock();
	cout<<"time for compute Flow Complex: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	::PostMessage(pMain->m_hWnd,WM_RESULT_FLOWCOMPLEX,0,0);
	return 0;
}

UINT  ThreadImrovedFCSpreadMerge(LPVOID pParam) 
{  
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	clock_t start,end;
	// topological reconstruction
	FCCRCtrlDialog *fc=(FCCRCtrlDialog*)pParam;
	start = clock();
	fc->fccr.OnCollapse();
	fc->fccr.ImprovedThicken();
	end = clock();
	cout<<"time for Spread And Merge: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	::PostMessage(pMain->m_hWnd,WM_RESULT_MERGE,0,0);
	return 0;
}

UINT  ThreadImrovedFCFindingCycles(LPVOID pParam) 
{  
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	clock_t start,end;
	//findcycyles
	FCCRCtrlDialog *fc=(FCCRCtrlDialog*)pParam;
	start = clock();
	fc->fccr.ImprovedFindCyclesForAllPatches();
	end = clock();
	cout<<"time for find cycles for wrong patches: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	::PostMessage(pMain->m_hWnd,WM_RESULT_CYCLES,0,0);
	return 0;
}

UINT  ThreadImrovedFCPruningAndComplete(LPVOID pParam)
{  
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	clock_t start,end;
	//findcycyles
	FCCRCtrlDialog *fc=(FCCRCtrlDialog*)pParam;
	start = clock();
	fc->fccr.ImprovedPruningAndTopoComplete();
	end = clock();
	cout<<"time for pruning and complete topology: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	fc->fccr.m_FlowComplex->NonmanifoldCurves();
	cout<<"**************************End***********************"<<endl;
	fc->fccr.showResult=true;
	fc->fccr.IsProcess=true;
	fc->SetTreePatches();
	::PostMessage(pMain->m_hWnd,WM_RESULT_PRUNING,0,0);
	return 0;
}

void FCCRCtrlDialog::OnBnClickedShowDarts()
{
	// TODO: 在此添加控件通知处理程序代码
	showdarts=!showdarts;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedButtonPara()
{
	// TODO: 在此添加控件通知处理程序代码
	ParaDialog m_paradialog;
	m_paradialog.fccr=&fccr;
	m_paradialog.DoModal();
}


void FCCRCtrlDialog::OnBnClickedCheckShowInterior()
{
	// TODO: 在此添加控件通知处理程序代码
	showinteriorpatches=!showinteriorpatches;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedCheckShowRMF()
{
	// TODO: 在此添加控件通知处理程序代码
	showRMF=!showRMF;
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


void FCCRCtrlDialog::OnBnClickedShowDelaunay()
{
	// TODO: 在此添加控件通知处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	if(!pView->showFC){
		MessageBox("请先选择FlowComplex");
		((CButton*)GetDlgItem(IDC_CHECK_DELAUNAY))->SetCheck(BST_UNCHECKED);
	}
	else{
		if(!showDelaunay&&pView->showFC)
		{
			showvoids=false;
			showcreators=false;
			((CButton*)GetDlgItem(IDC_CHECK_CREATOR))->SetCheck(BST_UNCHECKED);
			((CButton*)GetDlgItem(IDC_CHECK_VOIDS))->SetCheck(BST_UNCHECKED);
			showDelaunay=true;
		}else
			showDelaunay=false;
	}
	pView->Invalidate();
}
