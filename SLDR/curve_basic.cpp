#include "stdafx.h"
#include "curve_basic.h"
#include "CP_Curve.h"
#include "base_geometricoper.h"
#include "mathlib.h"
#include "CMatrix4.h"
#include "CP_PointVector.h"
#include "CLog.h"

namespace curve_basic {

	void NurbsDegreeElevation(CP_Nurbs *pNurbs) {
		int i, j, k;
		const double KNOT_DIF_TOLER = 1e-4;

		// Original curve's information
		int order = pNurbs->GetOrder();
		int numCtrl = pNurbs->GetCtrlNumber();
		vector<double> &vecKnot = pNurbs->m_knotArr;
		vector<CP_Point3D*> &vOriCtrls = pNurbs->m_ctrlArr;
		vector<double> &vOriWeight = pNurbs->m_weightArr;

		// 确定新节点: 所有相异节点重复度均加1
		vector<double> vKnotTmp(2*numCtrl+2*order);
		int l = 0;
		for ( i = 0; i < numCtrl+order-1; i++ )
		{   
			vKnotTmp[l] = vecKnot[i];   
			l++;
			if ( fabs(vecKnot[i]-vecKnot[i+1]) > KNOT_DIF_TOLER )
			{
				vKnotTmp[l] = vecKnot[i];
				l++;
			}       
		}                              
		vKnotTmp[l] = vKnotTmp[l-1];
		l++;
		vKnotTmp[l] = vKnotTmp[l-1];
		l++;

		// resultKnot为最终节点向量 
		vector<double> resultKnot(vKnotTmp);

		// 确定新控制点
		int nResultCtrl;
		nResultCtrl = l-(order+1);
		vector<CP_Point3D *> vResultCtrl(nResultCtrl);  
		vector<double> vResultWeight(nResultCtrl); 
		CP_Point3D tmpCtrl;
		double tmpWeight;

		vector<double> alf(numCtrl+order); // 离散B样条
		vector<double> nin(numCtrl+order); // 升阶公式中的系数因子
		double under1, under2;
		for ( i = 0; i < nResultCtrl; i++ )
		{ 
			// 升阶公式中的系数因子计算
			for( k = 0; k < order; k++ )
			{
				if ( k == 0 )   // Initialize
				{
					for ( j = 0; j < numCtrl+order; j++ )
					{
						if ( resultKnot[i]-vecKnot[j] > - KNOT_DIF_TOLER && vecKnot[j+1]-resultKnot[i] > TOLER )
							alf[j] = nin[j] = 1;    
						else 
							alf[j] = nin[j] = 0;   
					} 
				}    
				else
				{                          
					for ( j = 0; j < numCtrl+order-k-1; j++ )
					{
						under1 = vecKnot[j+k]-vecKnot[j];
						if ( fabs(under1) < KNOT_DIF_TOLER )
							under1 = 0;
						else 
							under1 = 1/under1;    
						under2 = vecKnot[j+k+1]-vecKnot[j+1];                    
						if ( fabs(under2) < KNOT_DIF_TOLER )
							under2 = 0;
						else 
							under2 = 1/under2;    
						alf[j] = (resultKnot[i+k]-vecKnot[j])*under1*alf[j]+
							(vecKnot[j+k+1]-resultKnot[i+k])*under2*alf[j+1];
						nin[j] = (resultKnot[i+k+1]-vecKnot[j])*under1*nin[j]+
							(vecKnot[j+k+1]-resultKnot[i+k+1])*under2*nin[j+1]+alf[j];
					} 
				}    
			}
			// Cohen-Lyche-Schumaker升阶公式
			tmpCtrl = ZeroP3D;
			tmpWeight = 0;
			for ( j = 0; j < numCtrl; j++ )
			{
				tmpCtrl += vOriWeight[j]*(*vOriCtrls[j])*nin[j];
				tmpWeight += vOriWeight[j]*nin[j];
			}                                                            
			vResultWeight[i] = tmpWeight/order;
			vResultCtrl[i] = new CP_Point3D((1.0/tmpWeight)*tmpCtrl);

		}    

		// Set result
		pNurbs->m_degree += 1;
		pNurbs->m_ctrlNumber = nResultCtrl;
		pNurbs->m_knotArr = resultKnot; 
		pNurbs->m_ctrlArr = vResultCtrl;
		pNurbs->m_weightArr = vResultWeight;
	}

