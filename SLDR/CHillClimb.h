#pragma once
#include "CP_TopologyExt.h"
#include "IObjectiveFuncCalculator.h"

class CHillClimb
{
private:
	IObjectiveFuncCalculator *	m_pObjectiveFunc;
	vector<double>				m_value;
	double						m_initStep[2];

public:
	CHillClimb(IObjectiveFuncCalculator *pObjectiveFunc, vector<double> &initValueArr, double initStep = 1.0);
	~CHillClimb(void);

public:
	void	HillClimb();

private:
	double	MoveToBestNeighbor(double curCost);
};

