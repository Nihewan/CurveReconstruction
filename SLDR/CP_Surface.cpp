
#include "stdafx.h"
#include "CP_Surface.h"
#include "gl/GL.h"
#include "base_geometricoper.h"
#include "CLog.h"
#include <algorithm>

/************************************************************************/
/* CP_Surface3D                                                         */
/************************************************************************/
void CP_Surface3D::DrawSolid(bool ) {
	if (m_pMesh) {
		for (int i = 0; i < m_pMesh->GetTriangleSize(); ++i) {
			CP_MeshTriangle3D *pTri = m_pMesh->GetTriangle(i);
			CP_MeshVertex3D *pVt0 = pTri->GetVert(0);
			CP_MeshVertex3D *pVt1 = pTri->GetVert(1);
			CP_MeshVertex3D *pVt2 = pTri->GetVert(2);
			CP_Vector3D n0 = pVt0->GetVtNormal();
			CP_Vector3D n1 = pVt1->GetVtNormal();
			CP_Vector3D n2 = pVt2->GetVtNormal();

			glBegin(GL_POLYGON);
			glNormal3d(n0.m_x, n0.m_y, n0.m_z);
			glVertex3d(pVt0->x(), pVt0->y(), pVt0->z());
			glNormal3d(n1.m_x, n1.m_y, n1.m_z);
			glVertex3d(pVt1->x(), pVt1->y(), pVt1->z());
			glNormal3d(n2.m_x, n2.m_y, n2.m_z);
			glVertex3d(pVt2->x(), pVt2->y(), pVt2->z());
			glEnd();
		}
	}
}

void CP_Surface3D::DrawMesh() {
	if (m_pMesh) {
		for (int i = 0; i < m_pMesh->GetTriangleSize(); ++i) {
			CP_MeshTriangle3D *pTri = m_pMesh->GetTriangle(i);
			glBegin(GL_LINE_LOOP);
			glVertex3d(pTri->GetVert(0)->x(), pTri->GetVert(0)->y(), pTri->GetVert(0)->z());
			glVertex3d(pTri->GetVert(1)->x(), pTri->GetVert(1)->y(), pTri->GetVert(1)->z());
			glVertex3d(pTri->GetVert(2)->x(), pTri->GetVert(2)->y(), pTri->GetVert(2)->z());
			glEnd();
		}
	}
}


/************************************************************************/
/* CP_Plane                                                             */
/************************************************************************/

/*
CP_Plane::CP_Plane( )
{
} 
*/

CP_Plane::CP_Plane(CP_Point3D *pt, const CP_Vector3D& n)
	: m_pKnownPt(pt), m_normal(n)
{
} 

CP_Point3D CP_Plane::GetPoint(double u, double v) const
{
	ASSERT(false);
	return CP_Point3D();
}

CP_Vector3D CP_Plane::GetNormal(double u, double v) const
{
	return m_normal;
}

CP_Vector3D CP_Plane::GetUnitNormal(double u, double v) const
{
	CP_Vector3D vz = m_normal;
	vz.Normalize( );
	return vz;
}

double CP_Plane::GetPointDepth(const CP_Point3D &ptDepUnknown) const {
	CP_Vector3D vec = ptDepUnknown - *m_pKnownPt;
	return m_pKnownPt->m_z + base_geometric::GetVectorZ(vec, m_normal);
}


void CP_Plane::DrawSolid(bool normalFlag)
{
	CP_Surface3D::DrawSolid();
}

void CP_Plane::DrawMesh( )
{
	CP_Surface3D::DrawMesh();
} 

/************************************************************************/
/* CP_NurbsSurface                                                      */
/************************************************************************/

CP_NURBSSurface::CP_NURBSSurface(int uOrder, int vOrder, int numCtrlU, int numCtrlV) :
	m_uOrder(uOrder), m_vOrder(vOrder)
{
	Init(numCtrlU, numCtrlV);
}

