#pragma once
#include "CP_Topology.h"
#include "CP_TopologyExt.h"

class CFaceCombination
{
public:
	CFaceCombination(void);
	~CFaceCombination(void);

public:
	static void		CombineFaces(CP_AssembledBody *pAsmbBody);
	static bool		ConstructLoop(CP_Half * baseHalf, CP_Half * subHalf);
	static double	GetCutCosValue(CP_Loop * , CP_Loop * , CP_Half * , CP_Half * );
	static void		CutHalfFromLoop(CP_Loop * loop, CP_Half * cutHalf);

private:
	static CObArray s_faceArr;		
};

