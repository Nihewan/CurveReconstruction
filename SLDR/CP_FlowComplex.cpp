#include "stdafx.h"
#include "CP_FlowComplex.h"
#include <queue>
#include <stack>
#include <iostream>
#include <functional> 
#include <algorithm>
using namespace std;

CP_Triganle3D::CP_Triganle3D()
{
	_patch=-1;
}

CP_Triganle3D::CP_Triganle3D(int p0, int p1, int p2)
{
	m_points[0] = p0;
	m_points[1] = p1;
	m_points[2] = p2;

	normalsetted=false;
	//	_2cell=-1;
	_patch=-1;
}

CP_Triganle3D::~CP_Triganle3D(void)
{
}

CircuAndTri::CircuAndTri()
{

}

CircuAndTri::~CircuAndTri()
{

}

CP_FlowComplex::CP_FlowComplex()
{
	desN=0;
	mergededge=0;
	inputPoints=0;
	ori2cells=0;
	inputCurveSegments=0;
	inputCurves=0;
	respatches=0;
	res_triangle_num=0;
	perweight=1;
	biweight=0;
}


CP_FlowComplex::~CP_FlowComplex()
{
	for(unsigned int i=0;i<m_1cells.size();i++)
		delete m_1cells[i];
	for( unsigned int i=0;i<m_2cells.size();i++)
		delete m_2cells[i];
	for(unsigned int i=0;i<delauny2cells.size();i++)
		delete delauny2cells[i];
	for(unsigned int i=0;i<tricells.size();i++)
		delete tricells[i];
	for(unsigned int i=0;i<m_3cells.size();i++)
		delete m_3cells[i];
	vector<CP_3cell*>().swap(m_3cells);
}

bool CP_FlowComplex::IsSmallAngle(const CP_Point3D &po,const CP_Point3D &pa,const CP_Point3D &pb)
{
	CP_Vector3D oa=pa-po;
	CP_Vector3D ob=pb-po;
	if(oa*ob/(oa.GetLength()*ob.GetLength())>PI/6.0)
		return true;
	else
		return false;
}

void CP_FlowComplex::IsBoundBox(const CP_Point3D &p)
{
	if(p.m_x<minx)
		minx=p.m_x;
	if(p.m_x>maxx)
		maxx=p.m_x;

	if(p.m_y<miny)
		miny=p.m_y;
	if(p.m_y>maxy)
		maxy=p.m_y;

	if(p.m_z<minz)
		minz=p.m_z;
	if(p.m_z>maxz)
		maxz=p.m_z;
}

void CP_FlowComplex::Gabrielize()
{
	//同一条曲线小角度问题
	for(unsigned int i=0;i<m_PolyLine.size();i++)
	{
		if(m_PolyLine[i].m_points.size()>2)
		{
			for(unsigned int j=1;j<m_PolyLine[i].m_points.size()-1;j++)
			{
				if(IsSmallAngle(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j-1],m_PolyLine[i].m_points[j+1]))
				{
					double k=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j-1])/2;
					double d=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1])/2;
					if(k<d)
					{   
						m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j,CP_Point3D((m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j-1].m_x)/2.0,(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j-1].m_y)/2.0,(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j-1].m_z)/2.0));
						m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j+2,EqualDisPoint(k,m_PolyLine[i].m_points[j+1],m_PolyLine[i].m_points[j+2]));
						j=j+2;
					}else if(k>d)
					{
						m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j+1,CP_Point3D((m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2.0,(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2.0,(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2.0));
						m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j,EqualDisPoint(d,m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j-1]));
						j=j+2;
					}
				}//small angle
			}//j
		}//>2
	}//i
	
	//两线间小角度冲突问题
	//在交汇点冲突时应该以此为joint点等距细分，这样后边不会再需要投影了
	vector<CP_Point3D> vjointtmp;
	for(unsigned int i=0;i<m_PolyLine.size();i++)
	{
		for(unsigned int j=0;j<m_PolyLine[i].m_points.size()-1;j++)
		{//线段j，j+1
			if(j==0||j==(m_PolyLine[i].m_points.size()-2)){
				double m_x=(m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2;
				double m_y=(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2;
				double m_z=(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2;
				CP_Point3D midPoint(m_x,m_y,m_z);//圆心
				double radius_pq=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1])/2;//半径

				for(unsigned int m=0;m<m_PolyLine.size();m++)
				{
					if(i==m)
						;
					else 
					{
						//只检查交汇点处
						if(!IsGabriel(midPoint,m_PolyLine[m].m_points[1],radius_pq))
						{
							if(!ExistPoint(vjointtmp,m_PolyLine[m].m_points[0]))
								vjointtmp.push_back(m_PolyLine[m].m_points[0]);
						}//while Gabriel

						if(!IsGabriel(midPoint,m_PolyLine[m].m_points[m_PolyLine[m].m_points.size()-2],radius_pq))
						{
							if(!ExistPoint(vjointtmp,m_PolyLine[m].m_points[m_PolyLine[m].m_points.size()-1]))
								vjointtmp.push_back(m_PolyLine[m].m_points[m_PolyLine[m].m_points.size()-1]);
						}//while Gabriel
					}//i==m else
				}//for m
			}//j==0||m_point.size-2
		}//j
	}//i
	
	for(unsigned int i=0;i<vjointtmp.size();i++)
		SubdivideSegsJointV(vjointtmp[i]);

	//gabriel and 1-cells 可在三角形中表示
	for(unsigned int i=0;i<m_PolyLine.size();i++)
	{
		for(unsigned int j=0;j<m_PolyLine[i].m_points.size()-1;j++)
		{//线段j，j+1
			double m_x=(m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2;
			double m_y=(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2;
			double m_z=(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2;
			CP_Point3D midPoint(m_x,m_y,m_z);//圆心
			double radius_pq=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1])/2;//半径
		
			//同一条曲线非Gabriel需投影
			//对j，j+1线段检查其他点
			for(unsigned int n=0;n<m_PolyLine[i].m_points.size();n++)
			{
				if(n!=j&&n!=(j+1))
				{
					if(!IsGabriel(midPoint,m_PolyLine[i].m_points[n],radius_pq))
					{
						m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j+1,ProjectionPoint(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1],m_PolyLine[i].m_points[n]));
						ReverseForProjection(m_PolyLine[i].m_points[j+1]);
						j++;
						midPoint.m_x=(m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2;
						midPoint.m_y=(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2;
						midPoint.m_z=(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2;
						radius_pq=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1])/2;
					}//while Gabriel
				}
			}//n

			//曲线间非Gabriel需投影
			for(unsigned int m=0;m<m_PolyLine.size();m++)
			{
				if(i==m)
					;
				else 
				{
					for(unsigned int n=0;n<m_PolyLine[m].m_points.size();n++)
					{
						//别的点不在线段内
						if(!IsGabriel(midPoint,m_PolyLine[m].m_points[n],radius_pq))
						{
							m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j+1,ProjectionPoint(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1],m_PolyLine[m].m_points[n]));
							ReverseForProjection(m_PolyLine[i].m_points[j+1]);
							midPoint.m_x=(m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2;
							midPoint.m_y=(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2;
							midPoint.m_z=(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2;
							radius_pq=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1])/2;
						}//while Gabriel
					}//n
				}//i==m else
			}//for m
		}//j
	}//i
}

void CP_FlowComplex::ReverseForProjection(const CP_Point3D & p)
{
	for(unsigned int i=0;i<m_PolyLine.size();i++)
	{
		for(unsigned int j=1;j<m_PolyLine[i].m_points.size()-2;j++)
		{
			double m_x=(m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2;
			double m_y=(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2;
			double m_z=(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2;
			CP_Point3D midPoint(m_x,m_y,m_z);//圆心
			double radius_pq=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1])/2;//半径

			if(!IsGabriel(midPoint,p,radius_pq))
			{
				m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j+1,ProjectionPoint(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1],p));
				ReverseForProjection(m_PolyLine[i].m_points[j+1]);
			}
		}//j
	}//i
}

bool CP_FlowComplex::IsGabriel(const CP_Point3D &c,const CP_Point3D &r,double radius_pq)
{//r是否在以c为圆心以radius为半径的球中,在返回false，不在true
	double dis=dist(c,r)-radius_pq;
	if(fabs(dis)>5e-6&&dis<0)
		return false;
	else
		return true;
}

CP_Point3D CP_FlowComplex::ProjectionPoint(const CP_Point3D &p1,const CP_Point3D &p2,const CP_Point3D &p3)
{
	double k=((p3.m_x-p1.m_x)*(p2.m_x-p1.m_x)+(p3.m_y-p1.m_y)*(p2.m_y-p1.m_y)+(p3.m_z-p1.m_z)*(p2.m_z-p1.m_z))/((p2.m_x-p1.m_x)*(p2.m_x-p1.m_x)+(p2.m_y-p1.m_y)*(p2.m_y-p1.m_y)+(p2.m_z-p1.m_z)*(p2.m_z-p1.m_z));
	return CP_Point3D(k*(p2.m_x-p1.m_x)+p1.m_x,k*(p2.m_y-p1.m_y)+p1.m_y,k*(p2.m_z-p1.m_z)+p1.m_z);
}

void CP_FlowComplex::SubdivideSegsJointV(const CP_Point3D & vp)
{
	//mep（m end point） 交汇点，polym  所在折线编号
	//只需插入相同距离的点即可
	double dmin=MAX_DISTANCE;
	map<int,int> polymap;//所在polyline，value 0开头，！=0结尾；同一曲线的头或尾只有一端出现在一个交汇点处，所以key不会重复
	for(unsigned int i=0;i<m_PolyLine.size();i++)
	{
		int pnum=m_PolyLine[i].m_points.size();
		if(m_PolyLine[i].m_points[0]==vp)
		{
			polymap.insert(pair<int,int>(i,0));
			double k=dist(m_PolyLine[i].m_points[0],m_PolyLine[i].m_points[1])/2;
			if(k<dmin)
				dmin=k;
		}
		if(m_PolyLine[i].m_points[pnum-1]==vp)
		{
			polymap.insert(pair<int,int>(i,pnum-1));
			double k=dist(m_PolyLine[i].m_points[pnum-1],m_PolyLine[i].m_points[pnum-2])/2;
			if(k<dmin)
				dmin=k;	
		}
	}//i

	map<int,int>::iterator it;
	for(it=polymap.begin();it!=polymap.end();++it)
	{
		if(it->second==0)
			m_PolyLine[it->first].m_points.insert(m_PolyLine[it->first].m_points.begin()+1,EqualDisPoint(dmin,m_PolyLine[it->first].m_points[0],m_PolyLine[it->first].m_points[1]));
		else
			m_PolyLine[it->first].m_points.insert(m_PolyLine[it->first].m_points.begin()+it->second,EqualDisPoint(dmin,m_PolyLine[it->first].m_points[it->second],m_PolyLine[it->first].m_points[it->second-1]));
	}
}

//O---->A----->B
CP_Point3D CP_FlowComplex::EqualDisPoint(double k,const CP_Point3D &po,const CP_Point3D &pb)
{
	double d=dist(po,pb);
	double x=k*(pb.m_x-po.m_x)/d+po.m_x;
	double y=k*(pb.m_y-po.m_y)/d+po.m_y;
	double z=k*(pb.m_z-po.m_z)/d+po.m_z;
	return CP_Point3D(x,y,z);
}

bool CP_FlowComplex::ExistPoint(const vector<CP_Point3D> &v,const CP_Point3D& p)
{
	for (unsigned int i = 0; i < v.size(); i++)
	{
		if (fabs(v[i].m_x - p.m_x) < TOL&&fabs(v[i].m_y - p.m_y) < TOL&&fabs(v[i].m_z - p.m_z) < TOL)
			return true;
	}
	return false;
}

int CP_FlowComplex::LocatePoint(const CP_Point3D &p)
{
	for (unsigned int i = 0; i < m_0cells.size(); i++)
	{
		if (fabs(m_0cells[i].m_x - p.m_x) < TOL&&fabs(m_0cells[i].m_y - p.m_y) < TOL&&fabs(m_0cells[i].m_z - p.m_z) < TOL)
			return i;
	}
	return -1;
}

void CP_FlowComplex::Insert2cellInto1cell(const CP_2cell& p2cell)
{
	for(unsigned int j=0;j<p2cell.m_boundary.size();j++)
	{
		int curve=LocateSegment(m_1cells,p2cell.m_boundary[j]);
		if(curve==-1)
		{
			CurveSegment *c=new CurveSegment(p2cell.m_boundary[j].GetPointIndex(0),p2cell.m_boundary[j].GetPointIndex(1));
			m_1cells.push_back(c);
			c->degree=1;
			c->incident2cell.push_back(Locate2cell(p2cell.index));
		}else
		{
			m_1cells[curve]->degree++;
			m_1cells[curve]->incident2cell.push_back(Locate2cell(p2cell.index));
		}
	}//j
}

int CP_FlowComplex::LocateSegment(const vector<CurveSegment*> &curveVec,const CurveSegment &line)
{

	for (unsigned int k = 0; k < curveVec.size(); k++)
	{
		if ((curveVec[k]->sp==line.sp&&curveVec[k]->ep==line.ep)||(curveVec[k]->sp==line.ep&&curveVec[k]->ep==line.sp))
		{
			return k;
		}
	}//k
	return -1;
}

int CP_FlowComplex::LocateTriangle(const vector<CP_Triganle3D*>& triangles,const CP_Triganle3D &tri)
{
	for (unsigned int i=0;i<triangles.size();i++)
	{
		unsigned int num=0;
		for(unsigned int m=0;m<3;m++)
		{
			for(unsigned int n=0;n<3;n++)
			{
				if(triangles[i]->m_points[m]==tri.m_points[n])
				{
					num++;
				}
			}//n
		}//m
		if(num==3)
		{
			return i;
		}
	}//i
	return -1;
}

int CP_FlowComplex::Locate2cell(int _2cell)
{
	for( unsigned int i=0;i<m_2cells.size();i++)
	{
		if(m_2cells[i]->index==_2cell)
			return i;
	}
	return -1;
}

vector<int> CP_FlowComplex::GetIncidentTri(const CP_Point3D& v1,const CP_Point3D& v2)
{
	vector<int> v;
	for(unsigned int i=0;i<v1.m_adjTriangle.size();i++)
	{
		for(unsigned int j=0;j<v2.m_adjTriangle.size();j++)
		{
			if(v1.m_adjTriangle[i]==v2.m_adjTriangle[j])
				v.push_back(v1.m_adjTriangle[i]);
		}
	}
	return v;
}

void CP_FlowComplex::SetAdjTriangle()
{
	//为每个点关联三角形
	for(unsigned int i=0;i<tricells.size();i++)
	{
		if(m_2cells[Locate2cell(tricells[i]->_2cell)]->flag)
			for(unsigned int k=0;k<3;k++)
				m_0cells[tricells[i]->m_points[k]].m_adjTriangle.push_back(i);
	}

	for(unsigned int i=0;i<tricells.size();i++)
	{
		if(m_2cells[Locate2cell(tricells[i]->_2cell)]->flag)
			for(unsigned int j=0;j<3;j++)
			{
				vector<int> v=GetIncidentTri(m_0cells[tricells[i]->m_points[j%3]],m_0cells[tricells[i]->m_points[(j+1)%3]]);
				for(unsigned int k=0;k<v.size();k++)
				{
					if(i!=v[k])
						tricells[i]->m_adjTriangle.push_back(v[k]);
				}
			}
	}
}