CP_NURBSSurface::CP_NURBSSurface(int uOrder, int vOrder, int numCtrlU, int numCtrlV, vector<double> &uKnotVec, vector<double> &vKnotVec)
	: m_uOrder(uOrder), m_vOrder(vOrder)
{
	Init(numCtrlU, numCtrlV);
	m_uKnotVec = uKnotVec;
	m_vKnotVec = vKnotVec;
}

CP_NURBSSurface::~CP_NURBSSurface()
{
}

void CP_NURBSSurface::Init(int numCtrlU, int numCtrlV) {
	for (int i = 0; i < numCtrlU; ++i) {
		m_ctrlPts.push_back(vector<CP_Point3D>(numCtrlV));
		m_weight.push_back(vector<double>(numCtrlV));
	}
}

void	CP_NURBSSurface::AddUKnot(double u) {
	m_uKnotVec.push_back(u);
}

void	CP_NURBSSurface::AddVKnot(double v) {
	m_vKnotVec.push_back(v);
}

double	CP_NURBSSurface::GetUKnot(int index) const {
	ASSERT(index < (int)m_uKnotVec.size() && index >= 0);
	return m_uKnotVec[index];
}

double	CP_NURBSSurface::GetVKnot(int index) const {
	ASSERT(index < (int)m_vKnotVec.size() && index >= 0);
	return m_vKnotVec[index];
}

int	CP_NURBSSurface::GetUIntervalIndex(double u) const {
	int a, b, c;
	a = m_uOrder - 1;
	b = GetUCtrlNum();
	u -= TOLER;
	if (u<= GetUKnot(a))
		return(a); // Yong: 981119
	if (u>= GetUKnot(b))
		return(b); // Yong: 981119
	while (b - a > 1)
	{
		c = (a + b) / 2;
		if (u <= GetUKnot(c) - TOLER)
			b = c;
		else
			a = c;
	}
	return a;
}

int CP_NURBSSurface::GetVIntervalIndex(double v) const {
	int a, b, c;
	a = m_vOrder - 1;
	b = GetVCtrlNum();
	v -= TOLER;
	if (v<= GetVKnot(a))
		return(a); // Yong: 981119
	if (v>= GetVKnot(b))
		return(b); // Yong: 981119
	while (b - a > 1)
	{
		c = (a + b) / 2;
		if (v <= GetVKnot(c) - TOLER)
			b = c;
		else
			a = c;
	}
	return a;
}

double CP_NURBSSurface::GetWeight(int i, int j) const {
	ASSERT(i < GetUCtrlNum() && j < GetVCtrlNum() && i >= 0 && j >= 0);
	return m_weight[i][j];
}

int CP_NURBSSurface::GetUCtrlNum() const {
	return m_ctrlPts.size();
}

int CP_NURBSSurface::GetVCtrlNum() const {
	return m_ctrlPts[0].size();
}

CP_Point3D CP_NURBSSurface::GetCtrlPoint(int i, int j) const {
	ASSERT(i < GetUCtrlNum() && j < GetVCtrlNum());
	return m_ctrlPts[i][j];
}

void CP_NURBSSurface::SetCtrlPoint(int i, int j, const CP_Point3D &pt, double weight) {
	ASSERT(i < GetUCtrlNum() && j < GetVCtrlNum() && i >= 0 && j >= 0);
	m_ctrlPts[i][j] = pt;
	m_weight[i][j] = weight;
}

