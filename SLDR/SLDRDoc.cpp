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

// SLDRDoc.cpp : CSLDRDoc ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "SLDR.h"
#endif

#include "SLDRDoc.h"
#include "CFileReader.h"
#include "MainFrm.h"
#include "SLDRView.h"
#include <fstream>

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSLDRDoc

IMPLEMENT_DYNCREATE(CSLDRDoc, CDocument)

BEGIN_MESSAGE_MAP(CSLDRDoc, CDocument)
END_MESSAGE_MAP()


// CSLDRDoc ����/����

CSLDRDoc::CSLDRDoc()
{
	// TODO: �ڴ����һ���Թ������
	m_pAsmbBody = NULL;
}

CSLDRDoc::~CSLDRDoc()
{
	if (m_pAsmbBody != NULL) {
		delete m_pAsmbBody;
		m_pAsmbBody = NULL;
	}
}

BOOL CSLDRDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)

	return TRUE;
}




// CSLDRDoc ���л�

void CSLDRDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �ڴ���Ӵ洢����
	}
	else
	{
		// TODO: �ڴ���Ӽ��ش���
	}
}

#ifdef SHARED_HANDLERS

// ����ͼ��֧��
void CSLDRDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// �޸Ĵ˴����Ի����ĵ�����
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// ������������֧��
void CSLDRDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ���ĵ����������������ݡ�
	// ���ݲ���Ӧ�ɡ�;���ָ�

	// ����:  strSearchContent = _T("point;rectangle;circle;ole object;")��
	SetSearchContent(strSearchContent);
}

void CSLDRDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CSLDRDoc ���

#ifdef _DEBUG
void CSLDRDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSLDRDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CSLDRDoc ����


BOOL CSLDRDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  �ڴ������ר�õĴ�������
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRView *pView = (CSLDRView *)(pMain->GetActiveView());
	if (m_pAsmbBody != NULL)
	{
		delete m_pAsmbBody;
		m_pAsmbBody = NULL;
		pView->ReSet();
		if(format=="dxf")
			pMain->m_ctrlPane->ShowPane(false,false,false);
		else{
			delete m_FlowComplex;
			m_FlowComplex=NULL;
			delete VT_PolyLine;
			VT_PolyLine=NULL;
			pMain->m_ctrlPaneFCCR->ShowPane(false,false,false);		
		}
	}

	m_pAsmbBody = new CP_AssembledBody;
	CP_Body *pBody = new CP_BodyExt;
	m_pAsmbBody->AddBody(pBody);

	CString fname;
	fname.Format(_T("%s"),lpszPathName);
	if (fname.Right(3).CompareNoCase(_T("dxf")) == 0)
	{
		format = "dxf";
		filename =fname;

		CFileReader::ReadDXF(lpszPathName, pBody);

		m_boundBox.InitBoundBox(pBody);
		
		// Init control pane
		pMain->m_ctrlPane=new CCtrlPane();
		pMain->m_ctrlPane->Create(_T("Ctrl Pane"), pMain, CRect(0, 0, 300, 300), TRUE, IDD_DIALOG_CTRL,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI);
		pMain->m_ctrlPane->EnableDocking(CBRS_ALIGN_ANY);
		pMain->EnableDocking(CBRS_ALIGN_ANY);
		pMain->DockPane(pMain->m_ctrlPane);

		CRect rect;
		pView->GetClientRect(&rect);
		pView->OnSize(0, rect.Width(), rect.Height());

		m_selectedItem.Reset();
		m_mouseSelEdge[0] = -1;
		m_mouseSelEdge[0] = -1;
		pMain->m_ctrlPane->m_dialog.SetTreeItems(m_pAsmbBody);
		pMain->m_ctrlPane->m_dialog.SetCurrentStep(STEP_FACE_IDENTIFICATION);
		
		pView->Invalidate();
	}
	else if(fname.Right(3).CompareNoCase(_T("rve")) == 0)
	{
		format = "curve";
		filename =fname;
		VT_PolyLine=new vector<CP_PolyLine3D>;
		m_FlowComplex=new CP_FlowComplex();
		CFileReader::ReadCURVE(lpszPathName, VT_PolyLine);

		// Init control pane
		pMain->m_ctrlPaneFCCR=new FCCRCtrlPane();
		pMain->m_ctrlPaneFCCR->Create(_T("Ctrl Pane"), pMain, CRect(0, 0, 300, 300), TRUE, IDD_DIALOG_CTRL_FCCR,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI);
		pMain->m_ctrlPaneFCCR->EnableDocking(CBRS_ALIGN_ANY);
		pMain->EnableDocking(CBRS_ALIGN_ANY);
		pMain->DockPane(pMain->m_ctrlPaneFCCR);

		pMain->m_ctrlPaneFCCR->m_dialog.SetItems(m_FlowComplex,VT_PolyLine);
		pView->format="curve";
		pView->showvoid=0;
		pView->m_Scale = 15.0f;
		pView->Invalidate();

	}
//	else if(fname.Right(3).CompareNoCase(_T("mdl")) == 0)
//		m_pBody = ReadLDR(lpszPathName,1);
	else
	{
		MessageBox(NULL, _T("�����ļ���ʽ����"), _T("ERROR"), 0);
		delete pBody;
		pBody = NULL;
		return TRUE;
	}

	return TRUE;
}


void CSLDRDoc::OutputCurveNetwork()
{
	CP_Body *pBody = m_pAsmbBody->GetOriginalBody();
	CString filter = "�ļ� (*.txt)|*.txt||";
	CFileDialog openFileDlg(false, NULL, NULL, OFN_HIDEREADONLY, filter);
	INT_PTR result = openFileDlg.DoModal();  
	if(result == IDOK) { 
		CString filePath = openFileDlg.GetPathName();  
		if (filePath.Find(_T(".txt")) != filePath.GetLength() - 4)
			filePath += _T(".txt");
		std::ofstream os;
		os.open(filePath);
		for (unsigned int i = 0; i < pBody->GetEdgeNumber(); ++i)
		{
			CP_Edge *pEdge = pBody->GetEdge(i);
			//os <<(LPCTSTR)pEdge->m_pCurve3D->ToString() <<endl;
		}
	}
}