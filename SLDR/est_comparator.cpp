#include "stdafx.h"
#include "est_comparator.h"

namespace est_comparator {
	int VertexWeightDesComp ( CP_VertexExt *const &v0,  CP_VertexExt *const &v1)
	{
		if (v0->GetIntegralProperties(PROPERTY_NAME_WEIGHT) <
			v1->GetIntegralProperties(PROPERTY_NAME_WEIGHT))
			return -1;
		return 1;
	}
}