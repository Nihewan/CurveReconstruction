// ParaDialog.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ParaDialog.h"
#include "afxdialogex.h"
#include "CP_FlowComplex.h"

// ParaDialog �Ի���

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


// ParaDialog ��Ϣ�������
BOOL ParaDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	GetDlgItem(IDC_EDIT_SAMPLE)->SetWindowText("0.045");
	GetDlgItem(IDC_EDIT_FEASA)->SetWindowText("0.7");
	GetDlgItem(IDC_EDIT_VOIDS)->SetWindowText("0");
	GetDlgItem(IDC_BI_WEIGHT)->SetWindowText("0");
	GetDlgItem(IDC_PER_WEIGHT)->SetWindowText("1");
	GetDlgItem(IDC_REFINE_ITER)->SetWindowText("4");
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


void ParaDialog::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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

