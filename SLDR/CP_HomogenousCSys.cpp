#include "StdAfx.h"
#include "CP_HomogenousCSys.h"
#include <math.h>
#include "base_geometricoper.h"

CP_HomogenousCSys::~CP_HomogenousCSys(void)
{
}

CP_HomogenousCSys::CP_HomogenousCSys ()
{
	memset (m_matrix.mat, 0, sizeof (m_matrix.mat));
}


CP_HomogenousCSys::CP_HomogenousCSys (const CP_HomogenousCSys& m)
{
	memcpy (m_matrix.mat, m.m_matrix.mat, sizeof (m_matrix.mat));
}

void CP_HomogenousCSys::SetShift (double dx, double dy, double dz)
{
	m_matrix.SetUnit ();
	m_matrix.mat[3][0] = dx;
	m_matrix.mat[3][1] = dy;
	m_matrix.mat[3][2] = dz;
}

void CP_HomogenousCSys::SetScale (double sx, double sy, double sz)
{
	m_matrix.SetZero ();
	m_matrix.mat[0][0] = sx;
	m_matrix.mat[1][1] = sy;
	m_matrix.mat[2][2] = sz;
	m_matrix.mat[3][3] = 1.;
}

void CP_HomogenousCSys::SetRotate (int axis, double angle)
{
	m_matrix.SetUnit ();
	switch (axis)
	{
	case 0: m_matrix.mat[1][1] = cos (angle);
		m_matrix.mat[1][2] = sin (angle);
		m_matrix.mat[2][1] =-m_matrix.mat[1][2];
		m_matrix.mat[2][2] = m_matrix.mat[1][1];
		break;
	case 1: m_matrix.mat[0][0] = cos (angle);
		m_matrix.mat[0][2] =-sin (angle);
		m_matrix.mat[2][0] =-m_matrix.mat[0][2];
		m_matrix.mat[2][2] = m_matrix.mat[0][0];
		break;
	case 2: m_matrix.mat[0][0] = cos (angle);
		m_matrix.mat[0][1] = sin (angle);
		m_matrix.mat[1][0] =-m_matrix.mat[0][1];
		m_matrix.mat[1][1] = m_matrix.mat[0][0];
		break;
	}
}

void CP_HomogenousCSys::SetRotateV (const CP_Vector3D& v, double ang)
{
	double  a, b, c, aa, bb, cc, ab, bc, ac, siu, cou;

	a = v.m_x;
	b = v.m_y;
	c = v.m_z;
	aa = a * a;
	bb = b * b;
	cc = c * c;
	ab = a * b;
	bc = b * c;
	ac = a * c;
	cou = cos (ang);
	siu = sin (ang);

	m_matrix.mat[0][0] = aa + (1-aa ) * cou;
	m_matrix.mat[1][0] = ab * (1-cou) - c*siu;
	m_matrix.mat[2][0] = ac * (1-cou) + b*siu;
	m_matrix.mat[3][0] = 0.;
	m_matrix.mat[0][1] = ab * (1-cou) + c*siu;
	m_matrix.mat[1][1] = bb + (1-bb ) * cou;
	m_matrix.mat[2][1] = bc * (1-cou) - a*siu;
	m_matrix.mat[3][1] = 0.;
	m_matrix.mat[0][2] = ac * (1-cou) - b*siu;
	m_matrix.mat[1][2] = bc * (1-cou) + a*siu;
	m_matrix.mat[2][2] = cc + (1-cc ) * cou;
	m_matrix.mat[3][2] = 0.;
	m_matrix.mat[0][3] = 0.;
	m_matrix.mat[1][3] = 0.;
	m_matrix.mat[2][3] = 0.;
	m_matrix.mat[3][3] = 1.;
}

void CP_HomogenousCSys::SetRotateZ (const CP_Vector3D& v)
{
	CP_Vector3D va;
	double ang, radius;

	// (a, b, c) is rotate axis
	radius = v.m_x*v.m_x + v.m_y*v.m_y;
	if (radius < 1.0e-4)
	{
		if (v.m_z < 0.)
		{
			va = CP_Vector3D (0, 1, 0);
			ang = PI;
		}
		else
		{
			m_matrix.SetUnit ();
		}
	}
	else
	{
		radius = sqrt (radius);
		va = CP_Vector3D (v.m_y / radius, -v.m_x / radius, 0);
		ang = base_geometric::GetASIN(radius);
		if (v.m_z < 0.) ang = PI - ang;
	}

	SetRotateV (va, ang);
}

void CP_HomogenousCSys::SetLocal (const CP_Point3D& p0, const CP_Vector3D& zv, const CP_Vector3D& xv)
{
	CP_Vector3D yv = zv ^ xv;
	m_matrix.mat[0][0] = xv.m_x;
	m_matrix.mat[1][0] = xv.m_y;
	m_matrix.mat[2][0] = xv.m_z;
	m_matrix.mat[3][0] = -(xv.m_x * p0.m_x + xv.m_y * p0.m_y + xv.m_z * p0.m_z);
	m_matrix.mat[0][1] = yv.m_x;
	m_matrix.mat[1][1] = yv.m_y;
	m_matrix.mat[2][1] = yv.m_z;
	m_matrix.mat[3][1] = -(yv.m_x * p0.m_x + yv.m_y * p0.m_y + yv.m_z * p0.m_z);
	m_matrix.mat[0][2] = zv.m_x;
	m_matrix.mat[1][2] = zv.m_y;
	m_matrix.mat[2][2] = zv.m_z;
	m_matrix.mat[3][2] = -(zv.m_x * p0.m_x + zv.m_y * p0.m_y + zv.m_z * p0.m_z);
	m_matrix.mat[0][3] = 0.0;
	m_matrix.mat[1][3] = 0.0;
	m_matrix.mat[2][3] = 0.0;
	m_matrix.mat[3][3] = 1.0;
}

void CP_HomogenousCSys::SetMirror (double a, double b, double c, double d)
{
	double l = sqrt(a*a + b*b + c*c);

	if (l > TOLER)
	{
		a /= l;
		b /= l;
		c /= l;
		d /= l;

		m_matrix.mat[0][0] = 1 - 2 * a * a;
		m_matrix.mat[1][0] = - 2 * a * b;
		m_matrix.mat[2][0] = - 2 * a * c;
		m_matrix.mat[3][0] = - 2 * a * d;
		m_matrix.mat[0][1] = - 2 * a * b;
		m_matrix.mat[1][1] = 1 - 2 * b * b;
		m_matrix.mat[2][1] = - 2 * b * c;
		m_matrix.mat[3][1] = - 2 * b * d;
		m_matrix.mat[0][2] = - 2 * a * c;
		m_matrix.mat[1][2] = - 2 * b * c;
		m_matrix.mat[2][2] = 1 - 2 * c * c;
		m_matrix.mat[3][2] = - 2 * c * d;
		m_matrix.mat[0][3] = 0;
		m_matrix.mat[1][3] = 0;
		m_matrix.mat[2][3] = 0;
		m_matrix.mat[3][3] = 1;
	}
}
