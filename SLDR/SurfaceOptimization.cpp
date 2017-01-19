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
	//MergeDegenerateTrianglesIn2cell();
	ElimateDegenerateTrianglesInPatch();
	
	//patch法向
}

void SurfaceOptimization::MergeDegenerateTrianglesIn2cell()
{
	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();++i){
		CP_Patch *pPatch = m_FlowComplex->m_patches[i];
		if(pPatch->flag){
			for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
			{
				CP_2cell *p2cell = m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
				if(p2cell->m_triangle.size()==3&&i<(unsigned int)m_FlowComplex->oripatches)
				{
					vector<int> ver;
					for (unsigned int k=0;k<p2cell->m_triangle.size();++k)
					{
						CP_Triganle3D *ptri=m_FlowComplex->tricells[p2cell->m_triangle[k]];
						for(int m=0;m<3;++m){
							if(ver.size()<3&&ptri->m_points[m]<(int)m_FlowComplex->inputPoints
								&&find(ver.begin(),ver.end(),ptri->m_points[m])==ver.end())
							{
								ver.push_back(ptri->m_points[m]);
							}
						}
						ptri->_patch=-1;
					}//k
					CP_Triganle3D *pTriangle =new CP_Triganle3D(ver[0], ver[1], ver[2]);
					pTriangle->_patch=i;
					vector<int>().swap(p2cell->m_triangle);
					p2cell->m_triangle.push_back(m_FlowComplex->tricells.size());
					m_FlowComplex->tricells.push_back(pTriangle);
				}
			}//j
		}//flag
	}//i
}

void SurfaceOptimization::ElimateDegenerateTrianglesInPatch()
{
	//between 2cells in the same patch
	//1.定位caps
	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();++i){
		CP_Patch *pPatch = m_FlowComplex->m_patches[i];
		if(pPatch->flag){
			for(unsigned int j=0;j<pPatch->m_triangle.size();++j){
				if(obtusetri(*m_FlowComplex->tricells[pPatch->m_triangle[j]]))
					m_FlowComplex->obt.push_back(pPatch->m_triangle[j]);
			}//j
		}
	}//i
	//2.swap edge
	for(unsigned int i=0;i<m_FlowComplex->obt.size();++i){
		if(m_FlowComplex->tricells[m_FlowComplex->obt[i]]->_patch!=-1){
			CP_Triganle3D *pTri = m_FlowComplex->tricells[m_FlowComplex->obt[i]];
			for(unsigned int j=0;j<pTri->m_adjTriangle.size();++j){
				CP_Triganle3D *opTri = m_FlowComplex->tricells[pTri->m_adjTriangle[j]];
				if(opTri->_patch==pTri->_patch){
					SwapEdge(*pTri,*opTri);
					pTri->_patch=-1;
					opTri->_patch=-1;
				}
			}//j
		}
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

void SurfaceOptimization::SwapEdge(CP_Triganle3D &ltri,CP_Triganle3D &rtri)
{
	int abc=0;
	int sp[2]={-1,-1};//公共边顶点
	int rsri=-1,lsri=-1;;//除公共边外的点
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
			ltri.m_points[(j+1)%3]=rtri.m_points[rsri];
			break;
		}
	}

	//right swap
	for (unsigned int j=0;j<3;j++)
	{
		if((rtri.m_points[j]==sp[0]&&rtri.m_points[(j+1)%3]==sp[1])||(rtri.m_points[(j+1)%3]==sp[0]&&rtri.m_points[j]==sp[1]))
		{
			rtri.m_points[(j+1)%3]=ltri.m_points[lsri];
			break;
		}
	}
}
