#pragma once
#include "CP_PointVector.h"

class CMatrix4
{
public:
	double mat[4][4];
public:
	CMatrix4(void);
	~CMatrix4(void);

	// ��ֵ������
	void operator += (const CMatrix4& m);
	void operator -= (const CMatrix4& m);
	void operator *= (const CMatrix4& m);
	void operator *= (double num);
	void operator /= (double num);

	// �����
	CMatrix4 operator + (const CMatrix4& m) const;
	// �����
	CMatrix4 operator - (const CMatrix4& m) const;
	// �����
	CMatrix4 operator * (const CMatrix4& m) const;
	// ����
	CMatrix4 operator * (double num) const;
	// ����
	CMatrix4 operator / (double num) const;
	// �������
	CMatrix4 operator ! () const;

	void SetUnit ();
	void SetZero ();
	void SetRotateV(const CP_Vector3D& v, double ang);
};

CP_Point3D operator * (const CP_Point3D& p, const CMatrix4& mat);
CP_Vector3D operator * (const CP_Vector3D& p, const CMatrix4& mat);