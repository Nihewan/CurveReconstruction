#pragma once
#include "CP_Topology.h"
#include "CP_TopologyExt.h"
#include <set>

class CReconstruction
{
private:
	CP_AssembledBody *m_pAsmbBody;

public:
	CReconstruction(CP_AssembledBody *pAsmbBody):m_pAsmbBody(pAsmbBody){};
	~CReconstruction(void);

public:
	void Reconstruction(const vector<int> &vWeight);

private:
	void VtDepOptimization(CP_Body *pBody, const vector<int> &vWeight);
	void FaceOptimization(CP_Body *pBody);
	void TransEdgeOptimization(CP_Body *pBody);

	void InsertMidVt();
	void RemoveMidVt();
	void GetAdjFaceOfGenFace(CP_Body *pBody, set<CP_Face *> &setFaces);
	void RotateSilhouette(CP_Body *pBody);
	double CalcCenterZ(CP_Face *pFace);

	void ReconstructSurface();
};

