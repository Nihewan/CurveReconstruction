#include "stdafx.h"
#include "est_surface.h"
#include "est_refplane.h"
#include "CVectorDataBlock.h"
#include "CDBScan.h"
#include "topo_geometricoper.h"
#include "base_geometricoper.h"
#include "propertiesnames.h"


namespace est_surface {
	CP_Vector3D EstimateLoopNormal(CP_Loop *pLoop);

	

	void EstimatePlane(CP_Face *pFace) {
		const double NORMAL_TOLER = 0.7;
		CP_LoopExt *pLoop = (CP_LoopExt *)pFace->GetLoop(0);
		CP_Vector3D matchedPaneNormal = *(CP_Vector3D *)pLoop->GetBaseGeoProperties(PROPERTY_NAME_MATCH_PANE_NORMAL);
		
		// If the loop has only 2 or less half, its normal is the normal of matched coordinate pane.
		((CP_Plane *)pFace->m_surface)->m_normal = matchedPaneNormal;
		
		// Calculate the normal of loop
		/*if (pLoop->GetHalfNumber() >= 3) {
			CP_Vector3D planeNormal = EstimateLoopNormal(pLoop);
			planeNormal.Normalize();
			if (abs(planeNormal * matchedPaneNormal) > NORMAL_TOLER) {
				((CP_Plane *)pFace->m_surface)->m_normal = planeNormal;

			}
		}*/
		//((CP_Plane *)pFace->m_surface)->m_pKnownPt = pFace->GetLoop(0)->GetFirstHalf()->m_pStartVt->m_pPoint;
	}

	CP_Vector3D EstimateLoopNormal(CP_Loop *pLoop) {
		const double EFFECTIVE_RADIUS = PI/12;
		CVectorDataBlock dataBlock(EFFECTIVE_RADIUS);
		typedef DataWithVLabel<CP_Vector3D> DWL;

		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_Vertex *pVt0 = pLoop->GetHalf(i)->m_pStartVt;
			CP_Vertex *pVt1 = pLoop->GetHalf(i)->m_pEndVt;
			CP_Vertex *pVt2 = pLoop->GetHalf((i+1)%pLoop->GetHalfNumber())->m_pEndVt;
			CP_Vector3D vec0(*pVt1->m_pPoint - *pVt0->m_pPoint);
			CP_Vector3D vec1(*pVt2->m_pPoint - *pVt1->m_pPoint);
			CP_Vector3D normalVec = vec0 ^ vec1;
			normalVec.Normalize();
			dataBlock.AddMember(&CP_Point3D(0, 0, 0), &CP_Point3D(normalVec.m_x, normalVec.m_y, normalVec.m_z));
		}
		dataBlock.InitDisMatrix();
		// Cluster
		CDBScan<CP_Vector3D> dbScan(&dataBlock, 0);
		std::vector<CCluster<CP_Vector3D> *> vResult;
		dbScan.DBScan(&vResult);

		// Find the cluster with max size
		if (vResult.size() == 0)
			return topo_geometric::CalcLoopNormal(pLoop);
		int maxIndex = 0;
		for (unsigned int i = 1; i < vResult.size(); ++i) {
			if (vResult.at(i)->GetSize() > vResult.at(maxIndex)->GetSize()) {
				maxIndex = i;
			}
		}

		CP_Vector3D result = *vResult.at(maxIndex)->CalcCenter(&base_geometric::GetMinIncludedAngle);

		// Release memory
		for (unsigned int i = 0; i < vResult.size(); ++i) {
			CCluster<CP_Vector3D> *p = vResult.at(i);
			delete p;
			p = NULL;
		}

		return result;
	}

}