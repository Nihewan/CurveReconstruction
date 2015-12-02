#include "stdafx.h"
#include "CFaceClipping.h"
#include "CP_Topology.h"
#include "CP_TopologyExt.h"
#include "CFaceLoopPool.h"
#include "CObjectFactory.h"
#include <vector>
#include <typeinfo.h>
#include "CCopyPool.h"
#include "CLog.h"
#include "topo_geometricoper.h"
using namespace std;


CFaceClipping::CFaceClipping(void)
{
}


CFaceClipping::~CFaceClipping(void)
{
}

// 检查是否有分离形体存在的操作
void CFaceClipping::ExamineAdjMerg(CFaceLoopPool &faceLoopPool, CP_AssembledBody *pAsmbBody)
{
	// CLog::log("Start [ExamineAdjMerg]");
	int tmpLoopSize = faceLoopPool.GetLoopNum();
	if(tmpLoopSize < 3) {
		// CLog::log("End [ExamineAdjMerg] size < 3");
		return;
	}
	int tmpFaceSize = faceLoopPool.GetFaceLoopNum();
	if(tmpFaceSize < 3) {
		// CLog::log("End [ExamineAdjMerg] size < 3");
		return;
	}

	//搜索所有已成环loop的邻接loop
	vector<CP_Loop *> tempAdjArr;
	for(int i = 0; i < tmpFaceSize; i++)
	{
		CP_Loop * loop = faceLoopPool.GetFaceLoop(i);
		int halfSize = loop->GetHalfNumber();
		for(int j = 0; j < halfSize; j++)
		{
			CP_LoopExt * twinLoop = (CP_LoopExt *)loop->GetHalf(j)->m_pTwin->m_pLoop;
			if(twinLoop->m_adjFaceNum > 0)
			{
				vector<CP_Loop *>::iterator iter = std::find(tempAdjArr.begin(),tempAdjArr.end(),twinLoop);
				if(iter == tempAdjArr.end()) tempAdjArr.push_back(twinLoop);
			}
		}
	}
	int loopSize = tempAdjArr.size();
	if(loopSize != 2)
	{
		tempAdjArr.clear();
		// CLog::log("End [ExamineAdjMerg] size != 2");
		return;
	}

	// Test circle
	/*
	if (tempAdjArr[0]->GetHalfNumber() + tempAdjArr[1]->GetHalfNumber() == 2) {
		tempAdjArr.clear();
		CLog::log("End [ExamineAdjMerg] half size == 2");
		return;
	}*/

	//检查是否构成merg
	CP_LoopExt * loop0 = (CP_LoopExt *)tempAdjArr.at(0);
	CP_LoopExt * loop1 = (CP_LoopExt *)tempAdjArr.at(1);
	if(!BeAdjMerg(&(faceLoopPool.m_loopArray), loop0,loop1)){
		//CLog::log("End [ExamineAdjMerg] false");
		return;
	}

	tempAdjArr.clear();

	// topo_geometric::OutputLoop(loop0);
	// topo_geometric::OutputLoop(loop1);
	faceLoopPool.ConnectLoop(loop0, loop1, pAsmbBody);
	faceLoopPool.RemoveLoopFromPool(loop0);
	faceLoopPool.RemoveLoopFromPool(loop1);

	BodyPartition(&(faceLoopPool.m_faceLoopArray), pAsmbBody);
	BodyRestCopy(&(faceLoopPool.m_loopArray), pAsmbBody);

	//CLog::log("End [ExamineAdjMerg]");

	/*if(tempBody == fullBody) return;
	//delete tempBody
	mObject->RemoveBody(tempBody);
	// tempBody->DeleteAllItems();
	delete tempBody;
	tempBody = NULL;*/
}

