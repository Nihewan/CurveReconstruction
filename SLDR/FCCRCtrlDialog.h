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
UINT ThreadPlayFC(LPVOID pParam);//FC����
#define WM_RESULT WM_USER+100
#define WM_RESULT_FCPLAY WM_USER+101
#define WM_RESULT_REFRESH WM_USER+102

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
	int selcycle;
	bool showvoids;
	bool showcircum;
	bool showbystep;
	bool showdarts;
	double mTrans; 
	int play;
	int pos;
	bool pause;
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
	void SetTreeCycles();
	void OnButtonReconstruction();
	void OnImprovedFCReconstruction();
	void OnPlayFC();
	void OnPlayFCPause();
	void OnIFCMethodNext();
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
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheckPatch();
	afx_msg void OnBnClickedCheckCircum();
	afx_msg void OnBnClickedCheckShowByStep();
	afx_msg void OnBnClickedShowDarts();
};
