#pragma once
#include "resource.h"
#include "CP_TopologyExt.h"
#include "afxcmn.h"
#include "afxbutton.h"
#include "FCCR.h"
#include "afxwin.h"
// FCCRCtrlDialog 对话框

class FCCRCtrlDialog : public CDialogEx
{
	DECLARE_DYNAMIC(FCCRCtrlDialog)
public:
	FCCR fccr;
	CTreeCtrl m_treeCtrl;
	bool select2cellEnable;
	bool selectpatchEnable;
	bool randomColor;
	bool colorByDis;
	bool triboundary;
	bool _2cellboundary;
	bool patchboundary;
	bool edgeByDegree;
	bool showpoly;
	bool showcreators;
	int selcreator;
	int sel3cell;
	bool showvoids;
	double mTrans; 
public:
	FCCRCtrlDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~FCCRCtrlDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_CTRL_FCCR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
public:
	void SetItems(CP_FlowComplex *FlowComplex,vector<CP_PolyLine3D> *VT_PolyLine);
	void SetTreeItems(int _2cell);
	void SetTreePatch(int _patch);
	void SetTreeCreator();
	void SetTree3cells();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelect2cell();
	afx_msg void OnBnClickedRandomColor();
	afx_msg void OnBnClickedColorByDistance();
	afx_msg void OnBnClickedCheckTri();
	afx_msg void OnBnClickedCheck2cell();
	afx_msg void OnBnClickedClear();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEditVoids();
	afx_msg void OnEnChangeEditEpsilon();
	afx_msg void OnEnChangeEditFeasa();
	afx_msg void OnBnClickedCheckPolyline();
	afx_msg void OnBnClickedCheckCreator();
	afx_msg void OnTvnSelchangedInfotree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckVoids();
	afx_msg void OnNMCustomdrawSliderTansparent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheckPatch();
};
