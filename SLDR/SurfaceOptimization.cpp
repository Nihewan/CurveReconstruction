#include "stdafx.h"
#include "SurfaceOptimization.h"
#include <iostream>
using namespace Eigen;

SurfaceOptimization::SurfaceOptimization()
{
	refine_max_iter=4;
}

SurfaceOptimization::~SurfaceOptimization(void)
{

}

void SurfaceOptimization::ElimateDegenerateTriangles()
{
	//预处理，消除退化三角形和remesh
	SetTrianglePatch();
	ResetPointsAndAdjacent();
	ElimateDegenerateTrianglesInPatch();
	Remesh();
	//FlipAndCollapseAfterRefine();
	//RMF法向
	LaplaceInterpolateNormals();
	NormalAjustment();

	
	cout<<"ok2"<<endl;
}

void SurfaceOptimization::ElimateDegenerateTrianglesInPatch()
{
	//between 2cells in the same patch
	//1.定位caps
	for(unsigned int n=0;n<m_FlowComplex->m_patches.size();++n){
		CP_Patch *pPatch = m_FlowComplex->m_patches[n];
		if(pPatch->flag&&pPatch->delaunay==0){
			for(unsigned int j=0;j<pPatch->m_triangle.size();++j){
				CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch->m_triangle[j]];
				for(int i=0;i<3;i++)
				{
					CP_Vector3D i1(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_z);
					CP_Vector3D i2(m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_z);
					i1.Normalize();
					i2.Normalize();
					if(i1*i2<=cos(120*PI/180)){
						//本来caps的夹角要接近180的才算退化，但IFC中钝角实在太多，且角度越接近60度越好，所以选择120
						vector<int> v=m_FlowComplex->GetIncidentTri(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]],m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]]);
						//长边不是曲线边，flip swapedge
						for(unsigned int k=0;k<v.size();k++)
						{
							if(pPatch->m_triangle[j]!=v[k]){
								CP_Triganle3D *opTri = m_FlowComplex->tricells[v[k]];
								if(opTri->_patch==tri->_patch){
									FlipEdge(*tri,*opTri,pPatch->m_triangle[j],v[k]);
								}
							}
						}
						break;
					}//caps i1*i2
				}//i
			}//j
		}//flag
	}//n
	 
	//2.定位needles
	for(unsigned int n=0;n<m_FlowComplex->m_patches.size();++n){
		CP_Patch *pPatch = m_FlowComplex->m_patches[n];
		if(pPatch->flag&&pPatch->delaunay==0){
			for(unsigned int j=0;j<pPatch->m_triangle.size();++j){
				CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch->m_triangle[j]];
				if(tri->_patch!=-1){
					for(int i=0;i<3;i++)
					{
						CP_Vector3D i1(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_z);
						CP_Vector3D i2(m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_z);
						i1.Normalize();
						i2.Normalize();
						if(i1*i2>=cos(30*PI/180)){
							if((m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].incidentpoly.size()==0&&m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].incidentpoly.size()==0)){
								int p=tri->m_points[(i+1)%3];
								int q=tri->m_points[(i+2)%3];
								CollapseEdge(p,q);
								break;
							}
						}//30
					}//i
				}//_patch!=-1
			}//j
			//从面片中清除已无效的三角形
			for(vector<int>::iterator it=pPatch->m_triangle.begin();it!=pPatch->m_triangle.end();){
				if(m_FlowComplex->tricells[*it]->_patch==-1){
					pPatch->m_triangle.erase(it);
				}else
					++it;
			}
		}
	}//n
}

void SurfaceOptimization::Remesh(){
	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();++i){
		CP_Patch *pPatch = m_FlowComplex->m_patches[i];
		if(pPatch->flag){
			int max_iter=refine_max_iter;
			double mean_edge_length=ComputeMeanEdgeLength(*pPatch);
			
			for(int iter=max_iter;iter>=0;--iter){
				//1.split edge
				unsigned int tri_size=pPatch->m_triangle.size();
				for(unsigned int j=0;j<tri_size;++j){
					CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch->m_triangle[j]];
					for(int k=0;k<3;++k){
						if(dist(m_FlowComplex->m_0cells[tri->m_points[k]],m_FlowComplex->m_0cells[tri->m_points[(k+1)%3]])>mean_edge_length*4/3){
							vector<int> v=m_FlowComplex->GetIncidentTri(m_FlowComplex->m_0cells[tri->m_points[k]],m_FlowComplex->m_0cells[tri->m_points[(k+1)%3]]);
							for(unsigned int t=0;t<v.size();t++)
							{
								if(pPatch->m_triangle[j]!=v[t]){
									CP_Triganle3D *opTri = m_FlowComplex->tricells[v[t]];
									if(opTri->_patch==tri->_patch){
										SplitEdge(*tri,*opTri,pPatch->m_triangle[j],v[t]);
									}
								}
							}
						}
					}
				}//j

				//2.collapse edge
				for(unsigned int j=0;j<pPatch->m_triangle.size();++j){
					CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch->m_triangle[j]];
					if(tri->_patch!=-1){
						for(int t=0;t<3;t++)
						{
							CP_Vector3D i1(m_FlowComplex->m_0cells[tri->m_points[(t+1)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[t%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(t+1)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[t%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(t+1)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[t%3]].m_z);
							CP_Vector3D i2(m_FlowComplex->m_0cells[tri->m_points[(t+2)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[t%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(t+2)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[t%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(t+2)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[t%3]].m_z);
							i1.Normalize();
							i2.Normalize();
							if(dist(m_FlowComplex->m_0cells[tri->m_points[(t+1)%3]],m_FlowComplex->m_0cells[tri->m_points[(t+2)%3]])<mean_edge_length*4/5){
								if((m_FlowComplex->m_0cells[tri->m_points[(t+1)%3]].incidentpoly.size()==0&&m_FlowComplex->m_0cells[tri->m_points[(t+2)%3]].incidentpoly.size()==0)){
									int p=tri->m_points[(t+1)%3];
									int q=tri->m_points[(t+2)%3];
									CollapseEdge(p,q);
									break;
								}
							}//30
						}//t
					}//_patch!=-1
				}//j
				//从面片中清除已无效的三角形
				for(vector<int>::iterator it=pPatch->m_triangle.begin();it!=pPatch->m_triangle.end();){
					if(m_FlowComplex->tricells[*it]->_patch==-1){
						pPatch->m_triangle.erase(it);
					}else
						++it;
				}

				//3.flip edge
				for(unsigned int j=0;j<pPatch->m_triangle.size();++j){
					CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch->m_triangle[j]];
					for(int t=0;t<3;t++)
					{
						vector<int> v=m_FlowComplex->GetIncidentTri(m_FlowComplex->m_0cells[tri->m_points[(t+1)%3]],m_FlowComplex->m_0cells[tri->m_points[(t+2)%3]]);						
						for(unsigned int k=0;k<v.size();k++)
						{
							if(pPatch->m_triangle[j]!=v[k]){
								CP_Triganle3D *opTri = m_FlowComplex->tricells[v[k]];
								if(opTri->_patch==tri->_patch){//保证不flip boundary
									int lsri=tri->m_points[t],rsri=0;
									for(int m=0;m<3;++m)
										if(opTri->m_points[m]!=tri->m_points[(t+1)%3]&&opTri->m_points[m]!=tri->m_points[(t+2)%3])
											rsri=opTri->m_points[m];
									int a1=m_FlowComplex->m_0cells[tri->m_points[(t+1)%3]].m_adjTriangle.size();
									int a2=m_FlowComplex->m_0cells[tri->m_points[(t+2)%3]].m_adjTriangle.size();
									int b1=m_FlowComplex->m_0cells[lsri].m_adjTriangle.size();
									int b2=m_FlowComplex->m_0cells[rsri].m_adjTriangle.size();
									int initial=abs(a1-6)+abs(a2-6)+abs(b1-6)+abs(b2-6);
									int after=abs(a1-1-6)+abs(a2-1-6)+abs(b1+1-6)+abs(b2+1-6);
									if(initial>after)
										FlipEdge(*tri,*opTri,pPatch->m_triangle[j],v[k]);
								}
							}
						}
					}//t
				}//j

				ComputeVertexUpdatePosition(*pPatch);
				map<int,int> mp;
				for(unsigned int j=0;j<pPatch->m_triangle.size();++j){
					CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch->m_triangle[j]];
					for(int k=0;k<3;++k){
						if(m_FlowComplex->m_0cells[tri->m_points[k]].incidentpoly.size()==0&&mp.count(tri->m_points[k])==0){
							m_FlowComplex->m_0cells[tri->m_points[k]].m_x=m_FlowComplex->m_0cells[tri->m_points[k]].m_newx;
							m_FlowComplex->m_0cells[tri->m_points[k]].m_y=m_FlowComplex->m_0cells[tri->m_points[k]].m_newy;
							m_FlowComplex->m_0cells[tri->m_points[k]].m_z=m_FlowComplex->m_0cells[tri->m_points[k]].m_newz;
							mp[tri->m_points[k]]++;
						}
					}//k
				}//j
			}//iter
		}//flag
	}//i
}

void SurfaceOptimization::SetTrianglePatch()
{
	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();++i){
		CP_Patch *pPatch = m_FlowComplex->m_patches[i];
		if(pPatch->flag){
			for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
			{
				CP_2cell *p2cell = m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
				for (unsigned int k=0;k<p2cell->m_triangle.size();++k){
					m_FlowComplex->tricells[p2cell->m_triangle[k]]->_patch=i;
					pPatch->m_triangle.push_back(p2cell->m_triangle[k]);//以每个patch为单元，剔除2cell
				}
			}
		}
	}
}

