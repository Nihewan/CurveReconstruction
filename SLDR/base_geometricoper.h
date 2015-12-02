#pragma once
#include <math.h>
#include "CConstants.h"
#include "CP_Topology.h"
#include "CP_PointVector.h"

namespace base_geometric {
	double	GetACOS(double value);
	double	GetASIN(double value);
	bool	IsSamePoint(const CP_Point3D *p1, const CP_Point3D *p2);
	bool	HasSameProjectionPt(const CP_Point3D *p1, const CP_Point3D *p2, double threshold); // True for two points having same x and y coordinate value.
	bool	IsVec2DInsideVlVR2DArea(CP_Vector3D &vMid, CP_Vector3D &vL, CP_Vector3D &vR); // True for vMid inside the vR-vMid-vL area
	double	GetIncludedAngle(const CP_Vector3D& u, const CP_Vector3D &v);
	double	GetMinIncludedAngle(const CP_Vector3D& u, const CP_Vector3D &v); // Calculate the smaller angle of two vector
	double  GetVectorZ(CP_Vector3D vecZUnknown, CP_Vector3D verticalVec);  // Return depth of v
	void	GetVSumScaleParam(CP_Vector3D vSum, CP_Vector3D v0, CP_Vector3D v1, double &resultScaleV0, double &resultScaleV1);
	double	GetCurveLen2D(CP_Curve3D *pCurve);
	void	Scale(CP_Point3D *pBasePt, CP_Point3D *pPt, double scale);
	void	Rotate(CP_Point3D *pBasePt, CP_Point3D *pPt, double ang, CP_Vector3D axis);
	void	Rotate(CP_Vector3D axis, CP_Point3D *pPt, double ang);
}