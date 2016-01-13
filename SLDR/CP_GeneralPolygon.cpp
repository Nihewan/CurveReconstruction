#include "stdafx.h"
#include "CP_Mesh.h"

CP_GeneralPolygon2D::~CP_GeneralPolygon2D() {
	for (unsigned int i = 0; i < m_edgeArr.size(); ++i)
		delete m_edgeArr[i];
	m_edgeArr.clear();
}

int	CP_GeneralPolygon2D::GetEdgeSize() {
	return m_edgeArr.size();
}

CP_GeneralPolygon2D::CP_GeneralEdge* CP_GeneralPolygon2D::GetEdge(int index) {
	ASSERT(index >= 0 && index < m_edgeArr.size());
	return m_edgeArr[index];
}

void CP_GeneralPolygon2D::AddEdge(CP_GeneralEdge *pEdge) {
	m_edgeArr.push_back(pEdge);
}


void CP_GeneralPolygon2D::AddEdge(CP_MeshVertex3D *start, CP_MeshVertex3D *end) {
	CP_GeneralEdge *pEdge = new CP_GeneralEdge(start, end);
	AddEdge(pEdge);
}

void CP_GeneralPolygon2D::RemoveEdge(int index) {
	ASSERT(index >= 0 && index < m_edgeArr.size());
	std::vector<CP_GeneralPolygon2D::CP_GeneralEdge *>::iterator it = m_edgeArr.begin() + index;
	delete m_edgeArr[index];
	m_edgeArr.erase(it);
}

int	CP_GeneralPolygon2D::GetVertSize() {
	return m_vertArr.size();
}

CP_MeshVertex3D* CP_GeneralPolygon2D::GetVert(int index) {
	ASSERT(index >= 0 && index < GetVertSize());
	return m_vertArr[index];
}

void CP_GeneralPolygon2D::AddVert(CP_MeshVertex3D *pVt) {
	m_vertArr.push_back(pVt);
}

int CP_GeneralPolygon2D::GetEdgeIndex(CP_MeshVertex3D *v1, CP_MeshVertex3D *v2 ) {
	int numOfEdge = GetEdgeSize();
	for (int i = 0; i < numOfEdge; ++i) {
		CP_GeneralEdge *edgei = GetEdge(i);
		if ((edgei->m_pStart == v1) && (edgei->m_pEnd == v2) ||
			(edgei->m_pStart == v2) && (edgei->m_pEnd == v1)) {
				return i;
		}
	}
	return -1;
}