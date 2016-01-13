#include "stdafx.h"
#include "CCavityDecomposition.h"
#include "CCopyPool.h"


CCavityDecomposition::~CCavityDecomposition(void)
{
}

// ���������ϵİ����棺����4�ȼ����ϵķ��ڽӵ��Ƿ��ұ��������ҵ�Χ�����С���Ǹ����ڲ���1
bool CCavityDecomposition::DetectCavityByLoop(CP_Body *& body, CP_Loop * cavityLoop)
{
	int halfSize = cavityLoop->GetHalfNumber();
	for(int i = 0; i < halfSize; i++)
	{
		CP_Vertex * vert0 = cavityLoop->GetHalf(i)->m_pStartVt;
		if(vert0->GetOutHalfNumber() < 4) continue;
		for(int j = i+2; j < halfSize; j++)
		{
			if(i == 0 && j == halfSize-1) continue;
			CP_Vertex * vert1 = cavityLoop->GetHalf(j)->m_pStartVt;
			if(vert1->GetOutHalfNumber() < 4) continue;
			CP_Half * half = GetConnectedHalf(vert0,vert1);
			if(half == NULL) continue;
			ConstructCavityFace(body,dynamic_cast<CP_LoopExt*>(cavityLoop),half);
			return true;
		}
	}
	return false;
}
// �ж���������֮���Ƿ��б������������������ı�
CP_Half * CCavityDecomposition::GetConnectedHalf(CP_Vertex * vert0, CP_Vertex * vert1)
{
	int adjSize = vert0->GetOutHalfNumber();
	for(int i = 0; i < adjSize; i++)
	{
		CP_Half * half = vert0->GetOutHalf(i);
		if(half->m_pEndVt == vert1) return half;
	}
	return NULL;
}
// ����Loop���������ڽӵ㣬������������İ�����
void CCavityDecomposition::ConstructCavityFace(CP_Body *& body, CP_LoopExt * cavityLoop, CP_Half * connectHalf)
{
	CP_LoopExt * interLoop0 = new CP_LoopExt();
	CP_LoopExt * interLoop1 = new CP_LoopExt();
	int halfSize = cavityLoop->GetHalfNumber();
	bool cutFlag = false;
	for(int i = 0; i < halfSize; i++)
	{
		CP_Half * half = cavityLoop->GetHalf(i);
		if(half->m_pStartVt == connectHalf->m_pStartVt)
		{
			interLoop0->AddHalf(connectHalf->m_pTwin);
			interLoop1->AddHalf(connectHalf);
			cutFlag = true;
		}
		if(half->m_pStartVt == connectHalf->m_pEndVt) cutFlag = false;
		if(cutFlag) interLoop0->AddHalf(half);
		else interLoop1->AddHalf(half);
	}
	CP_Loop * adjLoop0 = connectHalf->m_pLoop;
	CP_Loop * adjLoop1 = connectHalf->m_pTwin->m_pLoop;
	//���ø��滷
	interLoop0->SetParent(cavityLoop->GetParent());
	interLoop1->SetParent(cavityLoop->GetParent());

	//���Ʒ����滷�Լ����ӱߵ��ڽ��滷
	CP_Body * cutBody = new CP_BodyExt();
	std::vector<CP_Half *> * tempHalfArr = new std::vector<CP_Half *>();
	CP_Half * nConnectHalf0 = CopyCavityFace(cutBody,interLoop0,adjLoop0,connectHalf,tempHalfArr);
	CP_Half * nConnectHalf1 = CopyCavityFace(cutBody,interLoop1,adjLoop1,connectHalf,tempHalfArr);

	CVertexCopyPool vtPool;
	CEdgeCopyPool edgePool;

	//����ʣ�������滷
	int faceSize = body->GetFaceNumber();
	for(int i = 0; i < faceSize; i++)
	{
		CP_Face * face = body->GetFace(i);
		CP_LoopExt * loop = (CP_LoopExt *)face->GetLoop(0);
		if(loop == cavityLoop || loop == adjLoop0 || loop == adjLoop1) continue;
		//new loop
		CP_LoopExt * nLoop = new CP_LoopExt();
		CP_Face * nFace = new CP_Face(cutBody);
		nFace->AddLoop(nLoop);
		cutBody->AddFace(nFace);
		//new half & vert
		halfSize = loop->GetHalfNumber();
		for(int j = 0; j < halfSize; j++)
		{
			CP_Half * half = loop->GetHalf(j);
			CP_Vertex * strVert = half->m_pStartVt;
			CP_Vertex * endVert = half->m_pEndVt;
			CP_Vertex * nStrVert = NULL;
			CP_Vertex * nEndVert = NULL;
			if(strVert == connectHalf->m_pStartVt)
			{
				if(interLoop0->FindHalfIndex(half->m_pTwin) != -1) 
					nStrVert = nConnectHalf0->m_pStartVt;
				else if(adjLoop0->FindHalfIndex(half->m_pTwin) != -1) 
					nStrVert = nConnectHalf0->m_pStartVt;
				else nStrVert = nConnectHalf1->m_pStartVt;
			}
			else if(strVert == connectHalf->m_pEndVt)
			{
				if(interLoop0->FindHalfIndex(half->m_pTwin) != -1) 
					nStrVert = nConnectHalf0->m_pEndVt;
				else if(adjLoop0->FindHalfIndex(half->m_pTwin) != -1) 
					nStrVert = nConnectHalf0->m_pEndVt;
				else nStrVert = nConnectHalf1->m_pEndVt;
			}
			if(endVert == connectHalf->m_pStartVt)
			{
				if(interLoop0->FindHalfIndex(half->m_pTwin) != -1) 
					nEndVert = nConnectHalf0->m_pStartVt;
				else if(adjLoop0->FindHalfIndex(half->m_pTwin) != -1) 
					nEndVert = nConnectHalf0->m_pStartVt;
				else nEndVert = nConnectHalf1->m_pStartVt;
			}
			else if(endVert == connectHalf->m_pEndVt)
			{
				if(interLoop0->FindHalfIndex(half->m_pTwin) != -1) 
					nEndVert = nConnectHalf0->m_pEndVt;
				else if(adjLoop0->FindHalfIndex(half->m_pTwin) != -1) 
					nEndVert = nConnectHalf0->m_pEndVt;
				else nEndVert = nConnectHalf1->m_pEndVt;
			}
			if(nStrVert == NULL) nStrVert = vtPool.CopyVertex2Pool(half->m_pStartVt);
			if(nEndVert == NULL) nEndVert = vtPool.CopyVertex2Pool(half->m_pEndVt);

			CP_Edge *pEdge = edgePool.CopyEdge2Pool(half->m_pEdge, nStrVert, nEndVert);
			CP_HalfExt *nHalf = (CP_HalfExt *)
				(pEdge->GetHalf(0)->m_pStartVt == nStrVert ? pEdge->GetHalf(0) : pEdge->GetHalf(1));
			nHalf->SetParent(static_cast<CP_HalfExt*>(half)->GetParent());
			nHalf->m_pLoop = nLoop;
			nLoop->AddHalf(nHalf);
		}
		CP_Loop * pLoop = (CP_Loop *)loop->GetParent();
		if(pLoop != loop) nLoop->SetParent(pLoop);
	}
	vtPool.AddVertices2Body(cutBody);
	edgePool.AddEdges2Body(cutBody);

	m_pAsmbBody->AddBody(cutBody);
	//���ݰ������������ָ�
	PartitionBodyByCavity(cutBody);

	//delete temp
	delete tempHalfArr;
	tempHalfArr = NULL;
	//delete body
	if(body == m_pAsmbBody->GetOriginalBody()) return;
	m_pAsmbBody->RemoveBody(body);
	// body->DeleteAllItems();
	delete body;
	body = NULL;
}
// ���Ʒ����һ�������漰���Ӧ�ڽ���
CP_Half * CCavityDecomposition::CopyCavityFace(CP_Body * cutBody, CP_Loop * interLoop, CP_Loop * adjLoop, CP_Half * connectHalf,  std::vector<CP_Half *> * tempHalfArr)
{
	CP_Loop * loopArr[2];
	loopArr[0] = interLoop;
	loopArr[1] = adjLoop;

	CVertexCopyPool vtPool;
	CEdgeCopyPool edgePool;

	CP_VertexExt * nStr = vtPool.CopyVertex2Pool(connectHalf->m_pStartVt);
	CP_VertexExt * nEnd = vtPool.CopyVertex2Pool(connectHalf->m_pEndVt);
	CP_Edge *pEdge = edgePool.CopyEdge2Pool(connectHalf->m_pEdge, nStr, nEnd);
	CP_HalfExt *nHalf0 = (CP_HalfExt *)
		(pEdge->GetHalf(0)->m_pStartVt == nStr ? pEdge->GetHalf(0) : pEdge->GetHalf(1));
	nHalf0->SetParent(connectHalf);
	CP_HalfExt *nHalf1 = (CP_HalfExt *)
		(pEdge->GetHalf(0)->m_pStartVt == nStr ? pEdge->GetHalf(1) : pEdge->GetHalf(0));
	nHalf1->SetParent(connectHalf->m_pTwin);

	edgePool.AddEdges2Body(cutBody);

	for(int i = 0; i < 2; i++)
	{
		CP_LoopExt * loop = (CP_LoopExt *)loopArr[i];
		//new loop & face
		CP_LoopExt * nLoop = new CP_LoopExt();
		CP_Face * nFace = new CP_Face(cutBody);
		nFace->AddLoop(nLoop);
		cutBody->AddFace(nFace);
		//new half & vert
		int halfSize = loop->GetHalfNumber();
		for(int j = 0; j < halfSize; j++)
		{
			CP_Half * half = loop->GetHalf(j);
			if(half == connectHalf)
			{
				nHalf0->m_pLoop = nLoop;
				nLoop->AddHalf(nHalf0);
				continue;
			}
			if(half == connectHalf->m_pTwin)
			{
				nHalf1->m_pLoop = nLoop;
				nLoop->AddHalf(nHalf1);
				continue;
			}
			CP_VertexExt * strVert = dynamic_cast<CP_VertexExt*>(half->m_pStartVt);
			CP_VertexExt * endVert = dynamic_cast<CP_VertexExt*>( half->m_pEndVt);
			CP_VertexExt * nStrVert = NULL;
			CP_VertexExt * nEndVert = NULL;
			if(strVert->GetParent() == nStr->GetParent()) nStrVert = nStr;
			if(strVert->GetParent() == nEnd->GetParent()) nStrVert = nEnd;
			if(endVert->GetParent() == nStr->GetParent()) nEndVert = nStr;
			if(endVert->GetParent() == nEnd->GetParent()) nEndVert = nEnd;
			if(nStrVert == NULL) nStrVert = vtPool.CopyVertex2Pool(strVert);
			if(nEndVert == NULL) nEndVert = vtPool.CopyVertex2Pool(endVert);
			CP_HalfExt * nHalf = new CP_HalfExt(nStrVert,nEndVert,half->m_pEdge);
			nHalf->SetParent(half);
			nHalf->m_pLoop = nLoop;
			nLoop->AddHalf(nHalf);
			tempHalfArr->push_back(nHalf);
		}
		CP_Loop * pLoop = (CP_Loop *)loop->GetParent();
		if(pLoop != loop) nLoop->SetParent(pLoop);
	}
	vtPool.AddVertices2Body(cutBody);
	return nHalf0;
}
// ���뵱ǰ������
void CCavityDecomposition::PartitionBodyByCavity(CP_Body * body)
{
	//�����һ�������ڵ������滷ѹ��loopArr��
	std::vector<CP_Loop *> * loopArr = new std::vector<CP_Loop *>();
	CP_Loop * firstLoop = body->GetFace(0)->GetLoop(0);
	loopArr->push_back(firstLoop);
	unsigned int index = 0;
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

	int faceSize = body->GetFaceNumber();
	int loopSize = loopArr->size();
	if(faceSize == loopSize)
	{
		for(int i = 0; i < loopSize; i++)
		{
			CP_LoopExt * loop = (CP_LoopExt *)loopArr->at(i);
			loop->m_cutFlag = -1;
		}
		delete loopArr;
		return;
	}
	CP_Body * cutBody = new CP_BodyExt();
	for(int i = 0; i < loopSize; i++)
	{
		CP_LoopExt * loop = (CP_LoopExt *)loopArr->at(i);
		CP_Face * face = loop->m_pParentFace;
		int halfSize = loop->GetHalfNumber();
		for(int j = 0; j < halfSize; j++)
		{
			CP_Half * half = loop->GetHalf(j);
			CP_Edge * edge = half->m_pEdge;
			CP_Vertex * vert = half->m_pStartVt;
			cutBody->AddEdge(edge);
			cutBody->AddVertex(vert);
			body->RemoveEdge(edge);
			body->RemoveVertex(vert);
		}
		cutBody->AddFace(face);
		body->RemoveFace(face);
		loop->m_cutFlag = -1;
	}
	m_pAsmbBody->AddBody(cutBody);
}
