#ifndef CP_TOPOLOGY_H_
#define CP_TOPOLOGY_H_

/************************************************************************/
/* CP_Topology.h                                                        */
/*     include CP_Vertex, CP_Edge, CP_Loop, CP_Face, CP_Body, CP_Half   */
/************************************************************************/

#include "stdafx.h"
#include "CP_PointVector.h"
#include "CP_Curve.h"
#include "CP_Surface.h"

class CP_Edge;
class CP_Vertex;
class CP_Half;
class CP_Loop;
class CP_Face;
class CP_Body;
class CP_ArtificialLine;


// CP_Edge
class CP_Edge : public CObject
{
public:
	CP_Curve3D*  m_pCurve3D;			// Memory managed by CP_Edge
	CP_Vertex*   m_pStartVt;			// Memory managed by CP_Body
	CP_Vertex*   m_pEndVt;				// Memory managed by CP_Body
	CP_Half*	 m_pHalfs[2];			// Memory managed by CP_Edge

public:
	CP_Edge( );
	CP_Edge(CP_Curve3D* curve);
	CP_Edge(CP_Curve3D *pCurve, CP_Vertex *pStart, CP_Vertex *pEnd);
	CP_Edge(CP_Curve3D* curve, CP_Vertex* start, CP_Vertex* end, CP_Half *h0, CP_Half *h1);
	virtual ~CP_Edge( );

	CP_Half *	GetHalf(int index);
	void		SetHalf(int index, CP_Half *half);
	int			GetHalfNum();
	int			GetCurveType() const;
}; // End CP_Edge

// CP_Half
class CP_Half: public CObject
{
	// Public fields
public:
	CP_Vertex * m_pStartVt;			// Memory managed by CP_Body
	CP_Vertex * m_pEndVt;			// Memory managed by Cp_Body
	CP_Edge * m_pEdge;				// Memory managed by CP_Body
	CP_Half * m_pTwin;				// Memory managed by CP_Loop
	CP_Loop * m_pLoop;				// Memory managed by CP_Face

	// Public methods
public:
	CP_Half(void);
	CP_Half(CP_Vertex *start, CP_Vertex *end, CP_Edge *edge, CP_Loop *loop = NULL);
	virtual ~CP_Half(void);
}; // End CP_Half

// CP_Vertex
class CP_Vertex : public CObject
{
public:
	CP_Point3D*    m_pPoint;			// Memory managed by CP_Vertex
protected:
	CObArray    m_outHalfArray;			// Memory managed by CP_Body
public:
	CP_Vertex( );
	CP_Vertex(CP_Point3D* p);			// point's memory managed by CP_Vertex
	virtual ~CP_Vertex( );

	int         GetOutHalfNumber( ) const {return (int)m_outHalfArray.GetSize();};
	CP_Half*	GetOutHalf(int index) const;
	int			FindOutHalfIndex(CP_Half *half) const;
	void		AddOutHalf(CP_Half *half) {m_outHalfArray.Add(half);};
	void        RemoveOutHalf(int index) {m_outHalfArray.RemoveAt(index);};
	void        RemoveOutHalf( ) {m_outHalfArray.RemoveAll( );};
}; // End CP_Vertex

// CP_Loop
class CP_Loop: public CObject
{
public:
	CP_Face* m_pParentFace;					// Memory managed by CP_Body
protected:
	
	CObArray m_halfArray;					// Memory managed by CP_Edge
public:
	CP_Loop( );
	CP_Loop(CP_Face* face);
	virtual ~CP_Loop( );

	int         GetHalfNumber( ) const {return (int)m_halfArray.GetSize();};
	CP_Half*	GetHalf(int index) const ;
	int         FindHalfIndex(CP_Half *half) const;
	void        AddHalf(CP_Half *half);
	void        RemoveHalf(int index);
	void        RemoveAllHalf( );

	CP_Half*	GetFirstHalf() const{return GetHalf(0);};
	CP_Half*	GetLastHalf() const{return GetHalf(GetHalfNumber()-1);};

};// End CP_Loop

