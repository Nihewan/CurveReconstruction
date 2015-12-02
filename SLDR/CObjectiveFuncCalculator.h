#pragma once
#include "CP_Topology.h"
#include "CP_TopologyExt.h"
#include "IObjectiveFuncCalculator.h"
#include <memory>

struct CLineParal2DInfo
{
	int m_edge0Index;
	int m_edge1Index;
	double m_cosValue2D;
	CLineParal2DInfo(int index0, int index1, double cosVal) 
		: m_edge0Index(index0), m_edge1Index(index1), m_cosValue2D(cosVal){}

	static void InitLineParal2D(CP_Body *pBody, vector<CLineParal2DInfo> &vLineParal);

};


struct CTransitionEdgeInfo
{
	CP_Edge *m_pTransitionEdge;
	CP_Edge *m_pEdgeAtStart0;
	CP_Edge *m_pEdgeAtStart1;
	CP_Edge *m_pEdgeAtEnd0;
	CP_Edge *m_pEdgeAtEnd1;
	CTransitionEdgeInfo(CP_Edge *pTransitionEdge,
		CP_Edge *pEdgeAtStart0,
		CP_Edge *pEdgeAtStart1,
		CP_Edge *pEdgeAtEnd0,
		CP_Edge *pEdgeAtEnd1) 
	{
		m_pTransitionEdge = pTransitionEdge;
		m_pEdgeAtStart0 = pEdgeAtStart0;
		m_pEdgeAtStart1 = pEdgeAtStart1;
		m_pEdgeAtEnd0 = pEdgeAtEnd0;
		m_pEdgeAtEnd1 = pEdgeAtEnd1;
	}
};

struct CFaceSumData
{
	int m_index;
	double m_xSum;
	double m_ySum;
	double m_a1;
	double m_a2;
	double m_b1;
	double m_b2;
	CFaceSumData(int index, double xSum, double ySum, double a1, double a2, double b1, double b2) 
		: m_index(index), m_xSum(xSum), m_ySum(ySum), m_a1(a1), m_a2(a2), m_b1(b1), m_b2(b2)
	{}
};

struct CFaceBestFit
{
	double m_A;
	double m_B;
	double m_C; 
	double m_D;

	static double	BestFit(vector<CP_Point3D *> &vPts, shared_ptr<CFaceSumData> &faceXYSum, CFaceBestFit &face);
	static CFaceSumData* createFaceXYSum(int index, vector<CP_Point3D *> &vPts);
};

class CLineParallelism : public IObjectiveFuncCalculator
{
public:
	CLineParallelism(CP_Body *pBody, double paramLineParal, vector<CLineParal2DInfo> &lineParal2D);
	~CLineParallelism();

private:
	double m_paramLineParal;
	vector<CLineParal2DInfo> &m_lineParal2D;

public:
	virtual double	CalcObjectiveFunc() ;
	virtual void	SetValue(int index, double value){ };

private:
	double	CalcLineParal(CP_Edge *pEdgeI, CP_Edge *pEdgeJ, double cosVal2D) ;
	double	CalcWeight(CP_Edge *pEdgeI, CP_Edge *pEdgeJ);
};


class CFacePlanarity : public IObjectiveFuncCalculator
{
public:
	CFacePlanarity(CP_Body *pBody);
	~CFacePlanarity();

private:
	vector<shared_ptr<CFaceSumData> > m_faceVtXYSums;

public:
	virtual double	CalcObjectiveFunc() ;
	virtual void	SetValue(int index, double value){};
	
private:
	void	InitFaceSum();
};

class CIsometry : public IObjectiveFuncCalculator
{
private:
	vector<CLineParal2DInfo> &m_lineParal2D;
	vector<double> m_edgeLen2D;

public:
	CIsometry(CP_Body *m_pBody, vector<CLineParal2DInfo> &vLineParal);
	~CIsometry();

public:
	virtual double	CalcObjectiveFunc();
	virtual void	SetValue(int index, double value){};
	virtual double	CalcWeight(double r0, double r1);

private:
	void InitEdgeLen2D();
};

class CMSDA : public IObjectiveFuncCalculator
{
public:
	CMSDA(CP_Body *pBody);
	~CMSDA();

public:
	virtual double	CalcObjectiveFunc();
	virtual void	SetValue(int index, double value) {}
};

class CConcurvity : public IObjectiveFuncCalculator
{
public:
	CConcurvity(CP_Body *pBody, int concurveParam);
	~CConcurvity();

private:
	struct ConcurvePair2D
	{
		CP_Curve3D *m_pCurve0;
		double m_param0;
		CP_Curve3D *m_pCurve1;
		double m_param1;
		double m_ang2D;

