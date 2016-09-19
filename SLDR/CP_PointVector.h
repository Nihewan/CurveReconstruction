// CP_PointVector.h: 

#ifndef CP_POINTVECTOR_H_
#define CP_POINTVECTOR_H_

#include "CConstants.h"
#include <vector>

#define ZeroP3D		CP_Point3D(0.0, 0.0, 0.0) 
#define BaseXV3D	CP_Vector3D(1.0, 0.0, 0.0)
#define BaseYV3D	CP_Vector3D(0.0, 1.0, 0.0)
#define BaseZV3D	CP_Vector3D(0.0, 0.0, 1.0)

class CP_Vector3D;

class CP_BaseGeometricType
{
public:
	CP_BaseGeometricType(){};
	virtual ~CP_BaseGeometricType(){};
private:

};

class CP_Point3D : public CP_BaseGeometricType
{
public:
    double    m_x, m_y, m_z;
	std::vector<int> m_adjTriangle;
	std::vector<int> incidentpoly;
	bool flag;
public:
    explicit CP_Point3D (double newx=0.0, double newy=0.0, double newz=0.0);
	CP_Point3D& operator=(const CP_Point3D& tmp);
	CP_Point3D& operator += (const CP_Point3D &pt);
	CP_Point3D& operator += (const CP_Vector3D &v);
	CP_Point3D& operator -= (const CP_Point3D &pt);
	bool operator == (const CP_Point3D &pt);
	CP_Point3D operator -() const;
};

class CP_Point4D
{

public:
	double	m_x, m_y, m_z, m_w;
public:
	//构造函数
	explicit inline CP_Point4D (double x_=0., double y_=0., double z_=0., double w_=0.);
	~CP_Point4D(){}

	inline CP_Point4D& operator += (const CP_Point4D& v);
	inline CP_Point4D& operator -= (const CP_Point4D& v);

	//单目减
	inline CP_Point4D operator - () const;

	//取元素
	inline double& operator [] (unsigned int i);
	inline double  operator [] (unsigned int i) const;

	//赋值
	inline void SetValue(double x_=0., double y_=0., double z_=0., double w_=0.);
	inline void SetValue(const CP_Vector3D& v,double w_);

	//四维空间点和三维点奇次坐标的相互转化
	void GetValueFrom3D(const CP_Point3D& p, double w_);
	void SetPoint3DValue(CP_Point3D& p, double& weight) const;
};


class CP_Vector3D : public CP_BaseGeometricType
{
public:
    double    m_x, m_y, m_z;

public:
    explicit CP_Vector3D (double newx=0.0, double newy=0.0, double newz=0.0);

    CP_Vector3D& operator += (const CP_Vector3D& v);
    CP_Vector3D& operator -= (const CP_Vector3D& v);
    CP_Vector3D& operator *= (double num);
    CP_Vector3D& operator /= (double num);
    CP_Vector3D& operator ^= (const CP_Vector3D& v);

    CP_Vector3D operator - () const;

    double GetLength ( ) const; 

    void Normalize( ); 
    void SetValue(double newx=0.0, double newy=0.0,double newz=0.0);
};

extern CP_Point3D operator + (const CP_Point3D& pt, const CP_Vector3D& v);
extern CP_Point3D operator - (const CP_Point3D& pt, const CP_Vector3D& v);
extern CP_Vector3D operator - (const CP_Point3D& p, const CP_Point3D& q);
extern CP_Point3D operator * (double num, const CP_Point3D& p);
extern CP_Point3D operator * (const CP_Point3D& p, double num);
extern CP_Point3D operator + (const CP_Point3D& pt0, const CP_Point3D& pt1);
extern CP_Point3D operator / (const CP_Point3D& p, double num);

extern CP_Point4D operator * (double num, const CP_Point4D& pt);
extern CP_Point4D operator + (const CP_Point4D& p, const CP_Point4D& q);

extern CP_Vector3D operator + (const CP_Vector3D& u, const CP_Vector3D& v);
extern CP_Vector3D operator - (const CP_Vector3D& u, const CP_Vector3D& v);
extern double operator * (const CP_Vector3D& u, const CP_Vector3D& v);
extern CP_Vector3D operator ^ (const CP_Vector3D& u, const CP_Vector3D& v);
extern CP_Vector3D operator * (const CP_Vector3D& v, double num);
extern CP_Vector3D operator * (double num, const CP_Vector3D& v);
extern CP_Vector3D operator / (const CP_Vector3D& v, double num);

#endif