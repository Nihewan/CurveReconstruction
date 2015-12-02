#pragma once
#include "CP_Topology.h"
#include "CP_PointVector.h"
#include <map>

// CP_IBaseExt
class CP_AbstractBaseExt
{
public:
	CP_AbstractBaseExt(void);
	virtual ~CP_AbstractBaseExt(void);

private:
	CObject *m_pParent;							// The corresponding object of copied object . Do not manage memory
	map<string, int> m_extIntProperties;		// The extended integral properties
	map<string, CObject *> m_extCObjPtProperties;	// The extended CObject pointer properties
	map<string, CP_BaseGeometricType *> m_extBaseGeoProperties;

public:
	virtual void		SetProperties(string propertyName, int value);
	virtual void		SetProperties(string propertyName, CObject *pValue);
	virtual void		SetProperties(string propertyName, CP_BaseGeometricType *value);
	virtual int			GetIntegralProperties(string propertyName) const;
	virtual CObject*	GetCObjPtProperties(string propertyName) const;
	virtual CP_BaseGeometricType* GetBaseGeoProperties(string propertyName) const;
	virtual bool		HasProperty(string propertyName) const;
	virtual bool		RemoveProperty(string propertyName);

	virtual void		SetParent(CObject *pObj);	 // Set parent as object in original body
	virtual CObject*	GetParent();				 // Get corresponding object in original body
};

// CP_AssembledBody
class CP_AssembledBody: public CObject
{
public:
	CP_AssembledBody(void){};
	virtual ~CP_AssembledBody(void);	

public:
	CObArray	m_bodyArr;						// Memory managed by CP_AssembledBody

public:
	CP_Body*	GetBody(int index);
	CP_Body*	GetOriginalBody(){return GetBody(0);}
	int			GetBodyNumber();
	int			FindBodyIndex(CP_Body *pBody);
	void		AddBody(CP_Body *pBody);
	void		RemoveBody(int index);
	void		RemoveBody(CP_Body *pBody);
};

class CP_BodyExt: public CP_Body, public CP_AbstractBaseExt
{
public:
	CP_BodyExt():CP_Body(){};
	virtual ~CP_BodyExt(){};

public:
	CP_UCS	m_ucs;
};


class CP_FaceExt: public CP_Face, public CP_AbstractBaseExt
{
public:
	CP_FaceExt(){};
	~CP_FaceExt(){};
	CP_FaceExt(CP_Body *pBody):CP_Face(pBody){}

private:

};

// CP_LoopExt
class CP_LoopExt: public CP_Loop, public CP_AbstractBaseExt
{
public:
	CP_LoopExt();
	CP_LoopExt(CP_Face *face);
	virtual ~CP_LoopExt();
public:
	int m_adjFaceNum;
	int m_cutFlag;

public:	
	bool	IsLoopClosed();
	void	CalcLoopAdjFaceNum();
};

// CP_EdgeExt
class CP_EdgeExt: public CP_Edge, public CP_AbstractBaseExt
{
public:
	CP_EdgeExt(){}
	CP_EdgeExt(CP_Curve3D* curve, CP_Vertex* start, CP_Vertex* end);
	CP_EdgeExt(CP_Curve3D* curve, CP_Vertex* start, CP_Vertex* end, CP_Half *h0, CP_Half *h1):CP_Edge(curve, start, end, h0, h1){}
	virtual ~CP_EdgeExt(){}
};

// CP_HalfExt
class CP_HalfExt: public CP_Half, public CP_AbstractBaseExt
{
public:
	CP_HalfExt(){}
	CP_HalfExt(CP_Vertex *start, CP_Vertex *end, CP_Edge *edge, CP_Loop *loop = NULL):CP_Half(start, end, edge, loop){}
	virtual ~CP_HalfExt(){}
};

// CP_VertexExt
class CP_VertexExt: public CP_Vertex, public CP_AbstractBaseExt
{
public:
	CP_VertexExt(CP_Point3D *pt):CP_Vertex(pt){}
	virtual ~CP_VertexExt(){}
};


// CP_ParallelLoopsGroup
class CP_ParallelLoopsGroup
{
public:
	CP_ParallelLoopsGroup(CP_Loop * loop0, CP_Loop * loop1, double cos);
	CP_ParallelLoopsGroup(CP_Loop * loop0, CP_Loop * loop1, CP_Loop * loop2, double cos);
	virtual ~CP_ParallelLoopsGroup();
public:
	CP_Loop * loopExtd;				// Do not manage memory
	CP_Loop * loopNext;				// Do not manage memory
	CP_Loop * loopRest;				// Do not manage memory
	double cosValue;
	int degreeSum;

public:

	static bool CompareTo(CP_ParallelLoopsGroup*& itemA, CP_ParallelLoopsGroup*& itemB)
	{return itemA->cosValue < itemB->cosValue;}
};
typedef std::vector<CP_ParallelLoopsGroup *> CP_ParaLoopsGroupsArr;
void DeleteParaLoopsGroupsArr(CP_ParaLoopsGroupsArr *& arr);
