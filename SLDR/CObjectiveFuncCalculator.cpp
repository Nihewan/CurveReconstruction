#include "stdafx.h"
#include "CObjectiveFuncCalculator.h"
#include "topo_geometricoper.h"
#include "rec_utils.h"
#include "CLog.h"
#include "base_geometricoper.h"
#include "propertiesnames.h"
#include "rec_curve.h"

/************************************************************************/
/* CLineParal2DInfo                                                     */
/************************************************************************/
void CLineParal2DInfo::InitLineParal2D(CP_Body *pBody, vector<CLineParal2DInfo> &vLineParal) {
	int edgeSize = pBody->GetEdgeNumber();
	for (int i = 0; i < edgeSize; ++i) {
		CP_Edge *pEdgeI = pBody->GetEdge(i);
		for (int j = i+1; j < edgeSize; ++j) {
			CP_Edge *pEdgeJ = pBody->GetEdge(j);
			double cosVal = topo_geometric::GetCosValue2D(pEdgeI, pEdgeJ);
			if (cosVal > PARALLEL_COS_TOLER)
				vLineParal.push_back(CLineParal2DInfo(i, j, cosVal));
		}
	}
}

/************************************************************************/
/* CFaceBestFit                                                         */
/************************************************************************/

// Least-square fit
double CFaceBestFit::BestFit(vector<CP_Point3D *> &vPts, shared_ptr<CFaceSumData> &faceSum, CFaceBestFit &face) {
	double zSum = 0;
	double xzSum = 0;
	double yzSum = 0;
	double z2Sum = 0;
	int n = vPts.size();
	for (int i = 0; i < n; ++i) {
		zSum += vPts[i]->m_z;
		xzSum += vPts[i]->m_x * vPts[i]->m_z;
		yzSum += vPts[i]->m_y * vPts[i]->m_z;
		z2Sum += pow(vPts[i]->m_z, 2);
	}

	double z1 = n * xzSum - faceSum->m_xSum * zSum;
	double z2 = n * yzSum - faceSum->m_ySum * zSum;

	double den = (faceSum->m_a1 * faceSum->m_b2 - faceSum->m_a2 * faceSum->m_b1);
	double &A = face.m_A = -(z1 * faceSum->m_b2 - z2 * faceSum->m_b1) / den;
	double &B = face.m_B = -(faceSum->m_a1 * z2 - faceSum->m_a2 * z1) / den;
	double &C = face.m_C = 1;
	double &D = face.m_D = -(zSum + A * faceSum->m_xSum + B * faceSum->m_ySum) / n;
	double len = sqrt(A * A + B * B + 1);
	A /= len;
	B /= len;
	C /= len;
	D /= len;

	double cost = 0;
	for (int i = 0; i < n; ++i) {
		cost += abs(A * vPts[i]->m_x + B * vPts[i]->m_y + C * vPts[i]->m_z + D);
	}

	return cost;
}

// Input : 
//		index - face index in the body
//		vPts - points to be fit
CFaceSumData* CFaceBestFit::createFaceXYSum(int index, vector<CP_Point3D *> &vPts) {
	double xSum = 0;
	double ySum = 0;
	double xySum = 0;
	double x2Sum = 0;
	double y2Sum = 0;
	int n = vPts.size();
	for (int i = 0; i < n; ++i) {
		xSum += vPts[i]->m_x;
		ySum += vPts[i]->m_y;
		xySum += vPts[i]->m_x * vPts[i]->m_y;
		x2Sum += pow(vPts[i]->m_x, 2);
		y2Sum += pow(vPts[i]->m_y, 2);
	}
	double a1 = n * x2Sum - xSum * xSum;
	double a2 = n * xySum - xSum * ySum;
	double b1 = n * xySum - xSum * ySum;
	double b2 = n * y2Sum - ySum * ySum;
	return new CFaceSumData(index, xSum, ySum, a1, a2, b1, b2);
}

/************************************************************************/
/* CLineParallelismCalctor                                              */
/************************************************************************/
#define INIT_2D_PARAL_VALUE -2

CLineParallelism::CLineParallelism(CP_Body *pBody, double paramLineParal, vector<CLineParal2DInfo> &lineParal2D)
	: IObjectiveFuncCalculator(pBody), m_paramLineParal(paramLineParal), m_lineParal2D(lineParal2D)
{
	m_pWeights = new std::vector<double>(lineParal2D.size(), 1.0);
}

CLineParallelism::~CLineParallelism() {
}

