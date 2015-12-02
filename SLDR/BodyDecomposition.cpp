#include "stdafx.h"
#include "BodyDecomposition.h"
#include "CPseudoDecomposition.h"
#include "topo_geometricoper.h"
#include "CCavityDecomposition.h"
#include "CLog.h"

bool CBodyDecomposition::Decompose() {
	CPseudoDecomposition pseudoDec(m_pAsmbBody);
	pseudoDec.Decompose();
	//CCavityDecomposition cavDec(m_pAsmbBody);
	//cavDec.Decompose();

	//设置所有子形体中face的parent
	int bodySize = m_pAsmbBody->GetBodyNumber();
	for(int i = 1; i < bodySize; i++) {
		CP_Body * body = m_pAsmbBody->GetBody(i);
		int faceNum = body->GetFaceNumber();
		for(int j = 0; j < faceNum; j++) {
			CP_FaceExt * face = (CP_FaceExt *)body->GetFace(j);
			if (face->GetParent() == face) {
				CP_LoopExt * loop = (CP_LoopExt *)face->GetLoop(0);
				CP_Loop * pLoopParent = (CP_Loop *)loop->GetParent();
				if (pLoopParent != loop) {
					CP_Face * pFaceParent = pLoopParent->m_pParentFace;
					face->SetParent(pFaceParent);
				}
			}
		}
	}
	return true;
}
