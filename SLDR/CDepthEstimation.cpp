#include "stdafx.h"
#include "CDepthEstimation.h"
#include "topo_geometricoper.h"
#include "CLog.h"
#include "CVectorDataBlock.h"
#include "CDBScan.h"
#include <algorithm>
#include <functional> 
#include "cubiccorner.h"
#include <math.h>
#include <set>
#include "base_geometricoper.h"
#include "rec_utils.h"
#include "est_vtattroper.h"
#include "est_refplane.h"
#include "CBSplineEstimation.h"
#include "CArcEstimation.h"
#include "est_surface.h"
#include "rec_bodyassembly.h"
#include "CTimer.h"
#include "surface_basic.h"
#include "rec_curve.h"
#include "CBinaryHeep.h"


CDepthEstimation::CDepthEstimation(CP_AssembledBody *pAsmbBody): m_pAsmbBody(pAsmbBody), m_isDepthEstimated(false)
{
}


CDepthEstimation::~CDepthEstimation(void)
{
	if (m_isDepthEstimated) {
		int beginIndex = m_pAsmbBody->GetBodyNumber() == 1? 0 : 1;
		for (int i = beginIndex; i < m_pAsmbBody->GetBodyNumber(); ++i) 
			RemoveBodyProperties(m_pAsmbBody->GetBody(i));
	}
}


void CDepthEstimation::Estimate() {
	m_isDepthEstimated = true;
	// Reconstruct local coordinate system
	// EstimateUCS((CP_BodyExt *)m_pAsmbBody->GetOriginalBody());
	for (int i = 1; i < m_pAsmbBody->GetBodyNumber(); ++i) {
		// if (!EstimateUCS((CP_BodyExt*)m_pAsmbBody->GetBody(i)))
		((CP_BodyExt *)m_pAsmbBody->GetBody(i))->m_ucs
				= ((CP_BodyExt *)m_pAsmbBody->GetOriginalBody())->m_ucs;
	}

	// Find reference plane parallel to face
	est_refplane::InitBodyRefPlanes(m_pAsmbBody->GetOriginalBody(), m_bodyRefPlanes);
	vector<CP_BodyExt *> vBodysWithWeight;
	if (m_pAsmbBody->GetBodyNumber() == 1) {
		CP_BodyExt *pBody = (CP_BodyExt *)m_pAsmbBody->GetOriginalBody();
		est_refplane::CalcBodyWeight(pBody, m_bodyRefPlanes);
		vBodysWithWeight.push_back(pBody);
	} else {
		for (int i = 1; i < m_pAsmbBody->GetBodyNumber(); ++i) {
			CP_BodyExt *pBody = (CP_BodyExt *)m_pAsmbBody->GetBody(i);
			est_refplane::CalcBodyWeight(pBody, m_bodyRefPlanes);
			vBodysWithWeight.push_back(pBody);
			// CLog::log("Body %d weight = %d", i, pBody->GetIntegralProperties(PROPERTY_NAME_WEIGHT));
		}
		// Sort body by weight
		std::sort(vBodysWithWeight.begin(), vBodysWithWeight.end(), est_comparator::BodyWeightDesComparator::CmpBody);
	}
	
	// Initialize visited property for each vertex
	for (int i = 0; i < m_pAsmbBody->GetBodyNumber(); ++i) {
		CP_BodyExt *pBody = (CP_BodyExt *)m_pAsmbBody->GetBody(i);
		for (int j = 0; j < pBody->GetVertexNumber(); ++j) {
			CP_VertexExt *pVt = (CP_VertexExt *)pBody->GetVertex(j);
			pVt->SetProperties(PROPERTY_NAME_VISITED, 0);
			pVt->SetProperties(PROPERTY_NAME_WEIGHT, 0);
		}
	}

	CTimer myTimer;
	myTimer.Start();
	// Estimate body's vertice's depth according to the priority of body
	EstimateBodyByPriority(&vBodysWithWeight);
	CLog::log("Estimate body Run time = %f ms", myTimer.End());

	myTimer.Start();
	rec_bodyassembly::AssembleBody(m_pAsmbBody);
	CLog::log("Assemble body Run time = %f ms", myTimer.End());

}


