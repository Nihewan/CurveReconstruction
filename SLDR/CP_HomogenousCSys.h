#pragma once
#include "CP_PointVector.h"
#include "CMatrix4.h"

class CP_HomogenousCSys
{
public:
	CMatrix4 m_matrix;

public:
	CP_HomogenousCSys ();
	~CP_HomogenousCSys();
	CP_HomogenousCSys (const CP_HomogenousCSys& m);

	inline void SetShift (double dx, double dy, double dz);
	void SetScale (double sx, double sy, double sz);
	inline void SetRotate (int axis, double angle);
	inline void SetRotateV (const CP_Vector3D& v, double ang);
	inline void SetRotateZ (const CP_Vector3D& v);
	void SetLocal (const CP_Point3D& p0, const CP_Vector3D& zv, const CP_Vector3D& xv);
	inline void SetMirror (double a, double b, double c, double d);
};
