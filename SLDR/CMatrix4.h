#pragma once
#include "CP_PointVector.h"

class CMatrix4
{
public:
	double mat[4][4];
public:
	CMatrix4(void);
	~CMatrix4(void);

	// ¸³Öµ²Ù×÷·û
	void operator += (const CMatrix4& m);
	void operator -= (const CMatrix4& m);
	void operator *= (const CMatrix4& m);
	void operator *= (double num);
	void operator /= (double num);

	// ¾ØÕó¼Ó
	CMatrix4 operator + (const CMatrix4& m) const;
	// ¾ØÕó¼õ
	CMatrix4 operator - (const CMatrix4& m) const;
	// ¾ØÕó³Ë
	CMatrix4 operator * (const CMatrix4& m) const;
	// Êý³Ë
	CMatrix4 operator * (double num) const;
	// Êý³ý
	CMatrix4 operator / (double num) const;
	// ÇóÄæ¾ØÕó
	CMatrix4 operator ! () const;

	void SetUnit ();
	void SetZero ();
	void SetRotateV(const CP_Vector3D& v, double ang);
};

CP_Point3D operator * (const CP_Point3D& p, const CMatrix4& mat);
CP_Vector3D operator * (const CP_Vector3D& p, const CMatrix4& mat);