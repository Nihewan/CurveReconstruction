#include "stdafx.h"
#include "CPseudoDecomposition.h"
#include "CCopyPool.h"
#include "propertiesnames.h"


CPseudoDecomposition::~CPseudoDecomposition(void)
{
}

bool CPseudoDecomposition::Decompose() {
	int index = 0;
	while(index < m_pAsmbBody->GetBodyNumber())
	{
		CP_Body * pBody = m_pAsmbBody->GetBody(index);
		if(index == 0 && m_pAsmbBody->GetBodyNumber() > 1)
		{
			index++;
			pBody = m_pAsmbBody->GetBody(index);
		}
		if(DetectPseudoByVert(pBody)) continue;
		else index++;
	}

	return true;
}

// 完全由4度及以上点组成的内部面，不断搜索4度点及其邻接4度点，看是否能够组成非真面面环
bool CPseudoDecomposition::DetectPseudoByVert(CP_Body *& pBody)
{
	int vertSize = pBody->GetVertexNumber();
	for(int i = 0; i < vertSize; i++)
	{
		CP_Vertex * vert = pBody->GetVertex(i);
		AddUnvisitedVert(vert);
		AddPath(vert, NULL);
	}
	for(int i = 0; i < vertSize; i++)
	{
		CP_Vertex * vert = pBody->GetVertex(i);
		if(vert->GetOutHalfNumber() < 4) continue;
		CP_Loop * pseudoLoop = ConstructPseudoFace(vert);
		if(pseudoLoop != NULL)
		{
			PartitionBodyByPseudo(pBody, pseudoLoop);
			return true;
		}
	}
	return false;
}

// 递归算法构造完全由4度点组成的PseudoFace
CP_Loop * CPseudoDecomposition::ConstructPseudoFace(CP_Vertex * quaVert)
{
	SetVisited(quaVert);
	int adjSize = quaVert->GetOutHalfNumber();
	CP_Half * adjHalf = NULL;
	CP_Vertex * adjVert = NULL;
	bool flag = false;
	for(int i = 0; i < adjSize; i++)
	{
		adjHalf = quaVert->GetOutHalf(i);
		adjVert = adjHalf->m_pEndVt;
		if(adjVert->GetOutHalfNumber() == 3) continue;
		if(adjHalf->m_pTwin == GetPath(quaVert)) continue; //非回头边
		if(adjVert->GetOutHalfNumber() == 2)
		{
			SetPath(adjVert, adjHalf);
			return ConstructPseudoFace(adjVert);
		}
		if(ExistTrueFace(quaVert,adjHalf)) continue; //非真面边
		if(IsVisited(adjVert))
		{
			flag = true;
			break;
		}
		SetPath(adjVert, adjHalf);
		CP_Loop * retLoop = ConstructPseudoFace(adjVert);
		if(retLoop != NULL) return retLoop;
	}
	if(!flag)
	{
		SetUnvisited(quaVert);
		return NULL;
	}

	CP_Half * half = adjHalf->m_pTwin;
	CP_Vertex * vert = half->m_pEndVt;
	CP_LoopExt * nLoop = new CP_LoopExt();
	nLoop->AddHalf(half);
	while(vert != adjVert)
	{
		half = GetPath(vert)->m_pTwin;
		nLoop->AddHalf(half);
		vert = half->m_pEndVt;
	}
	return nLoop;
}

// 检测是否组成真面
bool CPseudoDecomposition::ExistTrueFace(CP_Vertex * quaVert, CP_Half * adjHalf)
{
	if(quaVert->GetOutHalfNumber() == 2) return false;
	if(GetPath(quaVert) == NULL) return false;
	CP_Half * half0 = GetPath(quaVert);
	CP_Half * half1 = adjHalf;
	if(half0->m_pLoop == half1->m_pLoop) return true;
	if(half0->m_pTwin->m_pLoop == half1->m_pTwin->m_pLoop) return true;
	return false;
}

// 根据pseudoLoop分离两个形体
void CPseudoDecomposition::PartitionBodyByPseudo(CP_Body *body, CP_Loop *pseudoLoop)
{
	std::vector<CP_Loop *> *tempLoopArr = new std::vector<CP_Loop *>();
	int halfSize = pseudoLoop->GetHalfNumber();
	for(int i = 0; i < halfSize; i++)
	{
		CP_Half * half = pseudoLoop->GetHalf(i);
		((CP_LoopExt *)half->m_pLoop)->m_cutFlag = 0;
		CP_LoopExt * loop = (CP_LoopExt *)half->m_pTwin->m_pLoop;
		if(loop->m_cutFlag == -1)
		{
			loop->m_cutFlag = 0;
			tempLoopArr->push_back(loop);
		}
	}
	unsigned int index = 0;
	while(index < tempLoopArr->size())
	{
		CP_LoopExt * loop = (CP_LoopExt *)tempLoopArr->at(index);
		//检测每一条邻接Half
		int halfSize = loop->GetHalfNumber();
		for(int i = 0; i < halfSize; i++)
		{
			CP_Half * half = loop->GetHalf(i);
			CP_LoopExt * adjLoop = (CP_LoopExt *)half->m_pTwin->m_pLoop;
			if(adjLoop->m_cutFlag == -1)
			{
				adjLoop->m_cutFlag = 0;
				tempLoopArr->push_back(adjLoop);
			}
		}
		loop->m_cutFlag = 1;
		index++;
	}
	tempLoopArr->push_back(pseudoLoop);
	CopyBodyByPsuedo(tempLoopArr);

	int faceSize = body->GetFaceNumber();
	for(int i = 0; i < faceSize; i++)
	{
		CP_Face * face = body->GetFace(i);
		CP_LoopExt * loop = (CP_LoopExt *)face->GetLoop(0);
		if(loop->m_cutFlag != 1) tempLoopArr->push_back(loop);
		loop->m_cutFlag = -1;
	}
	CP_LoopExt *pTwinLoop = new CP_LoopExt;
	CreateTwinLoop(pseudoLoop, pTwinLoop); // 创建一个与原来环反向的环pTwinLoop，pTwinLoop符合第二个形体的环方向
	tempLoopArr->push_back(pTwinLoop);
	CopyBodyByPsuedo(tempLoopArr); 

	// delete
	delete pseudoLoop;
	pseudoLoop = NULL;
	delete pTwinLoop;
	pTwinLoop = NULL;
	delete tempLoopArr;
	//delete body
	if(body == m_pAsmbBody->GetOriginalBody()) return;
	m_pAsmbBody->RemoveBody(body);
	// body->DeleteAllItems();
}