//计算参数u处B样条基函数
double** CP_NURBSSurface::CalcN(double u, const vector<double> &knot, int n, int order) const
{
	vector<double> t(knot);

	double** N = new double*[n+order];
	for(int i=0; i<n+order; i++)
		N[i] = new double[order+1];

	for(int i=0; i<n+order; i++)
		if(t[i] <= u && u <= t[i+1])
			N[i][0] = 1.0f;
		else
			N[i][0] = 0.0f;

	for(int k=1; k<=order; k++)
	{
		for(int i=0; i<n+order-k; i++)
		{
			if(t[k] <= u && u <= t[n])
			{
				N[i][k] = 0;
				if(t[i+k] - t[i] != 0)
					N[i][k] += (u - t[i]) * N[i][k-1] / (t[i+k] - t[i]);
				if(t[i+k+1] - t[i+1] != 0)
					N[i][k] += (t[i+k+1] - u) * N[i+1][k-1] / (t[i+k+1] - t[i+1]);
			}
		}
	}

	return N;
}

//计算参数u中B样条基函数的一阶导数
double** CP_NURBSSurface::CalcND1(double u, const vector<double> &knot, int n, int order) const
{
	vector<double> t(knot);
	double** N = CalcN(u, knot, n, order);

	double** N1 = new double*[n+order];
	for(int i=0; i<n+order; i++)
		N1[i] = new double[order+1];

	for(int i=0; i<n+order; i++)
		N1[i][0] = 0.0f;

	for(int i=0; i<=order; i++)
		N1[0][i] = 0;

	for(int k=1; k<=order; k++)
	{
		for(int i=0; i<n+order-k; i++)
		{
			if(t[k] <= u && u <= t[n])
			{
				N1[i][k] = 0;		
				if(t[i+k] - t[i] != 0)
					N1[i][k] += (N[i][k-1] + (u - t[i]) * N1[i][k-1]) / (t[i+k] - t[i]);
				if(t[i+k+1] - t[i+1] != 0)
					N1[i][k] += (-N[i+1][k-1] + (t[i+k+1] - u) * N1[i+1][k-1]) / (t[i+k+1] - t[i+1]);
				N1[i][k] *= k;

			}
		}
	}

	for(int i=0; i<n+order; i++)
		delete[] N[i];
	delete[] N;
	return N1;
}


double* CP_NURBSSurface::CalcNu(double u) const
{
	int n = GetUCtrlNum(), order = m_uOrder - 1;

	double** N = CalcN(u, m_uKnotVec, n, order);

	double* ret = new double[n];
	for(int i=0; i<n; i++)
		ret[i] = N[i][order];

	for(int i=0; i<n+order; i++)
		delete[] N[i];
	delete[] N;
	return ret;
}

double* CP_NURBSSurface::CalcNV(double v) const
{
	int n = GetVCtrlNum(), order = m_vOrder - 1;

	double** N = CalcN(v, m_vKnotVec, n, order);

	double* ret = new double[n];
	for(int i=0; i<n; i++)
		ret[i] = N[i][order];

	for(int i=0; i<n+order; i++)
		delete[] N[i];
	delete[] N;
	return ret;
}

double* CP_NURBSSurface::CalcNuD1(double u) const
{
	int n = GetUCtrlNum(), order = m_uOrder - 1;

	double** N = CalcND1(u, m_uKnotVec, n, order);

	double* ret = new double[n];
	for(int i=0; i<n; i++)
		ret[i] = N[i][order];

	for(int i=0; i<n+order; i++)
		delete[] N[i];
	delete[] N;
	return ret;
}

double* CP_NURBSSurface::CalcNvD1(double v) const
{
	int n = GetVCtrlNum(), order = m_vOrder - 1;

	double** N = CalcND1(v, m_vKnotVec, n, order);

	double* ret = new double[n];
	for(int i=0; i<n; i++)
		ret[i] = N[i][order];

	for(int i=0; i<n+order; i++)
		delete[] N[i];
	delete[] N;
	return ret;
}