		ConcurvePair2D(CP_Curve3D *pCurve0, double param0, CP_Curve3D *pCurve1, double param1, double ang2D)
			: m_pCurve0(pCurve0), m_param0(param0), m_pCurve1(pCurve1), m_param1(param1), m_ang2D(ang2D)
		{
		}
	};

private:
	vector<shared_ptr<CConcurvity::ConcurvePair2D> > m_concurvePairs;
	int m_concurveParam;

public:
	virtual double	CalcObjectiveFunc();
	virtual void	SetValue(int index, double value) {}

private:
	void InitConcurvePair2D();
	double CalcConcurvity(CP_Vector3D tangent0, CP_Vector3D tangent1, double ang2D);
};

class CArcRatio : public IObjectiveFuncCalculator 
{
public:
	explicit CArcRatio(CP_Body *pBody);
	~CArcRatio();

public:
	virtual double	CalcObjectiveFunc();
	virtual void	SetValue(int index, double value){};

};

class CTransEdgeNormalCalc: public IObjectiveFuncCalculator
{
public:
	CTransEdgeNormalCalc(CP_Body *pBody, vector<CTransitionEdgeInfo> &vTrans);
	~CTransEdgeNormalCalc();

private:
	vector<CTransitionEdgeInfo> &m_transitionEdgeInfo;
public:
	virtual double	CalcObjectiveFunc();
	virtual void	SetValue(int index, double value){};
private:
	void	InitTransitionEdge();
	double	CalcPara(CP_Vector3D &v0, CP_Vector3D &v1);

};

class CGenFacePerpendicular : public IObjectiveFuncCalculator
{
public:
	explicit CGenFacePerpendicular(CP_Body *pBody);
	~CGenFacePerpendicular();

private: 
	vector<shared_ptr<CFaceSumData> > m_faceVtXYSums;
	vector<vector<double>> m_pWeightsForFace;

public:
	virtual double	CalcObjectiveFunc();
	virtual void	SetValue(int index, double value) {}
	virtual double	CalcWeight(CP_Vector3D n, CP_Plane *pPlane);

private:
	void InitBestFitXYSum();
	void InitWeight();

};

class CCornerOrthogonality : public IObjectiveFuncCalculator
{
private:
	vector<CP_Vertex *> m_vpCandidate;

public:
	explicit CCornerOrthogonality(CP_Body *pBody);
	~CCornerOrthogonality();

public:
	virtual double	CalcObjectiveFunc();
	virtual void	SetValue(int index, double value) {}
	virtual double	CalcWeight(CP_Vertex *pVt);

private:
	void InitCandidate();
	bool IsCornerOrthogonal(vector<CP_Vector3D> &vTangents);
};

class CDepthObjFuncCalculator : public IObjectiveFuncCalculator
{

private:
	vector<pair<IObjectiveFuncCalculator *, int> > m_vObjFuncs;

public:
	virtual double	CalcObjectiveFunc() ;
	virtual void	SetValue(int index, double value) = 0;

public:
	CDepthObjFuncCalculator(CP_Body *pBody, const vector<int> &vWeight);
	~CDepthObjFuncCalculator(void);

public:
	void AddObjFunc(IObjectiveFuncCalculator *pObjFunc, int weight);
};

class CVtDepObjFuncCalculator : public CDepthObjFuncCalculator
{
private:
	vector<CLineParal2DInfo> m_vLineParal2D;

public:
	CVtDepObjFuncCalculator(CP_Body *pBody, const vector<int> &vWeight, 
		int paramLineParal = 100, int paramConcurve = 100);
	~CVtDepObjFuncCalculator();
public:
	virtual void	SetValue(int index, double value);
};

class CGenFaceDepObjFuncCalculator : public CDepthObjFuncCalculator
{
private:
	vector<pair<CP_Face *, double>> m_vpFaces;

public:
	CGenFaceDepObjFuncCalculator(CP_Body *pBody, const vector<int> &vWeight, vector<pair<CP_Face *, double> > &vFaces);
	~CGenFaceDepObjFuncCalculator();

public:
	virtual void	SetValue(int index, double value);

private:
	void RotateAdjCurve(CP_Vertex *pVt, CP_Loop *pLoop, const CP_Point3D &oriVec, const CP_Point3D &dstVec);
	void RotateCurve(CP_Edge *pEdge, const CP_Vector3D &axis, double ang);
};


class CTransitionCurveFuncCalculator : public CDepthObjFuncCalculator
{
private:
	vector<CTransitionEdgeInfo> &m_vTrans;

public:
	CTransitionCurveFuncCalculator(CP_Body *pBody, const vector<int> &vWeight, vector<CTransitionEdgeInfo> &vTrans);
	~CTransitionCurveFuncCalculator();

public:
	static void GetTransitionCurves(CP_Body *pBody, vector<CTransitionEdgeInfo> &);


private:
	virtual void	SetValue(int index, double value);

};