double CLineParallelism::CalcObjectiveFunc()  {
	double cost = 0;
	for (unsigned int  i = 0 ; i < m_lineParal2D.size(); ++ i) {
		int index0 = m_lineParal2D[i].m_edge0Index;
		int index1 = m_lineParal2D[i].m_edge1Index;
		CP_Edge *pEdgeI = m_pBody->GetEdge(index0);
		CP_Edge *pEdgeJ = m_pBody->GetEdge(index1);
		cost +=  CalcLineParal(pEdgeI, pEdgeJ, m_lineParal2D.at(i).m_cosValue2D);
		// m_pWeights->at(i) = CalcWeight(pEdgeI, pEdgeJ);
		//CLog::log("%f", m_pWeights->at(i));
	}
	return cost;
}

double CLineParallelism::CalcWeight(CP_Edge *pEdgeI, CP_Edge *pEdgeJ) {
	double r3D = topo_geometric::GetCosValue3D(pEdgeI, pEdgeJ);
	double ang = base_geometric::GetACOS(r3D);
	//CLog::log("ang = %f", ang);
	return exp( - ang * ang / 0.25);
}

double CLineParallelism::CalcLineParal(CP_Edge *pEdgeI, CP_Edge *pEdgeJ, double cosVal2D) {
	double w = 1.0 / (1+exp(-m_paramLineParal * (cosVal2D - 0.8)));  
	double r3D = topo_geometric::GetCosValue3D(pEdgeI, pEdgeJ);
	double ang = base_geometric::GetACOS(r3D);
	//CLog::log("w = %f, r3d = %f, cost = %f", w, r3D, w * (1-r3D) * (1-r3D));
	// return w * (1-r3D) * (1-r3D);
	return w * ang * ang;
}



/************************************************************************/
/* CFacePlanarity                                                       */
/************************************************************************/
CFacePlanarity::CFacePlanarity(CP_Body *pBody) : IObjectiveFuncCalculator(pBody)
{
	InitFaceSum();
}

CFacePlanarity::~CFacePlanarity() {

}

double CFacePlanarity::CalcObjectiveFunc() {
	double cost = 0;
	vector<CP_Point3D *> vPts;
	for (unsigned int i = 0; i < m_faceVtXYSums.size(); ++i) {
		CP_Loop *pLoop = m_pBody->GetFace(m_faceVtXYSums[i]->m_index)->GetLoop(0);
		for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
			CP_Half *pHalf = pLoop->GetHalf(j);
			vPts.push_back(pHalf->m_pStartVt->m_pPoint);
		}
		cost += CFaceBestFit::BestFit(vPts, m_faceVtXYSums[i], CFaceBestFit());
		vPts.clear();
	}
	return cost;
}


void CFacePlanarity::InitFaceSum() {
	vector<CP_Point3D *> vPts;
	for (int i = 0; i < m_pBody->GetFaceNumber(); ++i) {
		if (m_pBody->GetFace(i)->GetSurfaceType() == TYPE_SURFACE_PLANE) {
			CP_Loop *pLoop = m_pBody->GetFace(i)->GetLoop(0);
			ASSERT (pLoop->GetHalfNumber() >= 2);
			if (pLoop->GetHalfNumber() > 2) {
				for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
					CP_Half *pHalf = pLoop->GetHalf(j);
					vPts.push_back(pHalf->m_pStartVt->m_pPoint);
				}
				m_faceVtXYSums.push_back(shared_ptr<CFaceSumData>(CFaceBestFit::createFaceXYSum(i, vPts)));
				vPts.clear();
			}
		}
	}
}


/************************************************************************/
/* CIsometry                                                            */
/************************************************************************/
CIsometry::CIsometry(CP_Body *pBody, vector<CLineParal2DInfo> &vLineParal) 
	: IObjectiveFuncCalculator(pBody), m_lineParal2D(vLineParal)
{
	InitEdgeLen2D();
	m_pWeights = new std::vector<double>(vLineParal.size(), 1.0);
}

CIsometry::~CIsometry() {

}

double CIsometry::CalcObjectiveFunc() {

	vector<double> vRatio;
	double variance = 0;
	for (unsigned int i = 0; i < m_lineParal2D.size(); ++i) {
		CP_Edge *pEdge0 = m_pBody->GetEdge(m_lineParal2D[i].m_edge0Index);
		CP_Edge *pEdge1 = m_pBody->GetEdge(m_lineParal2D[i].m_edge1Index);

	//	if (pEdge0->GetCurveType() == TYPE_CURVE_LINESEGMENT &&
	//		pEdge1->GetCurveType() == TYPE_CURVE_LINESEGMENT)
		{
			double len3D = pEdge0->m_pCurve3D->GetLength();
			double len2D = m_edgeLen2D[m_lineParal2D[i].m_edge0Index];
			double r0 = len3D / len2D;

			len3D = pEdge1->m_pCurve3D->GetLength();
			len2D = m_edgeLen2D[m_lineParal2D[i].m_edge1Index];
			double r1 = len3D / len2D;
			double average = (r0 + r1) / 2;
			variance += m_pWeights->at(i) * (pow(r0 - average, 2) + pow(r1 - average, 2));
			m_pWeights->at(i) = CalcWeight(r0, r1);
		}
	}
	return variance;
}

