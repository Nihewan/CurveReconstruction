#ifndef CP_CURVE_H_
#define CP_CURVE_H_

#include "CP_PointVector.h"
#include "CP_UCS.h"
#include <vector>
using namespace std;


#define TYPE_NAME_CURVE_LINESEGMENT "class CP_LineSegment3D"
#define TYPE_NAME_CURVE_NURBS		"class CP_Nurbs"
#define TYPE_NAME_CURVE_ARC			"class CP_Arc"

#define TYPE_CURVE_LINESEGMENT 0
#define TYPE_CURVE_NURBS 1
#define TYPE_CURVE_ARC 2

class CP_Edge;
class CP_Body;

class CP_Curve3D : public CObject
{
public:
	CP_Curve3D(const CP_Curve3D&) {}

public:
	CP_Curve3D( ) { };
	virtual ~CP_Curve3D( ) { };
	virtual CP_Point3D	GetStartingPoint( ) const=0;
	virtual CP_Point3D	GetEndingPoint( ) const=0;
	virtual CP_Point3D	GetPoint(double t) const =0;
	virtual CP_Vector3D GetTangent(double t) const =0;
	virtual	double		GetCurvature(double t) const =0;
	virtual double		GetLength()const =0;
	virtual bool		HasPoint(const CP_Point3D &pt, double &outMinDis) const=0;
	virtual void		Split2LineSeg(double disErr, vector<double> &resultSeg, double beginT, double endT) const=0;
	virtual void		Draw( )=0;
	virtual CP_Curve3D& operator=(const CP_Curve3D&) = 0;
}; 

class CP_StraightLine : public CP_Curve3D
{
public:
	CP_Point3D   m_pa;           
	CP_Vector3D  m_vn;

private:
	virtual CP_Point3D	GetStartingPoint( ) const { ASSERT(false); return CP_Point3D();};
	virtual CP_Point3D	GetEndingPoint( ) const {ASSERT(false); return CP_Point3D();};
	virtual CP_Point3D	GetPoint(double t) const {ASSERT(false); return CP_Point3D();};
	virtual double		GetLength()const {ASSERT(false); return 0;};
	virtual bool		HasPoint(const CP_Point3D &pt, double &outMinDis) const {ASSERT(false); return false;};
	virtual void		Split2LineSeg(double disErr, vector<double> &resultSeg, double beginT, double endT) const {ASSERT(false);};
	virtual void		Draw( ) {ASSERT(false); };
public:
	CP_StraightLine(CP_Point3D &pa, CP_Vector3D &vn) : m_pa(pa), m_vn(vn) {}
	~CP_StraightLine(){}

	virtual CP_Vector3D GetTangent(double t) const { return m_vn;};
	virtual	double		GetCurvature(double t) const {return 0;};
	virtual CP_Curve3D& operator=(const CP_Curve3D& c) { m_pa == ((CP_StraightLine&)c).m_pa; m_vn = ((CP_StraightLine&)c).m_vn; return *this;};
};

class CP_LineSegment3D : public CP_Curve3D
{
public:
	CP_Point3D m_startPt, m_endPt;
	vector<CP_Point3D> resultSeg;
public:
	CP_LineSegment3D(double x1=0.0, double y1=0.0, double z1=0.0, double x2=1.0, double y2=0.0, double z2=0.0);
	CP_LineSegment3D(const CP_Point3D& s, const CP_Point3D& e);
	CP_LineSegment3D(const CP_LineSegment3D&);
	virtual~CP_LineSegment3D( ) { };
	
	virtual CP_Point3D	GetStartingPoint( ) const;
	virtual CP_Point3D	GetEndingPoint( ) const;
	virtual CP_Point3D	GetPoint(double t) const; 
	virtual CP_Vector3D GetTangent(double t) const;
	virtual double		GetLength() const;
	virtual double		GetCurvature(double t) const;
	virtual bool		HasPoint(const CP_Point3D &pt, double &outMinDis) const;
	virtual void		Split2LineSeg(double disErr, vector<double> &resultSeg, double beginT, double endT) const {};
	virtual void		Draw( );
	virtual void		Split2PolyLine(double ratio);
	virtual CP_LineSegment3D& operator=(const CP_Curve3D&);
	virtual CP_LineSegment3D& operator=(const CP_LineSegment3D&);
}; 


class CP_PolyLine3D{
public:
	vector<CP_Point3D> m_points;
	vector<int> idx;//点在0cells中的编号
	vector<vector<int>> cycle;//组成最小环的边
	bool tag;//标记预期非流形面片的边界曲线
	int s,e;//两端点的在0cells中的下标
	vector<double> len;
	double mincyclelength;//最小环的周长
	vector<int> sharppointv;
public:
	CP_PolyLine3D(void);
	double		GetLength() const;
	double      GetLength(int s,int e) const;
	void		Draw();
	void		ZoomDraw();
};

