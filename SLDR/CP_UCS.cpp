#include "StdAfx.h"
#include "CP_UCS.h"
#include "CP_HomogenousCSys.h"
#include "gl/GL.h"


const int CP_UCS::m_color[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

CP_UCS::CP_UCS(void)
{
	m_axisX.m_x = 0;
	m_axisX.m_y = 0;
	m_axisX.m_z = 0;
	m_axisY.m_x = 0;
	m_axisY.m_y = 0;
	m_axisY.m_z = 0;
	m_axisZ.m_x = 0;
	m_axisZ.m_y = 0;
	m_axisZ.m_z = 0;
}


CP_UCS::~CP_UCS(void)
{
}

CP_UCS::CP_UCS(CP_Point3D origin, CP_Vector3D axisX, CP_Vector3D axisZ, double scale /*= 1.0*/):
m_origin(origin), m_axisX(axisX), m_axisZ(axisZ), m_scale(scale)
{
	m_axisY = m_axisZ ^ m_axisX;
}

CP_Point3D CP_UCS::GetWCSPos(CP_Point3D pt) {
	CP_HomogenousCSys hcs1, hcs2;
	double scale = m_scale;
	hcs1.SetScale(scale, scale, scale);
	hcs2.SetLocal(m_origin, m_axisZ, m_axisX);
	return pt * (hcs1.m_matrix * (!(hcs2.m_matrix)));
}

void CP_UCS::SetUcs(CP_Point3D origin, CP_Vector3D xaxis, CP_Vector3D zaxis, double scale /* = 1.0 */) {
	m_origin = origin;
	m_axisX = xaxis;
	m_axisZ = zaxis;
	m_axisY = m_axisZ ^ m_axisX;
	m_scale = scale;
}

void CP_UCS::SetUcs(CP_Point3D origin, CP_Vector3D xaxis, CP_Vector3D yaxis, CP_Vector3D zaxis, double scale /* = 1.0 */) {
	m_origin = origin;
	m_axisX = xaxis;
	m_axisZ = zaxis;
	m_axisY = yaxis;
	m_scale = scale;
}

void CP_UCS::DrawUCS(CP_Point3D centerPt, double scale) {
	m_scale = scale;
	CP_Vector3D *uscVecs[3] = {&m_axisX, &m_axisY, &m_axisZ};
	glLineWidth(3.0f);
	for (int i = 0; i < 3; ++i) {
		glColor3d(m_color[i][0], m_color[i][1], m_color[i][2]);
		glBegin(GL_LINES);
		glVertex3d(centerPt.m_x, centerPt.m_y, centerPt.m_z);
		glVertex3d(centerPt.m_x + uscVecs[i]->m_x * scale,
			centerPt.m_y + uscVecs[i]->m_y * scale, 
			centerPt.m_z + uscVecs[i]->m_z * scale);
		glEnd();
	}
}