double CIsometry::CalcWeight(double r0, double r1) {
	// sigma = 0.3
	return exp(-pow(abs(r1 - r0), 2.0) / 0.18);
}

void CIsometry::InitEdgeLen2D() {
	for (int i = 0; i < m_pBody->GetEdgeNumber(); ++i) {
		m_edgeLen2D.push_back(base_geometric::GetCurveLen2D(m_pBody->GetEdge(i)->m_pCurve3D));
	}
}

/************************************************************************/
/* CMSDA                                                                */
/************************************************************************/

CMSDA::CMSDA(CP_Body *pBody) : IObjectiveFuncCalculator(pBody)
{
}

CMSDA::~CMSDA()
{
}

double CMSDA::CalcObjectiveFunc() {
	vector<double> vAng;
	double variance = 0;

	double sum = 0;
	for (int i = 0; i < m_pBody->GetVertexNumber(); ++i) {
		CP_Vertex *pVt = m_pBody->GetVertex(i);
		for (int j = 0; j < pVt->GetOutHalfNumber(); ++j) {
			CP_Half *pHalf0 = pVt->GetOutHalf(j);
			CP_Half *pHalf1 = pVt->GetOutHalf((j+1) % pVt->GetOutHalfNumber());

			// Red line of cone
			if (((CP_EdgeExt *)pHalf1->m_pEdge)->HasProperty(PROPERTY_NAME_ADDITIONAL_LINE)) {
				j ++;
				continue;
			}

			CP_Curve3D *pCurve0 = pHalf0->m_pEdge->m_pCurve3D;
			CP_Curve3D *pCurve1 = pHalf1->m_pEdge->m_pCurve3D;
			CP_Point3D midPt0 = pCurve0->GetPoint(0.5);
			CP_Point3D midPt1 = pCurve1->GetPoint(0.5);
			CP_Vector3D v0(*pVt->m_pPoint - midPt0);
			CP_Vector3D v1(*pVt->m_pPoint - midPt1);
			double ang = base_geometric::GetIncludedAngle(v0, v1);
			vAng.push_back(ang);
			sum += ang;
		}
	}
	
/*
	for (int i = 0; i < m_pBody->GetEdgeNumber(); ++i) {
		CP_Edge *pEdge = static_cast<CP_Edge*>(m_pBody->GetEdge(i));
		if (pEdge->GetCurveType() == !TYPE_CURVE_LINESEGMENT) {
		CP_Point3D midPt = pEdge->m_pCurve3D->GetPoint(0.5);
		CP_Point3D *pStr = pEdge->m_pStartVt->m_pPoint;
		CP_Point3D *pEnd = pEdge->m_pEndVt->m_pPoint;
		double ang = base_geometric::GetMinIncludedAngle(*pStr - midPt, *pEnd - midPt);
		vAng.push_back(ang);
		sum += ang;
		}
	}*/
	double average = sum / vAng.size();
	for (unsigned int k = 0; k < vAng.size(); ++k)
		variance += pow(vAng[k] - average, 2);
	vAng.clear();
	return variance;
}

/************************************************************************/
/* CConcurvity                                                          */
/************************************************************************/

CConcurvity::CConcurvity(CP_Body *pBody, int concurveParam) 
	: IObjectiveFuncCalculator(pBody), m_concurveParam(concurveParam) 
{
	InitConcurvePair2D();
}

CConcurvity::~CConcurvity() {
}

double CConcurvity::CalcObjectiveFunc() {
	double cost = 0;
	for (unsigned int i = 0; i < m_concurvePairs.size(); ++i) {
		CP_Curve3D *pCurve0 = m_concurvePairs[i]->m_pCurve0;
		CP_Curve3D *pCurve1 = m_concurvePairs[i]->m_pCurve1;
		CP_Vector3D tangent0 = pCurve0->GetTangent(m_concurvePairs[i]->m_param0);
		CP_Vector3D tangent1 = pCurve1->GetTangent(m_concurvePairs[i]->m_param1);

		cost += CalcConcurvity(tangent0, tangent1, m_concurvePairs[i]->m_ang2D);
	}
	return cost;
}

