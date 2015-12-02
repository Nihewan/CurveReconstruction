#include "stdafx.h"
#include "CP_Mesh.h"

CP_Mesh3D::CP_Mesh3D()
{
}

CP_Mesh3D::~CP_Mesh3D()
{
	for (unsigned int i = 0; i < m_triArr.size(); ++i)
		delete m_triArr[i];
	for (unsigned int i = 0; i < m_ptArr.size(); ++i)
		delete m_ptArr[i];
}

CP_MeshTriangle3D *	CP_Mesh3D::GetTriangle(int index) {
	ASSERT(index >= 0 && (unsigned int)index < m_triArr.size());
	return m_triArr[index];
}

void	CP_Mesh3D::AddTriangle(CP_MeshTriangle3D *pTri) {
	ASSERT(pTri != NULL);
	m_triArr.push_back(pTri);
}

void CP_Mesh3D::AddTriangle(CP_MeshVertex3D* v0, CP_MeshVertex3D* v1, CP_MeshVertex3D* v2) {
	ASSERT(v0 && v1 && v2);
	CP_MeshTriangle3D *pTri = new CP_MeshTriangle3D(v0, v1, v2);
	AddTriangle(pTri);
}

CP_MeshVertex3D *CP_Mesh3D::GetVert(int index) {
	ASSERT(index >= 0 && (unsigned int)index < m_ptArr.size());
	return m_ptArr[index];
}

void	CP_Mesh3D::AddVert(CP_MeshVertex3D *pt) {
	ASSERT(pt != NULL);
	m_ptArr.push_back(pt);
}
