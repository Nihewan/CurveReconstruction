#include "stdafx.h"
#include "surface_basic.h"
#include "base_geometricoper.h"
#include "curve_basic.h"
#include "topo_geometricoper.h"
#include <memory>
#include "CLog.h"
#include <algorithm>

namespace surface_basic {

	/************************************************************************/
	/* Coons face                                                           */
	/************************************************************************/
	void Init4LineSegPatch(vector<CP_LineSegment3D> &vLineSeg, 
		const CP_Point3D &pt0, const CP_Point3D &pt1, 
		const CP_Point3D &pt2, const CP_Point3D &pt3) 
	{
		vLineSeg.push_back(CP_LineSegment3D(pt0, pt1));
		vLineSeg.push_back(CP_LineSegment3D(pt1, pt3));
		vLineSeg.push_back(CP_LineSegment3D(pt3, pt2));
		vLineSeg.push_back(CP_LineSegment3D(pt0, pt2));
	}

	void AssignCoonsFaceEndPts(CP_CoonsSurface *pSurface, CP_Half *pHalf0, CP_Half *pHalf1) {
		pSurface->m_pEndPts[0] = pHalf0->m_pStartVt->m_pPoint;
		pSurface->m_pEndPts[2] = pHalf0->m_pEndVt->m_pPoint;
		pSurface->m_pEndPts[1] = pHalf1->m_pEndVt->m_pPoint;
		pSurface->m_pEndPts[3] = pHalf1->m_pStartVt->m_pPoint;
	}

	void AssignCoonsBoundary(CP_CoonsSurface *pSurface, CP_Half *pHalf, int bi, int pi) {
		pSurface->m_boundaries[bi] = pHalf->m_pEdge->m_pCurve3D;
		if (!base_geometric::HasSameProjectionPt(&pSurface->m_boundaries[bi]->GetStartingPoint(),
			pSurface->m_pEndPts[pi], SAME_POINT_THRESHOLD))
			pSurface->m_sameDirection[bi] = false;
		else
			pSurface->m_sameDirection[bi] = true;
	}

	bool CompareCurveCurvature(pair<double, int> a,pair<double, int> b)
	{
		return a.first < b.first; //升序排列，如果改为return a>b，则为降序
	}

	void Init3EdgesCoonsSurface(CP_Face *pFace) {
		CP_Loop *pLoop = pFace->GetLoop(0);
		ASSERT(pFace->GetLoop(0)->GetHalfNumber() == 3);

		CP_Curve3D *pCurves[3] = {
			pLoop->GetHalf(0)->m_pEdge->m_pCurve3D,
			pLoop->GetHalf(1)->m_pEdge->m_pCurve3D,
			pLoop->GetHalf(2)->m_pEdge->m_pCurve3D
		};
		vector<pair<double, int>> vCurves;
		vCurves.push_back(pair<double, int>(curve_basic::CalcAverageCurvature(pCurves[0]), 0));
		vCurves.push_back(pair<double, int>(curve_basic::CalcAverageCurvature(pCurves[1]), 1));
		vCurves.push_back(pair<double, int>(curve_basic::CalcAverageCurvature(pCurves[2]), 2));

		sort(vCurves.begin(), vCurves.end(), CompareCurveCurvature);
		
		int maxCurIndex = vCurves[2].second;
		int pairCurve[2] = {(maxCurIndex+1)%3, (maxCurIndex+2)%3};

		CP_CoonsSurface *pSurface = (CP_CoonsSurface *)pFace->m_surface;

		CP_Half *pHalf0 = pLoop->GetHalf(pairCurve[0]);
		CP_Half *pHalf1 = pLoop->GetHalf(pairCurve[1]);
		CP_Half *pHalf2 = pLoop->GetHalf(maxCurIndex);

		AssignCoonsFaceEndPts(pSurface, pHalf0, pHalf1);

		AssignCoonsBoundary(pSurface, pHalf0, 0, 0);
		AssignCoonsBoundary(pSurface, pHalf1, 1, 1);
		AssignCoonsBoundary(pSurface, pHalf2, 2, 0);
	}

	void Init4EdgesCoonsSurface(CP_Face *pFace) {
		CP_CoonsSurface *pSurface = (CP_CoonsSurface *)pFace->m_surface;
		CP_Loop *pLoop = pFace->GetLoop(0);
		CP_Half *pHalf0 = pLoop->GetHalf(0);
		CP_Half *pHalf2 = pLoop->GetHalf(2);
		AssignCoonsFaceEndPts(pSurface, pHalf0, pHalf2);

		// Adjust the direction of boundary
		AssignCoonsBoundary(pSurface, pHalf0, 0, 0);
		AssignCoonsBoundary(pSurface, pHalf2, 1, 1);
		AssignCoonsBoundary(pSurface, pLoop->GetHalf(3), 2, 0);
		AssignCoonsBoundary(pSurface, pLoop->GetHalf(1), 3, 2);

	}

