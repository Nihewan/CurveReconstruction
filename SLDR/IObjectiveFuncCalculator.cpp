#include "stdafx.h"
#include "IObjectiveFuncCalculator.h"


IObjectiveFuncCalculator::IObjectiveFuncCalculator(CP_Body *pBody)  : m_pBody(pBody), m_pWeights(NULL)
{
}


IObjectiveFuncCalculator::~IObjectiveFuncCalculator(void)
{
	if (m_pWeights)
		delete m_pWeights;
}
