#pragma once

class CP_Point3D;

namespace math_lib {
	int EquatSystemNumber(int degree, double **a, double *y, double *x);
	int EquatSystemPoint(int degree, double **a, CP_Point3D *y, CP_Point3D *x);
}