	void InitCoonsSurface(CP_Face *pFace) {
		CP_Loop *pLoop = pFace->GetLoop(0);
		if (pLoop->GetHalfNumber() == 3)
			Init3EdgesCoonsSurface(pFace);
		else if (pLoop->GetHalfNumber() == 4)
			Init4EdgesCoonsSurface(pFace);
		else 
			throw exception("Face is not 3 or 4 edges.");
	}



	/************************************************************************/
	/* NURBS face                                                           */
	/************************************************************************/

	void NurbsSurfCoonsInterpolation(CP_NURBSSurface *pSurface) {
		int n = pSurface->GetUCtrlNum()-1, m = pSurface->GetVCtrlNum()-1;
		for (int i = 1; i < n; ++i) {
			for (int j = 1; j < m; ++j) {
				double u = (double)i / n;
				double v = (double)j / m;
				CP_Point3D pt = (1-u) * pSurface->GetCtrlPoint(0, j) + u * pSurface->GetCtrlPoint(n, j);
					+ (1-v) * pSurface->GetCtrlPoint(i, 0) + v * pSurface->GetCtrlPoint(i, m) 
					+ (-(1-u) * v * pSurface->GetCtrlPoint(0, m)) + (- u * (1-v) * pSurface->GetCtrlPoint(n, 0))
					+ (- (1-u) * (1-v) * pSurface->GetCtrlPoint(0, 0)) + (- u * v * pSurface->GetCtrlPoint(n, m));
				double weight = (1-u) * pSurface->GetWeight(0, j) + u * pSurface->GetWeight(n, j);
					+ (1-v) * pSurface->GetWeight(i, 0) + v * pSurface->GetWeight(i, m) 
					+ (-(1-u) * v * pSurface->GetWeight(0, m)) + (- u * (1-v) * pSurface->GetWeight(n, 0))
					+ (- (1-u) * (1-v) * pSurface->GetWeight(0, 0)) + (- u * v * pSurface->GetWeight(n, m));
				pSurface->SetCtrlPoint(i, j, pt, weight);
			}
		}
	}



	void RedirectNurbsBoundary(vector<CP_Nurbs*> vBound, CP_Loop *pLoop) {
		CP_Half *pHalf0 = pLoop->GetHalf(0);
		CP_Half *pHalf1 = pLoop->GetHalf(1);
		CP_Half *pHalf2 = pLoop->GetHalf(2);
		CP_Half *pHalf3 = pLoop->GetHalf(3);

		curve_basic::AdjustNurbsDirect(vBound[0], pHalf0->m_pStartVt->m_pPoint);
		curve_basic::AdjustNurbsDirect(vBound[1], pHalf1->m_pStartVt->m_pPoint);
		curve_basic::AdjustNurbsDirect(vBound[2], pHalf2->m_pEndVt->m_pPoint);
		curve_basic::AdjustNurbsDirect(vBound[3], pHalf0->m_pStartVt->m_pPoint);
	}


	CP_NURBSSurface* CreateNURBSSurf(CP_Loop *pLoop) {
		vector<CP_Nurbs*> vBound(pLoop->GetHalfNumber());
		ASSERT(pLoop->GetHalfNumber() == 4);
		for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
			CP_Half *pHalf = pLoop->GetHalf(i);
			CP_Edge *pEdge = pHalf->m_pEdge;
			vBound[i] = new CP_Nurbs();
			if (pEdge->GetCurveType() == TYPE_CURVE_LINESEGMENT) {
				curve_basic::LineSeg2Nurbs(dynamic_cast<CP_LineSegment3D *>(pEdge->m_pCurve3D), vBound[i]);
			} else if (pEdge->GetCurveType() == TYPE_CURVE_ARC) {
				curve_basic::Arc2Nurbs(dynamic_cast<CP_Arc *>(pEdge->m_pCurve3D), vBound[i]);
			} else {
				*vBound[i] = *(pEdge->m_pCurve3D);
			}
		}

		RedirectNurbsBoundary(vBound, pLoop);

		// Interpolate section curves
		vector<shared_ptr<CP_Nurbs> > vSecCurs = InterpolateNurbsSectionCurves(vBound);
		CP_Nurbs **sectionCurs = new CP_Nurbs*[vSecCurs.size()];
		for (int i = 0; i < vSecCurs.size(); ++i)
			sectionCurs[i] = vSecCurs[i].get();

