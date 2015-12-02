#pragma once
#include "CP_Mesh.h"
#include "cp_pointvector.h"
#include <algorithm>
#include <map>
class CP_Tetrahedron
{
public:
	int m_points[4];
public:
	CP_Tetrahedron(int i,int j,int k,int l);
	~CP_Tetrahedron();

private:

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
	int aa;
public:
	CP_Triganle3D(int p0, int p1, int p2);
	~CP_Triganle3D(void);
	//CP_Vector3D GetNormal();
};


class CurveSegment :public CP_LineSegment3D
{
public:
	int sp,ep;
	int degree,tmpdegree;//-1:curve,1~：边，0度数为0
	vector<int> incident2cell;//the index of incident 2cell 
	int _triangle;//the index of incident triangle in 2cell
	int m_adj2cell;//the index of adjcent 2cell of the 2cell it belongs
public:
	CurveSegment(int lp,int rp);
	int GetPointIndex(int i);
	void ResetDegreee();
};

class CP_2cell
{
public:
	vector<int> m_triangle;
	vector<CurveSegment> m_boundary;
	vector<int> m_bp;//边界点
	int index;
	int type;//type=1 creator ;type=0,destoryer
	int p_critical;
	double distance;
	bool flag;//flag false消失 true存在
	bool normalsetted;
	vector<int> m_adj2cell;
public:
	CP_2cell(void);
	~CP_2cell(void);
};

class CP_FlowComplex
{
public:
	int desN;
	bool show;
	vector<CP_Point3D> m_0cells;
	vector<CurveSegment*> m_1cells;//input curve segment
	vector<CP_2cell*> m_2cells;//tricells中三角面片的id
	vector<CP_Triganle3D*> delauny2cells;
	vector<CP_PolyLine3D> m_PolyLine;
	vector<CP_LineSegment3D> seg;
	vector<CP_Triganle3D*> tricells;
	vector<CP_Triganle3D*> visitedtri;
	vector<CP_Triganle3D*> ctri;//中心三角形
	CP_Point3D cp;//中心点
	vector<CP_Point3D> vjoint;
public:
	CP_FlowComplex();
	~CP_FlowComplex();
	void Gabrielize();
	bool IsGabriel(CP_Point3D &c,CP_Point3D &r,double radius_pq);
	CP_Point3D ProjectionPoint(CP_Point3D &p1,CP_Point3D &p2,CP_Point3D &p3);
	void subdivideSegsJointV(CP_Point3D & vp);
	CP_Point3D equalDisPoint(double k,CP_Point3D &po,CP_Point3D &pb);
	bool IsSmallAngle(CP_Point3D &po,CP_Point3D &pa,CP_Point3D &pb);
	bool ExistPoint(vector<CP_Point3D> &v,CP_Point3D& p);
	int LocatePoint(const CP_Point3D &p);
	int ExistTriangle(vector<CP_Triganle3D*> &v,CP_Triganle3D &t);
	bool noCover(CP_Triganle3D &ltri,CP_Triganle3D &rtri);
	int LocateSegment(vector<CurveSegment*> &curveVec,CurveSegment& line);//若存在，返回线段的下标,否则返回-1
	int Locate2cell(int _2cell);
	void SetNormals();
	void spread(CP_Triganle3D* tri);
	void _2cellNormalConsensus();
	void spread2cellTri(int _2cell,CP_Triganle3D* tri);
	void spread2cellNormal(CP_2cell& p2cell);
	void convert2cellNormal(CP_2cell& _2cell);
	void SetAdjTriangle();
	vector<int> GetIncidentTri(const CP_Point3D& v1,const CP_Point3D& v2);
	void SetAdj2cell();
	void Set2cellNormal();
	void SetCreatorAndDestoryer();
	int CheckClosedVoid(vector<CurveSegment*> &vboundary,int i);
	void Reset2cellFlag(int len);
	void addSegToVec(vector<CurveSegment>& curveVec,CP_LineSegment3D line,CP_Triganle3D* tri);
	void cutBranch(vector<CurveSegment*> &vboundary,CurveSegment& curve);
};
extern double dist(const CP_Point3D &x,const CP_Point3D &y);