void SurfaceOptimization::ResetPointsAndAdjacent()
{
	//清空
	for(unsigned int i=0;i<m_FlowComplex->m_0cells.size();++i){
		vector<int>().swap(m_FlowComplex->m_0cells[i].m_adjTriangle);
	}
	for(unsigned int i=0;i<m_FlowComplex->tricells.size();++i){
		vector<int>().swap(m_FlowComplex->tricells[i]->m_adjTriangle);
	}
	//Reset
	for(unsigned int i=0;i<m_FlowComplex->tricells.size();i++)
	{
		if(m_FlowComplex->tricells[i]->_patch!=-1){
			for(unsigned int k=0;k<3;k++)
				m_FlowComplex->m_0cells[m_FlowComplex->tricells[i]->m_points[k]].m_adjTriangle.push_back(i);
		}
	}
	for(unsigned int i=0;i<m_FlowComplex->tricells.size();i++)
	{
		if(m_FlowComplex->tricells[i]->_patch!=-1){
			for(unsigned int j=0;j<3;j++)
			{
				vector<int> v=m_FlowComplex->GetIncidentTri(m_FlowComplex->m_0cells[m_FlowComplex->tricells[i]->m_points[j%3]],m_FlowComplex->m_0cells[m_FlowComplex->tricells[i]->m_points[(j+1)%3]]);
				for(unsigned int k=0;k<v.size();k++)
				{
					if(i!=v[k])
						m_FlowComplex->tricells[i]->m_adjTriangle.push_back(v[k]);
				}
			}
		}
	}
}

bool SurfaceOptimization::obtusetri(const CP_Triganle3D &tri)
{
	for(int i=0;i<3;i++)
	{
		CP_Vector3D i1(m_FlowComplex->m_0cells[tri.m_points[(i+1)%3]].m_x-m_FlowComplex->m_0cells[tri.m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri.m_points[(i+1)%3]].m_y-m_FlowComplex->m_0cells[tri.m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri.m_points[(i+1)%3]].m_z-m_FlowComplex->m_0cells[tri.m_points[i%3]].m_z);
		CP_Vector3D i2(m_FlowComplex->m_0cells[tri.m_points[(i+2)%3]].m_x-m_FlowComplex->m_0cells[tri.m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri.m_points[(i+2)%3]].m_y-m_FlowComplex->m_0cells[tri.m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri.m_points[(i+2)%3]].m_z-m_FlowComplex->m_0cells[tri.m_points[i%3]].m_z);
		i1.Normalize();
		i2.Normalize();
		if(i1*i2<=cos(120*PI/180))
			return true;
	}
	return false;
}

void SurfaceOptimization::FlipEdge(CP_Triganle3D &ltri,CP_Triganle3D &rtri,int lidx,int ridx)
{
	int abc=0;
	int sp[2]={-1,-1};//公共边顶点
	int rsri=-1,lsri=-1;;//除公共边外的点在三角形中的序号
	for (unsigned int j = 0; j < 3; j++)
	{
		for (int k = 0; k < 3; k++)
		{//公共边在ltri中j是按序的，如果相应k反序，则方向相逆
			if (ltri.m_points[j]==rtri.m_points[k])
			{   
				sp[abc++]=rtri.m_points[k];
			}
		}
	}

	for (unsigned int j=0;j<3;j++)
	{
		int tmp=-1;
		for(unsigned int k=0;k<2;k++)
		{
			if(ltri.m_points[j]==sp[k])
				tmp=j;
		}
		if(tmp==-1)
			lsri=j;
	}

	for (unsigned int j=0;j<3;j++)
	{
		int tmp=-1;
		for(unsigned int k=0;k<2;k++)
		{
			if(rtri.m_points[j]==sp[k])
				tmp=j;
		}
		if(tmp==-1)
			rsri=j;
	}

	if(IsFlippable(m_FlowComplex->m_0cells[sp[0]],m_FlowComplex->m_0cells[sp[1]],m_FlowComplex->m_0cells[rtri.m_points[rsri]],m_FlowComplex->m_0cells[ltri.m_points[lsri]])){
		//left swap
		for (unsigned int j=0;j<3;j++)
		{
			if((ltri.m_points[j]==sp[0]&&ltri.m_points[(j+1)%3]==sp[1])||(ltri.m_points[(j+1)%3]==sp[0]&&ltri.m_points[j]==sp[1]))
			{
				m_FlowComplex->m_0cells[ltri.m_points[(j+1)%3]].m_adjTriangle.erase(
					remove(m_FlowComplex->m_0cells[ltri.m_points[(j+1)%3]].m_adjTriangle.begin(),
					m_FlowComplex->m_0cells[ltri.m_points[(j+1)%3]].m_adjTriangle.end(),lidx),
					m_FlowComplex->m_0cells[ltri.m_points[(j+1)%3]].m_adjTriangle.end());
				ltri.m_points[(j+1)%3]=rtri.m_points[rsri];
				m_FlowComplex->m_0cells[rtri.m_points[rsri]].m_adjTriangle.push_back(lidx);
				break;
			}
		}

		//right swap
		for (unsigned int j=0;j<3;j++)
		{
			if((rtri.m_points[j]==sp[0]&&rtri.m_points[(j+1)%3]==sp[1])||(rtri.m_points[(j+1)%3]==sp[0]&&rtri.m_points[j]==sp[1]))
			{
				m_FlowComplex->m_0cells[rtri.m_points[(j+1)%3]].m_adjTriangle.erase(
					remove(m_FlowComplex->m_0cells[rtri.m_points[(j+1)%3]].m_adjTriangle.begin(),
					m_FlowComplex->m_0cells[rtri.m_points[(j+1)%3]].m_adjTriangle.end(),ridx),
					m_FlowComplex->m_0cells[rtri.m_points[(j+1)%3]].m_adjTriangle.end());
				rtri.m_points[(j+1)%3]=ltri.m_points[lsri];
				m_FlowComplex->m_0cells[ltri.m_points[lsri]].m_adjTriangle.push_back(ridx);
				break;
			}
		}
	}
}

