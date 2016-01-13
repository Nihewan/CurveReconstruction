
#include "stdafx.h"
#include "CP_TopologyExt.h"
#include "topo_geometricoper.h"
#include "base_geometricoper.h"
#include "propertiesnames.h"
#include <queue>

namespace rec_bodyassembly {
	void AdjustBodyLoopDirect(CP_LoopExt *pInitLoop, bool isClockwise);

	void InitAllLoopsUnvisited(CP_Body *pBody) {
		// Initialize loop unvisited property
		for (int i = 0; i < pBody->GetFaceNumber(); ++i) {
			CP_Face *pFace = pBody->GetFace(i);
			for (int j = 0; j < pFace->GetLoopNumber(); ++j) {
				CP_LoopExt *pLoop = dynamic_cast<CP_LoopExt*>(pFace->GetLoop(j));
				pLoop->SetProperties(PROPERTY_NAME_VISITED, 0);
			}
		}
	}

	void PushUnvisitedAdjLoops(CP_LoopExt *pLoop, vector<CP_LoopExt*> &vLoops) {
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_Half *pHalf = pLoop->GetHalf(i);
			CP_LoopExt *pTwinLoop = dynamic_cast<CP_LoopExt*>(pHalf->m_pTwin->m_pLoop);
			if (pTwinLoop->GetIntegralProperties(PROPERTY_NAME_VISITED) == 0) {
				pTwinLoop->SetProperties(PROPERTY_NAME_VISITED, 1);
				vLoops.push_back(pTwinLoop);
			}
		}
	}

	void PushUnvisitedAdjLoops(CP_LoopExt *pLoop, queue<CP_LoopExt*> &qLoops) {
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_Half *pHalf = pLoop->GetHalf(i);
			CP_LoopExt *pTwinLoop = dynamic_cast<CP_LoopExt*>(pHalf->m_pTwin->m_pLoop);
			if (pTwinLoop->GetIntegralProperties(PROPERTY_NAME_VISITED) == 0) {
				pTwinLoop->SetProperties(PROPERTY_NAME_VISITED, 1);
				qLoops.push(pTwinLoop);
			}
		}
	}

	void AdjustNestLoopDirect(queue<CP_LoopExt*> &qLoops) {
		CP_LoopExt *pLoop = qLoops.front();
		qLoops.pop();
		CP_Face *pFace = pLoop->m_pParentFace;
		int li = 0;
		for (; li < pFace->GetLoopNumber(); ++li) {
			if (dynamic_cast<CP_LoopExt*>(pFace->GetLoop(li)) == pLoop)
				break;
		}
		bool IsLoopClockwise = topo_geometric::IsClockwise(pLoop);
		// topo_geometric::OutputLoop(pLoop);
		for (int i = li+1; i < pFace->GetLoopNumber(); ++i) 
		{
			if (topo_geometric::IsLoopInLoop(pFace->GetLoop(i), pLoop))
				AdjustBodyLoopDirect(dynamic_cast<CP_LoopExt*>(pFace->GetLoop(i)), !IsLoopClockwise);
		}
		for (int i = li-1; i >= 0; --i) 
		{
			if (topo_geometric::IsLoopInLoop(pLoop, pFace->GetLoop(i)))
				AdjustBodyLoopDirect(dynamic_cast<CP_LoopExt*>(pFace->GetLoop(i)), !IsLoopClockwise);
		}
		PushUnvisitedAdjLoops(pLoop, qLoops);
	}

	void AdjustBodyLoopDirect(CP_LoopExt *pInitLoop, bool isClockwise) {
		if (pInitLoop->GetIntegralProperties(PROPERTY_NAME_VISITED) == 1)
			return;

		CP_LoopExt *pLoop = NULL;
		bool isLoopClockwise = topo_geometric::IsClockwise(pInitLoop);
		pInitLoop->SetProperties(PROPERTY_NAME_VISITED, 1);
		queue<CP_LoopExt*> qLoops;
		qLoops.push(pInitLoop);
		if (isClockwise && isLoopClockwise || !isClockwise && !isLoopClockwise) {
			while (!qLoops.empty()) {
				AdjustNestLoopDirect(qLoops);
			}
		}

		else {
			int index = 0;
			vector<CP_LoopExt*> vLoops;
			vLoops.push_back(pInitLoop);
			while (index != vLoops.size()) {
				CP_LoopExt *pLoop = vLoops[index++];
				PushUnvisitedAdjLoops(pLoop, vLoops);
			}
			while (!vLoops.empty()) {
				CP_LoopExt *pLoop = vLoops.back();
				vLoops.pop_back();
				topo_geometric::ReverseLoop(pLoop);
				pLoop->SetProperties(PROPERTY_NAME_VISITED, 0);
			}
			pInitLoop->SetProperties(PROPERTY_NAME_VISITED, 1);
			while (!qLoops.empty()) {
				AdjustNestLoopDirect(qLoops);
			}
		}
	}



	CP_Point3D GetCenter(vector<CP_Point3D> &poly) {
		CP_Point3D pt;
		for (unsigned int i = 0; i < poly.size(); ++i)
			pt += poly[i];
		return pt / poly.size();
	}



	void AdjustBodyLoopDirect(CP_AssembledBody *pAsmbBody) {
		CP_Body *pBody = pAsmbBody->GetOriginalBody();
		CP_Face *pFace = pBody->GetFace(0);
		CP_Point3D pt;
		vector<CP_Point3D> poly;

		if (pFace->GetSurfaceType() == TYPE_SURFACE_PLANE)
		{
			topo_geometric::Polygonization3D(pFace->GetLoop(0), poly);
			pt = GetCenter(poly);
		}
		else
			pt = pFace->m_surface->GetPoint(0.5, 0.5);
		pair<double, int> minDepth;
		minDepth.first = pt.m_z;
		minDepth.second = 0;
		pt.m_z = 0;

		for (int i = 1; i < pBody->GetFaceNumber(); ++i) {
			CP_Face *pFace = pBody->GetFace(i);
			poly.clear();
			topo_geometric::Polygonization2D(pFace->GetLoop(0), poly);
			if (!topo_geometric::IsPtInPolygon2D(pt, poly))
				continue;
			double depth = pFace->m_surface->GetPointDepth(pt);
			if (depth > minDepth.first) {
				minDepth.first = depth;
				minDepth.second = i;
			}
		}

		CP_LoopExt *pInitLoop = dynamic_cast<CP_LoopExt*>(pBody->GetFace(minDepth.second)->GetLoop(0));
		InitAllLoopsUnvisited(pBody);
		AdjustBodyLoopDirect(pInitLoop, false);

	}
}