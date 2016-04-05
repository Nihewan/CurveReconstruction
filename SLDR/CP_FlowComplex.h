//////////////////////////////////////////////////////////////////////////
//////////////////////CP_FlowComplex.h////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef FLOW_COMPLEX_H
#define FLOW_COMPLEX_H

#include <algorithm>
#include <map>
#include "CP_Mesh.h"
#include "cp_pointvector.h"
#include "gl/GLU.h"
#include "glut.h"

class CP_Tetrahedron
{
public:
	int m_points[4];
public:
	CP_Tetrahedron(int i,int j,int k,int l);
	~CP_Tetrahedron();

private:

};

class CircumPoint :
	public CP_Point3D
{
public:
	double vol;//所在四面体的体积
	bool flag;
	double distance;//Voronoi vertex和Delaunay tetrahedron顶点间的距离
public:
	explicit CircumPoint (double newx=0.0, double newy=0.0, double newz=0.0, double newvol=0.0);
	void SetMember(double newx, double newy, double newz, double newvol);
	void SetDistance(double d);
	CircumPoint& operator=(const CircumPoint& tmp);
	~CircumPoint(void);
};


class CP_2cell;
class CP_Triganle3D
{
public:
	int m_points[3];
	double m_color[3];
	bool normalsetted;
	int _2cell; //构造2-cell时设置三角面片所属的2-cell
	std::vector<int> m_adjTriangle;//邻接的三角面片
	double minx,maxx,miny,maxy;
	int aa;
public:
	CP_Triganle3D();
	CP_Triganle3D(int p0, int p1, int p2);
	~CP_Triganle3D(void);
	//CP_Vector3D GetNormal();
};

class CircuAndTri
{
public:
	vector<CP_LineSegment3D> m_circulator;//开头的中心三角形为空，表示圈和下一个三角形
	CP_Triganle3D tri;
	//
	vector<CircuAndTri*> m_circuAndTri;//访问时深度优先
public:
	CircuAndTri();
	~CircuAndTri(void);
};

class CurveSegment :public CP_LineSegment3D
{
public:
	int sp,ep;
	int degree,tmpdegree;//-1:curve,1~：边，0度数为0
	vector<int> incident2cell;//the index of incident 2cell 目前编号
	int _triangle;//the index of incident triangle in 2cell
	int m_adj2cell;//the index of adjcent 2cell of the 2cell it belongs
public:
	CurveSegment(int lp,int rp);
	int GetPointIndex(int i) const;
	void ResetDegreee();
};

class CP_2cell
{
public:
	CP_Triganle3D *pTri;
	vector<int> m_triangle;
	vector<CurveSegment> m_boundary;
	vector<int> m_bp;//边界点
	int index;
	int type;//type=1 creator ;type=0,destoryer
	int p_critical;
	double distance;
	bool flag;//flag false消失 true存在
	bool visited;
	vector<int> m_adj2cell;
	vector<int> p3cell;//组成paired 3cell的2cell 的index，因为collapse之后还要使用此信息
	int patch;
	CircuAndTri m_circulator;
public:
	CP_2cell(void);
	~CP_2cell(void);
};

class CP_3cell
{
public:
	vector<int> m_2cells;
	vector<int> cmp3cells;
	vector<int> m_circums;
	vector<CP_Point3D> v;
	double dis3cell;//其实是体积
	double distance;
	double persistence;
	bool flag;
public:
	CP_3cell(void);
	~CP_3cell(void);
};

class CP_Patch
{
public:
	vector<int> m_2cells;
	vector<CurveSegment> m_boundary;
	vector<int> m_adjPatch;
	bool visited;
	int index;//在fc中m_patch vector中的编号
	double r,g,b;
	int color;//颜色编号
	double f;
	bool flag;
public:
	CP_Patch(void);
	~CP_Patch(void);
};