	void NurbsMultiDegreeElevation(int number, CP_Nurbs *&curv)
	{
		for(int k = 0; k < number; k++)
			NurbsDegreeElevation(curv);
	}

	void Put2NurbsCurvesSameOrder(CP_Nurbs *pCurve0, CP_Nurbs *pCurve1) {
		int order1, order2, order;
		order1 = pCurve0->GetOrder();
		order2 = pCurve1->GetOrder();

		// 升阶成同阶曲线
		order = max(order1, order2);
		if(order1 < order) 
			NurbsMultiDegreeElevation(order-order1, pCurve0);
		else if(order2 < order)
			NurbsMultiDegreeElevation(order-order2, pCurve1);
	}

	void Put2NurbsCurvesCompatible(CP_Nurbs *pCurve0, CP_Nurbs *pCurve1) {
		Put2NurbsCurvesSameOrder(pCurve0, pCurve1);
		CP_Nurbs* pCurves[2] = {pCurve0, pCurve1};
		PutNNurbsCurvesCompatible(pCurves, 2);
	}

	void PutNNurbsCurvesCompatible(CP_Nurbs **pCurve, int N)
	{
		int		i, i0, ik, j, k, m0, mk, flag;
		int		*order, *ctrlNum;
		double  **knot;

		knot =		new double*[N];
		order =		new int[N];
		ctrlNum =	new int[N];

		for(i=0; i<N;i++)
		{
			ctrlNum[i] = pCurve[i]->GetCtrlNumber();
			order[i] = pCurve[i]->GetOrder();			 
			//here order[i]==constant,  otherewise cannot do compatible 980602
			knot[i] = new double[ctrlNum[i] + order[i] + 1 ];
		}

		for(i=0; i<N;i++)
		{
			for(j=0; j<ctrlNum[i] + order[i]; j++)
				knot[i][j] = pCurve[i]->GetKnotValue(j);
		}

		//////合并c[k]的节点矢量	
		////////////c[k]往c0中插节点
		//两端点不插点 1127
		for(k=1; k<N; k++)
		{
			for(j=order[k]; j<ctrlNum[k]; )		 
			{
				mk = pCurve[k]->GetKnotMulti(knot[k][j]); 	  //mk>=1.
				//先判节点属于那一节点区间	
				i0 = pCurve[0]->GetIntervalIndex(knot[k][j]);
				//计算该节点的重数
				m0 = pCurve[0]->GetKnotMulti(knot[k][j]);   //!!12.23.  //mo可以为0.
				flag = 0;

				if((knot[k][j] - knot[0][i0])>TOLER && (knot[0][i0+1] - knot[k][j])>TOLER)
				{
					pCurve[0]->InsertMultiKnot(mk, knot[k][j]);
					flag = 1;
				}
				else if(fabs(knot[k][j] - knot[0][i0+1])<=TOLER && mk > m0)
				{
					pCurve[0]->InsertMultiKnot(mk-m0, knot[k][j]);
					flag = 1;
				}

				if(flag == 1)
				{
					delete[] knot[0];
					knot[0] = new double[pCurve[0]->GetCtrlNumber() + order[0] ];
					for(int jj=0; jj<pCurve[0]->GetCtrlNumber() + order[0]; jj++)
						knot[0][jj] = pCurve[0]->GetKnotValue(jj);
				}

				j += mk;
			}
		}
		ctrlNum[0] = pCurve[0]->GetCtrlNumber();      //1226
		/////////////////////////
		////////////c0往c[k]中插节点
		for(k=1; k<N;k++)
		{
			for(j=order[0]; j<ctrlNum[0]; )		 
			{
				m0 = pCurve[0]->GetKnotMulti(knot[0][j]); 
				//先判节点属于c[k]中的那一节点区间	
				ik = pCurve[k]->GetIntervalIndex(knot[0][j]);
				//计算该节点在c[k]中的重数
				mk = pCurve[k]->GetKnotMulti(knot[0][j]);
				flag = 0;

				if((knot[0][j] - knot[k][ik])>TOLER && (knot[k][ik+1] - knot[0][j])>TOLER)     //1226
				{
					pCurve[k]->InsertMultiKnot(m0, knot[0][j]);
					flag = 1;
				}
				else if(fabs(knot[0][j] - knot[k][ik+1]) <= TOLER && m0 > mk)     //1226
				{
					pCurve[k]->InsertMultiKnot(m0 - mk, knot[0][j]);
					flag = 1;
				}

				if(flag == 1)
				{
					delete[] knot[k];														//1226		
					knot[k] = new double[pCurve[k]->GetCtrlNumber() + order[k] ];		//1226
					for(int jj=0; jj<pCurve[k]->GetCtrlNumber() + order[k]; jj++)				//1226
						knot[k][jj] = pCurve[k]->GetKnotValue(jj);								//1226
				}

				j += m0;
			}
		} 

		// Release memory
		for(i=0; i<N; i++)
			delete knot[i];
		delete []knot;

		delete[] order;
		delete[] ctrlNum;
	}


