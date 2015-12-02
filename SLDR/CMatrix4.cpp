#include "StdAfx.h"
#include "CMatrix4.h"
#include <math.h>

CMatrix4::CMatrix4(void)
{
}


CMatrix4::~CMatrix4(void)
{
}

void CMatrix4::operator += (const CMatrix4& m)
{
	mat[0][0] += m.mat[0][0];
	mat[0][1] += m.mat[0][1];
	mat[0][2] += m.mat[0][2];
	mat[0][3] += m.mat[0][3];
	mat[1][0] += m.mat[1][0];
	mat[1][1] += m.mat[1][1];
	mat[1][2] += m.mat[1][2];
	mat[1][3] += m.mat[1][3];
	mat[2][0] += m.mat[2][0];
	mat[2][1] += m.mat[2][1];
	mat[2][2] += m.mat[2][2];
	mat[2][3] += m.mat[2][3];
	mat[3][0] += m.mat[3][0];
	mat[3][1] += m.mat[3][1];
	mat[3][2] += m.mat[3][2];
	mat[3][3] += m.mat[3][3];
}

void CMatrix4::operator -= (const CMatrix4& m)
{
	mat[0][0] -= m.mat[0][0];
	mat[0][1] -= m.mat[0][1];
	mat[0][2] -= m.mat[0][2];
	mat[0][3] -= m.mat[0][3];
	mat[1][0] -= m.mat[1][0];
	mat[1][1] -= m.mat[1][1];
	mat[1][2] -= m.mat[1][2];
	mat[1][3] -= m.mat[1][3];
	mat[2][0] -= m.mat[2][0];
	mat[2][1] -= m.mat[2][1];
	mat[2][2] -= m.mat[2][2];
	mat[2][3] -= m.mat[2][3];
	mat[3][0] -= m.mat[3][0];
	mat[3][1] -= m.mat[3][1];
	mat[3][2] -= m.mat[3][2];
	mat[3][3] -= m.mat[3][3];
}

void CMatrix4::operator *= (double num)
{
	mat[0][0] *= num;
	mat[0][1] *= num;
	mat[0][2] *= num;
	mat[0][3] *= num;
	mat[1][0] *= num;
	mat[1][1] *= num;
	mat[1][2] *= num;
	mat[1][3] *= num;
	mat[2][0] *= num;
	mat[2][1] *= num;
	mat[2][2] *= num;
	mat[2][3] *= num;
	mat[3][0] *= num;
	mat[3][1] *= num;
	mat[3][2] *= num;
	mat[3][3] *= num;
}

void CMatrix4::operator /= (double num)
{
	double num2 = 1 / num;
	mat[0][0] *= num2;
	mat[0][1] *= num2;
	mat[0][2] *= num2;
	mat[0][3] *= num2;
	mat[1][0] *= num2;
	mat[1][1] *= num2;
	mat[1][2] *= num2;
	mat[1][3] *= num2;
	mat[2][0] *= num2;
	mat[2][1] *= num2;
	mat[2][2] *= num2;
	mat[2][3] *= num2;
	mat[3][0] *= num2;
	mat[3][1] *= num2;
	mat[3][2] *= num2;
	mat[3][3] *= num2;
}

