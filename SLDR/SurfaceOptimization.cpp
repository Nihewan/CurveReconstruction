#include "stdafx.h"
#include "SurfaceOptimization.h"
#include <iostream>

SurfaceOptimization::~SurfaceOptimization(void)
{

}

void SurfaceOptimization::ElimateDegenerateTriangles()
{
	SetTrianglePatch();
	ResetPointsAndAdjacent();
	ElimateDegenerateTrianglesInPatch();
	
	//patch法向
}

void SurfaceOptimization::ElimateDegenerateTrianglesInPatch()
{
	//between 2cells in the same patch
	//1.定位caps
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
					if(i1*i2<=cos(120*PI/180)){
						//本来caps的夹角要接近180的才算退化，但IFC中钝角实在太多，且角度越接近60度越好，所以选择120
						vector<int> v=m_FlowComplex->GetIncidentTri(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]],m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]]);
						//如果长边是曲线边，则插入中点
						bool is_curve_segment=false;
						for(unsigned int k=0;k<m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].incidentpoly.size();++k){
							for(unsigned int kk=0;kk<m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].incidentpoly.size();++kk){
								if(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].incidentpoly[k]==
									m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].incidentpoly[kk])
									is_curve_segment=true;
							}
						}
						if(is_curve_segment){
							CP_Point3D p((m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_x+m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_x)/2,
								(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_y+m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_y)/2,
								(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_z+m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_z)/2);
							m_FlowComplex->m_0cells.push_back(p);
							for(unsigned int vi=0;vi<v.size();++vi){
								CP_Triganle3D *opTri = m_FlowComplex->tricells[v[vi]];
								if(opTri->_patch!=-1){
									for(int k=0;k<3;k++){
										if((opTri->m_points[k]==tri->m_points[(i+1)%3]&&opTri->m_points[(k+1)%3]==tri->m_points[(i+2)%3])||(opTri->m_points[k]==tri->m_points[(i+2)%3]&&opTri->m_points[(k+1)%3]==tri->m_points[(i+1)%3]))
											;
										else{
											CP_Triganle3D *pLTriangle =new CP_Triganle3D(opTri->m_points[k], opTri->m_points[(k+1)%3], m_FlowComplex->m_0cells.size()-1);
											m_FlowComplex->tricells.push_back(pLTriangle);
											if(pPatch->index==opTri->_patch){
												pLTriangle->_patch=n;
												pPatch->m_triangle.push_back(m_FlowComplex->tricells.size()-1);
											}else{m_FlowComplex->obt.push_back(m_FlowComplex->tricells.size()-1);
												pLTriangle->_patch=opTri->_patch;
												m_FlowComplex->m_patches[opTri->_patch]->m_triangle.push_back(m_FlowComplex->tricells.size()-1);
											}
										}//三角形的这条边
									}//k
									opTri->_patch=-1;
								}//_patch!=-1
							}//vi
							//tri->_patch=-1;
						}else{
							//长边不是曲线边，flip swapedge
							for(unsigned int k=0;k<v.size();k++)
							{
								if(pPatch->m_triangle[j]!=v[k]){
									CP_Triganle3D *opTri = m_FlowComplex->tricells[v[k]];
									//要求所对角也是钝角
									int pidx=0;
									for(int ii=0;ii<3;++ii){
										if((opTri->m_points[ii]!=tri->m_points[(i+1)%3])&&(opTri->m_points[ii]!=tri->m_points[(i+2)%3])){
											pidx=opTri->m_points[ii];break;
										}
									}
									CP_Vector3D ii1(m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_x-m_FlowComplex->m_0cells[pidx].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_y-m_FlowComplex->m_0cells[pidx].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+1)%3]].m_z-m_FlowComplex->m_0cells[pidx].m_z);
									CP_Vector3D ii2(m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_x-m_FlowComplex->m_0cells[pidx].m_x,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_y-m_FlowComplex->m_0cells[pidx].m_y,m_FlowComplex->m_0cells[tri->m_points[(i+2)%3]].m_z-m_FlowComplex->m_0cells[pidx].m_z);
									ii1.Normalize();
									ii2.Normalize();
									if(opTri->_patch==tri->_patch&&ii1*ii2<0.866){
										SwapEdge(*tri,*opTri,pPatch->m_triangle[j],v[k]);
									}
								}
							}
						}//else
						break;
					}//caps i1*i2
				}//i
			}//j
			//从面片中清除已无效的三角形
			for(vector<int>::iterator it=pPatch->m_triangle.begin();it!=pPatch->m_triangle.end();){
				if(m_FlowComplex->tricells[*it]->_patch==-1){
					pPatch->m_triangle.erase(it);
				}else
					++it;
			}
		}//flag
	}//n
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

void SurfaceOptimization::SwapEdge(CP_Triganle3D &ltri,CP_Triganle3D &rtri,int lidx,int ridx)
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
