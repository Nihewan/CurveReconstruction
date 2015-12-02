#pragma once
#include "resource.h"
#include "CP_TopologyExt.h"
#include "afxcmn.h"
#include "afxbutton.h"

// CCTRLDialog �Ի���

class CCtrlDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CCtrlDialog)

public:
	CCtrlDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCtrlDialog();

// �Ի�������
	enum { IDD = IDD_DIALOG_CTRL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	void SetTreeItems(CP_AssembledBody *pAsmbBody);
	void SetCurrentStep(int step);

public:
	CP_AssembledBody *m_pAsmbBody;

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

public:
	CTreeCtrl m_bodyEleTree;
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
	CMFCButton m_btnFaceIdent;
	afx_msg void OnBnClickedMfcbuttonFaceident();
	afx_msg void OnBnClickedMfcbuttonCut();
	afx_msg void OnBnClickedMfcbuttonFacecls();
	afx_msg void OnBnClickedMfcbuttonReconstruction();
	afx_msg void OnBnClickedMfcbuttonDepthestim();
	afx_msg void OnBnClickedMfcbuttonObtainucs();
};