/************************************************************************/
/* Estimate coordinate system                                           */
/************************************************************************/

bool CDepthEstimation::EstimateUCS() {
	return EstimateUCS((CP_BodyExt *)m_pAsmbBody->GetOriginalBody());
}


// Estimate local coordinate system for body
bool CDepthEstimation::EstimateUCS(CP_BodyExt *pBody) {

	const double EFFECTIVE_RADIUS = PI/12;
	CVectorDataBlock dataBlock(EFFECTIVE_RADIUS);
	typedef DataWithVLabel<CP_Vector3D> DWL;
	bool hasUCS = true;

	// Add vector to data block. Curve and line segment has the same weight
	for (int i = 0; i < pBody->GetEdgeNumber(); ++i) {
		CP_Edge *pEdge = pBody->GetEdge(i);
		CP_Curve3D *pCurve = pEdge->m_pCurve3D;
		if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
			vector<double> segs;
			pEdge->m_pCurve3D->Split2LineSeg(CURVED_FACE_SPLIT_ERR, segs, 0, 1);
			CP_Point3D startPt = pCurve->GetStartingPoint();
			CP_Point3D endPt = pCurve->GetEndingPoint();
			if (segs.size() >= 1) {
				static_cast<CP_EdgeExt*>(pEdge)->SetProperties(PROPERTY_NAME_IS_SPLIT_IN_MID, 1);
				CP_Point3D midPt = pCurve->GetPoint(0.5);
				dataBlock.AddMember(&startPt, &midPt);
				dataBlock.AddMember(&midPt, &endPt); 
			} else {
				dataBlock.AddMember(&startPt, &endPt);
				dataBlock.AddMember(&startPt, &endPt); 
			}
		} else {
			dataBlock.AddMember(&pCurve->GetStartingPoint(), &pCurve->GetEndingPoint());
			dataBlock.AddMember(&pCurve->GetStartingPoint(), &pCurve->GetEndingPoint());
		}
	}
	dataBlock.InitDisMatrix();

	// Cluster
	CDBScan<CP_Vector3D> dbScan(&dataBlock, 2);
	std::vector<CCluster<CP_Vector3D> *> vResult;
	dbScan.DBScan(&vResult);
	std::sort(vResult.begin(), vResult.end(), CCluster<CP_Vector3D>::DSCCmp);

	// Find x-axis and z-axis
	pBody->m_ucs.m_axisY.m_y = 1;
	CP_Vector3D* uscVecs[3] = {&pBody->m_ucs.m_axisX,
		&pBody->m_ucs.m_axisZ, &pBody->m_ucs.m_axisY};

	if (vResult.size() > 2) {
		for (unsigned int i = 0, k = 0; i < vResult.size() && k < 2; ++i) {
			CP_Vector3D *pCenter = vResult.at(i)->CalcCenter(&base_geometric::GetMinIncludedAngle);
			pCenter->Normalize();
			if (base_geometric::GetMinIncludedAngle(*pCenter, pBody->m_ucs.m_axisY) > EFFECTIVE_RADIUS) {
				uscVecs[k]->m_x = pCenter->m_x;
				uscVecs[k]->m_y = pCenter->m_y;
				uscVecs[k]->m_z = pCenter->m_z;
				k++;
			}
		}
	} else {
		SetStandardUSC(uscVecs);
		hasUCS = false;
	}

	cubic_corner::NormalizeCubicCornerVecs(uscVecs);
	AdjustAbnormalUCS(uscVecs, 2);

	// Calculate Z value of UCS vectors
	cubic_corner::CalcCubicCorner(uscVecs, 2, 1);

	// Release memory
	for (unsigned int i = 0; i < vResult.size(); ++i) {
		CCluster<CP_Vector3D> *p = vResult.at(i);
		delete p;
		p = NULL;
	}
	return hasUCS;
}

