#pragma once
#include "resource.h"
#include "CP_TopologyExt.h"
#include "afxcmn.h"
#include "afxbutton.h"
#include "FCCR.h"
#include "afxwin.h"
// FCCRCtrlDialog �Ի���

class FCCRCtrlDialog : public CDialogEx
{
	DECLARE_DYNAMIC(FCCRCtrlDialog)
public:
	FCCR fccr;
	CTreeCtrl m_treeCtrl;
	bool selectEnable;
	bool randomColor;
	bool colorByDis;
	bool triboundary;
	bool _2cellboundary;
	int showNum;
public:
	FCCRCtrlDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~FCCRCtrlDialog();

// �Ի�������
	enum { IDD = IDD_DIALOG_CTRL_FCCR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
public:
	void SetItems(CP_FlowComplex *FlowComplex,vector<CP_PolyLine3D> *VT_PolyLine);
	void SetTreeItems(int _2cell);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelectTriangle();
	afx_msg void OnBnClickedRandomColor();
	afx_msg void OnBnClickedColorByDistance();
	afx_msg void OnBnClickedCheckTri();
	afx_msg void OnBnClickedCheck2cell();
	afx_msg void OnBnClickedSearch();
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnEditchangeCombovoid();
	CComboBox m_CComboBox;
};