void CP_FlowComplex::SetAdjPatch()
{
	for(unsigned int i=0;i<m_patches.size();i++)
	{
		if(m_patches[i]->flag){
			for(unsigned int j=0;j<m_patches[i]->boundary.size();j++)
			{
				CP_Triganle3D* ptri=tricells[m_patches[i]->boundary[j]._triangle];//本2cell中与此边界相关的triganle
				for(unsigned int k=0;k<ptri->m_adjTriangle.size();k++)
				{
					CP_2cell* p2celladj=m_2cells[Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)];
					if(p2celladj->patch!=-1&&p2celladj->patch!=m_patches[i]->index&&m_patches[p2celladj->patch]->flag){
						vector<int>::iterator it=find(m_patches[i]->m_adjPatch.begin(),m_patches[i]->m_adjPatch.end(),p2celladj->patch);
						if(it==m_patches[i]->m_adjPatch.end())
							m_patches[i]->m_adjPatch.push_back(p2celladj->patch);
					}
				}//k
			}//j
		}//flag
	}//i
}

void CP_FlowComplex::Convert2cellNormal(const CP_2cell& _2cell,CP_Triganle3D *tri)
{
	tri->normalsetted=true;
	swap(tri->m_points[0],tri->m_points[2]);
	Spread2cellTri(_2cell.index,tri);
	/*for(unsigned unsigned int i=0;i<_2cell.m_triangle.size();i++)
	{
	swap(tricells[_2cell.m_triangle[i]]->m_points[0],tricells[_2cell.m_triangle[i]]->m_points[1]);
	}*/
}

void CP_FlowComplex::ConvertpatchNormal(CP_Triganle3D* tri)
{
	CP_2cell *p2cell=m_2cells[Locate2cell(tri->_2cell)];
	Convert2cellNormal(*p2cell,tri);
	p2cell->visited=true;
	SpreadPatch2cell(p2cell->patch,p2cell);
}

int CommonEdgeReverse(CP_Triganle3D &ltri,CP_Triganle3D &rtri)
{//若方向相同返回rtri的源点，否则返回-1
	int abc=0;
	int sp[2]={-1,-1};//公共边顶点
	int rsri=-1,lsri=-1;;
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

	if(ltri.m_points[(lsri+1)%3]==rtri.m_points[(rsri+1)%3])
	{//调整为逆序
		return rsri;
	}
	else
		return -1;
}

double CP_FlowComplex::Dihedral(const CP_Triganle3D &ltri,const CP_Triganle3D &rtri)
{
	int abc=0;
	int sp[2]={-1,-1};//公共边顶点
	int rsri=-1,lsri=-1;;
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

	for(int i=0;i<3;++i){
		bool exist=false;
		for(int j=0;j<2;++j){
			if(sp[j]==ltri.m_points[i])
				exist=true;
		}
		if(!exist)
			lsri=ltri.m_points[i];
	}

	for(int i=0;i<3;++i){
		bool exist=false;
		for(int j=0;j<2;++j){
			if(sp[j]==rtri.m_points[i])
				exist=true;
		}
		if(!exist)
			rsri=rtri.m_points[i];
	}

	CP_Vector3D nl=(m_0cells[sp[0]]-m_0cells[lsri])^(m_0cells[sp[1]]-m_0cells[lsri]);
	CP_Vector3D nr=(m_0cells[sp[1]]-m_0cells[rsri])^(m_0cells[sp[0]]-m_0cells[rsri]);
	nl.Normalize();
	nr.Normalize();
	return nl*nr;
}

void CP_FlowComplex::Spread2cellTri(int _2cell,CP_Triganle3D* tri)
{
	for(unsigned int i=0;i<tri->m_adjTriangle.size();i++)
	{//邻接三角形公共边的方向应与tri相同
		if(!tricells[tri->m_adjTriangle[i]]->normalsetted&&tricells[tri->m_adjTriangle[i]]->_2cell==_2cell)
		{//同属一个2cell且未被访问,测试法向
			tricells[tri->m_adjTriangle[i]]->normalsetted=true;
			int rsri=CommonEdgeReverse(*tri,*tricells[tri->m_adjTriangle[i]]);
			if(rsri!=-1)
			{   //stl swap
				swap(tricells[tri->m_adjTriangle[i]]->m_points[(rsri+1)%3],tricells[tri->m_adjTriangle[i]]->m_points[(rsri+2)%3]);
			}
			Spread2cellTri(_2cell,tricells[tri->m_adjTriangle[i]]);
		}//normalsetted
	}//i
}

void CP_FlowComplex::Set2cellNormalConsensus()
{
	for(unsigned int i=0;i<m_2cells.size();i++)
	{
		//每个2cell内三角面片一致
		tricells[m_2cells[i]->m_triangle[0]]->normalsetted=true;
		Spread2cellTri(m_2cells[i]->index,tricells[m_2cells[i]->m_triangle[0]]);
	}
}

void CP_FlowComplex::SpreadPatch2cell(int _patch,CP_2cell *p2cell)
{
	for(unsigned int j=0;j<p2cell->m_boundary.size();j++)
	{
		CP_Triganle3D* ptri=tricells[p2cell->m_boundary[j]._triangle];//本2cell中与此边界相关的triganle
		for(unsigned int k=0;k<ptri->m_adjTriangle.size();k++)
		{
			CP_2cell* p2celladj=m_2cells[Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)];
			if(!p2celladj->visited&&_patch==p2celladj->patch)
			{//与此三角面片相邻的另一个2cell Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)中的三角形tricells[ptri->m_adjTriangle[k]]
				p2celladj->visited=true;
				int rsri=CommonEdgeReverse(*ptri,*tricells[ptri->m_adjTriangle[k]]);
				if(rsri!=-1)
				{
					Convert2cellNormal(*p2celladj,tricells[ptri->m_adjTriangle[k]]);
				}
				SpreadPatch2cell(_patch,p2celladj);

			}
		}//k
	}//j
}

void CP_FlowComplex::PatchNormalConsensus()
{
	for(unsigned int i=0;i<m_patches.size();i++)
	{
		if(m_patches[i]->flag&&m_patches[i]->m_2cells.size()>0){//加了flag
			m_2cells[m_patches[i]->m_2cells[0]]->visited=true;
			SpreadPatch2cell(m_patches[i]->index,m_2cells[m_patches[i]->m_2cells[0]]);
		}
	}
}

void CP_FlowComplex::PatchNormalConsensus(int i)
{
	if(m_patches[i]->m_2cells.size()>0){
		m_2cells[m_patches[i]->m_2cells[0]]->visited=true;
		SpreadPatch2cell(m_patches[i]->index,m_2cells[m_patches[i]->m_2cells[0]]);
	}
}

void CP_FlowComplex::SpreadTriangle(CP_Triganle3D* tri)
{
	for(unsigned int i=0;i<tri->m_adjTriangle.size();i++)
	{//&&m_2cells[tricells[tri->m_adjTriangle[i]]->_2cell]->flag
		if(!tricells[tri->m_adjTriangle[i]]->normalsetted)
		{//没访问过
			tricells[tri->m_adjTriangle[i]]->normalsetted=true;
			int rsri=CommonEdgeReverse(*tri,*tricells[tri->m_adjTriangle[i]]);
			if(rsri!=-1)
			{
				int temp=tricells[tri->m_adjTriangle[i]]->m_points[(rsri+1)%3];
				tricells[tri->m_adjTriangle[i]]->m_points[(rsri+1)%3]=tricells[tri->m_adjTriangle[i]]->m_points[(rsri+2)%3];
				tricells[tri->m_adjTriangle[i]]->m_points[(rsri+2)%3]=temp;
			}
			SpreadTriangle(tricells[tri->m_adjTriangle[i]]);
		}//if normalsetted
	}//i
}

void  CP_FlowComplex::SetNormals()
{
	for(unsigned int i=0;i<tricells.size();i++)
	{
		if(!tricells[i]->normalsetted&&m_2cells[Locate2cell(tricells[i]->_2cell)]->flag)
		{
			tricells[i]->normalsetted=true;
			SpreadTriangle(tricells[i]);
		}
	}
}  

void CP_FlowComplex::SetPatchNormal()
{
	for(unsigned int i=0;i<m_patches.size();i++)
	{
		if(!m_patches[i]->visited&&m_patches[i]->flag)
		{
			m_patches[i]->visited=true;
			CP_2cell *p2cell=m_2cells[m_patches[i]->m_2cells[0]];
			CP_Vector3D ntri = (m_0cells[tricells[p2cell->m_triangle[0]]->m_points[1]] - m_0cells[tricells[p2cell->m_triangle[0]]->m_points[0]]) ^ 
				(m_0cells[tricells[p2cell->m_triangle[0]]->m_points[2]] - m_0cells[tricells[p2cell->m_triangle[0]]->m_points[0]]);
			CP_Vector3D ncp=m_0cells[tricells[p2cell->m_triangle[0]]->m_points[0]]-cp;
			if(ntri*ncp<0)
				ConvertpatchNormal(tricells[p2cell->m_triangle[0]]);
			SpreadPatchNormal(*m_patches[i]);
		}
	}//i
}

void CP_FlowComplex::SpreadPatchNormal(const CP_Patch& pPatch)
{
	for(unsigned int j=0;j<pPatch.boundary.size();j++)
	{
		CP_Triganle3D* ptri=tricells[pPatch.boundary[j]._triangle];//本2cell中与此边界相关的triganle
		for(unsigned int k=0;k<ptri->m_adjTriangle.size();k++)
		{
			CP_2cell* p2celladj=m_2cells[Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)];
			if(p2celladj->patch!=-1){
				CP_Patch* pPatchadj=m_patches[p2celladj->patch];
				if(p2celladj->patch!=pPatch.index&&!pPatchadj->visited&&pPatchadj->flag)
				{//与此三角面片相邻的另一个2cell Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)中的三角形tricells[ptri->m_adjTriangle[k]]
					pPatchadj->visited=true;
					int rsri=CommonEdgeReverse(*ptri,*tricells[ptri->m_adjTriangle[k]]);
					if(rsri!=-1)
					{
						ConvertpatchNormal(tricells[ptri->m_adjTriangle[k]]);
					}
					SpreadPatchNormal(*pPatchadj);
				}
			}
		}//k
	}
}

void CP_FlowComplex::Set2cellNormal()
{
	for(int i=desN-1;i>=0;i--)
	{
		if(m_2cells[i]->flag&&!m_2cells[i]->visited)
		{
			m_2cells[i]->visited=true;
			CP_Vector3D ntri = (m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[1]] - m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[0]]) ^ 
				(m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[2]] - m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[0]]);
			CP_Vector3D ncp=m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[0]]-cp;
			if(ntri*ncp<0)
				Convert2cellNormal(*m_2cells[i],tricells[m_2cells[i]->m_triangle[0]]);
			Spread2cellNormal(*m_2cells[i]);
		}//flag visited
	}//i
}

void CP_FlowComplex::Spread2cellNormal(const CP_2cell& p2cell)
{
	for(unsigned int j=0;j<p2cell.m_boundary.size();j++)
	{
		CP_Triganle3D* ptri=tricells[p2cell.m_boundary[j]._triangle];//本2cell中与此边界相关的triganle
		for(unsigned int k=0;k<ptri->m_adjTriangle.size();k++)
		{
			CP_2cell* p2celladj=m_2cells[Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)];
			if(tricells[ptri->m_adjTriangle[k]]->_2cell!=p2cell.index&&p2celladj->flag&&!p2celladj->visited)
			{//与此三角面片相邻的另一个2cell Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)中的三角形tricells[ptri->m_adjTriangle[k]]
				p2celladj->visited=true;
				int rsri=CommonEdgeReverse(*ptri,*tricells[ptri->m_adjTriangle[k]]);
				if(rsri!=-1)
				{
					Convert2cellNormal(*p2celladj,tricells[ptri->m_adjTriangle[k]]);
				}
				Spread2cellNormal(*p2celladj);
			}
			//p2cell.m_adj2cell.push_back(Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell));
		}//k
	}//j
}

void CP_FlowComplex::SetCreatorAndDestoryer()
{
	vector<CurveSegment*> vboundary;
	for(unsigned int i=0;i<m_2cells.size();i++)
	{
		bool exist=true;
		for(unsigned int j=0;j<m_2cells[i]->m_boundary.size();j++)
		{
			int curve=LocateSegment(vboundary,m_2cells[i]->m_boundary[j]);
			if(curve==-1)
			{
				exist=false;
				vboundary.push_back(&m_2cells[i]->m_boundary[j]);
				m_2cells[i]->m_boundary[j].degree=1;
				m_2cells[i]->m_boundary[j].incident2cell.push_back(i);
			}else
			{
				vboundary[curve]->degree++;
				vboundary[curve]->incident2cell.push_back(i);
			}
		}//j

		for(unsigned int  j=0;j<vboundary.size();j++)
		{
			vboundary[j]->ResetDegreee();
		}

		if(exist)
		{//此2cell边界都存在，检查是否闭合空间的patch数增加，若增加，此面片为creator，否则destoryer
			int pCloseVoid=CheckClosedVoid(&vboundary,i);
			if(pCloseVoid!=0)
			{
				m_2cells[i]->type=1;

				for(unsigned int j=0;j<m_2cells[i]->m_boundary.size();j++)
				{//重置m_1cell
					int curve=LocateSegment(vboundary,m_2cells[i]->m_boundary[j]);
					vboundary[curve]->degree--;
					vboundary[curve]->tmpdegree--;
					vboundary[curve]->incident2cell.pop_back();
				}//j
				CP_3cell *p3cell=new CP_3cell();
				m_3cells.push_back(p3cell);
				p3cell->m_2cells.push_back(m_2cells[i]->index);
				//cout<<i<<"creator"<<pCloseVoid<<endl;
				for(unsigned int j=0;j<=i;j++)
				{
					if(m_2cells[j]->flag&&m_2cells[j]->type==0)
						p3cell->m_2cells.push_back(m_2cells[j]->index);
				}
			}//pclosevoid
		}//exist
		//重置所有flag true
		Reset2cellFlag(i);
	}//i

	//所有vboundary的数据归零
	for(unsigned int i=0;i<vboundary.size();i++)
	{
		vboundary[i]->degree=0;
		vboundary[i]->tmpdegree=0;
		vector<int>().swap(vboundary[i]->incident2cell);
	}//i
}

void CP_FlowComplex::CutBranch(vector<CurveSegment*> *vboundary,const CurveSegment& curve)
{
	//对使得curve存在的唯一一个2cell消去操作
	for (unsigned int i = 0; i < curve.incident2cell.size(); i++)
	{
		int icell=curve.incident2cell[i];
		if(m_2cells[icell]->flag)
		{
			m_2cells[icell]->flag=false;
			//所有边界线段度数-1并检查如果度数=1做剪枝操作
			for(unsigned int j=0;j<m_2cells[icell]->m_boundary.size();j++)
			{
				int icurve=LocateSegment(*vboundary,m_2cells[icell]->m_boundary[j]);
				if((*vboundary)[icurve]->tmpdegree>0)
				{
					(*vboundary)[icurve]->tmpdegree--;
					if((*vboundary)[icurve]->tmpdegree==1)
						CutBranch(vboundary,*(*vboundary)[icurve]);
				}
			}//j
		}//2cell flag
	}//i
}

int CP_FlowComplex::CheckClosedVoid(vector<CurveSegment*> *vboundary,int len)
{
	////剪枝法判断边集
	for(unsigned int i=0;i<vboundary->size();i++)
	{
		if((*vboundary)[i]->tmpdegree==1)
			CutBranch(vboundary,*(*vboundary)[i]);
	}
	//如果0-i 2cell有存在，返回存在的patch数
	int num=0;
	for(int i=0;i<=len;i++)
	{
		if(m_2cells[i]->flag&&m_2cells[i]->type==0)
			num++;
	}

	return num;
}

void CP_FlowComplex::Reset2cellFlag(int len)
{
	for(int i=0;i<=len;i++)
	{
		m_2cells[i]->flag=true;
	}
};