	void LinearInterpolateNurbs(CP_Nurbs *pNurbs0, CP_Nurbs *pNurbs1, double param, CP_Nurbs *pResult) {
		ASSERT(pNurbs0 != NULL && pNurbs1 != NULL && pResult != NULL);
		ASSERT(IsIsoparametric(pNurbs0, pNurbs1));
		ASSERT(param < 1 && param > 0);
		pResult->m_ctrlNumber = pNurbs0->m_ctrlNumber;
		pResult->m_degree = pNurbs0->m_degree;
		pResult->m_knotArr = pNurbs0->m_knotArr;

		for (int i = 0; i < pNurbs0->GetCtrlNumber(); ++i) {
			pResult->AddCtrlPoint( new CP_Point3D(param * *pNurbs0->GetCtrlPoint(i) + (1-param) * *pNurbs1->GetCtrlPoint(i)) );
			pResult->AddWeightValue(param * pNurbs0->GetWeightValue(i) + (1-param) * pNurbs1->GetWeightValue(i));
		}
	}

	void NurbsScale(CP_Nurbs *pNurbs, CP_Point3D *pBasePt, double scale) {
		for (int i = 0; i < pNurbs->GetCtrlNumber(); ++i) {
			base_geometric::Scale(pBasePt, pNurbs->GetCtrlPoint(i), scale);
		}
	}

	void NurbsRotate(CP_Nurbs *pNurbs, CP_Point3D *pBasePt, double ang, CP_Vector3D &axis) {
		for (int i = 0; i < pNurbs->GetCtrlNumber(); ++i) {
			CP_Point3D *pCtrlPt = pNurbs->GetCtrlPoint(i);
			base_geometric::Rotate(pBasePt, pCtrlPt, ang, axis);
		}
	}

	void NurbsTranslate(CP_Nurbs *pNurbs, CP_Point3D *pAttachPt, CP_Point3D *pPtOnNurbs) {
		CP_Vector3D v = *pAttachPt - *pPtOnNurbs;
		for (int i = 0; i < pNurbs->GetCtrlNumber(); ++i) {
			CP_Point3D *pCtrlPt = pNurbs->GetCtrlPoint(i);
			*pCtrlPt += v;
		}
	}

