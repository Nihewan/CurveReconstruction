#pragma once
#include "AbstractCurveEstimation.h"

class CBSplineEstimation : public AbstractCurveEstimation
{
public:
	CBSplineEstimation(void);
	~CBSplineEstimation(void);

public:
	virtual CP_Vector3D EstimateMatchedPaneNormal(CP_Edge *pEdge);
};

