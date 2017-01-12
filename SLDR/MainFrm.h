// ��� MFC ʾ��Դ������ʾ���ʹ�� MFC Microsoft Office Fluent �û����� 
// (��Fluent UI��)����ʾ�������ο���
// ���Բ��䡶Microsoft ������ο����� 
// MFC C++ ������渽����ص����ĵ���
// ���ơ�ʹ�û�ַ� Fluent UI ����������ǵ����ṩ�ġ�
// ��Ҫ�˽��й� Fluent UI ��ɼƻ�����ϸ��Ϣ�������  
// http://msdn.microsoft.com/officeui��
//
// ��Ȩ����(C) Microsoft Corporation
// ��������Ȩ����

// MainFrm.h : CMainFrame ��Ľӿ�
//

#pragma once
#include "CtrlPane.h"
#include "FCCRCtrlPane.h"
#include "UCSEditorPane.h"
#include "SLDRDoc.h"
#include <iostream>
using namespace std;

class CMainFrame : public CFrameWndEx
{
	
protected: // �������л�����
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// ����
public:
	CCtrlPane *m_ctrlPane;
	FCCRCtrlPane *m_ctrlPaneFCCR;
	CUCSEditorPane m_ucsEditor;
	CursorType	   m_cursorType;
	CWinThread* thread;//ȫ�ֱ���
// ����
public:
	int ShowUCSEditor();
	void SetCursor(CursorType type);
	CursorType GetCursor();
	CUCSEditorPane * GetUCSEditorPane() {return &m_ucsEditor;}
	CCtrlPane *GetCtrlPane() {return m_ctrlPane;}

// ��д
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// ʵ��
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // �ؼ���Ƕ���Ա
	CMFCRibbonBar     m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar  m_wndStatusBar;

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCbShowcp();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnButtonSavecnw();
	afx_msg LRESULT  ShowResult(WPARAM wParam,LPARAM  lParam);
	afx_msg LRESULT  ShowPolyline(WPARAM wParam,LPARAM  lParam);
	afx_msg LRESULT  ShowFlowComplex(WPARAM wParam,LPARAM  lParam);
	afx_msg LRESULT  ShowMerge(WPARAM wParam,LPARAM  lParam);
	afx_msg LRESULT  ShowCycles(WPARAM wParam,LPARAM  lParam);
	afx_msg LRESULT  ShowPruning(WPARAM wParam,LPARAM  lParam);
	afx_msg LRESULT  ResultRefresh(WPARAM wParam,LPARAM  lParam);
};