double CConcurvity::CalcConcurvity(CP_Vector3D tangent0, CP_Vector3D tangent1, double ang2D) {
	double r3D = (tangent0 ^ tangent1).GetLength();
	double w = 1.0 / (1 + exp(-m_concurveParam*(PI / 9 - ang2D)));
	return w * pow(r3D, 2);
}

void CConcurvity::InitConcurvePair2D() 
{
	static const double CONCURVE_TOLER = 0.2;
	for (int i = 0; i < m_pBody->GetVertexNumber(); ++i) {
		CP_Vertex *pVt = m_pBody->GetVertex(i);
		for (int j = 0; j < pVt->GetOutHalfNumber(); ++j) {
			CP_Edge *pEdge0 = pVt->GetOutHalf(j)->m_pEdge;
			CP_Curve3D *pCurve0 = pEdge0->m_pCurve3D;
			for (int k = j + 1; k < pVt->GetOutHalfNumber(); ++k) {
				CP_Edge *pEdge1 = pVt->GetOutHalf(k)->m_pEdge;
				CP_Curve3D *pCurve1 = pEdge1->m_pCurve3D;
				if (pEdge0->GetCurveType() != TYPE_CURVE_LINESEGMENT &&
					pEdge1->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
						pair<double, double> params = topo_geometric::FindMeetPtParam(pCurve0, pCurve1, pVt->m_pPoint);
						CP_Vector3D tangent0(pCurve0->GetTangent(params.first));
						CP_Vector3D tangent1(pCurve1->GetTangent(params.second));
						tangent0.m_z = 0;
						tangent1.m_z = 0;
						double ang = base_geometric::GetMinIncludedAngle(tangent0, tangent1);
						if (ang < CONCURVE_TOLER) {
							m_concurvePairs.push_back(shared_ptr<ConcurvePair2D>(new ConcurvePair2D(pCurve0, params.first, pCurve1, params.second, ang)));
						}
				}
			}
		}
	}
}

/************************************************************************/
/* CArcRatio                                                            */
/************************************************************************/

CArcRatio::CArcRatio(CP_Body *pBody) : IObjectiveFuncCalculator(pBody) {}

CArcRatio::~CArcRatio() {}

double CArcRatio::CalcObjectiveFunc() {
	double cost = 0;
	for (int i = 0; i < m_pBody->GetEdgeNumber(); ++i) {
		CP_Edge *pEdge = m_pBody->GetEdge(i);
		if (pEdge->GetCurveType() == TYPE_CURVE_ARC)
		{
			CP_Arc *pArc = static_cast<CP_Arc *>(pEdge->m_pCurve3D);
			CP_Vector3D vMajor(pArc->m_ucs.m_axisX * pArc->m_major);
			CP_Vector3D vMin(pArc->m_ucs.m_axisY * pArc->m_minor);
			double ratio = vMajor.GetLength() / vMin.GetLength();
			ratio = ratio > 1? ratio : 1/ratio;
			cost += abs(ratio - pArc->m_ratio);
			cost += abs(pArc->m_ucs.m_axisX * pArc->m_ucs.m_axisY);
		}
	}
	return cost;
}



/************************************************************************/
/* CTransitionEdgeCalc			                                        */
/************************************************************************/

CTransEdgeNormalCalc::CTransEdgeNormalCalc(CP_Body *pBody, vector<CTransitionEdgeInfo> &vTrans) : IObjectiveFuncCalculator(pBody), m_transitionEdgeInfo(vTrans)
{
	// InitTransitionEdge();
	
}

CTransEdgeNormalCalc::~CTransEdgeNormalCalc()
{
}

double CTransEdgeNormalCalc::CalcObjectiveFunc() {
	double cost = 0;
	for (unsigned int i = 0; i < m_transitionEdgeInfo.size(); ++i) {
		CTransitionEdgeInfo &info = m_transitionEdgeInfo[i];
		CP_Edge *pEdge = info.m_pTransitionEdge;
		CP_Point3D startPt = *pEdge->m_pStartVt->m_pPoint;
		CP_Point3D endPt = *pEdge->m_pEndVt->m_pPoint;

		CP_Vector3D sum = topo_geometric::GetCurveTangentAtPt(info.m_pEdgeAtStart0->m_pCurve3D, startPt);
		sum += topo_geometric::GetCurveTangentAtPt(info.m_pEdgeAtStart1->m_pCurve3D, startPt);
		sum += topo_geometric::GetCurveTangentAtPt(info.m_pEdgeAtEnd0->m_pCurve3D, endPt);
		sum += topo_geometric::GetCurveTangentAtPt(info.m_pEdgeAtEnd1->m_pCurve3D, endPt);
		sum.Normalize();
		CP_Vector3D n;
		if (pEdge->GetCurveType() == TYPE_CURVE_ARC) 
			n = dynamic_cast<CP_Arc*>(pEdge->m_pCurve3D)->m_ucs.m_axisZ ;
		else if (pEdge->GetCurveType() == TYPE_CURVE_NURBS) 
			n = dynamic_cast<CP_Nurbs*>(pEdge->m_pCurve3D)->m_normal;
		else
			continue;
		cost += CalcPara(sum, n);
	}
	return cost;
}