void SurfaceOptimization::ComputeCircumCenter(CP_Triganle3D *tri)
{
	CP_Vector3D ac = m_FlowComplex->m_0cells[tri->m_points[2]]-m_FlowComplex->m_0cells[tri->m_points[0]];
	CP_Vector3D ab = m_FlowComplex->m_0cells[tri->m_points[1]]-m_FlowComplex->m_0cells[tri->m_points[0]];
	CP_Vector3D abXac = ab^ac;

	// this is the vector from a TO the circumsphere center
	double balength=ab*ab;
	double calength=ac*ac;
	double denominator=0.5/(abXac*abXac);
	CP_Vector3D toCircumsphereCenter;
	toCircumsphereCenter.m_x = ((balength * ac.m_y - calength * ab.m_y) * abXac.m_z -
		(balength * ac.m_z - calength * ab.m_z) * abXac.m_y) * denominator;
	toCircumsphereCenter.m_y = ((balength * ac.m_z - calength * ab.m_z) * abXac.m_x -
		(balength * ac.m_x - calength * ab.m_x) * abXac.m_z) * denominator;
	toCircumsphereCenter.m_z = ((balength * ac.m_x - calength * ab.m_x) * abXac.m_y -
		(balength * ac.m_y - calength * ab.m_y) * abXac.m_x) * denominator;
	tri->radius = toCircumsphereCenter.GetLength();
	// The 3 space coords of the circumsphere center then:
	tri->circumcenter.m_x = m_FlowComplex->m_0cells[tri->m_points[0]].m_x  +  toCircumsphereCenter.m_x ;
	tri->circumcenter.m_y = m_FlowComplex->m_0cells[tri->m_points[0]].m_y  +  toCircumsphereCenter.m_y ;
	tri->circumcenter.m_z = m_FlowComplex->m_0cells[tri->m_points[0]].m_z  +  toCircumsphereCenter.m_z ;// now this is the actual 3space location

	double min_edge_length=MAX_DISTANCE;
	for (int i=0;i<3;++i)
	{
		double len=dist(m_FlowComplex->m_0cells[tri->m_points[i]],m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]]);
		if(len<min_edge_length)
			min_edge_length=len;
	}
	if(tri->radius/min_edge_length>1.414)
		tri->flag=true;
}

void SurfaceOptimization::ComputeRMFForCycle(CP_Patch* pPatch)
{
	if(pPatch->m_bcurve.size()==0)
		return;
	vector<int> polygon;
	vector<bool> visited(pPatch->m_bcurve.size(),false);
	int cur=m_FlowComplex->m_PolyLine[pPatch->m_bcurve[0]].s;
	vector<int> coner;
	polygon.push_back(cur);
	for(unsigned int j=0;j<pPatch->m_bcurve.size();++j){//次数
		for(unsigned int k=0;k<pPatch->m_bcurve.size();++k){//找一头是cur的线
			if(!visited[k]&&cur==m_FlowComplex->m_PolyLine[pPatch->m_bcurve[k]].s){
				int curve=pPatch->m_bcurve[k];
				for(unsigned int m=1;m<m_FlowComplex->m_PolyLine[curve].idx.size();++m){
					polygon.push_back(m_FlowComplex->m_PolyLine[curve].idx[m]);
				}
				cur=m_FlowComplex->m_PolyLine[curve].idx.back();
				visited[k]=true;
				coner.push_back(polygon.size()-1);
			}else if(!visited[k]&&cur==m_FlowComplex->m_PolyLine[pPatch->m_bcurve[k]].e){
				int curve=pPatch->m_bcurve[k];
				for(int m=m_FlowComplex->m_PolyLine[curve].idx.size()-2;m>=0;--m){
					polygon.push_back(m_FlowComplex->m_PolyLine[curve].idx[m]);
				}
				cur=m_FlowComplex->m_PolyLine[curve].idx.front();
				visited[k]=true;
				coner.push_back(polygon.size()-1);
			}
		}
	}
	coner.back()--;
	polygon.pop_back();
	pPatch->path.swap(polygon);
	ComputeCycleCost(pPatch->path,pPatch->r,coner);
	//翻转法向
	for(unsigned int i=0;i<pPatch->path.size();++i)
	for(unsigned int j=0;j<m_FlowComplex->m_0cells[pPatch->path[i]].m_adjTriangle.size();++j){
		CP_Triganle3D* tri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[pPatch->path[i]].m_adjTriangle[j]];
		if(tri->_patch==pPatch->index){
			CP_Vector3D ntmp = (m_FlowComplex->m_0cells[tri->m_points[1]] - m_FlowComplex->m_0cells[tri->m_points[0]]) ^ (m_FlowComplex->m_0cells[tri->m_points[2]] -m_FlowComplex->m_0cells[tri->m_points[0]]);
			ntmp.Normalize();
			if(ntmp*pPatch->r[i]<0)
			{
				for(unsigned int k=0;k<pPatch->r.size();++k)
					pPatch->r[k]*=(-1);
			}
		}
	}

}