void CP_FlowComplex::Reset2cellVisited()
{
	for(unsigned int i=0;i<m_2cells.size();i++)
	{
		m_2cells[i]->visited=false;
	}
}

void CP_FlowComplex::ResetPatchVisited()
{
	for(unsigned int i=0;i<m_patches.size();i++)
		m_patches[i]->visited=false;
}

void CP_FlowComplex::Expand2cell(const CP_2cell& p2cell,const vector<CurveSegment*> vb,CP_Patch *pPatch)
{
	m_2cells[Locate2cell(p2cell.index)]->visited=true;
	for(unsigned int i=0;i<p2cell.m_boundary.size();i++)
	{
		int curve=LocateSegment(vb,p2cell.m_boundary[i]);
		if(vb[curve]->degree==2&&vb[curve]->isBoundary==-1)
		{
			//incident 2cell 加入到p2cell中
			for(unsigned int j=0;j<vb[curve]->incident2cell.size();j++)
			{
				int _2cell=vb[curve]->incident2cell[j];
				if(m_2cells[_2cell]->index!=p2cell.index&&!m_2cells[_2cell]->visited)
				{
					pPatch->m_2cells.push_back(_2cell);
					Expand2cell(*m_2cells[_2cell],vb,pPatch);
				}
			}//j
		}//degree 1
	}//i
}

CP_Vector3D CP_FlowComplex::GetTangent(const CurveSegment &curve) const
{
	CP_Vector3D tangent = m_0cells[curve.ep] - m_0cells[curve.sp];
	tangent.Normalize();
	return tangent;
}

void CP_FlowComplex::ResetTriNormalset()
{
	for(unsigned int i=0;i<tricells.size();i++)
	{
		tricells[i]->normalsetted=false;
	}
}

bool colorNumCmp(CP_Patch* l,CP_Patch* r)
{
	return l->m_adjPatch.size()<r->m_adjPatch.size();
}
void CP_FlowComplex::SpreadPatchColor(CP_Patch* pPatch)
{
	pPatch->visited=true;
	vector<int> colored;
	for(unsigned int i=0;i<pPatch->m_adjPatch.size();i++)
	{
		if(m_patches[pPatch->m_adjPatch[i]]->visited)
			colored.push_back(m_patches[pPatch->m_adjPatch[i]]->color);
	}
	const int NUM_OF_COLORS=6;
	for(unsigned int i=0;i<NUM_OF_COLORS;i++)
	{
		vector<int>::iterator it=find(colored.begin(),colored.end(),i);
		if(it==colored.end())
		{
			pPatch->color=i;
			break;
		}
	}
	//spread
	for(unsigned int i=0;i<pPatch->m_adjPatch.size();i++)
	{
		if(!m_patches[pPatch->m_adjPatch[i]]->visited)
			SpreadPatchColor(m_patches[pPatch->m_adjPatch[i]]);
	}
}
void CP_FlowComplex::SetPatchColor()
{
	vector<CP_Patch*> vP;
	for(unsigned int i=0;i<m_patches.size();i++)
		if(m_patches[i])
			vP.push_back(m_patches[i]);

	sort(vP.begin(),vP.end(),colorNumCmp);
	//从vp中第一个，即相邻patch最多的开始
	for(int i=vP.size()-1;i>=0;i--)
	{
		if(!vP[i]->visited)
			SpreadPatchColor(vP[i]);
	}
}

void CP_FlowComplex::ClearAll()
{
	desN=0;
	_3cellN=0;
	show=0;
	inputPoints=0;
	inputCurveSegments=0;
	oripatches=0;
	ori2cells=0;
	respatches=0;
	res_triangle_num=0;
	minx=maxx=miny=maxy=minz=maxz=0.0;
	perweight=1;
	biweight=0;
	mergededge=0;
	graph.Reset();

	vector<CP_Point3D>().swap(m_0cells);
	vector<CP_Point3D>().swap(m_critical);
	for(unsigned int i=0;i<m_1cells.size();i++)
		delete m_1cells[i];
	vector<CurveSegment*>().swap(m_1cells);
	for(unsigned int i=0;i<m_2cells.size();i++)
		delete m_2cells[i];
	vector<CP_2cell*>().swap(m_2cells);
	for(unsigned int i=0;i<m_3cells.size();i++)
		delete m_3cells[i];
	vector<CP_3cell*>().swap(m_3cells);
	for(unsigned int i=0;i<m_patches.size();i++)
		delete m_patches[i];
	vector<CP_Patch *>().swap(m_patches);
	for(unsigned int i=0;i<m_cpatches.size();i++)
		delete m_cpatches[i];
	vector<CP_Patch *>().swap(m_cpatches);
	for(unsigned int i=0;i<delauny2cells.size();i++)
		delete delauny2cells[i];
	vector<CP_Triganle3D*>().swap(delauny2cells);
	vector<CP_Triganle3D*>().swap(non_gabriel_triangles);//Delauny中的部分三角形
	vector<CP_PolyLine3D>().swap(m_PolyLine);
	for(unsigned int i=0;i<tricells.size();i++)
		delete tricells[i];
	vector<CP_Triganle3D*>().swap(tricells);
	for(unsigned int i=0;i<visitedtri.size();i++)
		delete visitedtri[i];
	vector<CP_Triganle3D*>().swap(visitedtri);
	vector<int>().swap(vjoint);
	vector<CircumPoint>().swap(m_circums);
	vector<int>().swap(topo);

	delaunyexist.clear();
	mergededge=0;
	vector<vector<int>>().swap(overlap_by_delauny);
	vector<vector<int>>().swap(curve_incident_patch);
	vector<vector<int>>().swap(cycles);
	vector<int>().swap(interior_patches);
	vector<int>().swap(connectedPatches);
	vector<int>().swap(vec_curve_degree);
}

void CP_FlowComplex::DrawPoints()
{
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(6.0f);
	for (unsigned int j = 0; j < inputPoints; j++)
	{
		glBegin(GL_POINTS);//必须是加上s，要不然显示不了
		glNormal3f(1.0f,1.0f,1.0f);
		glVertex3f(m_0cells[j].m_x, m_0cells[j].m_y,m_0cells[j].m_z);
		glEnd();
	}
	//for (unsigned int j = 0; j < vjoint.size(); j++)
	//{
	//	glBegin(GL_POINTS);//必须是加上s，要不然显示不了
	//	glNormal3f(1.0f,1.0f,1.0f);
	//	glVertex3f(m_0cells[vjoint[j]].m_x, m_0cells[vjoint[j]].m_y,m_0cells[vjoint[j]].m_z);
	//	glEnd();
	//}
}

void CP_FlowComplex::DrawDelaunyTriangles()
{
	glColor4f(0.7,0.7,0.7,1.0);
	for (unsigned int i = 0; i <delauny2cells.size(); i++)
	{
		CP_Triganle3D *pTri = delauny2cells[i];
		DrawTriangle(*pTri);

		glDepthMask(GL_FALSE);
		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		glLineWidth(1.1f);
		glColor4f(0.7,0.7,0.7,1.0);
		glBegin(GL_LINE_LOOP);
		for (unsigned int j = 0; j < 3; j++){
			//glNormal3f(0, 0, 0);
			glVertex3f(m_0cells[pTri->m_points[j]].m_x, m_0cells[pTri->m_points[j]].m_y, m_0cells[pTri->m_points[j]].m_z);
		}
		glEnd();
		glDisable(GL_LINE_SMOOTH);
		glDepthMask(GL_TRUE);
	}
}

void CP_FlowComplex::DrawNonGabrielTriangles()
{
	for (unsigned int i = 0; i <non_gabriel_triangles.size(); i++)
	{
		glColor4f(0.6,0.6,0.9,0.1);
		CP_Triganle3D *pTri = non_gabriel_triangles[i];
		DrawTriangle(*pTri);

		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH, GL_NICEST);
		glLineWidth(0.1f);
		glColor4f(0.7,0.7,0.7,0.5);
		glBegin(GL_LINE_LOOP);
		for (unsigned int j = 0; j < 3; j++){
			//glNormal3f(0, 0, 0);
			glVertex3f(m_0cells[pTri->m_points[j]].m_x, m_0cells[pTri->m_points[j]].m_y, m_0cells[pTri->m_points[j]].m_z);
		}glEnd();
		glDisable(GL_LINE_SMOOTH);
	}
}

void CP_FlowComplex::DrawTriangle(const CP_Triganle3D &tri)
{
	CP_Vector3D ntmp = (m_0cells[tri.m_points[1]] - m_0cells[tri.m_points[0]]) ^ (m_0cells[tri.m_points[2]] -m_0cells[tri.m_points[0]]);
	glBegin(GL_POLYGON);
	glNormal3f(ntmp.m_x, ntmp.m_y, ntmp.m_z);
	glVertex3f(m_0cells[tri.m_points[0]].m_x, m_0cells[tri.m_points[0]].m_y, m_0cells[tri.m_points[0]].m_z);
	glNormal3f(ntmp.m_x, ntmp.m_y, ntmp.m_z);
	glVertex3f(m_0cells[tri.m_points[1]].m_x, m_0cells[tri.m_points[1]].m_y, m_0cells[tri.m_points[1]].m_z);
	glNormal3f(ntmp.m_x, ntmp.m_y, ntmp.m_z);
	glVertex3f(m_0cells[tri.m_points[2]].m_x, m_0cells[tri.m_points[2]].m_y, m_0cells[tri.m_points[2]].m_z);
	glEnd();

}

void CP_FlowComplex::DrawTriangleBoundary(const CP_2cell &p2cell)
{
	glPushAttrib (GL_ALL_ATTRIB_BITS);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, -1.0f);

	glDepthMask(GL_FALSE);
	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glLineWidth(1.0f);
	glColor3f(0.0,0.0,0.0);
	for(unsigned int k=0;k<p2cell.m_triangle.size();k++)
	{
		CP_Triganle3D *pTri = tricells[p2cell.m_triangle[k]];
		glBegin(GL_LINE_LOOP);
		for (unsigned int j = 0; j < 3; j++)
			glVertex3f(m_0cells[pTri->m_points[j]].m_x, m_0cells[pTri->m_points[j]].m_y, m_0cells[pTri->m_points[j]].m_z);
		glEnd();
	}//k
	glDisable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPopAttrib();
}

void CP_FlowComplex::Draw2cell(const CP_2cell &p2cell)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	if(p2cell.type==1)
	{
		glPolygonOffset(1.0f, 1.0f);
	}
	else
		glPolygonOffset(0.5f, 1.0f);
	for(unsigned int k=0;k<p2cell.m_triangle.size();k++)
	{
		CP_Triganle3D *pTri = tricells[p2cell.m_triangle[k]];
		DrawTriangle(*pTri);
	}//k
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void CP_FlowComplex::Draw2cellBoundary(const CP_2cell &p2cell)
{
	glPushAttrib (GL_ALL_ATTRIB_BITS);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.5f, -1.0f);

	glDepthMask(GL_FALSE);
	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glLineWidth(1.0f);glColor3f(0.0f, 0.0f, 0.0f);
	for(unsigned int j=0;j<p2cell.m_boundary.size();j++)
	{
		glBegin(GL_LINE_STRIP);
		glVertex3d(m_0cells[p2cell.m_boundary[j].sp].m_x,m_0cells[p2cell.m_boundary[j].sp].m_y,m_0cells[p2cell.m_boundary[j].sp].m_z);
		glVertex3d(m_0cells[p2cell.m_boundary[j].ep].m_x,m_0cells[p2cell.m_boundary[j].ep].m_y,m_0cells[p2cell.m_boundary[j].ep].m_z);
		glEnd();
	}
	glDisable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPopAttrib();
}

void CP_FlowComplex::DrawPatch(const CP_Patch &pPatch){
	for (unsigned int j = 0; j <pPatch.m_2cells.size(); j++)
	{
		CP_2cell *p2cell = m_2cells[pPatch.m_2cells[j]];
		Draw2cell(*p2cell);
	}//j
}

void CP_FlowComplex::DrawPatchBoundary(const CP_Patch &pPatch)
{
	glPushAttrib (GL_ALL_ATTRIB_BITS);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.5f, -1.0f);

	glDepthMask(GL_FALSE);
	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	for(unsigned int j=0;j<pPatch.m_boundary.size();j++)
	{
		
		glLineWidth(3.5f);
		if(pPatch.m_boundary[j]->newdegree==0)
			glColor3f(0.0,0.0,0.0);
		else if(pPatch.m_boundary[j]->newdegree==1)
			glColor3f(1.0,0.0,0.0);
		else if(pPatch.m_boundary[j]->newdegree==2)
			glColor3f(0.0, 1.0, 0.0);
		else if(pPatch.m_boundary[j]->newdegree>=3)
			glColor3f(0.173, 0.51, 1.0);
		glBegin(GL_LINE_STRIP);
		glVertex3d(m_0cells[pPatch.m_boundary[j]->sp].m_x,m_0cells[pPatch.m_boundary[j]->sp].m_y,m_0cells[pPatch.m_boundary[j]->sp].m_z);
		glVertex3d(m_0cells[pPatch.m_boundary[j]->ep].m_x,m_0cells[pPatch.m_boundary[j]->ep].m_y,m_0cells[pPatch.m_boundary[j]->ep].m_z);
		glEnd();
		
	}

	//if(pPatch.wrong)
	//{//如果wrong，画绿的边界
	//	glColor3f(0.0, 1.0, 0.0);
	//	glLineWidth(4.5f);
	//	vector<int> poly;
	//	for(unsigned i=0;i<pPatch.forest.size();++i){
	//		GraphList *ptree=pPatch.forest[i];
	//		for(unsigned int j=0;j<ptree->adjList.size();++j)
	//		{
	//			EdgeNode *e=ptree->adjList[j].firstedge;
	//			while(e)
	//			{
	//				if(find(poly.begin(),poly.end(),e->polyindex)==poly.end())
	//					poly.push_back(e->polyindex);
	//				e=e->next;
	//			}
	//		}
	//	}
	//	for(auto p:poly)
	//		m_PolyLine[p].Draw();
	//}

	glDisable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPopAttrib();
}