// 检查两个loop是否符合分离条件
bool CFaceClipping::BeAdjMerg(CObArray *pLoopArr, CP_Loop * loop0, CP_Loop * loop1)
{
	//CLog::log("Start [BeAdjMerg]");
	if(loop0->GetLastHalf()->m_pEndVt != loop1->GetFirstHalf()->m_pStartVt) return false;
	if(loop0->GetFirstHalf()->m_pStartVt != loop1->GetLastHalf()->m_pEndVt) return false;
	//检查loop0的尾部，和loop1的头部
	CP_Half * half0 = loop0->GetLastHalf();
	CP_Half * half1 = loop1->GetFirstHalf();
	while(half0->m_pTwin == half1)
	{
		CP_Loop * nLoop0 = new CP_LoopExt();
		nLoop0->AddHalf(half0);
		half0->m_pLoop = nLoop0;
		CP_Loop * nLoop1 = new CP_LoopExt();
		nLoop1->AddHalf(half1);
		half1->m_pLoop = nLoop1;
		pLoopArr->Add(nLoop0);
		pLoopArr->Add(nLoop1);
		loop0->RemoveHalf(loop0->GetHalfNumber()-1);
		loop1->RemoveHalf(0);
		half0 = loop0->GetLastHalf();
		half1 = loop1->GetFirstHalf();
	}

	//检查loop0的头部，和loop1的尾部
	half0 = loop0->GetFirstHalf();
	half1 = loop1->GetLastHalf();
	while(half0->m_pTwin == half1)
	{
		CP_Loop * nLoop0 = new CP_LoopExt();
		nLoop0->AddHalf(half0);
		half0->m_pLoop = nLoop0;
		CP_Loop * nLoop1 = new CP_LoopExt();
		nLoop1->AddHalf(half1);
		half1->m_pLoop = nLoop1;
		pLoopArr->Add(nLoop0);
		pLoopArr->Add(nLoop1);
		loop0->RemoveHalf(0);
		loop1->RemoveHalf(loop1->GetHalfNumber()-1);
		half0 = loop0->GetFirstHalf();
		half1 = loop1->GetLastHalf();
	}

	//CLog::log("End [BeAdjMerg]");

	if(loop0->GetHalfNumber() != 1 && loop1->GetHalfNumber() != 1) return false;
	return true;
}

// 根据pFaceArr分离当前形体
void CFaceClipping::BodyPartition(CObArray *pFaceArr, CP_AssembledBody *pAsmbBody)
{
	CLog::log("Start [BodyPartition]");
	CP_Body * newBody = new CP_BodyExt();
	int loopSize = pFaceArr->GetSize();

	CVertexCopyPool vtPool;
	CEdgeCopyPool edgePool;
	for(int i = 0; i < loopSize; i++)
	{
		CP_Loop * loop = (CP_Loop *)pFaceArr->GetAt(i);
		
		// New loop & face
		CP_Loop * nLoop = new CP_LoopExt();
		CP_FaceExt * nFace = new CP_FaceExt(newBody);

		nFace->AddLoop(nLoop);
		newBody->AddFace(nFace);
		
		// New half & vertex
		int halfSize = loop->GetHalfNumber();
		for(int j = 0; j < halfSize; j++)
		{
			CP_Half * half = loop->GetHalf(j);

			CP_VertexExt *nStrVert = vtPool.CopyVertex2Pool(half->m_pStartVt);
			CP_VertexExt * nEndVert = vtPool.CopyVertex2Pool(half->m_pEndVt);
			CP_Edge *pEdge = edgePool.CopyEdge2Pool(half->m_pEdge, nStrVert, nEndVert);

			CP_HalfExt *nHalf = (CP_HalfExt *)
				(pEdge->GetHalf(0)->m_pStartVt == nStrVert ? pEdge->GetHalf(0) : pEdge->GetHalf(1));
			nHalf->SetParent(half);
			nHalf->m_pLoop = nLoop;
			nLoop->AddHalf(nHalf);
			
			half->m_pLoop = NULL;
		}

		// Delete loop
		delete loop;
		loop = NULL;
	}
	pFaceArr->RemoveAll();
	vtPool.AddVertices2Body(newBody);
	edgePool.AddEdges2Body(newBody);
	pAsmbBody->AddBody(newBody);
	CLog::log("End [BodyPartition]");
}

