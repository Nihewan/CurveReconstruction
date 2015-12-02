#include "stdafx.h"
#include "CP_Topology.h"
#include "base_geometricoper.h"


/************************************************************************/
/* CP_Vertex                                                            */
/************************************************************************/

CP_Vertex::CP_Vertex( ): m_pPoint(NULL)
{
} 

CP_Vertex::CP_Vertex(CP_Point3D* p): m_pPoint(p)
{
} 

CP_Vertex::~CP_Vertex( )
{
	delete m_pPoint;
} 

CP_Half* CP_Vertex::GetOutHalf( int index ) const
{
	int n = (int)m_outHalfArray.GetSize( );
	if (index<0 || index>=n)
		return NULL;
	return (CP_Half *)(m_outHalfArray[index]);
} 

int CP_Vertex::FindOutHalfIndex( CP_Half *half ) const
{
	int n = (int)m_outHalfArray.GetSize();
	for (int i=0; i<n; i++)
		if (m_outHalfArray[i] == half)
			return i;
	return -1;
} 

/************************************************************************/
/* CP_Half                                                              */
/************************************************************************/

CP_Half::CP_Half(): 
m_pStartVt(NULL), m_pEndVt(NULL), m_pEdge(NULL), m_pTwin(NULL)
{
}

CP_Half::CP_Half(CP_Vertex *start, CP_Vertex *end, CP_Edge *edge, CP_Loop *loop /*=NULL*/): 
m_pStartVt(start), m_pEndVt(end), m_pEdge(edge), m_pTwin(NULL), m_pLoop(loop)
{
}

CP_Half::~CP_Half() {}

/************************************************************************/
/* CP_Edge                                                              */
/************************************************************************/

CP_Edge::CP_Edge():
m_pCurve3D(NULL), m_pStartVt(NULL), m_pEndVt(NULL)
{
	m_pHalfs[0] = NULL;
	m_pHalfs[1] = NULL;
}

CP_Edge::CP_Edge(CP_Curve3D* curve):m_pCurve3D(curve) {

}

CP_Edge::CP_Edge(CP_Curve3D* curve, CP_Vertex* start, CP_Vertex* end):
m_pCurve3D(curve), m_pStartVt(start), m_pEndVt(end)
{
	m_pHalfs[0] = new CP_Half(m_pStartVt, m_pEndVt,this);
	m_pStartVt->AddOutHalf(m_pHalfs[0]);
	m_pHalfs[1] = new CP_Half(m_pEndVt, m_pStartVt,this);
	m_pEndVt->AddOutHalf(m_pHalfs[1]);
	m_pHalfs[0]->m_pTwin = m_pHalfs[1];
	m_pHalfs[1]->m_pTwin = m_pHalfs[0];
}

CP_Edge::CP_Edge(CP_Curve3D* curve, CP_Vertex* start, CP_Vertex* end, CP_Half *h0, CP_Half *h1):
m_pCurve3D(curve), m_pStartVt(start), m_pEndVt(end)
{
	m_pHalfs[0] = h0;
	m_pHalfs[1] = h1;
	m_pHalfs[0]->m_pTwin = m_pHalfs[1];
	m_pHalfs[1]->m_pTwin = m_pHalfs[0];
}

CP_Edge::~CP_Edge() {
	delete m_pHalfs[0];
	delete m_pHalfs[1];
	delete m_pCurve3D; 
}

CP_Half* CP_Edge::GetHalf(int index) {
	ASSERT(index < 2);
	return m_pHalfs[index];
}


void CP_Edge::SetHalf(int index, CP_Half *half) {
	ASSERT(index < 2);
	m_pHalfs[index] = half;
}

int CP_Edge::GetHalfNum() {
	int count = 0;
	for (int i = 0; i < 2; ++i) {
		if (m_pHalfs[i] != NULL)
			count ++;
	}
	return count;
}

int CP_Edge::GetCurveType() const {
	CP_LineSegment3D *pLine = dynamic_cast<CP_LineSegment3D *>(m_pCurve3D);
	if (pLine != NULL)
		return TYPE_CURVE_LINESEGMENT;
	CP_Nurbs *pBspline = dynamic_cast<CP_Nurbs *>(m_pCurve3D);
	if (pBspline != NULL)
		return TYPE_CURVE_NURBS;
	CP_Arc *pArc = dynamic_cast<CP_Arc *>(m_pCurve3D);
	if (pArc != NULL)
		return TYPE_CURVE_ARC;
	return -1;
}

