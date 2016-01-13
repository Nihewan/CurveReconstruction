#include "stdafx.h"
#include "Reconstruction.h"
#include "CHillClimb.h"
#include "topo_geometricoper.h"
#include "rec_utils.h"
#include "base_geometricoper.h"
#include "rec_curve.h"
#include "CObjectiveFuncCalculator.h"
#include "propertiesnames.h"
#include "rec_bodyassembly.h"
#include "rec_surface.h"
#include "curve_basic.h"
#include "CObjectiveFuncCalculator.h"
#include "surface_basic.h"
#include "CTimer.h"
#include "CLog.h"

CReconstruction::~CReconstruction(void)
{
}

void CReconstruction::Reconstruction(const vector<int> &vWeight) {
	InsertMidVt();
	int beginIndex = m_pAsmbBody->GetBodyNumber() == 1? 0 : 1;
	CTimer timer;
	timer.Start();
	for (int i = beginIndex; i < m_pAsmbBody->GetBodyNumber(); ++i) {
		CP_Body *pBody = m_pAsmbBody->GetBody(i);
		//CLog::log("Vt op %d", i);
		VtDepOptimization(pBody, vWeight);
		//CLog::log("Face op %d", i);
		FaceOptimization(pBody);
		//CLog::log("Trans op %d", i);
		TransEdgeOptimization(pBody);
		// RotateSilhouette(pBody);
	}
	CLog::log("Hill climb time %f ms", timer.End());
	
	RemoveMidVt();
	rec_bodyassembly::AssembleBody(m_pAsmbBody);
	rec_bodyassembly::AdjustBodyLoopDirect(m_pAsmbBody);
	
	timer.Start();
	ReconstructSurface();
	CLog::log("Face reconstruction time %f ms", timer.End());
}

void CReconstruction::VtDepOptimization(CP_Body *pBody, const vector<int> &vWeight) {
	vector<double> depthArr;
	for (int i = 0; i < pBody->GetVertexNumber(); ++i) 
		depthArr.push_back(pBody->GetVertex(i)->m_pPoint->m_z);

	CVtDepObjFuncCalculator objFunc(pBody, vWeight);

	// Reconstruct vertex depth
	CHillClimb hill(&objFunc, depthArr);
	hill.HillClimb();
}

void CReconstruction::FaceOptimization(CP_Body *pBody) {
	// Adjacent face of generalized face
	set<CP_Face *> setAdjFaces;
	GetAdjFaceOfGenFace(pBody, setAdjFaces);
	if (setAdjFaces.size() == 0)
		return;

	vector<double> vCenterDeps;
	vector<pair<CP_Face*, double> > vecFaces;

	for(set<CP_Face *>::iterator iter = setAdjFaces.begin(); iter != setAdjFaces.end(); ++iter) {
		double centerZ = CalcCenterZ(*iter);
		vCenterDeps.push_back(centerZ);
		vecFaces.push_back(pair<CP_Face*, double>(*iter, centerZ));
	} 

	vector<int> vWeight;
	vWeight.push_back(1);
	CGenFaceDepObjFuncCalculator fObjFunc(pBody, vWeight, vecFaces);
	CHillClimb hill(&fObjFunc, vCenterDeps);
	hill.HillClimb();

}

void CReconstruction::TransEdgeOptimization(CP_Body *pBody)
{
	vector<CTransitionEdgeInfo> vTrans;
	CTransitionCurveFuncCalculator::GetTransitionCurves(pBody, vTrans);
	vector<double> depthArr;
	for (unsigned int i = 0; i < vTrans.size(); ++i) {
		CP_EdgeExt *pEdge = dynamic_cast<CP_EdgeExt*>(vTrans[i].m_pTransitionEdge);
		depthArr.push_back(((CP_Vertex *)pEdge->GetCObjPtProperties(PROPERTY_NAME_MID_VERTEX))->m_pPoint->m_z);
	}

	vector<int> vWeight;
	CTransitionCurveFuncCalculator objFunc(pBody, vWeight, vTrans);

	// Reconstruct vertex depth
	CHillClimb hill(&objFunc, depthArr);
	hill.HillClimb();
}

