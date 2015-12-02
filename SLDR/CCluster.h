#pragma once
#include "CP_Topology.h"

template<typename T>
struct CCluster
{
	typedef double(*DisFunc)(const T& a, const T& b);

	T* m_center;
	vector<T *> m_members;

	CCluster(){m_center = NULL;}

	void AddMember(T *pMember) {
		m_members.push_back(pMember);
	}

	T* GetMember(int index) {
		return m_members.at(index);
	}

	int GetSize(){return m_members.size();}

	static bool DSCCmp(CCluster *a, CCluster *b) {
		return a->GetSize() > b->GetSize();
	}

	// Calculate center of cluster
	T* CalcCenter(DisFunc disFunc){
		if (m_center != NULL)
			return m_center;

		const int size = GetSize();
		double **dis = new double*[GetSize()];
		for (int i = 0; i < GetSize(); ++i)
			dis[i] = new double[GetSize()];
		double minDisSum = 100;
		for (int i = 0; i < GetSize(); ++i) {
			for (int j = 0; j < GetSize(); ++j) {
				dis[i][j] = disFunc(*m_members.at(i), *m_members.at(j));
			}
		}
		for (int i = 0; i < GetSize(); ++i) {
			double disSum = 0;
			for (int j = 0; j < GetSize(); ++j) {
				disSum += dis[i][j];
			}
			if (disSum < minDisSum) {
				minDisSum = disSum;
				m_center = m_members.at(i);
			}
		}

		for (int i = 0; i < GetSize(); ++i) {
			delete []dis[i];
			dis[i] = NULL;
		}
		delete []dis;
		dis = NULL;

		return m_center;
	};
}; 
