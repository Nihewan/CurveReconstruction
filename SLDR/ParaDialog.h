#pragma once
#include "FCCR.h"
#include "Resource.h"

// ParaDialog �Ի���

class ParaDialog : public CDialogEx
{
	DECLARE_DYNAMIC(ParaDialog)

public:
	FCCR *fccr;
public:
	ParaDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~ParaDialog();
	virtual BOOL OnInitDialog();
// �Ի�������
	enum { IDD = IDD_PARADIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