//计算NURBS函数(d/du)R(u,v)
double** CP_NURBSSurface::CalcRDU(double u, double v) const 
{
	double* Nu = CalcNu(u);
	double* Nv = CalcNV(v);
	double* NuD1 = CalcNuD1(u);

	double** R = new double*[GetUCtrlNum()];
	for(int i=0; i < GetUCtrlNum(); i++)
		R[i] = new double[GetVCtrlNum()];

	double sum = 0;
	for(int i=0; i<GetUCtrlNum(); i++)
		for(int j=0; j<GetVCtrlNum(); j++)
			sum += Nu[i] * Nv[j] * GetWeight(i, j);

	double sumD = 0;
	for(int i=0; i<GetUCtrlNum(); i++)
		for(int j=0; j<GetVCtrlNum(); j++)
			sumD += NuD1[i] * Nv[j] * GetWeight(i, j);

	for(int i=0; i<GetUCtrlNum(); i++)
		for(int j=0; j<GetVCtrlNum(); j++)
			if(sum == 0)
				R[i][j] = 0;
			else
				R[i][j] = (GetWeight(i, j) * NuD1[i] * Nv[j] * sum -  GetWeight(i, j) * Nu[i] * Nv[j] * sumD) / (sum * sum);


	delete[] NuD1;
	delete[] Nu;
	delete[] Nv;

	return R;
}

//计算NURBS函数(d/dv)R(u,v)
double** CP_NURBSSurface::CalcRDv(double u, double v) const
{
	double* Nu = CalcNu(u);
	double* Nv = CalcNV(v);
	double* NvD1 = CalcNvD1(v);

	double** R = new double*[GetUCtrlNum()];
	for(int i=0; i<GetUCtrlNum(); i++)
		R[i] = new double[GetVCtrlNum()];

	double sum = 0;
	for(int i=0; i<GetUCtrlNum(); i++)
		for(int j=0; j<GetVCtrlNum(); j++)
			sum += Nu[i] * Nv[j] * GetWeight(i, j);

	double sumD = 0;
	for(int i=0; i<GetUCtrlNum(); i++)
		for(int j=0; j<GetVCtrlNum(); j++)
			sumD += Nu[i] * NvD1[j] * GetWeight(i, j);

	for(int i=0; i<GetUCtrlNum(); i++)
		for(int j=0; j<GetVCtrlNum(); j++)
			if(sum == 0)
				R[i][j] = 0;
			else
				R[i][j] = (GetWeight(i, j) * Nu[i] * NvD1[j] * sum -  GetWeight(i, j) * Nu[i] * Nv[j] * sumD) / (sum * sum);

	delete[] NvD1;
	delete[] Nu;
	delete[] Nv;

	return R;
}

CP_Vector3D CP_NURBSSurface::CalcDU(double u, double v) const 
{
	double** R = CalcRDU(u, v);
	int n = GetUCtrlNum(), m = GetVCtrlNum();
	CP_Vector3D p(0, 0, 0);
	for(int i=0; i<n; i++)
		for(int j=0; j<m; j++) {
			CP_Point3D pt = R[i][j] * GetCtrlPoint(i, j);
			p +=  CP_Vector3D(pt.m_x, pt.m_y, pt.m_z);
		}

		for(int i=0; i<n; i++)
			delete[] R[i];
		delete[] R;

		return p;
}

CP_Vector3D CP_NURBSSurface::CalcDV(double u, double v) const
{
	double** R = CalcRDv(u, v);
	int n = GetUCtrlNum(), m = GetVCtrlNum();
	CP_Vector3D p(0, 0, 0);
	for(int i=0; i<n; i++)
		for(int j=0; j<m; j++) {
			CP_Point3D pt = R[i][j] * GetCtrlPoint(i, j);
			p +=  CP_Vector3D(pt.m_x, pt.m_y, pt.m_z);
		}

		for(int i=0; i<n; i++)
			delete[] R[i];
		delete[] R;

		return p;
}

