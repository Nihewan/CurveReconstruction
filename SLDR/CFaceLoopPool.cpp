#include "StdAfx.h"
#include "CFaceLoopPool.h"
#include "CP_Topology.h"
#include "CP_TopologyExt.h"
#include "faceident_utils.h"
#include "CLog.h"
#include "CFaceClipping.h"
#include "base_geometricoper.h"
#include "topo_geometricoper.h"

CFaceLoopPool::CFaceLoopPool(CP_Body *pBody)
{
	// Take each half as a single loop
	int size = pBody->GetEdgeNumber();
	for(int i = 0; i < size; i++)
	{
		CP_Half * half0 = pBody->GetEdge(i)->GetHalf(0);
		CP_Half * half1 = pBody->GetEdge(i)->GetHalf(1);

		CP_LoopExt* loop0 = new CP_LoopExt();
		loop0->AddHalf(half0);
		CP_LoopExt* loop1 = new CP_LoopExt();
		loop1->AddHalf(half1);

		half0->m_pLoop = loop0;
		half1->m_pLoop = loop1;

		m_loopArray.Add(loop0);
		m_loopArray.Add(loop1);
	}
}

CFaceLoopPool::~CFaceLoopPool(void)
{
	m_faceLoopArray.RemoveAll();
	m_loopArray.RemoveAll();
}

void CFaceLoopPool::DeleteLoop(CP_LoopExt *loop)
{
	for (int i = 0; i < m_loopArray.GetSize(); ++i) {
		if (loop == (CP_LoopExt *)m_loopArray[i]) {
			delete loop;
			loop = NULL;
			m_loopArray.RemoveAt(i);
			return;
		}
	}
}

void CFaceLoopPool::RemoveLoopFromPool(CP_LoopExt *loop) {
	for (int i = 0; i < m_loopArray.GetSize(); ++i) {
		if (loop == (CP_LoopExt *)m_loopArray[i]) {
			m_loopArray.RemoveAt(i);
			return;
		}
	}
}

void CFaceLoopPool::AddFace2Body(CP_Body *pBody) {
	// if(tempBody == fullBody && fullBody->GetAILineNum() > 0) return BodyPartition();
	// if(pBody == pOriginalBody) return CFaceIdentUtils::AddFace2FullBody();

	int loopSize = m_faceLoopArray.GetSize();
	for(int i = 0; i < loopSize; i++)
	{
		CP_Loop * loop = (CP_Loop *)m_faceLoopArray[i];
		CP_Face * nFace = new CP_FaceExt(pBody);
		nFace->AddLoop(loop);
		pBody->AddFace(nFace);
	}
	m_faceLoopArray.RemoveAll();
}

void CFaceLoopPool::AddLoop2FaceLoop(CP_LoopExt * loop, CP_AssembledBody *pAsmbBody)
{
	loop->m_adjFaceNum = -1;
	m_faceLoopArray.Add(loop);

	CObArray tmpAdjLoopArr;

	// loop的每个邻接环的邻接面数加1
	int halfSize = loop->GetHalfNumber();
	for(int i = 0; i < halfSize; i++) {
		CP_LoopExt* adjLoop = (CP_LoopExt *)loop->GetHalf(i)->m_pTwin->m_pLoop;
		if(adjLoop->m_adjFaceNum < 0) continue;
		int j;
		for (j = 0; j < tmpAdjLoopArr.GetSize(); ++j) {
			if (tmpAdjLoopArr[j] == adjLoop)
				break;
		}
		if(j == tmpAdjLoopArr.GetSize()) tmpAdjLoopArr.Add(adjLoop);
	}

	int loopSize = tmpAdjLoopArr.GetSize();
	for(int i = 0; i < loopSize; i++) {
		CP_LoopExt * adjLoop = (CP_LoopExt *)tmpAdjLoopArr[i];
		adjLoop->m_adjFaceNum++;
	}
	tmpAdjLoopArr.RemoveAll();

	CFaceClipping::ExamineAdjMerg(*this, pAsmbBody);
}

void CFaceLoopPool::ConnectLoop(CP_LoopExt *&loopExtd, CP_LoopExt *&loopNext, CP_AssembledBody *pAsmbBody)
{
	int size = loopNext->GetHalfNumber();
	for(int i = 0; i < size; i++)
	{
		CP_Half * half = loopNext->GetHalf(i);
		loopExtd->AddHalf(half);
		half->m_pLoop = loopExtd;
	}
	DeleteLoop(loopNext);

	if(loopExtd->IsLoopClosed())
	{
		RemoveLoopFromPool(loopExtd);
		AddLoop2FaceLoop(loopExtd, pAsmbBody);
	}
	else loopExtd->CalcLoopAdjFaceNum();
}

void CFaceLoopPool::ConnectLoop(CP_LoopExt *& loopExtd, CP_LoopExt *& loopNext, CP_LoopExt *& loopRest,
								CP_AssembledBody *pAsmbBody)
{
	if(loopRest == NULL) return ConnectLoop(loopExtd,loopNext, pAsmbBody);

	int size = loopNext->GetHalfNumber();
	for (int i = 0; i < size; i++) {
		CP_Half * half = loopNext->GetHalf(i);
		loopExtd->AddHalf(half);
		half->m_pLoop = loopExtd;
	}
	DeleteLoop(loopNext);

	size = loopRest->GetHalfNumber();
	for (int i = 0; i < size; i++) {
		CP_Half * half = loopRest->GetHalf(i);
		loopExtd->AddHalf(half);
		half->m_pLoop = loopExtd;
	}
	DeleteLoop(loopRest);

	if(loopExtd->IsLoopClosed())
	{
		RemoveLoopFromPool(loopExtd);
		AddLoop2FaceLoop(loopExtd, pAsmbBody);
	}
	else loopExtd->CalcLoopAdjFaceNum();
}

