#include "stdafx.h"
#include "curve_basic.h"
#include "CP_Curve.h"
#include "base_geometricoper.h"

namespace curve_basic {


	void LineSeg2Nurbs(CP_LineSegment3D *pLine, CP_Nurbs *pNurbs);
	void Arc2Nurbs(CP_Arc *pArc, CP_Nurbs *pNurbs);

	// Input: pEdge
	// Output: pNurbs
	void Curve2Nurbs(CP_Edge *pEdge, CP_Nurbs *pNurbs) {
		switch (pEdge->GetCurveType())
		{
		case TYPE_CURVE_LINESEGMENT:
			LineSeg2Nurbs(dynamic_cast<CP_LineSegment3D*>(pEdge->m_pCurve3D), pNurbs);
			break;
		case TYPE_CURVE_ARC:
			Arc2Nurbs(dynamic_cast<CP_Arc*>(pEdge->m_pCurve3D), pNurbs);
			break;
		default:
			break;
		}
	}

	void LineSeg2Nurbs(CP_LineSegment3D *pLine, CP_Nurbs *pNurbs) {

		pNurbs->AddKnotValue(0);
		pNurbs->AddKnotValue(0);
		pNurbs->AddKnotValue(0);
		pNurbs->AddKnotValue(1);
		pNurbs->AddKnotValue(1);
		pNurbs->AddKnotValue(1);

		pNurbs->AddCtrlPoint(new CP_Point3D(pLine->GetStartingPoint()));
		pNurbs->AddCtrlPoint(new CP_Point3D((pLine->GetStartingPoint() + pLine->GetEndingPoint())/2));
		pNurbs->AddCtrlPoint(new CP_Point3D(pLine->GetEndingPoint()));

		pNurbs->AddWeightValue(1);
		pNurbs->AddWeightValue(1);
		pNurbs->AddWeightValue(1);

		pNurbs->SetCtrlNumber(3);
		pNurbs->SetDegree(2);
	}

#define ARC_PI_TOLER 1e-1

	void Arc2Nurbs(CP_Arc *pArc, CP_Nurbs *pNurbs) {
		// Only deal with arc whose angle less than or equal to PI.
		ASSERT(pArc->m_edangle - pArc->m_stangle - ARC_PI_TOLER < PI);
		if (abs(pArc->m_edangle - pArc->m_stangle - PI) < ARC_PI_TOLER) {
			ArcPI2Nurbs(pArc, pNurbs);
		} else {
			ArcLessPI2Nurbs(pArc, pNurbs);
		}
	}

