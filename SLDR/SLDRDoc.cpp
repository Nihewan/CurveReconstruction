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

// SLDRDoc.cpp : CSLDRDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "SLDR.h"
#endif

#include "SLDRDoc.h"
#include "CFileReader.h"
#include "MainFrm.h"
#include "SLDRView.h"
#include "CP_FlowComplex.h"
#include <fstream>

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSLDRDoc

IMPLEMENT_DYNCREATE(CSLDRDoc, CDocument)

BEGIN_MESSAGE_MAP(CSLDRDoc, CDocument)
END_MESSAGE_MAP()


// CSLDRDoc 构造/析构

CSLDRDoc::CSLDRDoc()
{
	// TODO: 在此添加一次性构造代码
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

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)
	r[0]=0.9;g[0]=0.5;b[0]=0.5;//红
	r[1]=0.65;g[1]=0.72;b[1]=0.22;//绿
	r[2]=0.84;g[2]=0.74;b[2]=0.58;//杏仁
	r[3]=0.6;g[3]=0.4;b[3]=0.7;//紫
	r[4]=0.81;g[4]=0.6;b[4]=0.01;//土黄
	r[5]=0.35;g[5]=0.96;b[5]=0.5;//草绿
	return TRUE;
}

// CSLDRDoc 序列化

void CSLDRDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CSLDRDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
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

// 搜索处理程序的支持
void CSLDRDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:  strSearchContent = _T("point;rectangle;circle;ole object;")；
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

// CSLDRDoc 诊断

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


// CSLDRDoc 命令


BOOL CSLDRDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  在此添加您专用的创建代码
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

		pMain->m_ucsEditor.Create(_T("UCS Editor"), pMain, CRect(300, 0, 600, 300), TRUE, IDD_DIALOG_UCSEDIT,
			WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI);
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
		pView->m_Scale = 15.0f;
		pView->Invalidate();

	}
//	else if(fname.Right(3).CompareNoCase(_T("mdl")) == 0)
//		m_pBody = ReadLDR(lpszPathName,1);
	else
	{
		MessageBox(NULL, _T("输入文件格式错误"), _T("ERROR"), 0);
		delete pBody;
		pBody = NULL;
		return TRUE;
	}

	return TRUE;
}


void CSLDRDoc::OutputCurveNetwork()
{
	if(format=="dxf"){
		CP_Body *pBody = m_pAsmbBody->GetOriginalBody();
		CString filter = "文件 (*.txt)|*.txt||";
		CFileDialog openFileDlg(false, NULL, NULL, OFN_HIDEREADONLY, filter);
		INT_PTR result = openFileDlg.DoModal();  
		if(result == IDOK) { 
			CString filePath = openFileDlg.GetPathName();  
			if (filePath.Find(_T(".txt")) != filePath.GetLength() - 4)
				filePath += _T(".txt");
			std::ofstream os;
			os.open(filePath);
			for (int i = 0; i < pBody->GetEdgeNumber(); ++i)
			{
				CP_Edge *pEdge = pBody->GetEdge(i);
				//os <<(LPCTSTR)pEdge->m_pCurve3D->ToString() <<endl;
			}
		}
	}//dxf
	else if(format=="curve")
	{
		CString filter = "文件 (*.obj)|*.obj||";
		CFileDialog openFileDlg(false, NULL, NULL, OFN_HIDEREADONLY, filter);
		INT_PTR result = openFileDlg.DoModal();  
		if(result == IDOK) { 
			CString filePath = openFileDlg.GetPathName();  
			if (filePath.Find(_T(".obj")) != filePath.GetLength() - 4)
				filePath += _T(".obj");
			std::ofstream out;
			out.open(filePath);
			out<<"# Vertices:"<<m_FlowComplex->m_0cells.size()<<endl;
			out<<"# Faces:"<<m_FlowComplex->tricells.size()<<endl;
			for(unsigned int i=0;i<m_FlowComplex->m_0cells.size();i++)
			{
				out<<"v "<<m_FlowComplex->m_0cells[i].m_x<<" "<<m_FlowComplex->m_0cells[i].m_y<<" "<<m_FlowComplex->m_0cells[i].m_z<<endl;
			}
			out<<"############"<<endl;

			for(unsigned int i=0;i<m_FlowComplex->m_patches.size();i++){
				CP_Patch *pPatch = m_FlowComplex->m_patches[i];
				if(pPatch->flag){
					for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++){
						CP_2cell *p2cell = m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
						for(unsigned int k=0;k<p2cell->m_triangle.size();k++){
							CP_Triganle3D *pTri = m_FlowComplex->tricells[p2cell->m_triangle[k]];
							out<<"f "<< pTri->m_points[0]+1<<" "<<pTri->m_points[1]+1<<" "<<pTri->m_points[2]+1<<endl;
						}//k
					}
				}
			}
		}
	}//curve
}