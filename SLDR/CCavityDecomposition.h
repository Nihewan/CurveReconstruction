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
	//========�����������==========
	bool DetectCavityByLoop(CP_Body *& body,CP_Loop * cavityLoop);
	// �ж���������֮���Ƿ��б������������������ı�
	CP_Half * GetConnectedHalf(CP_Vertex * vert0, CP_Vertex * vert1);
	// ������������İ�����
	void ConstructCavityFace(CP_Body *& body, CP_LoopExt * cavityLoop, CP_Half * connectHalf);
	// ���Ʒ����һ�������漰���Ӧ�ڽ���
	CP_Half * CopyCavityFace(CP_Body * cutBody, CP_Loop * interLoop, CP_Loop * adjLoop, CP_Half * connectHalf, std::vector<CP_Half *> * tempHalfArr);
	// ���뵱ǰ������
	void PartitionBodyByCavity(CP_Body * body);
};  

