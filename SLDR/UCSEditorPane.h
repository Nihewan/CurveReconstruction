
#pragma once
#include "UCSEditor.h"

enum CursorType {
	NORMAL, SEL, FOUND 
};

class CUCSEditorPane: public CDockablePane
{
	DECLARE_DYNAMIC(CUCSEditorPane)

public:
	CUCSEditorPane(void);
	virtual ~CUCSEditorPane(void);

protected:
	DECLARE_MESSAGE_MAP()

private:
	CUCSEditor m_dialog;

public:
	CUCSEditor* GetDialog() {return &m_dialog;}

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
};