	void ArcLessPI2Nurbs(CP_Arc *pArc, CP_Nurbs *pNurbs) {
		// Get start point and tangent at start point
		CP_Point3D b0 = pArc->GetStartingPoint();
		CP_Vector3D v0 = pArc->GetTangent(0);
		CP_StraightLine l_b0b1(b0, v0);

		// Get end point and tangent at end point
		CP_Point3D b2 = pArc->GetEndingPoint();
		CP_Vector3D v2 = pArc->GetTangent(1);
		CP_StraightLine l_b1b2(b2, v2);

		CP_Point3D b1;
		curve_basic::LinesIntersectAt(l_b0b1, l_b1b2, b1);

		CP_Point3D p = pArc->GetPoint(0.5);
		CP_StraightLine l_p0(p, v0);
		CP_StraightLine l_p2(p, v2);

		// Get projection of p on line_b0b1 and line_b1b2
		CP_Point3D pOnb1b2;
		CP_Point3D pOnb0b1;
		curve_basic::LinesIntersectAt(l_b0b1, l_p2, pOnb0b1);
		curve_basic::LinesIntersectAt(l_b1b2, l_p0, pOnb1b2);

		// Calculate alpha and beta
		double alpha = (b1-pOnb0b1).GetLength() / (b1-b0).GetLength();
		double beta = (b1-pOnb1b2).GetLength() / (b2-b1).GetLength();

		double tmp = 4 * alpha * beta / pow(1 - alpha - beta, 2);
		double w0 = 1, w2 = 1;
		double w1 = sqrt(w0 * w2 / tmp);

		pNurbs->AddCtrlPoint(new CP_Point3D(b0));
		pNurbs->AddCtrlPoint(new CP_Point3D(b1));
		pNurbs->AddCtrlPoint(new CP_Point3D(b2));

		pNurbs->AddWeightValue(w0);
		pNurbs->AddWeightValue(w1);
		pNurbs->AddWeightValue(w2);

		pNurbs->AddKnotValue(0);
		pNurbs->AddKnotValue(0);
		pNurbs->AddKnotValue(0);
		pNurbs->AddKnotValue(1);
		pNurbs->AddKnotValue(1);
		pNurbs->AddKnotValue(1);

		pNurbs->m_degree = 2;
		pNurbs->m_ctrlNumber = 3;
	}

	
	void ArcPI2Nurbs(CP_Arc *pSrcArc, CP_Nurbs *pDstNurbs) {
		ASSERT(pSrcArc->m_edangle > pSrcArc->m_stangle);
		ASSERT(abs(pSrcArc->m_edangle - pSrcArc->m_stangle - PI) < ARC_PI_TOLER);

		CP_Point3D b0 = pSrcArc->GetStartingPoint();
		CP_Point3D b2 = pSrcArc->GetEndingPoint();
		
		// Need to test the direction of tangent 
		CP_Point3D midPt = pSrcArc->GetPoint(0.5);
		CP_Vector3D testVec = midPt - pSrcArc->GetCenter();
		CP_Vector3D t0 = pSrcArc->GetTangent(0);
		if (t0 * testVec < 0)
			t0 = -t0;

		// Calculate the vector b1
		CP_Vector3D center2Start = b0 - pSrcArc->GetCenter();
		double ang = base_geometric::GetIncludedAngle(t0, center2Start);
		CP_Point3D b1EndPt = pSrcArc->GetPoint(ang / (pSrcArc->m_edangle - pSrcArc->m_stangle));
		CP_Vector3D b1 = b1EndPt - pSrcArc->GetCenter();

		// Calculate ue
		double u = b1 * (b2 - b0);
		double lambda = pow(b1.GetLength(), 2);
		double ue = abs(u) <= TOLER? 1/2.0 : (u - 2 * lambda + sqrt(pow(u, 2) + 4 * pow(lambda, 2))) / 2 / u;
		ASSERT(ue != 1 && ue != 0);

		// Set weight
		pDstNurbs->AddWeightValue(1);
		pDstNurbs->AddWeightValue(1 - ue);
		pDstNurbs->AddWeightValue(ue);
		pDstNurbs->AddWeightValue(1);

		// Set knot vector
		pDstNurbs->AddKnotValue(0);
		pDstNurbs->AddKnotValue(0);
		pDstNurbs->AddKnotValue(0);
		pDstNurbs->AddKnotValue(ue);
		pDstNurbs->AddKnotValue(1);
		pDstNurbs->AddKnotValue(1);
		pDstNurbs->AddKnotValue(1);

		// Set control point
		pDstNurbs->AddCtrlPoint(new CP_Point3D(b0));
		CP_Point3D pt = ((1 - ue) * b0 + b1 * ue) / pDstNurbs->GetWeightValue(1);
		pDstNurbs->AddCtrlPoint(new CP_Point3D(pt));
		pt = (ue * b2 + (b1 * (1 - ue))) / pDstNurbs->GetWeightValue(2);
		pDstNurbs->AddCtrlPoint(new CP_Point3D(pt));
		pDstNurbs->AddCtrlPoint(new CP_Point3D(b2));

		pDstNurbs->m_degree = 2;
		pDstNurbs->m_ctrlNumber = 4;
	}

#undef ARC_PI_TOLER
}