double CTransEdgeNormalCalc::CalcPara(CP_Vector3D &v0, CP_Vector3D &v1)
{
	return pow((v0 ^ v1).GetLength(), 2.0);
}

void CTransEdgeNormalCalc::InitTransitionEdge() {
	for (int i = 0; i < m_pBody->GetEdgeNumber(); ++i)
	{
		CP_Edge *pEdge = m_pBody->GetEdge(i);
		if (topo_geometric::IsTransitionEdge(pEdge)) {
			// 该处算法不严谨，应该是同一对面的边相互光滑连接
			CP_Edge *pEdge0 = NULL;
			CP_Edge *pEdge1 = NULL;
			ASSERT(topo_geometric::GetC2ContinuityEdgesAt(&pEdge0, &pEdge1, pEdge->m_pStartVt, pEdge));
			CP_Edge *pEdge2 = NULL;
			CP_Edge *pEdge3 = NULL;
			ASSERT(topo_geometric::GetC2ContinuityEdgesAt(&pEdge2, &pEdge3, pEdge->m_pEndVt, pEdge));
			m_transitionEdgeInfo.push_back(CTransitionEdgeInfo(pEdge, pEdge0, pEdge1, pEdge2, pEdge3));
		}
	}
}

/************************************************************************/
/* CGeneralizedFacePerpendicular                                        */
/************************************************************************/

CGenFacePerpendicular::CGenFacePerpendicular(CP_Body *pBody) :IObjectiveFuncCalculator(pBody)
{
	InitBestFitXYSum();
	InitWeight();
}
				

CGenFacePerpendicular::~CGenFacePerpendicular()
{}

double CGenFacePerpendicular::CalcObjectiveFunc() {
	double cost = 0;
	vector<CP_Point3D *> vPts;

	for (unsigned int i = 0; i < m_faceVtXYSums.size(); ++i) {
		CP_Face *pFace = m_pBody->GetFace(m_faceVtXYSums[i]->m_index);
		CP_Loop *pLoop = pFace->GetLoop(0);
		for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
			CP_Half *pHalf = pLoop->GetHalf(j);
			vPts.push_back(pHalf->m_pStartVt->m_pPoint);
		}
		CFaceBestFit face;
		CFaceBestFit::BestFit(vPts, m_faceVtXYSums[i], face);
		vPts.clear();
		CP_Vector3D n(face.m_A, face.m_B, face.m_C);
		for (int k = 0; k < pLoop->GetHalfNumber(); ++k) {
			CP_Half *pHalf = pLoop->GetHalf(k);
			if (pHalf->m_pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
				CP_Face *pAdjFace = pHalf->m_pTwin->m_pLoop->m_pParentFace;
				if (pAdjFace->GetSurfaceType() == TYPE_SURFACE_PLANE) {
					cost += (PI_2 - base_geometric::GetMinIncludedAngle(n, static_cast<CP_Plane*>(pAdjFace->m_surface)->GetNormal()));
					//m_pWeightsForFace[i][k] = CalcWeight(n, static_cast<CP_Plane*>(pAdjFace->m_surface));
					//CLog::log("%d, %f", i, m_pWeightsForFace[i][k]);
				}
			}
		}
	}
	return cost;
}

double CGenFacePerpendicular::CalcWeight(CP_Vector3D n, CP_Plane *pPlane){
	double ang = base_geometric::GetMinIncludedAngle(n, pPlane->GetNormal());
	return exp(-pow(PI_2 - ang, 2.0) / 0.08);
}

void CGenFacePerpendicular::InitWeight() {
	for (unsigned int i = 0; i < m_faceVtXYSums.size(); ++i) {
		CP_Face *pFace = m_pBody->GetFace(m_faceVtXYSums[i]->m_index);
		CP_Loop *pLoop = pFace->GetLoop(0);
		int count = 0;
		vector<double> tmp(pLoop->GetHalfNumber(), 1.0);
		m_pWeightsForFace.push_back(tmp);
	}
}