void CDepthEstimation::AdjustAbnormalUCS(CP_Vector3D* ucs[], int fixedAxisIndex) {
	for (int i = 1; i < 3; ++i) {
		// If two vector are almost parallel
		if (abs(*ucs[(fixedAxisIndex + i)%3] * *ucs[fixedAxisIndex]) < 1 - COS_MAX) {
			CP_Vector3D vecMid = *ucs[fixedAxisIndex] + *ucs[(fixedAxisIndex+i+1)%3];
			if (vecMid * *ucs[(fixedAxisIndex + i)%3] < 0)
				vecMid = - vecMid;
			*ucs[(fixedAxisIndex + i)%3] = vecMid;
			ucs[(fixedAxisIndex + i)%3]->Normalize();
			break;
		}
	}
}

void CDepthEstimation::SetStandardUSC(CP_Vector3D *uscVecs[3]) {
	double tmp = sqrt(3)/2;
	uscVecs[0]->m_x = tmp;
	uscVecs[0]->m_y = -0.5;
	uscVecs[1]->m_x = -tmp;
	uscVecs[1]->m_y = -0.5;
}



/************************************************************************/
/* Estimate depth                                                       */
/************************************************************************/

// Estimate vertices's depth in priority of body's weight
void CDepthEstimation::EstimateBodyByPriority(vector<CP_BodyExt *> *pPriQueueBodys) {
	for (unsigned int i = 0; i < pPriQueueBodys->size(); ++i) {
		CP_Body *pBody = pPriQueueBodys->at(i);

		/*CTimer myTimer;
		myTimer.Start();*/
		EstimateVertex(pBody);/*
		CLog::log("Estimate vertex run time %f ms", myTimer.End());*/

		/*myTimer.Start();*/
		EstimateSurface(pBody); 
		/*CLog::log("Estimate surface run time %f ms", myTimer.End());*/

		/*myTimer.Start();*/
		EstimateCurve(pBody);
		/*CLog::log("Estimate curve run time %f ms", myTimer.End());*/
	}
}



void CDepthEstimation::EstimateVertex(CP_Body *pBody) {

	// Find the loop with max weight
	CP_LoopExt* pMaxWeighLoop = NULL;
	int maxWeight = 0;
	for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
		CP_LoopExt *pLoop = (CP_LoopExt *)pBody->GetFace(i)->GetLoop(0);
		pLoop->SetProperties(PROPERTY_NAME_VISITED, 0);
		int weight = pLoop->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
		if (weight > maxWeight) {
			maxWeight = weight;
			pMaxWeighLoop = pLoop;
		}
	}

	// Begin estimating the loop with max weight, and its adjacent loops.
	priority_queue<CP_LoopExt *, vector<CP_LoopExt *>, est_comparator::LoopWeightDesComparator> queueLoops;
	queueLoops.push(pMaxWeighLoop);
	pMaxWeighLoop->SetProperties(PROPERTY_NAME_VISITED, 1);
	while (!queueLoops.empty()) {
		CP_LoopExt *pLoop = queueLoops.top();
		queueLoops.pop();
		if (pLoop->m_pParentFace->GetSurfaceType() == TYPE_SURFACE_PLANE)
			EstimatePlaneVt(pLoop);
		else 
			EstimateCurvedFaceVt(pLoop);

		set<CP_LoopExt *> setAdjLoops;
		topo_geometric::GetAdjLoops(pLoop, &setAdjLoops);
		for (set<CP_LoopExt *>::iterator si = setAdjLoops.begin(); si != setAdjLoops.end(); ++si) {
			if (!(*si)->GetIntegralProperties(PROPERTY_NAME_VISITED)) {
				queueLoops.push(*si);
				(*si)->SetProperties(PROPERTY_NAME_VISITED, 1);
			}
		}
	}

}

