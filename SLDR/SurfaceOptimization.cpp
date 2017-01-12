#include "stdafx.h"
#include "SurfaceOptimization.h"
#include <iostream>

SurfaceOptimization::~SurfaceOptimization(void)
{

}

void SurfaceOptimization::ElimateDegenerateTriangles()
{
	SetTrianglePatch();
	MergeDegenerateTrianglesIn2cell();
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
	//2.找到相邻三角形
	//3.swap edge并合并2cell

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
				}
			}
		}
	}
}