void CMatrix4::operator *= (const CMatrix4& m)
{
	CMatrix4	aa;

	aa.mat[0][0] = mat[0][0]*m.mat[0][0] + mat[0][1]*m.mat[1][0] + mat[0][2]*m.mat[2][0] + mat[0][3]*m.mat[3][0];
	aa.mat[0][1] = mat[0][0]*m.mat[0][1] + mat[0][1]*m.mat[1][1] + mat[0][2]*m.mat[2][1] + mat[0][3]*m.mat[3][1];
	aa.mat[0][2] = mat[0][0]*m.mat[0][2] + mat[0][1]*m.mat[1][2] + mat[0][2]*m.mat[2][2] + mat[0][3]*m.mat[3][2];
	aa.mat[0][3] = mat[0][0]*m.mat[0][3] + mat[0][1]*m.mat[1][3] + mat[0][2]*m.mat[2][3] + mat[0][3]*m.mat[3][3];

	aa.mat[1][0] = mat[1][0]*m.mat[0][0] + mat[1][1]*m.mat[1][0] + mat[1][2]*m.mat[2][0] + mat[1][3]*m.mat[3][0];
	aa.mat[1][1] = mat[1][0]*m.mat[0][1] + mat[1][1]*m.mat[1][1] + mat[1][2]*m.mat[2][1] + mat[1][3]*m.mat[3][1];
	aa.mat[1][2] = mat[1][0]*m.mat[0][2] + mat[1][1]*m.mat[1][2] + mat[1][2]*m.mat[2][2] + mat[1][3]*m.mat[3][2];
	aa.mat[1][3] = mat[1][0]*m.mat[0][3] + mat[1][1]*m.mat[1][3] + mat[1][2]*m.mat[2][3] + mat[1][3]*m.mat[3][3];

	aa.mat[2][0] = mat[2][0]*m.mat[0][0] + mat[2][1]*m.mat[1][0] + mat[2][2]*m.mat[2][0] + mat[2][3]*m.mat[3][0];
	aa.mat[2][1] = mat[2][0]*m.mat[0][1] + mat[2][1]*m.mat[1][1] + mat[2][2]*m.mat[2][1] + mat[2][3]*m.mat[3][1];
	aa.mat[2][2] = mat[2][0]*m.mat[0][2] + mat[2][1]*m.mat[1][2] + mat[2][2]*m.mat[2][2] + mat[2][3]*m.mat[3][2];
	aa.mat[2][3] = mat[2][0]*m.mat[0][3] + mat[2][1]*m.mat[1][3] + mat[2][2]*m.mat[2][3] + mat[2][3]*m.mat[3][3];

	aa.mat[3][0] = mat[3][0]*m.mat[0][0] + mat[3][1]*m.mat[1][0] + mat[3][2]*m.mat[2][0] + mat[3][3]*m.mat[3][0];
	aa.mat[3][1] = mat[3][0]*m.mat[0][1] + mat[3][1]*m.mat[1][1] + mat[3][2]*m.mat[2][1] + mat[3][3]*m.mat[3][1];
	aa.mat[3][2] = mat[3][0]*m.mat[0][2] + mat[3][1]*m.mat[1][2] + mat[3][2]*m.mat[2][2] + mat[3][3]*m.mat[3][2];
	aa.mat[3][3] = mat[3][0]*m.mat[0][3] + mat[3][1]*m.mat[1][3] + mat[3][2]*m.mat[2][3] + mat[3][3]*m.mat[3][3];

	(*this) = aa;
}

CMatrix4 CMatrix4::operator + (const CMatrix4& m) const
{
	CMatrix4	aa (*this);

	aa.mat[0][0] += m.mat[0][0];
	aa.mat[0][1] += m.mat[0][1];
	aa.mat[0][2] += m.mat[0][2];
	aa.mat[0][3] += m.mat[0][3];
	aa.mat[1][0] += m.mat[1][0];
	aa.mat[1][1] += m.mat[1][1];
	aa.mat[1][2] += m.mat[1][2];
	aa.mat[1][3] += m.mat[1][3];
	aa.mat[2][0] += m.mat[2][0];
	aa.mat[2][1] += m.mat[2][1];
	aa.mat[2][2] += m.mat[2][2];
	aa.mat[2][3] += m.mat[2][3];
	aa.mat[3][0] += m.mat[3][0];
	aa.mat[3][1] += m.mat[3][1];
	aa.mat[3][2] += m.mat[3][2];
	aa.mat[3][3] += m.mat[3][3];

	return aa;
}