	void NurbsTransformToAttach(CP_Nurbs *pNurbs, CP_Point3D *pAttachPt, bool isReverse /* = false */) {
		CP_Vector3D oriVec = pNurbs->GetEndingPoint() - pNurbs->GetStartingPoint();
		double baseParam, onNurbsParam;

		if (isReverse) {
			baseParam = 1;
			onNurbsParam = 0;
		} else {
			baseParam = 0;
			onNurbsParam = 1;
		}
		CP_Point3D basePt = pNurbs->GetPoint(baseParam);

		// Scale
		CP_Vector3D dstVec(*pAttachPt - basePt);
		ASSERT(oriVec.GetLength() > 0);
		double scale = dstVec.GetLength() / oriVec.GetLength();
		NurbsScale(pNurbs, &basePt, scale);

		// Rotate
		oriVec = pNurbs->GetPoint(onNurbsParam) - basePt;
		dstVec = *pAttachPt - basePt;
		double ang = base_geometric::GetIncludedAngle(oriVec, dstVec);
		if (abs(ang) < TOLER)
			return;
		CP_Vector3D axis = oriVec ^ dstVec;
		NurbsRotate(pNurbs, &basePt, ang, axis);
	}

	void AdjustCurveDirect(CP_Curve3D *pCurve, CP_Point3D *pStartPt) {
		CP_LineSegment3D *pLine = dynamic_cast<CP_LineSegment3D*>(pCurve);
		if (pLine) {
			AdjustLinesegDirect(pLine, pStartPt);
			return;
		}
		CP_Arc *pArc = dynamic_cast<CP_Arc*>(pCurve);
		if (pArc) {
			AdjustArcDirect(pArc, pStartPt);
			return;
		}
		CP_Nurbs *pNurbs = dynamic_cast<CP_Nurbs*>(pCurve);
		if (pNurbs) {
			AdjustNurbsDirect(pNurbs, pStartPt);
			return ;
		}
		ASSERT(false);
	}

	void AdjustArcDirect(CP_Arc *pArc, CP_Point3D *pStartPt) {
		if (!base_geometric::HasSameProjectionPt(&pArc->GetStartingPoint(), pStartPt, SAME_POINT_THRESHOLD)) {
			curve_basic::ArcReverse(pArc);
		}
	}

	void ArcReverse(CP_Arc *pArc) {
		swap(pArc->m_stangle, pArc->m_edangle);
	}

	void AdjustLinesegDirect(CP_LineSegment3D *pLineseg, CP_Point3D *pStartPt) {
		if (!base_geometric::HasSameProjectionPt(&pLineseg->GetStartingPoint(), pStartPt, SAME_POINT_THRESHOLD)) {
			curve_basic::LinesegReverse(pLineseg);
		}
	}

	void LinesegReverse(CP_LineSegment3D *pLineseg) {
		swap(pLineseg->m_startPt, pLineseg->m_endPt);
	}

	void AdjustNurbsDirect(CP_Nurbs *pNurbs, CP_Point3D *pStartPt) {
		if (!base_geometric::HasSameProjectionPt(&pNurbs->GetStartingPoint(), pStartPt, SAME_POINT_THRESHOLD)) {
			curve_basic::NurbsReverse(pNurbs);
		}
	}

	void NurbsReverse(CP_Nurbs *pNurbs) {
		int n = pNurbs->GetCtrlNumber();
		for (int i = 0; i < n/2; ++i) {
			std::swap(*pNurbs->GetCtrlPoint(i), *pNurbs->GetCtrlPoint(n-i-1));
			std::swap(pNurbs->m_weightArr[i], pNurbs->m_weightArr[n-i-1]);
		}
		for (int i = 0; i < pNurbs->GetKnotNumber(); ++i) {
			pNurbs->m_knotArr[i] = 1 - pNurbs->m_knotArr[i];
		}
		std::reverse(pNurbs->m_knotArr.begin(), pNurbs->m_knotArr.end());
	}