/************************************************************************/
/* CP_Loop                                                              */
/************************************************************************/

CP_Loop::CP_Loop(CP_Face* face): m_pParentFace(face) 
{
}

CP_Loop::CP_Loop():m_pParentFace(NULL)
{}

CP_Loop::~CP_Loop() {
	m_halfArray.RemoveAll();
}

CP_Half* CP_Loop::GetHalf(int index) const
{
	int n = (int)m_halfArray.GetSize( );
	if (index<0 || index>=n)
		return NULL;
	return (CP_Half *)(m_halfArray[index]);
} 

int CP_Loop::FindHalfIndex(CP_Half *half) const
{
	int n = (int)m_halfArray.GetSize();
	for (int i=0; i<n; i++)
		if (m_halfArray[i] == half)
			return i;
	return -1;
} 

void CP_Loop::AddHalf(CP_Half *half)
{
	m_halfArray.Add(half);
} 

void CP_Loop::RemoveHalf(int index)
{
	CP_Half* half = GetHalf(index);
	if (half != NULL) {
		m_halfArray.RemoveAt(index);
	}
} 

void CP_Loop::RemoveAllHalf( )
{
	m_halfArray.RemoveAll();
} 

/************************************************************************/
/* CP_Face                                                              */
/************************************************************************/

CP_Face::CP_Face( ): m_pParentBody(NULL), m_surface(NULL), m_sameNormalDirection(true)
{
}

CP_Face::CP_Face(CP_Body *parent, CP_Surface3D *surf, bool samedir)
	: m_pParentBody(parent), m_surface(surf), m_sameNormalDirection(samedir)
{
} 

CP_Face::~CP_Face( )
{
	delete m_surface;
	int n=(int)m_loopArray.GetSize( );
	for (int i = 0; i < n; i++)
		delete m_loopArray[i];
	m_loopArray.RemoveAll( );
}

CP_Loop* CP_Face::GetLoop(int index) const
{
	int n = (int)m_loopArray.GetSize( );
	if (index<0 || index>=n)
		return NULL;
	return (CP_Loop *)(m_loopArray[index]);
} 

int CP_Face::FindLoopIndex(CP_Loop *loop) const
{
	int n = (int)m_loopArray.GetSize();
	for (int i=0; i<n; i++)
		if (m_loopArray[i] == loop)
			return i;
	return -1;
} 

void CP_Face::AddLoop(CP_Loop *loop)
{
	m_loopArray.Add(loop);
	loop->m_pParentFace = this;
} 

void CP_Face::RemoveLoop(int index)
{
	CP_Loop* loop=GetLoop(index);
	if (loop!=NULL)
	{
		delete loop;
		m_loopArray.RemoveAt(index);
	} 
}

void CP_Face::RemoveAllLoop()
{
	int n=(int)m_loopArray.GetSize( );
	for (int i = 0; i < n; i++)
		delete m_loopArray[i];
	m_loopArray.RemoveAll( );
} 

int CP_Face::GetSurfaceType() const {
	if (m_surface == NULL)
		return TYPE_SURFACE_UNDEFINED;
	CP_Plane *pPlane = dynamic_cast<CP_Plane *>(m_surface);
	if (pPlane != NULL)
		return TYPE_SURFACE_PLANE;
	CP_NURBSSurface *pNURBS = dynamic_cast<CP_NURBSSurface *>(m_surface);
	if (pNURBS != NULL)
		return TYPE_SURFACE_NURBS;

	CP_CoonsSurface *pCoons = dynamic_cast<CP_CoonsSurface *>(m_surface);
	if (pCoons != NULL)
		return TYPE_SURFACE_COONS;
	else
		return TYPE_SURFACE_UNDEFINED;
}

/************************************************************************/
/* CP_Body                                                              */
/************************************************************************/