CMatrix4 CMatrix4::operator - (const CMatrix4& m) const
{
	CMatrix4	aa (*this);

	aa.mat[0][0] -= m.mat[0][0];
	aa.mat[0][1] -= m.mat[0][1];
	aa.mat[0][2] -= m.mat[0][2];
	aa.mat[0][3] -= m.mat[0][3];
	aa.mat[1][0] -= m.mat[1][0];
	aa.mat[1][1] -= m.mat[1][1];
	aa.mat[1][2] -= m.mat[1][2];
	aa.mat[1][3] -= m.mat[1][3];
	aa.mat[2][0] -= m.mat[2][0];
	aa.mat[2][1] -= m.mat[2][1];
	aa.mat[2][2] -= m.mat[2][2];
	aa.mat[2][3] -= m.mat[2][3];
	aa.mat[3][0] -= m.mat[3][0];
	aa.mat[3][1] -= m.mat[3][1];
	aa.mat[3][2] -= m.mat[3][2];
	aa.mat[3][3] -= m.mat[3][3];

	return aa;
}

CMatrix4 CMatrix4::operator * (const CMatrix4& m) const
{

	CMatrix4	aa;

	aa.mat[0][0] = mat[0][0]*m.mat[0][0] + mat[0][1]*m.mat[1][0] + mat[0][2]*m.mat[2][0] + mat[0][3]*m.mat[3][0];
	aa.mat[0][1] = mat[0][0]*m.mat[0][1] + mat[0][1]*m.mat[1][1] + mat[0][2]*m.mat[2][1] + mat[0][3]*m.mat[3][1];
	aa.mat[0][2] = mat[0][0]*m.mat[0][2] + mat[0][1]*m.mat[1][2] + mat[0][2]*m.mat[2][2] + mat[0][3]*m.mat[3][2];
	aa.mat[0][3] = mat[0][0]*m.mat[0][3] + mat[0][1]*m.mat[1][3] + mat[0][2]*m.mat[2][3] + mat[0][3]*m.mat[3][3];

	aa.mat[1][0] = mat[1][0]*m.mat[0][0] + mat[1][1]*m.mat[1][0] + mat[1][2]*m.mat[2][0] + mat[1][3]*m.mat[3][0];
	aa.mat[1][1] = mat[1][0]*m.mat[0][1] + mat[1][1]*m.mat[1][1] + mat[1][2]*m.mat[2][1] + mat[1][3]*m.mat[3][1];
	aa.mat[1][2] = mat[1][0]*m.mat[0][2] + mat[1][1]*m.mat[1][2] + mat[1][2]*m.mat[2][2] + mat[1][3]*m.mat[3][2];
	aa.mat[1][3] = mat[1][0]*m.mat[0][3] + mat[1][1]*m.mat[1][3] + mat[1][2]*m.mat[2][3] + mat[1][3]*m.mat[3][3];

	aa.mat[2][0] = mat[2][0]*m.mat[0][0] + mat[2][1]*m.mat[1][0] + mat[2][2]*m.mat[2][0] + mat[2][3]*m.mat[3][0];
	aa.mat[2][1] = mat[2][0]*m.mat[0][1] + mat[2][1]*m.mat[1][1] + mat[2][2]*m.mat[2][1] + mat[2][3]*m.mat[3][1];
	aa.mat[2][2] = mat[2][0]*m.mat[0][2] + mat[2][1]*m.mat[1][2] + mat[2][2]*m.mat[2][2] + mat[2][3]*m.mat[3][2];
	aa.mat[2][3] = mat[2][0]*m.mat[0][3] + mat[2][1]*m.mat[1][3] + mat[2][2]*m.mat[2][3] + mat[2][3]*m.mat[3][3];

	aa.mat[3][0] = mat[3][0]*m.mat[0][0] + mat[3][1]*m.mat[1][0] + mat[3][2]*m.mat[2][0] + mat[3][3]*m.mat[3][0];
	aa.mat[3][1] = mat[3][0]*m.mat[0][1] + mat[3][1]*m.mat[1][1] + mat[3][2]*m.mat[2][1] + mat[3][3]*m.mat[3][1];
	aa.mat[3][2] = mat[3][0]*m.mat[0][2] + mat[3][1]*m.mat[1][2] + mat[3][2]*m.mat[2][2] + mat[3][3]*m.mat[3][2];
	aa.mat[3][3] = mat[3][0]*m.mat[0][3] + mat[3][1]*m.mat[1][3] + mat[3][2]*m.mat[2][3] + mat[3][3]*m.mat[3][3];

	return aa;
}