int CFaceLoopPool::GetFaceLoopNum() {
	return m_faceLoopArray.GetSize();
}

int CFaceLoopPool::GetLoopNum() {
	return m_loopArray.GetSize();
}

CP_Loop* CFaceLoopPool::GetLoop(int index) {
	if (index < GetLoopNum() && index >= 0)
		return (CP_Loop *)m_loopArray.GetAt(index);
	return NULL;
}

CP_Loop* CFaceLoopPool::GetFaceLoop(int index) {
	if (index < GetFaceLoopNum() && index >= 0)
		return (CP_Loop *)m_faceLoopArray.GetAt(index);
	return NULL;
}

bool CFaceLoopPool::GetExtendLoopsByPriority(CObArray &resultArr) {
	CObArray extdLoopArr;
	CObArray maxLoopArr;
	CObArray numLoopArr;

	int size = m_loopArray.GetSize();
	for(int i = 0; i < size; i++) {
		CP_LoopExt * loop = (CP_LoopExt *)m_loopArray.GetAt(i);
		if(loop->GetHalfNumber() > 1 && loop->m_adjFaceNum > 0) extdLoopArr.Add(loop);
		else if(loop->m_adjFaceNum  > 0) maxLoopArr.Add(loop);
		else if(loop->GetHalfNumber() > 1) numLoopArr.Add(loop);
	}
	if(!extdLoopArr.IsEmpty()) {
		resultArr.Copy(extdLoopArr);
		return true;
	} else if(!maxLoopArr.IsEmpty()) {
		resultArr.Copy(maxLoopArr);
		return true;
	} else if(!numLoopArr.IsEmpty()){
		resultArr.Copy(numLoopArr);
		return true;
	} 
	return false;
}

CP_ParaLoopsGroupsArr * CFaceLoopPool::GetExtendLoopPair(CObArray &loops)
{
	if(loops.IsEmpty()) return NULL;

	//计算所有可扩展Loop0与其下一条Loop1之间的两个匹配值
	CP_ParaLoopsGroupsArr * mathArr = new CP_ParaLoopsGroupsArr();
	CP_ParaLoopsGroupsArr * mergArr1 = new CP_ParaLoopsGroupsArr();
	CP_ParaLoopsGroupsArr * mergArr2 = new CP_ParaLoopsGroupsArr();
	int size = (int)(loops.GetSize());
	for(int i = 0; i < size; i++)
	{
		CP_Loop * loop0 = (CP_Loop *)loops.GetAt(i);
		CP_Half * half0 = loop0->GetLastHalf();
		int size0 = half0->m_pEndVt->GetOutHalfNumber();

		for(int j = 0; j < size0; j++)
		{
			CP_Half * half1 = half0->m_pEndVt->GetOutHalf(j);
			CP_Loop * loop1 = half1->m_pLoop;
			if(topo_geometric::CanNotBeConnected(half0,half1)) continue;

			double cosValue = topo_geometric::GetCosValue2D(loop0,loop1);

			if(loop0->GetFirstHalf()->m_pStartVt == loop1->GetLastHalf()->m_pEndVt) {
				CP_ParallelLoopsGroup * merg = new CP_ParallelLoopsGroup(loop0,loop1,cosValue);
				mergArr1->push_back(merg);
				continue;
			}
			CP_Loop * loop2 = GetRestLoop(loop0,loop1);
			if(loop2 != NULL)
			{
				CP_ParallelLoopsGroup * merg = new CP_ParallelLoopsGroup(loop0,loop1,loop2,cosValue);
				mergArr2->push_back(merg);
				continue;
			}
			CP_ParallelLoopsGroup * math = new CP_ParallelLoopsGroup(loop0,loop1,cosValue);
			mathArr->push_back(math);
		}
	}

	//返回需要结果
	if(!mergArr1->empty())
	{
		DeleteParaLoopsGroupsArr(mathArr);
		DeleteParaLoopsGroupsArr(mergArr2);
		return mergArr1;
	}
	if(!mergArr2->empty())
	{
		DeleteParaLoopsGroupsArr(mathArr);
		DeleteParaLoopsGroupsArr(mergArr1);
		return mergArr2;
	}
	if(!mathArr->empty()) {
		DeleteParaLoopsGroupsArr(mergArr1);
		DeleteParaLoopsGroupsArr(mergArr2);
		return mathArr;
	}
	return NULL;
}

// 得到所有可合并的pairloop
CP_Loop * CFaceLoopPool::GetRestLoop(CP_Loop * loop0, CP_Loop * loop1)
{
	if(loop1->GetHalfNumber() != 1) return NULL;
	CP_Half * half1 = loop1->GetLastHalf();
	int size = half1->m_pEndVt->GetOutHalfNumber();
	for(int i = 0; i < size; i++)
	{
		CP_Half * half2 = half1->m_pEndVt->GetOutHalf(i);
		CP_Loop * loop2 = half2->m_pLoop;
		if(loop2->GetHalfNumber() != 1) continue;
		if(topo_geometric::CanNotBeConnected(half1,half2)) continue;
		if(half2->m_pEndVt == loop0->GetFirstHalf()->m_pStartVt) return loop2;
	}
	return NULL;
}