		// Loft
		int uCtrlNum = 0;
		int vCtrlNum = 0;
		double *knotU, *knotV;
		CP_Point3D **ctrlPts;
		double **weights;
		GetLoftPnts(sectionCurs, vSecCurs.size(), ctrlPts, weights, uCtrlNum, vCtrlNum, knotU, knotV);

		// Create NURBS surface
		int orderU = 4;
		int orderV = sectionCurs[0]->GetOrder();
		CP_NURBSSurface *pNurbsSurf = new CP_NURBSSurface(orderU, orderV, uCtrlNum, vCtrlNum);
		for (int i = 0; i < uCtrlNum; ++i) {
			for (int j = 0; j < vCtrlNum; ++j) {
				pNurbsSurf->SetCtrlPoint(i, j, ctrlPts[i][j], weights[i][j]);
			}
		}
		for (int i = 0; i < uCtrlNum + orderU; ++i) {
			pNurbsSurf->AddUKnot(knotU[i]);
		}
		for (int i = 0; i < vCtrlNum + orderV; ++i) {
			pNurbsSurf->AddVKnot(knotV[i]);
		}

		// Release memory
		for (int i = 0; i < 4; ++i)
			delete vBound[i];
		for (int i = 0; i < uCtrlNum; ++i) {
			delete []weights[i];
			delete []ctrlPts[i];
		}
		delete []ctrlPts;
		delete []weights;
		delete []sectionCurs;
		delete []knotU;
		delete []knotV;