void CP_FlowComplex::DrawPatchBoundary(const CP_Patch &pPatch,bool connection,bool cycle,int which,bool RMF)
{
	/*glPushAttrib (GL_ALL_ATTRIB_BITS);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.5f, -1.0f);*/

	//glDepthMask(GL_FALSE);
	if(pPatch.wrong&&(connection||cycle))
	{   
		//如果wrong，画绿的边界
		if(connection){
			glColor4f(0.0, 1.0, 0.0,1.0);
			glLineWidth(4.5f);
			vector<int> poly;
			for(unsigned i=0;i<pPatch.forest.size();++i){
				GraphList *ptree=pPatch.forest[i];
				for(unsigned int j=0;j<ptree->adjList.size();++j)
				{
					EdgeNode *e=ptree->adjList[j].firstedge;
					while(e)
					{
						if(find(poly.begin(),poly.end(),e->polyindex)==poly.end())
							poly.push_back(e->polyindex);
						e=e->next;
					}
				}//j
			}//i
			for(auto p:poly)
				m_PolyLine[p].Draw();
		}
		if(cycle){
			int num=pPatch.cycle.size();
			if(num>0){
				for(unsigned int j=0;j<pPatch.cycle[which].size();++j)
				{
					glColor4f(0.7,0.0, 0.0,1.0);
					glLineWidth(5);
					m_PolyLine[pPatch.cycle[which][j]].Draw();
				}
			}
		}
	}else{
		for(unsigned int j=0;j<pPatch.m_boundary.size();j++)
		{
			glEnable (GL_LINE_SMOOTH);
			glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
			glLineWidth(3.5f);
			if(pPatch.m_boundary[j]->newdegree==1&&!connection)//wrong false,connection true用来表示内部面
				glColor3f(1.0, 0.447, 0.463);
			else if(pPatch.m_boundary[j]->newdegree==2&&!connection)
				glColor3f(0.4, 1.0, 0.4);
				//glColor3f(0.7, 0.7, 0.7);
			else
				glColor3f(0.33, 0.47, 0.93);
			glBegin(GL_LINE_STRIP);
			glVertex3d(m_0cells[pPatch.m_boundary[j]->sp].m_x,m_0cells[pPatch.m_boundary[j]->sp].m_y,m_0cells[pPatch.m_boundary[j]->sp].m_z);
			glVertex3d(m_0cells[pPatch.m_boundary[j]->ep].m_x,m_0cells[pPatch.m_boundary[j]->ep].m_y,m_0cells[pPatch.m_boundary[j]->ep].m_z);
			glEnd();
			glDisable(GL_LINE_SMOOTH);
		}

		if(!pPatch.wrong&&RMF){
			for(unsigned int i=0;i<pPatch.r.size();++i)
			{
				glColor4f(0.5,0.1,0.1,1.0);
				CP_Vector3D vec(pPatch.r[i].m_x,pPatch.r[i].m_y,pPatch.r[i].m_z);
				double len = vec.GetLength( );
				vec.m_x /= len*20; // 注意: 这里没有处理除数为0的情况
				vec.m_y /= len*20; 
				vec.m_z /= len*20; 
				CP_Vector3D b(m_0cells[pPatch.path[i]].m_x+vec.m_x,m_0cells[pPatch.path[i]].m_y+vec.m_y,m_0cells[pPatch.path[i]].m_z+vec.m_z);
				CP_Point3D a(m_0cells[pPatch.path[i]].m_x,m_0cells[pPatch.path[i]].m_y,m_0cells[pPatch.path[i]].m_z);
				glBegin(GL_LINES);
				glVertex3f(a.m_x,a.m_y,a.m_z);
				glVertex3f(b.m_x,b.m_y,b.m_z);
				glEnd();
			}
		}
	}

	/*glDepthMask(GL_TRUE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPopAttrib();*/
}


void CP_FlowComplex::DrawDarts()
{
	for(unsigned int i=0;i<graph.adjList.size();++i)
	{
		if(graph.adjList[i].darts.size()>0)
		{
			for(unsigned int j=0;j<graph.adjList[i].darts.size();++j)
			{
				int first=graph.adjList[i].darts[j].first;
				int second=graph.adjList[i].darts[j].second;
				CP_Point3D next1,next2;
				if(m_PolyLine[first].s==i)
					next1=m_PolyLine[first].m_points[1];
				else
					next1=m_PolyLine[first].m_points[m_PolyLine[first].m_points.size()-2];

				if(m_PolyLine[second].s==i)
					next2=m_PolyLine[second].m_points[1];
				else
					next2=m_PolyLine[second].m_points[m_PolyLine[second].m_points.size()-2];
				glEnable (GL_LINE_SMOOTH);
				glHint (GL_LINE_SMOOTH, GL_NICEST);
				glLineWidth(4.0f);
				glColor3f(0.0,0.0,1.0);

				glBegin(GL_LINES);
				glVertex3f(m_0cells[i].m_x, m_0cells[i].m_y, m_0cells[i].m_z);
				glVertex3f(next1.m_x, next1.m_y, next1.m_z);
				glEnd();

				glBegin(GL_LINES);
				glVertex3f(m_0cells[i].m_x, m_0cells[i].m_y, m_0cells[i].m_z);
				glVertex3f(next2.m_x, next2.m_y, next2.m_z);
				glEnd();
				glDisable(GL_LINE_SMOOTH);
			}//j
		}
	}//i
}

void CP_FlowComplex::DrawVoids(int _3cell,int sel2cell,int seltriangle,bool _2cellboundary,bool triboundary,double mTrans)
{
	CP_3cell* p3cell=m_3cells[_3cell];
	for(unsigned int j=0;j<m_3cells[_3cell]->m_2cells.size();j++)
	{
		CP_2cell *p2cell=m_2cells[m_3cells[_3cell]->m_2cells[j]];
		if(sel2cell==p2cell->index||(p2cell->type==1))
			glColor4f(0.7,0.0,0.0,mTrans);
		else
			glColor4f(0.7,0.7,0.7,mTrans);

		for(unsigned int k=0;k<p2cell->m_triangle.size();k++)
		{
			CP_Triganle3D *pTri = tricells[p2cell->m_triangle[k]];
			if(seltriangle==p2cell->m_triangle[k])
				glColor4f(0.7,0.3,0.3,mTrans);
			else if(seltriangle!=-1)
				glColor4f(0.7,0.7,0.7,mTrans);
			DrawTriangle(*pTri);
		}//k
		if(_2cellboundary)
			Draw2cellBoundary(*p2cell);//显示是哪个2cell的triangle

		if(triboundary)
			DrawTriangleBoundary(*p2cell);
	}//j
}

void CP_FlowComplex::DrawFlowComplex(bool showcreators,int selcreator,bool _2cellboundary,bool triboundary){
	for (int i = 0; i <ori2cells; i++)
	{
		CP_2cell *p2cell = m_2cells[i];
		if(showcreators){
			if(p2cell->type==1){//
				if(selcreator==-1)
					glColor4f(0.7,0.3,0.3,0.6);
				else if(selcreator==i)
					glColor4f(0.85,0.7,0.32,0.9);
				else
					glColor4f(0.7,0.55,0.55,0.6);
			}
			else
				glColor4f(0.7,0.7,0.7,0.5);
		}else{
			glColor4f(0.7,0.7,0.7,1.0);
		}

		Draw2cell(*p2cell);
		if(triboundary)
			DrawTriangleBoundary(*p2cell);

		if(_2cellboundary)
			Draw2cellBoundary(*p2cell);

	}//2cell
}

void CP_FlowComplex::Calculate3cellvolume()
{
	for(unsigned int i=0;i<m_circums.size();i++)
	{
		for(unsigned int j=0;j<m_3cells.size();j++)
		{
			if(IsPointInside3cell(m_circums[i],*m_3cells[j]))
				m_3cells[j]->m_circums.push_back(i);
		}//j
	}//i

	//将每个3cell包含的四面体体积相加
	for(unsigned int i=0;i<m_3cells.size();i++)
	{
		double sum=0;
		for(unsigned int j=0;j<m_3cells[i]->m_circums.size();j++)
		{
			sum+=m_circums[m_3cells[i]->m_circums[j]].vol;
		}
		m_3cells[i]->dis3cell=sum;
		//cout<<i<<"cell:"<<m_3cells[i]->m_circums.size()<<endl;
	}
}

bool CP_FlowComplex::IsPointInside3cell(const CircumPoint& p,const CP_3cell& p3cell)
{
	vector<CP_Point3D> vp;
	int above=0,under=0;
	for(unsigned int i=0;i<p3cell.m_2cells.size();i++)
	{
		CP_2cell *p2cell=m_2cells[Locate2cell(p3cell.m_2cells[i])];
		for(unsigned int j=0;j<p2cell->m_triangle.size();j++)
		{
			if((p.m_x>=tricells[p2cell->m_triangle[j]]->minx)&&
				(p.m_x<=tricells[p2cell->m_triangle[j]]->maxx)&&
				(p.m_y>=tricells[p2cell->m_triangle[j]]->miny)&&
				(p.m_y<=tricells[p2cell->m_triangle[j]]->maxy))
			{//三角形位于p所在z轴直线可能相交的位置才计算交点
				int resultinter=IsPointZLineIntersectTriangle(p,*tricells[p2cell->m_triangle[j]],&vp);
			}
		}//j
	}//i
	for(unsigned int i=0;i<vp.size();i++)
	{
		if(vp[i].m_z>p.m_z)
			above++;
		else
			under++;
	}
	if(above%2==1&&under%2==1)
		return true;
	else
		return false;
}

int CP_FlowComplex::IsPointZLineIntersectTriangle(const CircumPoint& p,
												  const CP_Triganle3D& tri,
												  vector<CP_Point3D>* vp){
													  //交点在p之上返回1；在p之下返回-1；不相交返回0
													  //三角形所在平面的法向量
													  CP_Vector3D TriangleV;
													  //三角形的边方向向量
													  CP_Vector3D VP12, VP13;
													  //直线与平面的交点
													  CP_Point3D CrossPoint;
													  //平面方程常数项
													  double TriD;
													  CP_Vector3D LineV = CP_Point3D(p.m_x,p.m_y,p.m_z-1.0)-p;
													  /*-------计算平面的法向量及常数项-------*/
													  //point1->point2
													  VP12=m_0cells[tri.m_points[1]]-m_0cells[tri.m_points[0]];
													  //point1->point3
													  VP13=m_0cells[tri.m_points[2]]-m_0cells[tri.m_points[0]];
													  //VP12xVP13
													  TriangleV=VP12^VP13;
													  //计算常数项
													  TriD = -(TriangleV.m_x*m_0cells[tri.m_points[0]].m_x
														  + TriangleV.m_y*m_0cells[tri.m_points[0]].m_y
														  + TriangleV.m_z*m_0cells[tri.m_points[0]].m_z);
													  /*-------求解直线与平面的交点坐标---------*/
													  /* 思路：
													  *     首先将直线方程转换为参数方程形式，然后代入平面方程，求得参数t，
													  * 将t代入直线的参数方程即可求出交点坐标
													  */
													  double tempU, tempD;  //临时变量
													  tempU = TriangleV.m_x*p.m_x + TriangleV.m_y*p.m_y 
														  + TriangleV.m_z*p.m_z + TriD;
													  tempD = TriangleV.m_x*LineV.m_x + TriangleV.m_y*LineV.m_y + TriangleV.m_z*LineV.m_z;
													  //直线与平面平行或在平面上
													  if(tempD == 0.0)
													  {
														  // printf("The line is parallel with the plane.\n");
														  return 0;
													  }
													  //计算参数t
													  double t = -tempU/tempD;
													  //计算交点坐标
													  CrossPoint.m_x = LineV.m_x*t + p.m_x;
													  CrossPoint.m_y = LineV.m_y*t + p.m_y;
													  CrossPoint.m_z = LineV.m_z*t + p.m_z;
													  /*----------判断交点是否在三角形内部---------*/

													  //计算三角形三条边的长度
													  double d12 = dist(m_0cells[tri.m_points[0]], m_0cells[tri.m_points[1]]);
													  double d13 = dist(m_0cells[tri.m_points[0]], m_0cells[tri.m_points[2]]);
													  double d23 = dist(m_0cells[tri.m_points[1]], m_0cells[tri.m_points[2]]);
													  //计算交点到三个顶点的长度
													  double c1 = dist(CrossPoint, m_0cells[tri.m_points[0]]);
													  double c2 = dist(CrossPoint, m_0cells[tri.m_points[1]]);
													  double c3 = dist(CrossPoint, m_0cells[tri.m_points[2]]);
													  //求三角形及子三角形的面积
													  double areaD = Area(d12, d13, d23);  //三角形面积
													  double area1 = Area(c1, c2, d12);    //子三角形1
													  double area2 = Area(c1, c3, d13);    //子三角形2
													  double area3 = Area(c2, c3, d23);    //子三角形3
													  //根据面积判断点是否在三角形内部
													  if(fabs(area1+area2+area3-areaD) < TOL)
													  {
														  if(!ExistPoint(*vp,CrossPoint))
															  vp->push_back(CrossPoint);
														  if(CrossPoint.m_z>p.m_z)
															  return 1;
														  else if(CrossPoint.m_z<p.m_z)
															  return -1;
													  }

													  return 0;
}

void CP_FlowComplex::Set3cellDistance()
{
	for(unsigned int i=0;i<m_3cells.size();i++)
	{
		double mindistance=100000000;
		for(unsigned int j=0;j<m_3cells[i]->m_circums.size();j++)
		{
			if(m_circums[m_3cells[i]->m_circums[j]].distance<mindistance)
			{
				mindistance=m_circums[m_3cells[i]->m_circums[j]].distance;
			}
		}//circums
		//m_3cells[i]->distance=maxdistance/m_2cells[Locate2cell(m_3cells[i]->m_2cells[0])]->distance;
		m_3cells[i]->distance=mindistance;
		//cout<<m_3cells[i]->distance<<endl;
		m_3cells[i]->persistence=log(m_3cells[i]->distance/m_2cells[Locate2cell(m_3cells[i]->m_2cells[0])]->distance);
	}//3cells
}

void CP_FlowComplex::SetTriangleBound()
{
	for (unsigned int i = 0; i < tricells.size(); i++)
	{
		double tmpminx=MAX_DISTANCE;
		double tmpminy=MAX_DISTANCE;
		double tmpmaxx=FAR_Z;
		double tmpmaxy=FAR_Z;
		for(int j=0;j<3;j++)
		{
			if(m_0cells[tricells[i]->m_points[j]].m_x<tmpminx)
				tmpminx=m_0cells[tricells[i]->m_points[j]].m_x;
			if(m_0cells[tricells[i]->m_points[j]].m_y<tmpminy)
				tmpminy=m_0cells[tricells[i]->m_points[j]].m_y;
			if(m_0cells[tricells[i]->m_points[j]].m_x>tmpmaxx)
				tmpmaxx=m_0cells[tricells[i]->m_points[j]].m_x;
			if(m_0cells[tricells[i]->m_points[j]].m_y>tmpmaxy)
				tmpmaxy=m_0cells[tricells[i]->m_points[j]].m_y;
		}
		tricells[i]->minx=tmpminx;
		tricells[i]->miny=tmpminy;
		tricells[i]->maxx=tmpmaxx;
		tricells[i]->maxy=tmpmaxy;
	}
}

void CP_FlowComplex::GetPatchBoundary(int i)
{
	vector<CurveSegment> lvec;
	for(unsigned int j=0;j<m_patches[i]->m_2cells.size();j++)
	{
		CP_2cell *p2cell = m_2cells[m_patches[i]->m_2cells[j]];
		for(unsigned int k=0;k<p2cell->m_boundary.size();k++)
		{
			int lindex=ExistLineSeg(lvec,p2cell->m_boundary[k]);
			if(lindex!=-1)
			{
				lvec.erase(lvec.begin()+lindex);
			}else{
				lvec.push_back(p2cell->m_boundary[k]);
			}
		}
	}//j
	for(unsigned int j=0;j<lvec.size();j++){
		m_patches[i]->m_boundary.push_back(m_1cells[LocateSegment(m_1cells,lvec[j])]);
		m_patches[i]->boundary.push_back(lvec[j]);
	}	
}

void CP_FlowComplex::SeekCreatorPatch(int pi)
{
	CP_Patch *pPatch=new CP_Patch();
	m_patches.push_back(pPatch);
	pPatch->m_2cells.push_back(m_3cells[pi]->m_2cells[0]);
	//如果非封闭模型却要expand最大creator，边界没有加入导致消去destoryer面
	Expand2cell(*m_2cells[m_3cells[pi]->m_2cells[0]],m_1cells,pPatch);

	pPatch->index=m_patches.size()-1;
	pPatch->merged=pPatch->index;
	int _patch=m_patches.size()-1;
	for(unsigned int i=0;i<pPatch->m_2cells.size();i++)
	{
		m_2cells[pPatch->m_2cells[i]]->patch=_patch;
	}
	GetPatchBoundary(_patch);
}

void CP_FlowComplex::SeekDestoryerPatch()
{
	for (int i =desN-1; i >= 0; i--)
	{
		if(m_2cells[i]->flag&&!m_2cells[i]->visited)
		{
			CP_Patch *pPatch=new CP_Patch();
			m_patches.push_back(pPatch);
			pPatch->m_2cells.push_back(i);
			Expand2cell(*m_2cells[i],m_1cells,pPatch);

			int _patch=m_patches.size()-1;
			pPatch->index=_patch;
			pPatch->merged=_patch;
			for(unsigned int i=0;i<pPatch->m_2cells.size();i++)
			{
				m_2cells[pPatch->m_2cells[i]]->patch=_patch;
			}
			GetPatchBoundary(_patch);
		}
	}//i
}

