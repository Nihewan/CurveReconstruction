#pragma once
#include "FCCR.h"
#include "Resource.h"

// ParaDialog 对话框

class ParaDialog : public CDialogEx
{
	DECLARE_DYNAMIC(ParaDialog)

public:
	FCCR *fccr;
public:
	ParaDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~ParaDialog();
	virtual BOOL OnInitDialog();
// 对话框数据
	enum { IDD = IDD_PARADIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
