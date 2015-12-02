#pragma once
#include "CP_TopologyExt.h"
#include <map>

/*
* Create and store the copy of objects
* 
*/

typedef std::map<CObject *, CObject*> Map_SrcCopy;

class ICopyPool {
public:
	virtual ~ICopyPool(){};
protected:
	Map_SrcCopy m_pool;
			// Key : source object pointer. Do not manage memory
			// Value : copy object pointer. Create members but do not release them. Members released by CP_Body
};

class CVertexCopyPool: public ICopyPool {
public:
	CP_VertexExt*	CopyVertex2Pool(CP_Vertex *pVt);
	void			AddVertices2Body(CP_Body *pBody);
};

class CEdgeCopyPool: public ICopyPool {
public:
	CP_EdgeExt*		CopyEdge2Pool(CP_Edge *pEdge, CP_Vertex *pStartVt, CP_Vertex *pEndVt);
	void			AddEdges2Body(CP_Body *pBody);
};