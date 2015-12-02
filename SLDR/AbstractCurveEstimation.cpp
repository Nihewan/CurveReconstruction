#include "stdafx.h"
#include "AbstractCurveEstimation.h"


AbstractCurveEstimation::AbstractCurveEstimation()
{
}

AbstractCurveEstimation::~AbstractCurveEstimation()
{
}

CP_Vector3D AbstractCurveEstimation::EstimateNormal(CP_Edge *pEdge) {
	CP_Vector3D normal = EstimateMatchedPaneNormal(pEdge);
	CP_Vector3D start2End = *pEdge->m_pEndVt->m_pPoint - *pEdge->m_pStartVt->m_pPoint;
	start2End.Normalize();
	CP_Vector3D tmpVec = start2End * (start2End * normal);
	return normal - tmpVec;
}