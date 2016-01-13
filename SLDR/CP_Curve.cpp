#include "stdafx.h"
#include "CP_Curve.h"
#include "gl/GL.h"
#include "GL/GLU.H" 
#include "CP_Topology.h"
#include "base_geometricoper.h"
#include "topo_geometricoper.h"
#include <algorithm>

/************************************************************************/
/* CP_LineSegment                                                       */
/************************************************************************/

CP_LineSegment3D::CP_LineSegment3D(double x1, double y1, double z1, double x2, double y2, double z2)
	:m_startPt(x1, y1, z1), m_endPt(x2, y2, z2)
{
} 

CP_LineSegment3D::CP_LineSegment3D(const CP_Point3D& s, const CP_Point3D& e)
	:m_startPt(s.m_x, s.m_y, s.m_z), m_endPt(e.m_x, e.m_y, e.m_z)
{
} 

CP_LineSegment3D::CP_LineSegment3D(const CP_LineSegment3D &line)
	:m_startPt(line.m_startPt), m_endPt(line.m_endPt) 
{
}

CP_Point3D CP_LineSegment3D::GetStartingPoint( ) const
{
	return m_startPt;
} 

CP_Point3D CP_LineSegment3D::GetEndingPoint( ) const
{
	return m_endPt;
} 

CP_Point3D CP_LineSegment3D::GetPoint(double t) const
{
	return m_startPt + (m_endPt - m_startPt) * t;
}

CP_Vector3D CP_LineSegment3D::GetTangent(double t) const
{
	CP_Vector3D tangent = m_endPt - m_startPt;
	tangent.Normalize();
	return tangent;
}

double CP_LineSegment3D::GetLength() const {
	return CP_Vector3D(m_endPt - m_startPt).GetLength();
}

double CP_LineSegment3D::GetCurvature(double t) const {
	return 0;
}

bool CP_LineSegment3D::HasPoint(const CP_Point3D &pt, double &outMinDis) const {
	if (CP_Vector3D(pt - m_startPt).GetLength() < TOLER) {
		outMinDis = 0;
		return TRUE;
	}
	if (CP_Vector3D(pt - m_endPt).GetLength() < TOLER){
		outMinDis = 0;
		return TRUE;
	}
	CP_Vector3D dir = m_endPt - m_startPt;
	double Len = dir.GetLength();
	if ( Len < TOLER ){
		outMinDis = ATTACH_CURVE_THRESHOLD;
		return FALSE;
	}
	dir /= Len;
	CP_Vector3D v = pt - m_startPt;
	double d = v * dir;
	if ( d < TOLER || d > Len-TOLER ) {
		outMinDis = ATTACH_CURVE_THRESHOLD;
		return FALSE;
	}
	double d2 = v*v - d*d;
	if (d2 < ATTACH_CURVE_THRESHOLD) {
		outMinDis = 0;
		return TRUE;
	}
	return FALSE;
}

void CP_LineSegment3D::Draw( )
{
	glBegin(GL_LINES);
	glVertex3d(m_startPt.m_x, m_startPt.m_y, m_startPt.m_z);
	glVertex3d(m_endPt.m_x, m_endPt.m_y, m_endPt.m_z);
	glEnd();
} 

CP_LineSegment3D& CP_LineSegment3D::operator = (const CP_Curve3D &val) {
	if (this == &val)
		return *this;
	const CP_LineSegment3D& line = dynamic_cast<const CP_LineSegment3D&>(val);
	this->m_startPt = line.m_startPt;
	this->m_endPt = line.m_endPt;
	return *this;
}

CP_LineSegment3D& CP_LineSegment3D::operator=(const CP_LineSegment3D &line)
{
	if (this == &line)
		return *this;
	this->m_startPt = line.m_startPt;
	this->m_endPt = line.m_endPt;
	return *this;
}
void CP_LineSegment3D::Split2PolyLine(double ratio)
{
	double len = GetLength();
	int numOfBlock = static_cast<int>(len*ratio);
	if (numOfBlock < 2)
		numOfBlock=2;
	for (int i = 0; i <=numOfBlock; i++)
	{
		CP_Point3D pt = GetPoint((double)i / numOfBlock);
		resultSeg.push_back(pt);
	}
}
/************************************************************************/
/* CP_PolyLine                                                          */
/************************************************************************/
void CP_PolyLine3D::Draw()
{
	glBegin(GL_LINE_STRIP);
	for (unsigned int i = 0; i < m_points.size(); i++)
	{
		glVertex3d(m_points[i].m_x, m_points[i].m_y, m_points[i].m_z);
	}

	glEnd();
}

double CP_PolyLine3D::GetLength() const
{
	double len = 0;
	for (unsigned int i = 0; i < m_points.size() - 1;i++)
	{
		CP_Point3D p(m_points[i]);
		len += sqrt(p.m_x*p.m_x + p.m_y*p.m_y + p.m_z*p.m_z);
	}
	return len;
}

