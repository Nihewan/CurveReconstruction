#pragma once
#include "CP_Topology.h"

namespace cubic_corner {
	void NormalizeCubicCornerVecs(CP_Vector3D* vec[3]);

	void CalcCubicCorner(CP_Vector3D* vecs[3], 
		int fixedAxisIndex, int fixedAxisDirection); // Calculate the Z-axis value of 3 vector in a cubic corner. Note that the 3 vectors must be normalize. 

}