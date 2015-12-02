#include "stdafx.h"
#include <typeinfo>
#include "CObjectFactory.h"
#include "CCopyPool.h"
#include "propertiesnames.h"

/************************************************************************/
/* CVertexCopyPool                                                      */
/************************************************************************/

CP_VertexExt* CVertexCopyPool::CopyVertex2Pool( CP_Vertex *pVt )
{
	Map_SrcCopy::iterator it = m_pool.find(pVt);
	if (it != m_pool.end()) {
		return (CP_VertexExt *)it->second;
	}
	CP_VertexExt *pNewVt = new CP_VertexExt(new CP_Point3D(*pVt->m_pPoint));
	pNewVt->SetParent(static_cast<CP_VertexExt*>(pVt)->GetParent());
	m_pool.insert(pair<CObject *, CObject *>(pVt, pNewVt));
	return pNewVt;
}

void CVertexCopyPool::AddVertices2Body(CP_Body *pBody) {
	Map_SrcCopy::iterator it = m_pool.begin();
	while (it != m_pool.end())
	{
		pBody->AddVertex((CP_Vertex *)it->second);
		it++;
	}
}

/************************************************************************/
/* CEdgeCopyPool                                                        */
/************************************************************************/

CP_EdgeExt* CEdgeCopyPool::CopyEdge2Pool(CP_Edge *pEdge, CP_Vertex *pStartVt, CP_Vertex *pEndVt) {
	Map_SrcCopy::iterator it = m_pool.find(pEdge);
	if (it != m_pool.end()) {
		return (CP_EdgeExt *)it->second;
	}
	const char* strClassName = typeid(*(pEdge->m_pCurve3D)).name();

	CP_Curve3D *pNewCurve = (CP_Curve3D *)CObjectFactory::CopyObject(strClassName, *(pEdge->m_pCurve3D));
	CP_EdgeExt *pNewEdge = new CP_EdgeExt(pNewCurve, pStartVt, pEndVt);
	pNewEdge->SetParent(static_cast<CP_EdgeExt *>(pEdge)->GetParent());
	if (static_cast<CP_EdgeExt*>(pEdge)->HasProperty(PROPERTY_NAME_ADDITIONAL_LINE))
		pNewEdge->SetProperties(PROPERTY_NAME_ADDITIONAL_LINE, 1);
	m_pool.insert(pair<CObject *, CObject *>(pEdge, pNewEdge));
	return pNewEdge;
}

void CEdgeCopyPool::AddEdges2Body(CP_Body *pBody) {
	Map_SrcCopy::iterator it = m_pool.begin();
	while (it != m_pool.end())
	{
		pBody->AddEdge((CP_Edge *)it->second);
		it++;
	}
}
