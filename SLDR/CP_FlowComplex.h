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
#include "glext.h"
//////////////////////////////////////////////////////////////////////////
/////////////////////////��ͼ��ʽ�洢�����߶˵�//////////////////////////////
//////////////////////////////////////////////////////////////////////////
class EdgeNode         //�߱���
{
public:
	int adjvex;         //�ڽӵ��򣬴洢�ö����Ӧ���±�
	int polyindex;		//���ߵ��±�
	bool visited;
	EdgeNode *next;      //����ָ����һ���ڽӵ�
public:
	EdgeNode(){visited=false;};
	~EdgeNode(){};
};

class VertexNode       //�����ṹ
{
public:
	int data;        //�����򣬴洢�����±�
	int degree;
	bool visited;
	EdgeNode *firstedge;        //�߱�ͷָ��
	vector<pair<int,int> > darts;
	//vector<int> dartofpatch;//�ĸ�����darts
public:
	VertexNode(){degree=0;visited=false;};
	~VertexNode(){};
};

struct compare: binary_function<VertexNode, int,bool>
{
	bool operator()(VertexNode &v, int data) const
	{
		if (v.data== data)
			return true;
		else
			return false;
	}
};

class GraphList
{
public:
	vector<VertexNode> adjList;
	unsigned int numVertexes, numEdges;  //ͼ�е�ǰ�������ͱ���
	//int sb;
public:
	GraphList();
	~GraphList();
	void Reset();
};
//////////////////////////////////////////////////////////////////////////

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
	double area;
	CP_Point3D circumcenter;
	double radius;
	bool flag;
	bool normalsetted;
	int _2cell; //����2-cellʱ����������Ƭ������2-cell
	int _patch;//-1ʱ��ʾ��������
	std::vector<int> m_adjTriangle;//�ڽӵ�������Ƭ
	double minx,maxx,miny,maxy;
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
	//-1:curve,1~���ߣ�0����Ϊ0,
	//tmpdegree 1ȷ��creator��destoryerʱ��cutbrunch 2Ϊ��patch�Ļ�·Ѱ����ͨ����ʱʹ��
	//newdegree�������patch�ڲ�����
	int degree,tmpdegree,newdegree;
	int isBoundary;//������Ǳ߽磬���Ϊ-1;����ǣ����Ϊ���߱��
	vector<int> incident2cell;//the index of incident 2cell Ŀǰ���
	int _triangle;//patch��2cell�ı߽紦��Ӧ��������,����patch�䷨��һ��
	vector<int> incidentpatch;//non-maifold edge��ص�patch���
public:
	CurveSegment(int lp,int rp);
	int GetPointIndex(int i) const;
	void ResetDegreee();
	bool EqualToCurvement(CurveSegment* c);
};

class CP_2cell
{
public:
	CP_Triganle3D *pTri;
	vector<int> m_triangle;
	vector<CurveSegment> m_boundary;
	int index;
	int type;//type=1 creator ;type=0,destoryer
	int p_critical;
	double distance;
	bool flag;//flag false��ʧ true����
	bool visited;
	vector<int> p3cell;//���paired 3cell��2cell ��index����Ϊcollapse֮��Ҫʹ�ô���Ϣ
	int patch;
	CircuAndTri m_circulator;
	vector<int> delaunytri;//��¼����ЩDelauny�����ε���
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
	vector<CurveSegment*> m_boundary;//cllapse
	vector<CurveSegment> boundary;//�ҵ��ڲ��ڽ�������
	vector<int> m_adjPatch;
	vector<int> m_triangle;//�����Ż�ʱ��������Σ���������2cell��Ѱ��
	bool visited;
	int index;//��fc��m_patch vector�еı��
	int delaunay;
	int color;//��ɫ���
	bool flag;//�������
	bool wrong;
	int nonmanifoldedge;
	int merged;
	int pairedp;
	double dihedral;
	vector<GraphList*> forest;//������ͨ����
	vector<vector<int>> cycle;//�����С���ıߣ�����wrong true��patch�����ж��
	vector<vector<int>> polygon;//cycle��Ӧ��polyline
	vector<int> path;
	vector<CP_Vector3D> r;
	vector<vector<int>> patches;//ÿ��patch�������εı�ű�ʾ 
	vector<int> m_bcurve;//�ų�wrong true���׼ȷ�߽�
	int patchsize;//չʾ�ϲ�ǰ������Ƭ��СMerge
	int pridx;//չʾ�ϲ�ʱ�ĵ�������Ƭ
	vector<int> interior_points;//patch�ڲ���
	vector<CP_Vector3D> interior_points_normal;
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
	int inputPoints;
	unsigned int inputCurveSegments;
	int inputCurves;//������
	int oripatches;//�����patch
	int ori2cells;//�����2cells,չʾflow complex
	int respatches;
	int res_triangle_num;
	double minx,maxx,miny,maxy,minz,maxz;
	int perweight,biweight;
	int type;//1-fc;2-ifc
	GraphList graph;
	vector<CP_Point3D> m_0cells;
	vector<CP_Point3D> m_critical;
	vector<CurveSegment*> m_1cells;//input curve segment
	vector<CP_2cell*> m_2cells;//tricells��������Ƭ��id
	vector<CP_3cell*> m_3cells;
	vector<CP_Patch *> m_patches;//����destoryer�����creator��patch
	vector<CP_Patch *> m_cpatches;//����creator��patch,���ڵı��
	vector<CP_Triganle3D*> delauny2cells;
	vector<CP_Triganle3D*> non_gabriel_triangles;
	vector<CP_PolyLine3D> m_PolyLine;
	vector<CP_Triganle3D*> tricells;
	vector<CP_Triganle3D*> visitedtri;//����������
	CP_Point3D cp;//���ĵ�
	vector<int> vjoint;
	vector<CircumPoint> m_circums;
	vector<int> topo;
	int mergededge;
	vector<int> temporaryfalse; 
	map<int,int> delaunyexist;//xΪDelauny��ţ�y����ռ�õ�patch
	vector<vector<int> > overlap_by_delauny;//<x,y>,xΪdelauny��ţ�y���ص���patch���
	vector<vector<int> > curve_incident_patch;
	vector<int> minpatch;
	vector<vector<int>> cycles;
	vector<int> interior_patches;
	vector<int> connectedPatches;//��������Ƭ
	vector<int> vec_curve_degree;
	vector<int> obt;
	int origin;
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
	int LocateTriangle(const vector<CP_Triganle3D*>& triangles,const CP_Triganle3D &tri);
	void SetNormals();
	void SpreadTriangle(CP_Triganle3D* tri);
	void Set2cellNormalConsensus();
	void PatchNormalConsensus();
	void PatchNormalConsensus(int i);
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