/************************************************************************/
/* CP_BSpline                                                           */
/************************************************************************/

CP_Nurbs::CP_Nurbs()
{
	m_normal.m_x = 0;
	m_normal.m_y = 0;
	m_normal.m_z = 1;
}

CP_Nurbs::CP_Nurbs(const CP_Nurbs &nurbs):
m_knotArr(nurbs.m_knotArr), m_weightArr(nurbs.m_weightArr),
m_degree(nurbs.m_degree), m_ctrlNumber(nurbs.m_ctrlNumber)
{
	for (unsigned int i = 0; i < m_ctrlArr.size(); ++i) {
		delete m_ctrlArr[i];
	}
	m_ctrlArr.clear();
	for (int i = 0; i < nurbs.GetCtrlNumber(); ++i) {
		CP_Point3D *pPoint = nurbs.GetCtrlPoint(i);
		CP_Point3D *pCopy = new CP_Point3D(*pPoint);
		AddCtrlPoint(pCopy);
	}
	m_normal = nurbs.m_normal;
}

CP_Nurbs::CP_Nurbs(int number, int degree):
m_ctrlNumber(number), m_degree(degree)
{
	m_normal.m_x = 0;
	m_normal.m_y = 0;
	m_normal.m_z = 1;
}

CP_Nurbs::CP_Nurbs(int number, int degree, vector<CP_Point3D *> &vCtrls, vector<double> &vKnots, vector<double> &vWeights)
	: m_ctrlNumber(number), m_degree(degree), m_ctrlArr(vCtrls), m_knotArr(vKnots), m_weightArr(vWeights)
{
	m_normal.m_x = 0;
	m_normal.m_y = 0;
	m_normal.m_z = 1;
}

CP_Nurbs::CP_Nurbs(int number, int degree, CP_Point3D *ctrls, double *knots, double *weights)
	: m_ctrlNumber(number), m_degree(degree), m_knotArr(knots, knots + number + degree + 1), m_weightArr(weights, weights + number)
{
	for (int i = 0; i < m_ctrlNumber; ++i)
		m_ctrlArr.push_back(new CP_Point3D(ctrls[i]));
}

CP_Nurbs::~CP_Nurbs(void)
{
	for(unsigned int i = 0; i < m_ctrlArr.size(); i++)
		delete m_ctrlArr[i];
	m_ctrlArr.clear();
	m_knotArr.clear();
	m_weightArr.clear();
}

int CP_Nurbs::GetCtrlNumber() const
{
	return m_ctrlNumber;
}

void CP_Nurbs::SetCtrlNumber(int ctrlNumber) {
	m_ctrlNumber = ctrlNumber;
}

int CP_Nurbs::GetDegree() const
{
	return m_degree;
}

int CP_Nurbs::GetOrder() const
{
	return m_degree + 1;
}

void   CP_Nurbs::SetDegree(int degree)
{
	m_degree = degree;
}

CP_Point3D*  CP_Nurbs::GetCtrlPoint(int index) const
{
	return (CP_Point3D *)m_ctrlArr.at(index);
}

int    CP_Nurbs::GetKnotNumber()
{
	return m_ctrlNumber + m_degree + 1;
}

void   CP_Nurbs::AddCtrlPoint(CP_Point3D *pt)
{
	m_ctrlArr.push_back(pt);
}

double CP_Nurbs::GetKnotValue(int index) const
{
	return m_knotArr.at(index);
}

void   CP_Nurbs::AddKnotValue(double knot)
{
	m_knotArr.push_back(knot);
}

int CP_Nurbs::GetKnotMulti(double value)
{
	int i,j,multiple;
	for(i = 0; i < GetKnotNumber(); i++)
	{
		double pardis = m_knotArr[i] - value;
		if(fabs(pardis)<TOLER)
			break;
	}
	if(i == GetKnotNumber())
		return 0;
	multiple = 1;
	for(j = i+1; j < GetKnotNumber(); j++) {
		if(fabs(m_knotArr[j] - value) < TOLER)
			multiple++;
		else
			break;
	}
	return multiple;
}

double CP_Nurbs::GetWeightValue(int index) const
{
	return m_weightArr.at(index);
}

void   CP_Nurbs::AddWeightValue(double weight)
{
	m_weightArr.push_back(weight);
}

int CP_Nurbs::GetIntervalIndex(double value) const
{
	int a, b, c, ret2 = -1;
 	a = m_degree;
 	b = m_ctrlNumber;
	value -= TOLER;
	while (b - a > 1)
	{
		c = (a + b) / 2;
		if (value <= m_knotArr.at(c) - TOLER)
			b = c;
		else
			a = c;
	}
	ret2 = a;
	return ret2;
}