bool SurfaceOptimization::IsFlippable(const CP_Point3D& h,const CP_Point3D& ho,const CP_Point3D& hnext,const CP_Point3D& honext)//ho=h opposite
{
	// the two edges involved in the flip

	CP_Vector3D plane_v0 = h - ho;
	CP_Vector3D plane_v1 = honext - hnext;		

	plane_v0.Normalize();
	plane_v1.Normalize();
	// the plane defined by the two edges

	CP_Vector3D plane_n = plane_v1^plane_v0;
	plane_n.Normalize();

	// orthogonalize in-plane vectors

	plane_v0 = plane_v0 - (plane_v0*plane_v1) * plane_v1;
	plane_v0.Normalize();

	CP_Vector3D plane_origin(hnext.m_x,hnext.m_y,hnext.m_z);


	// 2d coordinates in plane

	CP_Vector3D local_t = local_plane_coords(h, plane_v0, plane_v1, plane_n, plane_origin);
	CP_Vector3D local_b = local_plane_coords(ho, plane_v0, plane_v1, plane_n, plane_origin);
	CP_Vector3D local_l = local_plane_coords(hnext, plane_v0, plane_v1, plane_n, plane_origin);
	CP_Vector3D local_r = local_plane_coords(honext, plane_v0, plane_v1, plane_n, plane_origin);


	// check if edge intersections lies inside triangles pair (in plane)

	CP_Vector3D tb = local_t - local_b;
	CP_Vector3D lr = local_l - local_r;

	double ntb[2];
	double ctb;

	double nlr[2];
	double clr;

	ntb[0] = - tb.m_y;
	ntb[1] = tb.m_x;

	ctb = -(ntb[0] * local_t.m_x + ntb[1] * local_t.m_y);


	nlr[0] = - lr.m_y;
	nlr[1] = lr.m_x;

	clr = -(nlr[0] * local_l.m_x + nlr[1] * local_l.m_y);


	double det = ntb[0] * nlr[1] - nlr[0] * ntb[1];

	CP_Vector3D intersection(- (nlr[1] * ctb - ntb[1] * clr) / det, 
		- (-nlr[0] * ctb + ntb[0] * clr) / det, 
		0.0);


	double l0 = (intersection - local_r)* lr / (lr*lr);
	double l1 = (intersection - local_b)*tb / (tb* tb);

	return l0 > 0.0 && l0 < 1.0 && l1 > 0.0 && l1 < 1.0;
}

CP_Vector3D SurfaceOptimization::local_plane_coords(
	const CP_Point3D& p, const CP_Vector3D& v0, const CP_Vector3D& v1, const CP_Vector3D& plane_n, const CP_Vector3D& plane_origin
	) 
{
	CP_Vector3D pv(p.m_x,p.m_y,p.m_z);
	CP_Vector3D q(pv + ((plane_origin - pv)* plane_n) * plane_n);
	return CP_Vector3D((q - plane_origin)* v0, (q - plane_origin)*v1, 0.0);
}

bool SurfaceOptimization::CollapseEdge(int p,int q)
{
	int newidx=m_FlowComplex->m_0cells.size();
	m_FlowComplex->m_0cells.push_back(CP_Point3D((m_FlowComplex->m_0cells[p].m_x+m_FlowComplex->m_0cells[q].m_x)/2.0,(m_FlowComplex->m_0cells[p].m_y+m_FlowComplex->m_0cells[q].m_y)/2.0,(m_FlowComplex->m_0cells[p].m_z+m_FlowComplex->m_0cells[q].m_z)/2.0));
	if(IsCollapsable(p,q,newidx)){
		//删除含pq边的三角形
		set<int> pt;
		vector<int> v=m_FlowComplex->GetIncidentTri(m_FlowComplex->m_0cells[p],m_FlowComplex->m_0cells[q]);
		for(unsigned int k=0;k<v.size();++k){
			CP_Triganle3D* tri=m_FlowComplex->tricells[v[k]];
			tri->_patch=-1;
			for(int i=0;i<3;++i){
				for(vector<int>::iterator it=m_FlowComplex->m_0cells[tri->m_points[i]].m_adjTriangle.begin();it!=m_FlowComplex->m_0cells[tri->m_points[i]].m_adjTriangle.end();){
					if(m_FlowComplex->tricells[*it]->_patch==-1){
						m_FlowComplex->m_0cells[tri->m_points[i]].m_adjTriangle.erase(it);
					}else
						++it;
				}
				pt.insert(tri->m_points[i]);
			}
		}
		//三角形含p、q点的改为newidx
		for(unsigned int k=0;k<m_FlowComplex->m_0cells[p].m_adjTriangle.size();++k){
			CP_Triganle3D* ptri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[p].m_adjTriangle[k]];
			if(ptri->_patch!=-1){
				for(int m=0;m<3;++m){
					if(ptri->m_points[m]==p){
						ptri->m_points[m]=newidx;
						m_FlowComplex->m_0cells[newidx].m_adjTriangle.push_back(m_FlowComplex->m_0cells[p].m_adjTriangle[k]);
					}
				}//m
			}
		}//k
		for(unsigned int k=0;k<m_FlowComplex->m_0cells[q].m_adjTriangle.size();++k){
			CP_Triganle3D* ptri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[q].m_adjTriangle[k]];
			if(ptri->_patch!=-1){
				for(int m=0;m<3;++m){
					if(ptri->m_points[m]==q){
						ptri->m_points[m]=newidx;
						m_FlowComplex->m_0cells[newidx].m_adjTriangle.push_back(m_FlowComplex->m_0cells[q].m_adjTriangle[k]);
					}
				}//m
			}
		}//k
		return true;
	}
	return false;
}

bool SurfaceOptimization::IsCollapsable(int p,int q,int newidx)
{
	set<int> vec[2];
	//Connectivity
	for(unsigned int k=0;k<m_FlowComplex->m_0cells[p].m_adjTriangle.size();++k){
		CP_Triganle3D* ptri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[p].m_adjTriangle[k]];
		for(int m=0;m<3;++m){
			if(ptri->m_points[m]!=p&&ptri->m_points[m]!=q){
				vec[0].insert(ptri->m_points[m]);
			}
		}//m
	}//k
	for(unsigned int k=0;k<m_FlowComplex->m_0cells[q].m_adjTriangle.size();++k){
		CP_Triganle3D* ptri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[q].m_adjTriangle[k]];
		for(int m=0;m<3;++m){
			if(ptri->m_points[m]!=p&&ptri->m_points[m]!=q){
				vec[1].insert(ptri->m_points[m]);
			}
		}//m
	}//k

	map<int,int> mp;
	int num=0;
	for(auto e:vec[0])
		mp[e]++;
	for(auto e:vec[1])
		if(mp.count(e)!=0)
			num++;
	if(num!=2)
		return false;

	//Geometry
	for(unsigned int k=0;k<m_FlowComplex->m_0cells[p].m_adjTriangle.size();++k){
		CP_Triganle3D* ptri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[p].m_adjTriangle[k]];
		if(ptri->_patch!=-1){
			CP_Vector3D ntmp = (m_FlowComplex->m_0cells[ptri->m_points[1]] - m_FlowComplex->m_0cells[ptri->m_points[0]]) ^ (m_FlowComplex->m_0cells[ptri->m_points[2]] -m_FlowComplex->m_0cells[ptri->m_points[0]]);
			int pidx[3]={0,0,0};
			for(int m=0;m<3;++m){
				if(ptri->m_points[m]==p){
					pidx[m]=newidx;
				}else
					pidx[m]=ptri->m_points[m];
			}//m
			CP_Vector3D tmp = (m_FlowComplex->m_0cells[pidx[1]] - m_FlowComplex->m_0cells[pidx[0]]) ^ (m_FlowComplex->m_0cells[pidx[2]] -m_FlowComplex->m_0cells[pidx[0]]);
			if(ntmp*tmp<=0){
				return false;
			}
		}
	}//k

	for(unsigned int k=0;k<m_FlowComplex->m_0cells[q].m_adjTriangle.size();++k){
		CP_Triganle3D* ptri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[q].m_adjTriangle[k]];
		if(ptri->_patch!=-1){
			CP_Vector3D ntmp = (m_FlowComplex->m_0cells[ptri->m_points[1]] - m_FlowComplex->m_0cells[ptri->m_points[0]]) ^ (m_FlowComplex->m_0cells[ptri->m_points[2]] -m_FlowComplex->m_0cells[ptri->m_points[0]]);
			int pidx[3]={0,0,0};
			for(int m=0;m<3;++m){
				if(ptri->m_points[m]==q){
					pidx[m]=newidx;
				}else
					pidx[m]=ptri->m_points[m];
			}//m
			CP_Vector3D tmp = (m_FlowComplex->m_0cells[pidx[1]] - m_FlowComplex->m_0cells[pidx[0]]) ^ (m_FlowComplex->m_0cells[pidx[2]] -m_FlowComplex->m_0cells[pidx[0]]);
			if(ntmp*tmp<=0){
				return false;
			}
		}
	}//k
	return true;
}

