#include "stdafx.h"
#include "est_vtattroper.h"
#include "propertiesnames.h"

/************************************************************************/
/* Vertex visited and weight attribute                                  */
/************************************************************************/

namespace est_vtattroper {
	CP_VertexExt* FindVertexVisited(CP_LoopExt *pLoop, int &outHalfIndex) {
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_VertexExt *pVt = (CP_VertexExt *)pLoop->GetHalf(i)->m_pStartVt;
			if (IsVisited(pVt)) {
				outHalfIndex = i;
				return pVt;
			}
		}
		return NULL;
	}

	int IsVisited( CP_VertexExt *pVt )
	{
		ASSERT(pVt->HasProperty(PROPERTY_NAME_VISITED));
		return pVt->GetIntegralProperties(PROPERTY_NAME_VISITED);
	}

	void SetVertexDepthAndWeight(CP_VertexExt *pVt, double depth, int weight) {
/*
		CP_VertexExt *pVtParent = (CP_VertexExt *)pVt->GetParent();
		pVtParent->SetProperties(PROPERTY_NAME_VISITED, 1);
		pVt->m_pPoint->m_z = depth;
		pVtParent->m_pPoint->m_z = depth;
		pVtParent->SetProperties(PROPERTY_NAME_WEIGHT, weight);*/
		ASSERT(pVt->HasProperty(PROPERTY_NAME_VISITED));
		ASSERT(pVt->HasProperty(PROPERTY_NAME_WEIGHT));
		pVt->m_pPoint->m_z = depth;
		pVt->SetProperties(PROPERTY_NAME_VISITED, 1);
		pVt->SetProperties(PROPERTY_NAME_WEIGHT, weight);
	}

	int GetVertexWeight(CP_VertexExt *pVt) {/*
		CP_VertexExt *pVtParent = (CP_VertexExt *)pVt->GetParent();
		return pVtParent->GetIntegralProperties(PROPERTY_NAME_WEIGHT);*/
		ASSERT(pVt->HasProperty(PROPERTY_NAME_WEIGHT));
		return pVt->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
	}


	void InitNotInQueue(CP_Loop *pLoop) {
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_VertexExt *pVt = (CP_VertexExt *)pLoop->GetHalf(i)->m_pStartVt;
			pVt->SetProperties(PROPERTY_NAME_IS_IN_QUEUE, 0);
		}
	}
}