// Find generalized face's adjacent plane face by curve and calculate the geometric center's depth
void CReconstruction::GetAdjFaceOfGenFace(CP_Body *pBody, set<CP_Face *> &setFaces) {
	for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
		CP_Face *pFace = pBody->GetFace(i);

		// Curve face is a generalized face
		if (pFace->GetSurfaceType() != TYPE_SURFACE_PLANE) {
			CP_Loop *pLoop = pFace->GetLoop(0);
			for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
				CP_Half *pHalf = pLoop->GetHalf(i);

				// Adjacent by curve
				if (pHalf->m_pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
					CP_Face *pAdjFace = pHalf->m_pTwin->m_pLoop->m_pParentFace;

					// Plane
					if (pAdjFace->GetSurfaceType() == TYPE_SURFACE_PLANE) {
						setFaces.insert(pAdjFace);
					}
				}
			}
		}
	}
}

void CReconstruction::RotateSilhouette(CP_Body *pBody) {
	for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
		CP_Face *pFace = pBody->GetFace(i);

		if (pFace->GetSurfaceType() != TYPE_SURFACE_PLANE) {
			CP_Loop *pLoop = pFace->GetLoop(0);
			CP_Vector3D *pN = NULL;
			for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
				CP_EdgeExt *pEdge = dynamic_cast<CP_EdgeExt*>(pLoop->GetHalf(j)->m_pEdge);
				CP_Half *pTwin = pLoop->GetHalf(j)->m_pTwin;

				// Find Silhouette
				if (topo_geometric::IsTransitionEdge(pEdge))
				{
					pEdge->SetProperties(PROPERTY_NAME_VISITED, 1);

					// Fit plane
					if (!pN) {
						vector<CP_Point3D *> vPts;
						for (int k = 0; k < pLoop->GetHalfNumber(); ++k) {
							vPts.push_back(pLoop->GetHalf(k)->m_pStartVt->m_pPoint);
						}
						CFaceBestFit face;
						shared_ptr<CFaceSumData> sumData(CFaceBestFit::createFaceXYSum(i, vPts));
						CFaceBestFit::BestFit(vPts, sumData, face);
						pN = new CP_Vector3D(face.m_A, face.m_B, face.m_C);
						pN->Normalize();
					} 
					if (pEdge->GetCurveType() == TYPE_CURVE_ARC) {
						dynamic_cast<CP_Arc*>(pEdge->m_pCurve3D)->m_ucs.m_axisZ = *pN;
						rec_curve::ReconstructArcUCS(pEdge);
					} else if (pEdge->GetCurveType() == TYPE_CURVE_NURBS) {
						dynamic_cast<CP_Nurbs*>(pEdge->m_pCurve3D)->m_normal = *pN;
						rec_curve::ReconstructBsplineCtrlPts(pEdge);
					}
					
				}
			}
			delete pN;
		}
	}
}

double CReconstruction::CalcCenterZ(CP_Face *pFace) {
	CP_Loop *pLoop = pFace->GetLoop(0);
	double sum = 0;
	for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
		CP_Half *pHalf = pLoop->GetHalf(i);
		CP_Vertex *pVt = pHalf->m_pStartVt;
		CP_EdgeExt *pEdge = dynamic_cast<CP_EdgeExt *>(pHalf->m_pEdge);
		if (pEdge->HasProperty(PROPERTY_NAME_MID_VERTEX))
			sum += ((CP_Vertex *)pEdge->GetCObjPtProperties(PROPERTY_NAME_MID_VERTEX))->m_pPoint->m_z;
		sum += pVt->m_pPoint->m_z;
	}
	return sum / pLoop->GetHalfNumber();
}

