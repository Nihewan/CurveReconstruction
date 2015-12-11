// FCCRCtrlDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "SLDR.h"
#include "MainFrm.h"
#include "SLDRView.h"
#include "SLDRDoc.h"
#include "FCCRCtrlDialog.h"
#include "afxdialogex.h"
// FCCRCtrlDialog 对话框

IMPLEMENT_DYNAMIC(FCCRCtrlDialog, CDialogEx)

FCCRCtrlDialog::FCCRCtrlDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(FCCRCtrlDialog::IDD, pParent)
{
	selectEnable=false;
	randomColor=false;
	colorByDis=false;
	triboundary=false;
	_2cellboundary=false;
	showNum=-1;
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
	ON_BN_CLICKED(IDC_SELECT_TRIANGLE, &FCCRCtrlDialog::OnBnClickedSelectTriangle)
	ON_BN_CLICKED(IDC_CHECK1, &FCCRCtrlDialog::OnBnClickedRandomColor)
	ON_BN_CLICKED(IDC_CHECK2, &FCCRCtrlDialog::OnBnClickedColorByDistance)
	ON_BN_CLICKED(IDC_CHECK_TRI, &FCCRCtrlDialog::OnBnClickedCheckTri)
	ON_BN_CLICKED(IDC_CHECK_2CELL, &FCCRCtrlDialog::OnBnClickedCheck2cell)
	ON_BN_CLICKED(IDC_SEARCH, &FCCRCtrlDialog::OnBnClickedSearch)
	ON_EN_CHANGE(IDC_EDIT_VOIDS, &FCCRCtrlDialog::OnEnChangeEditVoids)
	ON_EN_CHANGE(IDC_EDIT_EPSILON, &FCCRCtrlDialog::OnEnChangeEditEpsilon)
END_MESSAGE_MAP()


// FCCRCtrlDialog 消息处理程序



void FCCRCtrlDialog::SetItems(CP_FlowComplex *FlowComplex,vector<CP_PolyLine3D> *VT_PolyLine)
{
	fccr.m_FlowComplex=FlowComplex;
	fccr.m_VT_PolyLine=VT_PolyLine;
}

void FCCRCtrlDialog::SetTreeItems(int _2cell)
{
	CP_2cell *m_2cell=fccr.m_FlowComplex->m_2cells[_2cell];
	CString cstr;
	m_treeCtrl.DeleteAllItems(); 

	cstr.Format("2cell: %d",_2cell);//2cell在m_2cell中的index
	HTREEITEM h2cell=m_treeCtrl.InsertItem(cstr);

	for(int i=0;i<m_2cell->m_triangle.size();i++)
	{
		cstr.Format("triangle : %d",m_2cell->m_triangle[i]);
		HTREEITEM htri = m_treeCtrl.InsertItem(cstr,h2cell);
		CP_Triganle3D *ptrTri=fccr.m_FlowComplex->tricells[m_2cell->m_triangle[i]];
		for(int j=0;j<3;j++)
		{
			cstr.Format("vertices : %d",ptrTri->m_points[j]);
			HTREEITEM hpoint = m_treeCtrl.InsertItem(cstr,htri);

			cstr.Format("x: %f",fccr.m_FlowComplex->m_0cells[ptrTri->m_points[j]].m_x);
			HTREEITEM hVert_x = m_treeCtrl.InsertItem(cstr,hpoint);
			cstr.Format("y: %f",fccr.m_FlowComplex->m_0cells[ptrTri->m_points[j]].m_y);
			HTREEITEM hVert_y = m_treeCtrl.InsertItem(cstr,hpoint);
			cstr.Format("z: %f",fccr.m_FlowComplex->m_0cells[ptrTri->m_points[j]].m_z);
			HTREEITEM hVert_z = m_treeCtrl.InsertItem(cstr,hpoint);
		}//j
	}//i
	m_treeCtrl.Expand(h2cell,TVE_EXPAND);
}

void FCCRCtrlDialog::OnBnClickedSelectTriangle()
{
	// TODO: 在此添加控件通知处理程序代码
	if(selectEnable)
		selectEnable=false;
	else
		selectEnable=true;
}


void FCCRCtrlDialog::OnBnClickedRandomColor()
{
	// TODO: 在此添加控件通知处理程序代码
	if(randomColor)
		randomColor=false;
	else
		randomColor=true;
}


void FCCRCtrlDialog::OnBnClickedColorByDistance()
{
	// TODO: 在此添加控件通知处理程序代码
	if(colorByDis)
		colorByDis=false;
	else
		colorByDis=true;

}


void FCCRCtrlDialog::OnBnClickedCheckTri()
{
	// TODO: 在此添加控件通知处理程序代码
	if(triboundary)
		triboundary=false;
	else
		triboundary=true;
}


void FCCRCtrlDialog::OnBnClickedCheck2cell()
{
	// TODO: 在此添加控件通知处理程序代码
	if(_2cellboundary)
		_2cellboundary=false;
	else
		_2cellboundary=true;
}


void FCCRCtrlDialog::OnBnClickedSearch()
{
	// TODO: 在此添加控件通知处理程序代码
	CEdit* pBoxOne;
	pBoxOne = (CEdit*) GetDlgItem(IDC_SHOWNUM);
	CString str;
	pBoxOne-> GetWindowText(str);
	showNum=_ttoi(str);

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView * pView = (CSLDRView *)(pMain->GetActiveView());
	pView->Invalidate();
}


BOOL FCCRCtrlDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	GetDlgItem(IDC_EDIT_EPSILON)->SetWindowText("0.055");
	GetDlgItem(IDC_EDIT_VOIDS)->SetWindowText("0");
	GetDlgItem(IDC_COMBO_SHOWCELL)->SetWindowText("2cell");
	GetDlgItem(IDC_SHOWNUM)->SetWindowText("0");
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void FCCRCtrlDialog::OnEnChangeEditVoids()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;   
	GetDlgItemText(IDC_EDIT_VOIDS,str);
	fccr.voids=_ttoi(str);
}


void FCCRCtrlDialog::OnEnChangeEditEpsilon()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;   
	GetDlgItemText(IDC_EDIT_EPSILON,str);
	fccr.epsilon=atof(str);
}