	CP_Nurbs* InterpolateNurbsCurve(int count, CP_Point3D* intpoPnt, double* intpoWeight, 
		int order, CP_Vector3D vs, CP_Vector3D ve, double *knotU) 
	{
		int i;

		vs *= CP_Vector3D(intpoPnt[0] - intpoPnt[1]).GetLength()/7;
		ve *= CP_Vector3D(intpoPnt[count-2] - intpoPnt[count-1]).GetLength()/7;

		//在超平面上计算U向控制点  //here order==4
		CP_Nurbs curve;
		curve.m_degree = 3;
		curve.m_ctrlNumber = count+2;
		for (int i = 0; i < curve.m_ctrlNumber + 3; ++i)
			curve.AddKnotValue(knotU[i]);

		double **matrix;
		matrix = new double*[count+2];

		for ( i = 1; i < count+1; i++ )
		{
			matrix[i] = new double [count+2];
			BscurvBaseValue(curve, curve.GetKnotValue(i+2), matrix[i]);  
		}
		matrix[0] = new double [count+2];
		matrix[count+1] = new double [count+2];
		for ( i = 0; i < count+2; i++ )
		{
			matrix[0][i] = 0;
			matrix[count+1][i] = 0;
		}
		matrix[0][0] = 1;
		matrix[0][1] = -1;
		matrix[count+1][count] = 1;
		matrix[count+1][count+1] = -1;

		CP_Point3D *Q, *P;
		Q = new CP_Point3D [count+2];
		P = new CP_Point3D [count+2];
		double* w, *wx;
		w = new double[count + 2];
		wx = new double[count + 2];
		Q[0] = CP_Point3D(0,0,0) - vs;
		//	Q[0] = intpoPnt[0] + vs;
		w[0] = 0;
		for ( i = 1; i < count+1; i++ )
		{
			Q[i] = intpoPnt[i-1];
			w[i] = intpoWeight[i-1];
		}
		Q[count+1] = CP_Point3D(0,0,0) - ve;
		//	Q[count+1] = intpoPnt[count-1] + ve;

		w[count+1] = 0;
		for ( i = 1; i < count+1; i++ )
		{
			Q[i].m_x *= w[i];
			Q[i].m_y *= w[i];
			Q[i].m_z *= w[i];
		}

		if ( ! math_lib::EquatSystemNumber(count+2, matrix, w, wx) )
			return 0;

		if ( ! math_lib::EquatSystemPoint(count+2, matrix, Q, P) )
			return 0;
		for ( i = 0; i < count+2; i++ )
		{
			if ( wx[i] != 0 )
			{
				P[i].m_x /= wx[i];
				P[i].m_y /= wx[i];
				P[i].m_z /= wx[i];
			}
		}

		CP_Nurbs *pNurbs = new CP_Nurbs(count+2, order, P, knotU, wx);

		delete []P;
		delete []Q;
		delete []w;
		delete []wx;
		for(i=0;i<count+2; i++)
			delete []matrix[i];
		delete []matrix;

		return pNurbs; 
	}

	//求B样条基函数u处的值, 
	//Added by LiuXiaoming 2007-5-18
	//采用NURBS book P70 算法
	void BscurvBaseValueNonZero(const CP_Nurbs& curve, double u, double *coef)  
	{
		int n(curve.m_ctrlNumber), order(curve.GetOrder());
		int j,r;
		double saved,temp;
		double *left=new double[order];
		double *right=new double[order];
		int	i = curve.GetIntervalIndex(u);
		coef[0]=1.0;
		for(j=1; j<order; j++)
		{
			left[j] = u - curve.GetKnotValue(i+1-j);
			right[j] = curve.GetKnotValue(i+j)-u;
			saved=0.0;
			for(r=0; r<j; r++)
			{
				temp=coef[r]/(right[r+1]+left[j-r]);
				coef[r]=saved+right[r+1]*temp;
				saved=left[j-r]*temp;
			}
			coef[j]=saved;
		}
		delete []left;
		delete []right;
	}

