#pragma once
#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Cartesian_converter.h>
#include <CGAL/Cartesian.h> 
#include "SurfaceOptimization.h"
using namespace std;

typedef CGAL::Exact_predicates_inexact_constructions_kernel IK;
typedef CGAL::Exact_predicates_exact_constructions_kernel EK;

typedef CGAL::Lazy_exact_nt<CGAL::Gmpq> NT; 
typedef CGAL::Cartesian<NT> K; 
typedef CGAL::Delaunay_triangulation_3<K> Delaunay;
typedef CGAL::Cartesian_converter<IK, EK>	IK_to_EK;
typedef CGAL::Cartesian_converter<EK, IK>   EK_to_IK;
typedef K::Point_3   Point;
typedef K::Ray_3	Ray;
typedef K::Segment_3 Segment;
typedef K::Triangle_3 Triangle;
typedef K::Tetrahedron_3 Tetrahedron;
typedef K::Line_3   Line;
typedef K::Plane_3 Plane;
typedef K::Vector_3 Vector;
typedef Delaunay::Facet Facet;
typedef Delaunay::Cell_handle Cell_handle;
typedef Delaunay::Finite_facets_iterator  Facets_iterator;
typedef Delaunay::Finite_cells_iterator Cells_iterator;
typedef Delaunay::Edge Edge;

class CP_FlowComplex;
class CP_PolyLine3D;
class CurveSegment;
class CP_Point3D;
class CP_Triganle3D;
class CircuAndTri;
class FCCR
{
public:
	double maxhd;
	CString filename;
	unsigned int voids;
	double epsilon;
	double feasa;
	Delaunay T;
	CP_FlowComplex *m_FlowComplex;
	SurfaceOptimization surface_optimization;
	std::vector<CP_PolyLine3D> *m_VT_PolyLine;
	bool showIFCResult;//多线程中不能调用view的变量
	bool showOptResult;
	bool IsProcess;
	bool IsOpt;
public:
	FCCR(void);
	~FCCR(void);

	void ReSet();
	void ToPolyLine();
	void OnDelaunyTriangulation();
	void ToFlowcomplex();
	void OnCollapse();
	void OnThicken();
	void collapse(const CurveSegment &curve);
	bool obtusetri(const Triangle &tri);
	double getHausdorffDistance(const CP_PolyLine3D &curveA,const  CP_PolyLine3D &curveB);
	void addrFacet(const Facet &fit);
	void addrVoroFace(const Edge &e,int vIntersect,const Triangle &tricir,int _2cell,CircuAndTri* pTrcirculator);
	void deleteTriangle(vector<CP_Triganle3D*> &non_gabriel_triangles,const Triangle &tri);
	void generOBJ();

	void ImprovedFlowcomplex();
	void ImprovedThicken();
	void ImprovedFindCyclesForAllPatches();
	void ImprovedPolyline();
	void ImprovedPruningAndTopoComplete();
	void ConfirmClassification();
	void SetSymmetricCurveTagTrue();
	bool IsExistSharpChange(CP_PolyLine3D &poly);
};


