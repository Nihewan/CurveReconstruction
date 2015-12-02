#include "stdafx.h"
#include "CBSplineEstimation.h"
#include "CP_TopologyExt.h"
#include "est_refplane.h"
#include "est_vtattroper.h"
#include "rec_utils.h"
#include "propertiesnames.h"

CBSplineEstimation::CBSplineEstimation(void)
{
}


CBSplineEstimation::~CBSplineEstimation(void)
{
}

CP_Vector3D CBSplineEstimation::EstimateMatchedPaneNormal(CP_Edge *pEdge) {
	CP_LoopExt *pLoop0 = (CP_LoopExt *)pEdge->GetHalf(0)->m_pLoop;
	CP_LoopExt *pLoop1 = (CP_LoopExt *)pEdge->GetHalf(1)->m_pLoop;
	CP_LoopExt *pLoop = NULL;

	int loop0Type = pLoop0->m_pParentFace->GetSurfaceType();
	int loop1Type = pLoop1->m_pParentFace->GetSurfaceType();

	if (loop0Type == TYPE_SURFACE_PLANE && loop1Type != TYPE_SURFACE_PLANE) {
		pLoop = pLoop0;
	} else if (loop0Type != TYPE_SURFACE_PLANE && loop1Type == TYPE_SURFACE_PLANE) {
		pLoop = pLoop1;
	} else {
		pLoop = pLoop0->GetIntegralProperties(PROPERTY_NAME_WEIGHT) > 
			pLoop1->GetIntegralProperties(PROPERTY_NAME_WEIGHT)? pLoop0 : pLoop1;
	}

	// If the face is plane, set b-spline's normal as plane's normal
	if (pLoop->m_pParentFace->GetSurfaceType() == TYPE_SURFACE_PLANE) 
		return ((CP_Plane *)pLoop->m_pParentFace->m_surface)->GetNormal();
	return *(CP_Vector3D *)pLoop->GetBaseGeoProperties(PROPERTY_NAME_MATCH_PANE_NORMAL);
}