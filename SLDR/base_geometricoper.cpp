#include "stdafx.h"
#include "base_geometricoper.h"
#include "CP_Topology.h"
#include <algorithm>
#include "CP_TopologyExt.h"
#include <numeric>

namespace base_geometric {

	double GetACOS(double value)
	{
		if(value - TOLER <= -1.0)
			return PI;
		if(value + TOLER >= 1.0)
			return 0.0;
		if(fabs(value) < TOLER)
			return PI_2;
		return acos(value);
	}

	double GetASIN(double value)
	{
		if(value - TOLER <= -1.0)
			return PI*1.5;
		if(value + TOLER >= 1.0)
			return PI_2;
		if(fabs(value) < TOLER)
			return 0;
		return asin(value);
	}


	bool IsSamePoint(const CP_Point3D *p1, const CP_Point3D *p2) {
		CP_Vector3D vec(*p1 - *p2);
		double len = vec.GetLength();
		return vec.GetLength() < SAME_POINT_THRESHOLD;
	}

	bool HasSameProjectionPt(const CP_Point3D *p1, const CP_Point3D *p2, double threshold) {
		CP_Vector3D vec(p1->m_x - p2->m_x, p1->m_y - p2->m_y, 0);
		double len = vec.GetLength();
		return vec.GetLength() < threshold;
	}

	bool IsVec2DInsideVlVR2DArea( CP_Vector3D &vMid, CP_Vector3D &vL, CP_Vector3D &vR )
	{
		vMid.m_z = 0;
		vL.m_z = 0;
		vR.m_z = 0;
		return (vR ^ vMid).m_z > 0 && (vMid ^ vL).m_z > 0;
	}


	double GetIncludedAngle(const CP_Vector3D& u, const CP_Vector3D &v) {
		double cosValue = u * v / u.GetLength() / v.GetLength();
		return base_geometric::GetACOS(cosValue);
	}

	double GetMinIncludedAngle(const CP_Vector3D& u, const CP_Vector3D &v) {
		double angle = GetIncludedAngle(u, v);
		return angle > PI_2? PI - angle : angle;
	}

	double GetVectorZ(CP_Vector3D vecZUnknown, CP_Vector3D verticalVec) {
		return -(verticalVec.m_x * vecZUnknown.m_x + verticalVec.m_y * vecZUnknown.m_y) / verticalVec.m_z;
	}

	void GetVSumScaleParam(CP_Vector3D vSum, CP_Vector3D v0, CP_Vector3D v1, 
		double &resultScaleV0, double &resultScaleV1) {
		resultScaleV0 = (vSum.m_x*v1.m_y - v1.m_x*vSum.m_y) /
			(v0.m_x*v1.m_y - v1.m_x*v0.m_y);
		resultScaleV1 = (vSum.m_x*v0.m_y - v0.m_x*vSum.m_y) / 
			(v1.m_x*v0.m_y - v0.m_x*v1.m_y);
	}

	double GetCurveLen2D(CP_Curve3D *pCurve) {
		int numBlock = 10;
		double len = 0;
		for (int i = 0; i < numBlock; ++i) {
			CP_Point3D startPt = pCurve->GetPoint((double) i / numBlock);
			CP_Point3D endPt = pCurve->GetPoint((double) (i+1) / numBlock);
			startPt.m_z = 0;
			endPt.m_z = 0;

			len += (endPt - startPt).GetLength();
		}
		return len;
	}

	void Scale(CP_Point3D *pBasePt, CP_Point3D *pPt, double scale) {
		ASSERT(scale >= 0);
		CP_Vector3D v = *pPt - *pBasePt;
		v *= scale;
		*pPt = *pBasePt + v;
	}

	void Rotate(CP_Point3D *pBasePt, CP_Point3D *pPt, double ang, CP_Vector3D axis) {
		*pPt -= *pBasePt;
		Rotate(axis, pPt, ang);
		*pPt += *pBasePt;
	}

	// Rotate by angle in right hand direction
	void Rotate(CP_Vector3D axis, CP_Point3D *pPt, double ang) {
		axis.Normalize();
		double cosa = cos(ang);
		double sina = sin(ang);
		double vers = 1 - cos(ang);
		CP_Point3D tmpPt;
		double kx = axis.m_x, ky = axis.m_y, kz = axis.m_z;
		double px = pPt->m_x, py = pPt->m_y, pz = pPt->m_z;
		pPt->m_x = (kx * kx * vers + cosa) * px
			+ (ky * kx * vers - kz * sina) * py
			+ (kz * kx * vers + ky * sina) * pz;
		pPt->m_y = (kx * ky * vers + kz * sina) * px
			+ (ky * ky * vers + cosa) * py
			+ (kz * ky * vers - kx * sina) * pz;
		pPt->m_z = (kx * ky * vers - ky * sina) * px
			+ (ky * kz * vers + kx * sina) * py
			+ (kz * kz * vers + cosa) * pz;
	}

	
	
}