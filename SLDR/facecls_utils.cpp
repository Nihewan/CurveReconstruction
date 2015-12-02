#include "stdafx.h"
#include "facecls_utils.h"
#include "base_geometricoper.h"
#include "topo_geometricoper.h"

namespace facecls_utils {


	double CalcCurvedProbability(const CP_Face *pFace) {
		double maxCosValue = 0;
		for (int i = 0; i < pFace->GetLoopNumber(); ++i) {
			CP_Loop *pLoop = pFace->GetLoop(i);
			for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
				CP_Edge *pEdgeJ = pLoop->GetHalf(j)->m_pEdge;
				if (pEdgeJ->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
					for (int k = j+1; k < pLoop->GetHalfNumber(); ++k) {
						CP_Edge *pEdgeK = pLoop->GetHalf(k)->m_pEdge;
						if (pEdgeK->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
							double ratio = pEdgeJ->m_pCurve3D->GetLength() / pEdgeK->m_pCurve3D->GetLength();
							ratio = ratio > 1? 1/ratio : ratio;
							double cosValue = topo_geometric::GetCosValue2D(pEdgeJ, pEdgeK) * ratio;
							if (cosValue > maxCosValue)
								maxCosValue = cosValue;
						}
					}
				}
			}
		}
		return maxCosValue;
	}

	int GetParentFaceType(CP_Face *pFace) {
		CP_FaceExt *pFaceExt = dynamic_cast<CP_FaceExt *>(pFace);
		if (pFaceExt == NULL)
			return pFace->GetSurfaceType();
		CP_Face* pParentFace = (CP_Face *)pFaceExt->GetParent();
		return pParentFace->GetSurfaceType();
	}
}
