#include "stdafx.h"
#include "cubiccorner.h"
#include <math.h>

namespace cubic_corner {


	void NormalizeCubicCornerVecs(CP_Vector3D* vec[3]) {

		for (int i = 0; i < 3; ++i) {
			vec[i]->m_z = 0;
			vec[i]->Normalize();
		}
	}


	/*
	* parameter 
	*	vecs[3]: the 3 vectors 
	*   fixedAxisIndex is the axis index whose direction is fixed, 
	*   fixedAxisDirection is 1 for z>0 or -1 for z<0. 
	*/
	void CalcCubicCorner(CP_Vector3D* vecs[3], int fixedAxisIndex, int fixedAxisDirection) {
		ASSERT(abs(vecs[0]->GetLength() - 1) < TOLER && 
			abs(vecs[1]->GetLength() - 1) < TOLER  && 
			abs(vecs[2]->GetLength() - 1) < TOLER );
		ASSERT(abs(vecs[0]->m_z) < TOLER && 
			abs(vecs[1]->m_z) < TOLER  && 
			abs(vecs[2]->m_z) < TOLER );

		double cos[3];
		for(int i = 0; i < 3; i++) {
			cos[i] = (*vecs[i]) * (*vecs[(i+1)%3]);
		}
		for(int i = 0; i < 3; i++) {
			vecs[i]->m_z = (double)sqrt(fabs(cos[(i+2)%3]*cos[i]/cos[(i+1)%3]));
		}

		vecs[fixedAxisIndex]->m_z *= fixedAxisDirection;
		for (int i = 0; i < 3; ++i) 
			vecs[i]->Normalize();

		// Adjust z sign so that three vectors are orthogonality
		if (abs(*vecs[fixedAxisIndex] * *vecs[(fixedAxisIndex+1)%3]) > TOLER) {
			vecs[(fixedAxisIndex+1)%3]->m_z *= -1;
			if (abs(*vecs[(fixedAxisIndex+1)%3] * *vecs[(fixedAxisIndex+2)%3]) > TOLER)
				vecs[(fixedAxisIndex+2)%3]->m_z *= -1;
		}
		if (abs(*vecs[(fixedAxisIndex+1)%3] * *vecs[(fixedAxisIndex+2)%3]) > TOLER)
			vecs[(fixedAxisIndex+2)%3]->m_z *= -1;
	}
}