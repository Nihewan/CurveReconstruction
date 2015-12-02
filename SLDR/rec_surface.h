#pragma once
#include <vector>

class CP_Face;
class CP_Body;
class CP_GeneralPolygon2D;

namespace rec_surface {
	void ReconstructNurbsSurf(CP_Face *pFace);
	void ReconstructCoonsSurf(CP_Face *pFace);
	void CreateMesh(CP_Face *pFace);
	void InitGeneralPoly(CP_GeneralPolygon2D &poly, CP_Face *pFace, std::vector<double> &depth);
}