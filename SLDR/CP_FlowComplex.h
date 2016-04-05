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
	double vol;//��������������
	bool flag;
	double distance;//Voronoi vertex��Delaunay tetrahedron�����ľ���
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
	int _2cell; //����2-cellʱ����������Ƭ������2-cell
	std::vector<int> m_adjTriangle;//�ڽӵ�������Ƭ
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
	vector<CP_LineSegment3D> m_circulator;//��ͷ������������Ϊ�գ���ʾȦ����һ��������
	CP_Triganle3D tri;
	//
	vector<CircuAndTri*> m_circuAndTri;//����ʱ�������
public:
	CircuAndTri();
	~CircuAndTri(void);
};

class CurveSegment :public CP_LineSegment3D
{
public:
	int sp,ep;
	int degree,tmpdegree;//-1:curve,1~���ߣ�0����Ϊ0
	vector<int> incident2cell;//the index of incident 2cell Ŀǰ���
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
	vector<int> m_bp;//�߽��
	int index;
	int type;//type=1 creator ;type=0,destoryer
	int p_critical;
	double distance;
	bool flag;//flag false��ʧ true����
	bool visited;
	vector<int> m_adj2cell;
	vector<int> p3cell;//���paired 3cell��2cell ��index����Ϊcollapse֮��Ҫʹ�ô���Ϣ
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
	double dis3cell;//��ʵ�����
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
	int index;//��fc��m_patch vector�еı��
	double r,g,b;
	int color;//��ɫ���
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
	vector<CP_2cell*> m_2cells;//tricells��������Ƭ��id
	vector<CP_3cell*> m_3cells;
	vector<CP_Patch *> m_patches;//����destoryer�����creator��patch
	vector<CP_Patch *> m_cpatches;//����creator��patch,���ڵı��
	vector<CP_Triganle3D*> delauny2cells;
	vector<CP_Triganle3D*> non_gabriel_triangles;
	vector<CP_Triganle3D*> right_triangle;
	vector<CP_PolyLine3D> m_PolyLine;
	vector<CP_Triganle3D*> tricells;
	vector<CP_Triganle3D*> visitedtri;//����������
	CP_Point3D cp;//���ĵ�
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
	int LocateSegment(const vector<CurveSegment*> &curveVec,const CurveSegment& line);//�����ڣ������߶ε��±�,���򷵻�-1
	int Locate2cell(int _2cell);//����ԭʼ�ı��ȥ�����ڵ�λ��
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