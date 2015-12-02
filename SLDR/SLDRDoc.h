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

// SLDRDoc.h : CSLDRDoc 类的接口
//


#pragma once
#include "Cp_TopologyExt.h"
#include "CBoundBox2D.h"
#include "CTreeSelectedItem.h"
#include "CP_FlowComplex.h"
class CSLDRDoc : public CDocument
{
protected: // 仅从序列化创建
	CSLDRDoc();
	DECLARE_DYNCREATE(CSLDRDoc)

// 特性
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
// 操作
public:
	void OutputCurveNetwork();

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CSLDRDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

};
