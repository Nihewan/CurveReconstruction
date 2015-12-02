#pragma once
#include "CP_Topology.h"
#include "IDecomposition.h"
#include <map>
#include <vector>


class CPseudoDecomposition: public IDecomposition
{
public:
	CPseudoDecomposition(CP_AssembledBody *pAsmbBody): IDecomposition(pAsmbBody){}
	~CPseudoDecomposition(void);

public:
	bool	Decompose();

private:
	std::map<CP_Vertex *, bool>			m_vertVisited;
	std::map<CP_Vertex *, CP_Half *>	m_vertPath;

private:
	bool		DetectPseudoByVert(CP_Body *& pBody);
	
	// �ݹ��㷨������ȫ��4�ȵ���ɵ�PseudoFace
	CP_Loop*	ConstructPseudoFace(CP_Vertex * pQuaVert);
	
	// ����Ƿ��������
	bool		ExistTrueFace(CP_Vertex * pQuaVert, CP_Half * pAdjHalf);
	
	// ����pseudoLoop������������
	void		PartitionBodyByPseudo(CP_Body *pBody, CP_Loop * pPseudoLoop);
	
	// ���Ʋ�������
	void		CopyBodyByPsuedo(std::vector<CP_Loop *> *loopArr);

	void		CreateTwinLoop(CP_Loop *pSrcLoop, CP_LoopExt *pDstLoop);

private:
	CP_Half*	GetPath(CP_Vertex *vert);
	void		AddPath(CP_Vertex *vert, CP_Half *half);
	void		SetPath(CP_Vertex *vert, CP_Half *half);

	bool		IsVisited(CP_Vertex *vert);
	void		SetVisited(CP_Vertex *vert);
	void		SetUnvisited(CP_Vertex* vert);
	void		AddUnvisitedVert(CP_Vertex *vert);


};

