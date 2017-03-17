#pragma once
#include "resource.h"
#include "CP_TopologyExt.h"
#include "afxcmn.h"
#include "afxbutton.h"
#include "FCCR.h"
#include "afxwin.h"
// FCCRCtrlDialog �Ի���

UINT ThreadFunc(LPVOID pParam);//FC method
UINT ThreadImrovedFC(LPVOID pParam);//IFC method
UINT ThreadImrovedFCPoyline(LPVOID pParam);//IFC method Poyline
UINT ThreadImrovedFCFlowComplex(LPVOID pParam);//IFC method FlowComplex
UINT ThreadImrovedFCSpreadMerge(LPVOID pParam);//IFC method SpreadMerge
UINT ThreadImrovedFCFindingCycles(LPVOID pParam);//IFC method FindingCycles
UINT ThreadImrovedFCPruningAndComplete(LPVOID pParam);//IFC method PruningAndComplete
#define WM_RESULT WM_USER+100
#define WM_RESULT_FCPLAY WM_USER+101
#define WM_RESULT_REFRESH WM_USER+102
#define WM_RESULT_POLYLINE WM_USER+103
#define WM_RESULT_FLOWCOMPLEX WM_USER+104
#define WM_RESULT_MERGE WM_USER+105
#define WM_RESULT_CYCLES WM_USER+106
#define WM_RESULT_PRUNING WM_USER+107

class FCCRCtrlDialog : public CDialogEx
{
	DECLARE_DYNAMIC(FCCRCtrlDialog)
public:
	CWinThread* thread;//ȫ�ֱ���
	CWinThread* pFCthread;//ȫ�ֱ���
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
	int sel2cell;
	int seltriangle;
	int selpoly;
	int selpatch;
	bool showvoids;
	bool showDelaunay;
	bool showcircum;
	bool showbystep;
	bool showdarts;
	bool showinteriorpatches;
	bool showRMF;
	double mTrans; 
public:
	FCCRCtrlDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~FCCRCtrlDialog();

// �Ի�������
	enum { IDD = IDD_DIALOG_CTRL_FCCR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
public:
	void SetItems(CP_FlowComplex *FlowComplex,vector<CP_PolyLine3D> *VT_PolyLine);
	void SetTreeItems(int _2cell);//��ʾѡ��2cell������������
	void SetTreePatch(int _patch);//��ʾѡ��patch������2cells
	void SetTreeCreator();
	void SetTree3cells();
	void SetTreePatches();
	void OnButtonReconstruction();

	void OnImprovedFCReconstruction();
	void OnImprovedFCPolyline();
	void OnImprovedFCFlowComplex();
	void OnImprovedFCSpreadMerge();
	void OnImprovedFCFindingCycles();
	void OnImprovedFCPruning();
	void SmoothCurves(vector<CP_PolyLine3D>* VT_PolyLine);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelect2cell();
	afx_msg void OnBnClickedRandomColor();
	afx_msg void OnBnClickedColorByDistance();
	afx_msg void OnBnClickedCheckTri();
	afx_msg void OnBnClickedCheck2cell();
	afx_msg void OnBnClickedClear();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCheckPolyline();
	afx_msg void OnBnClickedCheckCreator();
	afx_msg void OnTvnSelchangedInfotree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckVoids();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheckPatch();
	afx_msg void OnBnClickedCheckCircum();
	afx_msg void OnBnClickedCheckShowByStep();
	afx_msg void OnBnClickedShowDarts();
	afx_msg void OnBnClickedButtonPara();
	afx_msg void OnBnClickedCheckShowInterior();
	afx_msg void OnBnClickedCheckShowRMF();
	afx_msg void OnBnClickedShowDelaunay();
};
