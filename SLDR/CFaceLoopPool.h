#pragma once
class CP_LoopExt;
class CP_Body;
class CP_Half;
class CP_Loop;

#include <vector>
#include "CP_TopologyExt.h"


class CFaceLoopPool
{

public:
	CFaceLoopPool(CP_Body *pBody);
	~CFaceLoopPool(void);

public:
	CObArray	m_loopArray;		// δ������Ļ�
	CObArray	m_faceLoopArray;	// �Ѿ�������Ļ�

public:
	CP_Loop*	GetLoop(int index);
	CP_Loop*	GetFaceLoop(int index);
	int			GetLoopNum();
	int			GetFaceLoopNum();

	void		DeleteLoop(CP_LoopExt *loop);				// Release memory
	void		RemoveLoopFromPool(CP_LoopExt *loop);		// Do not release memory

	void		ConnectLoop(CP_LoopExt *&loopExtd, CP_LoopExt *&loopNext, CP_AssembledBody *pAsmbBody);
	void		ConnectLoop(CP_LoopExt *& loopExtd, CP_LoopExt *& loopNext, CP_LoopExt *& loopRest, CP_AssembledBody *pAsmbBody);
	void		AddLoop2FaceLoop(CP_LoopExt * loop, CP_AssembledBody *pAsmbBody);
	void		AddFace2Body(CP_Body *pBody);

	bool		GetExtendLoopsByPriority(CObArray &resultArr);			// �������ȼ���ȡ��չ����
	CP_Loop *	GetRestLoop(CP_Loop * loop0, CP_Loop * loop1);
	CP_ParaLoopsGroupsArr*	GetExtendLoopPair(CObArray &loops);
};