// Calculate normal of plane
void CDepthEstimation::EstimateSurface(CP_Body *pBody) {
	for (int i = 0 ; i < pBody->GetFaceNumber(); ++i) {
		CP_Face *pFace = pBody->GetFace(i);
		if (pFace->GetSurfaceType() == TYPE_SURFACE_PLANE) {
			est_surface::EstimatePlane(pFace);
		} 
	}
}



void CDepthEstimation::EstimateCurve(CP_Body *pBody) {
	for (int i = 0; i < pBody->GetEdgeNumber(); ++i) {
		CP_EdgeExt *pEdge = (CP_EdgeExt *)pBody->GetEdge(i);
		if (pEdge->GetCurveType() == TYPE_CURVE_LINESEGMENT) {
			rec_curve::ReconstructLineSegEndPts(pEdge);
		} else if (pEdge->GetCurveType() == TYPE_CURVE_NURBS) {
			CBSplineEstimation est;
			((CP_Nurbs *)pEdge->m_pCurve3D)->m_normal = 
				est.EstimateNormal(pEdge);
			rec_curve::ReconstructBsplineCtrlPts(pEdge);
		} else if (pEdge->GetCurveType() == TYPE_CURVE_ARC) {
			CArcEstimation est;
			((CP_Arc *)pEdge->m_pCurve3D)->m_ucs.m_axisZ = 
				est.EstimateNormal(pEdge);
			rec_curve::ReconstructArcUCS(pEdge);
		}
	}
}



void CDepthEstimation::EstimatePlaneVt(CP_LoopExt *pLoop) {
	CP_Vector3D n = *(CP_Vector3D *)pLoop->GetBaseGeoProperties(PROPERTY_NAME_MATCH_PANE_NORMAL);
	int curVtWeight = pLoop->GetIntegralProperties(PROPERTY_NAME_WEIGHT);

	// Initialize priority queue with visited vertex 
	//est_vtattroper::InitNotInQueue(pLoop);
	//priority_queue<CP_VertexExt *, vector<CP_VertexExt *>, est_comparator::VertexWeightDesComparator> queueVertex;
	//InitVertexPriQueue(pLoop, queueVertex);

	//while (!queueVertex.empty())
	//{
	//	CP_VertexExt *pVt = queueVertex.top();
	//	queueVertex.pop();
	//	for (int i = 0; i < pVt->GetOutHalfNumber(); ++i) {
	//		CP_Half *pHalf = pVt->GetOutHalf(i);
	//		if (pHalf->m_pLoop == pLoop || pHalf->m_pTwin->m_pLoop == pLoop) {
	//			CP_VertexExt *pAdjVt = dynamic_cast<CP_VertexExt *>(pHalf->m_pEndVt);
	//			if (curVtWeight > est_vtattroper::GetVertexWeight(pAdjVt)) 
	//			{
	//				CP_Vector3D v(*pAdjVt->m_pPoint - *pVt->m_pPoint);
	//				double z = pVt->m_pPoint->m_z + base_geometric::GetVectorZ(v, n);

	//				est_vtattroper::SetVertexDepthAndWeight(pAdjVt, z, curVtWeight);
	//				queueVertex.push(pAdjVt);
	//			} 
	//		}
	//	}
	//}

	CBinaryHeap<CP_VertexExt *, est_comparator::VertexWeightDesComp> maxHeap(pLoop->GetHalfNumber());
	InitVertexMaxHeap(pLoop, maxHeap);

	while (!maxHeap.empty())
	{
		CP_VertexExt *pVt = maxHeap.top();
		maxHeap.del(0);
		for (int i = 0; i < pVt->GetOutHalfNumber(); ++i) {
			CP_Half *pHalf = pVt->GetOutHalf(i);
			if (pHalf->m_pLoop == pLoop || pHalf->m_pTwin->m_pLoop == pLoop) {
				CP_VertexExt *pAdjVt = dynamic_cast<CP_VertexExt *>(pHalf->m_pEndVt);
				if (curVtWeight > est_vtattroper::GetVertexWeight(pAdjVt)) 
				{
					CP_Vector3D v(*pAdjVt->m_pPoint - *pVt->m_pPoint);
					double z = pVt->m_pPoint->m_z + base_geometric::GetVectorZ(v, n);
					est_vtattroper::SetVertexDepthAndWeight(pAdjVt, z, curVtWeight);

					int index = maxHeap.find(pAdjVt);
					if (index >= 0)
						maxHeap.del(index);
					maxHeap.insert(pAdjVt);
				} 
			}
		}
	}

}

