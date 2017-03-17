// CP_PointVector.cpp 实现类CP_Point2D、CP_Point3D、CP_Vector2D和CP_Vector3D
#include "stdafx.h"
#include "CP_PointVector.h"
#include "base_geometricoper.h"
#include <math.h>
/************************************************************************/
/* CP_Point3D                                                           */
/************************************************************************/

CP_Point3D::CP_Point3D(double newx, double newy, double newz):m_x(newx), m_y(newy), m_z(newz)
{
	flag=false;
} 

CP_Point3D& CP_Point3D::operator=(const CP_Point3D& tmp)
{
	m_x=tmp.m_x;
	m_y=tmp.m_y;
	m_z=tmp.m_z;
	return *this;
}

CP_Point3D& CP_Point3D::operator+= (const CP_Point3D &pt) {
	m_x += pt.m_x;
	m_y += pt.m_y;
	m_z += pt.m_z;
	return *this;
}

CP_Point3D& CP_Point3D::operator+= (const CP_Vector3D &v) {
	m_x += v.m_x;
	m_y += v.m_y;
	m_z += v.m_z;
	return *this;
}

CP_Point3D& CP_Point3D::operator-= (const CP_Point3D &pt) {
	m_x -= pt.m_x;
	m_y -= pt.m_y;
	m_z -= pt.m_z;
	return *this;
}

bool CP_Point3D::operator==(const CP_Point3D &pt) {
	return (m_x == pt.m_x) && (m_y == pt.m_y) && (m_z == pt.m_z);
}


CP_Point3D CP_Point3D::operator - ()const {
	return CP_Point3D(-m_x, -m_y, -m_z);
}

//CP_Vector3D CP_Point3D::GetVtNormal()
//{
//	CP_Vector3D n;
//	for (unsigned int i = 0; i < m_adjTriangle.size(); ++i) {
//		n += m_adjTriangle[i]->GetNormal();
//	}
//	n.Normalize();
//	return n;
//}
//
//void	CP_Point3D::AddTriangle(CP_Triganle3D *pTri) {
//	ASSERT(pTri != NULL); 
//	m_adjTriangle.push_back(pTri);
//}

/************************************************************************/
/* CP_Point4D                                                           */
/************************************************************************/

CP_Point4D::CP_Point4D (double x, double y, double z, double w)
	:m_x(x), m_y(y), m_z(z), m_w(w)
{
}

CP_Point4D& CP_Point4D::operator += (const CP_Point4D& v)
{ 
	m_x += v.m_x;	
	m_y += v.m_y;	
	m_z += v.m_z;	
	m_w += v.m_w; 
	return *this;
}

CP_Point4D& CP_Point4D::operator -= (const CP_Point4D& v)
{ 
	m_x -= v.m_x;	
	m_y -= v.m_y;	
	m_z -= v.m_z;	
	m_w -= v.m_w; 
	return *this;
}

CP_Point4D CP_Point4D::operator - () const
{
	return CP_Point4D (-m_x, -m_y, -m_z, -m_w);  
}

CP_Point4D operator + (const CP_Point4D& p, const CP_Point4D& q)
{
	return CP_Point4D (p.m_x + q.m_x, p.m_y + q.m_y, p.m_z + q.m_z, p.m_w+q.m_w); 
}

double& CP_Point4D::operator [] (unsigned int i)
{
	ASSERT(i<4);
	double* p=reinterpret_cast<double*>(this);
	return p[i];
}

double CP_Point4D::operator [] (unsigned int i) const
{
	ASSERT(i<4);
	const double* p=reinterpret_cast<const double*>(this);
	return p[i];
}

void CP_Point4D::SetValue(double x_, double y_, double z_, double w_)
{ 
	m_x = x_;		m_y = y_;		m_z = z_;		m_w = w_;
}

void CP_Point4D::SetValue(const CP_Vector3D& v,double w_)
{
	m_x = v.m_x*w_;	  m_y = v.m_y * w_;	  m_z = v.m_z* w_;	 m_w = w_;
}

void CP_Point4D::GetValueFrom3D(const CP_Point3D& p, double w_)
{
	m_x = p.m_x * w_;  
	m_y = p.m_y * w_;  
	m_z = p.m_z* w_;  
	m_w = w_;
}

void CP_Point4D::SetPoint3DValue(CP_Point3D& p, double& w_) const
{
	w_ = m_w;
	double cw = 1.0 / m_w;
	p.m_x = m_x * cw;
	p.m_y = m_y * cw;
	p.m_z = m_z * cw;
}

CP_Point4D operator * (const CP_Point4D& pt, double num) 
{ 
	return CP_Point4D (pt.m_x * num, pt.m_y * num, pt.m_z*num, pt.m_w*num); 
}
CP_Point4D operator * (double num, const CP_Point4D& pt)
{
	return CP_Point4D(num*pt.m_x, num*pt.m_y, num*pt.m_z, num*pt.m_w);
}

CP_Point4D operator / (const CP_Point4D& pt, double num) 
{ 
	num  = 1.0/num;
	return CP_Point4D (pt.m_x * num, pt.m_y * num, pt.m_z*num, pt.m_w*num); 
}


