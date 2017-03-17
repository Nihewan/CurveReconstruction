#pragma once
#include <iterator>
#include <Eigen/Dense>
#include "CP_FlowComplex.h"
class SurfaceOptimization
{
public:
	int refine_max_iter;
	CP_FlowComplex *m_FlowComplex;
public:
	SurfaceOptimization();
	SurfaceOptimization(CP_FlowComplex *fc):m_FlowComplex(fc){};
	~SurfaceOptimization(void);
	void SetTrianglePatch();
	void ResetPointsAndAdjacent();
	void ElimateDegenerateTriangles();
	void ElimateDegenerateTrianglesInPatch();
	void Remesh();
	void ComputeCircumCenter(CP_Triganle3D *tri);
	bool obtusetri(const CP_Triganle3D &tri);
	void FlipEdge(CP_Triganle3D &ltri,CP_Triganle3D &rtri,int lidx,int ridx);
	bool IsFlippable(const CP_Point3D& h,const CP_Point3D& ho,const CP_Point3D& hnext,const CP_Point3D& honext);
	CP_Vector3D local_plane_coords(
		const CP_Point3D& p, const CP_Vector3D& v0, const CP_Vector3D& v1, const CP_Vector3D& plane_n, const CP_Vector3D& plane_origin
		);
	void SplitEdge(CP_Triganle3D &ltri,CP_Triganle3D &rtri,int lidx,int ridx);
	bool CollapseEdge(int p,int q);
	bool IsCollapsable(int p,int q,int newidx);
	double ComputeMeanEdgeLength(const CP_Patch& pPatch);
	void ComputeVertexUpdatePosition(const CP_Patch& pPatch);
	void ComputeRMFForCycle(CP_Patch* pPatch);
	double ComputeCycleCost(const vector<int> &path,vector<CP_Vector3D>& normal,const vector<int>& coner);
	void FlipAndCollapseAfterRefine();
	void InterpolateNormalValue(map<int,double>& mp_normal_value,CP_Patch* pPatch,vector<double>& res);
	void GetInteriorPointsAndRing(CP_Patch* pPatch);
	void LaplaceInterpolateNormals();
	void NormalAjustment();
	void VSpringModelFairing();
};

