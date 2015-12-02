#include "stdafx.h"
#include "CVectorDataBlock.h"
#include "base_geometricoper.h"

CVectorDataBlock::CVectorDataBlock(double radius):AbstractDataBlock(radius), m_hashFactor(0) {
	int numOfBlock = (int)ceil(PI2 / m_radius);
	for (int i = 0; i < numOfBlock; ++i) {
		AddBlock();
	}
}

CVectorDataBlock::~CVectorDataBlock(){
	for (int i = 0; i < GetBlockSize() * m_hashFactor; ++i) {
		delete []m_disMatrix[i];
		m_disMatrix[i] = NULL;
	}
	delete m_disMatrix;
	m_disMatrix = NULL;
}

void CVectorDataBlock::GetNeighborData(DataWithVLabel<CP_Vector3D>* pData, vector<DataWithVLabel<CP_Vector3D>*> *pVResult) {
	int blockIndex = pData->m_blockIndex;
	int dataIndex = pData->m_dataIndexInBlock;
	int oppositveBlock = m_blocks.size()/2;
	int effectiveBlock[] = {-1, 0, 1, oppositveBlock-1, oppositveBlock, oppositveBlock + 1};
	for (int i = 0; i < 6; ++i) {
		int curBlockIndex = (blockIndex + effectiveBlock[i]);
		if (curBlockIndex < 0)
			curBlockIndex = m_blocks.size() - 1;
		else
			curBlockIndex %= m_blocks.size();
		for (unsigned int j = 0; j < GetBlock(curBlockIndex).size(); ++j) {
			CP_Vector3D *pTmpV = GetMember(curBlockIndex, j)->m_data;
			if (pTmpV != pData->m_data && GetDis(blockIndex, dataIndex, curBlockIndex, j) < m_radius)
				pVResult->push_back(GetMember(curBlockIndex, j));
		}
	}
}



int CVectorDataBlock::GetBlockIndex(DataWithVLabel<CP_Vector3D>* pData) {
	CP_Vector3D v(1.0, 0.0, 0.0);
	CP_Vector3D r = *pData->m_data ^ v;
	double cosValue = *pData->m_data * v / pData->m_data->GetLength();
	double angle = base_geometric::GetACOS(cosValue);
	if (r.m_z > TOLER)
		angle = PI2 - angle;

	return (int)(angle / m_radius);
}


void CVectorDataBlock::AddMember(DataWithVLabel<CP_Vector3D>* pData) {
	int blockIndex = GetBlockIndex(pData);
	pData->m_blockIndex = blockIndex;
	pData->m_dataIndexInBlock = GetDataSizeOfBlock(blockIndex);
	AbstractDataBlock::AddMember2Block(blockIndex, pData);
}

void CVectorDataBlock::AddMember(CP_Point3D *pStartPt, CP_Point3D *pEndPt) {
	CP_Vector3D *pVec = new CP_Vector3D(*pEndPt - *pStartPt);
	DataWithVLabel<CP_Vector3D> *pNewD = new DataWithVLabel<CP_Vector3D>(pVec);
	AddMember(pNewD);
}

void CVectorDataBlock::InitDisMatrix() {
	for (int i = 0; i < GetBlockSize(); ++i) {
		int size = GetDataSizeOfBlock(i);
		m_hashFactor = size > m_hashFactor? size : m_hashFactor;
	}

	// Initialize the size of m_disMatrix to [Block size * Max size of single block]^2
	m_disMatrix = new double*[m_hashFactor * GetBlockSize()];
	for (int i = 0; i < m_hashFactor * GetBlockSize(); ++i) {
		m_disMatrix[i] = new double[m_hashFactor * GetBlockSize()];
		for (int j = 0; j < m_hashFactor * GetBlockSize(); ++j)
			m_disMatrix[i][j] = 0;
	}
}

int CVectorDataBlock::HashFunc(int blockIndex, int dataIndex) {
	return blockIndex * m_hashFactor + dataIndex;
}

double CVectorDataBlock::GetDis(int blockIndex0, int dataIndex0, int blockIndex1, int dataIndex1) {
	int index0 = HashFunc(blockIndex0, dataIndex0);
	int index1 = HashFunc(blockIndex1, dataIndex1);

	if (m_disMatrix[index0][index1] > 0)
		return m_disMatrix[index0][index1];
	else if (m_disMatrix[index1][index0] > 0)
		return m_disMatrix[index1][index0];

	m_disMatrix[index0][index1] = 
		base_geometric::GetMinIncludedAngle(*GetMember(blockIndex0, dataIndex0)->m_data,
		*GetMember(blockIndex1, dataIndex1)->m_data);

	return m_disMatrix[index0][index1];
}