CP_Vector3D CP_Nurbs::ComputDerivative(double value) const
{
	int			i, r, j, m, order;
	double		fz1, fz2, fm;

	double		W[4], w_value, tw_value;
	CP_Vector3D	C[4], pVector, tpVector, tVector;

	order = m_degree + 1;
	j = GetIntervalIndex(value);
	m = j - m_degree;

	for( i = m; i <= j; i++ )
	{
		W[i-m] = m_weightArr.at(i);
		C[i-m] = (*m_ctrlArr.at(i) - ZeroP3D) * m_weightArr.at(i);
	}

	for( r = 1; r <= order - 1; r++ )
	{
		for( i = m; i <= j - r; i++ )
		{
			fz1 = m_knotArr.at(i+order) - value;
			fz2 = value - m_knotArr.at(i+r);
			fm  = m_knotArr.at(i+order) - m_knotArr.at(i+r);
			W[i-m] = (W[i-m]*fz1 + W[i-m+1]*fz2)/fm;
			C[i-m] = (C[i-m]*fz1 + C[i-m+1]*fz2)/fm;
		}
	}
	pVector = C[0];
	w_value = W[0];

	order--;
	for( i = m; i <= j; i++ )
	{
		W[i-m] = m_weightArr.at(i);
		C[i-m] = (*m_ctrlArr.at(i) - ZeroP3D)*m_weightArr.at(i);
	}
	for( i = j; i > m; i-- )
	{
		fm = order / (m_knotArr.at(i+order) - m_knotArr.at(i));
		W[i-m] = (W[i-m] - W[i-m-1])*fm;
		C[i-m] = (C[i-m] - C[i-m-1])*fm;
	}

	for( r = 1; r <= order - 1; r++ )
	{
		for( i = m + 1; i <= j - r; i++ )
		{
			fz1 = m_knotArr.at(i+order) - value;
			fz2 = value - m_knotArr.at(i+r);
			fm  = m_knotArr.at(i+order) - m_knotArr.at(i+r);
			W[i-m] = (W[i-m]*fz1 + W[i-m+1]*fz2)/fm;
			C[i-m] = (C[i-m]*fz1 + C[i-m+1]*fz2)/fm;
		}
	}
	tpVector = C[1];
	tw_value = W[1];

	tVector = (tpVector*w_value - pVector*tw_value)/(w_value*w_value);
	return tVector;
}

CP_Vector3D CP_Nurbs::ComputSecondDerivative(double value) const {
	int order = m_degree + 1;
	double		W[4], tw_value, tw2_value;
	CP_Vector3D	C[4], tpVector, tp2Vector;
	int j = GetIntervalIndex(value);
	int m = j - m_degree;

	// First derivative part
	order--;
	for( int i = m; i <= j; i++ )
	{
		W[i-m] = m_weightArr.at(i);
		C[i-m] = (*m_ctrlArr.at(i) - ZeroP3D)*m_weightArr.at(i);
	}

	// Compute new control point and new weight
	for( int i = j; i > m; i-- )
	{
		double fm = order / (m_knotArr.at(i+order) - m_knotArr.at(i));
		W[i-m] = (W[i-m] - W[i-m-1])*fm;
		C[i-m] = (C[i-m] - C[i-m-1])*fm;
	}

	// De boor
	DeBoor(value, order, W, C, j, m, 1);
	
	tpVector = C[1];
	tw_value = W[1];

	// Second derivative part
	for( int i = m; i <= j; i++ )
	{
		W[i-m] = m_weightArr.at(i);
		C[i-m] = (*m_ctrlArr.at(i) - ZeroP3D)*m_weightArr.at(i);
	}

	// Compute new control point and new weight
	for( int i = j; i > m; i-- ) {
		double fm = order / (m_knotArr.at(i+order) - m_knotArr.at(i));
		W[i-m] = (W[i-m] - W[i-m-1])*fm;
		C[i-m] = (C[i-m] - C[i-m-1])*fm;
	}
	order--;
	for( int i = j; i > m + 1; i-- ) {
		double fm = order / (m_knotArr.at(i+order) - m_knotArr.at(i));
		W[i-m] = (W[i-m] - W[i-m-1])*fm;
		C[i-m] = (C[i-m] - C[i-m-1])*fm;
	}

	// De boor
	DeBoor(value, order, W, C, j, m, 2);

	tp2Vector = C[2];
	tw2_value = W[2];

	if (tw_value == 0)
		return tp2Vector;
	return (tp2Vector*tw_value - tpVector*tw2_value)/(tw_value*tw_value);
}

