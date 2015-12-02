#pragma once
#include "CP_Topology.h"

namespace rec_curve {	
	void RecBodyEdgeNormal(CP_Body *pBody);
	void ReconstructBodyEdge(CP_Body *pBody); // Reconstruct edges' normal and end points and control points etc that anything needed to reconstruct a edge.
	
	void ReconstructBsplineCtrlPts(CP_Edge *pBSplineEdge); // Should reconstruct the normal of b-spline firstly, and than reconstruct control points.
	void ReconstructArcUCS(CP_Edge *pArdEdge); // Should reconstruct the normal of arc firstly, and than reconstruct the arc's coordinate system.
	void ReconstructLineSegEndPts(CP_Edge *pLineSegEdge);

}