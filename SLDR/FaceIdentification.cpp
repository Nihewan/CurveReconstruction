#include "StdAfx.h"
#include "FaceIdentification.h"
#include "CFaceLoopPool.h"
#include "faceident_utils.h"
#include "CP_TopologyExt.h"
#include <algorithm>
#include "CFaceClipping.h"
#include "CLog.h"
#include "CFaceCombination.h"
#include "topo_geometricoper.h"
#include "CTimer.h"

CFaceIdentification::CFaceIdentification(CP_AssembledBody *pAsmbBody): m_pAsmbBody(pAsmbBody), m_pFLPool(NULL)
{
	m_pCurrentBody = pAsmbBody->GetBody(0);
}


CFaceIdentification::~CFaceIdentification(void)
{
	if (m_pFLPool != NULL)
	{
		delete m_pFLPool;
		m_pFLPool = NULL;
	}
}

bool CFaceIdentification::FaceIdentify() {
	CTimer myTimer;
	myTimer.Start();
	int faceSize = m_pCurrentBody->GetFaceNumber();
	while(faceSize == 0)
	{
		SingleBodyFaceIdentify();
		if(m_pFLPool->GetFaceLoopNum() != 0) 
			m_pFLPool->AddFace2Body(m_pCurrentBody);
		m_pCurrentBody = m_pAsmbBody->GetBody(m_pAsmbBody->GetBodyNumber()-1);
		faceSize = m_pCurrentBody->GetFaceNumber();
	}
	faceSize = m_pAsmbBody->GetOriginalBody()->GetFaceNumber();
	if(faceSize == 0) 
		CFaceCombination::CombineFaces(m_pAsmbBody);

	CLog::log("Face identification Run time = %f ms", myTimer.End());

	return true;
}

bool CFaceIdentification::SingleBodyFaceIdentify() {
	if (m_pFLPool != NULL) {
		delete m_pFLPool;
		m_pFLPool = NULL;
	}
	m_pFLPool = new CFaceLoopPool(m_pCurrentBody);

	CP_Vertex *triVert = GetMaxTriVert();
	if (triVert != NULL) {
		CP_Loop * loop0 = triVert->GetOutHalf(0)->m_pTwin->m_pLoop;
		CP_Loop * loop1 = triVert->GetOutHalf(1)->m_pLoop;
		m_pFLPool->ConnectLoop((CP_LoopExt *&)loop0, (CP_LoopExt *&)loop1, m_pAsmbBody);
	}
	else {
		CP_Half *pHalf1 = NULL, *pHalf2 = NULL, *pHalf3 = NULL;
		FindTriLoop(pHalf1, pHalf2, pHalf3);
		CP_Loop *loop0 = pHalf1->m_pLoop;
		CP_Loop *loop1 = pHalf2->m_pLoop;
		CP_Loop *loop2 = pHalf3->m_pLoop;
		m_pFLPool->ConnectLoop((CP_LoopExt *&)loop0, (CP_LoopExt *&)loop1,
			(CP_LoopExt *&)loop2, m_pAsmbBody);
	}

	TopologyDeduce();

	return true;
}

// 算法2.2：拓扑推导算法
void CFaceIdentification::TopologyDeduce()
{
	int size = (int)m_pFLPool->GetLoopNum();
	if(size == 0) return;

	int i = 0;
	for(; i < size; i++)
	{
		CP_Loop * loop0 = m_pFLPool->GetLoop(i);
		CP_Half * half0 = loop0->GetLastHalf();
		int adjSize = half0->m_pEndVt->GetOutHalfNumber();

		CP_Loop * loop1 = NULL;
		if(adjSize == 2)
		{
			CP_Half * half1 = half0->m_pEndVt->GetOutHalf(0);
			if(half1->m_pTwin == half0) half1 = half0->m_pEndVt->GetOutHalf(1);
			loop1 = half1->m_pLoop;
		}
		else
		{
			int connectNum = 0;
			for(int j = 0; j < adjSize; j++)
			{
				CP_Half * half1 = half0->m_pEndVt->GetOutHalf(j);
				if(topo_geometric::CanNotBeConnected(half0,half1)) continue;
				if(connectNum == 0) loop1 = half1->m_pLoop;
				connectNum++;
				if(connectNum > 1) break;
			}
			if(connectNum != 1) continue;
		}
		m_pFLPool->ConnectLoop((CP_LoopExt *&)loop0, (CP_LoopExt *&)loop1, m_pAsmbBody);
		break;
	}
	if(i < size) TopologyDeduce();
	else ExtendLoopByPriority();	
}

