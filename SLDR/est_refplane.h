#include "stdafx.h"

#include "CP_TopologyExt.h"

namespace est_refplane {

	void	CalcBodyWeight(CP_Body *pBody, vector<vector<CP_LineSegment3D>> &vRefPlanes);
	void	InitBodyRefPlanes(CP_Body *pBody, vector<vector<CP_LineSegment3D>> &vRefPlanes);
	void	InitRefPlanesByCooSys(CP_Vector3D coorSys[3], vector<vector<CP_LineSegment3D>> &vResultRefPlanes);
	int		FindPlaneParaRefPlane(CP_Loop *pLoop, vector<vector<CP_LineSegment3D>> &vRefPlanes); // Find each loop's best parallel pane and return the parallelism
	int		FindCurvedFaceParaRefPlane(CP_Loop *pLoop, vector<vector<CP_LineSegment3D>> &vRefPlanes);
	pair<int, vector<CP_LineSegment3D>> FindParaRefPlane(vector<CP_LineSegment3D> &vLineSeg, vector<vector<CP_LineSegment3D>> &vRefPlanes); // Find parallel reference plane of line segments.
	void	InitPlaneByVecs(vector<CP_LineSegment3D> &vRefPlane, const CP_Vector3D *pVec0, const CP_Vector3D *pVec1);	// Initialize certain coordinate pane according to its two axes
	CP_Vector3D CalcParaPlaneNormal(vector<CP_LineSegment3D> &matchPlane);;
}