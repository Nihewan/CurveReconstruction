// ParaDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ParaDialog.h"
#include "afxdialogex.h"
#include "CP_FlowComplex.h"

// ParaDialog 对话框

IMPLEMENT_DYNAMIC(ParaDialog, CDialogEx)

ParaDialog::ParaDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(ParaDialog::IDD, pParent)
{

}

ParaDialog::~ParaDialog()
{
}

void ParaDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ParaDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &ParaDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// ParaDialog 消息处理程序
BOOL ParaDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	GetDlgItem(IDC_EDIT_SAMPLE)->SetWindowText("0.045");
	GetDlgItem(IDC_EDIT_FEASA)->SetWindowText("0.7");
	GetDlgItem(IDC_EDIT_VOIDS)->SetWindowText("0");
	GetDlgItem(IDC_BI_WEIGHT)->SetWindowText("0");
	GetDlgItem(IDC_PER_WEIGHT)->SetWindowText("1");
	GetDlgItem(IDC_REFINE_ITER)->SetWindowText("4");
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void ParaDialog::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();

	CString str;   
	GetDlgItemText(IDC_EDIT_VOIDS,str);
	fccr->voids=_ttoi(str);

	GetDlgItemText(IDC_EDIT_SAMPLE,str);
	fccr->epsilon=atof(str);

	GetDlgItemText(IDC_EDIT_FEASA,str);
	fccr->feasa=atof(str);

	GetDlgItemText(IDC_BI_WEIGHT,str);
	fccr->m_FlowComplex->biweight=_ttoi(str);

	GetDlgItemText(IDC_PER_WEIGHT,str);
	fccr->m_FlowComplex->perweight=_ttoi(str);

	GetDlgItemText(IDC_REFINE_ITER,str);
	fccr->surface_optimization.refine_max_iter=_ttoi(str);
}

