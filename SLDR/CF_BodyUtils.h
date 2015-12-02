#pragma once

#include "Cp_Topology.h"
#include "CP_TopologyExt.h"
#include "CBoundBox2D.h"

class CF_BodyUtils
{
private:
	CF_BodyUtils(void);

public:
	~CF_BodyUtils(void);

public:
	static bool ValidateBody(CP_Body *pBody); 
	static void DrawWireframeBody(CP_Body *pBody);
	static void DrawSolidBody(CP_Body *pBody);
	static void DrawEdges(CP_Body *pBody);
	static void DrawSelectedHighLight(CP_Body *pBody, CString strSelectedItem);
	static void	DrawSelectedHighLightEdge(CP_Body *pBody, int edgeIndex);
	static void DrawUCS(CP_Body *pBody, CP_Point3D *pCenter, double scale);
};