void CGenFacePerpendicular::InitBestFitXYSum() {
	vector<CP_Point3D *> vPts;
	for (int i = 0; i < m_pBody->GetFaceNumber(); ++i) {
		CP_Face *pFace = m_pBody->GetFace(i);

		// Curve face is a generalized face
		if (pFace->GetSurfaceType() != TYPE_SURFACE_PLANE) {
			CP_Loop *pLoop = pFace->GetLoop(0);
			ASSERT (pLoop->GetHalfNumber() >= 2);
			if (pLoop->GetHalfNumber() > 2 && !topo_geometric::HasRedLine(pLoop)) {
				for (int j = 0; j < pLoop->GetHalfNumber(); ++j) {
					CP_Half *pHalf = pLoop->GetHalf(j);
					vPts.push_back(pHalf->m_pStartVt->m_pPoint);
				}
				m_faceVtXYSums.push_back(shared_ptr<CFaceSumData>(CFaceBestFit::createFaceXYSum(i, vPts)));
				vPts.clear();
			}
		}
	}
}


/************************************************************************/
/* CCornerOrthogonality                                                 */
/************************************************************************/

CCornerOrthogonality::CCornerOrthogonality(CP_Body *pBody) : IObjectiveFuncCalculator(pBody)
{
	InitCandidate();
	m_pWeights = new std::vector<double>( m_vpCandidate.size(), 1.0);
}

CCornerOrthogonality::~CCornerOrthogonality() {}

double CCornerOrthogonality::CalcObjectiveFunc() {
	double cost = 0;
	vector<CP_Vector3D> vTangents;
	for (unsigned int i = 0; i < m_vpCandidate.size(); ++i) {
		CP_Vertex *pVt = m_vpCandidate[i];
		double tmp = 0;
		for (int j = 0; j < pVt->GetOutHalfNumber(); ++j) {
			CP_Half *pHalf = pVt->GetOutHalf(j);
			CP_Vector3D tangent = topo_geometric::GetCurveTangentAtPt(pHalf->m_pEdge->m_pCurve3D, *pVt->m_pPoint);
			vTangents.push_back(tangent);
		}
		for (unsigned int k = 0; k < vTangents.size(); ++k) {
			tmp += abs(vTangents[k] * vTangents[(k+1) % vTangents.size()]);
		}
		//CLog::log("%f", m_pWeights->at(i));
		cost += tmp * m_pWeights->at(i);
		m_pWeights->at(i) = CalcWeight(pVt);
		vTangents.clear();
	}
	return cost;
}

double CCornerOrthogonality::CalcWeight(CP_Vertex *pVt) {
	vector<CP_Vector3D> vTangents;
	double weight = 0;
	for (int j = 0; j < pVt->GetOutHalfNumber(); ++j) {
		CP_Half *pHalf = pVt->GetOutHalf(j);
		CP_Vector3D tangent = topo_geometric::GetCurveTangentAtPt(pHalf->m_pEdge->m_pCurve3D, *pVt->m_pPoint);
		vTangents.push_back(tangent);
	}
	for (unsigned int k = 0; k < vTangents.size(); ++k) {
		double ang = base_geometric::GetACOS(vTangents[k] * vTangents[(k+1) % vTangents.size()]);
		weight += exp(- pow(PI_2 - ang, 2.0) / 0.08);
	}
	return weight / 3;
}

void CCornerOrthogonality::InitCandidate() {
	vector<CP_Vector3D> vTangents;
	for (int i = 0; i < m_pBody->GetVertexNumber(); ++i) {
		CP_Vertex *pVt = m_pBody->GetVertex(i);
		if (pVt->GetOutHalfNumber() == 3) {
			for (int j = 0; j < pVt->GetOutHalfNumber(); ++j) {
				CP_Half *pHalf = pVt->GetOutHalf(j);
				CP_Vector3D tangent = topo_geometric::GetCurveTangentAtPt(pHalf->m_pEdge->m_pCurve3D, *pVt->m_pPoint);
				tangent.m_z = 0;
				vTangents.push_back(tangent);
			}
			if(IsCornerOrthogonal(vTangents))
				m_vpCandidate.push_back(pVt);
		}
		vTangents.clear();
	}
		
}

