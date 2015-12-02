#include "stdafx.h"
#include "CHillClimb.h"
#include "CObjectiveFuncCalculator.h"
#include "CLog.h"
#include "CTimer.h"

#define MAX_VALUE 100000
#define MIN_COST 1


CHillClimb::CHillClimb(IObjectiveFuncCalculator *pObjectiveFunc, vector<double> &initValueArr, double initStep) 
	: m_pObjectiveFunc(pObjectiveFunc), m_value(initValueArr)
{
	m_initStep[0] = initStep;
	m_initStep[1] = -initStep;
}


CHillClimb::~CHillClimb(void)
{
}

void CHillClimb::HillClimb() {
	// CLog::log("start hill climb");
	// CTimer myTimer;
	// myTimer.Start();
	double minCost = m_pObjectiveFunc->CalcObjectiveFunc();
	double cost = 0;
	if (minCost < MIN_COST) {
		//CLog::log("end hill climb without execution");
		return;
	}
	while (m_initStep[0] >= 0.25) {
		//CLog::log("min cost = %f", minCost);
		cost = MoveToBestNeighbor(minCost);
		if (abs(cost - minCost) < 1e-6)
			break;
		if (cost > minCost) {
			m_initStep[0] /= 2;
			m_initStep[1] /= 2;
		}
		minCost = cost;
	}
	// CLog::log("end hill climb");
	// CLog::log("Hill climb time %f ms", myTimer.End());
}

double CHillClimb::MoveToBestNeighbor(double curCost) { 
	if (m_value.size() == 0) 
		return curCost;
	pair<int, double> minCostDirect;
	minCostDirect.first = 0;
	minCostDirect.second = m_initStep[0];
	double minCost = MAX_VALUE;
	for (unsigned int i = 0; i < m_value.size(); ++i) {
		for (int j = 0; j < 2; ++j) {
			m_pObjectiveFunc->SetValue(i, m_value[i] + m_initStep[j]);
			double cost = m_pObjectiveFunc->CalcObjectiveFunc();
			if (cost < minCost) {
				minCostDirect.first = i;
				minCostDirect.second = m_initStep[j];
				minCost = cost;
			}
			m_pObjectiveFunc->SetValue(i, m_value[i]);
		}
	}
	if (abs(minCost - curCost) < 1e-6)
		return curCost;
	m_value[minCostDirect.first] += minCostDirect.second;
	m_pObjectiveFunc->SetValue(minCostDirect.first, m_value[minCostDirect.first]);
	return minCost;
}