#pragma once
#include "IDecomposition.h"

class CBodyDecomposition: public IDecomposition
{
public:
	CBodyDecomposition(CP_AssembledBody *pAsmbBody): IDecomposition(pAsmbBody){}
	~CBodyDecomposition(void){};

public:
	bool	Decompose();
};

