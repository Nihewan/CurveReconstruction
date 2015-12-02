#pragma once
#include "CP_Topology.h"

class CP_AssembledBody;

namespace rec_utils
{
	void ReconstructBodyPlaneFaceNormal(CP_Body *pBody);
	void AssignVtDepth(CP_Body *pBody, vector<double> &depthArr); // Assign vertex's depth

};