void CDepthEstimation::EstimateCurvedFaceVt(CP_LoopExt *pLoop) {
	int curVtWeight = pLoop->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
	// topo_geometric::OutputLoop(pLoop);

	// Initialize priority queue with visited vertex
	priority_queue<CP_VertexExt *, vector<CP_VertexExt *>, est_comparator::VertexWeightDesComparator> queueVertex;
	InitVertexPriQueue(pLoop, queueVertex);

	CP_CoonsSurface *pCoonsFace = (CP_CoonsSurface *)pLoop->m_pParentFace->m_surface;
	vector<double> usegs, vsegs;
	pCoonsFace->SplitSurface(CURVED_FACE_SPLIT_ERR, usegs, vsegs);
	usegs.push_back(1);
	usegs.insert(usegs.begin(), 0);
	vsegs.push_back(1);
	vsegs.insert(vsegs.begin(), 0);

	while (!queueVertex.empty())
	{
		CP_VertexExt *pVt = queueVertex.top();
		queueVertex.pop();
		pair<double, double> ptUV = pCoonsFace->GetEndPtUV(*pVt->m_pPoint);
		for (int i = 0; i < pVt->GetOutHalfNumber(); ++i) {
			CP_Half *pHalf = pVt->GetOutHalf(i);
			if (pHalf->m_pLoop == pLoop || pHalf->m_pTwin->m_pLoop == pLoop) {
				CP_VertexExt *pAdjVt = (CP_VertexExt *)pHalf->m_pEndVt;
				if (curVtWeight > est_vtattroper::GetVertexWeight(pAdjVt)) {
					pair<double, double> adjPtUV = pCoonsFace->GetEndPtUV(*pAdjVt->m_pPoint);
					double z = EstimateAdjVtOnCurvedFace(pCoonsFace, usegs, vsegs, ptUV, adjPtUV, pVt->m_pPoint->m_z);

					est_vtattroper::SetVertexDepthAndWeight(pAdjVt, z, curVtWeight);
					queueVertex.push(pAdjVt);
				}
			}
		}
	}

// 	typedef map<CP_VertexExt *, int, est_comparator::VertexWeightDesComparator> PriQueue;
// 	PriQueue queueVt;
// 	InitVertexPriQueue(pLoop, queueVt);
// 	CP_CoonsSurface *pCoonsFace = (CP_CoonsSurface *)pLoop->m_pParentFace->m_surface;
// 	vector<double> usegs, vsegs;
// 	pCoonsFace->SplitSurface(CURVED_FACE_SPLIT_ERR, usegs, vsegs);
// 	usegs.push_back(1);
// 	usegs.insert(usegs.begin(), 0);
// 	vsegs.push_back(1);
// 	vsegs.insert(vsegs.begin(), 0);
// 
// 	while (!queueVt.empty())
// 	{
// 		CP_VertexExt *pVt = queueVt.begin()->first;
// 		queueVt.erase(queueVt.begin());
// 		pair<double, double> ptUV = pCoonsFace->GetEndPtUV(*pVt->m_pPoint);
// 		for (int i = 0; i < pVt->GetOutHalfNumber(); ++i) {
// 			CP_Half *pHalf = pVt->GetOutHalf(i);
// 			if (pHalf->m_pLoop == pLoop || pHalf->m_pTwin->m_pLoop == pLoop) {
// 				CP_VertexExt *pAdjVt = (CP_VertexExt *)pHalf->m_pEndVt;
// 				if (curVtWeight > est_vtattroper::GetVertexWeight(pAdjVt)) {
// 					pair<double, double> adjPtUV = pCoonsFace->GetEndPtUV(*pAdjVt->m_pPoint);
// 					double z = EstimateAdjVtOnCurvedFace(pCoonsFace, usegs, vsegs, ptUV, adjPtUV, pVt->m_pPoint->m_z);
// 					est_vtattroper::SetVertexDepthAndWeight(pAdjVt, z, curVtWeight);
// 
// 					PriQueue::iterator it = queueVt.find(pAdjVt);
// 					if (it != queueVt.end())
// 						queueVt.erase(it);
// 					queueVt.insert(pair<CP_VertexExt*, int>(pAdjVt, 0));
// 				}       
// 			}
// 		}
// 	}

}

