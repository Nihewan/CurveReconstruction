#pragma once

#include "resource.h"
#include "afxcmn.h"
#include "afxbutton.h"

class CP_AssembledBody;

// UCSEditor �Ի���

class CUCSEditor : public CDialogEx
{
	DECLARE_DYNAMIC(CUCSEditor)

private:
	int m_alignAxisIndex; // 0 for x-axis; 1 for z-axis

public:
	CUCSEditor(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CUCSEditor();

// �Ի�������
	enum { IDD = IDD_DIALOG_UCSEDIT };

public:
	void	AlignWith(CP_AssembledBody *pAsmbBody, int bodyIndex, int edgeIndex);
	int		GetAlignAxisIndex() { return m_alignAxisIndex;};
	void	CancelAlign();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedCommand1();
	afx_msg void OnBnClickedMfcbuttonAlignx();
	afx_msg void OnBnClickedMfcbuttonAlignz();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBnClickedMfcbuttonAlignok();
};