class CP_FlowComplex
{
public:
	unsigned int desN;
	unsigned int _3cellN;
	bool show;
	unsigned int inputPoints;
	unsigned int inputCurves;
	int oripatches;
	double minx,maxx,miny,maxy,minz,maxz;
	vector<CP_Point3D> m_0cells;
	vector<CP_Point3D> m_critical;
	vector<CurveSegment*> m_1cells;//input curve segment
	vector<CP_2cell*> m_2cells;//tricells中三角面片的id
	vector<CP_3cell*> m_3cells;
	vector<CP_Patch *> m_patches;//包括destoryer和最大creator的patch
	vector<CP_Patch *> m_cpatches;//其他creator的patch,现在的编号
	vector<CP_Triganle3D*> delauny2cells;
	vector<CP_Triganle3D*> non_gabriel_triangles;
	vector<CP_Triganle3D*> right_triangle;
	vector<CP_PolyLine3D> m_PolyLine;
	vector<CP_Triganle3D*> tricells;
	vector<CP_Triganle3D*> visitedtri;//中心三角形
	CP_Point3D cp;//中心点
	vector<CP_Point3D> vjoint;
	vector<CircumPoint> m_circums;
	vector<int> topo;
public:
	CP_FlowComplex();
	~CP_FlowComplex();
	void ClearAll();
	void IsBoundBox(const CP_Point3D &p);
	void Set3cellDistance();
	void SetTriangleBound();
	void Gabrielize();
	bool IsGabriel(const CP_Point3D &c,const CP_Point3D &r,double radius_pq);
	void ReverseForProjection(const CP_Point3D & p);
	CP_Point3D ProjectionPoint(const CP_Point3D &p1,const CP_Point3D &p2,const CP_Point3D &p3);
	void SubdivideSegsJointV(const CP_Point3D & vp);
	void Insert2cellInto1cell(const CP_2cell& p2cell);
	CP_Point3D EqualDisPoint(double k,const CP_Point3D &po,const CP_Point3D &pb);
	bool IsSmallAngle(const CP_Point3D &po,const CP_Point3D &pa,const CP_Point3D &pb);
	bool ExistPoint(const vector<CP_Point3D> &v,const CP_Point3D& p);
	int LocatePoint(const CP_Point3D &p);
	int LocateSegment(const vector<CurveSegment*> &curveVec,const CurveSegment& line);//若存在，返回线段的下标,否则返回-1
	int Locate2cell(int _2cell);//用最原始的编号去找现在的位置
	void SetNormals();
	void SpreadTriangle(CP_Triganle3D* tri);
	void Set2cellNormalConsensus();
	void PatchNormalConsensus();
	void Spread2cellTri(int _2cell,CP_Triganle3D* tri);
	void SpreadPatch2cell(int _patch,CP_2cell *p2cell);
	void Spread2cellNormal(const CP_2cell& p2cell);
	void SpreadPatchNormal(const CP_Patch& pPatch);
	void Convert2cellNormal(const CP_2cell& _2cell,CP_Triganle3D *tri);
	void ConvertpatchNormal(CP_Triganle3D* tri);
	void SetAdjTriangle();
	void SetAdjPatch();
	vector<int> GetIncidentTri(const CP_Point3D& v1,const CP_Point3D& v2);
	void SetAdj2cell();
	void Set2cellNormal();
	void SetPatchNormal();
	void SetCreatorAndDestoryer();
	bool IsPointInside3cell(const CircumPoint& p,const CP_3cell& p3cell);
	void Calculate3cellvolume();
	int IsPointZLineIntersectTriangle(const CircumPoint& p,const CP_Triganle3D& tri,vector<CP_Point3D>* vp);
	void Expand2cell(const CP_2cell& p2cell,const vector<CurveSegment*> vb,CP_Patch *pPatch);
	int CheckClosedVoid(vector<CurveSegment*> *vboundary,int i);
	void Reset2cellFlag(int len);
	void Reset2cellVisited();
	void ResetPatchVisited();
	void ResetTriNormalset();
	void CutBranch(vector<CurveSegment*> *vboundary,const CurveSegment& curve);
	CP_Vector3D GetTangent(const CurveSegment &curve) const;
	void SetPatchColor();
	void SpreadPatchColor(CP_Patch* pPatch);

	void SeekCreatorPatch();
	void SeekDestoryerPatch();
	void GetPatchBoundary();

	void DrawPoints();
	void DrawDelaunyTriangles();
	void DrawRightTriangles();
	void DrawNonGabrielTriangles();
	void DrawTriangle(const CP_Triganle3D &tri);
	void DrawTriangleBoundary(const CP_2cell &p2cell);
	void Draw2cell(const CP_2cell &p2cell);
	void Draw2cellBoundary(const CP_2cell &p2cell);
	void DrawPatchBoundary(const CP_Patch &pPatch);
};
extern double dist(const CP_Point3D &x,const CP_Point3D &y);
extern double Area(double a,double b,double c);
extern int ExistLineSeg(const vector<CurveSegment> &lvec,CurveSegment &l);


#endif //FLOW_COMPLEX_H