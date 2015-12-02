#pragma once
#include "IDecomposition.h"

class CCavityDecomposition: public IDecomposition
{
public:
	CCavityDecomposition(CP_AssembledBody *pAsmbBody): IDecomposition(pAsmbBody){}
	~CCavityDecomposition(void);

public:
	bool	Decompose(){return true;};

private:
	//========处理凹槽面操作==========
	bool DetectCavityByLoop(CP_Body *& body,CP_Loop * cavityLoop);
	// 判断两个顶点之间是否有边相连，并返回相连的边
	CP_Half * GetConnectedHalf(CP_Vertex * vert0, CP_Vertex * vert1);
	// 构造两个分离的凹槽面
	void ConstructCavityFace(CP_Body *& body, CP_LoopExt * cavityLoop, CP_Half * connectHalf);
	// 复制分离的一个凹槽面及其对应邻接面
	CP_Half * CopyCavityFace(CP_Body * cutBody, CP_Loop * interLoop, CP_Loop * adjLoop, CP_Half * connectHalf, std::vector<CP_Half *> * tempHalfArr);
	// 分离当前子形体
	void PartitionBodyByCavity(CP_Body * body);
};  

