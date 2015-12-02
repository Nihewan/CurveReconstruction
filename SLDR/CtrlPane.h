#pragma once
#include "CTRLDialog.h"

class CCtrlPane: public CDockablePane
{
	DECLARE_DYNAMIC(CCtrlPane)

public:
	CCtrlPane(void);
	virtual ~CCtrlPane(void);

protected:
	DECLARE_MESSAGE_MAP()

public:
	CCtrlDialog m_dialog;
public:
	CCtrlDialog* GetDialog() {return &m_dialog;}
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
};