double CDepthEstimation::EstimateAdjVtOnCurvedFace(CP_CoonsSurface *pCoonsFace, 
											  vector<double> usegs,
											  vector<double> vsegs,
											  pair<double, double> beginUV,
											  pair<double, double> endUV, double beginZ) 
{
	double resultZ = beginZ;

	// Adjacent vertex is on u direction or v direction of the known point
	bool isUDirection = true;
	if (abs(beginUV.first - endUV.first) < TOLER) 
		isUDirection = false;

	// Is the known point the last point in the other direction or not
	int isLast = false;
	if (isUDirection && abs(beginUV.second - 1) < TOLER)
		isLast = true;
	if (!isUDirection && abs(beginUV.first - 1) < TOLER)
		isLast = true;

	int direct = 1;
	if (beginUV.first > endUV.first || beginUV.second > endUV.second)
		direct = -1;

	if (isUDirection) {
		int curIndex = direct == 1? 0 : usegs.size()-1;
		double vNeighbor = isLast? vsegs[vsegs.size()-2] : vsegs[1];
		double vBound = isLast? 1 : 0;
		for (unsigned int i = 1; i < usegs.size(); ++i) {
			CP_Point3D pt0 = pCoonsFace->GetPoint(usegs[curIndex], vBound);
			CP_Point3D pt1 = pCoonsFace->GetPoint(usegs[curIndex], vNeighbor);
			CP_Point3D pt2 = pCoonsFace->GetPoint(usegs[curIndex+direct], vBound);
			CP_Point3D pt3 = pCoonsFace->GetPoint(usegs[curIndex+direct], vNeighbor);
			resultZ += CalcDeltaZ(pt0, pt1, pt2, pt3);
			//curIndex ++;
		}
	} else {
		int curIndex = direct == 1? 0 : vsegs.size()-1;
		double uNeighbor = isLast? usegs[usegs.size() - 2] : usegs[1];
		double uBound = isLast? 1 : 0;
		for (unsigned int i = 1; i < vsegs.size(); ++i) {
			CP_Point3D pt0 = pCoonsFace->GetPoint(uBound, vsegs[curIndex]);
			CP_Point3D pt1 = pCoonsFace->GetPoint(uNeighbor, vsegs[curIndex]);
			CP_Point3D pt2 = pCoonsFace->GetPoint(uBound, vsegs[curIndex+direct]);
			CP_Point3D pt3 = pCoonsFace->GetPoint(uNeighbor, vsegs[curIndex+direct]);
			resultZ += CalcDeltaZ(pt0, pt1, pt2, pt3);
			//curIndex++;
		}
	}
	return resultZ;
}

