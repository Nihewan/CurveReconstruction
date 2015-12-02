#ifndef CP_SURFACE_H_
#define CP_SURFACE_H_

#include "CP_PointVector.h"
#include "CP_Curve.h"
#include "CP_Mesh.h"

#define TYPE_SURFACE_PLANE 0
#define TYPE_SURFACE_NURBS 1
#define TYPE_SURFACE_COONS 2
#define TYPE_SURFACE_UNDEFINED -1

// CP_Surface3D
class CP_Surface3D: public CObject
{
public:
	CP_Mesh3D  *m_pMesh;

public:
	CP_Surface3D( ) : m_pMesh(NULL) { };
	virtual ~CP_Surface3D( ) { delete m_pMesh; };
	virtual CP_Point3D GetPoint(double u, double v) const=0;
	virtual CP_Vector3D GetNormal(double u, double v) const=0;
	virtual CP_Vector3D GetUnitNormal(double u, double v) const=0;
	virtual double GetPointDepth(const CP_Point3D &pt)const = 0;

	virtual void DrawSolid(bool normalFlag=true)=0; // normalFlag: true(the same normal direction); 
	virtual void DrawMesh( )=0;
}; // End CP_Surface3D

// CP_Plane
class CP_Plane : public CP_Surface3D
{
public:
	CP_Point3D  *m_pKnownPt;	// Memory managed by input 
	CP_Vector3D m_normal;

private:
	virtual CP_Point3D	GetPoint(double u, double v) const;

public:
	// CP_Plane();
	CP_Plane(CP_Point3D *pt, const CP_Vector3D& n);
	CP_Vector3D	GetNormal() {return GetNormal(0,0);}

	virtual ~CP_Plane( ) { };
	virtual CP_Vector3D GetNormal(double u, double v) const;
	virtual CP_Vector3D GetUnitNormal(double u, double v) const;
	virtual double		GetPointDepth(const CP_Point3D &pt) const;

	virtual void DrawSolid(bool normalFlag=true);
	virtual void DrawMesh();
}; // End CP_Plane

// CP_NURBSSurface

class CP_NURBSSurface: public CP_Surface3D
{
private:
	vector<vector<CP_Point3D> > m_ctrlPts; // col for u direction, row for v direction. left up corner is (0, 0)
	vector<vector<double> > m_weight;

	vector<double> m_uKnotVec;
	vector<double> m_vKnotVec;

	int m_uOrder;
	int m_vOrder;

// Constructor and deconstruction
public:
	CP_NURBSSurface(int uOrder, int vOrder, int numCtrlU, int numCtrlV);
	CP_NURBSSurface(int uOrder, int vOrder, int numCtrlU, int numCtrlV, vector<double> &uKnotVec, vector<double> &vKnotVec);
	~CP_NURBSSurface();

public:
	void	Init(int numCtrlU, int numCtrlV);
	void	AddUKnot(double u);
	void	AddVKnot(double v);
	double	GetUKnot(int index) const;
	double	GetVKnot(int index) const;

	int     GetUIntervalIndex(double u) const;
	int     GetVIntervalIndex(double v) const;

	double	GetWeight(int i, int j) const;
	
	int		GetUCtrlNum() const;
	int		GetVCtrlNum() const;
	CP_Point3D GetCtrlPoint(int i, int j) const;
	void	SetCtrlPoint(int i, int j, const CP_Point3D &pt, double weight);

	double**	CalcN(double u, const vector<double> &knot, int n, int order) const;
	double**	CalcND1(double u, const vector<double> &knot, int n, int order) const;
	double*		CalcNu(double u) const;
	double*		CalcNV(double v) const;
	double*		CalcNuD1(double u) const;
	double*		CalcNvD1(double v) const;

	double **	CalcRDU(double u, double v) const;
	double**	CalcRDv(double u, double v) const;
	CP_Vector3D CalcDU(double u, double v) const;
	CP_Vector3D CalcDV(double u, double v) const;

public:
	virtual CP_Point3D	GetPoint(double u, double v) const;
	virtual CP_Vector3D GetNormal(double u, double v) const;
	virtual CP_Vector3D GetUnitNormal(double u, double v) const;
	virtual double		GetPointDepth(const CP_Point3D &pt) const;

	virtual void DrawSolid(bool normalFlag=true); // normalFlag: true(the same normal direction); 
	virtual void DrawMesh( );

	virtual void DrawControlNet();
};

class CP_CoonsSurface : public CP_Surface3D
{
public:
	CP_CoonsSurface();
	~CP_CoonsSurface();

public:
	CP_Point3D *m_pEndPts[4];			// 0: S(0,0); 1: S(1,0); 2: S(0,1); 3: S(1,1)
	CP_Curve3D *m_boundaries[4];		// 0: S(0,v); 1: S(1,v); 2: S(u,0); 3: S(u,1)
	bool       m_sameDirection[4];      // 0: S(0,v); 1: S(1,v); 2: S(u,0); 3: S(u,1)
	
public:
	virtual CP_Point3D	GetPoint(double u, double v) const;
	virtual CP_Vector3D GetNormal(double u, double v) const;
	virtual CP_Vector3D GetUnitNormal(double u, double v) const;
	virtual double		GetPointDepth(const CP_Point3D &pt) const;

	virtual void DrawSolid(bool normalFlag=true); // normalFlag: true(the same normal direction); 
	virtual void DrawMesh( );

	CP_Point3D GetBoundaryPoint(int i, double t) const;
	void	SplitSurface(double fitErr, vector<double> &usegs, vector<double> &vsegs);
	pair<double, double> GetEndPtUV(const CP_Point3D &pt) const ; // Return the end point's (u,v) : (0,0), (0,1), (1,0), (1,1)
};



#endif