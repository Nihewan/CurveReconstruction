#include "stdafx.h"
#include "rec_bodyassembly.h"
#include "rec_utils.h"
#include "CLog.h"
#include "topo_geometricoper.h"
#include "propertiesnames.h"
#include "CBoundBox2D.h"

namespace rec_bodyassembly {
	void AssembleBody(CP_AssembledBody *pAsmbBody) {
		if (pAsmbBody->GetBodyNumber() == 1)
			return;

		for (int i = 1; i < pAsmbBody->GetBodyNumber(); ++i) {
			CP_BodyExt *pBody = (CP_BodyExt *)pAsmbBody->GetBody(i);
			pBody->SetProperties(PROPERTY_NAME_VISITED, 0);
		}

		// Fix the first body and adjust the other bodies' depth 
		CP_BodyExt *pFirstBody = (CP_BodyExt *)(pAsmbBody->GetBody(1));
		pFirstBody->SetProperties(PROPERTY_NAME_VISITED, 1);
		
		BFSAdjustAdjBodyDepth(pAsmbBody, pFirstBody);

		RecoverOriginalBody(pAsmbBody);
		rec_utils::ReconstructBodyPlaneFaceNormal(pAsmbBody->GetOriginalBody());
	}

	void BFSAdjustAdjBodyDepth(CP_AssembledBody *pAsmbBody, CP_BodyExt *pFirstBody) {
		std::queue<CP_BodyExt *> queueBody;
		FindAdjBody(pAsmbBody, pFirstBody, queueBody);
		
		while (!queueBody.empty())
		{
			CP_BodyExt *pBody = queueBody.front();
			queueBody.pop();

			// Visited the unvisited body and adjust its adjacent bodies' depth.
			if (!pBody->GetIntegralProperties(PROPERTY_NAME_VISITED)) {
				pBody->SetProperties(PROPERTY_NAME_VISITED, 1);
				FindAdjBody(pAsmbBody, pBody, queueBody);
			}
		}

	}

	void FindAdjBody(CP_AssembledBody *pAsmbBody, CP_Body *pRefBody, std::queue<CP_BodyExt *> &qBody) {
		for (int i = 1; i < pAsmbBody->GetBodyNumber(); ++i) {
			CP_BodyExt *pBody = (CP_BodyExt *)pAsmbBody->GetBody(i);
			if (!pBody->GetIntegralProperties(PROPERTY_NAME_VISITED)) {
				// If body is adjacent to the reference body, 
				// adjust its depth and push it into queue.
				if (AdjustBodyDepth(pRefBody, pBody))
					qBody.push(pBody);
			}
		}
	}

	bool AdjustBodyDepth(CP_Body *pRefBody, CP_Body *pInputBody) {
		pair<CP_Face *, CP_Face *> conjointFacePair = rec_bodyassembly::FindConjointFace(pRefBody, pInputBody);

		// Find conjoint vertex
		if (conjointFacePair.first) {
			double depthDif = 1.0;

			// Make the bigger curved face first
			if (conjointFacePair.first->GetSurfaceType() != TYPE_SURFACE_PLANE) {
				CBoundBox2D box0;
				box0.InitBoundBox(conjointFacePair.first->GetLoop(0));
				CBoundBox2D box1;
				box1.InitBoundBox(conjointFacePair.second->GetLoop(0));
				if (!box0.Contain(box1)) { 
					swap(conjointFacePair.first, conjointFacePair.second);
					// CP_Face *pTmpFace = conjointFacePair.first;
					// conjointFacePair.first = conjointFacePair.second;
					// conjointFacePair.second = pTmpFace;
					depthDif = -1.0;
				}
			}

			vector<CP_Vertex *> vConjointVt;
			FindConjointVt(conjointFacePair.second, vConjointVt);
			depthDif *= CalcConjointVtDepthDif(conjointFacePair.first, vConjointVt);
			TranslateBody(pInputBody, depthDif);
			return true;
		} 
		return false;
	}

	// Calculate the points' depth on surface and calculate the difference. Use least-squares method to minimize error.
	double CalcConjointVtDepthDif(CP_Face *pRefFace, vector<CP_Vertex *> &vConjointVt) {
		double sum = 0;
		for (unsigned int i = 0; i < vConjointVt.size(); ++i) {
			CP_Point3D pt = *vConjointVt[i]->m_pPoint;
			sum += pt.m_z - pRefFace->m_surface->GetPointDepth(pt);
		}
		return -1.0/2 - sum / vConjointVt.size();
	}