void SurfaceOptimization::SplitEdge(CP_Triganle3D &ltri,CP_Triganle3D &rtri,int lidx,int ridx)
{
	int abc=0;
	int sp[2]={-1,-1};//公共边顶点
	int rsri=-1,lsri=-1;;//除公共边外的点在三角形中的序号
	for (unsigned int j = 0; j < 3; j++)
	{
		for (int k = 0; k < 3; k++)
		{//公共边在ltri中j是按序的，如果相应k反序，则方向相逆
			if (ltri.m_points[j]==rtri.m_points[k])
			{   
				sp[abc++]=rtri.m_points[k];
			}
		}
	}

	for (unsigned int j=0;j<3;j++)
	{
		int tmp=-1;
		for(unsigned int k=0;k<2;k++)
		{
			if(ltri.m_points[j]==sp[k])
				tmp=j;
		}
		if(tmp==-1)
			lsri=j;
	}

	for (unsigned int j=0;j<3;j++)
	{
		int tmp=-1;
		for(unsigned int k=0;k<2;k++)
		{
			if(rtri.m_points[j]==sp[k])
				tmp=j;
		}
		if(tmp==-1)
			rsri=j;
	}

	int newidx=m_FlowComplex->m_0cells.size();
	m_FlowComplex->m_0cells.push_back(CP_Point3D((m_FlowComplex->m_0cells[sp[0]].m_x+m_FlowComplex->m_0cells[sp[1]].m_x)/2.0,(m_FlowComplex->m_0cells[sp[0]].m_y+m_FlowComplex->m_0cells[sp[1]].m_y)/2.0,(m_FlowComplex->m_0cells[sp[0]].m_z+m_FlowComplex->m_0cells[sp[1]].m_z)/2.0));
	//left
	for (unsigned int j=0;j<3;j++)
	{
		if((ltri.m_points[j]==sp[0]&&ltri.m_points[(j+1)%3]==sp[1])||(ltri.m_points[(j+1)%3]==sp[0]&&ltri.m_points[j]==sp[1]))
		{
			m_FlowComplex->m_0cells[ltri.m_points[(j+1)%3]].m_adjTriangle.erase(
				remove(m_FlowComplex->m_0cells[ltri.m_points[(j+1)%3]].m_adjTriangle.begin(),
				m_FlowComplex->m_0cells[ltri.m_points[(j+1)%3]].m_adjTriangle.end(),lidx),
				m_FlowComplex->m_0cells[ltri.m_points[(j+1)%3]].m_adjTriangle.end());
			CP_Triganle3D *pTriangle =new CP_Triganle3D(ltri.m_points[(j+1)%3], ltri.m_points[(j+2)%3], newidx);
			m_FlowComplex->tricells.push_back(pTriangle);
			m_FlowComplex->m_patches[ltri._patch]->m_triangle.push_back(m_FlowComplex->tricells.size()-1);
			pTriangle->_patch=ltri._patch;
			m_FlowComplex->m_0cells[ltri.m_points[(j+1)%3]].m_adjTriangle.push_back(m_FlowComplex->tricells.size()-1);
			m_FlowComplex->m_0cells[newidx].m_adjTriangle.push_back(m_FlowComplex->tricells.size()-1);
			m_FlowComplex->m_0cells[ltri.m_points[(j+2)%3]].m_adjTriangle.push_back(m_FlowComplex->tricells.size()-1);

			m_FlowComplex->m_0cells[newidx].m_adjTriangle.push_back(lidx);
			ltri.m_points[(j+1)%3]=newidx;
			break;
		}
	}

	//right
	for (unsigned int j=0;j<3;j++)
	{
		if((rtri.m_points[j]==sp[0]&&rtri.m_points[(j+1)%3]==sp[1])||(rtri.m_points[(j+1)%3]==sp[0]&&rtri.m_points[j]==sp[1]))
		{
			m_FlowComplex->m_0cells[rtri.m_points[(j+1)%3]].m_adjTriangle.erase(
				remove(m_FlowComplex->m_0cells[rtri.m_points[(j+1)%3]].m_adjTriangle.begin(),
				m_FlowComplex->m_0cells[rtri.m_points[(j+1)%3]].m_adjTriangle.end(),ridx),
				m_FlowComplex->m_0cells[rtri.m_points[(j+1)%3]].m_adjTriangle.end());
			CP_Triganle3D *pTriangle =new CP_Triganle3D(rtri.m_points[(j+1)%3], rtri.m_points[(j+2)%3], newidx);
			m_FlowComplex->tricells.push_back(pTriangle);
			m_FlowComplex->m_patches[rtri._patch]->m_triangle.push_back(m_FlowComplex->tricells.size()-1);
			pTriangle->_patch=rtri._patch;
			m_FlowComplex->m_0cells[rtri.m_points[(j+1)%3]].m_adjTriangle.push_back(m_FlowComplex->tricells.size()-1);
			m_FlowComplex->m_0cells[newidx].m_adjTriangle.push_back(m_FlowComplex->tricells.size()-1);
			m_FlowComplex->m_0cells[rtri.m_points[(j+2)%3]].m_adjTriangle.push_back(m_FlowComplex->tricells.size()-1);
			
			m_FlowComplex->m_0cells[newidx].m_adjTriangle.push_back(ridx);
			rtri.m_points[(j+1)%3]=newidx;
			break;
		}
	}
}