int CP_FlowComplex::ConnectToPolyBothEnds(int i)
{
	//Input:曲线编号
	//Output:与一个tag为true的两端直接相连曲线的编号
	CP_PolyLine3D *poly=&m_PolyLine[i];
	int start=poly->s;
	int end=poly->e;
	EdgeNode *es=graph.adjList[start].firstedge;//以start为头的边表
	EdgeNode *ee=graph.adjList[end].firstedge;//以end为头的边表
	//两边表中相同的线
	while (es)
	{
		if(m_PolyLine[es->polyindex].tag){
			while (ee)
			{
				if(ee->polyindex==es->polyindex)
					return es->polyindex;
				ee=ee->next;
			}
		}
		ee=graph.adjList[end].firstedge;
		es=es->next;
	}
	return -1;
}

void CP_FlowComplex::FindCyclesForaCurve(int i)
{   //查找包含编号为i的至少2个最小环，从i的start出发，目标点为end，BFS
	//长度为n的为1个，然后长度为n+1的一层所有几个
	CP_PolyLine3D *poly=&m_PolyLine[i];
	double delta=0.005;
	if(poly->s==poly->e)
	{//单个曲线头尾相连表示一环面 
		vector<int> newpath;
		newpath.push_back(poly->s);
		newpath.push_back(i);
		poly->cycle.push_back(newpath);
		return;
	}

	queue<vector<int>> paths;//每个vector中，0位置存当前路径的头端点下标，后边是路径
	int end=poly->e;//结束端点

	vector<int> vpath;
	vpath.push_back(poly->s);//加入头
	vpath.push_back(i);//加入本曲线
	paths.push(vpath);
	int last=0;
	while(!paths.empty())
	{
		vector<int> prepath=paths.front();
		paths.pop();
		if(last!=prepath.size()&&poly->cycle.size()>=2)//此深度的一层遍历完，且找到了2个以上最小环
			return;

		last=prepath.size();
		EdgeNode *e=graph.adjList[prepath[0]].firstedge;//此头的边表
		while(e)
		{//对此端点关联的所有未访问曲线BFS
			vector<int> newpath(prepath);

			if(find(newpath.begin()+1,newpath.end(),e->polyindex)==newpath.end()){
				if(newpath[0]==m_PolyLine[e->polyindex].s)//设置新头
					newpath[0]=m_PolyLine[e->polyindex].e;
				else
					newpath[0]=m_PolyLine[e->polyindex].s;

				newpath.push_back(e->polyindex);
				if(newpath[0]==end)
				{  //记录可返回
					poly->cycle.push_back(newpath);
				}else
					paths.push(newpath);
			}
			e=e->next;
		}//e
	}//path.empty
}

bool CP_FlowComplex::ContainSubCycle(const vector<int> &newpath)
{//路径形成子环
	int head=newpath[0];
	for (unsigned int i=1;i<newpath.size()-1;++i)
	{
		if(m_PolyLine[newpath[i]].s==head||m_PolyLine[newpath[i]].e==head)
			return true;
	}
	return false;
}

double CP_FlowComplex::ComputeCost(const vector<int> &path)
{
	/*if(path[0]==m_PolyLine[path.size()-1].e)
	return ComputeCycleCost(path);
	else*/
	return ComputePathCost(path);
}

double CP_FlowComplex::ComputeCycleCost(const vector<int> &path,vector<CP_Vector3D>& normal)
{
	double minCost=MAX_DISTANCE;
	int nTwistNormalNum=30;
	//compute reflection vector of R1，并调整顺序
	vector<CP_Vector3D> reflectVectorMatrices1;//V1=X(i+1)-Xi

	for(unsigned int i=0;i<path.size()-1;i++){
		CP_Vector3D t=m_0cells[path[i+1]]-m_0cells[path[i]];
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

	double anglemin=PI;
	vector<CP_Vector3D> res;
	for(int i=0;i<nTwistNormalNum;i++){
		vector<CP_Vector3D> r;
		double angle = (i+1)*2*PI/double(nTwistNormalNum);
		CP_Vector3D r0 = rotateNormal(initNormal,reflectVectorMatrices1[0],angle);//nm起始法向
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
			r.push_back(rnext);
		}//j reflectVectorMatrices1
		//加上最开始的拐角
		double tmp=acos((r0*r.back())/(r0.GetLength()*r.back().GetLength()));
		if(tmp<anglemin)
		{
			anglemin=tmp;
			res.swap(r);
		}
	}//i nTwistNormalNum
	//for(auto v:r)
	//	v.Normalize();
	normal.swap(res);
	return anglemin;
}

double CP_FlowComplex::ComputePathCost(const vector<int> &path)
{

	return 0.0;
}

CP_Vector3D CP_FlowComplex::rotateNormal(const CP_Vector3D &normal, const CP_Vector3D &axis, const double &angle)
{
	double d = normal*axis;
	double c = cos(angle);
	double s = sin(angle);
	CP_Vector3D res;
	res.m_x= axis.m_x*d*(1-c)+normal.m_x*c+(-axis.m_z*normal.m_y+axis.m_y*normal.m_z)*s;
	res.m_y= axis.m_y*d*(1-c)+normal.m_y*c+(axis.m_z*normal.m_x-axis.m_x*normal.m_z)*s;
	res.m_z= axis.m_z*d*(1-c)+normal.m_z*c+(-axis.m_y*normal.m_x+axis.m_x*normal.m_y)*s;
	return res;
}

int CP_FlowComplex::TopologicalEnable(const CurveSegment &curve,const CP_Patch &patchl,const CP_Patch &patchr)
{
	int spnextl=-1,spnextr=-1;//sp
	int epnextl=-1,epnextr=-1;//ep
	for (unsigned int i=0;i<patchl.m_boundary.size();++i)
	{
		//sp
		if (patchl.m_boundary[i]->sp==curve.sp&&patchl.m_boundary[i]->ep!=curve.ep&&patchl.m_boundary[i]->isBoundary>=0)
		{//与curve不同的另一条线的另一端
			spnextl=patchl.m_boundary[i]->ep;
		}
		else if (patchl.m_boundary[i]->ep==curve.sp&&patchl.m_boundary[i]->sp!=curve.ep&&patchl.m_boundary[i]->isBoundary>=0)
		{
			spnextl=patchl.m_boundary[i]->sp;
		}

		//ep
		if (patchl.m_boundary[i]->sp==curve.ep&&patchl.m_boundary[i]->ep!=curve.sp&&patchl.m_boundary[i]->isBoundary>=0)
		{//与curve不同的另一条线的另一端
			epnextl=patchl.m_boundary[i]->ep;
		}
		else if (patchl.m_boundary[i]->ep==curve.ep&&patchl.m_boundary[i]->sp!=curve.sp&&patchl.m_boundary[i]->isBoundary>=0)
		{
			epnextl=patchl.m_boundary[i]->sp;
		}
	}

	for (unsigned int i=0;i<patchr.m_boundary.size();++i)
	{
		//sp
		if (patchr.m_boundary[i]->sp==curve.sp&&patchr.m_boundary[i]->ep!=curve.ep&&patchr.m_boundary[i]->isBoundary>=0)
		{//与curve不同的另一条线的另一端
			spnextr=patchr.m_boundary[i]->ep;
		}
		else if (patchr.m_boundary[i]->ep==curve.sp&&patchr.m_boundary[i]->sp!=curve.ep&&patchr.m_boundary[i]->isBoundary>=0)
		{
			spnextr=patchr.m_boundary[i]->sp;
		}

		//ep
		if (patchr.m_boundary[i]->sp==curve.ep&&patchr.m_boundary[i]->ep!=curve.sp&&patchr.m_boundary[i]->isBoundary>=0)
		{//与curve不同的另一条线的另一端
			epnextr=patchr.m_boundary[i]->ep;
		}
		else if (patchr.m_boundary[i]->ep==curve.ep&&patchr.m_boundary[i]->sp!=curve.sp&&patchr.m_boundary[i]->isBoundary>=0)
		{
			epnextr=patchr.m_boundary[i]->sp;
		}
	}

	if (((spnextl!=-1)&&(spnextr!=-1))||((epnextl!=-1)&&(epnextr!=-1)))
	{//-1说明这边不是边界,有一边不重合的边在输入曲线上就符合条件,返回符合条件的数目0,1,2
		int topoOnTwoSide=0;
		if(spnextl!=spnextr&&((spnextl!=-1)&&(spnextr!=-1)))
			topoOnTwoSide++;
		if(epnextl!=epnextr&&((epnextl!=-1)&&(epnextr!=-1)))
			topoOnTwoSide++;
		return topoOnTwoSide;
	}else
		return 0;
}

double CP_FlowComplex::JointNormalAngleOfNeighbourPatch(CurveSegment &curve,CP_Patch &patchl,CP_Patch &patchr)
{
	CP_Patch *pPatch=&patchl;
	vector<int> path;
	int start=curve.sp;
	int end=curve.ep;
	path.push_back(end);
	path.push_back(start);
	vector<bool> visitl(pPatch->m_boundary.size(),false);
	bool xcycle=false;
	while (start!=end)
	{int len=path.size();
	for(unsigned int i=0;i<pPatch->m_boundary.size();++i)
	{
		if(pPatch->m_boundary[i]->sp==path.back()&&pPatch->m_boundary[i]->ep!=path[path.size()-2]&&!visitl[i])
		{visitl[i]=true;
		path.push_back(pPatch->m_boundary[i]->ep);start=pPatch->m_boundary[i]->ep;break;
		}else if(pPatch->m_boundary[i]->ep==path.back()&&pPatch->m_boundary[i]->sp!=path[path.size()-2]&&!visitl[i])
		{visitl[i]=true;
		path.push_back(pPatch->m_boundary[i]->sp);start=pPatch->m_boundary[i]->sp;break;
		}
	}//i
	if(len==path.size()){xcycle=true;break;}
	}
	path.erase(path.begin());

	vector<int> pathr;
	start=curve.ep;
	end=curve.sp;
	pathr.push_back(end);
	pathr.push_back(start);
	vector<bool> visitr(patchr.m_boundary.size(),false);
	bool ycycle=false;
	while (start!=end)
	{int len=pathr.size();
	for(unsigned int i=0;i<patchr.m_boundary.size();++i)
	{
		if(patchr.m_boundary[i]->sp==pathr.back()&&patchr.m_boundary[i]->ep!=pathr[pathr.size()-2]&&!visitr[i])
		{visitr[i]=true;
		pathr.push_back(patchr.m_boundary[i]->ep);start=patchr.m_boundary[i]->ep;break;
		}else if(patchr.m_boundary[i]->ep==pathr.back()&&patchr.m_boundary[i]->sp!=pathr[pathr.size()-2]&&!visitr[i])
		{visitr[i]=true;
		pathr.push_back(patchr.m_boundary[i]->sp);start=patchr.m_boundary[i]->sp;break;
		}
	}//i
	if(len==pathr.size()){ycycle=true;break;}
	}
	pathr.erase(pathr.begin());

	int num=path.size();
	path.insert(path.end(),pathr.begin()+1,pathr.end());
	pPatch->path.swap(path);
	double x1=PI,x2=PI;
	if(num<2||pathr.size()<2)
		;
	else{
		ComputeCycleCost(pPatch->path,pPatch->r);

		if(pPatch->r.size()==pPatch->path.size()){//cout<<pPatch->path.size()<<endl;
			x1=acos((pPatch->r.front()*pPatch->r.back())/(pPatch->r.front().GetLength()*pPatch->r.back().GetLength()));
			x2=acos((pPatch->r[num-2]*pPatch->r[num-1])/(pPatch->r[num-2].GetLength()*pPatch->r[num-1].GetLength()));
		}
	}
	return x1+x2;
	//patchl从sp出发路径不包括curve
	vector<int> pl,pr;
	int forwardl=curve.sp;
	pl.push_back(forwardl);
	while(forwardl!=curve.ep)
	{
		for(unsigned int i=0;i<patchl.m_boundary.size();++i)
		{
			//找一端是forward的线，并且整条线不能是curve
			if(forwardl==patchl.m_boundary[i]->sp&&(find(pl.begin(),pl.end(),patchl.m_boundary[i]->ep)==pl.end())&&!patchl.m_boundary[i]->EqualToCurvement(&curve))
			{
				forwardl=patchl.m_boundary[i]->ep;
				break;
			}else if(forwardl==patchl.m_boundary[i]->ep&&(find(pl.begin(),pl.end(),patchl.m_boundary[i]->sp)==pl.end())&&!patchl.m_boundary[i]->EqualToCurvement(&curve))
			{
				forwardl=patchl.m_boundary[i]->sp;
				break;
			}
		}//i

		if(find(pl.begin(),pl.end(),forwardl)==pl.end()){
			pl.push_back(forwardl);
		}
		else
			break;
	}

	//patchr从ep出发，
	int forwardr=curve.ep;
	pr.push_back(forwardr);
	while(forwardr!=curve.sp)
	{
		for(unsigned int i=0;i<patchr.m_boundary.size();++i)
		{
			//找一端是forward的线，并且整条线不能是curve
			if(forwardr==patchr.m_boundary[i]->sp&&(find(pr.begin(),pr.end(),patchr.m_boundary[i]->ep)==pr.end())&&!patchr.m_boundary[i]->EqualToCurvement(&curve))
			{
				forwardr=patchr.m_boundary[i]->ep;
				break;
			}else if(forwardr==patchr.m_boundary[i]->ep&&(find(pr.begin(),pr.end(),patchr.m_boundary[i]->sp)==pr.end())&&!patchr.m_boundary[i]->EqualToCurvement(&curve))
			{
				forwardr=patchr.m_boundary[i]->sp;
				break;
			}
		}//i

		if(find(pr.begin(),pr.end(),forwardr)==pr.end()){
			pr.push_back(forwardr);
		}
		else
			break;
	}

	//分别计算法向总和
	//两个patch法向总和的夹角
	CP_Vector3D normalLeftSum;
	for(int i=1;i<(int)pl.size()-1;++i)
	{
		CP_Vector3D tmp=(m_0cells[pl[i-1]] - m_0cells[pl[i]]) ^ (m_0cells[pl[i+1]] -m_0cells[pl[i]]);
		normalLeftSum+=tmp;
	}

	CP_Vector3D normalRightSum;
	for(int i=1;i<(int)pr.size()-1;++i)
	{
		CP_Vector3D tmp=(m_0cells[pr[i-1]] - m_0cells[pr[i]]) ^ (m_0cells[pr[i+1]] -m_0cells[pr[i]]);
		normalRightSum+=tmp;
	}

	double x= acos((normalLeftSum*normalRightSum)/(normalLeftSum.GetLength()*normalRightSum.GetLength()));
	return x;
}

double CP_FlowComplex::DihedralOfNeighbourPatch(CurveSegment &curve,CP_Patch &patchl,CP_Patch &patchr)
{
	int tril=-1,trir=-1;
	for (unsigned int i=0;i<patchl.m_2cells.size();++i)
	{
		for(unsigned int j=0;j<m_2cells[patchl.m_2cells[i]]->m_triangle.size();++j)
		{
			int triidx=m_2cells[patchl.m_2cells[i]]->m_triangle[j];
			int isedge=0;
			for(int k=0;k<3;++k){
				if(tricells[triidx]->m_points[k]==curve.sp||tricells[triidx]->m_points[k]==curve.ep){
					isedge++;
				}
			}
			if(isedge==2){
				tril=triidx;break;
			}
		}
		if(tril!=-1)break;
	}

	for (unsigned int i=0;i<patchr.m_2cells.size();++i)
	{
		for(unsigned int j=0;j<m_2cells[patchr.m_2cells[i]]->m_triangle.size();++j)
		{
			int triidx=m_2cells[patchr.m_2cells[i]]->m_triangle[j];
			int isedge=0;
			for(int k=0;k<3;++k){
				if(tricells[triidx]->m_points[k]==curve.sp||tricells[triidx]->m_points[k]==curve.ep){
					isedge++;
				}
			}
			if(isedge==2){
				trir=triidx;break;
			}
		}
		if(trir!=-1)break;
	}
	return acos(Dihedral(*tricells[tril],*tricells[trir]));
}

