#pragma once
#include "CP_Topology.h"
#include "CP_TopologyExt.h"

class IDecomposition
{
protected:
	CP_AssembledBody *m_pAsmbBody;										// Memory managed by others	

public:
	IDecomposition(CP_AssembledBody *pAsmbBody):m_pAsmbBody(pAsmbBody){}

	virtual ~IDecomposition(void){};

public:
	virtual bool Decompose() =0;			// Decompose body by some method
};

