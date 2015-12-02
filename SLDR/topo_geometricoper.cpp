#include "stdafx.h"
#include "topo_geometricoper.h"

#include "CP_Topology.h"
#include <algorithm>
#include "CP_TopologyExt.h"
#include <numeric>
#include "base_geometricoper.h"
#include <set>
#include "CLog.h"
#include "propertiesnames.h"

namespace topo_geometric {
	bool IsEdgesC2Continuity(const CP_Edge *pEdge0, const CP_Edge *pEdge1, const CP_Vertex *pAdjVt) {
		double t0 = 1;
		if (base_geometric::IsSamePoint(&pEdge0->m_pCurve3D->GetStartingPoint(), pAdjVt->m_pPoint))
			t0 = 0;
		double t1 = 1;
		if (base_geometric::IsSamePoint(&pEdge1->m_pCurve3D->GetStartingPoint(), pAdjVt->m_pPoint))
			t1 = 0;

		CP_Vector3D tangent0 = pEdge0->m_pCurve3D->GetTangent(t0);	// Tangent already normalized
		CP_Vector3D tangent1 = pEdge1->m_pCurve3D->GetTangent(t1);

		if (tangent0 * tangent1 < COS_MAX) 
			return false;

		double curvature0 = pEdge0->m_pCurve3D->GetCurvature(t0);
		double curvature1 = pEdge1->m_pCurve3D->GetCurvature(t1);

		if (abs(curvature0 - curvature1) < SAME_CURVATURE)
			return true;
		return false;
	}