	void TranslateBody(CP_Body *pBody, double depth) {
		// Move vertex
		for (int i = 0; i < pBody->GetVertexNumber(); ++i) {
			CP_Vertex *vt = pBody->GetVertex(i);
			vt->m_pPoint->m_z += depth;
		}
		for (int i = 0; i < pBody->GetEdgeNumber(); ++i) {
			CP_Edge *pEdge = pBody->GetEdge(i);
			// Move line segment end points
			if (pEdge->GetCurveType() == TYPE_CURVE_LINESEGMENT) {
				CP_LineSegment3D *pLineSeg = (CP_LineSegment3D *)pEdge->m_pCurve3D;
				pLineSeg->m_startPt.m_z += depth;
				pLineSeg->m_endPt.m_z += depth;
			}
			// Move BSpline control points for the next time calculation
			if (pEdge->GetCurveType() == TYPE_CURVE_NURBS) {
				CP_Nurbs *pBSpline = (CP_Nurbs *)pEdge->m_pCurve3D;
				for (int i = 0; i < pBSpline->GetCtrlNumber(); ++i) {
					CP_Point3D *pPt = pBSpline->GetCtrlPoint(i);
					pPt->m_z += depth;
				}
			}
			// Move arc center point
			if (pEdge->GetCurveType() == TYPE_CURVE_ARC) {
				CP_Arc *pArc = (CP_Arc *)pEdge->m_pCurve3D;
				pArc->m_ucs.m_origin.m_z += depth;
			}
		}
		
	}

	void FindConjointVt(CP_Face *conjointFace, vector<CP_Vertex *> &vConjointVt) {
		CP_Loop *pLoop = conjointFace->GetLoop(0);
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			vConjointVt.push_back(pLoop->GetHalf(i)->m_pStartVt);
		}
	}

	pair<CP_Face *, CP_Face *> FindConjointFace(CP_Body *pBody0, CP_Body *pBody1) {
		for (int i = 0; i < pBody0->GetFaceNumber(); ++i) {
			for (int j = 0; j < pBody1->GetFaceNumber(); ++j) {
				CP_FaceExt *pFace0 = (CP_FaceExt *)pBody0->GetFace(i);
				CP_FaceExt *pFace1 = (CP_FaceExt *)pBody1->GetFace(j);
				if (pFace0->GetParent() == pFace1->GetParent()) {
					// One of the loops should contain the other one
					CBoundBox2D box0;
					box0.InitBoundBox(pFace0->GetLoop(0));
					CBoundBox2D box1;
					box1.InitBoundBox(pFace1->GetLoop(0));
					if (!box0.Contain(box1) && !box1.Contain(box0) ) 
						return pair<CP_Face *, CP_Face*>(NULL, NULL);

					return pair<CP_Face *, CP_Face*>(pFace0, pFace1);
				}
				// Pseudo faces have no common parent
				if (IsSamePseudoFace(pFace0, pFace1)) {
					return pair<CP_Face *, CP_Face *>(pFace0, pFace1);
				}
			}
		}
		return pair<CP_Face *, CP_Face*>(NULL, NULL);
	}

	bool IsSamePseudoFace(CP_FaceExt *pFace0, CP_FaceExt *pFace1) {
		// Pseudo faces have no parent. So their parent point to themselves.
		if (pFace0->GetParent() == pFace0 && pFace1->GetParent() == pFace1) {
			CP_Loop *pLoop0 = pFace0->GetLoop(0);
			CP_Loop *pLoop1 = pFace1->GetLoop(0);
			int numSameVt = 0;
			for (int i = 0; i < pLoop0->GetHalfNumber(); ++i) {
				CP_VertexExt* pVt0 = (CP_VertexExt* )pLoop0->GetHalf(i)->m_pStartVt;
				for (int j = 0; j < pLoop1->GetHalfNumber(); ++j) {
					CP_VertexExt* pVt1 = (CP_VertexExt*)pLoop1->GetHalf(j)->m_pStartVt;
					if (pVt0->GetParent() == pVt1->GetParent()) {
						numSameVt++;
						break;
					} // Find vertice that have common parent.
				}
			}
			return numSameVt == pLoop0->GetHalfNumber(); 
			// Two loops combine together while each vertex in one loop found a vertex with common parent in the other loop.
		}
		return false;
	}

	void RecoverOriginalBody(CP_AssembledBody *pAsmbBody) {
		for (int i = 1; i < pAsmbBody->GetBodyNumber(); ++i) {
			CP_Body *pBody = pAsmbBody->GetBody(i);
			for (int j = 0; j < pBody->GetVertexNumber(); ++j) {
				CP_Vertex *pOriVt = pBody->GetVertex(j);
				CP_VertexExt *pVt = dynamic_cast<CP_VertexExt *>(pBody->GetVertex(j));
				ASSERT(pVt);
				CP_Vertex *pVtParent = static_cast<CP_Vertex *>(pVt->GetParent());
				pVtParent->m_pPoint->m_z = pVt->m_pPoint->m_z;
			}
			for (int j = 0; j < pBody->GetEdgeNumber(); ++j) {
				CP_EdgeExt *pEdge = dynamic_cast<CP_EdgeExt *>(pBody->GetEdge(j));
				CP_Edge *pEdgeParent = static_cast<CP_Edge *>(pEdge->GetParent());
				*pEdgeParent->m_pCurve3D = *pEdge->m_pCurve3D;
			}
		}
	}
}