void CReconstruction::InsertMidVt() {
	for (int i = 0; i < m_pAsmbBody->GetBodyNumber(); i++)
	{
		CP_Body *pBody = m_pAsmbBody->GetBody(i);

		// Insert a middle vertex if the plane face has only two vertex.
		// When a plane face has only two vertex, we cannot get the normal of it.
		for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
			CP_Face *pFace = pBody->GetFace(i);
			if (pFace->GetSurfaceType() == TYPE_SURFACE_PLANE &&
				pFace->GetLoop(0)->GetHalfNumber() == 2) 
			{
				for (int j = 0; j < 2; ++j) {
					CP_Half *pHalf = pFace->GetLoop(0)->GetHalf(j);
					if (pHalf->m_pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
						CP_EdgeExt *pEdge = static_cast<CP_EdgeExt *>(pHalf->m_pEdge);
						CP_Point3D *pPt = new CP_Point3D( pEdge->m_pCurve3D->GetPoint(0.5));
						CP_Vertex *pVt = new CP_Vertex(pPt);
						pBody->AddVertex(pVt);
						pEdge->SetProperties(PROPERTY_NAME_MID_VERTEX, pVt);
						break;
					}
				}
			}
		}

		// Insert a middle vertex if the curve edge's two adjacent faces are curved face.
		// When a curve edge has 3 vertices, we can calculate the normal of it.
		for (int i = 0; i < pBody->GetEdgeNumber(); ++i) {
			CP_EdgeExt *pEdge = static_cast<CP_EdgeExt *>(pBody->GetEdge(i));
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT &&
				pEdge->m_pHalfs[0]->m_pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE &&
				pEdge->m_pHalfs[1]->m_pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE) 
			{
				CP_Point3D *pPt = new CP_Point3D( pEdge->m_pCurve3D->GetPoint(0.5));
				CP_Vertex *pVt = new CP_Vertex(pPt);
				pBody->AddVertex(pVt);
				pEdge->SetProperties(PROPERTY_NAME_MID_VERTEX, pVt);
			}
		}
	}
}

void CReconstruction::RemoveMidVt() {
	for (int i = 0; i < m_pAsmbBody->GetBodyNumber(); i++)
	{
		CP_Body *pBody = m_pAsmbBody->GetBody(i);

		// Remove a middle vertex if the plane face has only two vertex.
		// When a plane face has only two vertex, we cannot get the normal of it.
		for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
			CP_Face *pFace = pBody->GetFace(i);
			if (pFace->GetSurfaceType() == TYPE_SURFACE_PLANE &&
				pFace->GetLoop(0)->GetHalfNumber() == 2) 
			{
				for (int j = 0; j < 2; ++j) {
					CP_Half *pHalf = pFace->GetLoop(0)->GetHalf(j);
					if (pHalf->m_pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
						CP_EdgeExt *pEdge = static_cast<CP_EdgeExt *>(pHalf->m_pEdge);
						CP_Vertex *pVt = static_cast<CP_Vertex*>(pEdge->GetCObjPtProperties(PROPERTY_NAME_MID_VERTEX));
						pBody->RemoveVertex(pVt);
						pEdge->RemoveProperty(PROPERTY_NAME_MID_VERTEX);
						break;
					}
				}
			}
		}

		// Remove a middle vertex if the curve edge's two adjacent faces are curved face.
		// When a curve edge has 3 vertices, we can calculate the normal of it.
		for (int i = 0; i < pBody->GetEdgeNumber(); ++i) {
			CP_EdgeExt *pEdge = static_cast<CP_EdgeExt *>(pBody->GetEdge(i));
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT &&
				pEdge->m_pHalfs[0]->m_pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE &&
				pEdge->m_pHalfs[1]->m_pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE) 
			{
				ASSERT(pEdge->HasProperty(PROPERTY_NAME_MID_VERTEX));
				CP_Vertex *pVt = static_cast<CP_Vertex*>(pEdge->GetCObjPtProperties(PROPERTY_NAME_MID_VERTEX));
				pBody->RemoveVertex(pVt);
				pEdge->RemoveProperty(PROPERTY_NAME_MID_VERTEX);
			}
		}
	}
}

void CReconstruction::ReconstructSurface() {
	//for (int i = 0; i < m_pAsmbBody->GetBodyNumber(); ++i) {
		CP_Body *pBody = m_pAsmbBody->GetBody(0);
		for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
			CP_Face *pFace = pBody->GetFace(i);
			if (pFace->GetSurfaceType() != TYPE_SURFACE_PLANE) {
				if (pFace->GetLoop(0)->GetHalfNumber() == 4) {
					//CLog::log("create nurbs surf for %d", i);
					rec_surface::ReconstructNurbsSurf(pFace);
				} else if (pFace->GetLoop(0)->GetHalfNumber() == 3) {
					//CLog::log("create coons surf for %d", i);
					rec_surface::ReconstructCoonsSurf(pFace);
				}
				else
					;//CLog::log("surf type cannot be reconstructed %d", i);
			}
			else
				;//CLog::log("create plane surf for %d", i);
			//CLog::log("creating mesh for %d", i);
			rec_surface::CreateMesh(pFace);
			//CLog::log("create mesh completed %d", i);
		}
	//}
}