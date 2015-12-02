#pragma once

#include "CP_TopologyExt.h"
#include <set>
#include <memory>

class CP_NURBSSurface;

namespace surface_basic {
	void	Init4LineSegPatch(vector<CP_LineSegment3D> &vLineSeg, const CP_Point3D &pt0, const CP_Point3D &pt1, const CP_Point3D &pt2, const CP_Point3D &pt3);
	
	// Assign 4 curves(or line segments) to 4 boundaries of a coons surface
	void	InitCoonsSurface(CP_Face *pFace);	

	// void	RedirectCoonsBoundary(CP_Face *pFace);

	// Coons interpolation of NURBS surface.
	// Interpolate control points and weight.
	// Boundaries must be isoparametric NURBS curves.
	void	NurbsSurfCoonsInterpolation(CP_NURBSSurface *pSurface);

	// Create NURBS surface.
	CP_NURBSSurface* CreateNURBSSurf(CP_Loop *pLoop);

	vector<shared_ptr<CP_Nurbs> > InterpolateNurbsSectionCurves(vector<CP_Nurbs*> vBound);

	shared_ptr<CP_Nurbs> InterpolateSectionCurveAt(CP_Nurbs *pNurbs0, CP_Nurbs *pNurbs1, double param, CP_Curve3D *pBoundary0, CP_Curve3D *pBoundary1) ;

	void GetLoftPnts(CP_Nurbs** pInSecNurbs, int inSectionNum,
		CP_Point3D**& outLoftCtrlPnt, double **&outLoftWeight, int &outLoftCtrlNum, 
		int &outSecCtrlNum, double*& outKnotU, double*& outKnotV, double err = 0, bool bebound = false);

}