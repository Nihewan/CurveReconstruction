#pragma once
#include "CObjectFactory.h"
#include "CP_Curve.h"

namespace func_reg {
	void RegisterCopier() {
		CObjectFactory::RegisterCopier(TYPE_NAME_CURVE_ARC, &CopyT<CP_Arc>);
		CObjectFactory::RegisterCopier(TYPE_NAME_CURVE_NURBS, &CopyT<CP_Nurbs>);
		CObjectFactory::RegisterCopier(TYPE_NAME_CURVE_LINESEGMENT, &CopyT<CP_LineSegment3D>);
	}
}