	//改变参数传递为常量引用传递
	//采用NURBS book P70 算法
	void BscurvBaseValue(const CP_Nurbs& curve, double u, double *coef)  
	{
		int n(curve.m_ctrlNumber), order(curve.GetOrder()),p;
		p = order-1;
		double *NonzeroCoef = new double[order];
		int	i = curve.GetIntervalIndex(u);
		BscurvBaseValueNonZero(curve,u,NonzeroCoef);
		memset(coef,0,n*sizeof(double));
		memcpy(coef+i-p,NonzeroCoef,sizeof(double)*order);
		delete []NonzeroCoef;
	}


	void CalKnotU(int SubdivNum, int sec_ctrl_num, 
		CP_Point3D **sc_pnt,int order, double* knotU)
	{
		int    i,j;
		double **u, *paraU;
		double L;
		BOOL   degen = FALSE;
		int    degen_index;
		paraU = new double [SubdivNum];
		u = new double*[sec_ctrl_num];
		for(i = 0;i < sec_ctrl_num;i ++)
			u[i] = new double[SubdivNum];

		for(i = 0;i < sec_ctrl_num;i ++)
		{
			L = 0.0;
			for(j = 1;j < SubdivNum;j ++)
				L += CP_Vector3D(sc_pnt[i][j-1] - sc_pnt[i][j]).GetLength();
			if(fabs(L) < TOLER)
			{
				degen = TRUE;
				degen_index = i;
				continue;
			}
			u[i][0] = 0.;
			for(j = 1; j < SubdivNum; j ++) {
				u[i][j] = u[i][j-1] + CP_Vector3D(sc_pnt[i][j-1] - sc_pnt[i][j]).GetLength()/L;
			}

			ASSERT( fabs(u[i][SubdivNum-1] - 1.) < TOLER);
		}
		if(degen)
		{
			for(i = 0; i < SubdivNum; i ++)
				u[degen_index][i] = u[(int)sec_ctrl_num/2][i];
		}
		for(i = 0; i < SubdivNum; i ++)
		{
			paraU[i] = 0.0;
			for(j=0;j<sec_ctrl_num;j++)
				paraU[i] += u[j][i] ;
			paraU[i] /= sec_ctrl_num;
		}

		for(i = 0;i < sec_ctrl_num;i ++)
			delete []u[i];
		delete []u;

		for(i = 0;i < order;i ++)
		{
			knotU[i] = 0.0;
			knotU[SubdivNum+order-2 + i] = 1.;
		}
		for(i = 1; i < SubdivNum-1; i ++)
			knotU[i+order-1] = paraU[i];

		delete []paraU;
	}

