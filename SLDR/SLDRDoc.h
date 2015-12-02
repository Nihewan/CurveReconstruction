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

// SLDRDoc.h : CSLDRDoc ��Ľӿ�
//


#pragma once
#include "Cp_TopologyExt.h"
#include "CBoundBox2D.h"
#include "CTreeSelectedItem.h"
#include "CP_FlowComplex.h"
class CSLDRDoc : public CDocument
{
protected: // �������л�����
	CSLDRDoc();
	DECLARE_DYNCREATE(CSLDRDoc)

// ����
public:
	//SLDR
	CP_AssembledBody *m_pAsmbBody;
	CBoundBox2D m_boundBox;
	CTreeSelectedItem m_selectedItem;
	int			m_mouseSelEdge[2]; // body index and edge index
	//FCCR
	CString format;
	CString filename;
	CP_FlowComplex *m_FlowComplex;
	vector<CP_PolyLine3D> *VT_PolyLine;
	vector<CP_Arc> VT_Arc;
	vector<CP_LineSegment3D> VT_LineSegment;
	vector<CP_Nurbs> VT_Nurbs;
// ����
public:
	void OutputCurveNetwork();

// ��д
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// ʵ��
public:
	virtual ~CSLDRDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// ����Ϊ����������������������ݵ� Helper ����
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

};
