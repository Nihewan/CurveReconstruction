#pragma once
#include "CP_Topology.h"

class AbstractCurveEstimation
{
public:
	AbstractCurveEstimation();
	~AbstractCurveEstimation();

public:
	CP_Vector3D	EstimateNormal(CP_Edge *pEdge);
	virtual CP_Vector3D EstimateMatchedPaneNormal(CP_Edge *pEdge) = 0;

};