CP_Point3D CP_NURBSSurface::GetPoint(double u, double v) const {
	int			i, k, r, ju, jv, mu, mv, numberu, numberv;
	//------------------------------------------------------------------
	//Yupiqiang 2003/07/21
	double		/*E[4], F[4],*/*E, *F, fz1, fz2, fm;
	CP_Vector3D	*C, *D; //C[4], D[4];
	C = (CP_Vector3D*) alloca(sizeof(CP_Vector3D)*(m_uOrder + m_vOrder));
	D = C + m_vOrder;
	E = (double*) alloca(sizeof(double)*(m_uOrder + m_vOrder));
	F = E + m_vOrder;
	//E=new double[surface.orderv];
	//C=new Vector3D[surface.orderv];
	//F=new double[surface.orderu];
	//   D=new Vector3D[surface.orderu];
	//-----------------------------------------------------------------
	numberu = GetUCtrlNum();
	numberv = GetVCtrlNum();
	jv = GetVIntervalIndex(v);
	ju = GetUIntervalIndex(u);
	mv = jv - m_vOrder + 1;
	mu = ju - m_uOrder + 1;

	for( k = mu; k <= ju; k++ )
	{
		for( i = mv; i <= jv; i++ )
		{
			E[i-mv] = GetWeight(k, i);
			C[i-mv] = (GetCtrlPoint(k,i) - ZeroP3D)*E[i-mv];
		}
		for( r = 1; r <= m_vOrder - 1; r++ )
		{
			for( i = mv; i <= jv - r; i++ )
			{
				fz1 = GetVKnot(i + m_vOrder) - v;
				fz2 = v - GetVKnot(i + r);
				fm  = GetVKnot(i + m_vOrder) - GetVKnot(i + r);
				E[i-mv] = (E[i-mv]*fz1 + E[i-mv+1]*fz2)/fm;
				C[i-mv] = (C[i-mv]*fz1 + C[i-mv+1]*fz2)/fm;
			}
		}
		F[k-mu] = E[0];
		D[k-mu] = C[0];
	}

	for( r = 1; r <= m_uOrder - 1; r++ )
	{
		for( i = mu; i <= ju - r; i++ )
		{
			fz1 = GetUKnot(i + m_uOrder) - u;
			fz2 = u - GetUKnot(i + r);
			fm  = GetUKnot(i + m_uOrder) - GetUKnot(i + r);
			F[i-mu] = (F[i-mu]*fz1 + F[i-mu+1]*fz2)/fm;
			D[i-mu] = (D[i-mu]*fz1 + D[i-mu+1]*fz2)/fm;
		}
	}
	CP_Vector3D resultVec(D[0]/F[0]);
	CP_Point3D resultPt(resultVec.m_x, resultVec.m_y, resultVec.m_z);

	return resultPt;
}

CP_Vector3D CP_NURBSSurface::GetNormal(double u, double v) const {
	CP_Vector3D du = CalcDU(u, v);
	CP_Vector3D dv = CalcDV(u, v);

	return du ^ dv;
}

CP_Vector3D CP_NURBSSurface::GetUnitNormal(double u, double v) const {
	CP_Vector3D n = GetNormal(u, v);
	n.Normalize();
	return n;
}

double CP_NURBSSurface::GetPointDepth(const CP_Point3D &pt) const {
	double step = 0.25;
	double minDis = 10000;
	CP_Point3D closestPt;
	double refU = 0.5, refV = 0.5;

	const double DIS_TORLER = 1.0;
	const double STEP_TORLER = 0.001;
	const double MIN_DEPTH = -10000;

	// Search 
	while (minDis > DIS_TORLER && step > STEP_TORLER) 
	{
		double direct[8][2] = {{0, step}, {0, -step}, {step, 0}, {-step, 0}, 
		{step, step}, {-step, -step}, {step, -step}, {-step, step}};
		int minDirect = 0;
		for (int i = 0; i < 8; ++i) {
			CP_Point3D refPt = GetPoint(refU + direct[i][0], refV + direct[i][1]);
			CP_Vector3D v = pt - refPt;
			v.m_z = 0;
			double dis = v.GetLength();
			if (minDis > dis) {
				minDis = dis;
				closestPt = refPt;
				minDirect = i;
			}
		}
		refU += direct[minDirect][0];
		refV += direct[minDirect][1];
		if (refU < 0 || refU > 1 || refV < 0 || refV > 1)
			return MIN_DEPTH;
		step /= 2;
	}
	return closestPt.m_z;
}

