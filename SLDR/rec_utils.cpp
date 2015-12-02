#include "stdafx.h"
#include "rec_utils.h"
#include "CP_TopologyExt.h"
#include "topo_geometricoper.h"
#include "base_geometricoper.h"
#include "propertiesnames.h"
#include "CLog.h"
#include <queue>

namespace rec_utils {

	void AdjustBodyLoopDirect(CP_LoopExt *pInitLoop, bool isClockwise);

	void RecSinglePlaneFaceNormal(CP_Face *pFace) {
		if (pFace->GetSurfaceType() == TYPE_SURFACE_PLANE) {
			// Face which has only two edges
			if (pFace->GetLoop(0)->GetHalfNumber() == 2) {
				for (int j = 0; j < 2; ++j) {
					CP_Half *pHalf = pFace->GetLoop(0)->GetHalf(j);
					CP_EdgeExt* pEdge = static_cast<CP_EdgeExt*>(pHalf->m_pEdge);
					if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT &&
						pEdge->HasProperty(PROPERTY_NAME_MID_VERTEX)) 
					{
						CP_Point3D midPt = *static_cast<CP_Vertex *>(pEdge->GetCObjPtProperties(PROPERTY_NAME_MID_VERTEX))->m_pPoint;
						CP_Vector3D v0 = *pEdge->m_pStartVt->m_pPoint - midPt;
						CP_Vector3D v1 = *pEdge->m_pEndVt->m_pPoint - midPt;
						((CP_Plane *)(pFace->m_surface))->m_normal = v0 ^ v1;
					}
				}
			} 
			// Face which has more than 2 edges
			else {
				CP_Loop *pLoop = pFace->GetLoop(0);
				((CP_Plane *)pFace->m_surface)->m_normal = topo_geometric::CalcLoopNormal(pLoop);
			}
		}
	}


	void ReconstructBodyPlaneFaceNormal(CP_Body *pBody) {
		for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
			CP_FaceExt *pFace = (CP_FaceExt *)pBody->GetFace(i);
			RecSinglePlaneFaceNormal(pFace);
		}
	}


	void AssignVtDepth(CP_Body *pBody, vector<double> &depthArr) {
		ASSERT(pBody->GetVertexNumber() == depthArr.size());
		for (int i = 0; i < pBody->GetVertexNumber(); ++i) {
			CP_Vertex *pVt = pBody->GetVertex(i);
			pVt->m_pPoint->m_z = depthArr[i];
		}
	}


}
