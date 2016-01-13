#include "stdafx.h"
#include "CP_Mesh.h"
#include "CP_Topology.h"

CP_MeshVertex3D::CP_MeshVertex3D(CP_Point3D &pt) : m_point(pt) {}
CP_MeshVertex3D::~CP_MeshVertex3D() {}

void	CP_MeshVertex3D::AddTriangle(CP_MeshTriangle3D *pTri) {
	ASSERT(pTri != NULL); 
	m_adjTriangle.push_back(pTri);
}

CP_MeshTriangle3D*	CP_MeshVertex3D::GetTriangle(int index) {
	ASSERT(index >= 0 && (unsigned int)index < m_adjTriangle.size());
	return m_adjTriangle[index];
};

double&	CP_MeshVertex3D::x() {
	return m_point.m_x;
}

double&	CP_MeshVertex3D::y() {
	return m_point.m_y;
}

double&	CP_MeshVertex3D::z() {
	return m_point.m_z;
}

CP_Vector3D CP_MeshVertex3D::GetVtNormal() {
	CP_Vector3D n;
	for (unsigned int i = 0; i < m_adjTriangle.size(); ++i) {
		n += m_adjTriangle[i]->GetNormal();//每个点的m_adjTriangle
		//n += m_adjTriangle[i]->n;
	}
	n.Normalize();
	return n;
}

bool CP_MeshVertex3D::ExistTriangle(CP_MeshTriangle3D *t)
{
	for (unsigned int i = 0; i < m_adjTriangle.size(); i++)
	{
		int abc = 0;
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				if (abs(m_adjTriangle[i]->GetVert(k)->x() - t->GetVert(j)->x()) < TOL&&abs(m_adjTriangle[i]->GetVert(k)->y() - t->GetVert(j)->y()) < TOL&&abs(m_adjTriangle[i]->GetVert(k)->z() - t->GetVert(j)->z()) < TOL)
					abc++;
			}

		}
		if (abc == 3)
			return true;
	}
	return false;
}
