// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// MainFrm.h : CMainFrame 类的接口
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
	
protected: // 仅从序列化创建
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 特性
public:
	CCtrlPane *m_ctrlPane;
	FCCRCtrlPane *m_ctrlPaneFCCR;
	CUCSEditorPane m_ucsEditor;
	CursorType	   m_cursorType;
	CWinThread* thread;//全局变量
// 操作
public:
	int ShowUCSEditor();
	void SetCursor(CursorType type);
	CursorType GetCursor();
	CUCSEditorPane * GetUCSEditorPane() {return &m_ucsEditor;}
	CCtrlPane *GetCtrlPane() {return m_ctrlPane;}

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // 控件条嵌入成员
	CMFCRibbonBar     m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar  m_wndStatusBar;

// 生成的消息映射函数
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