void CP_NURBSSurface::DrawControlNet() {
	// Control net
	glColor3f(1.0, 0, 0);
	for (int i = 0; i < GetUCtrlNum() - 1; ++i) {
		for (int j = 0; j < GetVCtrlNum()-1; ++j) {
			CP_Point3D pt0 = GetCtrlPoint(i, j);
			CP_Point3D pt1 = GetCtrlPoint(i+1, j);
			CP_Point3D pt2 = GetCtrlPoint(i+1, j+1);
			CP_Point3D pt3 = GetCtrlPoint(i, j+1);

			glBegin(GL_LINE_LOOP);
			glVertex3d(pt0.m_x, pt0.m_y, pt0.m_z);
			glVertex3d(pt1.m_x, pt1.m_y, pt1.m_z);
			glVertex3d(pt2.m_x, pt2.m_y, pt2.m_z);
			glVertex3d(pt3.m_x, pt3.m_y, pt3.m_z);
			glEnd();
		}
	}
}

void CP_NURBSSurface::DrawSolid(bool normalFlag/* =true */) {
	// rawControlNet();
	CP_Surface3D::DrawSolid();
}

void CP_NURBSSurface::DrawMesh() {
	CP_Surface3D::DrawMesh();
}

/************************************************************************/
/* CP_CoonsSurface                                                      */
/************************************************************************/
CP_CoonsSurface::CP_CoonsSurface() {
	m_boundaries[0] = NULL;
	m_boundaries[1] = NULL;
	m_boundaries[2] = NULL;
	m_boundaries[3] = NULL;

	m_sameDirection[0] = true;
	m_sameDirection[1] = true;
	m_sameDirection[2] = true;
	m_sameDirection[3] = true;
}

CP_CoonsSurface::~CP_CoonsSurface() {

}

CP_Point3D CP_CoonsSurface::GetPoint(double u, double v) const {
	if (u > 1) u = 1;
	if (v > 1) v = 1;
	if (u < 0) u = 0;
	if (v < 0) v = 0;

	double v0 = m_sameDirection[0]? v : 1-v;
	double v1 = m_sameDirection[1]? v : 1-v;
	double u0 = m_sameDirection[2]? u : 1-u;
	double u1 = m_sameDirection[3]? u : 1-u;

	return (1-u) * GetBoundaryPoint(0, v0) + u * GetBoundaryPoint(1, v1)
		+ (1-v) * GetBoundaryPoint(2, u0) + v * GetBoundaryPoint(3, u1) 
		+ (-(1-u) * v * *m_pEndPts[2]) + (- u * (1-v) * *m_pEndPts[1])
		+ (- (1-u) * (1-v) * *m_pEndPts[0]) + (- u * v * *m_pEndPts[3]);
}

CP_Point3D CP_CoonsSurface::GetBoundaryPoint(int i, double t) const {
	// The 3rd boundary is set to NULL when coons face consist of 3 edges.
	if (m_boundaries[i] != NULL)
		return m_boundaries[i]->GetPoint(t);

	return *m_pEndPts[2]; // Return the end point.
}

double CP_CoonsSurface::GetPointDepth(const CP_Point3D &pt) const {
	double step = 0.25;
	double minDis = 10000;
	CP_Point3D closestPt;
	double refU = 0.5, refV = 0.5;

	const double DIS_TORLER = 1.0;
	const double STEP_TORLER = 0.001;

	// Search 
	while (minDis > DIS_TORLER && step > STEP_TORLER) {
		double direct[8][2] = {{0, step}, {0, -step}, {step, 0}, {-step, 0}, 
		{step, step}, {-step, -step}, {step, -step}, {-step, step}};
		int minDirect = 0;
		for (int i = 0; i < 8; ++i) {
			CP_Point3D refPt = GetPoint(refU + direct[i][0], refV + direct[i][1]);
			CP_Vector3D v = pt - refPt;
			v.m_z = 0;
			double dis = v.GetLength();
			if (minDis > dis) {
				minDis = dis;
				closestPt = refPt;
				minDirect = i;
			}
		}
		refU += direct[minDirect][0];
		refV += direct[minDirect][1];
		step /= 2;
	}
	return closestPt.m_z;
}

