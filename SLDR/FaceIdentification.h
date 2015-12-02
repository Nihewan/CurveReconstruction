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

	void		TopologyDeduce();				// 算法2.2：拓扑推导算法
	void		ExtendLoopByPriority();			// 算法2.4：链条扩展算法
	

// field
private:
	CP_Body*			m_pCurrentBody;
	CP_AssembledBody*	m_pAsmbBody;
	CFaceLoopPool*		m_pFLPool;
};