CP_Point3D CP_Nurbs::GetPoint(double t) const
{
	// assert(t >= m_knotArr[0] && t <= m_knotArr[m_ctrlNumber + m_degree] );
	t = m_knotArr[m_degree] + t * (m_knotArr[m_ctrlNumber] - m_knotArr[m_degree]);

	if (t < m_knotArr[m_degree])
		t = m_knotArr[m_degree];
	if (t > m_knotArr[m_ctrlNumber])
		t = m_knotArr[m_ctrlNumber];

	int multi = 0;
	int index = 0;
	if (t == m_knotArr[0]) {
		multi = m_degree;
		while (index <= m_degree && m_knotArr[++index] == t);
		index -= 1;
	} else if (t == m_knotArr[m_ctrlNumber + m_degree]) {
		multi = m_degree;
		index = m_ctrlNumber + m_degree - 1;
	} else {
		index = GetUpperBoundIndex(t);
		multi = index - GetLowerBoundIndex(t);
	}

	CP_Point3D pt = DeBoor(t, index, multi);
	return pt;
}

void CP_Nurbs::NormalizeKnotValue() {

	int size = m_knotArr.size();

	double increase = -m_knotArr[0];
	for (int i = 0; i < size; ++i) {
		m_knotArr[i] += increase;
	}

	for (int i = 0; i < size; ++i)
	{
		m_knotArr[i] /= m_knotArr[size-1];
	}
}

int CP_Nurbs::GetUpperBoundIndex(double value) const{
	int l = 0, r = m_degree +m_ctrlNumber;
	int mid = (l+r)/2;
	while (value < m_knotArr[mid] || value >= m_knotArr[mid+1]) {
		if (value < m_knotArr[mid])
			r = mid;
		else
			l = mid;
		mid = (l+r)/2;
	}
	return mid;
}

int CP_Nurbs::GetLowerBoundIndex(double value) const{
	int l = 0, r = m_degree + m_ctrlNumber;
	int mid = (l+r)/2;
	while (value <= m_knotArr[mid] || value > m_knotArr[mid+1])
	{
		if (value <= m_knotArr[mid])
			r = mid;
		else
			l = mid;
		mid = (l+r)/2;
	}
	return mid;
}

CP_Point3D CP_Nurbs::DeBoor(double value, int index, int multi) const {

	double		W[4];
	CP_Vector3D	C[4];

	for (int i = 0; i <= m_degree - multi; ++i) {
		W[i] = m_weightArr[index-m_degree+i];
		C[i] = (*m_ctrlArr[index-m_degree+i] - ZeroP3D) * W[i];
	}

	int j = index - m_degree;
	for (int r = 1; r <= m_degree - multi; ++r)
	{
		for (int i = 0; i <= m_degree - r - multi; ++i)
		{
			double denom = (m_knotArr[j+m_degree+i+1] - m_knotArr[j+i+r]);
			double alpha;
			if (denom < TOLER)
				alpha = 0;
			else
				alpha = (value - m_knotArr[j+i+r]) / denom;
			C[i] = C[i+1]*alpha + C[i]*(1-alpha);
			W[i] = W[i+1]*alpha + W[i]*(1-alpha);
		}
	}
	CP_Vector3D resultVec = C[0] / W[0];
	return CP_Point3D(resultVec.m_x, resultVec.m_y, resultVec.m_z);
}

void CP_Nurbs::DeBoor(double value, int order, double W[], CP_Vector3D C[], int index, int beginPos, int dOrder) const{
	for(int r = 1; r <= order - 1; r++ )
	{
		for(int i = beginPos + dOrder; i <= index - r; i++ )
		{
			double alpha;
			double fm  = m_knotArr.at(i+order) - m_knotArr.at(i+r);
			if (fm < TOLER)
				alpha = 0;
			else
				alpha = (value - m_knotArr.at(i+r))/fm;
			W[i-beginPos] = W[i-beginPos]*(1-alpha) + W[i-beginPos+1]*alpha;
			C[i-beginPos] = C[i-beginPos]*(1-alpha) + C[i-beginPos+1]*alpha;
		}
	}
}

CP_Vector3D CP_Nurbs::GetTangent(double t) const
{
	t = m_knotArr.at(m_degree) + t * ( m_knotArr.at(m_ctrlNumber) - m_knotArr.at(m_degree) );
	CP_Vector3D vect;
	vect = ComputDerivative(t);
	vect.Normalize();
	return vect;
}

double CP_Nurbs::GetCurvature(double t) const {
	t = m_knotArr.at(m_degree) + t * ( m_knotArr.at(m_ctrlNumber) - m_knotArr.at(m_degree) );
	CP_Vector3D tangent;
	tangent = ComputDerivative(t);
	tangent.Normalize();

	CP_Vector3D secTangent = ComputSecondDerivative(t);
	secTangent.Normalize();

	return (tangent ^ secTangent).GetLength();
}