CP_Body::~CP_Body( )
{
	int i;
	int n = (int)m_faceArray.GetSize( );
	for (i = 0; i < n; i++)
		delete m_faceArray[i];
	m_faceArray.RemoveAll( );

	n = (int)m_edgeArray.GetSize( );
	for (i = 0; i < n; i++)
		delete m_edgeArray[i];
	m_edgeArray.RemoveAll( );

	n = m_vertexArray.GetSize( );
	for (i = 0; i < n; i++)
		delete m_vertexArray[i];
	m_vertexArray.RemoveAll( );

	n = m_aiLineArray.GetSize();
	for (i = 0; i < n; ++i)
		delete m_aiLineArray[i];
	m_aiLineArray.RemoveAll();
} 

CP_Face* CP_Body::GetFace(int index) const
{
	int n = (int)m_faceArray.GetSize( );
	if (index<0 || index>=n)
		return NULL;
	return (CP_Face *)(m_faceArray[index]);
}

CP_Edge* CP_Body::GetEdge(int index) const
{
	int n = m_edgeArray.GetSize( );
	if (index<0 || index>=n)
		return NULL;
	return (CP_Edge *)(m_edgeArray[index]);
}


CP_Vertex* CP_Body::GetVertex(int index) const
{
	int n = m_vertexArray.GetSize( );
	if (index<0 || index>=n)
		return NULL;
	return (CP_Vertex *)(m_vertexArray[index]);
}

CP_ArtificialLine *CP_Body::GetAILine(int index) const {
	int n = m_aiLineArray.GetSize();
	if (index < 0 || index >= n)
		return NULL;
	return (CP_ArtificialLine *)(m_aiLineArray[index]);
}

int CP_Body::FindFaceIndex(CP_Face *face) const
{
	int n = (int)m_faceArray.GetSize();
	for (int i=0; i<n; i++)
		if (m_faceArray[i] == face)
			return i;
	return -1;
}

int CP_Body::FindEdgeIndex(CP_Edge *edge) const
{
	int n = (int)m_edgeArray.GetSize();
	for (int i=0; i<n; i++)
		if (m_edgeArray[i] == edge)
			return i;
	return -1;
}

int CP_Body::FindVertexIndex(CP_Vertex *vertex) const
{
	int n = (int)m_vertexArray.GetSize();
	for (int i=0; i<n; i++)
		if (m_vertexArray[i] == vertex)
			return i;
	return -1;
} 

int CP_Body::FindAILineIndex(CP_ArtificialLine *aiLine) const
{
	int n = (int)m_aiLineArray.GetSize();
	for (int i=0; i<n; i++)
		if (m_aiLineArray[i] == aiLine)
			return i;
	return -1;
}

void CP_Body::AddFace(CP_Face *face)
{
	m_faceArray.Add(face);
	face->m_pParentBody = this;
}

void CP_Body::AddEdge(CP_Edge *edge)
{
	m_edgeArray.Add(edge);
}

void CP_Body::AddAILine(CP_ArtificialLine *aiLine) {
	m_aiLineArray.Add(aiLine);
}

void CP_Body::AddVertex(CP_Vertex *vertex)
{
	m_vertexArray.Add(vertex);
} 

CP_Vertex* CP_Body::HasVertex( CP_Point3D *pt )
{
	for(int i = 0; i < m_vertexArray.GetSize(); ++i)
	{
		CP_Vertex * pVert =  (CP_Vertex *)m_vertexArray[i];
		if(base_geometric::IsSamePoint(pVert->m_pPoint, pt)) {
			delete pt;
			return pVert;
		}
	}
	return NULL;
}

void CP_Body::RemoveFace(int index)
{
	CP_Face* face=GetFace(index);
	if (face!=NULL)
	{
		delete face;
		m_faceArray.RemoveAt(index);
	}
}

void CP_Body::RemoveFace(CP_Face *pFace) {
	for (int i = 0; i < GetFaceNumber(); ++i) {
		if (m_faceArray[i] == pFace) {
			RemoveFace(i);
			return;
		}
	}
	ASSERT(false);
}

void CP_Body::RemoveEdge(int index)
{
	CP_Edge* edge=GetEdge(index);
	if (edge!=NULL)
	{
		delete edge;
		m_edgeArray.RemoveAt(index);
	} 
} 