CP_Vector3D::CP_Vector3D (double newx, double newy, double newz):m_x(newx), m_y(newy), m_z(newz)
{
} 

CP_Vector3D::CP_Vector3D(const CP_Vector3D& v)
{
	m_x = v.m_x;
	m_y = v.m_y;
	m_z = v.m_z;  
}

CP_Vector3D& CP_Vector3D::operator += (const CP_Vector3D& v)
{ 
    m_x += v.m_x;
    m_y += v.m_y;
    m_z += v.m_z;  
    return *this; 
} 

CP_Vector3D& CP_Vector3D::operator -= (const CP_Vector3D& v)
{
    m_x -= v.m_x;
    m_y -= v.m_y;
    m_z -= v.m_z; 
    return *this; 
}

CP_Vector3D& CP_Vector3D::operator *= (double num)
{ 
    m_x *= num;
    m_y *= num;
    m_z *= num; 
    return *this; 
} 

CP_Vector3D& CP_Vector3D::operator /= (double num)
{
    num = 1.0/num;
    m_x *= num;
    m_y *= num;
    m_z *= num;
    return *this;
} 

CP_Vector3D& CP_Vector3D::operator ^= (const CP_Vector3D& v)
{ 
    double a =   m_y * v.m_z - m_z * v.m_y;
    double b = - m_x * v.m_z + m_z * v.m_x;
    double c =   m_x * v.m_y - m_y * v.m_x;

    m_x = a;
    m_y = b;
    m_z = c;
    return *this;
} 

CP_Vector3D CP_Vector3D::operator - ( ) const
{ 
    return CP_Vector3D (-m_x, -m_y, -m_z); 
} 

double CP_Vector3D::GetLength( )  const                             
{ 
    return sqrt(m_x*m_x + m_y*m_y + m_z*m_z); 
} 

void CP_Vector3D::Normalize( )
{
    double a = GetLength( );
    (*this) /= a; // 注意: 这里没有处理除数为0的情况
}

void CP_Vector3D::SetValue(double newx, double newy, double newz)
{
    m_x=newx;
    m_y=newy;
    m_z=newz;
} 

CP_Point3D operator + (const CP_Point3D& pt, const CP_Vector3D& v)
{
    return CP_Point3D (pt.m_x + v.m_x, pt.m_y + v.m_y, pt.m_z + v.m_z); 
}

CP_Point3D operator - (const CP_Point3D& pt, const CP_Vector3D& v)
{
    return CP_Point3D (pt.m_x - v.m_x, pt.m_y - v.m_y, pt.m_z - v.m_z); 
} 

CP_Vector3D operator - (const CP_Point3D& p, const CP_Point3D& q)
{
    return CP_Vector3D (p.m_x - q.m_x, p.m_y - q.m_y, p.m_z - q.m_z); 
} 

CP_Point3D operator * (double num, const CP_Point3D &p) {
	return CP_Point3D(p.m_x * num, p.m_y * num, p.m_z * num);
}

CP_Point3D operator * (const CP_Point3D &p, double num) {
	return CP_Point3D(p.m_x * num, p.m_y * num, p.m_z * num);
}

CP_Point3D operator + (const CP_Point3D &pt0, const CP_Point3D &pt1) {
	return CP_Point3D(pt0.m_x + pt1.m_x, pt0.m_y + pt1.m_y, pt0.m_z + pt1.m_z);
}

CP_Point3D operator / (const CP_Point3D &pt0, double num) {
	return CP_Point3D(pt0.m_x / num, pt0.m_y / num, pt0.m_z / num);
}

CP_Vector3D operator + (const CP_Vector3D& u, const CP_Vector3D& v)
{
   return CP_Vector3D(u.m_x + v.m_x, u.m_y + v.m_y, u.m_z + v.m_z);
} 

CP_Vector3D operator - (const CP_Vector3D& u, const CP_Vector3D& v)
{
    return CP_Vector3D (u.m_x - v.m_x, u.m_y - v.m_y, u.m_z - v.m_z);
} 

// Operator: dot product
double operator * (const CP_Vector3D& u, const CP_Vector3D& v)
{
    return (u.m_x * v.m_x+u.m_y * v.m_y+ u.m_z * v.m_z);
} 

// Operator: cross product
CP_Vector3D operator ^ (const CP_Vector3D& u, const CP_Vector3D& v)
{
    return CP_Vector3D(u.m_y * v.m_z - u.m_z*v.m_y, 
                       -u.m_x*v.m_z+u.m_z*v.m_x,
                       u.m_x*v.m_y-u.m_y*v.m_x
                      );
} 

CP_Vector3D operator * (const CP_Vector3D& v, double num)
{
    return CP_Vector3D (v.m_x * num, v.m_y * num, v.m_z * num);
} 

CP_Vector3D operator * (double num,const CP_Vector3D& v)
{
	return CP_Vector3D (v.m_x * num, v.m_y * num, v.m_z * num);
} 

CP_Vector3D operator / (const CP_Vector3D& v, double num)
{
    num = 1.0/num; // 注意: 这里没有处理除数为0的情况
    return CP_Vector3D (v.m_x * num, v.m_y * num, v.m_z * num);
} 
