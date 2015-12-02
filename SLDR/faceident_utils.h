#pragma once
#include "CP_Topology.h"
#include "CP_TopologyExt.h"

namespace faceident_utils
{
	void		AddFace2FullBody(CP_AssembledBody *pAsmbBody, CObArray &faceArray);
	CP_Half *	GetHalfInSameLoop(CP_Edge * edge0, CP_Edge * edge1);
	void		ReverseLoop(CP_Loop * cutLoop);
};

