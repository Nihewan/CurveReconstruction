#include "stdafx.h"
#include "FaceClassification.h"
#include "facecls_utils.h"
#include <queue>
#include "CLog.h"
#include "topo_geometricoper.h"
#include "surface_basic.h"
#include "CTimer.h"


void CFaceClassification::ClassifyFace() {
	InitEdgeUsedProperty(m_pAsmbBody->GetOriginalBody());

	CTimer timer;
	timer.Start();
	ClassifyByLineSeg(m_pAsmbBody->GetOriginalBody());
	ClassifyByTransitionEdge(m_pAsmbBody->GetOriginalBody());
	ClassifyRecursion(m_pAsmbBody->GetOriginalBody());
	ClassifyByProbability(m_pAsmbBody->GetOriginalBody());
	InitChildBodySurface();
	ClassifyPseudoFace();
	RemoveEdgeUsedProperty(m_pAsmbBody->GetOriginalBody());
	CLog::log("Face classification time %f ms", timer.End());
}

// Classify faces adjacent to silhouette as curved face
void CFaceClassification::ClassifyByTransitionEdge(CP_Body *pBody) {
	for (int i = 0; i < pBody->GetEdgeNumber(); ++i) {
		CP_Edge *pEdge = pBody->GetEdge(i);

		if (topo_geometric::IsTransitionEdge(pEdge)) {
			for (int j = 0; j < 2; j++) {
				CP_Face *pFace = pEdge->GetHalf(j)->m_pLoop->m_pParentFace;
				if (pFace->GetSurfaceType() == TYPE_SURFACE_UNDEFINED) {
					pFace->m_surface = new CP_CoonsSurface;
					surface_basic::InitCoonsSurface(pFace);
					SetCurveEdgeOfFaceUsed(pFace);
				}
			}
		}
	}
}

void CFaceClassification::ClassifyByProbability(CP_Body *pBody) {
	priority_queue<CP_FaceExt *, vector<CP_FaceExt *>, ProbCmp> queueFace;
	for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
		CP_FaceExt *pFace = (CP_FaceExt *)pBody->GetFace(i);
		if (pFace->GetSurfaceType() == TYPE_SURFACE_UNDEFINED) {
			double prob = facecls_utils::CalcCurvedProbability(pFace);
			pFace->SetProperties(PROPERTY_NAME_PROBABILITY, static_cast<int>(prob * 100));
			queueFace.push(pFace);
		}
	}

	while (!queueFace.empty()) {
		CP_FaceExt *pFace = queueFace.top();
		CLog::log("Face prob = %d", pFace->GetIntegralProperties(PROPERTY_NAME_PROBABILITY));
		if (pFace->GetSurfaceType() == TYPE_SURFACE_UNDEFINED){
			pFace->m_surface = new CP_CoonsSurface;
			surface_basic::InitCoonsSurface(pFace);
			SetCurveEdgeOfFaceUsed(pFace);
			ClassifyAdjFaceRecursion(pFace);
		}
		queueFace.pop();
	}
}

void CFaceClassification::ClassifyByLineSeg(CP_Body *pBody) {
	for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
		CP_Face *pFace = pBody->GetFace(i);
		if (pFace->GetSurfaceType() == TYPE_SURFACE_UNDEFINED)
			ClassifyFaceAsPlane(pFace);
	}
}

void CFaceClassification::ClassifyRecursion(CP_Body *pBody) {
	for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
		CP_Face *pFace = pBody->GetFace(i);
		if (pFace->GetSurfaceType() != TYPE_SURFACE_UNDEFINED)
			ClassifyAdjFaceRecursion(pFace);
	}
}

// Classify face as plane when all edges is used or line segment
bool CFaceClassification::ClassifyFaceAsPlane(CP_Face *pFace) {
	for (int j = 0; j < pFace->GetLoopNumber(); ++j) {
		CP_Loop *pLoop = pFace->GetLoop(j);
		int k;
		for (k = 0; k < pLoop->GetHalfNumber(); ++k) {
			CP_EdgeExt *pEdge = (CP_EdgeExt *)pLoop->GetHalf(k)->m_pEdge;

			// Neither line segment nor curve edge used
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT &&
				static_cast<CP_EdgeExt *>(pEdge->GetParent())->GetIntegralProperties(PROPERTY_NAME_IS_EDGEUSED) == 0 ) {
					break;				
			}
		}
		// If all edge are line segment, the face is planar. 
		if (k == pLoop->GetHalfNumber()) {
			if (pFace->GetSurfaceType() == TYPE_SURFACE_UNDEFINED) {
				pFace->m_surface = new CP_Plane(pLoop->GetHalf(0)->m_pStartVt->m_pPoint, CP_Vector3D());
				return true;
			}
		}
	}
	return false;
}

// Initialize curve edge "used" property
void CFaceClassification::InitEdgeUsedProperty(CP_Body *pBody) {
	for (int i = 0; i < pBody->GetEdgeNumber(); ++i) {
		if (pBody->GetEdge(i)->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
			CP_EdgeExt *pEdge = (CP_EdgeExt *)(pBody->GetEdge(i));
			pEdge->SetProperties(PROPERTY_NAME_IS_EDGEUSED, 0);
		}
	}
}