void CP_Body::RemoveEdge(CP_Edge *pEdge) 
{
	for (int i = 0; i < GetEdgeNumber(); ++i) {
		if (m_edgeArray[i] == pEdge) {
			RemoveEdge(i);
			return;
		}
	}
	ASSERT(false);
}


void CP_Body::RemoveVertex(int index)
{
	CP_Vertex* v=GetVertex(index);
	if (v!=NULL)
	{
		delete v;
		m_vertexArray.RemoveAt(index);
	}
}

bool CP_Body::RemoveVertex(CP_Vertex *pVt) {
	int index = FindVertexIndex(pVt);
	if (index != -1) {
		RemoveVertex(index);
		return true;
	}
	return false;
}

void CP_Body::RemoveAILine(int index) {
	CP_ArtificialLine *al = GetAILine(index);
	if (al != NULL) {
		delete al;
		m_aiLineArray.RemoveAt(index);
	}
}

void CP_Body::RemoveAllFace()
{
	int n = (int)m_faceArray.GetSize( );
	for (int i = 0; i < n; i++)
		delete m_faceArray[i];
	m_faceArray.RemoveAll( );
} 

void CP_Body::RemoveAllEdge()
{
	int n = (int)m_edgeArray.GetSize( );
	for (int i = 0; i < n; i++)
		delete m_edgeArray[i];
	m_edgeArray.RemoveAll( );
} 

void CP_Body::RemoveAllVertex()
{
	int n = (int)m_vertexArray.GetSize( );
	for (int i = 0; i < n; i++)
		delete m_vertexArray[i];
	m_vertexArray.RemoveAll( );
} 

void CP_Body::RemoveAllAILine() {
	for (int i = 0; i < m_aiLineArray.GetSize(); ++i) {
		delete m_aiLineArray[i];
	}
	m_aiLineArray.RemoveAll();
}


/************************************************************************/
/* CP_ArtificialLine                                                    */
/************************************************************************/

CP_ArtificialLine::CP_ArtificialLine(void): m_pStartEdge(NULL), m_pEndEdge(NULL), m_pLineSeg(NULL),
	m_pStrPoint(NULL), m_pEndPoint(NULL)
{
}

CP_ArtificialLine::CP_ArtificialLine(CP_Point3D *startPt, CP_Point3D *endPt, CP_Body *pBody)
{
	m_pStrPoint = startPt;
	m_pEndPoint = endPt;
	m_pLineSeg = new CP_LineSegment3D(*startPt, *endPt);
	m_pStartEdge = NULL;
	m_pEndEdge = NULL;
	int size = pBody->GetEdgeNumber();

	double minStrDis = ATTACH_CURVE_THRESHOLD;
	double minEndDis = ATTACH_CURVE_THRESHOLD;
	double dis = 0;
	for(int i = 0; i < size; i++)
	{
		CP_Edge * pEdge = pBody->GetEdge(i);
		if(pEdge->m_pCurve3D->HasPoint(*startPt, dis))
		{
			if (minStrDis > dis) {
				m_pStartEdge = pEdge;
				minStrDis = dis;
			}
		}
		else if(pEdge->m_pCurve3D->HasPoint(*endPt, dis))
		{
			if (minEndDis > dis) {
				m_pEndEdge = pEdge;
				minEndDis = dis;
			}
		}
		if(m_pStartEdge != NULL && m_pEndEdge != NULL) break;
	}
}

CP_ArtificialLine::~CP_ArtificialLine(void)
{
	m_pStartEdge = NULL;
	m_pEndEdge = NULL;
	if (m_pLineSeg != NULL) {
		delete m_pLineSeg;
		m_pLineSeg = NULL;
	}
	if (m_pStrPoint != NULL) {
		delete m_pStrPoint;
		m_pStrPoint = NULL;
	}
	if (m_pEndPoint != NULL) {
		delete m_pEndPoint;
		m_pEndPoint = NULL;
	}
}

CP_Edge * CP_ArtificialLine::GetStrEdge()
{
	return m_pStartEdge;
}

CP_Edge * CP_ArtificialLine::GetEndEdge()
{
	return m_pEndEdge;
}