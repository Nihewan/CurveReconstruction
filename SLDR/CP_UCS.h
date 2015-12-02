#pragma once
#include "CP_PointVector.h"

class CP_UCS
{
public:
	CP_Point3D   m_origin;
	CP_Vector3D  m_axisX;
	CP_Vector3D	 m_axisY;
	CP_Vector3D  m_axisZ;
	double  m_scale;
	static const int	m_color[3][3];

public:
	CP_UCS(void);
	~CP_UCS(void);
	CP_UCS(CP_Point3D origin, CP_Vector3D axisX, CP_Vector3D axisZ, double scale = 1.0);

	CP_Point3D GetWCSPos(CP_Point3D pt);
	void SetUcs(CP_Point3D origin, CP_Vector3D xaxis, CP_Vector3D zaxis, double scale = 1.0);
	void SetUcs(CP_Point3D origin, CP_Vector3D xaxis, CP_Vector3D yaxis,CP_Vector3D zaxis, double scale = 1.0);
	void DrawUCS(CP_Point3D centerPt, double scale);
};

