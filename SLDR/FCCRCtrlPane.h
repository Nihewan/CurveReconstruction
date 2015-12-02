#pragma once
#include "FCCRCtrlDialog.h"

class FCCRCtrlPane: public CDockablePane
{
	DECLARE_DYNAMIC(FCCRCtrlPane)

public:
	FCCRCtrlPane(void);
	virtual ~FCCRCtrlPane(void);

protected:
	DECLARE_MESSAGE_MAP()

public:
	FCCRCtrlDialog m_dialog;
public:
	FCCRCtrlDialog* GetDialog() {return &m_dialog;}
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
};

