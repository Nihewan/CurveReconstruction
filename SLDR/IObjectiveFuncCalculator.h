#pragma once
#include <vector>

class CP_Body;

class IObjectiveFuncCalculator
{
protected:
	CP_Body *m_pBody;
	std::vector<double> *m_pWeights;

public:
	IObjectiveFuncCalculator(CP_Body *pBody);
	virtual ~IObjectiveFuncCalculator(void);

public:
	virtual double CalcObjectiveFunc() = 0;
	virtual void SetValue(int index, double value) = 0;
};