void CFaceClassification::SetCurveEdgeOfFaceUsed(const CP_Face *pFace) {
	for (int i = 0; i < pFace->GetLoopNumber(); ++i) {
		CP_Loop *pLoop = pFace->GetLoop(i);
		for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
			CP_EdgeExt *pEdge = (CP_EdgeExt *)pLoop->GetHalf(j)->m_pEdge;
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
				pEdge->SetProperties(PROPERTY_NAME_IS_EDGEUSED, 1);
			}
		}
	}
}

void CFaceClassification::RemoveEdgeUsedProperty(CP_Body *pBody) {
	for (int i = 0; i < pBody->GetEdgeNumber(); ++i) {
		if (pBody->GetEdge(i)->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
			CP_EdgeExt *pEdge = (CP_EdgeExt *)(pBody->GetEdge(i));
			pEdge->RemoveProperty(PROPERTY_NAME_IS_EDGEUSED);
		}
	}
}

// Classify face adjacent to face input
void CFaceClassification::ClassifyAdjFaceRecursion(const CP_Face *pFace) {
	for (int i = 0; i < pFace->GetLoopNumber(); ++i) {
		// Each loop in face
		CP_Loop *pLoop = pFace->GetLoop(i);
		for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
			CP_Half *pHalf = pLoop->GetHalf(j);
			// Each edge in loop
			CP_Edge *pEdge = pHalf->m_pEdge; 
			// Find Adjacent face
			CP_Face *pAdjFace = pHalf->m_pTwin->m_pLoop->m_pParentFace;
			if (pAdjFace->GetSurfaceType() == TYPE_SURFACE_UNDEFINED) {
				if (ClassifyAdjFaceAsCurved(pFace, pAdjFace, pEdge)
					|| ClassifyFaceAsPlane(pAdjFace))
					ClassifyAdjFaceRecursion(pAdjFace);
			}
		}
	}
}

// If current face is planar, its adjacent face is a curved face
bool CFaceClassification::ClassifyAdjFaceAsCurved(const CP_Face *pFace, CP_Face *pAdjFace, const CP_Edge *pEdge)
{
	if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT 
		&& pFace->GetSurfaceType() == TYPE_SURFACE_PLANE) {
			pAdjFace->m_surface = new CP_CoonsSurface;
			surface_basic::InitCoonsSurface(pAdjFace);
			SetCurveEdgeOfFaceUsed(pAdjFace);
			return true;
	}
	return false;
}

void CFaceClassification::ClassifyPseudoFace() {
	for (int i = 1; i < m_pAsmbBody->GetBodyNumber(); ++i) {
		CP_Body *pBody = m_pAsmbBody->GetBody(i);
		for (int j = 0; j < pBody->GetFaceNumber(); ++j) {
			CP_Face *pFace = pBody->GetFace(j);
			if (pFace->GetSurfaceType() == TYPE_SURFACE_UNDEFINED) {
				if (!ClassifyFaceAsPlane(pFace)) {
					for (int i = 0; i < pFace->GetLoopNumber(); ++i) {
						CP_Loop *pLoop = pFace->GetLoop(i);
						for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
							CP_Half *pHalf = pLoop->GetHalf(j);
							CP_Edge *pEdge = pHalf->m_pEdge; 
							CP_Face *pAdjFace = pHalf->m_pTwin->m_pLoop->m_pParentFace;
							ClassifyAdjFaceAsCurved(pAdjFace, pFace, pEdge);
						}
					}
				}
			}
		}
	}
}

void CFaceClassification::InitChildBodySurface() {
	for (int bi = 1; bi < m_pAsmbBody->GetBodyNumber(); ++bi) {
		CP_Body *pBody = m_pAsmbBody->GetBody(bi);
		for (int fi = 0; fi < pBody->GetFaceNumber(); ++fi) {
			CP_Face *pFace = pBody->GetFace(fi);
			CP_Surface3D **pSurface = &pFace->m_surface;
			if (facecls_utils::GetParentFaceType(pFace) == TYPE_SURFACE_PLANE) {
				CP_Face *pParentFace = (CP_Face*)(dynamic_cast<CP_FaceExt*>(pFace)->GetParent());
				CP_Plane *pPlane = dynamic_cast<CP_Plane*>(pParentFace->m_surface);
				CP_Point3D *pKnownPt = pFace->GetLoop(0)->GetFirstHalf()->m_pStartVt->m_pPoint;
				*pSurface = new CP_Plane(pKnownPt, pPlane->m_normal);
			}
			else if (facecls_utils::GetParentFaceType(pFace) == TYPE_SURFACE_COONS) {
				*pSurface = new CP_CoonsSurface;
				surface_basic::InitCoonsSurface(pFace);
			} 
			// Do not deal with face without parent. for example, pseudo face.
		}
	}
}