double SurfaceOptimization::ComputeMeanEdgeLength(const CP_Patch& pPatch)
{
	double boundary_sum=0.0,interior_sum=0.0;
	int boundary_num=0,interior_num=0;
	for(unsigned int j=0;j<pPatch.m_triangle.size();++j){
		CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch.m_triangle[j]];
		for(int k=0;k<3;++k){
			set<int> incident;
			incident.insert(m_FlowComplex->m_0cells[tri->m_points[k]].incidentpoly.begin(),m_FlowComplex->m_0cells[tri->m_points[k]].incidentpoly.end());
			incident.insert(m_FlowComplex->m_0cells[tri->m_points[(k+1)%3]].incidentpoly.begin(),m_FlowComplex->m_0cells[tri->m_points[(k+1)%3]].incidentpoly.end());
			if(incident.size()!=m_FlowComplex->m_0cells[tri->m_points[k]].incidentpoly.size()+m_FlowComplex->m_0cells[tri->m_points[(k+1)%3]].incidentpoly.size()){
				boundary_sum+=dist(m_FlowComplex->m_0cells[tri->m_points[k]],m_FlowComplex->m_0cells[tri->m_points[(k+1)%3]]);
				boundary_num++;
			}else{
				interior_sum+=dist(m_FlowComplex->m_0cells[tri->m_points[k]],m_FlowComplex->m_0cells[tri->m_points[(k+1)%3]]);
				interior_num++;
			}
		}
	}//j
	//return (interior_sum/2+boundary_sum)/(interior_num+boundary_num);
	return boundary_sum/boundary_num;
}

void SurfaceOptimization::ComputeVertexUpdatePosition(const CP_Patch& pPatch)
{
	map<int,int> mp;
	for(unsigned int j=0;j<pPatch.m_triangle.size();++j){
		CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch.m_triangle[j]];
		for(int k=0;k<3;++k){
			if(m_FlowComplex->m_0cells[tri->m_points[k]].incidentpoly.size()==0&&mp.count(tri->m_points[k])==0){
				//内部点&&未计算过
				set<int> pt;
				double area_sum=0.0;
				for(unsigned int i=0;i<m_FlowComplex->m_0cells[tri->m_points[k]].m_adjTriangle.size();++i){
					CP_Triganle3D *otri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[tri->m_points[k]].m_adjTriangle[i]];
					double a=dist(m_FlowComplex->m_0cells[otri->m_points[0]],m_FlowComplex->m_0cells[otri->m_points[1]]);
					double b=dist(m_FlowComplex->m_0cells[otri->m_points[0]],m_FlowComplex->m_0cells[otri->m_points[2]]);
					double c=dist(m_FlowComplex->m_0cells[otri->m_points[1]],m_FlowComplex->m_0cells[otri->m_points[2]]);
					otri->area=Area(a,b,c);
					area_sum+=otri->area;
					for(int t=0;t<3;++t)
						if(otri->m_points[t]!=tri->m_points[k])
							pt.insert(otri->m_points[t]);
				}
			
				//计算中心
				CP_Point3D centroid(0,0,0);
				for (auto e:pt)
				{
					centroid.m_x+=m_FlowComplex->m_0cells[e].m_x;
					centroid.m_y+=m_FlowComplex->m_0cells[e].m_y;
					centroid.m_z+=m_FlowComplex->m_0cells[e].m_z;
				}
				centroid.m_x/=pt.size();
				centroid.m_y/=pt.size();
				centroid.m_z/=pt.size();

				//计算方向v=c-p
				//v=v-(N*v)N
				CP_Vector3D v=centroid-m_FlowComplex->m_0cells[tri->m_points[k]];
				/*CP_Vector3D N;
				for(unsigned int i=0;i<m_FlowComplex->m_0cells[tri->m_points[k]].m_adjTriangle.size();++i){
					CP_Triganle3D *otri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[tri->m_points[k]].m_adjTriangle[i]];
					if(otri->_patch!=-1){
						CP_Vector3D ntmp = (m_FlowComplex->m_0cells[otri->m_points[1]] - m_FlowComplex->m_0cells[otri->m_points[0]]) ^ (m_FlowComplex->m_0cells[otri->m_points[2]] -m_FlowComplex->m_0cells[otri->m_points[0]]);
						ntmp.Normalize();
						N+=(otri->area/area_sum)*ntmp;
					}
				}
				N.Normalize();
				v=v-(N*v)*N;*/
				double weight=0.9;
				m_FlowComplex->m_0cells[tri->m_points[k]].m_newx=m_FlowComplex->m_0cells[tri->m_points[k]].m_x+weight*v.m_x;
				m_FlowComplex->m_0cells[tri->m_points[k]].m_newy=m_FlowComplex->m_0cells[tri->m_points[k]].m_y+weight*v.m_y;
				m_FlowComplex->m_0cells[tri->m_points[k]].m_newz=m_FlowComplex->m_0cells[tri->m_points[k]].m_z+weight*v.m_z;
				mp[tri->m_points[k]]++;
			}
		}
	}//j
}

void SurfaceOptimization::FlipAndCollapseAfterRefine()
{
	//2.定位needles
	for(unsigned int n=0;n<m_FlowComplex->m_patches.size();++n){
		CP_Patch *pPatch = m_FlowComplex->m_patches[n];
		if(pPatch->flag){
			for(unsigned int j=0;j<pPatch->m_triangle.size();++j){
				CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch->m_triangle[j]];
				if(tri->_patch!=-1){
					for(int i=0;i<3;i++)
					{
						CP_Vector3D i1(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_z);
						CP_Vector3D i2(m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_z);
						i1.Normalize();
						i2.Normalize();
						double times=dist(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]],m_FlowComplex->m_0cells[tri->m_points[i%3]])/dist(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]],m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]]);
						if(i1*i2>=cos(30*PI/180)){
							if((m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].incidentpoly.size()==0&&m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].incidentpoly.size()==0)){
								int p=tri->m_points[(i+1)%3];
								int q=tri->m_points[(i+2)%3];
								CollapseEdge(p,q);
								break;
							}
						}//30
					}//i
				}//_patch!=-1
			}//j
			//从面片中清除已无效的三角形
			for(vector<int>::iterator it=pPatch->m_triangle.begin();it!=pPatch->m_triangle.end();){
				if(m_FlowComplex->tricells[*it]->_patch==-1){
					pPatch->m_triangle.erase(it);
				}else
					++it;
			}
		}
	}//n

	for(unsigned int n=0;n<m_FlowComplex->m_patches.size();++n){
		CP_Patch *pPatch = m_FlowComplex->m_patches[n];
		if(pPatch->flag){
			for(unsigned int j=0;j<pPatch->m_triangle.size();++j){
				CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch->m_triangle[j]];
				for(int i=0;i<3;i++)
				{
					CP_Vector3D i1(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_z);
					CP_Vector3D i2(m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_x-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_y-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_z-m_FlowComplex->m_0cells[tri->m_points[i%3]].m_z);
					i1.Normalize();
					i2.Normalize();
					if(i1*i2<=cos(150*PI/180)){
						//本来caps的夹角要接近180的才算退化，但IFC中钝角实在太多，且角度越接近60度越好，所以选择120
						vector<int> v=m_FlowComplex->GetIncidentTri(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]],m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]]);
						//长边不是曲线边，flip swapedge
						for(unsigned int k=0;k<v.size();k++)
						{
							if(pPatch->m_triangle[j]!=v[k]){
								CP_Triganle3D *opTri = m_FlowComplex->tricells[v[k]];
								if(opTri->_patch==tri->_patch){
									FlipEdge(*tri,*opTri,pPatch->m_triangle[j],v[k]);
								}
							}
						}
						break;
					}//caps i1*i2
				}//i
			}//j
		}//flag
	}//n
}