// 复制剩余形体
void CFaceClipping::BodyRestCopy(CObArray *pLoopArr, CP_AssembledBody *pAsmbBody)
{
	CP_Body * restBody = new CP_BodyExt();
	int loopSize = pLoopArr->GetSize();
	
	CVertexCopyPool vtPool;
	CEdgeCopyPool edgePool;
	for(int i = 0; i < loopSize; i++)
	{
		CP_Loop * loop = (CP_Loop *)pLoopArr->GetAt(i);
		int halfSize = loop->GetHalfNumber();
		for(int i = 0; i < halfSize; i++)
		{
			CP_Half * half = loop->GetHalf(0);

			CP_VertexExt *nStrVert = vtPool.CopyVertex2Pool(half->m_pStartVt);
			CP_VertexExt * nEndVert = vtPool.CopyVertex2Pool(half->m_pEndVt);
			CP_Edge *pNewEdge = edgePool.CopyEdge2Pool(half->m_pEdge, nStrVert, nEndVert);

			CP_HalfExt *nHalf = (CP_HalfExt *)
				(pNewEdge->GetHalf(0)->m_pStartVt == nStrVert ? pNewEdge->GetHalf(0) : pNewEdge->GetHalf(1));
			nHalf->SetParent(half);			
			half->m_pLoop = NULL;
		}
		delete loop;
		loop = NULL;
	}
	pLoopArr->RemoveAll();

	vtPool.AddVertices2Body(restBody);
	edgePool.AddEdges2Body(restBody);

	// 增加断点处的连接边
	AddRestEdge(restBody);
	pAsmbBody->AddBody(restBody);

	// 如果newBody和restBody不是由originalBody分裂而得到，则删除上次遗留的restBody
	if (pAsmbBody->GetBodyNumber() != 3) {
		pAsmbBody->RemoveBody(pAsmbBody->GetBodyNumber()-3);
	}
}

// 增加剩余形体中的缺失边
void CFaceClipping::AddRestEdge(CP_Body * restBody)
{
	CObArray vertArr;
	int vertSize = restBody->GetVertexNumber();
	for(int i = 0; i < vertSize; i++)
	{
		CP_Vertex * vert = restBody->GetVertex(i);
		if(vert->GetOutHalfNumber() > 1) continue;
		vertArr.Add(vert);
	}
	int cutVertSize = vertArr.GetSize();

	//处理AILine情况
	if(cutVertSize == 0)
	{
		vertArr.RemoveAll();
		return;
	}

	if(cutVertSize != 2) {
		AfxMessageBox("ERROR IN BODY REST EDGE");
		return ;
	}

	CP_VertexExt * vert0 = (CP_VertexExt *)vertArr.GetAt(0);
	CP_VertexExt * vert1 = (CP_VertexExt *)vertArr.GetAt(1);
	
	vertArr.RemoveAll();

	int adjHalfSize = ((CP_Vertex *)(vert0->GetParent()))->GetOutHalfNumber();
	for(int i = 0; i < adjHalfSize; i++) {
		CP_Half * pCutHalf = ((CP_Vertex *)(vert0->GetParent()))->GetOutHalf(i);
		if((CP_Vertex *)vert1->GetParent() != pCutHalf->m_pEndVt)
			continue;
		
		CP_Edge *pCutEdge = pCutHalf->m_pEdge;
		CP_EdgeExt * nEdge = NULL;
		if(pCutEdge->m_pStartVt == vert0) {
			CP_HalfExt* half0 = new CP_HalfExt(vert0, vert1, NULL);
			vert0->AddOutHalf(half0);
			CP_HalfExt* half1 = new CP_HalfExt(vert1, vert0, NULL);
			vert1->AddOutHalf(half1);

			const char* strClassName = typeid(*(pCutEdge->m_pCurve3D)).name();
			CP_Curve3D *pNewCurve = (CP_Curve3D *)CObjectFactory::CopyObject(strClassName, *(pCutEdge->m_pCurve3D));

			nEdge = new CP_EdgeExt(pNewCurve, vert0, vert1, half0, half1);
			half0->m_pEdge = nEdge;
			half1->m_pEdge = nEdge;
			half0->SetParent(pCutHalf);
			half1->SetParent(pCutHalf->m_pTwin);
		}

		else {
			CP_HalfExt* half0 = new CP_HalfExt(vert1, vert0, NULL);
			vert1->AddOutHalf(half0);
			CP_HalfExt* half1 = new CP_HalfExt(vert0, vert1, NULL);
			vert0->AddOutHalf(half1);

			const char* strClassName = typeid(*(pCutEdge->m_pCurve3D)).name();
			CP_Curve3D *pNewCurve = (CP_Curve3D *)CObjectFactory::CopyObject(strClassName, *(pCutEdge->m_pCurve3D));

			nEdge = new CP_EdgeExt(pNewCurve, vert1, vert0, half0, half1);
			half0->m_pEdge = nEdge;
			half1->m_pEdge = nEdge;
			half0->SetParent(pCutHalf->m_pTwin);
			half1->SetParent(pCutHalf);
		}
		nEdge->SetParent(pCutEdge);
		restBody->AddEdge(nEdge);
	}
	
}

