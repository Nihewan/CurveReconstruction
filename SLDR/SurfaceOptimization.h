#pragma once
#include "CP_FlowComplex.h"
class SurfaceOptimization
{
private:
	CP_FlowComplex *m_FlowComplex;
public:
	SurfaceOptimization(CP_FlowComplex *fc):m_FlowComplex(fc){};
	~SurfaceOptimization(void);
public:
	void SetTrianglePatch();
	void ElimateDegenerateTriangles();
	void MergeDegenerateTrianglesIn2cell();
	void ElimateDegenerateTrianglesInPatch();
};