double SurfaceOptimization::ComputeCycleCost(const vector<int> &path,vector<CP_Vector3D>& normal,const vector<int>& coner)
{
	double minCost=MAX_DISTANCE;
	int nTwistNormalNum=100;
	//compute reflection vector of R1，并调整顺序
	vector<CP_Vector3D> reflectVectorMatrices1;//V1=X(i+1)-Xi

	for(unsigned int i=0;i<path.size()-1;i++){
		CP_Vector3D t=m_FlowComplex->m_0cells[path[i+1]]-m_FlowComplex->m_0cells[path[i]];
		reflectVectorMatrices1.push_back(t);
	}//i

	//最开头不算拐点
	vector<CP_Vector3D> edgeVecs;//每条边的切向
	edgeVecs.push_back(reflectVectorMatrices1[0]);
	for(unsigned int i=1;i<reflectVectorMatrices1.size();++i)
	{
		edgeVecs.push_back(reflectVectorMatrices1[i-1]+reflectVectorMatrices1[i]);
	}
	//edgeVecs.insert(edgeVecs.begin(),reflectVectorMatrices1.begin(),reflectVectorMatrices1.end());
	edgeVecs.push_back(reflectVectorMatrices1.back());//n个切向
	
	CP_Vector3D randomVector;
	randomVector.m_x=1;
	randomVector.m_y=exp(1.);
	randomVector.m_z=PI;

	CP_Vector3D initNormal = reflectVectorMatrices1[0]^randomVector;
	initNormal.Normalize();

	double anglemin=INT_MAX;
	vector<CP_Vector3D> res;
	for(int i=0;i<nTwistNormalNum;i++){
		vector<CP_Vector3D> r;
		double angle = (i+1)*2*PI/double(nTwistNormalNum);
		CP_Vector3D r0 = m_FlowComplex->rotateNormal(initNormal,reflectVectorMatrices1[0],angle);//nm起始法向
		r0.Normalize();
		r.push_back(r0);
		double costAng=0,costTwt=0;
		for (unsigned int j=0;j<reflectVectorMatrices1.size();j++)//n-1个，每次计算j+1点的法向
		{
			double c1=reflectVectorMatrices1[j]*reflectVectorMatrices1[j];
			CP_Vector3D riL=r[j]-(2/c1)*(reflectVectorMatrices1[j]*r[j])*reflectVectorMatrices1[j];
			CP_Vector3D tiL=edgeVecs[j]-(2/c1)*(reflectVectorMatrices1[j]*edgeVecs[j])*reflectVectorMatrices1[j];
			//compute reflection vector of R2
			CP_Vector3D v2=edgeVecs[j+1]-tiL;
			double c2=v2*v2;
			CP_Vector3D rnext=riL-(2/c2)*(v2*riL)*v2;
			rnext.Normalize();
			r.push_back(rnext);
		}//j reflectVectorMatrices1
		//加上最开始的拐角
		vector<CP_Vector3D>::iterator n1,n2;
		for(unsigned int j=0;j<coner.size()-1;++j){
			n1=r.begin()+coner[j]-1;
			n2=r.begin()+coner[j]+1;
			costAng+=acos((*n1)*(*n2));
		}
		
		n1=r.begin()+coner.back();
		n2=r.begin()+1;
		costAng+=acos((*n1)*(*n2));

		double tmp=costAng;
		if(tmp<anglemin)
		{
			anglemin=tmp;
			res.swap(r);
		}
	}//i nTwistNormalNum
	normal.swap(res);
	return anglemin;
}

void SurfaceOptimization::GetInteriorPointsAndRing(CP_Patch* pPatch)
{
	map<int,int> mp;
	for(unsigned int j=0;j<pPatch->m_triangle.size();++j){
		CP_Triganle3D *tri=m_FlowComplex->tricells[pPatch->m_triangle[j]];
		for(int k=0;k<3;++k){
			if(m_FlowComplex->m_0cells[tri->m_points[k]].incidentpoly.size()==0&&mp.count(tri->m_points[k])==0){
				//内部点&&未计算过
				set<int> pt;
				for(unsigned int i=0;i<m_FlowComplex->m_0cells[tri->m_points[k]].m_adjTriangle.size();++i){
					CP_Triganle3D *otri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[tri->m_points[k]].m_adjTriangle[i]];
					if(otri->_patch!=-1){
						for(int t=0;t<3;++t)
							if(otri->m_points[t]!=tri->m_points[k])
								pt.insert(otri->m_points[t]);
					}
				}
				//ring
				copy(pt.begin(), pt.end(), back_inserter(m_FlowComplex->m_0cells[tri->m_points[k]].ring));
				//ring-weight
				for(unsigned int i=0;i<m_FlowComplex->m_0cells[tri->m_points[k]].ring.size();++i){
					int p=tri->m_points[k];
					int q=m_FlowComplex->m_0cells[tri->m_points[k]].ring[i];
					vector<int> v=m_FlowComplex->GetIncidentTri(m_FlowComplex->m_0cells[p],m_FlowComplex->m_0cells[q]);
					if(v.size()>=2){
						double wpq=0.0;
						for(int t=0;t<2;++t){
							CP_Triganle3D *otri=m_FlowComplex->tricells[v[t]];
							for(int m=0;m<3;++m){
								if(otri->m_points[m]!=p&&otri->m_points[m]!=q){
									CP_Vector3D pi=m_FlowComplex->m_0cells[p]-m_FlowComplex->m_0cells[otri->m_points[m]];
									CP_Vector3D qi=m_FlowComplex->m_0cells[q]-m_FlowComplex->m_0cells[otri->m_points[m]];
									pi.Normalize();
									qi.Normalize();
									double cos_alpha=pi*qi;
									double wm=cos_alpha/sqrt(1-cos_alpha*cos_alpha);
										//m_FlowComplex->m_0cells[tri->m_points[k]].flag=true;
									if(!_isnan(wm)&&_finite(wm)){
										wpq+=wm;
									}//else
									//	m_FlowComplex->m_0cells[tri->m_points[k]].flag=true;
									//else
									//	cout<<"sandian:"<<p<<","<<q<<","<<otri->m_points[m]<<":"<<wpq<<endl;
									break;
								}
							}//m
						}
						m_FlowComplex->m_0cells[tri->m_points[k]].weight.push_back(wpq);
					}
					//cout<<m_FlowComplex->m_0cells[tri->m_points[k]].weight.back()<<",";
				}
				pPatch->interior_points.push_back(tri->m_points[k]);
				mp[tri->m_points[k]]++;
			}
		}
	}//j
}