CMatrix4 CMatrix4::operator * (double num) const
{
	CMatrix4	aa (*this);

	aa.mat[0][0] *= num;
	aa.mat[0][1] *= num;
	aa.mat[0][2] *= num;
	aa.mat[0][3] *= num;
	aa.mat[1][0] *= num;
	aa.mat[1][1] *= num;
	aa.mat[1][2] *= num;
	aa.mat[1][3] *= num;
	aa.mat[2][0] *= num;
	aa.mat[2][1] *= num;
	aa.mat[2][2] *= num;
	aa.mat[2][3] *= num;
	aa.mat[3][0] *= num;
	aa.mat[3][1] *= num;
	aa.mat[3][2] *= num;
	aa.mat[3][3] *= num;

	return aa;
}

CMatrix4 CMatrix4::operator / (double num) const
{
	CMatrix4	aa (*this);

	if (fabs(num) > TOLER)
	{
		double num2 = 1 / num;
		aa.mat[0][0] *= num2;
		aa.mat[0][1] *= num2;
		aa.mat[0][2] *= num2;
		aa.mat[0][3] *= num2;
		aa.mat[1][0] *= num2;
		aa.mat[1][1] *= num2;
		aa.mat[1][2] *= num2;
		aa.mat[1][3] *= num2;
		aa.mat[2][0] *= num2;
		aa.mat[2][1] *= num2;
		aa.mat[2][2] *= num2;
		aa.mat[2][3] *= num2;
		aa.mat[3][0] *= num2;
		aa.mat[3][1] *= num2;
		aa.mat[3][2] *= num2;
		aa.mat[3][3] *= num2;	
	}
	return aa;
}

CMatrix4 CMatrix4::operator ! () const
{
	double delta, buf;
	int i, j, k;

	CMatrix4 aa (*this), mat2;

	mat2.SetUnit();
	for (i=0; i<4; i++)
	{
		delta = aa.mat[i][i];

		if (fabs (delta)<=TOLER)
		{
			for (j=i+1; j<4; j++)
				if (fabs (aa.mat[j][i]) > TOLER) break;
			if (j >= 4)
				return (*this);
			buf				= aa.mat[j][0];
			aa.mat[j][0]	= aa.mat[i][0];
			aa.mat[i][0]	= buf;
			buf				= mat2.mat[j][0];
			mat2.mat[j][0]	= mat2.mat[i][0];
			mat2.mat[i][0]	= buf;

			buf				= aa.mat[j][1];
			aa.mat[j][1]	= aa.mat[i][1];
			aa.mat[i][1]	= buf;
			buf				= mat2.mat[j][1];
			mat2.mat[j][1]	= mat2.mat[i][1];
			mat2.mat[i][1]	= buf;

			buf				= aa.mat[j][2];
			aa.mat[j][2]	= aa.mat[i][2];
			aa.mat[i][2]	= buf;
			buf				= mat2.mat[j][2];
			mat2.mat[j][2]	= mat2.mat[i][2];
			mat2.mat[i][2]	= buf;

			buf				= aa.mat[j][3];
			aa.mat[j][3]	= aa.mat[i][3];
			aa.mat[i][3]	= buf;
			buf				= mat2.mat[j][3];
			mat2.mat[j][3]	= mat2.mat[i][3];
			mat2.mat[i][3]	= buf;
			delta			= 1 / aa.mat[i][i];
		}
		else
			delta = 1 / delta;

		aa.mat[i][0]   *= delta;
		aa.mat[i][1]   *= delta;
		aa.mat[i][2]   *= delta;
		aa.mat[i][3]   *= delta;
		mat2.mat[i][0] *= delta;
		mat2.mat[i][1] *= delta;
		mat2.mat[i][2] *= delta;
		mat2.mat[i][3] *= delta;

		for (k=0; k<i; k++)
		{
			buf = -1*aa.mat[k][i];
			aa.mat[k][0]   = aa.mat[i][0]*buf	+ aa.mat[k][0];
			aa.mat[k][1]   = aa.mat[i][1]*buf	+ aa.mat[k][1];
			aa.mat[k][2]   = aa.mat[i][2]*buf	+ aa.mat[k][2];
			aa.mat[k][3]   = aa.mat[i][3]*buf	+ aa.mat[k][3];
			mat2.mat[k][0] = mat2.mat[i][0]*buf + mat2.mat[k][0];
			mat2.mat[k][1] = mat2.mat[i][1]*buf + mat2.mat[k][1];
			mat2.mat[k][2] = mat2.mat[i][2]*buf + mat2.mat[k][2];
			mat2.mat[k][3] = mat2.mat[i][3]*buf + mat2.mat[k][3];

		}
		for (k=i+1; k<4; k++)		
		{
			buf = -1*aa.mat[k][i];
			aa.mat[k][0]   = aa.mat[i][0]*buf	+ aa.mat[k][0];
			aa.mat[k][1]   = aa.mat[i][1]*buf	+ aa.mat[k][1];
			aa.mat[k][2]   = aa.mat[i][2]*buf	+ aa.mat[k][2];
			aa.mat[k][3]   = aa.mat[i][3]*buf	+ aa.mat[k][3];
			mat2.mat[k][0] = mat2.mat[i][0]*buf + mat2.mat[k][0];
			mat2.mat[k][1] = mat2.mat[i][1]*buf + mat2.mat[k][1];
			mat2.mat[k][2] = mat2.mat[i][2]*buf + mat2.mat[k][2];
			mat2.mat[k][3] = mat2.mat[i][3]*buf + mat2.mat[k][3];
		}
	}

	return mat2;
}