	void GetIntpoEndTangV(int num, CP_Point3D *Pnt, CP_Vector3D &sT, CP_Vector3D &eT)
	{
		double angle, radius;
		if( num==2 )
		{
			sT = (Pnt[0] + Pnt[1]) / 2 - Pnt[0];
			eT = Pnt[1] - (Pnt[0] + Pnt[1]) / 2; 
			return;
		}

		if(CP_Vector3D((Pnt[1]-Pnt[0])^(Pnt[2]-Pnt[1])).GetLength() < TOLER)
			sT = Pnt[1] - Pnt[0];
		else
		{
			CP_Point3D cp1 = GetArcCenterp(Pnt[0], Pnt[1], Pnt[2], angle, radius);
			if(angle>PI)
				sT = Pnt[1]-Pnt[0];
			else
			{
				CP_Vector3D v1 = cp1 - Pnt[0];
				CP_Vector3D vectZ = (Pnt[1]-Pnt[0])^(Pnt[2]-Pnt[1]);
				vectZ.Normalize();
				CMatrix4	mat;
				mat.SetRotateV(vectZ, -PI_2);
				sT = v1 * mat;

				double d1 = CP_Vector3D(Pnt[0] - Pnt[1]).GetLength() + CP_Vector3D(Pnt[1] - Pnt[2]).GetLength();
				if( sT.GetLength() > d1 - TOLER )
				{
					sT.Normalize();
					sT *= d1;
				}
			}
		}

		if(CP_Vector3D((Pnt[num-1]-Pnt[num-2])^(Pnt[num-2]-Pnt[num-3])).GetLength() < TOLER)
			eT = Pnt[num-1] - Pnt[num-2];
		else
		{
			CP_Point3D cp2 = GetArcCenterp(Pnt[num-3], Pnt[num-2], Pnt[num-1], angle, radius);
			if(angle>PI)
				eT = Pnt[num-1]-Pnt[num-2];
			else
			{
				CP_Vector3D v2 = cp2 - Pnt[num-1];
				CP_Vector3D vectZ = (Pnt[num-2]-Pnt[num-3])^(Pnt[num-1]-Pnt[num-2]);
				vectZ.Normalize();
				CMatrix4	mat;
				mat.SetRotateV(vectZ, -PI_2);
				eT = v2 * mat;

				double d2 = (Pnt[num-3] - Pnt[num-2]).GetLength() + (Pnt[num-2] - Pnt[num-1]).GetLength();
				if( eT.GetLength() > d2 - TOLER )
				{
					eT.Normalize();
					eT *= d2;
				}
			}
		}
	}  

	CP_Point3D GetArcCenterp(CP_Point3D p1, CP_Point3D p2, CP_Point3D p3, double &angle, double &radius)
	{
		CP_Point3D mp1, mp2;
		mp1 = (p1 + p2) / 2;
		mp2 = (p2 + p3) / 2;
		CP_Vector3D v1, v2;
		v1 = p2-p1;
		v2 = p3-p2;
		v1.Normalize();
		v2.Normalize();

		CP_Vector3D vectZ = v1^v2;

		ASSERT(vectZ.GetLength() > TOLER);

		vectZ.Normalize();
		CMatrix4	mat;
		mat.SetRotateV(vectZ, PI_2);
		CP_Vector3D v3 = v1 * mat;
		CP_Vector3D v4 = v2 * mat;

		CP_StraightLine line1(mp1, v3), line2(mp2, v4);

		CP_Point3D intp;
		LinesIntersectAt(line1, line2, intp);

		CP_Vector3D v5 = p1 - intp;
		CP_Vector3D v6 = p3 - intp;

		radius = v5.GetLength();
		angle = acos((v5*v6)/(radius*radius));
		mat.SetRotateV(vectZ, angle);
		if((v5*mat - v6).GetLength() > TOLER)
			angle = PI2-angle;

		return intp;
	}


	int LinesIntersectAt (CP_StraightLine &ln1, CP_StraightLine &ln2, CP_Point3D &pi) 
	{
		if (ln1.m_vn.GetLength() < TOLER || ln2.m_vn.GetLength() < TOLER )
			return -1;

		const double eps = 1e-4;
		ln1.m_vn.Normalize() ;
		ln2.m_vn.Normalize() ;
		CP_Vector3D pp = ln2.m_pa - ln1.m_pa;  
		if ( fabs(pp * (ln1.m_vn ^ ln2.m_vn)) > eps )
			return 0;//异面直线
		else
		{    
			if ( CP_Vector3D(ln1.m_vn ^ ln2.m_vn).GetLength() <= TOLER )//平行
				if ( CP_Vector3D(ln1.m_vn ^ pp).GetLength() > eps )
					return 0;//平行不重合
				else
					return 2;//重合                 
			else//相交不重合
			{   
				double t, Dxy, Dyz, Dzx;
				Dxy = ln1.m_vn.m_x * ln2.m_vn.m_y - ln2.m_vn.m_x * ln1.m_vn.m_y;
				Dyz = ln1.m_vn.m_y * ln2.m_vn.m_z - ln2.m_vn.m_y * ln1.m_vn.m_z;
				Dzx = ln1.m_vn.m_z * ln2.m_vn.m_x - ln2.m_vn.m_z * ln1.m_vn.m_x;
				if ( fabs(Dxy) > fabs(Dyz) )
				{
					if ( fabs(Dxy) > fabs(Dzx) )    // |Dxy| is max
						t = (pp.m_x * ln2.m_vn.m_y - pp.m_y * ln2.m_vn.m_x) / Dxy;
					else                            // |Dzx| is max
						t = (pp.m_z * ln2.m_vn.m_x - pp.m_x * ln2.m_vn.m_z) / Dzx;
				}
				else
				{
					if ( fabs(Dyz) > fabs(Dzx) )    // |Dyz| is max
						t = (pp.m_y * ln2.m_vn.m_z - pp.m_z * ln2.m_vn.m_y) / Dyz;
					else                            // |Dzx| is max
						t = (pp.m_z * ln2.m_vn.m_x - pp.m_x * ln2.m_vn.m_z) / Dzx;
				}
				pi = ln1.m_pa + ln1.m_vn * t;
				return 1;                       
			}
		}
	}               