void SurfaceOptimization::InterpolateNormalValue(map<int,double>& mp_normal_value,CP_Patch* pPatch,vector<double>& res)
{
	map<int,int> mp_idx;
	for(unsigned int i=0;i<pPatch->interior_points.size();++i){
		mp_idx[pPatch->interior_points[i]]=i;
	}

	VectorXd b;
	vector<double> bvec;
	vector<vector<double> > coefvec;
	int col=pPatch->interior_points.size();
	MatrixXd m(col,col);
	for(unsigned int i=0;i<pPatch->interior_points.size();++i){
		int vi=pPatch->interior_points[i];//第i个内部点
		double bi=0.0;
		vector<double> coefficent(col,0.0);
		for(unsigned int j=0;j<m_FlowComplex->m_0cells[vi].ring.size();++j){
			int vj=m_FlowComplex->m_0cells[vi].ring[j];
			if(m_FlowComplex->m_0cells[vj].incidentpoly.size()>0){
				//非内部点
				coefficent[mp_idx[vi]]+=m_FlowComplex->m_0cells[vi].weight[j];
				bi+=m_FlowComplex->m_0cells[vi].weight[j]*mp_normal_value[vj];
			}else{
				//内部点
				coefficent[mp_idx[vi]]+=m_FlowComplex->m_0cells[vi].weight[j];
				coefficent[mp_idx[vj]]=m_FlowComplex->m_0cells[vi].weight[j];
			}
		}
		coefvec.push_back(coefficent);
		bvec.push_back(bi);
	}
	
	for(unsigned int i=0;i<pPatch->interior_points.size();++i)
		m.row(i) = VectorXd::Map(&coefvec[i][0],coefvec[i].size());
	
	b=VectorXd::Map(&bvec[0],bvec.size());
	MatrixXd x = m.fullPivHouseholderQr().solve(b);
	vector<double> vec(x.data(), x.data() + x.rows() * x.cols());
	res.swap(vec);
}

void SurfaceOptimization::LaplaceInterpolateNormals()
{
	for(unsigned int n=0;n<m_FlowComplex->m_patches.size();++n){
		CP_Patch *pPatch = m_FlowComplex->m_patches[n];
		if(pPatch->flag){
			ComputeRMFForCycle(pPatch);
			GetInteriorPointsAndRing(pPatch);
			//所有内部点的法向一次计算出来
			pPatch->interior_points_normal.resize(pPatch->interior_points.size());
			map<int,double> mp_normal_value_x;
			for(unsigned int i=0;i<pPatch->path.size();++i){
				mp_normal_value_x[pPatch->path[i]]=pPatch->r[i].m_x;
			}
			vector<double> resx;
			InterpolateNormalValue(mp_normal_value_x,pPatch,resx);
			for(unsigned int i=0;i<resx.size();++i){
				pPatch->interior_points_normal[i].m_x=resx[i];
			}

			map<int,double> mp_normal_value_y;
			for(unsigned int i=0;i<pPatch->path.size();++i){
				mp_normal_value_y[pPatch->path[i]]=pPatch->r[i].m_y;
			}
			vector<double> resy;
			InterpolateNormalValue(mp_normal_value_y,pPatch,resy);
			for(unsigned int i=0;i<resy.size();++i){
				pPatch->interior_points_normal[i].m_y=resy[i];
			}

			map<int,double> mp_normal_value_z;
			for(unsigned int i=0;i<pPatch->path.size();++i){
				mp_normal_value_z[pPatch->path[i]]=pPatch->r[i].m_z;
			}
			vector<double> resz;
			InterpolateNormalValue(mp_normal_value_z,pPatch,resz);
			for(unsigned int i=0;i<resz.size();++i){
				pPatch->interior_points_normal[i].m_z=resz[i];
			}
			for (unsigned int i=0;i<pPatch->interior_points_normal.size();++i)
			{
				pPatch->interior_points_normal[i].Normalize();
			}
		}
	}
}

void SurfaceOptimization::NormalAjustment()
{
	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();++i){
		CP_Patch *pPatch = m_FlowComplex->m_patches[i];
		if(pPatch->flag){
			//cout<<"patch"<<i<<endl;
			//反向
			for(unsigned int j=0;j<pPatch->interior_points.size();++j){
				int vj=pPatch->interior_points[j];
				for(unsigned int k=0;k<m_FlowComplex->m_0cells[vj].m_adjTriangle.size();++k){
				//if(m_FlowComplex->m_0cells[vj].m_adjTriangle.size()>0){
					CP_Triganle3D* tri=m_FlowComplex->tricells[m_FlowComplex->m_0cells[vj].m_adjTriangle[k]];
					if(tri->_patch==pPatch->index){
						CP_Vector3D ntmp = (m_FlowComplex->m_0cells[tri->m_points[1]] - m_FlowComplex->m_0cells[tri->m_points[0]]) ^ (m_FlowComplex->m_0cells[tri->m_points[2]] -m_FlowComplex->m_0cells[tri->m_points[0]]);
						if(ntmp*pPatch->interior_points_normal[j]<0)
						{
							pPatch->interior_points_normal[j]*=(-1);
							break;
						}
					}
				}
			}//j

			map<int,CP_Vector3D> mp_normal;//x-0cells下标，y-normal
			for(unsigned int j=0;j<pPatch->interior_points.size();++j)
				mp_normal[pPatch->interior_points[j]]=pPatch->interior_points_normal[j];
			for(unsigned int j=0;j<pPatch->path.size();++j)
				mp_normal[pPatch->path[j]]=pPatch->r[j];
			//与1-ring的平均方向偏离
			for(unsigned int j=0;j<pPatch->interior_points.size();++j){
				vector<double> angle;
				int vj=pPatch->interior_points[j];
				for(unsigned int k=0;k<m_FlowComplex->m_0cells[vj].ring.size();++k){
					angle.push_back(acos(mp_normal[vj]*mp_normal[m_FlowComplex->m_0cells[vj].ring[k]]));
				}
				//越小角越好，由小到大排序
				sort(angle.begin(),angle.end());
				int median_idx=angle.size()/2;
				if(median_idx+1<=(int)angle.size()&&angle[median_idx]>PI/6)//如果中位数都有问题，这个本身有问题
				{//暂时替换map里的法向，去掉噪声取平均
					vector<int> avg_idx;
					for(unsigned int k=0;k<angle.size();++k){
						if(fabs(angle[median_idx]-angle[k])<PI/3){
							avg_idx.push_back(m_FlowComplex->m_0cells[vj].ring[k]);
						}
					}
					CP_Vector3D avg;
					for(auto e:avg_idx)
						avg+=mp_normal[e];
					avg/=avg_idx.size();
					mp_normal[pPatch->interior_points[j]]=avg;
					//m_FlowComplex->m_0cells[vj].flag=true;
				}//需要改正
			}
			for(unsigned int j=0;j<pPatch->interior_points.size();++j)
				pPatch->interior_points_normal[j]=mp_normal[pPatch->interior_points[j]];
			//cout<<"endpatch"<<i<<endl;
		}//flag 
	}//i
}
