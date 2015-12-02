#pragma once
class CP_AssembledBody;
class CP_Body;
class CFaceLoopPool;
class CP_Loop;
class CP_VertexExt;
class CP_Vertex;
class CP_EdgeExt;
class CP_Edge;

// √Êª∑≤√ºÙÀ„∑®
class CFaceClipping
{
public:
	CFaceClipping(void);
	~CFaceClipping(void);

public:
	static void	ExamineAdjMerg(CFaceLoopPool &faceLoopPool, CP_AssembledBody *pABody);
	static bool BeAdjMerg(CObArray *pLoopArr, CP_Loop * loop0, CP_Loop * loop1);
	static void	BodyPartition(CObArray *pFaceArr, CP_AssembledBody *pABody);
	static void	BodyRestCopy(CObArray *pLoopArr, CP_AssembledBody *pABody);
	static void	AddRestEdge(CP_Body * restBody);

};