	bool IsLoopsC2Continuity(const CP_Loop *pLoop0, const CP_Loop *pLoop1, const CP_Half *pAdjHalf) {
		CP_Vertex* adjVts[2] = {pAdjHalf->m_pStartVt, pAdjHalf->m_pEndVt};
		CP_Edge *pEdge0 = NULL;
		CP_Edge *pEdge1 = NULL;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < adjVts[i]->GetOutHalfNumber(); ++j) {
				CP_Half *pHalf = adjVts[i]->GetOutHalf(j);
				if (pHalf != pAdjHalf) {
					if (HasHalfInLoop(pLoop0, pHalf) || HasTwinHalfInLoop(pLoop0, pHalf)) {
						pEdge0 = pHalf->m_pEdge;
					}
					else if (HasHalfInLoop(pLoop1, pHalf) || HasTwinHalfInLoop(pLoop1, pHalf)) {
						pEdge1 = pHalf->m_pEdge;
					}
				}
			}
			if (!IsEdgesC2Continuity(pEdge0, pEdge1, adjVts[i]))
				return false;
		}
		return true;
	}

	bool IsTransitionEdge(const CP_Edge *pEdge) {

		CP_Vertex *pStartVt = pEdge->m_pStartVt;
		CP_Vertex *pEndVt = pEdge->m_pEndVt;
		CP_Edge *pEdgeTmp = const_cast<CP_Edge*>(pEdge);
		CP_Loop *pLoop0 = pEdgeTmp->GetHalf(0)->m_pLoop;
		CP_Loop *pLoop1 = pEdgeTmp->GetHalf(1)->m_pLoop;

		if (pLoop0->GetHalfNumber() == 3 || pLoop1->GetHalfNumber() == 3)
			return IsTransitionVert(pEdge, pStartVt) || IsTransitionVert(pEdge, pEndVt);
		return IsTransitionVert(pEdge, pStartVt) && IsTransitionVert(pEdge, pEndVt);
	}

	bool IsTransitionVert(const CP_Edge *pEdgeCandidate, const CP_Vertex *pEdgeEndVt) {
		std::vector<double> vecEdgesCurvature;

		CP_Edge *pEdge = const_cast<CP_Edge*>(pEdgeCandidate);
		CP_Loop *pLoop0 = pEdge->GetHalf(0)->m_pLoop;
		CP_Loop *pLoop1 = pEdge->GetHalf(1)->m_pLoop;

		// Find edges connected continuity
		for (int i = 0; i < pEdgeEndVt->GetOutHalfNumber(); ++i)	{
			CP_Half *pHalfI = pEdgeEndVt->GetOutHalf(i);
			CP_Edge *pEdge0 = pEdgeEndVt->GetOutHalf(i)->m_pEdge;
			if (pEdge0 == pEdgeCandidate)
				continue;
			
			if (pHalfI->m_pLoop == pLoop0 || pHalfI->m_pLoop == pLoop1 || 
				pHalfI->m_pTwin->m_pLoop == pLoop0 || pHalfI->m_pTwin->m_pLoop == pLoop1)
			{
				for (int j = i+1; j < pEdgeEndVt->GetOutHalfNumber(); ++j) {
					CP_Half *pHalfJ = pEdgeEndVt->GetOutHalf(j);
					if (pHalfJ->m_pLoop == pLoop0 || pHalfJ->m_pLoop == pLoop1 || 
						pHalfJ->m_pTwin->m_pLoop == pLoop0 || pHalfJ->m_pTwin->m_pLoop == pLoop1)
					{
						CP_Edge *pEdge1 = pHalfJ->m_pEdge;
						if (pEdge1 == pEdgeCandidate)
							continue;

						if (topo_geometric::IsEdgesC2Continuity(pEdge0, pEdge1, pEdgeEndVt))
							return true;
					}
				}
			}

		}
		return false;
	}

	// 此处只获取第一对找到的光滑连接边对
	bool GetC2ContinuityEdgesAt(CP_Edge **ppEdge0, CP_Edge **ppEdge1, CP_Vertex *pVt, CP_Edge *pTransitionEdge) {
		for (int i = 0; i < pVt->GetOutHalfNumber(); ++i) {
			CP_Edge *pEdgeI = pVt->GetOutHalf(i)->m_pEdge;
			if (pEdgeI == pTransitionEdge)
				continue;

			for (int j = i+1; j < pVt->GetOutHalfNumber(); ++j) {
				CP_Edge *pEdgeJ = pVt->GetOutHalf(j)->m_pEdge;
				if (pEdgeJ == pTransitionEdge)
					continue;

				if (topo_geometric::IsEdgesC2Continuity(pEdgeI, pEdgeJ, pVt)) {
					*ppEdge0 = pEdgeI;
					*ppEdge1 = pEdgeJ;
					return true;
				}
			}
		}
		return false;
	}

	bool HasHalfInLoop( const CP_Loop *pLoop, const CP_Half *pHalf )
	{
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i)
			if (pLoop->GetHalf(i) == pHalf)
				return true;
		return false;
	}

	bool HasTwinHalfInLoop (const CP_Loop *pLoop, const CP_Half *pHalf) {
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i)
			if (pLoop->GetHalf(i) == pHalf->m_pTwin)
				return true;
		return false;
	}

	// 连接合法性检测算法
	bool CanNotBeConnected(CP_Half * half0, CP_Half * half1)
	{
		if(half1->m_pTwin == half0) return true;
		CP_LoopExt* loop = (CP_LoopExt *)(half1->m_pLoop);
		if(loop->m_adjFaceNum < 0) return true;
		if(half1->m_pLoop->GetFirstHalf() != half1) return true;
		if(half0->m_pTwin->m_pLoop == half1->m_pTwin->m_pLoop) return true;
		return false;
	}	

	CP_Vector3D CalcLoopNormal(CP_Loop *pLoop) {
		/*if (pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE) {
			return CP_Vector3D(0, 0, 0);
		}*/
		CP_Vertex *pVt0 = pLoop->GetFirstHalf()->m_pStartVt;
		CP_Vertex *pVt1 = pLoop->GetFirstHalf()->m_pEndVt;
		CP_Vertex *pVt2 = pLoop->GetHalf(1)->m_pEndVt;
		CP_Point3D pt0 = *pVt0->m_pPoint;
		CP_Point3D pt1 = *pVt1->m_pPoint;
		CP_Point3D pt2 = *pVt2->m_pPoint;

		if (pVt2 == pVt0)
			pt2 = pLoop->GetHalf(1)->m_pEdge->m_pCurve3D->GetPoint(0.5);
			
		CP_Vector3D vec0(pt1 - pt0);
		CP_Vector3D vec1(pt2 - pt1);
		CP_Vector3D normalVec = vec0 ^ vec1;
		normalVec.Normalize();
		return normalVec;
	}

	CP_Vector3D CalcPlaneNormal(const vector<CP_LineSegment3D> &plane) {
		CP_Point3D pt0 = plane[0].m_startPt;
		CP_Point3D pt1 = plane[0].m_endPt;
		CP_Point3D pt2 = plane[1].m_endPt;
		CP_Vector3D vec0(pt1 - pt0);
		CP_Vector3D vec1(pt2 - pt1);
		CP_Vector3D normalVec = vec0 ^ vec1;
		normalVec.Normalize();
		return normalVec;
	}

	void GetAdjLoops(CP_Loop *pLoop, set<CP_LoopExt *> *pVecResult) {
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_Half *pHalf = pLoop->GetHalf(i);
			pVecResult->insert((CP_LoopExt *)pHalf->m_pTwin->m_pLoop);
		}
	}

	void ReverseLoop(CP_Loop *pLoop) {
		vector<CP_Half *> halfArr;
		int halfSize = pLoop->GetHalfNumber();
		for(int i = halfSize-1; i >= 0; i--)
		{
			CP_Half * half = pLoop->GetHalf(i);
			halfArr.push_back(half->m_pTwin);
		}
		pLoop->RemoveAllHalf();
		for(int i = 0; i < halfSize; i++)
		{
			CP_Half * half = halfArr[i];
			pLoop->AddHalf(half);
			half->m_pLoop = pLoop;
		}
	}

	void Polygonization3D(CP_Loop *pLoop, vector<CP_Point3D> &result) {
		const int numSeg = 20;
		CP_Point3D pt;
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_Half *pHalf = pLoop->GetHalf(i);
			CP_Edge *pEdge = pHalf->m_pEdge;
			pt = *pHalf->m_pStartVt->m_pPoint;
			result.push_back(pt);
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
				if (base_geometric::IsSamePoint(&pEdge->m_pCurve3D->GetStartingPoint(), pHalf->m_pStartVt->m_pPoint)) {
					for (int k = 1; k < numSeg; ++k) {
						pt = pEdge->m_pCurve3D->GetPoint((double)k/numSeg);
						result.push_back(pt);
					}
				} else {
					for (int k = numSeg-1; k > 0; --k) {
						pt = pEdge->m_pCurve3D->GetPoint((double)k/numSeg);
						result.push_back(pt);
					}
				}
			}
		}
	}

	void Polygonization2D(CP_Loop *pLoop, vector<CP_Point3D> &result) {
		const int numSeg = 20;
		CP_Point3D pt;
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_Half *pHalf = pLoop->GetHalf(i);
			CP_Edge *pEdge = pHalf->m_pEdge;
			pt = *pHalf->m_pStartVt->m_pPoint;
			pt.m_z = 0;
			result.push_back(pt);
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
				if (base_geometric::IsSamePoint(&pEdge->m_pCurve3D->GetStartingPoint(), pHalf->m_pStartVt->m_pPoint)) {
					for (int k = 1; k < numSeg; ++k) {
						pt = pEdge->m_pCurve3D->GetPoint((double)k/numSeg);
						pt.m_z = 0;
						result.push_back(pt);
					}
				} else {
					for (int k = numSeg-1; k > 0; --k) {
						pt = pEdge->m_pCurve3D->GetPoint((double)k/numSeg);
						pt.m_z = 0;
						result.push_back(pt);
					}
				}
			}
		}
	}

	bool IsPtInPolygon2D (CP_Point3D &p, vector<CP_Point3D> &ptPolygon) 
	{ 
		int nCross = 0;
		int nCount = ptPolygon.size();
		for (int i = 0; i < nCount; i++) 
		{ 
			CP_Point3D p1 = ptPolygon[i]; 
			CP_Point3D p2 = ptPolygon[(i + 1) % nCount];
			// 求解 y=p.y 与 p1p2 的交点
			if ( p1.m_y == p2.m_y ) // p1p2 与 y=p0.y平行 
				continue;
			if ( p.m_y < min(p1.m_y, p2.m_y) ) // 交点在p1p2延长线上 
				continue; 
			if ( p.m_y >= max(p1.m_y, p2.m_y) ) // 交点在p1p2延长线上 
				continue;
			// 求交点的 X 坐标 -------------------------------------------------------------- 
			double x = (double)(p.m_y - p1.m_y) * (double)(p2.m_x - p1.m_x) / (double)(p2.m_y - p1.m_y) + p1.m_x;
			if ( x > p.m_x ) 
				nCross++; // 只统计单边交点 
		}
		// 单边交点为偶数，点在多边形之外 --- 
		return (nCross % 2 == 1); 
	}

	bool IsPolyInPoly(vector<CP_Point3D> &polygon0, vector<CP_Point3D> &polygon1) {
		// Take one point in polygon0 and test if it is inside polygon1
		return IsPtInPolygon2D(polygon0[0], polygon1);
	}

	bool IsLoopInLoop(CP_Loop *pLoop0, CP_Loop *pLoop1) {
		vector<CP_Point3D> vPoly0;
		Polygonization2D(pLoop0, vPoly0);
		vector<CP_Point3D> vPoly1;
		Polygonization2D(pLoop1, vPoly1);
		return IsPolyInPoly(vPoly0, vPoly1);
	}

	bool IsClockwise(CP_Loop *pLoop) {
		vector<CP_Point3D> poly;
		Polygonization2D(pLoop, poly);
		int maxX = 0, maxY = 0, minX = 0, minY = 0;
		int num = poly.size();
		for (int i = 1; i < num; ++i) {
			maxX = poly[i].m_x > poly[maxX].m_x ? i : maxX;
			maxY = poly[i].m_y > poly[maxY].m_y ? i : maxY;
			minX = poly[i].m_x < poly[minX].m_x ? i : minX;
			minY = poly[i].m_y < poly[minY].m_y ? i : minY;
		}
		vector<int> vIndex;
		vIndex.push_back(maxX);
		vIndex.push_back(maxY);
		vIndex.push_back(minX);
		vIndex.push_back(minY);
		sort(vIndex.begin(), vIndex.end());
		vector<int>::iterator iter_end;
		iter_end = std::unique(vIndex.begin(), vIndex.end() );
		vIndex.erase(iter_end, vIndex.end() );
		
		if (vIndex.size() == 2) {
			int i = vIndex[0];
			int j = vIndex[1];
			return ((poly[i] - poly[(i-1+num) % num]) ^ (poly[(i+1)%num] - poly[i])).m_z < 0;
		} else {
			int p0 = vIndex[0];
			int p1 = vIndex[1];
			int p2 = vIndex[2];
			return ((poly[p1] - poly[p0]) ^ (poly[p2] - poly[p1])).m_z < 0;
		}
	}


	pair<double, double> FindMeetPtParam(CP_Curve3D *pCurve0, CP_Curve3D *pCurve1, CP_Point3D *pMeetPt) {
		pair<double, double> params;
		params.first = params.second = 1;
		if (base_geometric::HasSameProjectionPt(&pCurve0->GetStartingPoint(), pMeetPt, SAME_POINT_THRESHOLD))
			params.first = 0;
		if (base_geometric::HasSameProjectionPt(&pCurve1->GetStartingPoint(), pMeetPt, SAME_POINT_THRESHOLD))
			params.second = 0;
		return params;
	}

	double GetCosValue2D(CP_Curve3D *pCurve0, CP_Curve3D *pCurve1) {
		ASSERT(pCurve0 && pCurve1);
		CP_Vector3D vec0 = (pCurve0->GetStartingPoint()) - (pCurve0->GetEndingPoint());
		CP_Vector3D vec1 = (pCurve1->GetStartingPoint()) - (pCurve1->GetEndingPoint());
		vec0.m_z = vec1.m_z = 0;
		double cosvalue = vec0 * vec1 / sqrt((vec0*vec0) * (vec1*vec1));
		if (dynamic_cast<CP_LineSegment3D *>(pCurve0) != NULL && dynamic_cast<CP_LineSegment3D *>(pCurve1) != NULL)
			return fabs(cosvalue);

		const int secSize = 9;
		double secArr[secSize] = {0.1, 0.2, 0.3, 0.4 ,0.5, 0.6, 0.7, 0.8, 0.9};
		vector<double> vSec(secArr, secArr + secSize);
		vector<CP_Vector3D> vTangent0;
		vector<CP_Vector3D> vTangent1;

		for (int i = 0; i < secSize; ++i) {
			CP_Vector3D vec = pCurve0->GetTangent(vSec[i]);
			vec.m_z = 0; 
			vTangent0.push_back(vec);
		}
		if (cosvalue < 0) std::reverse(vSec.begin(), vSec.end());
		for (int i = 0; i < secSize; ++i) {
			CP_Vector3D vec = pCurve1->GetTangent(vSec[i]);
			vec.m_z = 0;
			vTangent1.push_back(vec);
		}
		double minCosValue = 1.0;
		for (int i = 0; i < secSize; ++i) {
			double cosvalue = fabs(vTangent0[i]*vTangent1[i]/vTangent0[i].GetLength()/vTangent1[i].GetLength());
			minCosValue = min(minCosValue, cosvalue);
		}

		return minCosValue;
	}

	// 计算两条边的CosValue值
	double GetCosValue2D(CP_Edge * edge0, CP_Edge * edge1)
	{
		return GetCosValue2D(edge0->m_pCurve3D, edge1->m_pCurve3D);
	}

	double GetCosValue3D(CP_Edge * edge0, CP_Edge * edge1) {
		return GetCosValue3D(edge0->m_pCurve3D, edge1->m_pCurve3D);
	}

	double GetCosValue3D(CP_Curve3D *pCurve0, CP_Curve3D *pCurve1) {
		CP_Vector3D vec0 = (pCurve0->GetStartingPoint()) - (pCurve0->GetEndingPoint());
		CP_Vector3D vec1 = (pCurve1->GetStartingPoint()) - (pCurve1->GetEndingPoint());
		double cosvalue = vec0 * vec1 / sqrt((vec0*vec0) * (vec1*vec1));
		if (dynamic_cast<CP_LineSegment3D *>(pCurve0) != NULL && dynamic_cast<CP_LineSegment3D *>(pCurve1) != NULL)
			return fabs(cosvalue);

		const int secSize = 5;
		double secArr[secSize] = {0.1, 0.3, 0.5, 0.7, 0.9};
		vector<double> vSec(secArr, secArr + secSize);
		vector<CP_Vector3D> vTangent0;
		vector<CP_Vector3D> vTangent1;

		for (int i = 0; i < secSize; ++i) {
			CP_Vector3D vec = pCurve0->GetTangent(vSec[i]);
			vTangent0.push_back(vec);
		}
		if (cosvalue < 0) std::reverse(vSec.begin(), vSec.end());
		for (int i = 0; i < secSize; ++i) {
			CP_Vector3D vec = pCurve1->GetTangent(vSec[i]);
			vTangent1.push_back(vec);
		}
		double sum = 0.0;
		for (int i = 0; i < secSize; ++i) {
			double cosvalue = fabs(vTangent0[i]*vTangent1[i]/vTangent0[i].GetLength()/vTangent1[i].GetLength());
			// Modify here
			sum += cosvalue;
		}

		return sum / secSize;
	}

	// Get cos value between loop0 and loop1 by calculating the cos value between edges in loops and loop1
	double GetCosValue2D(const CP_Loop * loop0, const CP_Loop * loop1)
	{
		int size0 = loop0->GetHalfNumber();
		int size1 = loop1->GetHalfNumber();
		if(size0 == 1 && size1 == 1) return 1.0;

		std::vector<double> cosArr;
		//double retvalue = -1;
		for (int i = 0; i < size0; i++)
		{
			for (int j = 0; j < size1; j++)
			{
				CP_Half *half0 = loop0->GetHalf(i);
				CP_Half *half1 = loop1->GetHalf(j);
				double cosvalue = GetCosValue2D(half0->m_pEdge, half1->m_pEdge);
				cosArr.push_back(cosvalue);
			}
		}
		
		return GetAverageCosValue(cosArr);
	}

	// Get cos value of loop0 and loop1 by calculating the cos value between polygons of loop0 and loop1
	// When loop0 and loop1 are both constructed by line segment, this method is same to GetCosValue(loop0, loop1)
	double GetPolygonCosValue(CP_Loop *pLoop0, CP_Loop *pLoop1) {
		vector<CP_LineSegment3D> vLineSegs0;
		vector<CP_LineSegment3D> vLineSegs1;

		for (int i = 0; i < pLoop0->GetHalfNumber(); ++i) {
			CP_Edge *pEdge = pLoop0->GetHalf(i)->m_pEdge;
			CP_Curve3D *pCurve = pEdge->m_pCurve3D;
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
				CP_Point3D startPt = pCurve->GetStartingPoint();
				CP_Point3D midPt = pCurve->GetPoint(0.5);
				CP_Point3D endPt = pCurve->GetEndingPoint();
				vLineSegs0.push_back(CP_LineSegment3D(startPt, midPt));
				vLineSegs0.push_back(CP_LineSegment3D(midPt, endPt)); 
			} else {
				vLineSegs0.push_back(*(CP_LineSegment3D *)pCurve);
				vLineSegs0.push_back(*(CP_LineSegment3D *)pCurve);
			}
		}
		
		for (int i = 0; i < pLoop1->GetHalfNumber(); ++i) {
			CP_Edge *pEdge = pLoop1->GetHalf(i)->m_pEdge;
			CP_Curve3D *pCurve = pEdge->m_pCurve3D;
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
				CP_Point3D startPt = pCurve->GetStartingPoint();
				CP_Point3D midPt = pCurve->GetPoint(0.5);
				CP_Point3D endPt = pCurve->GetEndingPoint();
				vLineSegs1.push_back(CP_LineSegment3D(startPt, midPt));
				vLineSegs1.push_back(CP_LineSegment3D(midPt, endPt)); 
			} else {
				vLineSegs1.push_back(*(CP_LineSegment3D *)pCurve);
				vLineSegs1.push_back(*(CP_LineSegment3D *)pCurve);
			}
		}

		return GetPolygonCosValue(vLineSegs0, vLineSegs1);
	}

	double GetPolygonCosValue(CP_Loop *pLoop0, vector<CP_LineSegment3D> &vLineSegs1) {
		vector<CP_LineSegment3D> vLineSegs0;
		for (int i = 0; i < pLoop0->GetHalfNumber(); ++i) {
			CP_Edge *pEdge = pLoop0->GetHalf(i)->m_pEdge;
			CP_Curve3D *pCurve = pEdge->m_pCurve3D;
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
				CP_EdgeExt *pEdgeExt = static_cast<CP_EdgeExt*>(static_cast<CP_EdgeExt*>(pEdge)->GetParent());
				CP_Point3D startPt = pCurve->GetStartingPoint();
				CP_Point3D endPt = pCurve->GetEndingPoint();
				if (pEdgeExt->HasProperty(PROPERTY_NAME_IS_SPLIT_IN_MID)) {
					CP_Point3D midPt = pCurve->GetPoint(0.5);
					vLineSegs0.push_back(CP_LineSegment3D(startPt, midPt));
					vLineSegs0.push_back(CP_LineSegment3D(midPt, endPt)); 
				} else {
					vLineSegs0.push_back(CP_LineSegment3D(startPt, endPt));
					vLineSegs0.push_back(CP_LineSegment3D(startPt, endPt)); 
				}
				
			} else {
				vLineSegs0.push_back(*(CP_LineSegment3D *)pCurve);
				vLineSegs0.push_back(*(CP_LineSegment3D *)pCurve);
			}
		}
		return GetPolygonCosValue(vLineSegs0, vLineSegs1);
	}

	double GetPolygonCosValue(vector<CP_LineSegment3D> &vLineSegs0, vector<CP_LineSegment3D> &vLineSegs1) {
		std::vector<double> cosArr;
		for (unsigned int i = 0; i < vLineSegs0.size(); ++i) {
			for (unsigned int j = 0; j < vLineSegs1.size(); ++j) {
				double cosValue = GetCosValue2D(&vLineSegs0.at(i), &vLineSegs1.at(j));
				cosArr.push_back(cosValue);
			}
		}
		return GetAverageCosValue(cosArr);
	}

	double GetPolygonCosValue(vector<CP_LineSegment3D> &vLineSegs0, CP_Loop *pLoop) {
		vector<CP_LineSegment3D> vLineSegs1;
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_Edge *pEdge = pLoop->GetHalf(i)->m_pEdge;
			CP_Curve3D *pCurve = pEdge->m_pCurve3D;
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
				CP_Point3D startPt = pCurve->GetStartingPoint();
				CP_Point3D midPt = pCurve->GetPoint(0.5);
				CP_Point3D endPt = pCurve->GetEndingPoint();
				vLineSegs1.push_back(CP_LineSegment3D(startPt, midPt));
				vLineSegs1.push_back(CP_LineSegment3D(midPt, endPt)); 
			} else {
				vLineSegs1.push_back(*(CP_LineSegment3D *)pCurve);
				vLineSegs1.push_back(*(CP_LineSegment3D *)pCurve);
			}
		}
		return GetPolygonCosValue(vLineSegs0, vLineSegs1); 
	}

	double GetAverageCosValue(std::vector<double> &cosArr) {
		std::sort(cosArr.begin(),cosArr.end());

		int size = cosArr.size()/2;
		for(std::vector<double>::iterator iter = cosArr.begin();iter != cosArr.end()-size;)
		{
			iter = cosArr.erase(iter);
		}
		size = cosArr.size();
		double sum = std::accumulate(cosArr.begin(),cosArr.end(),0.0);
		return sum/size;
	}

	void OutputLoop(CP_Loop *pLoop) {
		CLog::log("Out put loop");
		for (int k = 0; k < pLoop->GetHalfNumber(); ++k) {
			CP_Half *pHalf = pLoop->GetHalf(k);
			CP_Point3D midPt = pHalf->m_pEdge->m_pCurve3D->GetPoint(0.5);
			CLog::log("(%f, %f, %f)-(%f, %f, %f)", 
				pHalf->m_pStartVt->m_pPoint->m_x, pHalf->m_pStartVt->m_pPoint->m_y, pHalf->m_pStartVt->m_pPoint->m_z, 
				midPt.m_x, midPt.m_y, midPt.m_z);
		}
	}



	bool HasRedLine(CP_Loop *pLoop) {
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_Half *pHalf = pLoop->GetHalf(i);
			CP_EdgeExt* pEdge = static_cast<CP_EdgeExt*>(pHalf->m_pEdge);
			if (pEdge->HasProperty(PROPERTY_NAME_ADDITIONAL_LINE))
				return true;
		}
		return false;
	}

	CP_Vector3D GetCurveTangentAtPt(CP_Curve3D *pCurve, CP_Point3D pt) {
		if (base_geometric::IsSamePoint(&pCurve->GetStartingPoint(), &pt)) {
			return pCurve->GetTangent(0);
		}
		return pCurve->GetTangent(1);
	}
}