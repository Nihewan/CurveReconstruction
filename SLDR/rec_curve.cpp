#include "stdafx.h"
#include "rec_curve.h"
#include "base_geometricoper.h"
#include "CP_TopologyExt.h"
#include "propertiesnames.h"

namespace rec_curve {


	void ReconstructBsplineCtrlPts(CP_Edge *pBSplineEdge) {
		CP_Nurbs *pBSpline = (CP_Nurbs *)pBSplineEdge->m_pCurve3D;
		CP_Point3D *pStartPt = pBSplineEdge->m_pStartVt->m_pPoint;
		CP_Point3D *pEndPt = pBSplineEdge->m_pEndVt->m_pPoint;

		// Reconstruct control points
		for (int i = 0; i < pBSpline->GetCtrlNumber(); ++i) {
			CP_Point3D *pPt = pBSpline->GetCtrlPoint(i);
			if (base_geometric::HasSameProjectionPt(pPt, pStartPt, SAME_POINT_THRESHOLD)) {
				pPt->m_z = pStartPt->m_z;
			} else {
				CP_Vector3D v(*pPt - *pStartPt);
				pPt->m_z = pStartPt->m_z + base_geometric::GetVectorZ(v, pBSpline->m_normal);
			}
		}
	}

	void ReconstructArcUCS(CP_Edge *pArcEdge) {
		CP_Arc *pArc = (CP_Arc *)pArcEdge->m_pCurve3D;

		// Reconstruct the local coordinate system in the arc center
		CP_Vector3D n = pArc->m_ucs.m_axisZ;
		CP_Point3D *pPt = pArcEdge->m_pStartVt->m_pPoint;
		pArc->m_ucs.m_origin.m_z = base_geometric::GetVectorZ(pArc->GetCenter() - *pPt, n) + pPt->m_z;
		pArc->m_ucs.m_axisX.m_z = base_geometric::GetVectorZ(pArc->m_ucs.m_axisX, n);
		pArc->m_ucs.m_axisY.m_z = base_geometric::GetVectorZ(pArc->m_ucs.m_axisY, n);
		// CLog::log("X * Y = %f", pArc->m_ucs.m_axisX * pArc->m_ucs.m_axisY);  
	}

	void ReconstructLineSegEndPts(CP_Edge *pLineSegEdge) {
		CP_LineSegment3D *pLine = (CP_LineSegment3D *)pLineSegEdge->m_pCurve3D;

		// Reconstruct the curve's start and end point
		if (base_geometric::HasSameProjectionPt(&pLine->m_startPt, pLineSegEdge->m_pStartVt->m_pPoint, SAME_POINT_THRESHOLD)) {
			pLine->m_startPt.m_z = pLineSegEdge->m_pStartVt->m_pPoint->m_z;
			pLine->m_endPt.m_z = pLineSegEdge->m_pEndVt->m_pPoint->m_z;
		}
		else {
			pLine->m_endPt.m_z = pLineSegEdge->m_pStartVt->m_pPoint->m_z;
			pLine->m_startPt.m_z = pLineSegEdge->m_pEndVt->m_pPoint->m_z;
		}
	}


	void RecSingleEdgeNormal(CP_Edge *pEdgeInput) {
		if (pEdgeInput->GetCurveType() == TYPE_CURVE_LINESEGMENT)
			return ;
		CP_Vector3D n;
		int i;
		for (i = 0; i < 2; ++i) {
			CP_FaceExt *pFace = (CP_FaceExt *)pEdgeInput->GetHalf(i)->m_pLoop->m_pParentFace;
			if (pFace->GetSurfaceType() == TYPE_SURFACE_PLANE) {

				n = pFace->m_surface->GetNormal(0, 0);
				break;
			}
		}
		if (i == 2) {
			CP_EdgeExt *pEdge = (CP_EdgeExt *)pEdgeInput;
			CP_Vector3D v0 = *pEdge->m_pEndVt->m_pPoint - *pEdge->m_pStartVt->m_pPoint;
			ASSERT(pEdge->HasProperty(PROPERTY_NAME_MID_VERTEX));
			CP_Vector3D v1 = *((CP_Vertex *)pEdge->GetCObjPtProperties(PROPERTY_NAME_MID_VERTEX))->m_pPoint
				- *pEdge->m_pStartVt->m_pPoint;
			n = v0 ^ v1;
		}
		CP_Vector3D start2End = *pEdgeInput->m_pEndVt->m_pPoint - *pEdgeInput->m_pStartVt->m_pPoint;
		start2End.Normalize();
		CP_Vector3D tmpVec = start2End * (start2End * n);
		n -= tmpVec;
		n.Normalize();

		if (pEdgeInput->GetCurveType() == TYPE_CURVE_NURBS) {
			static_cast<CP_Nurbs *>(pEdgeInput->m_pCurve3D)->m_normal = n;
		} else if (pEdgeInput->GetCurveType() == TYPE_CURVE_ARC) {
			static_cast<CP_Arc *>(pEdgeInput->m_pCurve3D)->m_ucs.m_axisZ = n;
		}
	}

	void RecBodyEdgeNormal(CP_Body *pBody) {
		for (int i = 0; i < pBody->GetEdgeNumber(); ++i) {
			CP_Edge *pEdge = pBody->GetEdge(i);
			RecSingleEdgeNormal(pEdge);
		}
	}

	void ReconstructBodyEdge(CP_Body *pBody) {
		RecBodyEdgeNormal(pBody);

		for (int ei = 0; ei < pBody->GetEdgeNumber(); ++ei) {
			CP_Edge *pEdge = pBody->GetEdge(ei);
			if (pEdge->GetCurveType() == TYPE_CURVE_LINESEGMENT) {
				ReconstructLineSegEndPts(pEdge);
			} else if (pEdge->GetCurveType() == TYPE_CURVE_ARC) {
				ReconstructArcUCS(pEdge);
			} else if (pEdge->GetCurveType() == TYPE_CURVE_NURBS) {
				ReconstructBsplineCtrlPts(pEdge);
			}
		}
	}

	
}