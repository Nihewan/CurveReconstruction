#pragma once
#include <vector>
#include <set>
#include "CP_PointVector.h"
#include "CP_Curve.h"

class CP_MeshTriangle3D;
class CP_MeshVertex3D;
class CP_GeneralPolygon2D;
class CP_Mesh3D
{
public:
	CP_Mesh3D();
	~CP_Mesh3D();

private:
	// Memory managed by CP_Mesh3D
	std::vector<CP_MeshVertex3D *>		m_ptArr;
	std::vector<CP_MeshTriangle3D *>	m_triArr;

public:
	CP_MeshTriangle3D *	GetTriangle(int index);
	void	AddTriangle(CP_MeshTriangle3D *pTri);
	void	AddTriangle(CP_MeshVertex3D* v0, CP_MeshVertex3D* v1, CP_MeshVertex3D* v2);
	int		GetTriangleSize() {return m_triArr.size();}

	CP_MeshVertex3D * GetVert(int index);
	void	AddVert(CP_MeshVertex3D *pt);
	int		GetVertSize() {return m_ptArr.size();}
};

class CP_MeshVertex3D
{
public:
	CP_MeshVertex3D(CP_Point3D &pt);
	~CP_MeshVertex3D();

public:
	CP_Point3D m_point;

public:
	std::vector<CP_MeshTriangle3D*> m_adjTriangle;

public:
	void AddTriangle(CP_MeshTriangle3D *pTri);
	bool ExistTriangle(CP_MeshTriangle3D *t);
	CP_MeshTriangle3D*	GetTriangle(int index) ;

	double&	x();
	double&	y();
	double&	z();

	CP_Vector3D GetVtNormal();
};


class CP_GeneralPolygon2D  {
public:
	~CP_GeneralPolygon2D();

public:
	struct CP_GeneralEdge
	{
		CP_MeshVertex3D *m_pStart;
		CP_MeshVertex3D *m_pEnd;
		CP_GeneralEdge(CP_MeshVertex3D *start, CP_MeshVertex3D *end) : m_pStart(start), m_pEnd(end){};
	};

private:

	std::vector<CP_MeshVertex3D *> m_vertArr; // Memory managed by CP_Mesh3D
	std::vector<CP_GeneralEdge *> m_edgeArr; // Memory managed by CP_GeneralPolygon

public:
	int			GetEdgeSize();
	CP_GeneralEdge*	GetEdge(int index);
	void		AddEdge(CP_GeneralEdge *pEdge);
	void		AddEdge(CP_MeshVertex3D *m_start, CP_MeshVertex3D *m_end);
	void		RemoveEdge(int index);

	int			GetVertSize();
	CP_MeshVertex3D*	GetVert(int index);
	void		AddVert(CP_MeshVertex3D *pVt);

	int			GetEdgeIndex(CP_MeshVertex3D *v1, CP_MeshVertex3D *v2 );
};

// Triangle mesh
class CP_MeshTriangle3D
{
public:
	CP_MeshVertex3D* m_vertice[3];	// Memory managed by CP_Mesh3D
public:
	
public:
	CP_MeshTriangle3D(CP_MeshVertex3D *v0, CP_MeshVertex3D *v1, CP_MeshVertex3D *v2);
	CP_MeshTriangle3D(CP_GeneralPolygon2D::CP_GeneralEdge *, CP_GeneralPolygon2D::CP_GeneralEdge *);
	~CP_MeshTriangle3D(void);

	CP_MeshVertex3D *GetVert(int index);
	CP_Vector3D	GetNormal();

	void DrawEdge();
	void DrawSolid(bool isReverse = false);
};