CP_Point3D operator * (const CP_Point3D& p, const CMatrix4& mat)
{
	double x, y, z, w;

	const double *mt = &mat.mat[0][0];
	x = p.m_x*mt[0] + p.m_y*mt[4] + p.m_z*mt[8]  + mt[12];
	y = p.m_x*mt[1] + p.m_y*mt[5] + p.m_z*mt[9]  + mt[13];
	z = p.m_x*mt[2] + p.m_y*mt[6] + p.m_z*mt[10] + mt[14];
	w = p.m_x*mt[3] + p.m_y*mt[7] + p.m_z*mt[11] + mt[15];

	if (fabs (w) < TOLER)
		return CP_Point3D (0, 0, 0);
	else
		return CP_Point3D (x/w, y/w, z/w);
}

CP_Vector3D operator * (const CP_Vector3D& p, const CMatrix4& mat)
{
	double x, y, z;

	const double *mt = &mat.mat[0][0];
	x = p.m_x*mt[0] + p.m_y*mt[4] + p.m_z*mt[8];
	y = p.m_x*mt[1] + p.m_y*mt[5] + p.m_z*mt[9];
	z = p.m_x*mt[2] + p.m_y*mt[6] + p.m_z*mt[10];

	return CP_Vector3D (x, y, z);
}

void CMatrix4::SetUnit ()
{
	SetZero();
	mat[0][0]=1;
	mat[1][1]=1;
	mat[2][2]=1;
	mat[3][3]=1;
}

void CMatrix4::SetZero ()
{
	memset (mat, 0, sizeof (mat));
}

void CMatrix4::SetRotateV (const CP_Vector3D& v, double ang)
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

	mat[0][0] = aa + (1-aa ) * cou;
	mat[1][0] = ab * (1-cou) - c*siu;
	mat[2][0] = ac * (1-cou) + b*siu;
	mat[3][0] = 0.;
	mat[0][1] = ab * (1-cou) + c*siu;
	mat[1][1] = bb + (1-bb ) * cou;
	mat[2][1] = bc * (1-cou) - a*siu;
	mat[3][1] = 0.;
	mat[0][2] = ac * (1-cou) - b*siu;
	mat[1][2] = bc * (1-cou) + a*siu;
	mat[2][2] = cc + (1-cc ) * cou;
	mat[3][2] = 0.;
	mat[0][3] = 0.;
	mat[1][3] = 0.;
	mat[2][3] = 0.;
	mat[3][3] = 1.;
}