	void SeekCreatorPatch(int i);//iΪ��i��3cell
	void SeekDestoryerPatch();
	void GetPatchBoundary(int i);//iΪpatch���
	double GetCyclelength(const vector<int> &cycle);
	double GetTriangleArea(const CP_Triganle3D &tri);
	double GetSumTriangleArea(const vector<int> &vt);
	//��
	void DrawPoints();
	void DrawDelaunyTriangles();
	void DrawNonGabrielTriangles();
	void DrawTriangle(const CP_Triganle3D &tri);
	void DrawTriangleBoundary(const CP_2cell &p2cell);
	void DrawTriangleBoundary(const CP_Patch &p2cell);
	void Draw2cell(const CP_2cell &p2cell);
	void Draw2cellBoundary(const CP_2cell &p2cell);
	void DrawPatch(const CP_Patch &pPatch);
	void DrawPatchBoundary(const CP_Patch &pPatch);
	void DrawPatchBoundary(CP_Patch &pPatch,bool connection,bool cycle,int which,bool RMF);
	void DrawDarts();
	void DrawVoids(int _3cell,int sel2cell,int seltriangle,bool _2cellboundary,bool triboundary,double mTrans);
	void DrawFlowComplex(bool showcreators,int selcreator,bool _2cellboundary,bool triboundary);
	//�Ľ��ķ���
	void SetPatchFlagFalse(int i);
	void SetPatchFlagTrue(int i);
	double Dihedral(const CP_Triganle3D &t1,const CP_Triganle3D &t2);
	void BuildGraphFromCurves(const vector<int>& poly,GraphList &graphall);
	vector<GraphList*> GetConnectedComponents(GraphList& graphall);
	void NoDulplicateDarts(const VertexNode& v,vector<int>& poly2delete);
	void AddTreeWithoutbranch(vector<GraphList*>& resgraph,GraphList* tmpforest);
	void FindShortestCycleForComponent(vector<vector<int> > &cycle,const GraphList& ptree,bool degreeEnable);
	double GetCycleLength(vector<int> &cycle);
	int ConnectToPolyBothEnds(int i);
	void FindCyclesForaCurve(int i);
	bool ContainSubCycle(const vector<int> &newpath);
	void ComputeDelaunyPatchForCycles(CP_Patch &patch);
	vector<int> TriangulatingSinglePolygon(const vector<int>& polygon,int s,int e,CP_Triganle3D *t,vector<vector<int> >& dp,vector<vector<int> >& vt,double& cost);
	void CheckInteriorForPatch(int _patch,const vector<int>& newpatch);
	vector<int> ElimateInteriorPatch(vector<int>& group);
	void ElimateRecursive(vector<int>& group,vector<int>& res,vector<int>& tmp,int &quality_max,int quality);
	void ProcessingOverlappingPatch();
	void ProcessingInteriorPatch();
	void ConstructFromCycle(const vector<int> &cycle,vector<int> &newpatch);
	int IsCycleExistInPatches(const vector<int>& cycle);
	void AddPatchForCycles(const vector<int> &newpatch,const vector<int>& cycle);
	void GenerateCycle(int polyidx);
	int NonmanifoldCurves();
	void TopologyComplete();

	int TopologicalEnable(const CurveSegment &curve,const CP_Patch &patchl,const CP_Patch &patchr);
	double JointNormalAngleOfNeighbourPatch(CurveSegment &curve,CP_Patch &patchl,CP_Patch &patchr);
	double DihedralOfNeighbourPatch(CurveSegment &curve,CP_Patch &patchl,CP_Patch &patchr);
	void MergePatch(CurveSegment &curve,CP_Patch &pl,CP_Patch &pr,int other);
	double ComputeCost(const vector<int> &path);
	double ComputeCycleCost(const vector<int> &path,vector<CP_Vector3D> &r);
	double ComputePathCost(const vector<int> &path);
	CP_Vector3D rotateNormal(const CP_Vector3D &normal, const CP_Vector3D &axis, const double &angle);
};
extern double dist(const CP_Point3D &x,const CP_Point3D &y);
extern double distsquare(const CP_Point3D &x,const CP_Point3D &y);
extern double Area(double a,double b,double c);
extern int ExistLineSeg(const vector<CurveSegment> &lvec,const CurveSegment &l);

#endif //FLOW_COMPLEX_H