		return pNurbsSurf;
	}

	vector<shared_ptr<CP_Nurbs> > InterpolateNurbsSectionCurves(vector<CP_Nurbs*> vBound) {
		int maxCosIndex = 0;
		double maxCos = 0.0;

		// Find parallel curves in opposite side
		for (int i = 0; i < 2; ++i) {
			CP_Curve3D *pCurve0 = vBound[i];
			CP_Curve3D *pCurve1 = vBound[i+2];
			double cosValue = topo_geometric::GetCosValue3D(pCurve0, pCurve1);
			if (cosValue > maxCos) {
				maxCos = cosValue;
				maxCosIndex = i;
			}
		}
		
		pair<CP_Nurbs *, CP_Nurbs *> initSecCur;
		pair<CP_Nurbs *, CP_Nurbs *> boundCur;
		initSecCur.first = vBound[maxCosIndex];
		initSecCur.second = vBound[maxCosIndex+2];
		boundCur.first = vBound[(maxCosIndex+3)%4];
		boundCur.second = vBound[maxCosIndex+1];

		// Make two curves isoparametric 
		curve_basic::Put2NurbsCurvesCompatible(initSecCur.first, initSecCur.second);
		
		// Reverse boundary for boundary point calculation.
		//		--2->
		//		^	^
		//		3   1
		//		|	|
		//		--0->
		if (maxCosIndex == 1) {
			curve_basic::NurbsReverse(boundCur.first);
			curve_basic::NurbsReverse(boundCur.second);
		}

		// Interpolate section curves
		int nSection = 20;
		vector<shared_ptr<CP_Nurbs> > vSecCurs;
		vSecCurs.push_back(shared_ptr<CP_Nurbs>(new CP_Nurbs(*initSecCur.first)));
		for (int i = 1; i < nSection; ++i) {
			vSecCurs.push_back(InterpolateSectionCurveAt(initSecCur.first, initSecCur.second, (double)i / nSection, boundCur.first, boundCur.second));
		}
		vSecCurs.push_back(shared_ptr<CP_Nurbs>(new CP_Nurbs(*initSecCur.second)));

		return vSecCurs;
	}

	shared_ptr<CP_Nurbs> InterpolateSectionCurveAt(CP_Nurbs *pNurbs0, CP_Nurbs *pNurbs1, double param, CP_Curve3D *pBoundary0, CP_Curve3D *pBoundary1) {
		shared_ptr<CP_Nurbs> secCur(new CP_Nurbs);
		curve_basic::LinearInterpolateNurbs(pNurbs0, pNurbs1, 1-param, secCur.get());
		CP_Point3D attachPt0 = pBoundary0->GetPoint(param);
		CP_Point3D attachPt1 = pBoundary1->GetPoint(param);

		CP_Point3D startPt = secCur->GetStartingPoint();
		curve_basic::NurbsTranslate(secCur.get(), &attachPt0, &startPt);
		curve_basic::NurbsTransformToAttach(secCur.get(), &attachPt1);
		return secCur;
	}

	// Loft by section curves
	// Input:
	//		pInSecNurbs - section curves
	//		inSectionNum - number of section curves 
	//		err - 
	//		bebound - 
	// Output: 
	//		outLoftCtrlPnt - control points of the loft surface
	//		outLoftWeight - weight value of each control point
	//		outLoftCtrlNum - number of control points in direction of spine(U)
	//		outSecCtrlNum - number of control points in direction of section curve(V)
	//		outKnotU - knot vector of direction of spine
	//		outKnotV - knot vector of direction of section curve
	void GetLoftPnts(CP_Nurbs** pInSecNurbs, int inSectionNum,
						CP_Point3D**& outLoftCtrlPnt, double **&outLoftWeight, int &outLoftCtrlNum, 
						int &outSecCtrlNum, double*& outKnotU, double*& outKnotV, double err /* = 0 */, bool bebound /* = false */)
	{
		int			i, j, orderV, orderU = 4;
		CP_Point3D	**pnt;
		double		**weight;
	
		// Curves Compatible
		curve_basic::PutNNurbsCurvesCompatible(pInSecNurbs, inSectionNum);	
		
		/*
		// Reduce control points
		if(err > TOLER)
		{
			if (bebound && section_num>2)
			{
				CP_Nurbs **curve = new CP_Nurbs*[section_num-2];
				for (int i=0; i<section_num-2; i++)
					curve[i] = (CP_Nurbs*)pSecNurbs[i+1]->Copy();
				ReduceCtrlPts(curve, section_num-2, err);
				for (int i=0; i<section_num-2; i++)
				{
					delete pSecNurbs[i+1];
					pSecNurbs[i+1] = (CP_Nurbs*)curve[i]->Copy();
					delete curve[i];
				}
				delete []curve;
			}
			else
				ReduceCtrlPts(pSecNurbs, section_num, err);
		}
		*/

		outSecCtrlNum = pInSecNurbs[0]->GetCtrlNumber();
		orderV      = pInSecNurbs[0]->GetOrder();

		pnt = new CP_Point3D*[outSecCtrlNum];	
		weight = new double*[outSecCtrlNum];	
		outLoftCtrlPnt= new CP_Point3D*[inSectionNum+(orderU-2)];
		outLoftWeight= new double*[inSectionNum+(orderU-2)];
	
		for(i=0;i<outSecCtrlNum;i++)				   
		{
			pnt[i]=new CP_Point3D[inSectionNum];
			weight[i] = new double[inSectionNum];
		}
		for(i=0;i<inSectionNum+(orderU-2);i++)				   
		{   
			outLoftCtrlPnt[i] = new CP_Point3D[outSecCtrlNum];
			outLoftWeight[i] = new double [outSecCtrlNum];
		}
	
		for(i=0;i<outSecCtrlNum;i++) {
			for(j=0; j<inSectionNum; j++)
			{
				pnt[i][j] = *pInSecNurbs[j]->GetCtrlPoint(i);
				weight[i][j] = pInSecNurbs[j]->GetWeightValue(i);
			}
		}

		outKnotV = new double[outSecCtrlNum+orderV];
		for(i=0;i<outSecCtrlNum+orderV; i++)
			outKnotV[i] = pInSecNurbs[0]->GetKnotValue(i);
	
		outKnotU=new double[inSectionNum + 2*orderU-2];	

		curve_basic::CalKnotU(inSectionNum, outSecCtrlNum, pnt, orderU,  outKnotU);

		// 纵向skining
		CP_Vector3D sT, eT;
		for(i=0;i<outSecCtrlNum;i++)
		{
			curve_basic::GetIntpoEndTangV(inSectionNum, pnt[i], sT, eT);  
			sT.Normalize();
			eT.Normalize();

			CP_Nurbs *pNurbs = curve_basic::InterpolateNurbsCurve(inSectionNum, pnt[i],
											weight[i], orderU, sT, eT, outKnotU);

			for(j=0; j<inSectionNum+orderU-2; j++)
			{
				outLoftCtrlPnt[j][i] = *pNurbs->GetCtrlPoint(j);
				outLoftWeight[j][i]  = pNurbs->GetWeightValue(j);
			}
			delete pNurbs;
		}	 

		outLoftCtrlNum = inSectionNum+orderU-2;
	
		///delete
		for(i=0;i<outSecCtrlNum;i++)
		{
			delete []pnt[i];
			delete []weight[i];
		}
		delete[] pnt;
		delete []weight;
	}


}