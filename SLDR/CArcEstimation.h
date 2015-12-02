#pragma once
#include "CP_Topology.h"
#include "AbstractCurveEstimation.h"

class CArcEstimation : public AbstractCurveEstimation
{
public:
	CArcEstimation(void);
	~CArcEstimation(void);

public:
	virtual CP_Vector3D EstimateMatchedPaneNormal(CP_Edge *pEdge);
};