	bool IsIsoparametric(CP_Nurbs *pNurbs0, CP_Nurbs *pNurbs1) {
		if (pNurbs0->GetDegree() != pNurbs1->GetDegree())
			return false;
		if (pNurbs0->GetKnotNumber() != pNurbs1->GetKnotNumber())
			return false;
		for (int i = 0; i < pNurbs0->GetKnotNumber(); ++i) {
			if (abs(pNurbs0->GetKnotValue(i) - pNurbs1->GetKnotValue(i)) > TOLER)
				return false;
		}
		return true;
	}

	// Function: Quick test box intersection
	bool QuickRejection(CP_LineSegment3D *l1, CP_LineSegment3D *l2) {
		return (max(l1->m_startPt.m_x, l1->m_endPt.m_x)
			>= min(l2->m_startPt.m_x, l2->m_endPt.m_x)) &&
			(max(l2->m_startPt.m_x, l2->m_endPt.m_x)
			>= min(l1->m_startPt.m_x, l1->m_endPt.m_x)) &&
			(max(l1->m_startPt.m_y, l1->m_endPt.m_y)
			>= min(l2->m_startPt.m_y, l2->m_endPt.m_y)) &&
			(max(l2->m_startPt.m_y, l2->m_endPt.m_y)
			>= min(l1->m_startPt.m_y, l1->m_endPt.m_y));
	}

	// Function: Straddles test if one line segment straddles the other
	bool StraddlesTest(CP_LineSegment3D *l1, CP_LineSegment3D *l2) {
		CP_Vector3D v1(l1->m_startPt - l2->m_startPt);
		CP_Vector3D v2(l2->m_endPt - l2->m_startPt);
		CP_Vector3D v3(l1->m_endPt - l2->m_startPt);
		return (v1 ^ v2) * (v2 ^ v3) > 0; 
	}

	// Function: Test if two line segments intersect 
	bool IsLinesegIntersect(CP_LineSegment3D *line1, CP_LineSegment3D *line2) {
		return QuickRejection(line1, line2) &&
			StraddlesTest(line1, line2) &&
			StraddlesTest(line2, line1);
	}

	bool IsLinesegOverlap(CP_LineSegment3D *line1, CP_LineSegment3D *line2) {
		return line1->m_startPt == line2->m_startPt && line1->m_endPt == line2->m_endPt ||
			line1->m_startPt == line2->m_endPt && line1->m_endPt == line2->m_startPt;
	}

	double CalcAverageCurvature(CP_Curve3D *pCurve) {
		const int numSeg = 10;
		double sum = 0;
		for (int i = 1; i < numSeg; ++i) {
			sum += pCurve->GetCurvature((double)i/numSeg);
		}
		return sum / (numSeg-1);
	}

}