CP_Vector3D CP_CoonsSurface::GetNormal(double u, double v) const {
	return CP_Vector3D(0, 0, 0);
}

CP_Vector3D CP_CoonsSurface::GetUnitNormal(double u, double v) const {
	return CP_Vector3D(0, 0, 0);
}

void CP_CoonsSurface::DrawSolid(bool normalFlag) {
	CP_Surface3D::DrawSolid();
}

void CP_CoonsSurface::DrawMesh( ) {

/*
	if (m_boundaries[0] == NULL)
		return;

	const int numOfBlock = 5;
	double delta = 1.0/numOfBlock;
	for (double u = 0; u < 1 - delta + TOLER; u += delta) {
		for (double v = 0; v < 1 - delta + TOLER; v += delta) {
			CP_Point3D pt0 = GetPoint(u, v);
			CP_Point3D pt1 = GetPoint(u+delta, v);
			CP_Point3D pt2 = GetPoint(u+delta, v+delta);
			glBegin(GL_LINE_LOOP);
			glVertex3d(pt0.m_x, pt0.m_y, pt0.m_z);
			glVertex3d(pt1.m_x, pt1.m_y, pt1.m_z);
			glVertex3d(pt2.m_x, pt2.m_y, pt2.m_z);
			glEnd();
		}
	}*/
	CP_Surface3D::DrawMesh();
}

void CP_CoonsSurface::SplitSurface(double fitErr, vector<double> &usegs, vector<double> &vsegs) {
	vector<double> v1segs;
	m_boundaries[0]->Split2LineSeg(fitErr, vsegs, 0, 1);
	m_boundaries[1]->Split2LineSeg(fitErr, v1segs, 0, 1);

	for (unsigned int i = 0; i < v1segs.size(); ++i) {
		unsigned int j;
		for (j = 0; j < vsegs.size(); ++j) {
			if (abs(vsegs[j] - v1segs[i]) < TOLER) {
				break;
			}
		}
		if (j == vsegs.size())
			vsegs.push_back(v1segs[i]);
	}
	std::sort(vsegs.begin(), vsegs.end());

	vector<double> u1segs;
	m_boundaries[2]->Split2LineSeg(fitErr, usegs, 0, 1);
	if (m_boundaries[3] != NULL)
		m_boundaries[3]->Split2LineSeg(fitErr, u1segs, 0, 1);

	for (unsigned int i = 0; i < u1segs.size(); ++i) {
		unsigned int j;
		for (j= 0; j < usegs.size(); ++j) {
			if (abs(usegs[j] - u1segs[i]) < TOLER) {
				break;
			}
		}
		if (j == usegs.size()) 
			usegs.push_back(u1segs[i]);
	}
	std::sort(usegs.begin(), usegs.end());
}

pair<double, double> CP_CoonsSurface::GetEndPtUV(const CP_Point3D &pt) const {
	if (base_geometric::IsSamePoint(m_pEndPts[0], &pt))
		return pair<double, double>(0, 0);
	if (base_geometric::IsSamePoint(m_pEndPts[1], &pt)) 
		return pair<double, double>(1, 0);
	if (base_geometric::IsSamePoint(m_pEndPts[2], &pt))
		return pair<double, double>(0, 1);
	if (base_geometric::IsSamePoint(m_pEndPts[3], &pt))
		return pair<double, double>(1, 1);
	throw exception("<U, V> not found.");
}