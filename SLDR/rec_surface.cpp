#include "stdafx.h"
#include "rec_surface.h"
#include "CP_Topology.h"
#include "curve_basic.h"
#include "surface_basic.h"
#include "topo_geometricoper.h"
#include "base_geometricoper.h"
#include "surface_triangulation.h"
#include "CLog.h"

namespace rec_surface {

	// Convert curve face's boundaries to NURBS
	void ReconstructNurbsSurf(CP_Face *pFace) {

		ASSERT(pFace->GetSurfaceType() != TYPE_SURFACE_PLANE);
		if (pFace->GetLoop(0)->GetHalfNumber() < 4) {
			return;
		}
		CP_Loop *pLoop = pFace->GetLoop(0);
		CP_NURBSSurface *pSurface = surface_basic::CreateNURBSSurf(pLoop);
		//surface_basic::NurbsSurfCoonsInterpolation(pSurface);

		delete pFace->m_surface;
		pFace->m_surface = pSurface;

	}

	void ReconstructCoonsSurf(CP_Face *pFace) {
		surface_basic::InitCoonsSurface(pFace);
	}


	void CreateMesh(CP_Face *pFace) {
		// Method of creating mesh for plane and curved face should be the same in the future

		// Plane
		if (pFace->GetSurfaceType() == TYPE_SURFACE_PLANE) {
			CP_GeneralPolygon2D poly;
			vector<double> depth;
			// ASSERT(pFace->GetLoopNumber()!= 2);
			InitGeneralPoly(poly, pFace, depth);
			CP_Mesh3D *pMesh = new CP_Mesh3D;
			surface_tri::Triangulation(poly, topo_geometric::IsClockwise(pFace->GetLoop(0)), *pMesh);
			for (int i = 0; i < depth.size(); ++i)
				poly.GetVert(i)->z() = depth[i];

			pFace->m_surface->m_pMesh = pMesh;
		}

		// Should be modified in the future
		else if (pFace->GetSurfaceType() == TYPE_SURFACE_NURBS || pFace->GetSurfaceType() == TYPE_SURFACE_COONS){
			CP_Surface3D *pSurf = pFace->m_surface;
			CP_Mesh3D *pMesh = new CP_Mesh3D;
			const int numSeg = 20;
			for (int ui = 0; ui <= numSeg; ++ui) {
				double u0 = (double) ui / numSeg;
				u0 = u0 > 1? 1 : u0;
				for (int vi = 0; vi <= numSeg; ++vi) {
					double v0 = (double) vi / numSeg;
					v0 = v0 > 1? 1 : v0;
					CP_MeshVertex3D *pVt0 = new CP_MeshVertex3D(pSurf->GetPoint(u0, v0));
					pMesh->AddVert(pVt0);
				}

				if (ui > 0) {
					int vtSize = pMesh->GetVertSize();
					for (int i = 1; i <= numSeg; ++i) {
						pMesh->AddTriangle(pMesh->GetVert(vtSize-numSeg-i-2), 
							pMesh->GetVert(vtSize-numSeg-i-1), 
							pMesh->GetVert(vtSize-i));

						pMesh->AddTriangle(pMesh->GetVert(vtSize-numSeg-i-2), 
							pMesh->GetVert(vtSize-i), 
							pMesh->GetVert(vtSize-i-1));
					}
				}
			}
			pSurf->m_pMesh = pMesh;
		}
	}

	void InitGeneralPoly(CP_GeneralPolygon2D &poly, CP_Face *pFace, vector<double> &depth) {
		const int numSeg = 10;

		// Each loop
		int vtLoopStart = 0;
		for (int i = 0; i < pFace->GetLoopNumber(); ++i) {
			CP_Loop *pLoop = pFace->GetLoop(i);

			// Each half
			for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
				CP_Half *pHalf = pLoop->GetHalf(j);
				CP_Edge *pEdge = pHalf->m_pEdge;
				CP_MeshVertex3D *pNewVt = new CP_MeshVertex3D(*pHalf->m_pStartVt->m_pPoint);
				depth.push_back(pNewVt->z());
				pNewVt->z() = 0;
				poly.AddVert(pNewVt);

				// Curve
				if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
					if (base_geometric::HasSameProjectionPt(&pEdge->m_pCurve3D->GetStartingPoint(), pHalf->m_pStartVt->m_pPoint, SAME_POINT_THRESHOLD)) {
						for (int k = 1; k < numSeg; ++k) {
							pNewVt = new CP_MeshVertex3D(pEdge->m_pCurve3D->GetPoint((double)k/numSeg));
							poly.AddVert(pNewVt);
							depth.push_back(pNewVt->z());
							pNewVt->z() = 0;
						}
					} else if (base_geometric::HasSameProjectionPt(&pEdge->m_pCurve3D->GetStartingPoint(), pHalf->m_pEndVt->m_pPoint, SAME_POINT_THRESHOLD)){
						for (int k = numSeg-1; k > 0; --k) {
							pNewVt = new CP_MeshVertex3D(pEdge->m_pCurve3D->GetPoint((double)k/numSeg));
							poly.AddVert(pNewVt);
							depth.push_back(pNewVt->z());
							pNewVt->z() = 0;
						}
					} else {/*
						CP_Point3D pt = pEdge->m_pCurve3D->GetStartingPoint();
						CLog::log("Curve start pt : %f, %f, %f", pt.m_x, pt.m_y, pt.m_z);
						pt = pEdge->m_pCurve3D->GetEndingPoint();
						CLog::log("Curve end pt : %f, %f, %f", pt.m_x, pt.m_y, pt.m_z);
						pt = *pHalf->m_pStartVt->m_pPoint;
						CLog::log("Half start pt : %f, %f, %f", pt.m_x, pt.m_y, pt.m_z);
						pt = *pHalf->m_pEndVt->m_pPoint;
						CLog::log("Half end pt : %f, %f, %f", pt.m_x, pt.m_y, pt.m_z);*/
						ASSERT(false);
					}
				}
			}

			// New CP_GeneralEdge
			for (int vi = vtLoopStart; vi < poly.GetVertSize(); ++vi) {
				int next = vi+1 != poly.GetVertSize()? vi+1: vtLoopStart;
				poly.AddEdge(poly.GetVert(vi), poly.GetVert(next));
			}
			vtLoopStart = poly.GetVertSize();
		}
	}
}