void CP_FlowComplex::MergePatch(CurveSegment &curve,CP_Patch &pl,CP_Patch &pr)
{
	mergededge++;
	pr.merged=pl.index;

	for (unsigned int i=0;i<pr.m_2cells.size();++i)
		m_2cells[pr.m_2cells[i]]->patch=pl.index;
	pl.m_2cells.insert(pl.m_2cells.end(),pr.m_2cells.begin(),pr.m_2cells.end());

	pr.m_boundary.erase(pr.m_boundary.begin()+LocateSegment(pr.m_boundary,curve));
	pl.m_boundary.erase(pl.m_boundary.begin()+LocateSegment(pl.m_boundary,curve));
	pl.m_boundary.insert(pl.m_boundary.end(),pr.m_boundary.begin(),pr.m_boundary.end());

	pr.boundary.erase(pr.boundary.begin()+ExistLineSeg(pr.boundary,curve));
	pl.boundary.erase(pl.boundary.begin()+ExistLineSeg(pl.boundary,curve));
	pl.boundary.insert(pl.boundary.end(),pr.boundary.begin(),pr.boundary.end());

	pl.nonmanifoldedge=pl.nonmanifoldedge+pr.nonmanifoldedge-2;
	if (pl.nonmanifoldedge==0)
		pl.wrong=false;
	pr.wrong=false;
	pr.flag=false;

	/*for(vector<CurveSegment*>::iterator itl=pl.m_boundary.begin();itl!=pl.m_boundary.end();)
	{
	bool dele=false;
	for(vector<CurveSegment*>::iterator itr=itl+1;itr!=pl.m_boundary.end();)
	{
	if((*itr)->EqualToCurvement(*itl)){
	dele=true;
	pl.m_boundary.erase(itr);
	}
	else
	++itr;
	}
	if(dele)
	pl.m_boundary.erase(itl);
	else
	++itl;
	}*/
}

vector<GraphList*> CP_FlowComplex::GetConnectedComponents(GraphList& graphall)
{
	//查找各个连通分量
	vector<GraphList*> forest;
	for(unsigned int j=0;j<graphall.adjList.size();++j)
	{
		if(!graphall.adjList[j].visited&&graphall.adjList[j].firstedge!=NULL)//连通分量删减后无边就无点
		{
			GraphList *tree=new GraphList;//每次新进来新的开始新的分支
			vector<int> ptree;
			vector<int> polytree;
			stack<int> stackp;
			stackp.push(graphall.adjList[j].data);
			while(!stackp.empty())
			{
				int cur=stackp.top();
				stackp.pop();
				vector<VertexNode>::iterator t=find_if(graphall.adjList.begin(),graphall.adjList.end(),bind2nd(compare(),cur));
				if(!t->visited)
				{
					t->visited=true;
					ptree.push_back(t->data);//点记录
					EdgeNode *e=t->firstedge;//此头的边表
					while(e)
					{
						if(!e->visited){
							e->visited=true;
							if(find(polytree.begin(),polytree.end(),e->polyindex)==polytree.end()){
								stackp.push(e->adjvex);
								polytree.push_back(e->polyindex);//边记录
							}
							e=e->next;}
					}
				}
			}
			//记录的点、边构造连通分支
			for (unsigned int k = 0; k < ptree.size(); k++)
			{
				VertexNode v;
				v.data=ptree[k];v.firstedge=NULL;
				tree->adjList.push_back(v);
			}
			for (unsigned int k = 0; k < polytree.size(); ++k)
			{
				EdgeNode* e;
				int start=m_PolyLine[polytree[k]].s;//曲线start，end
				int end=m_PolyLine[polytree[k]].e;
				for(unsigned int m=0;m<tree->adjList.size();++m)
				{
					if(tree->adjList[m].data==start)
					{
						tree->adjList[m].degree++;
						e =new EdgeNode;
						e->adjvex = end;//邻接序号为end
						e->polyindex=polytree[k];
						e->next = tree->adjList[m].firstedge;//将e指针指向当前顶点指向的结构
						tree->adjList[m].firstedge = e;//将当前顶点的指针指向e,新边放到头上
					}
					if(tree->adjList[m].data==end)
					{
						tree->adjList[m].degree++;
						e =new EdgeNode;
						e->adjvex = start;//邻接序号为end
						e->polyindex=polytree[k];
						e->next = tree->adjList[m].firstedge;//将e指针指向当前顶点指向的结构
						tree->adjList[m].firstedge = e;//将当前顶点的指针指向e,新边放到头上
					}
				}//m
			}//k

			forest.push_back(tree);
		}
	}//j
	return forest;
}

void CP_FlowComplex::BuildGraphFromCurves(const vector<int>& poly,GraphList &graphall)
{
	vector<int> p;
	//1.构造图结构
	//顶点
	for (unsigned int j = 0; j < poly.size(); ++j)
	{
		int start=m_PolyLine[poly[j]].s;//曲线start，end
		int end=m_PolyLine[poly[j]].e;

		if(find(p.begin(),p.end(),start)==p.end())
			p.push_back(start);
		if(find(p.begin(),p.end(),end)==p.end())
			p.push_back(end);
	}
	for (unsigned int j = 0; j < p.size(); j++)
	{
		VertexNode v;
		v.data=p[j];v.firstedge=NULL;
		graphall.adjList.push_back(v);
	}

	//边
	for (unsigned int j = 0; j < poly.size(); ++j)
	{
		EdgeNode* e;
		int start=m_PolyLine[poly[j]].s;//曲线start，end
		int end=m_PolyLine[poly[j]].e;
		for(unsigned int k=0;k<graphall.adjList.size();++k)
		{
			if(graphall.adjList[k].data==start)
			{
				graphall.adjList[k].degree++;
				e =new EdgeNode;
				e->adjvex = end;//邻接序号为end
				e->polyindex=poly[j];
				e->next = graphall.adjList[k].firstedge;//将e指针指向当前顶点指向的结构
				graphall.adjList[k].firstedge = e;//将当前顶点的指针指向e,新边放到头上
			}
			if(graphall.adjList[k].data==end)
			{
				graphall.adjList[k].degree++;
				e =new EdgeNode;
				e->adjvex = start;//邻接序号为end
				e->polyindex=poly[j];
				e->next = graphall.adjList[k].firstedge;//将e指针指向当前顶点指向的结构
				graphall.adjList[k].firstedge = e;//将当前顶点的指针指向e,新边放到头上
			}
		}//k
	}//j
}

void CP_FlowComplex::NoDulplicateDarts(const VertexNode& v,vector<int>& poly2delete)
{
	EdgeNode *e=v.firstedge;
	map<int,int>::iterator iter;
	while(e)
	{
		EdgeNode* pre=e->next;
		while(pre)
		{
			//如果dart vertex的pair里有(e->polyindex,pre->polyindex)或(pre->polyindex,e->polyindex)，应删除这两个
			for(auto dart:graph.adjList[v.data].darts)
			{
				if((dart.first==e->polyindex&&dart.second==pre->polyindex)
					||(dart.first==pre->polyindex&&dart.second==e->polyindex))
				{
					if(find(poly2delete.begin(),poly2delete.end(),e->polyindex)==poly2delete.end())
						poly2delete.push_back(e->polyindex);
					if(find(poly2delete.begin(),poly2delete.end(),pre->polyindex)==poly2delete.end())
						poly2delete.push_back(pre->polyindex);
				}
			}
			pre=pre->next;
		}
		e=e->next;
	}
}

void CP_FlowComplex::AddTreeWithoutbranch(vector<GraphList*>& resgraph,GraphList* ptree)
{
	//分量中是不是只有分叉（只从分叉处能找到所有线2*poly.size()==len）
	int len=0;//分量中的线数目的2倍
	for(unsigned int i=0;i<ptree->adjList.size();++i)
	{
		EdgeNode* e=ptree->adjList[i].firstedge;
		while(e)
		{
			len++;
			e=e->next;
		}
	}//i
	vector<int> poly;//只从分叉点能找到的所有线
	vector<int> branch;//分叉点index,不是在0cells中的
	for(unsigned int i=0;i<ptree->adjList.size();++i)
	{
		if(ptree->adjList[i].degree>2)
			branch.push_back(i);
	}
	for(unsigned int i=0;i<branch.size();++i)
	{
		EdgeNode *e=ptree->adjList[branch[i]].firstedge;
		while(e)
		{
			if(find(poly.begin(),poly.end(),e->polyindex)==poly.end()) poly.push_back(e->polyindex);
			e=e->next;
		}
	}

	vector<int> poly2delete;//记录删除的线
	if((int)poly.size()*2==len)
	{//只有分叉
		for(int i=0;i<(int)branch.size()-1;++i)//剩最后一个branch
		{
			EdgeNode* e=ptree->adjList[branch[i]].firstedge;
			while(e)
			{
				if(find(poly2delete.begin(),poly2delete.end(),e->polyindex)==poly2delete.end())
					poly2delete.push_back(e->polyindex);
				e=e->next;
			}
		}//i

		//在原分支上删除,都找完了才能删
		for(unsigned int k=0;k<ptree->adjList.size();++k)
		{
			EdgeNode *e=ptree->adjList[k].firstedge;
			EdgeNode *dummy=new EdgeNode;
			dummy->next=e;
			EdgeNode *pre=dummy;
			while(e)
			{

				if(find(poly2delete.begin(),poly2delete.end(),e->polyindex)!=poly2delete.end())
				{
					pre->next=e->next;ptree->adjList[k].degree--;
				}else
					pre=e;
				e=e->next;
			}
			ptree->adjList[k].firstedge=dummy->next;
		}

		//最后一个点处曲线自由组合
		for(vector<int>::iterator iter=poly.begin();iter!=poly.end();)
		{
			if(find(poly2delete.begin(),poly2delete.end(),*iter)!=poly2delete.end())
				poly.erase(iter);
			else
				iter++;
		}
		for(unsigned int i=0;i<poly.size();++i)
		{
			for(unsigned int j=i+1;j<poly.size();++j)
			{
				GraphList *g=new GraphList;
				vector<int> tmp;
				tmp.push_back(poly[i]);
				tmp.push_back(poly[j]);
				BuildGraphFromCurves(tmp,*g);//g->sb=1;
				resgraph.push_back(g);
			}
			if((int)poly.size()==1)
			{
				GraphList *g=new GraphList;
				vector<int> tmp;
				tmp.push_back(poly[i]);
				BuildGraphFromCurves(tmp,*g);//g->sb=1;
				resgraph.push_back(g);//如果只剩一条线
			}
		}
	}else{
		for(unsigned int i=0;i<branch.size();++i)
		{
			EdgeNode* e=ptree->adjList[branch[i]].firstedge;
			while(e)
			{
				if(find(poly2delete.begin(),poly2delete.end(),e->polyindex)==poly2delete.end())
					poly2delete.push_back(e->polyindex);
				e=e->next;
			}
		}//i

		//在原分支上删除,都找完了才能删
		for(unsigned int k=0;k<ptree->adjList.size();++k)
		{
			EdgeNode *e=ptree->adjList[k].firstedge;
			EdgeNode *dummy=new EdgeNode;
			dummy->next=e;
			EdgeNode *pre=dummy;
			while(e)
			{

				if(find(poly2delete.begin(),poly2delete.end(),e->polyindex)!=poly2delete.end())
				{
					pre->next=e->next;ptree->adjList[k].degree--;
				}else
					pre=e;
				e=e->next;
			}
			ptree->adjList[k].firstedge=dummy->next;
		}
		vector<GraphList*> tmpforest=GetConnectedComponents(*ptree);
		resgraph.insert(resgraph.end(),tmpforest.begin(),tmpforest.end());
	}//else
}

void CP_FlowComplex::FindShortestCycleForComponent(vector<vector<int> > &cycle,const GraphList& ptree,bool degreeEnable)
{
	bool isCycle=true;
	vector<int> ends;
	vector<int> alpoly;//已有曲线
	//判断是否有环
	for(unsigned int i=0;i<ptree.adjList.size();++i)
	{
		if(ptree.adjList[i].degree<2){
			isCycle=false;
			ends.push_back(ptree.adjList[i].data);
		}
	}

	if(isCycle)
	{//如有环找环
		for(unsigned int i=0;i<ptree.adjList.size();++i)
		{
			EdgeNode *e=ptree.adjList[i].firstedge;
			while(e)
			{
				if(find(alpoly.begin(),alpoly.end(),e->polyindex)==alpoly.end())
					alpoly.push_back(e->polyindex);
				e=e->next;
			}
		}
		alpoly.insert(alpoly.begin(),0);
		cycle.push_back(alpoly);
	}else
	{//无环找最短路径
		int head=ends[1];
		while(head!=ends[0])
		{
			bool loop=true;
			for(unsigned int i=0;i<ptree.adjList.size();++i)
			{
				if(ptree.adjList[i].data==head)
				{
					EdgeNode *e=ptree.adjList[i].firstedge;
					while (e)
					{
						if(find(alpoly.begin(),alpoly.end(),e->polyindex)==alpoly.end())
						{
							head=e->adjvex;alpoly.push_back(e->polyindex);break;
						}
						e=e->next;
					}//e
					loop=false;
					break;
				}
			}//i
			if(loop) break;
		}//head

		int start=ends[0];
		int end=ends[1];//结束端点
		queue<vector<int>> paths;//每个vector中，0位置存当前路径的头端点下标，后边是路径

		vector<int> vpath;
		vpath.push_back(start);//加入头
		vpath.insert(vpath.begin()+1,alpoly.begin(),alpoly.end());//加入已有曲线，alpoly应有序：含end[1]poly---含end[0]poly

		paths.push(vpath);
		int last=0;
		bool exist=false;
		while(!paths.empty())
		{
			vector<int> prepath=paths.front();
			paths.pop();
			if(last!=prepath.size()&&exist)//此深度的一层遍历完，且找到了最小环
			{
				return;
			}
			last=prepath.size();
			EdgeNode *e=graph.adjList[prepath[0]].firstedge;//此头的边表
			while(e)
			{//对此端点关联的所有未访问曲线BFS
				vector<int> newpath(prepath);//cout<<newpath.size()<<endl;

				bool accept=true;
				if(degreeEnable&&vec_curve_degree[e->polyindex]>2)
					accept=false;
				//防止死循环
				map<int,int> mp;
				for(unsigned int i=1;i<newpath.size();++i){
					mp[m_PolyLine[newpath[i]].s]++;
					mp[m_PolyLine[newpath[i]].e]++;
					if(mp[m_PolyLine[newpath[i]].s]>2||mp[m_PolyLine[newpath[i]].e]>2)
						accept=false;
				}
				if(newpath.size()==m_PolyLine.size()+1)//不可能有比曲线边数多的环
					accept=false;
				
				if(find(newpath.begin()+1,newpath.end(),e->polyindex)==newpath.end()&&accept){
					int newhead=-1;
					if(newpath[0]==m_PolyLine[e->polyindex].s)//设置新头
						newhead=m_PolyLine[e->polyindex].e;
					else
						newhead=m_PolyLine[e->polyindex].s;
					newpath[0]=newhead;
					newpath.push_back(e->polyindex);
					if(newpath[0]==end)
					{  //记录可返回
						exist=true;
						cycle.push_back(newpath);
					}
					paths.push(newpath);
				}
				e=e->next;
			}//e
		}//path.empty
	}

}

