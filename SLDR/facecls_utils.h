#pragma once
#include "CP_Topology.h"
#include "CP_TopologyExt.h"
#include "propertiesnames.h"

namespace facecls_utils {
	double	CalcCurvedProbability(const CP_Face *pFace);
	int		GetParentFaceType(CP_Face *pFace);
};

struct ProbCmp{  
	bool operator()(CP_FaceExt *a, CP_FaceExt *b){  
		ASSERT(a->HasProperty(PROPERTY_NAME_PROBABILITY) && b->HasProperty(PROPERTY_NAME_PROBABILITY)); 
		int proba = a->GetIntegralProperties(PROPERTY_NAME_PROBABILITY);
		int probb = b->GetIntegralProperties(PROPERTY_NAME_PROBABILITY);
		return proba < probb;
	}  
};  