class CP_Nurbs : public CP_Curve3D
{
public:
	CP_Nurbs(void);
	CP_Nurbs(const CP_Nurbs &nurbs);
	CP_Nurbs(int number, int degree);
	CP_Nurbs(int number, int degree, vector<CP_Point3D *> &vCtrls, vector<double> &vKnots, vector<double> &vWeights);
	CP_Nurbs(int number, int degree, CP_Point3D *ctrls, double *knots, double *weights);
	virtual ~CP_Nurbs(void);

public:
	int m_ctrlNumber;
	int m_degree;                
	vector<CP_Point3D *> m_ctrlArr;			// Memory managed by CP_Nurbs
	vector<double> m_knotArr;				// Knot vector
	vector<double> m_weightArr; 
	vector<CP_Point3D> resultSeg;
	CP_Vector3D m_normal;

public:
	int    GetDegree() const;
	int	   GetOrder() const;
	void   SetDegree(int degree);

	int    GetCtrlNumber() const;
	void   SetCtrlNumber(int ctrlNumber);
	CP_Point3D*  GetCtrlPoint(int index) const;
	void   AddCtrlPoint(CP_Point3D *pt);
	
	int    GetKnotNumber();
	double GetKnotValue(int index) const;
	void   AddKnotValue(double knot);
	int	   GetKnotMulti(double knot);

	double GetWeightValue(int index) const;
	void   AddWeightValue(double weight);
	void   SetWeightValue(int index, double weight);

	int GetIntervalIndex(double value) const;
	int GetUpperBoundIndex(double value) const;
	int GetLowerBoundIndex(double value) const;
	CP_Point3D DeBoor(double value, int index, int multi) const;
	void DeBoor(double value, int order, double W[], CP_Vector3D C[], int index, int beginPos, int dOrde) const;
	void NormalizeKnotValue();
	void InsertMultiKnot(int multiplicity, double value);

	CP_Vector3D ComputDerivative(double value) const;
	CP_Vector3D ComputSecondDerivative(double value) const;

	virtual CP_Point3D	GetStartingPoint( ) const;
	virtual CP_Point3D	GetEndingPoint( ) const;
	virtual CP_Point3D	GetPoint(double t) const;
	virtual CP_Vector3D GetTangent(double t) const;
	virtual double		GetCurvature(double t) const;
	virtual double		GetLength() const;
	virtual bool		HasPoint(const CP_Point3D &pt, double &outMinDis) const;
	virtual void		Split2LineSeg(double disErr, vector<double> &resultSeg, double beginT, double endT) const;
	virtual void		Split2PolyLine(double ratio);
	virtual void		Draw( );
	virtual CP_Nurbs&   operator = (const CP_Curve3D &val);

private:
	void	Split2LineSeg(double disErr, vector<double> &resultSeg, vector<CP_Point3D> &ctrlPoints, double beginT, double endT) const;
};

class CP_Arc : public CP_Curve3D
{
public:
	CP_Arc(void);
	CP_Arc(CP_Point3D center, CP_Vector3D xaxis, double major, double minor, double stangle, double edangle);
	CP_Arc(CP_Point3D center, CP_Vector3D xaxis, CP_Vector3D yaxis, double major, double minor, double stangle, double edangle);
	CP_Arc(const CP_Arc&);
public:
	double m_major, m_minor;
	double m_stangle, m_edangle;
	CP_UCS m_ucs;
	double m_ratio;
	vector<CP_Point3D> resultSeg;
public:
	CP_Point3D			GetCenter();
	CP_Point3D			GetPointByAngle(double angle) const;
	void				SetRatio(double ratio);

	virtual CP_Point3D	GetStartingPoint( ) const;
	virtual CP_Point3D	GetEndingPoint( ) const;
	virtual CP_Point3D	GetPoint(double t) const;
	virtual CP_Vector3D GetTangent(double t) const;
	virtual	double		GetCurvature(double t) const;
	virtual double		GetLength() const;
	virtual bool		HasPoint(const CP_Point3D &pt, double &outMinDis) const;
	virtual void		Split2LineSeg(double disErr, vector<double> &resultSeg, double beginT, double endT) const;
	virtual void		Split2PolyLine(double ratio);
	virtual void		Draw( );
	virtual CP_Arc&		operator=(const CP_Curve3D&);
};
extern double dist(const CP_Point3D &x,const CP_Point3D &y);

#endif