// CP_Face
class CP_Face : public CObject
{
public:
	CP_Body* m_pParentBody;					// Memory managed by CP_Body

	CP_Surface3D* m_surface;				// Memory managed by CP_Face
	bool m_sameNormalDirection;				// True if same normal as the surface nature normal direction
protected:
	CObArray m_loopArray;					// First loop is outer loop, and the others are inner loop 
											// Memory managed by CP_Face

public:
	CP_Face( );
	CP_Face(CP_Body *parent, CP_Surface3D *surf = NULL, bool samedir = false);
	virtual ~CP_Face( );

	int         GetLoopNumber() const {return (int)m_loopArray.GetSize();};
	CP_Loop*    GetLoop(int index) const;
	int         FindLoopIndex(CP_Loop *loop) const;
	void        AddLoop(CP_Loop *loop);
	void        RemoveLoop(int index);
	void        RemoveAllLoop( );
	int			GetSurfaceType() const;
}; // End CP_Face

// CP_Body
class CP_Body : public CObject
{
protected:
	CObArray m_faceArray;			// Memory managed by CP_Body
	CObArray m_edgeArray;  			// Memory managed by CP_Body   
	CObArray m_vertexArray; 		// Memory managed by CP_Body  
	CObArray m_aiLineArray;			// Memory managed by CP_Body

public:
	CP_Body( ) {};
	virtual ~CP_Body( );

	CP_Face*    GetFace(int index) const;
	CP_Edge*    GetEdge(int index) const;
	CP_Vertex*  GetVertex(int index) const;
	CP_ArtificialLine *GetAILine(int index) const;
	int         GetFaceNumber( ) const {return (int)m_faceArray.GetSize();};
	int         GetEdgeNumber( ) const {return (int)m_edgeArray.GetSize();};
	int         GetVertexNumber( ) const {return (int)m_vertexArray.GetSize();};
	int			GetAILineNumber( ) const {return (int)m_aiLineArray.GetSize();};

	int         FindFaceIndex(CP_Face *face) const;
	int         FindEdgeIndex(CP_Edge *edge) const;
	int         FindVertexIndex(CP_Vertex *vertex) const;
	int			FindAILineIndex(CP_ArtificialLine *aiLine) const;

	void        AddFace(CP_Face *face);
	void        AddEdge(CP_Edge *edge);
	void		AddAILine(CP_ArtificialLine *aiLine);
	void        AddVertex(CP_Vertex *vertex);
	CP_Vertex*  HasVertex(CP_Point3D *pPoint);

	void        RemoveFace(int index);   
	void		RemoveFace(CP_Face *pFace);
	void        RemoveEdge(int index);
	void		RemoveEdge(CP_Edge *pEdge);
	void        RemoveVertex(int index);
	bool		RemoveVertex(CP_Vertex *pVt);
	void		RemoveAILine(int index);

	void        RemoveAllFace( );
	void        RemoveAllEdge( );
	void        RemoveAllVertex( );
	void		RemoveAllAILine( );



}; // End CP_Body

// CP_ArtificialLine
class CP_ArtificialLine : public CObject
{
public:
	CP_LineSegment3D *m_pLineSeg;				// Memory managed by CP_ArtificialLine
	
public:
	CP_ArtificialLine(void);
	CP_ArtificialLine(CP_Point3D *pStr, CP_Point3D *pEnd, CP_Body *pBody);
	virtual ~CP_ArtificialLine(void);
private:
	CP_Point3D *m_pStrPoint;					// Memory managed by CP_ArtificialLine
	CP_Point3D *m_pEndPoint;					// Memory managed by CP_ArtificialLine
	CP_Edge *	m_pStartEdge;					// Memory managed by CP_Body
	CP_Edge *	m_pEndEdge;						// Memory managed by CP_Body
public:
	CP_Edge * GetStrEdge();
	CP_Edge * GetEndEdge();
}; // End CP_ArtificialLine


#endif