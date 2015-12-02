#pragma once
#include "CP_Topology.h"
#include "CP_TopologyExt.h"

class CFaceClassification
{
private:
	CP_AssembledBody *m_pAsmbBody;

public:
	void ClassifyFace();					// Classify face as plane or NURBS surface

public:
	CFaceClassification(CP_AssembledBody *pAsmbBody):m_pAsmbBody(pAsmbBody) {};
	~CFaceClassification(void){};

private:
	void ClassifyRecursion(CP_Body *pBody);
	void ClassifyAdjFaceRecursion(const CP_Face *pFace);

	void ClassifyByTransitionEdge(CP_Body *pBody);
	void ClassifyByLineSeg(CP_Body *pBody);
	void ClassifyByProbability(CP_Body *pBody);

	bool ClassifyFaceAsPlane(CP_Face *pFace);
	bool ClassifyAdjFaceAsCurved(const CP_Face *pFace, CP_Face *pAdjFace, const CP_Edge *pEdge);

	void ClassifyPseudoFace();

	void InitEdgeUsedProperty(CP_Body *pBody);
	void SetCurveEdgeOfFaceUsed(const CP_Face *pFace);
	void RemoveEdgeUsedProperty(CP_Body *pBody);

	void InitChildBodySurface();

};

