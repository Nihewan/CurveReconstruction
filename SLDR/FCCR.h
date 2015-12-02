#include "CP_FlowComplex.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Cartesian_converter.h>
#include <CGAL/Cartesian.h> 

typedef CGAL::Exact_predicates_inexact_constructions_kernel IK;
typedef CGAL::Exact_predicates_exact_constructions_kernel EK;

typedef CGAL::Lazy_exact_nt<CGAL::Gmpq> NT; 
typedef CGAL::Cartesian<NT> K; 
typedef CGAL::Delaunay_triangulation_3<EK> Delaunay;
typedef CGAL::Cartesian_converter<IK, EK>	IK_to_EK;
typedef CGAL::Cartesian_converter<EK, IK>   EK_to_IK;
typedef EK::Point_3   Point;
typedef EK::Ray_3	Ray;
typedef EK::Segment_3 Segment;
typedef EK::Triangle_3 Triangle;
typedef EK::Tetrahedron_3 Tetrahedron;
typedef EK::Line_3   Line;
typedef EK::Plane_3 Plane;
typedef EK::Vector_3 Vector;
typedef Delaunay::Facet Facet;
typedef Delaunay::Cell_handle Cell_handle;
typedef Delaunay::Finite_facets_iterator  Facets_iterator;
typedef Delaunay::Edge Edge;
class FCCR
{
public:
	double maxhd;
	CString filename;
	int voids;
	Delaunay T;
	CP_FlowComplex *m_FlowComplex;
	vector<CP_PolyLine3D> *m_VT_PolyLine;
	//queue<Edge,CP_MeshVertex3D,Triangle> q;
	int nn;
public:
	FCCR(void);
	~FCCR(void);

	void ToPolyLine();
	void OnDelaunyTriangulation();
	void ToFlowcomplex();
	void OnCollapse();
	void collapse(vector<CurveSegment*> &v1cell,CurveSegment &curve);
	bool IsPlane(CP_Point3D p0, CP_Point3D p1, CP_Point3D p2, CP_Point3D p3);
	bool obtusetri(Triangle &tri);
	bool sharpBoundTri(Triangle tri);
	double getHausdorffDistance(CP_PolyLine3D &curveA, CP_PolyLine3D &curveB);
	void addrFacet(Facet fit);
	//void addrVoroFace(Edge e,CP_MeshVertex3D &vIntersect,Triangle tri,bool parentDe,int num);//tri-此时的三角形，e此时三角形的边，此三角形的saddle，退化情况时为父三角形saddle，父三角形退化时parentDe为true，num-2-cell的编号
	void addrVoroFace(Edge e,int vIntersect,Triangle tricir,int _2cell);
	void generOBJ();
};

