#include "stdafx.h"
#include "faceident_utils.h"
#include <algorithm>
#include <numeric>
#include <typeinfo>
#include "CBoundBox2D.h"
#include "CFaceLoopPool.h"
#include "CLog.h"

namespace faceident_utils {



	void AddFace2FullBody(CP_AssembledBody *pAsmbBody, CObArray &faceArray)
	{
		CP_Body *pOriginalBody = pAsmbBody->GetOriginalBody();
		int size = pOriginalBody->GetAILineNumber()/2;
		for (int i = 0; i < size; i++)
		{
			CP_ArtificialLine * line0 = pOriginalBody->GetAILine(2*i);
			CP_ArtificialLine * line1 = pOriginalBody->GetAILine(2*i+1);
			ASSERT(line0->GetStrEdge() != NULL && line0->GetEndEdge() != NULL);
			ASSERT(line1->GetStrEdge() != NULL && line1->GetEndEdge() != NULL);

			CP_Half * half0 = GetHalfInSameLoop(line0->GetStrEdge(),line1->GetStrEdge());
			CP_Half * half1 = GetHalfInSameLoop(line0->GetEndEdge(),line1->GetEndEdge());
			if(half0 == NULL)
			{
				half0 = GetHalfInSameLoop(line0->GetStrEdge(),line1->GetEndEdge());
				half1 = GetHalfInSameLoop(line0->GetEndEdge(),line1->GetStrEdge());
			}
			ASSERT(half0 != NULL && half1 != NULL);

			CP_Loop * loop0 = half0->m_pLoop;
			CP_Loop * loop1 = half1->m_pLoop;

			CBoundBox2D bBox0;
			bBox0.InitBoundBox(loop0);
			CBoundBox2D bBox1;
			bBox1.InitBoundBox(loop1);

			CP_Loop * outerLoop = NULL;
			CP_Loop * innerLoop = NULL;
			if(bBox0.m_leftPos < bBox1.m_leftPos)
			{
				outerLoop = loop0;
				innerLoop = loop1;
			}
			else
			{
				outerLoop = loop1;
				innerLoop = loop0;
			}
			//reverse
			CP_Vector3D vec0 = *half0->m_pStartVt->m_pPoint - *half0->m_pEndVt->m_pPoint;
			CP_Vector3D vec1 = *half1->m_pStartVt->m_pPoint - *half1->m_pEndVt->m_pPoint;
			double cosvalue = vec0 * vec1;
			if(cosvalue > 0) ReverseLoop(innerLoop);
			//new face
			CP_Face * nFace = outerLoop->m_pParentFace;
			if(nFace == NULL)
			{
				nFace = new CP_FaceExt(pOriginalBody);
				pOriginalBody->AddFace(nFace);
				nFace->AddLoop(outerLoop);
			}
			nFace->AddLoop(innerLoop);
		}
		pOriginalBody->RemoveAllAILine();

		//生成剩余所有单环的Loop
		int faceSize = faceArray.GetSize();
		for(int i = 0; i < faceSize; i++) {
			CP_Loop * loop = (CP_Loop *)faceArray.GetAt(i);
			if(loop->m_pParentFace != NULL) continue;
			CP_Face * nFace = new CP_FaceExt(pOriginalBody);
			nFace->AddLoop(loop);
			pOriginalBody->AddFace(nFace);
		}

		//设置所有子形体中face的parent
		int bodySize = pAsmbBody->GetBodyNumber();
		for(int i = 1; i < bodySize; i++) {
			CP_Body * body = pAsmbBody->GetBody(i);
			int faceNum = body->GetFaceNumber();
			for(int j = 0; j < faceNum; j++) {
				CP_FaceExt * face = (CP_FaceExt *)body->GetFace(j);
				CP_LoopExt * loop = (CP_LoopExt *)face->GetLoop(0);
				CP_Loop * pLoopParent = (CP_Loop *)loop->GetParent();
				CP_Face * pFaceParent = pLoopParent->m_pParentFace;
				face->SetParent(pFaceParent);
			}
		}
	}

	// 根据AILine，检查当前两条Edge中edge0处的同一个Loop的half
	CP_Half * GetHalfInSameLoop(CP_Edge * edge0, CP_Edge * edge1)
	{
		CP_Half * half0[2];
		half0[0] = edge0->GetHalf(0);
		half0[1] = edge0->GetHalf(1);
		CP_Half * half1[2];
		half1[0] = edge1->GetHalf(0);
		half1[1] = edge1->GetHalf(1);

		for(int i = 0; i < 2; i++) {
			CP_Half * pHalf0 = half0[i];
			for(int j = 0; j < 2; j++) {
				CP_Half * pHalf1 = half1[j];
				if(pHalf0->m_pLoop == pHalf1->m_pLoop) return pHalf0;
			}
		}
		return NULL;
	}

	// 调整面环顺序
	void ReverseLoop(CP_Loop * cutLoop)
	{

		std::vector<CP_Loop *> * loopArr = new std::vector<CP_Loop *>();
		loopArr->push_back(cutLoop);
		unsigned int index = 0;

		//压入完整形体中所有需要调整面环顺序的loop
		while(index < loopArr->size())
		{
			CP_LoopExt * loop = (CP_LoopExt *)loopArr->at(index);
			int halfSize = loop->GetHalfNumber();
			for(int i = 0; i < halfSize; i++)
			{
				CP_Half * half = loop->GetHalf(i);
				CP_LoopExt * adjLoop = (CP_LoopExt *)half->m_pTwin->m_pLoop;
				if(adjLoop->m_cutFlag == -1)
				{
					adjLoop->m_cutFlag = 0;
					loopArr->push_back(adjLoop);
				}
			}
			loop->m_cutFlag = 1;
			index++;
		}

		//调整面环顺序
		int loopSize = loopArr->size();
		std::vector<CP_Half *> * halfArr = new std::vector<CP_Half *>();
		for(int i = 0; i < loopSize; i++)
		{
			CP_LoopExt * loop = (CP_LoopExt *)loopArr->at(i);
			halfArr->clear();
			int halfSize = loop->GetHalfNumber();
			for(int i = halfSize-1; i >= 0; i--)
			{
				CP_Half * half = loop->GetHalf(i);
				halfArr->push_back(half->m_pTwin);
			}
			loop->RemoveAllHalf();
			for(int i = 0; i < halfSize; i++)
			{
				CP_Half * half = halfArr->at(i);
				loop->AddHalf(half);
				half->m_pLoop = loop;
			}
			loop->m_cutFlag = -1;
		}
		delete loopArr;
		delete halfArr;
	}

}