double CP_Nurbs::GetLength() const {
	int numBlock = 10;
	double len = 0;
	for (int i = 0; i < numBlock; ++i) {
		CP_Point3D startPt = GetPoint((double) i / numBlock);
		CP_Point3D endPt = GetPoint((double) (i+1) / numBlock);

		len += (endPt - startPt).GetLength();
	}
	return len;
}

CP_Point3D CP_Nurbs::GetStartingPoint() const {
	return GetPoint(0);
}

CP_Point3D CP_Nurbs::GetEndingPoint() const {
	return GetPoint(1);
}

void CP_Nurbs::SetWeightValue(int index, double weight) {
	m_weightArr[index] = weight;
}

bool CP_Nurbs::HasPoint(const CP_Point3D &pt, double &outMinDis) const
{
	outMinDis = CP_Vector3D(GetPoint(0) - pt).GetLength();
	double minPos = 0;
	int numOfSeg = 4;
	double blockSize = 1;
	double beginPos = 0, curPos;

	// 分割法计算近似距离
	while (blockSize > 0.005/numOfSeg)
	{
		double step = (double)blockSize / numOfSeg;
		for (int i = 0; i <= numOfSeg; ++i) {
			curPos = beginPos + i * step;
			curPos = curPos > 1? 1 : curPos;
			CP_Point3D breakPt = GetPoint(curPos);
			double dis = CP_Vector3D(breakPt - pt).GetLength();
			if (dis < outMinDis) {
				outMinDis = dis;
				minPos = curPos;
			}
		}
		beginPos = minPos - step;
		beginPos = beginPos < 0? 0 : beginPos;

		if (minPos == 0 || minPos == 1)
			blockSize /= numOfSeg;
		else 
			blockSize /= numOfSeg/2;
	}

	if (outMinDis < ATTACH_CURVE_THRESHOLD)
		return true;
	return false;
}

void CP_Nurbs::Split2LineSeg(double disErr, vector<double> &resultSeg, double beginT, double endT) const {
	vector<CP_Point3D> ctrlPts;
	for (int i = 0; i < m_ctrlNumber; ++i)
		ctrlPts.push_back(*m_ctrlArr[i]);
	Split2LineSeg(disErr, resultSeg, ctrlPts, beginT, endT);
	std::sort(resultSeg.begin(), resultSeg.end());
}

void CP_Nurbs::Split2LineSeg(double disErr, vector<double> &resultSeg, vector<CP_Point3D> &ctrlPoints, double beginT, double endT) const {
	
	// Calculate max distance between control point and Line(beginT-endT) 
	CP_Point3D bPt = GetPoint(beginT);
	CP_Point3D ePt = GetPoint(endT);
	double totalLen = (GetEndingPoint() - GetStartingPoint()).GetLength();
	double dx = ePt.m_x - bPt.m_x;
	double dy = ePt.m_y - bPt.m_y;
	double maxDis = 0;
	for (unsigned int i = 1; i < ctrlPoints.size()-1; ++i) {
		CP_Point3D pt = ctrlPoints[i];
		maxDis = max(maxDis, fabs((pt.m_x - bPt.m_x)*dy - (pt.m_y - bPt.m_y)*dx));
	}

	// If max distance is larger than tolerance, split control points to two set.
	if (maxDis > disErr * totalLen * sqrt(dx*dx + dy*dy) ) {
		double mid = (beginT + endT) / 2;
		resultSeg.push_back(mid);
		vector<CP_Point3D> vMidPt(ctrlPoints);
		vector<CP_Point3D> ctrlPt0;
		vector<CP_Point3D> ctrlPt1;
		ctrlPt0.push_back(ctrlPoints[0]);
		ctrlPt1.push_back(ctrlPoints.back());

		for (unsigned int k = 0; k < ctrlPoints.size()-1; ++k) {
			for (unsigned int i = 0; i < ctrlPoints.size()-k-1; ++i) {
				CP_Point3D midPt = vMidPt[i] + vMidPt[i+1];
				midPt.m_x /= 2;
				midPt.m_y /= 2;
				vMidPt[i] = midPt;
			}
			ctrlPt0.push_back(vMidPt[0]);
			ctrlPt1.push_back(vMidPt[ctrlPoints.size()-k-2]);
		}
		Split2LineSeg(disErr, resultSeg, ctrlPt0, beginT, mid);
		Split2LineSeg(disErr, resultSeg, ctrlPt1, endT, mid);
	}
}

