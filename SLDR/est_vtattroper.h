#pragma once

#include "CP_Topology.h"
#include "CP_TopologyExt.h"



namespace est_vtattroper {
	CP_VertexExt* FindVertexVisited(CP_LoopExt *pLoop, int &outHalfIndex);	// Find visited vertex in loop. outHalfIndex is the index of half that started with visited vertex
	void	SetVertexDepthAndWeight(CP_VertexExt *pVt, double depth, int weight); // Set vertex's depth and weight and visited=1
	int		IsVisited(CP_VertexExt *pVt);
	int		GetVertexWeight(CP_VertexExt *pVt);

	// This property is just for test
	void	InitNotInQueue(CP_Loop *pLoop);
}