bool CCornerOrthogonality::IsCornerOrthogonal (vector<CP_Vector3D> &vTangents) {
	ASSERT(vTangents.size() == 3);
	int nLargeAng = 0; // more than 90 degree
	int nSmallAng = 0; // less than 90 degree
	double sumSmallAng = 0;
	for (unsigned int i = 0; i < vTangents.size(); ++i) {
		double ang = base_geometric::GetIncludedAngle(vTangents[i], vTangents[(i+1)%vTangents.size()]);
		if (ang > PI_2) // more than 90 degree
			nLargeAng++;
		else {
			nSmallAng++;
			sumSmallAng += ang;
		}
	}
	return nLargeAng == 3 || nSmallAng == 2 && sumSmallAng > PI_2;
}


/************************************************************************/
/* CDepthObjFuncCalculator                                              */
/************************************************************************/

CDepthObjFuncCalculator::CDepthObjFuncCalculator(CP_Body *pBody, const vector<int> &vWeight)
	: IObjectiveFuncCalculator(pBody)
{
}

CDepthObjFuncCalculator::~CDepthObjFuncCalculator() {
	for (unsigned int i = 0; i < m_vObjFuncs.size(); ++i) {
		delete m_vObjFuncs[i].first;
	}
}

double CDepthObjFuncCalculator::CalcObjectiveFunc()  {
	rec_utils::ReconstructBodyPlaneFaceNormal(m_pBody);
	rec_curve::ReconstructBodyEdge(m_pBody);

	double cost = 0;

	for (unsigned int i = 0; i < m_vObjFuncs.size(); ++i) {
		double curCost = m_vObjFuncs[i].second * m_vObjFuncs[i].first->CalcObjectiveFunc();
		cost += curCost;
		// CLog::log("Cost %d = %f", i, curCost);
	}
	return cost;
}


void CDepthObjFuncCalculator::AddObjFunc(IObjectiveFuncCalculator *pObjFunc, int weight) {
	m_vObjFuncs.push_back(pair<IObjectiveFuncCalculator *, int>(pObjFunc, weight));
}

/************************************************************************/
/* CVtDepObjFuncCalculator                                              */
/************************************************************************/

CVtDepObjFuncCalculator::CVtDepObjFuncCalculator(CP_Body *pBody, const vector<int> &vWeight, int paramLineParal /* = 100 */, int paramConcurve /* = 100 */)
	: CDepthObjFuncCalculator(pBody, vWeight)
{
	CLineParal2DInfo::InitLineParal2D(pBody, m_vLineParal2D);

	AddObjFunc(new CLineParallelism(pBody, paramLineParal, m_vLineParal2D), 40);
	AddObjFunc(new CFacePlanarity(pBody), 1);
	//AddObjFunc(new CIsometry(pBody, m_vLineParal2D), 15);
	AddObjFunc(new CMSDA(pBody), 1);
	AddObjFunc(new CArcRatio(pBody), 40);
	AddObjFunc(new CCornerOrthogonality(pBody), 10);
	AddObjFunc(new CGenFacePerpendicular(pBody), 20);
	//AddObjFunc(new CConcurvity(pBody, paramConcurve), 10);
}

CVtDepObjFuncCalculator::~CVtDepObjFuncCalculator()
{}

void CVtDepObjFuncCalculator::SetValue(int index, double value) {
	m_pBody->GetVertex(index)->m_pPoint->m_z = value;
}

/************************************************************************/
/* CGenFaceDepObjFuncCalculator                                         */
/************************************************************************/

CGenFaceDepObjFuncCalculator::CGenFaceDepObjFuncCalculator(CP_Body *pBody, const vector<int> &vWeight, vector<pair<CP_Face *, double> > &vFaces)
	: CDepthObjFuncCalculator(pBody, vWeight), m_vpFaces(vFaces)
{
	AddObjFunc(new CArcRatio(pBody), 10);
	AddObjFunc(new CGenFacePerpendicular(pBody), 20);
	AddObjFunc(new CFacePlanarity(pBody), 20);
}

CGenFaceDepObjFuncCalculator::~CGenFaceDepObjFuncCalculator(){}

void CGenFaceDepObjFuncCalculator::SetValue(int index, double value) {
	CP_Face *pFace = m_vpFaces[index].first;
	CP_Loop *pLoop = pFace->GetLoop(0);
	double average = m_vpFaces[index].second;
	double dif = value - average;
	for (int i = 0; i < pLoop->GetHalfNumber(); ++i) {
		CP_Half *pHalf = pLoop->GetHalf(i);
		CP_Vertex *pVt = pHalf->m_pStartVt;
		CP_EdgeExt *pEdge = dynamic_cast<CP_EdgeExt *>(pHalf->m_pEdge);
		if (pEdge->HasProperty(PROPERTY_NAME_MID_VERTEX))
			((CP_Vertex *)pEdge->GetCObjPtProperties(PROPERTY_NAME_MID_VERTEX))->m_pPoint->m_z += dif;
		pVt->m_pPoint->m_z += dif;
	}
	m_vpFaces[index].second = value;
}

