#include "stdafx.h"
#include "CP_Mesh.h"
#include "gl/GL.h"
#include "gl/GLU.h"
#include "CP_PointVector.h"

CP_MeshTriangle3D::CP_MeshTriangle3D(CP_MeshVertex3D *v0, CP_MeshVertex3D *v1, CP_MeshVertex3D *v2)
{
	m_vertice[0] = v0;
	m_vertice[1] = v1;
	m_vertice[2] = v2;
	v0->AddTriangle(this);
	v1->AddTriangle(this);
	v2->AddTriangle(this);
}

CP_MeshTriangle3D::CP_MeshTriangle3D(CP_GeneralPolygon2D::CP_GeneralEdge *e1, 
									 CP_GeneralPolygon2D::CP_GeneralEdge *e2) 
{
	if (e1->m_pEnd == e2->m_pStart)
		m_vertice[2] = e2->m_pEnd;
	else
		m_vertice[2] = e2->m_pStart;
	m_vertice[0] = e1->m_pStart;
	m_vertice[1] = e1->m_pEnd;

	m_vertice[0]->AddTriangle(this);
	m_vertice[1]->AddTriangle(this);
	m_vertice[2]->AddTriangle(this);
}


CP_MeshTriangle3D::~CP_MeshTriangle3D(void)
{
}

CP_MeshVertex3D *CP_MeshTriangle3D::GetVert(int index) {
	ASSERT(index >= 0 && index < 3);
	return m_vertice[index];
}

CP_Vector3D CP_MeshTriangle3D::GetNormal() {
	CP_Vector3D ntmp = (m_vertice[1]->m_point - m_vertice[0]->m_point) ^ 
		(m_vertice[2]->m_point - m_vertice[0]->m_point);
	return ntmp;
}

void CP_MeshTriangle3D::DrawEdge() {
	glBegin(GL_LINES);
	for (int i = 0; i < 3; ++i) {
		glVertex3d(m_vertice[i]->x(), m_vertice[i]->y(), m_vertice[i]->z());
	}
	glEnd();
}

void CP_MeshTriangle3D::DrawSolid(bool isReverse /* = false */) {
	glBegin(GL_POLYGON);
	if (isReverse) {
		for (int i = 0; i < 3; ++i) {
			glVertex3d(m_vertice[i]->x(), m_vertice[i]->y(), m_vertice[i]->z());
		}
	} else {
		for (int i = 2; i >= 0; --i) {
			glVertex3d(m_vertice[i]->x(), m_vertice[i]->y(), m_vertice[i]->z());
		}
	}
	glEnd();
}