void CP_Nurbs::InsertMultiKnot(int multiplicity, double value)
{
	//参考 The NURBS book P151,但书中的程序只能处理插入后重复度不大于次数的情形
	//现增加处理重复度大于次数的情形,(无须计算,只须插入重复顶点)
	if (multiplicity<=0)
		return;
	int number = m_ctrlNumber;
	int order = GetOrder();
	//求插入节点的位置和原有的重复度
	int intervailIndex = GetIntervalIndex(value);
	int r = GetKnotMulti(value);

	//处理插入后重复度大于次数的情况
	int moreMulti = multiplicity + r - (order-1);

	if (moreMulti>0) //多余的重复度
		multiplicity -= moreMulti;
	else
		moreMulti = 0;
	int j,s,L;	

	//提取要处理的控制点
	CP_Point4D* ExchangeCtrs = (CP_Point4D*)_alloca(sizeof(CP_Point4D)*order);
	for (int i = m_ctrlNumber; i < number+multiplicity+moreMulti; ++i) {
		m_ctrlArr.push_back(new CP_Point3D);
		m_weightArr.push_back(0);
	}

	for (j = 0; j<= order-1-r; j++)
		ExchangeCtrs[j].GetValueFrom3D(*m_ctrlArr[intervailIndex-order+1+j], m_weightArr[intervailIndex-order+1+j]);

	//移动原有控制点中无需计算的部分
	int startIndex0 = intervailIndex-r+multiplicity+moreMulti;
	int startIndex1 = intervailIndex-r;
	int num = number-intervailIndex+r;
	for (int i = num-1; i >= 0; --i) {
		*m_ctrlArr[startIndex0 + i]= *m_ctrlArr[startIndex1 + i];
		m_weightArr[startIndex0 + i] = m_weightArr[startIndex1 + i];
	}

	//计算控制点
	L = intervailIndex- order+1; //为了防止下面的循环不执行时,L无初值

	for (s = 1;s<=multiplicity;s++)
	{
		L = intervailIndex- order+1 + s;
		for (j = 0;j<=order-1-r-s;j++)
		{
			double beta = (value - m_knotArr[j+L]) / (m_knotArr[j+intervailIndex+1]-m_knotArr[j+L]);
			ExchangeCtrs[j] = beta * ExchangeCtrs[j+1] + (1-beta) * ExchangeCtrs[j]; 
		}
		ExchangeCtrs[0].SetPoint3DValue(*m_ctrlArr[L], m_weightArr[L]);
		ExchangeCtrs[order-1-s-r].SetPoint3DValue(*m_ctrlArr[intervailIndex+multiplicity-s-r+moreMulti], m_weightArr[intervailIndex+multiplicity-s-r+moreMulti]);
	}
	for (j = L+1;j<intervailIndex-r;j++)
		ExchangeCtrs[j-L].SetPoint3DValue(*m_ctrlArr[j], m_weightArr[j]);

	//当重复度大于次数时,复制重复顶点
	for ( ;j<intervailIndex-r+moreMulti;j++)
	{
		*m_ctrlArr[j] = *m_ctrlArr[intervailIndex-r+moreMulti];
		m_weightArr[j] = m_weightArr[intervailIndex-r+moreMulti];
	}

	// 插节点,先确定插入位置
	int inspos = intervailIndex - r;
	while (value > m_knotArr[inspos])
		inspos++;

	vector<double>::iterator iter = m_knotArr.begin() + inspos;
	m_knotArr.insert(iter, multiplicity + moreMulti, value);

	m_ctrlNumber += (multiplicity+moreMulti);
}

void CP_Nurbs::Draw() {
	int numOfBlock = 20;

	for (int i = 0; i < numOfBlock; ++i) {
	 	CP_Point3D startPt = GetPoint((double) i / numOfBlock);
	 	CP_Point3D endPt = GetPoint((double) (i+1) / numOfBlock);
	 	glBegin(GL_LINES);
	 		glVertex3d(startPt.m_x, startPt.m_y, startPt.m_z);
	 		glVertex3d(endPt.m_x, endPt.m_y, endPt.m_z);
	 	glEnd();
	}

/*
	vector<double> segs;
	Split2LineSeg(CURVED_FACE_SPLIT_ERR, segs, 0, 1);
	segs.push_back(1);
	segs.insert(segs.begin(), 0);
	glBegin(GL_LINE_STRIP);
	for(unsigned int i = 0; i < segs.size(); ++i)
	{
		CP_Point3D pt = GetPoint(segs[i]);
		glVertex3d(pt.m_x, pt.m_y, pt.m_z);
	}
	glEnd();*/
}