void CGenFaceDepObjFuncCalculator::RotateAdjCurve(CP_Vertex *pVt, CP_Loop *pLoop, const CP_Point3D &oriPt, const CP_Point3D &dstPt) {
	for (int i = 0; i < pVt->GetOutHalfNumber(); ++i) {
		CP_Half *pHalf = pVt->GetOutHalf(i);

		// Find edge not in loop
		if (pHalf->m_pLoop != pLoop && pHalf->m_pTwin->m_pLoop != pLoop) {
			CP_Edge *pEdge = pHalf->m_pEdge;

			// Find curve
			if (pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT) {
				CP_Point3D *pStr = pHalf->m_pEndVt->m_pPoint;
				CP_Vector3D srcVec = oriPt - *pStr;
				CP_Vector3D dstVec = dstPt - *pStr;
				CP_Vector3D axis = srcVec ^ dstVec;
				double ang = base_geometric::GetIncludedAngle(srcVec, dstVec);
				RotateCurve(pEdge, axis, ang);
				rec_curve::ReconstructBsplineCtrlPts(pEdge);
			}
		}
	}
}

void CGenFaceDepObjFuncCalculator::RotateCurve(CP_Edge *pEdge, const CP_Vector3D &axis, double ang) {
	if (pEdge->GetCurveType() == TYPE_CURVE_ARC) {

	} else if (pEdge->GetCurveType() == TYPE_CURVE_NURBS) {
		CP_Vector3D &n = dynamic_cast<CP_Nurbs*>(pEdge->m_pCurve3D)->m_normal;
		CP_Point3D pt(n.m_x, n.m_y, n.m_z);
		base_geometric::Rotate(axis, &pt, ang);
		n.m_x = pt.m_x;
		n.m_y = pt.m_y;
		n.m_z = pt.m_z;
	} else {
		ASSERT(false);
	}
}

/************************************************************************/
/* CTransitionEdgeFuncCalculator                                        */
/************************************************************************/

CTransitionCurveFuncCalculator::CTransitionCurveFuncCalculator(CP_Body *pBody, const vector<int> &vWeight, vector<CTransitionEdgeInfo> &vTrans)
	: CDepthObjFuncCalculator(pBody, vWeight), m_vTrans(vTrans)
{
	AddObjFunc(new CArcRatio(pBody), 10);
	AddObjFunc(new CTransEdgeNormalCalc(pBody, m_vTrans), 20);
}

CTransitionCurveFuncCalculator::~CTransitionCurveFuncCalculator()
{
}

void CTransitionCurveFuncCalculator::GetTransitionCurves(CP_Body *pBody, vector<CTransitionEdgeInfo> &transitionEdgeInfos)
{
	for (int i = 0; i < pBody->GetEdgeNumber(); ++i)
	{
		CP_Edge *pEdge = pBody->GetEdge(i);
		if (topo_geometric::IsTransitionEdge(pEdge) &&
			pEdge->GetCurveType() != TYPE_CURVE_LINESEGMENT &&
			pEdge->m_pHalfs[0]->m_pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE &&
			pEdge->m_pHalfs[1]->m_pLoop->m_pParentFace->GetSurfaceType() != TYPE_SURFACE_PLANE) {
			// 该处算法不严谨，应该是同一对面的边相互光滑连接
			CP_Edge *pEdge0 = NULL;
			CP_Edge *pEdge1 = NULL;
			topo_geometric::GetC2ContinuityEdgesAt(&pEdge0, &pEdge1, pEdge->m_pStartVt, pEdge);
			CP_Edge *pEdge2 = NULL;
			CP_Edge *pEdge3 = NULL;
			topo_geometric::GetC2ContinuityEdgesAt(&pEdge2, &pEdge3, pEdge->m_pEndVt, pEdge);
			transitionEdgeInfos.push_back(CTransitionEdgeInfo(pEdge, pEdge0, pEdge1, pEdge2, pEdge3));
		}
	}
}

void CTransitionCurveFuncCalculator::SetValue(int index, double value)
{
	CP_EdgeExt *pEdge = dynamic_cast<CP_EdgeExt *>(m_vTrans[index].m_pTransitionEdge);
	((CP_Vertex *)pEdge->GetCObjPtProperties(PROPERTY_NAME_MID_VERTEX))->m_pPoint->m_z = value;
}