// 复制部分形体
void CPseudoDecomposition::CopyBodyByPsuedo(std::vector<CP_Loop *> *loopArr)
{
	CP_Body * cutBody = new CP_BodyExt();
	int loopSize = loopArr->size();

	CVertexCopyPool vtPool;
	CEdgeCopyPool edgePool;

	for(int i = 0; i < loopSize; i++)
	{
		CP_LoopExt * loop = (CP_LoopExt *)loopArr->at(i);
		//new loop & face
		CP_LoopExt * nLoop = new CP_LoopExt();
		CP_FaceExt * nFace = new CP_FaceExt(cutBody);

		nFace->AddLoop(nLoop);
		cutBody->AddFace(nFace);
		//new half & vert
		int halfSize = loop->GetHalfNumber();
		for(int j = 0; j < halfSize; j++)
		{
			CP_Half * half = loop->GetHalf(j);
			CP_Vertex * nStrVert = vtPool.CopyVertex2Pool(half->m_pStartVt);
			CP_Vertex * nEndVert = vtPool.CopyVertex2Pool(half->m_pEndVt);
			CP_Edge *pEdge = edgePool.CopyEdge2Pool(half->m_pEdge, nStrVert, nEndVert);

			CP_HalfExt *nHalf = (CP_HalfExt *)
				(pEdge->GetHalf(0)->m_pStartVt == nStrVert ? pEdge->GetHalf(0) : pEdge->GetHalf(1));
			nHalf->SetParent(static_cast<CP_HalfExt*>(half)->GetParent());
			nHalf->m_pLoop = nLoop;
			nLoop->AddHalf(nHalf);

			//half->SetLoop(NULL);
		}
		CP_Loop * pLoop = (CP_Loop *)loop->GetParent();
		if (i < loopSize-1 && !loop->HasProperty(PROPERTY_NAME_IS_PSEUDO))
			nLoop->SetParent(dynamic_cast<CP_LoopExt*>(pLoop)->GetParent());
		else
			nLoop->SetProperties(PROPERTY_NAME_IS_PSEUDO, 1);
	}
	loopArr->clear();
	vtPool.AddVertices2Body(cutBody);
	edgePool.AddEdges2Body(cutBody);
	m_pAsmbBody->AddBody(cutBody);
}

void CPseudoDecomposition::CreateTwinLoop(CP_Loop *pSrcLoop, CP_LoopExt *pDstLoop) {
	int size = pSrcLoop->GetHalfNumber();
	for(int i = size-1; i >= 0; i--)
	{
		CP_Half * half = pSrcLoop->GetHalf(i);
		pDstLoop->AddHalf(half->m_pTwin);
	}
}

/************************************************************************/
/*		                                                                */
/************************************************************************/

void CPseudoDecomposition::AddPath(CP_Vertex *vert, CP_Half *half) {
	m_vertPath.insert(pair<CP_Vertex *, CP_Half *>(vert, NULL));
}

CP_Half *CPseudoDecomposition::GetPath(CP_Vertex *vert) {
	map<CP_Vertex *, CP_Half* >::iterator it; 
	it = m_vertPath.find(vert);
	if (it != m_vertPath.end())
		return it->second;
	return NULL;
}

void CPseudoDecomposition::SetPath(CP_Vertex *vert, CP_Half *half) {
	map<CP_Vertex *, CP_Half* >::iterator it; 
	it = m_vertPath.find(vert);
	if (it != m_vertPath.end()) {
		it->second = half;
	} else {
		ASSERT(false);
	}
}

void CPseudoDecomposition::AddUnvisitedVert(CP_Vertex *vert) {
	m_vertVisited.insert(pair<CP_Vertex *, bool>(vert, false));
}


void CPseudoDecomposition::SetUnvisited(CP_Vertex* vert) {
	map<CP_Vertex *, bool >::iterator it; 
	it = m_vertVisited.find(vert);
	if (it != m_vertVisited.end())
		it->second = false;
}

bool CPseudoDecomposition::IsVisited(CP_Vertex *vert) {
	map<CP_Vertex *, bool >::iterator it; 
	it = m_vertVisited.find(vert);
	if (it != m_vertVisited.end())
		return it->second;
	return false;
}

void CPseudoDecomposition::SetVisited(CP_Vertex *vert) {
	map<CP_Vertex *, bool >::iterator it; 
	it = m_vertVisited.find(vert);
	if (it != m_vertVisited.end())
		it->second = true;
}