#pragma once
#include <vector>
#include "CCluster.h"
#include "AbstractDataBlock.h"

/*
* DBScan
*/

template<typename T>
class CDBScan
{
private:

	AbstractDataBlock<T> *m_pDataBlock;
	unsigned int m_minPt;

public:
	CDBScan(AbstractDataBlock<T> *pDataBlock, unsigned int minPt);
	~CDBScan(void);

public:
	void	DBScan(vector<CCluster<T> *> *pResultCluster);
	void	Search(DataWithVLabel<T>* curData, vector<CCluster<T> *> *pResultCluster);

};

template<typename T>
CDBScan<T>::CDBScan(AbstractDataBlock<T> *pDataBlock, unsigned int minPt)
{
	m_pDataBlock = pDataBlock;
	m_minPt = minPt;
}

template<typename T>
CDBScan<T>::~CDBScan(void)
{
}

template<typename T>
void CDBScan<T>::DBScan(vector<CCluster<T> *> *pResultCluster) {
	for (int i = 0; i < m_pDataBlock->GetBlockSize(); ++i) {
		for (int j = 0; j < m_pDataBlock->GetDataSizeOfBlock(i); ++j) {

			DataWithVLabel<T> *pDataWithL = m_pDataBlock->GetMember(i, j);
			if (pDataWithL->m_label == 0) {
				Search(pDataWithL, pResultCluster);
			}
		}
    }
}

template<typename T>
void CDBScan<T>::Search(DataWithVLabel<T>* pCurData, vector<CCluster<T> *> *pResultCluster) {
	vector<DataWithVLabel<T>*> sExtensibleData;
	vector<DataWithVLabel<T>*> vNeighbors;
	do {
		m_pDataBlock->GetNeighborData(pCurData, &vNeighbors);
		if (vNeighbors.size() > m_minPt) {
			if (pCurData->m_label == 0) {
				CCluster<T> *pCluster = new CCluster<T>;
				pCluster->AddMember(pCurData->m_data);
				pResultCluster->push_back(pCluster);
				pCurData->m_label = pResultCluster->size();
			}
			CCluster<T> *clus = pResultCluster->at(pCurData->m_label-1);
			for (unsigned int i = 0; i < vNeighbors.size(); ++i) {
				if (vNeighbors.at(i)->m_label == 0) {
					clus->AddMember(vNeighbors.at(i)->m_data);
					sExtensibleData.push_back(vNeighbors.at(i));
					vNeighbors.at(i)->m_label = pCurData->m_label;
				}
			}
		}
		if (sExtensibleData.empty())break;
		pCurData = sExtensibleData.back();
		sExtensibleData.pop_back();
		vNeighbors.clear();
	} while (!sExtensibleData.empty());
}