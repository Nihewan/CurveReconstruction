#pragma once
#include "CP_TopologyExt.h"
#include <queue>

namespace rec_bodyassembly {
	void	AssembleBody(CP_AssembledBody *pAsmbBody);
	double	CalcConjointVtDepthDif(CP_Face *pRefFace, vector<CP_Vertex *> &vConjointVt);
	void	FindConjointVt(CP_Face *conjointFace, vector<CP_Vertex *> &vConjointVt);
	void	TranslateBody(CP_Body *pBody, double depth);
	pair<CP_Face *, CP_Face *> FindConjointFace(CP_Body *pBody0, CP_Body *pBody1);
	void	BFSAdjustAdjBodyDepth(CP_AssembledBody *pAsmbBody, CP_BodyExt *pFirstBody); // Adjust bodies depth using breadth-first search
	void	FindAdjBody(CP_AssembledBody *pAsmbBody, CP_Body *pRefBody, std::queue<CP_BodyExt *> &qBody);
	bool	AdjustBodyDepth(CP_Body *pRefBody, CP_Body *pInputBody);
	bool	IsSamePseudoFace(CP_FaceExt *pFace0, CP_FaceExt *pFace1);
	void	RecoverOriginalBody(CP_AssembledBody *pAsmbBody);

	// 调整面环方向
	void	AdjustBodyLoopDirect(CP_AssembledBody *pAsmbBody);
}