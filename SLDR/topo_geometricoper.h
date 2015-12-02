#pragma once
#include "CP_Topology.h"
#include "CP_TopologyExt.h"
#include <set>

namespace topo_geometric {
	bool	IsEdgesC2Continuity(const CP_Edge *pEdge0, const CP_Edge *pEdge1, const CP_Vertex *pAdjVt);
	bool	IsLoopsC2Continuity(const CP_Loop *pLoop0, const CP_Loop *pLoop1, const CP_Half *pAdjHalf);
	bool	IsTransitionEdge(const CP_Edge *pEdge);
	bool	IsTransitionVert(const CP_Edge *pEdgeCandidate, const CP_Vertex *pEdgeEndVt);
	bool	GetC2ContinuityEdgesAt(CP_Edge **ppEdge0, CP_Edge **ppEdge1, CP_Vertex *pVt, CP_Edge *pTransitionEdge);

	bool	HasHalfInLoop(const CP_Loop *pLoop, const CP_Half *pHalf);
	bool	HasTwinHalfInLoop (const CP_Loop *pLoop, const CP_Half *pHalf);
	bool	CanNotBeConnected(CP_Half * half0, CP_Half * half1);
	CP_Vector3D CalcLoopNormal(CP_Loop *pLoop);
	CP_Vector3D CalcPlaneNormal(const vector<CP_LineSegment3D> &plane);
	void	GetAdjLoops(CP_Loop *pLoop, set<CP_LoopExt *> *pVecResult);
	void	ReverseLoop(CP_Loop *pLoop);

	void	Polygonization3D(CP_Loop *pLoop, vector<CP_Point3D> &result);
	void	Polygonization2D(CP_Loop *pLoop, vector<CP_Point3D> &result);
	bool	IsPtInPolygon2D (CP_Point3D &p, vector<CP_Point3D> &ptPolygon);

	// True for polygon0 inside polygon1, otherwise false
	bool	IsPolyInPoly(vector<CP_Point3D> &polygon0,  vector<CP_Point3D> &polygon1);
	// True for loop0 inside loop1, otherwise false
	bool	IsLoopInLoop(CP_Loop *pLoop0, CP_Loop *pLoop1);

	bool	IsClockwise(CP_Loop *pLoop);
	pair<double, double> FindMeetPtParam(CP_Curve3D *pCurve0, CP_Curve3D *pCurve1, CP_Point3D *pMeetPt);
	
	double	GetCosValue3D(CP_Edge * edge0, CP_Edge * edge1, CP_Vector3D *n0, CP_Vector3D *n1);
	double  GetCosValue3D(CP_Edge * edge0, CP_Edge * edge1);
	double	GetCosValue3D(CP_Curve3D *pCurve0, CP_Curve3D *pCurve1);

	double	GetCosValue2D(CP_Edge * edge0, CP_Edge * edge1);
	double	GetCosValue2D(const CP_Loop * loop0, const CP_Loop * loop1);
	double	GetCosValue2D(CP_Curve3D *pCurve0, CP_Curve3D *pCurve1);

	double	GetAverageCosValue(std::vector<double> &cosArr);
	
	double	GetPolygonCosValue(CP_Loop *pLoop0, CP_Loop *pLoop1);
	double	GetPolygonCosValue(vector<CP_LineSegment3D> &vLineSegs0, vector<CP_LineSegment3D> &vLineSegs1);
	double	GetPolygonCosValue(CP_Loop *pLoop0, vector<CP_LineSegment3D> &vLineSegs1);
	double	GetPolygonCosValue(vector<CP_LineSegment3D> &vLineSegs0, CP_Loop *pLoop);
	
	void	OutputLoop(CP_Loop *pPane);
	
	bool	HasRedLine(CP_Loop *pLoop);
	CP_Vector3D GetCurveTangentAtPt(CP_Curve3D *pCurve, CP_Point3D pt); // Get curve tangent at end point
}