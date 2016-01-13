#include "stdafx.h"

#include "est_refplane.h"
#include "CLog.h"
#include "est_vtattroper.h"
#include "topo_geometricoper.h"
#include "est_comparator.h"
#include "surface_basic.h"
#include <queue>

namespace est_refplane {

	void CalcBodyWeight( CP_Body *pBody, vector<vector<CP_LineSegment3D>> &vRefPlanes )
	{
		// Initialize coordinate panes
		CP_BodyExt *pBodyExt = (CP_BodyExt *)pBody;

		// Calculate weight
		int weight = 0;
		for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
			CP_LoopExt *pLoop = (CP_LoopExt *)pBody->GetFace(i)->GetLoop(0);
			//if (pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE) 
			//	weight += FindCurvedFaceParaRefPlane(pLoop, vRefPlanes);
			//else 
			weight += FindPlaneParaRefPlane(pLoop, vRefPlanes);
		}
		weight /= pBody->GetFaceNumber();
		pBodyExt->SetProperties(PROPERTY_NAME_WEIGHT, weight);
		// AdjustRefPlaneByPriority(pBody, vRefPlanes);
		return ;
	}

	void InitBodyRefPlanes(CP_Body *pBody, vector<vector<CP_LineSegment3D>> &vRefPlanes) {
		CP_BodyExt *pBodyExt = (CP_BodyExt *)pBody;
		CP_Vector3D coorSys[3] = {pBodyExt->m_ucs.m_axisX, pBodyExt->m_ucs.m_axisY, pBodyExt->m_ucs.m_axisZ};
		InitRefPlanesByCooSys(coorSys, vRefPlanes);
	}

	void InitRefPlanesByCooSys(CP_Vector3D coorSys[3], vector<vector<CP_LineSegment3D>> &vResultRefPlanes) {
		const double X_OFFSET_TOLER = 0.2;
		for (int i = 0; i < 3; ++i) {
			vector<CP_LineSegment3D> vRefPlane;
			InitPlaneByVecs(vRefPlane, &coorSys[i], &coorSys[(i+1)%3]);
			vResultRefPlanes.push_back(vRefPlane);
		}

		// The last reference plane is construct by Y-axis and v(1,0,0)

		vector<CP_LineSegment3D> vRefPlane;
		InitPlaneByVecs(vRefPlane, &CP_Vector3D(1, 0, 0), &coorSys[1]);
		vResultRefPlanes.push_back(vRefPlane);
	}

	void InitPlaneByVecs(vector<CP_LineSegment3D> &vRefPlane, const CP_Vector3D *pVec0, const CP_Vector3D *pVec1 )
	{
		CP_Vector3D vecDiagonal = *pVec0 + *pVec1;
		CP_Point3D pts[4] = {CP_Point3D(0, 0, 0), 
			CP_Point3D(pVec0->m_x, pVec0->m_y, pVec0->m_z),
			CP_Point3D(vecDiagonal.m_x, vecDiagonal.m_y, vecDiagonal.m_z), 
			CP_Point3D(pVec1->m_x, pVec1->m_y, pVec1->m_z)};

		for (int i = 0; i < 4; ++i) {
			vRefPlane.push_back(CP_LineSegment3D(pts[i], pts[(i+1)%4]));
		}
	}

	int FindPlaneParaRefPlane(CP_Loop *pLoop, vector<vector<CP_LineSegment3D>> &vRefPlanes) {
		const double CURVED_FACE_WEIGHT_RATIO = 0.90;

		int maxValue = 0;
		vector<CP_LineSegment3D> *pMatchPane;
		CLog::log("The original loop:");
		topo_geometric::OutputLoop(pLoop);
		vector<CP_Loop *>::iterator iterExc;

		for (unsigned int i = 0; i < vRefPlanes.size(); ++i) {
			int cosValue = (int)(topo_geometric::GetPolygonCosValue(pLoop, vRefPlanes[i]) * 100);
			if (cosValue > maxValue) {
				maxValue = cosValue;
				pMatchPane = &vRefPlanes[i];
			}
		}

		// Curved face has lower weight
		if (pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE)
			maxValue = static_cast<int>(maxValue*CURVED_FACE_WEIGHT_RATIO);
		((CP_LoopExt *)pLoop)->SetProperties(PROPERTY_NAME_WEIGHT, maxValue);
		((CP_LoopExt *)pLoop)->SetProperties(PROPERTY_NAME_MATCH_PANE_NORMAL, 
			new CP_Vector3D(topo_geometric::CalcPlaneNormal(*pMatchPane)));

		return maxValue;
	}

	// Find line segments' parallel plane
	pair<int, vector<CP_LineSegment3D>> FindParaRefPlane(vector<CP_LineSegment3D> &vLineSeg, vector<vector<CP_LineSegment3D>> &vRefPlanes) {
		ASSERT(vLineSeg.size() > 0);
		int maxValue = 0;
		vector<CP_LineSegment3D> *pMatchPane = NULL;
		for (unsigned int i = 0; i < vRefPlanes.size(); ++i) {
			int cosValue = (int)(topo_geometric::GetPolygonCosValue(vLineSeg, vRefPlanes[i]) * 100);
			if (cosValue > maxValue) {
				maxValue = cosValue;
				pMatchPane = &vRefPlanes[i];
			}
		}
		return pair<int, vector<CP_LineSegment3D>>(maxValue, *pMatchPane);
	}

	// Find curved face's parallel reference plane by splitting the surface.
	int FindCurvedFaceParaRefPlane(CP_Loop *pLoop, vector<vector<CP_LineSegment3D>> &vRefPlanes) {
		ASSERT(pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE);
		CP_CoonsSurface *pCoonsFace = (CP_CoonsSurface *)pLoop->m_pParentFace->m_surface;
		vector<double> usegs;
		vector<double> vsegs;
		pCoonsFace->SplitSurface(CURVED_FACE_SPLIT_ERR, usegs, vsegs);
		usegs.push_back(1);
		usegs.insert(usegs.begin(), 0);
		vsegs.push_back(1);
		vsegs.insert(vsegs.begin(), 0);

		int sum = 0;
		vector<CP_LineSegment3D> *pMatchPane = NULL;
		int maxValue = 0;
		for (unsigned int i = 0; i < usegs.size()-1; ++i) {
			for (unsigned int j = 0; j < vsegs.size()-1; ++j) {
				CP_Point3D pt0 = pCoonsFace->GetPoint(usegs[i], vsegs[j]);
				CP_Point3D pt1 = pCoonsFace->GetPoint(usegs[i+1], vsegs[j]);
				CP_Point3D pt3 = pCoonsFace->GetPoint(usegs[i+1], vsegs[j+1]);
				CP_Point3D pt2 = pCoonsFace->GetPoint(usegs[i], vsegs[j+1]);
				vector<CP_LineSegment3D> grid;
				surface_basic::Init4LineSegPatch(grid, pt0, pt1, pt2, pt3);
				pair<int, vector<CP_LineSegment3D>> valuePair = FindParaRefPlane(grid, vRefPlanes);
				if (valuePair.first > maxValue) {
					maxValue = valuePair.first;
					pMatchPane = &valuePair.second;
				}
				sum += valuePair.first;
			}
		}
		int weight = sum / (usegs.size()-1) / (vsegs.size()-1);
		((CP_LoopExt *)pLoop)->SetProperties(PROPERTY_NAME_WEIGHT, weight);
		((CP_LoopExt *)pLoop)->SetProperties(PROPERTY_NAME_MATCH_PANE_NORMAL, 
			new CP_Vector3D(topo_geometric::CalcPlaneNormal(*pMatchPane)));
		return weight;
	}

	CP_Vector3D CalcParaPlaneNormal(vector<CP_LineSegment3D> &matchPlane)
	{
		return topo_geometric::CalcPlaneNormal(matchPlane);
	}


}