void CP_FlowComplex::ComputeDelaunyPatchForCycles(CP_Patch &patch)
{
	vector<vector<int>> patches;
	for(unsigned int i=0;i<patch.cycle.size();++i){
		
		vector<int> newpatch;
		random_shuffle(patch.cycle[i].begin(),patch.cycle[i].end());
		ConstructFromCycle(patch.cycle[i],newpatch);
		if(newpatch.size()==0){
			random_shuffle(patch.cycle[i].begin(),patch.cycle[i].end());
			ConstructFromCycle(patch.cycle[i],newpatch);
		}
		
		if((newpatch.size()>0)&&IsCycleExistInPatches(patch.cycle[i])==-1){//能完全被曲面化
			AddPatchForCycles(newpatch,patch.cycle[i]);
		}//(newpatch.size()==polygon.size()-2)
	}//i
	//patch.patches.swap(patches);
}

vector<int> CP_FlowComplex::TriangulatingSinglePolygon(const vector<int>& polygon,int sidx,int eidx,CP_Triganle3D *t,vector<vector<int> >& dp,vector<vector<int> >& vt,double &cost)
{
	if(dp[sidx][eidx]!=-1)
		return vt[dp[sidx][eidx]];
	double mincost=MAX_DISTANCE;
	vector<int> res;
	for(unsigned int i=0;i<delauny2cells.size();++i){
		CP_Triganle3D *tri=delauny2cells[i];
		int times=0,third=0;
		for(unsigned int k=0;k<3;++k){
			if((tri->m_points[k]==polygon[sidx])||(tri->m_points[k]==polygon[eidx]))
				times++;
			else
				third=tri->m_points[k];
		}//times-sidx和eidx是三角形的一条边
		if(times==2&&find(polygon.begin()+sidx,polygon.begin()+eidx,third)!=polygon.begin()+eidx){
			vector<int> left;
			vector<int> right;
			double leftcost=0.0,rightcost=0.0;
			if(eidx-sidx==2){
				cost=perweight*tri->area+biweight*Dihedral(*t,*tri);
				res.push_back(i);
				vt.push_back(res);
				dp[sidx][eidx]=vt.size()-1;
				return res;
			}else{
				int mid=distance(polygon.begin(),find(polygon.begin()+sidx,polygon.begin()+eidx,third));
				if(mid-sidx>1){
					left=TriangulatingSinglePolygon(polygon,sidx,mid,tri,dp,vt,leftcost);
				}
				if(eidx-mid>1){
					right=TriangulatingSinglePolygon(polygon,mid,eidx,tri,dp,vt,rightcost);
				}
				double angle=0.0;
				if(t!=NULL)
					angle=Dihedral(*t,*tri);
				double sum=perweight*(tri->area+GetSumTriangleArea(left)+GetSumTriangleArea(right))+biweight*(leftcost+rightcost+angle);
				if(sum<mincost){
					mincost=sum;
					vector<int>().swap(res);
					res.push_back(i);
					res.insert(res.end(),left.begin(),left.end());
					res.insert(res.end(),right.begin(),right.end());
				}
			}
		}
	}
	cost=mincost;
	vt.push_back(res);
	dp[sidx][eidx]=vt.size()-1;
	return res;
}

void CP_FlowComplex::CheckInteriorForPatch(int _patch,const vector<int>& newpatch)
{
	bool selfinterior=true;
	for(unsigned int i=0;i<m_patches[_patch]->m_boundary.size();++i){

		if(m_patches[_patch]->m_boundary[i]->newdegree<2){
			selfinterior=false;
		}
		m_patches[_patch]->m_boundary[i]->newdegree++;
	}//i

	//自己在内部
	if(selfinterior){
		SetPatchFlagFalse(_patch);
		return;
	}

	//是否使别的patch成为内部
	for(unsigned int i=0;i<m_patches.size();++i){
		if(i!=_patch&&m_patches[i]->flag&&!m_patches[i]->wrong){
			bool beinteriored=true;
			for(unsigned int j=0;j<m_patches[i]->m_boundary.size();++j){
				if(m_patches[i]->m_boundary[j]->newdegree<=2)
					beinteriored=false;
			}
			if(beinteriored){
				SetPatchFlagFalse(i);
				for(unsigned int j=0;j<newpatch.size();++j){
					if(delaunyexist.count(newpatch[j])>0&&m_patches[delaunyexist[newpatch[j]]]->flag){
						SetPatchFlagFalse(_patch);
						return;
					}
				}
				return;
			}
		}
	}//i

	//delauny三角面唯一存在于一个patch
	for(unsigned int j=0;j<newpatch.size();++j){
		if(delaunyexist.count(newpatch[j])>0&&m_patches[delaunyexist[newpatch[j]]]->flag){
			SetPatchFlagFalse(_patch);
			return;
		}
	}
}

vector<int> CP_FlowComplex::ElimateInteriorPatch(vector<int>& group)
{
	int quality_max=0;
	vector<int> vquality;

	for(unsigned int i=0;i<group.size();++i){
		int depth=0;
		vector<int> tmp;
		for(unsigned int k=0;k<m_patches[group[i]]->m_boundary.size();++k)
			m_patches[group[i]]->m_boundary[k]->newdegree--;
		tmp.push_back(group[i]);
		ElimateRecursive(group,vquality,tmp,quality_max,1);
		tmp.pop_back();
		for(unsigned int k=0;k<m_patches[group[i]]->m_boundary.size();++k)
			m_patches[group[i]]->m_boundary[k]->newdegree++;
	}
	return vquality;
}

void CP_FlowComplex::ElimateRecursive(vector<int>& group,vector<int>& res,vector<int>& tmp,int &quality_max,int quality)
{
	bool stoprecur=true;
	for(unsigned int i=0;i<group.size();++i){
		//先确定还在内部
		bool selfinterior=true;
		for(unsigned int j=0;j<m_patches[group[i]]->m_boundary.size();j++){
			if(m_patches[group[i]]->m_boundary[j]->newdegree<=2){
				selfinterior=false;break;
			}
		}
		if(selfinterior&&find(tmp.begin(),tmp.end(),group[i])==tmp.end()){
			stoprecur=false;
			for(unsigned int k=0;k<m_patches[group[i]]->m_boundary.size();++k)
				m_patches[group[i]]->m_boundary[k]->newdegree--;
			tmp.push_back(group[i]);
			ElimateRecursive(group,res,tmp,quality_max,quality+1);
			tmp.pop_back();
			for(unsigned int k=0;k<m_patches[group[i]]->m_boundary.size();++k)
				m_patches[group[i]]->m_boundary[k]->newdegree++;
		}
	}
	if(stoprecur&&quality>quality_max){
		quality_max=quality;
		res.swap(tmp);
	}
}

void CP_FlowComplex::ProcessingInteriorPatch()
{
	curve_incident_patch.resize(m_PolyLine.size());
	for(unsigned int i=0;i<m_patches.size();++i){
		if(m_patches[i]->flag){
			for(unsigned int j=0;j<m_patches[i]->m_bcurve.size();++j){
				int curve=m_patches[i]->m_bcurve[j];
				curve_incident_patch[curve].push_back(i);
			}
		}
	}
	vector<bool> visitedpatch(m_patches.size(),false);
	vector<vector<int> > group;
	vector<int> interiortmp;
	for(unsigned int i=0;i<m_patches.size();++i){
		if(m_patches[i]->flag){
			bool selfinterior=true;
			for(unsigned int j=0;j<m_patches[i]->m_boundary.size();j++){
				if(m_patches[i]->m_boundary[j]->newdegree<=2){
					selfinterior=false;break;
				}
			}
			if(selfinterior){
				interiortmp.push_back(i);
			}
		}
	}

	for(unsigned int i=0;i<interiortmp.size();++i){
		int _patch=interiortmp[i];
		if(m_patches[_patch]->flag&&!visitedpatch[_patch]){
			//一个新的group
			vector<int> gp;
			queue<int> q;//度为>2的边
			for(unsigned int j=0;j<m_patches[_patch]->m_bcurve.size();++j)
				q.push(m_patches[_patch]->m_bcurve[j]);
			gp.push_back(_patch);
			visitedpatch[_patch]=true;
			while(!q.empty()){
				int curve=q.front();q.pop();
				for(unsigned int j=0;j<curve_incident_patch[curve].size();++j){
					int newpatch=curve_incident_patch[curve][j];
					if(find(interiortmp.begin(),interiortmp.end(),newpatch)!=interiortmp.end()
						&&!visitedpatch[newpatch]){
							gp.push_back(newpatch);
							visitedpatch[newpatch]=true;
							for(unsigned int k=0;k<m_patches[newpatch]->m_bcurve.size();++k){
								if(curve!=m_patches[newpatch]->m_bcurve[k])
									q.push(m_patches[newpatch]->m_bcurve[k]);}
					}
				}//j
			}//while
			group.push_back(gp);
		}
	}
	//TODO:如果有多组解
	for(auto gp:group){
		for(auto e:gp){
			interior_patches.push_back(e);
		}
		vector<int> elimate_patch=ElimateInteriorPatch(gp);
		for(auto _patch:elimate_patch){
			SetPatchFlagFalse(_patch);
		}
	}
}

void CP_FlowComplex::ProcessingOverlappingPatch()
{
	for(unsigned int i=0;i<overlap_by_delauny.size();++i){
		if(overlap_by_delauny[i].size()>=2){
			int minlen=INT_MAX;
			int minlenidx=-1;
			for(unsigned int j=0;j<overlap_by_delauny[i].size();++j){
				if((int)m_patches[overlap_by_delauny[i][j]]->m_bcurve.size()<minlen){
					minlen=m_patches[overlap_by_delauny[i][j]]->m_bcurve.size();
					minlenidx=overlap_by_delauny[i][j];
				}
				else if((int)m_patches[overlap_by_delauny[i][j]]->m_bcurve.size()==minlen){
					if(GetCyclelength(m_patches[overlap_by_delauny[i][j]]->m_bcurve)<GetCyclelength(m_patches[minlenidx]->m_bcurve)){
						minlen=m_patches[overlap_by_delauny[i][j]]->m_bcurve.size();
						minlenidx=overlap_by_delauny[i][j];
					}
				}
			}
		//	cout<<minlenidx<<"###";
			for(unsigned int j=0;j<overlap_by_delauny[i].size();++j){
				if(overlap_by_delauny[i][j]!=minlenidx&&m_patches[overlap_by_delauny[i][j]]->flag&&m_patches[minlenidx]->flag){
					if(/*overlap_by_delauny[i][j]<oripatches&&minlenidx<oripatches*/GetTriangleArea(*delauny2cells[i])<1e-5){
						//cout<<overlap_by_delauny[i][j]<<"queshiyou"<<endl;minpatch.push_back(i);
						//cout<<LocateTriangle(tricells,*delauny2cells[i]);
						//cout<<GetTriangleArea(*delauny2cells[i]);
						continue;
					}
					
					//重复边
					vector<int> dulpcurve;
					map<int,int> tmpmin;
					for(unsigned int k=0;k<m_patches[minlenidx]->m_bcurve.size();++k)
						tmpmin[m_patches[minlenidx]->m_bcurve[k]]++;

					for(unsigned int k=0;k<m_patches[overlap_by_delauny[i][j]]->m_bcurve.size();++k){
						if(tmpmin.count(m_patches[overlap_by_delauny[i][j]]->m_bcurve[k])>0)
							dulpcurve.push_back(m_patches[overlap_by_delauny[i][j]]->m_bcurve[k]);
					}
					//新环边=（其他环-重复边）+（最小环-重复边）
					tmpmin.clear();
					for(unsigned int k=0;k<dulpcurve.size();++k)
						tmpmin[dulpcurve[k]]++;
					
					vector<int> newcycle;
					for(unsigned int k=0;k<m_patches[minlenidx]->m_bcurve.size();++k)
						if(tmpmin.count(m_patches[minlenidx]->m_bcurve[k])==0)
							newcycle.push_back(m_patches[minlenidx]->m_bcurve[k]);
					for(unsigned int k=0;k<m_patches[overlap_by_delauny[i][j]]->m_bcurve.size();++k){
						if(tmpmin.count(m_patches[overlap_by_delauny[i][j]]->m_bcurve[k])==0)
							newcycle.push_back(m_patches[overlap_by_delauny[i][j]]->m_bcurve[k]);
					}
					int patch_if_exist=IsCycleExistInPatches(newcycle);
					if(patch_if_exist==-1){
						//是否存在或存在过flag false
						vector<int> newpatch;
						if(newcycle.size()>0){
							ConstructFromCycle(newcycle,newpatch);
							//新生成与minlenidx不重叠
							map<int,int> mp;
							for(unsigned int jj=0;jj<m_patches[minlenidx]->m_2cells.size();++jj){
								int _2cell=m_patches[minlenidx]->m_2cells[jj];
								for(unsigned int k=0;k<m_2cells[_2cell]->delaunytri.size();++k){
									mp[m_2cells[_2cell]->delaunytri[k]]++;
								}
							}//jj
							for(auto e:newpatch){
								if(mp.count(e)>0)
									vector<int>().swap(newpatch);
							}
						}
						//if(newpatch.size()==0){
						//	random_shuffle(newcycle.begin(),newcycle.end());
						//	ConstructFromCycle(newcycle,newpatch);
						//	//新生成与minlenidx不重叠
						//	map<int,int> mp;
						//	for(unsigned int jj=0;jj<m_patches[minlenidx]->m_2cells.size();++jj){
						//		int _2cell=m_patches[minlenidx]->m_2cells[jj];
						//		for(unsigned int k=0;k<m_2cells[_2cell]->delaunytri.size();++k){
						//			mp[m_2cells[_2cell]->delaunytri[k]]++;
						//		}
						//	}//jj
						//	for(auto e:newpatch){
						//		if(mp.count(e)>0)
						//			vector<int>().swap(newpatch);
						//	}
						//}
						if(newpatch.size()>0){
							AddPatchForCycles(newpatch,newcycle);//cout<<"生成"<<m_patches.size();
						}
					}else{
						if(!m_patches[patch_if_exist]->flag){
							SetPatchFlagTrue(patch_if_exist);//cout<<"有效"<<patch_if_exist;
						}
						//bool flag=true;
						//
						//map<int,int> mp;
						//for(unsigned int jj=0;jj<m_patches[minlenidx]->m_2cells.size();++jj){
						//	int _2cell=m_patches[minlenidx]->m_2cells[jj];
						//	for(unsigned int k=0;k<m_2cells[_2cell]->delaunytri.size();++k){
						//		mp[m_2cells[_2cell]->delaunytri[k]]++;
						//	}
						//}//jj
						//for(unsigned int jj=0;jj<m_patches[patch_if_exist]->m_2cells.size();++jj){
						//	int _2cell=m_patches[patch_if_exist]->m_2cells[jj];
						//	for(unsigned int k=0;k<m_2cells[_2cell]->delaunytri.size();++k){
						//		if(mp.count(m_2cells[_2cell]->delaunytri[k])>0){m_patches[patch_if_exist]->flag==false;break;}
						//	}
						//}//jj
						
						//cout<<patch_if_exist<<"patch_if_exist"<<endl;
					}//cout<<GetTriangleArea(*delauny2cells[i]);
					SetPatchFlagFalse(overlap_by_delauny[i][j]);
					//cout<<overlap_by_delauny[i][j]<<",";
				}//flag
			}//j
			//cout<<endl;
		}//overlap_by_delauny[i].size()==2
		
	}//i
}

double CP_FlowComplex::GetCyclelength(const vector<int>& cycle)
{
	double len=0.0;
	for(unsigned int i=0;i<cycle.size();++i){
		len+=m_PolyLine[cycle[i]].GetLength();
	}
	return len;
}

