#pragma once
#include <vector>
using namespace std;

class CP_Arc;
class CP_LineSegment3D;
class CP_Nurbs;
class CP_Point3D;
class CP_Vector3D;
class CP_StraightLine;
class CP_Curve3D;

namespace curve_basic {

	// Nurbs curve degree elevation
	void NurbsDegreeElevation(CP_Nurbs *pNurbs);	
	
	// Nurbs curve degree elevation more than once. 
	// Input : number - number of times of elevation 
	void NurbsMultiDegreeElevation(int number, CP_Nurbs *&curv); 

	// Make 2 NURBS same order
	void Put2NurbsCurvesSameOrder(CP_Nurbs *pCurve0, CP_Nurbs *pCurve1);

	// Make 2 NURBS curves with same knot vector
	void Put2NurbsCurvesCompatible(CP_Nurbs *pCurve0, CP_Nurbs *pCurve1);

	// Make NURBS curves with same knot vector
	void PutNNurbsCurvesCompatible(CP_Nurbs **pCurve, int N); 

	// Convert arc to NURBS. For detail, see ��������������������Ǿ�������B������ chapter 10
	// Input : pSrcArc
	// Output : pDstNurbs
	void Arc2Nurbs(CP_Arc *pArc, CP_Nurbs *pNurbs);

	// Convert arc whose angle less than PI to NURBS. For detail, see ��������������������Ǿ�������B������ chapter 10
	// Input : pSrcArc
	// Output : pDstNurbs
	void ArcLessPI2Nurbs(CP_Arc *pArc, CP_Nurbs *pNurbs) ;

	// Convert arc whose angle is PI to NURBS. For detail, see ��������������������Ǿ�������B������ chapter 10
	// Input : pSrcArc
	// Output : pDstNurbs
	void ArcPI2Nurbs(CP_Arc *pSrcArc, CP_Nurbs *pDstNurbs);

	// Convert line segment to NURBS. 
	// Input : pLine
	// Output : pNurbs
	void LineSeg2Nurbs(CP_LineSegment3D *pLine, CP_Nurbs *pNurbs);

	// Linear interpolation between two NURBS
	void LinearInterpolateNurbs(CP_Nurbs *pNurbs0, CP_Nurbs *pNurbs1, double param, CP_Nurbs *pResult);

	// Scale NURBS
	void NurbsScale(CP_Nurbs *pNurbs, CP_Point3D *pBasePt, double scale);

	// Rotate NURBS
	void NurbsRotate(CP_Nurbs *pNurbs, CP_Point3D *pBasePt, CP_Vector3D &rotateVec);

	// Translate NURBS to make the point on the NURBS to attach the given point.
	// Input : pAttachPt - the point to be attached
	//			pPtOnNurbs - the point on the curve to attach the *pAttchPt
	void NurbsTranslate(CP_Nurbs *pNurbs, CP_Point3D *pAttachPt, CP_Point3D *pPtOnNurbs);

	// isReverse = true : Fix start point, scale and rotate the NURBS to attach its end point to the given point.
	// isReverse = false : Fix end point, scale and rotate the NURBS to attach its start point to the given point.
	void NurbsTransformToAttach(CP_Nurbs *pNurbs, CP_Point3D *pAttachPt, bool isReverse = false);

	// Reverse NURBS
	void AdjustNurbsDirect(CP_Nurbs *pNurbs, CP_Point3D *pStartPt);
	void NurbsReverse(CP_Nurbs *pNurbs);

	void AdjustCurveDirect(CP_Curve3D *pCurve, CP_Point3D *pStartPt);

	void AdjustArcDirect(CP_Arc *pArc, CP_Point3D *pStartPt) ;
	void ArcReverse(CP_Arc *pArc) ;

	void AdjustLinesegDirect(CP_LineSegment3D *pLineseg, CP_Point3D *pStartPt);
	void LinesegReverse(CP_LineSegment3D *pLineseg);

	// Interpolation
	CP_Nurbs* InterpolateNurbsCurve(int count, CP_Point3D* intpoPnt, double *intpoWeight, 
		int order, CP_Vector3D vs, CP_Vector3D ve, double *knotU);

	void BscurvBaseValueNonZero(const CP_Nurbs& curve, double u, double *coef);

	void BscurvBaseValue(const CP_Nurbs& curve, double u, double *coef);

	// ���ڼ���ڵ��������㷨��The Nurbs book��371ҳ�㷨��ͬ, by wangwenke 2005-01-13
	// ԭ���ĺ���������ƽ������������ڵ����������������εĸ���ĩ�������������˵
	// ����ֱ�ӽ�����ֵ������Ӧ�Ľڵ����������ڽ��߷ֲ��ȽϾ��ȵ���������߲�𲻴�
	// ���Ƕ���bug689�����Ľ��޷ֲ��ܲ����ȵ����������Ч�����Ժ���ǰ��
	void CalKnotU(int SubdivNum, int sec_ctrl_num, CP_Point3D **sc_pnt,int order, double* knotU);

	void GetIntpoEndTangV(int num, CP_Point3D *Pnt, CP_Vector3D &sT, CP_Vector3D &eT);

	CP_Point3D GetArcCenterp(CP_Point3D p1, CP_Point3D p2, CP_Point3D p3, double &angle, double &radius);

	// ֱ����ֱ����  (κ������12)
	// Input :
	//		ln1 - line 1
	//		ln2 - line 2
	// Output:
	//		pi - the intersect point
	// return 
	//		-1 when Line is too short
	//		1 when intersect
	//		0 when not intersect
	//		2 when overlap
	int LinesIntersectAt (CP_StraightLine &ln1, CP_StraightLine &ln2, CP_Point3D &pi);

	bool IsIsoparametric(CP_Nurbs *pNurbs0, CP_Nurbs *pNurbs1);

	// Quick test if lines intersect
	bool IsLinesegIntersect(CP_LineSegment3D *line1, CP_LineSegment3D *line2);

	bool IsLinesegOverlap(CP_LineSegment3D *line1, CP_LineSegment3D *line2);

	// Calculate average curvature of curve
	double CalcAverageCurvature(CP_Curve3D *pCurve);
}