double CDepthEstimation::CalcDeltaZ(const CP_Point3D &pt0, const CP_Point3D &pt1, 
								  const CP_Point3D &pt2, const CP_Point3D &pt3 ) {
	vector<CP_LineSegment3D> vLineSeg;
	surface_basic::Init4LineSegPatch(vLineSeg, pt0, pt1, pt2, pt3);
	vector<CP_LineSegment3D> matchPlane = est_refplane::FindParaRefPlane(vLineSeg, m_bodyRefPlanes).second;
	CP_Vector3D n = topo_geometric::CalcPlaneNormal(matchPlane);
	CP_Vector3D v(pt2 - pt0);
	return base_geometric::GetVectorZ(v, n);
}


void CDepthEstimation::InitVertexPriQueue(CP_LoopExt *pLoop,
										  priority_queue<CP_VertexExt *, vector<CP_VertexExt *>, est_comparator::VertexWeightDesComparator> &queueVertex) 
{
	int curVtWeight = pLoop->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
	for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
		CP_VertexExt *pVt = (CP_VertexExt *)pLoop->GetHalf(i)->m_pStartVt;
		if (est_vtattroper::IsVisited(pVt)) {
			queueVertex.push(pVt);
		}
	}
	if (queueVertex.empty()) {
		CP_VertexExt *pStrVt = (CP_VertexExt *)pLoop->GetFirstHalf()->m_pStartVt;
		est_vtattroper::SetVertexDepthAndWeight(pStrVt, 0, curVtWeight);
		queueVertex.push(pStrVt);
	}
}

void CDepthEstimation::InitVertexMaxHeap(CP_LoopExt *pLoop,
										  CBinaryHeap<CP_VertexExt *, est_comparator::VertexWeightDesComp> &maxHeap) 
{
	int curVtWeight = pLoop->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
	for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
		CP_VertexExt *pVt = (CP_VertexExt *)pLoop->GetHalf(i)->m_pStartVt;
		if (est_vtattroper::IsVisited(pVt)) {
			maxHeap.insert(pVt);
		}
	}
	if (maxHeap.empty()) {
		CP_VertexExt *pStrVt = (CP_VertexExt *)pLoop->GetFirstHalf()->m_pStartVt;
		est_vtattroper::SetVertexDepthAndWeight(pStrVt, 0, curVtWeight);
		maxHeap.insert(pStrVt);
	}
}

void CDepthEstimation::InitVertexQueueMap(CP_LoopExt *pLoop, 
										  map<CP_VertexExt *, int, est_comparator::VertexWeightDesComparator> &queueVt) 
{
	int curVtWeight = pLoop->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
	for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
		CP_VertexExt *pVt = (CP_VertexExt *)pLoop->GetHalf(i)->m_pStartVt;
		if (est_vtattroper::IsVisited(pVt)) {
			queueVt.insert(pair<CP_VertexExt *, int>(pVt, 0));
		}
	}
	if (queueVt.empty()) {
		CP_VertexExt *pStrVt = (CP_VertexExt *)pLoop->GetFirstHalf()->m_pStartVt;
		est_vtattroper::SetVertexDepthAndWeight(pStrVt, 0, curVtWeight);
		queueVt.insert(pair<CP_VertexExt *, int>(pStrVt, 0));
	}
}


void CDepthEstimation::RemoveBodyProperties(CP_Body *pBody) {
	for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
		CP_LoopExt *pLoop = (CP_LoopExt *)pBody->GetFace(i)->GetLoop(0);
		CP_Vector3D *pV = (CP_Vector3D *)pLoop->GetBaseGeoProperties(PROPERTY_NAME_MATCH_PANE_NORMAL);
		delete pV;
		pV = NULL;
		pLoop->RemoveProperty(PROPERTY_NAME_MATCH_PANE_NORMAL);
		pLoop->RemoveProperty(PROPERTY_NAME_WEIGHT);
		pLoop->RemoveProperty(PROPERTY_NAME_VISITED);
	}
}