// 算法2.4：链条扩展算法
void CFaceIdentification::ExtendLoopByPriority()
{
	CObArray extdLoopArr;
	m_pFLPool->GetExtendLoopsByPriority(extdLoopArr);
	
	if (extdLoopArr.GetSize() == 0) {
		// Do partition
		CFaceClipping::BodyPartition(&(m_pFLPool->m_faceLoopArray), m_pAsmbBody);
		CFaceClipping::BodyRestCopy(&(m_pFLPool->m_loopArray), m_pAsmbBody);

		return;
	}

	// 搜索得到所有扩展对的类型：Merg1,Thrd2和Math3
	CP_ParaLoopsGroupsArr * plgArr = m_pFLPool->GetExtendLoopPair(extdLoopArr);
	extdLoopArr.RemoveAll();
	if(plgArr == NULL) 
		return TopologyDeduce();
	

	CP_ParallelLoopsGroup * bestParaGroup = *(std::max_element(plgArr->begin(), plgArr->end(), CP_ParallelLoopsGroup::CompareTo));
	m_pFLPool->ConnectLoop((CP_LoopExt *&)bestParaGroup->loopExtd,
		(CP_LoopExt *&)bestParaGroup->loopNext, (CP_LoopExt *&)bestParaGroup->loopRest, m_pAsmbBody);
	DeleteParaLoopsGroupsArr(plgArr);

	TopologyDeduce();

}

// Find the vertex which has the largest number of 3 degree neighbor vertex
CP_Vertex * CFaceIdentification::GetMaxTriVert()
{
	int size = m_pCurrentBody->GetVertexNumber();
	int degnum = -1;
	CP_Vertex* triVert = NULL;
	for(int i = 0; i < size; i++)
	{
		CP_Vertex * vert = m_pCurrentBody->GetVertex(i);
		if(vert->GetOutHalfNumber() != 3) continue;
		int num = 0;
		for(int j = 0; j < 3; j++) {
			CP_Half *pHalf = vert->GetOutHalf(j);
			if(pHalf->m_pEndVt->GetOutHalfNumber() == 3) num++;
		}
		if(num == 3) 
			return vert;
		if(degnum < num) {
			degnum = num;
			triVert = vert;
		}
	}
	return triVert;
}

// Find the loop which has 3 edges
bool CFaceIdentification::FindTriLoop(CP_Half *&pOutHalf0, CP_Half *&pOutHalf1, CP_Half *&pOutHalf2)
{
	int vertSize = m_pCurrentBody->GetVertexNumber();
	for(int i = 0; i < vertSize; i++) {
		CP_Vertex * pVert = m_pCurrentBody->GetVertex(i);
		int adjSize = pVert->GetOutHalfNumber();
		for(int j = 0; j < adjSize; j++) {
			pOutHalf0 = pVert->GetOutHalf(j);
			for(int k = j+1; k < adjSize; k++) {
				pOutHalf2 = pVert->GetOutHalf(k)->m_pTwin;

				// Check 3rd edge
				int endsize = pOutHalf0->m_pEndVt->GetOutHalfNumber();
				for(int t = 0; t < endsize; t++) {
					CP_Half * half = pOutHalf0->m_pEndVt->GetOutHalf(t);
					if(half != pOutHalf0 && pOutHalf2->m_pStartVt == half->m_pEndVt) {
						pOutHalf1 = half;
						return true;
					}
				}
			}
		}
	}
	return false;
}