void CP_Nurbs::Split2PolyLine(double ratio)
{
	int len = static_cast<int>(GetLength());
	int numOfBlock = static_cast<int>(len*ratio);
	if (numOfBlock < 2)
		numOfBlock = 2;
	for (int i = 0; i <= numOfBlock; i++)
	{
		CP_Point3D pt = GetPoint((double)i / numOfBlock);
		resultSeg.push_back(pt);
	}
}
CP_Nurbs& CP_Nurbs::operator = (const CP_Curve3D &val) {
	if (this == &val)
		return *this;

	const CP_Nurbs& spline = dynamic_cast<const CP_Nurbs&>(val);
	CP_Nurbs tmp(spline); 
	
	m_knotArr = tmp.m_knotArr;
	m_weightArr = tmp.m_weightArr;
	m_degree = tmp.m_degree;
	m_ctrlNumber = tmp.m_ctrlNumber;

	for (unsigned int i = 0; i < m_ctrlArr.size(); ++i) {
		delete m_ctrlArr[i];
	}
	m_ctrlArr.clear();

	for (int i = 0; i < tmp.GetCtrlNumber(); ++i) {
		CP_Point3D *pPoint = tmp.GetCtrlPoint(i);
		CP_Point3D *pCopy = new CP_Point3D(*pPoint);
		AddCtrlPoint(pCopy);
	}
	m_normal = tmp.m_normal;
	return *this;
}

/************************************************************************/
/* CP_Arc                                                               */
/************************************************************************/

CP_Arc::CP_Arc(void)
{
}

CP_Arc::CP_Arc(CP_Point3D center, CP_Vector3D xaxis, double major, double minor, double stangle, double edangle)
{
	m_major = major;
	m_minor = minor;
	if ( stangle > PI2-TOLER )
	{
		if ( stangle < PI2+TOLER )
			m_stangle = 0.0;
		else
			m_stangle = stangle - PI2;
		m_edangle = edangle - PI2;
	}
	else
	{
		m_stangle = stangle;
		m_edangle = edangle;
	}

	m_ucs.SetUcs(center, xaxis, BaseZV3D);
	m_ratio = DERAULT_ARC_RATIO;
}

CP_Arc::CP_Arc(CP_Point3D center, CP_Vector3D xaxis, CP_Vector3D yaxis, double major, double minor, double stangle, double edangle)
{
	m_major = major;
	m_minor = minor;
	if (stangle > PI2 - TOLER)
	{
		if (stangle < PI2 + TOLER)
			m_stangle = 0.0;
		else
			m_stangle = stangle - PI2;
		m_edangle = edangle - PI2;
	}
	else
	{
		m_stangle = stangle;
		m_edangle = edangle;
	}

	m_ucs.SetUcs(center, xaxis, yaxis, BaseZV3D);
	m_ratio = DERAULT_ARC_RATIO;
}

CP_Arc::CP_Arc(const CP_Arc &val) 
	: m_major(val.m_major), m_minor(val.m_minor),
	m_stangle(val.m_stangle), m_edangle(val.m_edangle), m_ucs(val.m_ucs), m_ratio(val.m_ratio)
{

}

CP_Point3D CP_Arc::GetCenter()
{
	return m_ucs.m_origin;
}

CP_Point3D CP_Arc::GetPointByAngle(double angle) const  {
	return m_ucs.m_origin + m_ucs.m_axisX*cos(angle)*m_major + m_ucs.m_axisY*sin(angle)*m_minor;
}

void CP_Arc::SetRatio(double ratio) {
	m_ratio = ratio;
}

CP_Point3D  CP_Arc::GetPoint(double t) const
{
	double ang = (1-t) * m_stangle + t * m_edangle;
	return GetPointByAngle(ang);
}
CP_Vector3D CP_Arc::GetTangent(double t) const
{
	double ang = (1-t) * m_stangle + t * m_edangle;
	double cosa = cos( ang );
	double sina = sin( ang );
	
	CP_Vector3D tangent = - m_ucs.m_axisX * m_major * sina + m_ucs.m_axisY * m_minor * cosa;
	tangent.Normalize();
	return tangent;
}

double CP_Arc::GetCurvature(double t) const {
	double ang = (1-t) * m_stangle + t * m_edangle;
	double cosa = cos( ang );
	double sina = sin( ang );

	// Second derivative
	CP_Vector3D secDerivative = - m_ucs.m_axisX * m_major * cosa - m_ucs.m_axisY * m_minor * sina;
	secDerivative.Normalize();
	
	CP_Vector3D tangent = GetTangent(t);			// Tangent already normalized
	double curvature = (tangent ^ secDerivative).GetLength();
	return curvature;
}

double CP_Arc::GetLength()const {
	int numBlock = 10;
	double len = 0;
	for (int i = 0; i < numBlock; ++i) {
		CP_Point3D startPt = GetPoint((double) i / numBlock);
		CP_Point3D endPt = GetPoint((double) (i+1) / numBlock);

		len += (endPt - startPt).GetLength();
	}
	return len;
}

CP_Point3D CP_Arc::GetStartingPoint( ) const {
	return GetPoint(0);
}
CP_Point3D CP_Arc::GetEndingPoint( ) const {
	return GetPoint(1);
}

