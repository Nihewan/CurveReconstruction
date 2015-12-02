#pragma once
#include "CP_Topology.h"
#include "CP_TopologyExt.h"
#include <queue>
#include <map>
#include "est_comparator.h"
#include "CBinaryHeep.h"



class CDepthEstimation
{
private:
	CP_AssembledBody *m_pAsmbBody;
	vector<vector<CP_LineSegment3D>> m_bodyRefPlanes;	// Memory managed by CDepthEstimation

public:
	CDepthEstimation(CP_AssembledBody *pAsmbBody);
	~CDepthEstimation(void);

private:
	bool	m_isDepthEstimated;

public:
	void	Estimate();
	bool	EstimateUCS();

private:
	bool	EstimateUCS(CP_BodyExt *pBody);		// Reconstruct local coordinate system
	void	AdjustAbnormalUCS(CP_Vector3D* ucs[3], int fixedAxisIndex);		// Eliminate abnormal coordinate system. And adjust the coordinate system. The input vectors must be normalized.
	void	NormalizeUCS(CP_Vector3D ucs[3]);
	void	SetStandardUSC(CP_Vector3D *uscVecs[3]);

	void	EstimateBodyByPriority(vector<CP_BodyExt *> *pPriQueueBodys);	// Estimate vertex depth in the order of priority. The high priority depth will replace the low priority depth
	void	EstimateVertex(CP_Body *pBody);	
	void	EstimateSurface(CP_Body *pBody);  // Calculate normal of plane
	void	EstimatePlaneVt(CP_LoopExt *pLoop); // Estimate vertex's depth on the face
	void	EstimateCurve(CP_Body *pBody);	// Estimate curve's normal
	void	EstimateCurvedFaceVt(CP_LoopExt *pLoop);

	// Initialize queue with visited vertices of loop. If there is no visited vertex, select the first vertex, set its depth 0 and push it into queue.
	void	InitVertexPriQueue(CP_LoopExt *pLoop, priority_queue<CP_VertexExt *, vector<CP_VertexExt *>, est_comparator::VertexWeightDesComparator> &queueVertex); 
	void	InitVertexMaxHeap(CP_LoopExt *pLoop, CBinaryHeap<CP_VertexExt *, est_comparator::VertexWeightDesComp> &maxHeap);
	void	InitVertexQueueMap(CP_LoopExt *pLoop,  map<CP_VertexExt *, int, est_comparator::VertexWeightDesComparator> &queueVt);



	double	EstimateAdjVtOnCurvedFace(CP_CoonsSurface *pCoonsFace, vector<double> usegs,
		vector<double> vsegs, pair<double, double> beginUV, pair<double, double> endUV, double beginZ);
	double	CalcDeltaZ(const CP_Point3D &pt0, const CP_Point3D &pt1, const CP_Point3D &pt2, const CP_Point3D &pt3 );

	void	RemoveBodyProperties(CP_Body *pBody);
};



