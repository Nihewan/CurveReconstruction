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

// SLDRView.h : CSLDRView ��Ľӿ�
//

#pragma once

//#include "gl/GL.h"
//#include "gl/glew.h"
#include "gl/GLU.h"
#include "glut.h"

#include "CP_PointVector.h"
#include "CBoundBox2D.h"
#include <string>
class CSLDRDoc;

class CSLDRView : public CView
{
protected: // �������л�����
	CSLDRView();
	DECLARE_DYNCREATE(CSLDRView)

public:
	// A handle to an OpenGL rendering context
	HGLRC m_hRC;
	
	// Rotate value: 0 around x, 1 around y
	int m_rotate[2];	
//	Delaunay T;

// ����
public:
	CSLDRDoc* GetDocument() const;
	int swidth, sheight;
	float aspect_ratio ;
	float m_Scale;
	GLfloat m_xAngle;
	GLfloat m_yAngle;
	GLfloat m_xPos;
	GLfloat m_yPos;
	CPoint m_MouseDownPoint;
	CString format;
	bool IsDelauny;
	bool IsPoly;
	bool IsFC;
	int selectedTriangle;
	int selectedPolyline;
	bool ctrlDown;
	bool zDown;
// ����
public:
	void ReSet();
	void InitOpenGL();
	void InitMaterial();
	void InitLight0();
	void InitLight01();
	void InitLight02();
	void InitLight1();
	void InitLight2();
	void InitLight3();
	void FindTriangle(CPoint point); 
	void FindSelNode(CPoint point);
	void FindPolyLine(CPoint point);
	void ProcessHits(GLint hits, GLuint buffer[]);
	void SetProjectionMatrix(int cx, int cy);
	void SetModelViewMatrix() ;
	void  drawScene();
	void drawString(const char* str);
	void drawData();
	void drawMeshTri(CP_Triganle3D* pTri);
// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CSLDRView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPolyline();
	afx_msg void OnDelauny();
	afx_msg void OnFlowcomplex();
	afx_msg void OnCollapse();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // SLDRView.cpp �еĵ��԰汾
inline CSLDRDoc* CSLDRView::GetDocument() const
   { return reinterpret_cast<CSLDRDoc*>(m_pDocument); }
#endif

