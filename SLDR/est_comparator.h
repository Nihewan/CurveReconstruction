#pragma once
#include "CP_TopologyExt.h"
#include "propertiesnames.h"

namespace est_comparator {

	struct LoopWeightDesComparator
	{
		bool operator() (CP_LoopExt *l0, CP_LoopExt *l1){
			return l0->GetIntegralProperties(PROPERTY_NAME_WEIGHT) <
				l1->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
		}
	};

	struct LoopWeightAscComparator 
	{
		bool operator() (CP_LoopExt *l0, CP_LoopExt *l1){
			return l0->GetIntegralProperties(PROPERTY_NAME_WEIGHT) >
				l1->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
		}
	};


	struct BodyWeightDesComparator {
		static bool CmpBody(CP_BodyExt* b0, CP_BodyExt *b1) {
			return b0->GetIntegralProperties(PROPERTY_NAME_WEIGHT) > 
				b1->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
		}
	};

	struct VertexWeightDesComparator
	{
		bool operator() (const CP_VertexExt *v0, const CP_VertexExt *v1) {
			return v0->GetIntegralProperties(PROPERTY_NAME_WEIGHT) <
				v1->GetIntegralProperties(PROPERTY_NAME_WEIGHT);
		}
	};

	int VertexWeightDesComp ( CP_VertexExt *const &v0,  CP_VertexExt *const &v1) ;
}