void CP_FlowComplex::ConstructFromCycle(const vector<int> &cycle,vector<int> &newpatch)
{
	//cycle 得到polygon
	vector<int> polygon;
	vector<bool> visited(cycle.size(),false);
	int cur=m_PolyLine[cycle[0]].s;
	polygon.push_back(cur);
	for(unsigned int j=0;j<cycle.size();++j){//次数
		for(unsigned int k=0;k<cycle.size();++k){//找一头是cur的线
			if(!visited[k]&&(cur==m_PolyLine[cycle[k]].s)){
				int curve=cycle[k];
				for(unsigned int m=1;m<m_PolyLine[curve].idx.size();++m){
					polygon.push_back(m_PolyLine[curve].idx[m]);
				}
				cur=m_PolyLine[curve].idx.back();
				visited[k]=true;
			}else if(!visited[k]&&(cur==m_PolyLine[cycle[k]].e)){
				int curve=cycle[k];
				for(int m=m_PolyLine[curve].idx.size()-2;m>=0;--m){
					polygon.push_back(m_PolyLine[curve].idx[m]);
				}
				cur=m_PolyLine[curve].idx.front();
				visited[k]=true;
			}
		}
	}
	polygon.pop_back();
	//polygon三角化,newpatch里为triangle的编号
	int sizen=polygon.size();
	int times=0;
	while((newpatch.size()!=polygon.size()-2)&&times<sizen){
		if(times>0){
			polygon.push_back(polygon.front());
			polygon.erase(polygon.begin());
		}
		double cost=0.0;
		vector<vector<int> > dp(sizen,vector<int>(sizen,-1));
		vector<vector<int> > vt;
		newpatch=TriangulatingSinglePolygon(polygon,0,polygon.size()-1,NULL,dp,vt,cost);
		times++;
	}
	if(newpatch.size()!=polygon.size()-2)//能完全被曲面化,newpatch置为空
		vector<int>().swap(newpatch);
}

int CP_FlowComplex::IsCycleExistInPatches(const vector<int>& cycle)
{
	int idx=-1;
	for(unsigned int i=0;i<m_patches.size();++i){
		if(m_patches[i]->flag&&!m_patches[i]->wrong){
			//与已有环不会完全重复
			if(m_patches[i]->m_bcurve.size()==cycle.size()){//cycle多出一个head
				bool same=true;
				for(unsigned int k=0;k<cycle.size();++k){
					if(find(m_patches[i]->m_bcurve.begin(),m_patches[i]->m_bcurve.end(),cycle[k])==m_patches[i]->m_bcurve.end()){
						same=false;break;}
				}//k
				if(same){
					idx=i;
					//break;
				}
			}//len
		}
	}
	return idx;
}

void CP_FlowComplex::AddPatchForCycles(const vector<int> &newpatch,const vector<int>& cycle)
{
	unsigned int ori=tricells.size();
	CP_Patch *pPatch=new CP_Patch();
	m_patches.push_back(pPatch);
	int _patch=m_patches.size()-1;
	pPatch->index=_patch;
	pPatch->merged=1;
	pPatch->m_bcurve=cycle;
	for(auto curve:cycle)
		vec_curve_degree[curve]++;

	for(unsigned int j=0;j<newpatch.size();++j){
		CP_Triganle3D *pTriangle =new CP_Triganle3D(delauny2cells[newpatch[j]]->m_points[0], delauny2cells[newpatch[j]]->m_points[1], delauny2cells[newpatch[j]]->m_points[2]);
		tricells.push_back(pTriangle);
		CP_2cell *p2cell=new CP_2cell();
		m_2cells.push_back(p2cell);
		int _2cell=m_2cells.size()-1;
		p2cell->index=_2cell;
		p2cell->patch=_patch;
		pPatch->m_2cells.push_back(_2cell);
		pTriangle->_2cell=_2cell;
		p2cell->m_triangle.push_back(tricells.size()-1);
		p2cell->delaunytri.push_back(newpatch[j]);

		vector<CurveSegment* > lvec;
		for(int k=0;k<3;k++)
		{
			CurveSegment* l=new CurveSegment(pTriangle->m_points[k%3],pTriangle->m_points[(k+1)%3]);
			l->_triangle=tricells.size()-1;
			lvec.push_back(l);
		}//k
		for(unsigned int j=0;j<lvec.size();j++)
			p2cell->m_boundary.push_back(*lvec[j]);
	}
	//设置相邻三角形
	for(unsigned int j=ori;j<tricells.size();j++)
	{
		if(m_2cells[Locate2cell(tricells[j]->_2cell)]->flag)
			for(unsigned int k=0;k<3;k++)
				m_0cells[tricells[j]->m_points[k]].m_adjTriangle.push_back(j);
	}
	
	for(unsigned int j=ori;j<tricells.size();j++)
	{
		if(m_2cells[Locate2cell(tricells[j]->_2cell)]->flag)
			for(unsigned int k=0;k<3;k++)
			{
				vector<int> v=GetIncidentTri(m_0cells[tricells[j]->m_points[k%3]],m_0cells[tricells[j]->m_points[(k+1)%3]]);
				for(unsigned int m=0;m<v.size();m++)
				{
					if(j!=v[m]){
						tricells[j]->m_adjTriangle.push_back(v[m]);
						tricells[v[m]]->m_adjTriangle.push_back(j);
					}
				}
			}
	}
	GetPatchBoundary(_patch);
	
	for(unsigned int i=0;i<m_patches[_patch]->m_boundary.size();++i){
		m_patches[_patch]->m_boundary[i]->newdegree++;
		///m_patches[_patch]->m_boundary[i]->tmpdegree++;
	}//i
	
	//CheckInteriorForPatch(_patch,newpatch);
	//记录delauny三角面占用情况
	//cout<<newpatch.size()<<endl;
	if(m_patches[_patch]->flag){
		for(unsigned int j=0;j<newpatch.size();++j){
			delaunyexist[newpatch[j]]=_patch;//cout<<j<<endl;
			overlap_by_delauny[newpatch[j]].push_back(_patch);///cout<<j<<endl;
		}
	}

}

void CP_FlowComplex::SetPatchFlagFalse(int i)
{
	m_patches[i]->flag=false;
	for(unsigned int j=0;j<m_patches[i]->m_boundary.size();++j)
		m_patches[i]->m_boundary[j]->newdegree--;
	for(auto curve:m_patches[i]->m_bcurve)
		vec_curve_degree[curve]--;
}

void CP_FlowComplex::SetPatchFlagTrue(int i)
{
	m_patches[i]->flag=true;
	for(unsigned int j=0;j<m_patches[i]->m_boundary.size();++j)
		m_patches[i]->m_boundary[j]->newdegree++;
}

double CP_FlowComplex::GetTriangleArea(const CP_Triganle3D &tri)
{
	double d12 = dist(m_0cells[tri.m_points[0]], m_0cells[tri.m_points[1]]);
	double d13 = dist(m_0cells[tri.m_points[0]], m_0cells[tri.m_points[2]]);
	double d23 = dist(m_0cells[tri.m_points[1]], m_0cells[tri.m_points[2]]);
	//求三角形及子三角形的面积
	double areaD = Area(d12, d13, d23);  //三角形面积
	return areaD;
}

void CP_FlowComplex::GenerateCycle(int polyidx)
{
	int start=cycles.size();
	vector<int> poly(1,polyidx);
	GraphList  graphall;
	BuildGraphFromCurves(poly,graphall);
	vector<GraphList*> newforest;
	newforest=GetConnectedComponents(graphall);
	
	for(unsigned int j=0;j<newforest.size();++j)
	{
		FindShortestCycleForComponent(cycles,*newforest[j],true);
	}
	
	for(unsigned int j=start;j<cycles.size();++j){
		cycles[j].erase(cycles[j].begin());
	}
	
	for(unsigned int i=start;i<cycles.size();++i){
		if(IsCycleExistInPatches(cycles[i])==-1){
			vector<int> newpatch;
			random_shuffle(cycles[i].begin(),cycles[i].end());
			ConstructFromCycle(cycles[i],newpatch);
			if(newpatch.size()>0){
				AddPatchForCycles(newpatch,cycles[i]);}
		}
	}
}

void CP_FlowComplex::NonmanifoldCurves()
{
	vector<int> vec_curve_degree(m_PolyLine.size(),0);
	for(unsigned int i=0;i<m_patches.size();++i){
		if(m_patches[i]->flag){
			for(unsigned int j=0;j<m_patches[i]->m_bcurve.size();++j){
				int curve=m_patches[i]->m_bcurve[j];
				vec_curve_degree[curve]++;
			}
		}
	}//i
	vector<int> vec_degree0;
	vector<int> vec_degree1;
	vector<int> vec_non_manifold;
	for(unsigned int i=0;i<vec_curve_degree.size();++i){
		if(vec_curve_degree[i]==0)
			vec_degree0.push_back(i);
		else if(vec_curve_degree[i]==1)
			vec_degree1.push_back(i);
		else if(vec_curve_degree[i]>2)
			vec_non_manifold.push_back(i);
	}
	cout<<"remain non-manifolds："<<vec_degree1.size()+vec_non_manifold.size()<<endl;
}

double CP_FlowComplex::GetSumTriangleArea(const vector<int> &vt)
{
	double sum=0.0;
	for(unsigned int i=0;i<vt.size();++i){
		sum+=delauny2cells[vt[i]]->area;
	}
	return sum;
}

void CP_FlowComplex::TopologyComplete()
{
	vector<int> patch12;
	vector<int> patch23;
	queue<int> patch123;
	queue<vector<int> > degree1ToFindCycles;
	for(unsigned int i=0;i<m_patches.size();++i){
		if(m_patches[i]->flag){
			map<int,int> mp;//x-曲线，y-曲线度数
			bool degree_1=false;
			bool degree_2=false;
			bool degree_above2=false;
			for(unsigned int j=0;j<m_patches[i]->m_bcurve.size();++j){
				int curve=m_patches[i]->m_bcurve[j];
				mp[curve]=vec_curve_degree[curve];
				if(vec_curve_degree[curve]==1)
					degree_1=true;
				else if(vec_curve_degree[curve]==2)
					degree_2=true;
				else if(vec_curve_degree[curve]>2)
					degree_above2=true;
			}
			if(degree_1&&degree_2&&degree_above2){
				patch123.push(i);
				vector<int> tmp;
				for(auto e:mp){
					if(e.second==1){
						tmp.push_back(e.first);
					}
				}
				degree1ToFindCycles.push(tmp);
			}else if(degree_1&&!degree_2&&degree_above2){
				map<int,int> vp;
				for(auto e:mp){
					if(e.second==1){
						vp[m_PolyLine[e.first].s]++;
						vp[m_PolyLine[e.first].e]++;
					}
				}
				int once=0;
				for(auto v:vp){
					if(v.second==1)
						once++;
				}
				if(once!=2){
					patch123.push(i);
					vector<int> tmp;
					for(auto e:mp){
						if(e.second==1){
							tmp.push_back(e.first);
						}
					}
					degree1ToFindCycles.push(tmp);
				}
			}else if(degree_1&&degree_2&&!degree_above2){
				patch12.push_back(i);
			}
		}//flag
	}
	while(!patch123.empty()){//cout<<degree1ToFindCycles.size()<<","<<patch123.size()<<endl;
		int _patch=patch123.front();
		vector<int> degree1curve=degree1ToFindCycles.front();
		patch123.pop();
		degree1ToFindCycles.pop();
		for(auto e:degree1curve){
			int start=cycles.size();
			vector<int> poly(1,e);
			GraphList  graphall;
			BuildGraphFromCurves(poly,graphall);
			vector<GraphList*> newforest;
			newforest=GetConnectedComponents(graphall);

			for(unsigned int j=0;j<newforest.size();++j)
			{
				FindShortestCycleForComponent(cycles,*newforest[j],true);
			}
			
			for(unsigned int j=start;j<cycles.size();++j){
				cycles[j].erase(cycles[j].begin());
			}

			for(unsigned int i=start;i<cycles.size();++i){
				if(IsCycleExistInPatches(cycles[i])==-1){
					vector<int> newpatch;
					random_shuffle(cycles[i].begin(),cycles[i].end());
					ConstructFromCycle(cycles[i],newpatch);
					if(newpatch.size()>0){
						AddPatchForCycles(newpatch,cycles[i]);
						for(auto p:patch12){
							bool degree_1=false;
							bool degree_2=false;
							bool degree_above2=false;
							vector<int> tmp;
							for(unsigned int j=0;j<m_patches[i]->m_bcurve.size();++j){
								int curve=m_patches[i]->m_bcurve[j];
								if(vec_curve_degree[curve]==1){
									degree_1=true;
									tmp.push_back(vec_curve_degree[curve]);
								}
								else if(vec_curve_degree[curve]==2)
									degree_2=true;
								else if(vec_curve_degree[curve]>2)
									degree_above2=true;
							}
							if(degree_1&&degree_2&&degree_above2){
								patch123.push(p);
								degree1ToFindCycles.push(tmp);
							}
						}
					}
				}
			}//i
		}//degree1curve
	}//while
}

void CurveSegment::ResetDegreee()
{
	tmpdegree=degree;
}

CurveSegment::CurveSegment(int lp,int rp)
{	
	sp=lp;
	ep=rp;
	degree=0;
	tmpdegree=0;
	newdegree=0;
	isBoundary=-1;
	_triangle=-1;
}


int CurveSegment::GetPointIndex(int i) const
{
	if(i==0)
		return sp;
	else
		return ep;
}

bool CurveSegment::EqualToCurvement(CurveSegment* c)
{
	if ((sp==c->sp&&ep==c->ep)||(sp==c->ep&&ep==c->sp))
		return true;
	else
		return false;
}

CP_2cell::CP_2cell(void)
{
	index=-1;
	visited=false;
	distance=0.0;
	flag=true;
	type=0;
	patch=-1;
}


CP_2cell::~CP_2cell(void)
{
}



CP_Patch::CP_Patch(void)
{
	visited=false;
	index=-1;
	merged=-1;
	color=-1;
	flag=true;
	wrong=false;
	nonmanifoldedge=0;
	pairedp=-1;
	dihedral=0;
}


CP_Patch::~CP_Patch(void)
{
}


CP_3cell::CP_3cell(void)
{
	dis3cell=0.0;flag=false;
}


CP_3cell::~CP_3cell(void)
{
}


CircumPoint::CircumPoint(double newx, double newy, double newz,double newvol):CP_Point3D(newx,newy,newz),vol(newvol)
{
	flag=false;
}

void CircumPoint::SetMember(double newx, double newy, double newz, double newvol)
{
	m_x=newx;
	m_y=newy;
	m_z=newz;
	vol=newvol;
}

CircumPoint::~CircumPoint(void)
{
}

CircumPoint& CircumPoint::operator=(const CircumPoint& tmp)
{
	m_x=tmp.m_x;
	m_y=tmp.m_y;
	m_z=tmp.m_z;
	vol=tmp.vol;
	flag=tmp.flag;
	distance=tmp.distance;
	return *this;
}

void CircumPoint::SetDistance(double d)
{
	distance=d;
}

GraphList::~GraphList()
{
	EdgeNode *p,*q;
	for(unsigned int i=0;i<adjList.size();++i)
	{
		p=adjList[i].firstedge;
		while (p)
		{
			q=p;
			p=p->next;
			delete q;
		}
	}
}

GraphList::GraphList()
{
	//sb=0;
}

void GraphList::Reset()
{
	EdgeNode *p,*q;
	for(unsigned int i=0;i<adjList.size();++i)
	{
		p=adjList[i].firstedge;
		while (p)
		{
			q=p;
			p=p->next;
			delete q;
		}
	}
	vector<VertexNode>().swap(adjList);
}