bool CP_Arc::HasPoint(const CP_Point3D &pt, double &outMinDis) const {
	CP_Point3D centerPt = m_ucs.m_origin;
	CP_Vector3D vCenter2Pt = pt - centerPt;
	double ang = base_geometric::GetACOS(vCenter2Pt * m_ucs.m_axisX / vCenter2Pt.GetLength() / m_ucs.m_axisX.GetLength());
	if ((vCenter2Pt^m_ucs.m_axisX).m_z > 0) ang = PI2 - ang; 
	double step = 0.1;

	// 与圆心的距离
	CP_Point3D ptOnArc = GetPointByAngle(ang);
	double len = CP_Vector3D(pt - ptOnArc).GetLength();

	// 爬山法计算近似距离
	ptOnArc = GetPointByAngle(ang+step);
	double lenPos = CP_Vector3D(pt - ptOnArc).GetLength();
	ptOnArc = GetPointByAngle(ang-step);
	double lenNag = CP_Vector3D(pt - ptOnArc).GetLength();
	if (lenNag < lenPos) step = -step;

	while (abs(step) > 0.001)
	{
		do {
			outMinDis = len;
			ang += step;
			ptOnArc = GetPointByAngle(ang);
			len = CP_Vector3D(pt - ptOnArc).GetLength();
		} while (len < outMinDis);
		step /= 2;
		step = -step;
	}
	
	// CLog::log("Dis = %f", outMinDis);
	if (outMinDis*outMinDis < ATTACH_CURVE_THRESHOLD) {
		CP_Vector3D vStart = GetStartingPoint() - centerPt;
		CP_Vector3D vEnd = GetEndingPoint() - centerPt;
		CP_Vector3D vPt = pt - centerPt;

		if (m_stangle + PI < m_edangle) { 
			ptOnArc = GetPointByAngle(m_stangle + PI);
			CP_Vector3D vPI = ptOnArc - centerPt;
			return (vStart ^ vPt).m_z > 0 && (vPt ^ vPI).m_z > 0
				|| (vPI ^ vPt).m_z > 0 && (vPt ^ vEnd).m_z > 0;
		}
		else 
			return (vStart ^ vPt).m_z > 0 && (vPt ^ vEnd).m_z > 0;
	}
	return false;
}

void CP_Arc::Split2LineSeg(double disErr, vector<double> &resultSeg, double beginT, double endT) const {
	double mid = (beginT + endT)/2;
	CP_Point3D ptMid = GetPoint(mid);

	CP_Point3D curveEnd = GetPoint(1);
	CP_Point3D curveStr = GetPoint(0);
	double len = (curveEnd - curveStr).GetLength();
	CP_Point3D rPt = GetPoint(endT);
	CP_Point3D lPt = GetPoint(beginT);
	double dy = rPt.m_y - lPt.m_y;
	double dx = rPt.m_x - lPt.m_x;
	double dis = (ptMid.m_x - rPt.m_x) * dy - (ptMid.m_y - rPt.m_y) * dx;

	if (dis < disErr * len * sqrt(dx*dx + dy*dy)) {
		return;
	} else {
		resultSeg.push_back(mid);
		Split2LineSeg(disErr, resultSeg, beginT, mid);
		Split2LineSeg(disErr, resultSeg, mid, endT);
	}
	std::sort(resultSeg.begin(), resultSeg.end());
}

void CP_Arc::Draw( ) {
	double delta = 1.0/20;
	glBegin(GL_LINE_STRIP);
	for(double t = 0; t <= 1 + TOLER; t += delta)
	{
		CP_Point3D pt = GetPoint(t);
		glVertex3d(pt.m_x, pt.m_y, pt.m_z);
	}
	glEnd();


	/*vector<double> segs;
	Split2LineSeg(CURVED_FACE_SPLIT_ERR, segs, 0, 1);
	segs.push_back(1);
	segs.insert(segs.begin(), 0);
	glBegin(GL_LINE_STRIP);
	for(unsigned int i = 0; i < segs.size(); ++i)
	{
		CP_Point3D pt = GetPoint(segs[i]);
		glVertex3d(pt.m_x, pt.m_y, pt.m_z);
	}
	glEnd();*/
}

void CP_Arc::Split2PolyLine(double ratio)
{
	int len = static_cast<int>(GetLength());
	int num = static_cast<int>(len*ratio);
	if (num < 2)
		num = 2;
	double delta = 1.0 / num;
	for (int i = 0; i <= num; i++)
	{
		CP_Point3D pt = GetPoint((double)i /num );
		resultSeg.push_back(pt);
	}
}

CP_Arc& CP_Arc::operator = (const CP_Curve3D& val) {
	if (this == &val) {
		return *this;
	}

	const CP_Arc &arc = dynamic_cast<const CP_Arc&>(val);

	m_major = arc.m_major;
	m_minor = arc.m_minor;
	m_stangle = arc.m_stangle;
	m_edangle = arc.m_edangle;
	m_ucs = arc.m_ucs;
	return *this;
}
