#pragma once
#include "CP_Topology.h"
class CFaceLoopPool;
class CP_AssembledBody;

class CFaceIdentification
{

public:
	CFaceIdentification(CP_AssembledBody *pAsmbBody);
	virtual ~CFaceIdentification(void);
// public method
public:
	bool		FaceIdentify();

// private method
private:
	CP_Vertex*	GetMaxTriVert();
	bool		FindTriLoop(CP_Half *&pOutHalf0, CP_Half *&pOutHalf1, CP_Half *&pOutHalf2);
	bool		SingleBodyFaceIdentify();

	void		TopologyDeduce();				// �㷨2.2�������Ƶ��㷨
	void		ExtendLoopByPriority();			// �㷨2.4��������չ�㷨
	

// field
private:
	CP_Body*			m_pCurrentBody;
	CP_AssembledBody*	m_pAsmbBody;
	CFaceLoopPool*		m_pFLPool;
};

