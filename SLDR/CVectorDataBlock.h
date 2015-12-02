#pragma once

#include "AbstractDataBlock.h"
#include "CP_Topology.h"

class CVectorDataBlock: public AbstractDataBlock<CP_Vector3D>
{
private:
	double **m_disMatrix;
	int		 m_hashFactor;

public:
	CVectorDataBlock(double radius);
	~CVectorDataBlock();

public:
	virtual void GetNeighborData(DataWithVLabel<CP_Vector3D>* data, vector<DataWithVLabel<CP_Vector3D>*> *vResult);	// Get neighbor data whose distance to data is less than radius
	virtual int GetBlockIndex(DataWithVLabel<CP_Vector3D>* data);

	void AddMember(DataWithVLabel<CP_Vector3D>* data);
	void AddMember(CP_Point3D *pStartPt, CP_Point3D *pEndPt);

	void InitDisMatrix();

private:
	int  HashFunc(int blockIndex, int dataIndex);
	double GetDis(int blockIndex0, int dataIndex0, int blockIndex1, int dataIndex1);
};