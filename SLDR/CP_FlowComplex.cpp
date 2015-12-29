#include "stdafx.h"
#include "CP_FlowComplex.h"
#include <iostream>
//using namespace std;

CP_Tetrahedron::CP_Tetrahedron(int i,int j,int k,int l)
{
	m_points[0] = i;
	m_points[1] = j;
	m_points[2] = k;
	m_points[3] = l;
}

CP_Tetrahedron::~CP_Tetrahedron()
{


}

CP_Triganle3D::CP_Triganle3D(int p0, int p1, int p2)
{
	m_points[0] = p0;
	m_points[1] = p1;
	m_points[2] = p2;

	normalsetted=false;
//	_2cell=-1;
}

CP_Triganle3D::~CP_Triganle3D(void)
{
}

CP_FlowComplex::CP_FlowComplex()
{
	desN=0;
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
	for(unsigned int i=0;i<ctri.size();i++)
		delete ctri[i];
	for(unsigned int i=0;i<m_3cells.size();i++)
		delete m_3cells[i];
	vector<CP_3cell*>().swap(m_3cells);
}

bool CP_FlowComplex::IsSmallAngle(CP_Point3D &po,CP_Point3D &pa,CP_Point3D &pb)
{
	CP_Vector3D oa=pa-po;
	CP_Vector3D ob=pb-po;
	if(oa*ob/(oa.GetLength()*ob.GetLength())>PI/6.0)
		return true;
	else
		return false;
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
						m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j+2,equalDisPoint(k,m_PolyLine[i].m_points[j+1],m_PolyLine[i].m_points[j+2]));
						j=j+2;
					}else if(k>d)
					{
						m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j+1,CP_Point3D((m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2.0,(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2.0,(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2.0));
						m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j,equalDisPoint(d,m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j-1]));
						j=j+2;
					}
				}//small angle
			}//j
		}//>2
	}//i

	//两线间小角度冲突问题
	//在交汇点冲突时应该以此为joint点等距细分，这样后边不会再需要投影了
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
						if(!ExistPoint(vjoint,m_PolyLine[m].m_points[0]))
							vjoint.push_back(m_PolyLine[m].m_points[0]);
					}//while Gabriel

					if(!IsGabriel(midPoint,m_PolyLine[m].m_points[m_PolyLine[m].m_points.size()-2],radius_pq))
					{
						if(!ExistPoint(vjoint,m_PolyLine[m].m_points[m_PolyLine[m].m_points.size()-1]))
							vjoint.push_back(m_PolyLine[m].m_points[m_PolyLine[m].m_points.size()-1]);
					}//while Gabriel
				}//i==m else
			}//for m
			}//j==0||m_point.size-2
		}//j
	}//i

	for(unsigned int i=0;i<vjoint.size();i++)
		subdivideSegsJointV(vjoint[i]);

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
						reverseForProjection(m_PolyLine[i].m_points[j+1]);
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
							reverseForProjection(m_PolyLine[i].m_points[j+1]);
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

void CP_FlowComplex::reverseForProjection(CP_Point3D & p)
{
	for(unsigned int i=0;i<m_PolyLine.size();i++)
	{
		bool done=false;
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
				reverseForProjection(m_PolyLine[i].m_points[j]);
				done=true;
				break;
			}
		}//j
		if(done)
			break;
	}//i
}

bool CP_FlowComplex::IsGabriel(CP_Point3D &c,CP_Point3D &r,double radius_pq)
{//r是否在以c为圆心以radius为半径的球中,在返回false，不在true
	double dis=dist(c,r)-radius_pq;
	if(fabs(dis)>TOL&&dis<0)
		return false;
	else
		return true;
}

CP_Point3D CP_FlowComplex::ProjectionPoint(CP_Point3D &p1,CP_Point3D &p2,CP_Point3D &p3)
{
	double k=((p3.m_x-p1.m_x)*(p2.m_x-p1.m_x)+(p3.m_y-p1.m_y)*(p2.m_y-p1.m_y)+(p3.m_z-p1.m_z)*(p2.m_z-p1.m_z))/((p2.m_x-p1.m_x)*(p2.m_x-p1.m_x)+(p2.m_y-p1.m_y)*(p2.m_y-p1.m_y)+(p2.m_z-p1.m_z)*(p2.m_z-p1.m_z));
	return CP_Point3D(k*(p2.m_x-p1.m_x)+p1.m_x,k*(p2.m_y-p1.m_y)+p1.m_y,k*(p2.m_z-p1.m_z)+p1.m_z);
}

void CP_FlowComplex::subdivideSegsJointV(CP_Point3D & vp)
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
	for(it=polymap.begin();it!=polymap.end();it++)
	{
		if(it->second==0)
			m_PolyLine[it->first].m_points.insert(m_PolyLine[it->first].m_points.begin()+1,equalDisPoint(dmin,m_PolyLine[it->first].m_points[0],m_PolyLine[it->first].m_points[1]));
		else
			m_PolyLine[it->first].m_points.insert(m_PolyLine[it->first].m_points.begin()+it->second,equalDisPoint(dmin,m_PolyLine[it->first].m_points[it->second],m_PolyLine[it->first].m_points[it->second-1]));
	}
}

//O---->A----->B
CP_Point3D CP_FlowComplex::equalDisPoint(double k,CP_Point3D &po,CP_Point3D &pb)
{
	double d=dist(po,pb);
	double x=k*(pb.m_x-po.m_x)/d+po.m_x;
	double y=k*(pb.m_y-po.m_y)/d+po.m_y;
	double z=k*(pb.m_z-po.m_z)/d+po.m_z;
	return CP_Point3D(x,y,z);
}

bool CP_FlowComplex::ExistPoint(vector<CP_Point3D> &v,CP_Point3D& p)
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

bool CP_FlowComplex::noCover(CP_Triganle3D &ltri,CP_Triganle3D &rtri)
{
	int abc = 0;
	int sp[2]={-1,-1};//公共边顶点的编号
	int lsr=-1,rsr=-1;//ltri 和rtri的源点（计算法向）
	for (unsigned int j = 0; j < 3; j++)
	{
		for (int k = 0; k < 3; k++)
		{
			if (ltri.m_points[j]==rtri.m_points[k])
				{sp[abc++]=ltri.m_points[j];//按公共边在ltr中顺序存
			}
		}
	}

	if (abc == 2)
	{//有公共边
		for (unsigned int j=0;j<3;j++)
		{
			int tmp=-1;
			for(unsigned int k=0;k<2;k++)
			{
				if(ltri.m_points[j]==sp[k])
					tmp=j;
			}
			if(tmp==-1)
				lsr=ltri.m_points[j];
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
				rsr=rtri.m_points[j];
		}
		CP_Vector3D nl=(m_0cells[sp[0]] - m_0cells[lsr]) ^ 
			(m_0cells[sp[1]] - m_0cells[lsr]);
		//r 0-1-2
		CP_Vector3D nr=(m_0cells[sp[0]] - m_0cells[rsr]) ^ 
			(m_0cells[sp[1]] - m_0cells[rsr]);
		
		if((nl*nr)/(nl.GetLength()*nr.GetLength())>0.93)
			return false;//二面角很小，接近覆盖
			
	}//abc==2
	return true;
}

int CP_FlowComplex::LocateSegment(vector<CurveSegment*> &curveVec,CurveSegment &line)
{

	for (int k = 0; k < curveVec.size(); k++)
	{
		if ((curveVec[k]->sp==line.sp&&curveVec[k]->ep==line.ep)||(curveVec[k]->sp==line.ep&&curveVec[k]->ep==line.sp))
		{
				return k;
		}
	}//k
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
		for(unsigned int j=0;j<m_patches[i]->m_boundary.size();j++)
		{
			CP_Triganle3D* ptri=tricells[m_patches[i]->m_boundary[j]._triangle];//本2cell中与此边界相关的triganle
			for(unsigned int k=0;k<ptri->m_adjTriangle.size();k++)
			{
				CP_2cell* p2celladj=m_2cells[Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)];
				if(p2celladj->patch!=-1&&p2celladj->patch!=m_patches[i]->index){
					vector<int>::iterator it=find(m_patches[i]->m_adjPatch.begin(),m_patches[i]->m_adjPatch.end(),p2celladj->patch);
					if(it==m_patches[i]->m_adjPatch.end())
						m_patches[i]->m_adjPatch.push_back(p2celladj->patch);
				}
			}//k
		}//j
	}//i
}

void CP_FlowComplex::convert2cellNormal(CP_2cell& _2cell,CP_Triganle3D& tri)
{
	tri.normalsetted=true;
	swap(tri.m_points[0],tri.m_points[2]);
	spread2cellTri(_2cell.index,&tri);
	/*for(unsigned unsigned int i=0;i<_2cell.m_triangle.size();i++)
	{
		swap(tricells[_2cell.m_triangle[i]]->m_points[0],tricells[_2cell.m_triangle[i]]->m_points[1]);
	}*/
}

void CP_FlowComplex::convertpatchNormal(CP_Patch& pPatch,CP_Triganle3D& tri)
{
	CP_2cell *p2cell=m_2cells[Locate2cell(tri._2cell)];
	convert2cellNormal(*p2cell,tri);
	p2cell->visited=true;
	spreadPatch2cell(p2cell->patch,p2cell);
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

void CP_FlowComplex::spread2cellTri(int _2cell,CP_Triganle3D* tri)
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
			spread2cellTri(_2cell,tricells[tri->m_adjTriangle[i]]);
		}//normalsetted
	}//i
}

void CP_FlowComplex::_2cellNormalConsensus()
{
	for(unsigned int i=0;i<m_2cells.size();i++)
	{
		//每个2cell内三角面片一致
		tricells[m_2cells[i]->m_triangle[0]]->normalsetted=true;
		spread2cellTri(m_2cells[i]->index,tricells[m_2cells[i]->m_triangle[0]]);
	}
}

void CP_FlowComplex::spreadPatch2cell(int _patch,CP_2cell *p2cell)
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
					convert2cellNormal(*p2celladj,*tricells[ptri->m_adjTriangle[k]]);
				}
				spreadPatch2cell(_patch,p2celladj);
			}
		}//k
	}//j
}

void CP_FlowComplex::patchNormalConsensus()
{
	for(unsigned int i=0;i<m_patches.size();i++)
	{
		m_2cells[m_patches[i]->m_2cells[0]]->visited=true;
		spreadPatch2cell(i,m_2cells[m_patches[i]->m_2cells[0]]);
	}
}

void CP_FlowComplex::spread(CP_Triganle3D* tri)
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
			spread(tricells[tri->m_adjTriangle[i]]);
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
			spread(tricells[i]);
		}
	}
}  

void CP_FlowComplex::SetPatchNormal()
{
	//for(int i=m_patches.size()-1;i>=0;i--)
	for(unsigned int i=0;i<m_patches.size();i++)
	{
		if(!m_patches[i]->visited)
		{
			m_patches[i]->visited=true;
			CP_2cell *p2cell=m_2cells[m_patches[i]->m_2cells[0]];
			CP_Vector3D ntri = (m_0cells[tricells[p2cell->m_triangle[0]]->m_points[1]] - m_0cells[tricells[p2cell->m_triangle[0]]->m_points[0]]) ^ 
				(m_0cells[tricells[p2cell->m_triangle[0]]->m_points[2]] - m_0cells[tricells[p2cell->m_triangle[0]]->m_points[0]]);
			CP_Vector3D ncp=m_0cells[tricells[p2cell->m_triangle[0]]->m_points[0]]-cp;
			if(ntri*ncp<0)
				convertpatchNormal(*m_patches[i],*tricells[p2cell->m_triangle[0]]);
			spreadPatchNormal(*m_patches[i]);
		}
	}//i
}

void CP_FlowComplex::spreadPatchNormal(CP_Patch& pPatch)
{
	for(unsigned int j=0;j<pPatch.m_boundary.size();j++)
	{
		CP_Triganle3D* ptri=tricells[pPatch.m_boundary[j]._triangle];//本2cell中与此边界相关的triganle
		if(ptri->m_adjTriangle.size()<=3){
		for(unsigned int k=0;k<ptri->m_adjTriangle.size();k++)
		{
			CP_2cell* p2celladj=m_2cells[Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)];
			if(p2celladj->patch!=-1){
				CP_Patch* pPatchadj=m_patches[p2celladj->patch];
				if(p2celladj->patch!=pPatch.index&&!pPatchadj->visited)
				{//与此三角面片相邻的另一个2cell Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)中的三角形tricells[ptri->m_adjTriangle[k]]
					pPatchadj->visited=true;
					int rsri=CommonEdgeReverse(*ptri,*tricells[ptri->m_adjTriangle[k]]);
					if(rsri!=-1)
					{
						convertpatchNormal(*pPatchadj,*tricells[ptri->m_adjTriangle[k]]);
					}
					spreadPatchNormal(*pPatchadj);
				}
			}
			//p2cell.m_adj2cell.push_back(Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell));
		}//k
		}
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
				convert2cellNormal(*m_2cells[i],*tricells[m_2cells[i]->m_triangle[0]]);
			spread2cellNormal(*m_2cells[i]);
		}//flag visited
	}//i
}

void CP_FlowComplex::spread2cellNormal(CP_2cell& p2cell)
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
					convert2cellNormal(*p2celladj,*tricells[ptri->m_adjTriangle[k]]);
				}
				spread2cellNormal(*p2celladj);
			}
			//p2cell.m_adj2cell.push_back(Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell));
		}//k
	}//j
}

void CP_FlowComplex::SetCreatorAndDestoryer()
{
	int pNumOfVoid=0;
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

		for(int  j=0;j<vboundary.size();j++)
		{
			vboundary[j]->ResetDegreee();
		}

		if(exist)
		{//此2cell边界都存在，检查是否闭合空间的patch数增加，若增加，此面片为creator，否则destoryer
			int pCloseVoid=CheckClosedVoid(vboundary,i);
			if(pCloseVoid>pNumOfVoid)
			{
				pNumOfVoid=pCloseVoid;
				m_2cells[i]->type=1;
			}
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

void CP_FlowComplex::cutBranch(vector<CurveSegment*> &vboundary,CurveSegment& curve)
{
	//对使得curve存在的唯一一个2cell消去操作
	for (int i = 0; i < curve.incident2cell.size(); i++)
	{
		int icell=curve.incident2cell[i];
		if(m_2cells[icell]->flag)
		{
			m_2cells[icell]->flag=false;
			//所有边界线段度数-1并检查如果度数=1做剪枝操作
			for(unsigned int j=0;j<m_2cells[icell]->m_boundary.size();j++)
			{
				int icurve=LocateSegment(vboundary,m_2cells[icell]->m_boundary[j]);
				if(vboundary[icurve]->tmpdegree>0)
				{
					vboundary[icurve]->tmpdegree--;
					if(vboundary[icurve]->tmpdegree==1)
						cutBranch(vboundary,*vboundary[icurve]);
				}
			}//j
		}//2cell flag
	}//i
}

void CP_FlowComplex::setpaired3cells()
{
	vector<CurveSegment*> vboundary;
	for (int i = 0; i < desN; i++)
	{
		for(unsigned int j=0;j<m_2cells[i]->m_boundary.size();j++)
		{
			int curve=LocateSegment(vboundary,m_2cells[i]->m_boundary[j]);
			if(curve==-1)
			{
				vboundary.push_back(&m_2cells[i]->m_boundary[j]);
				m_2cells[i]->m_boundary[j].degree=1;
				m_2cells[i]->m_boundary[j].incident2cell.push_back(i);
			}else
			{
				vboundary[curve]->degree++;
				vboundary[curve]->incident2cell.push_back(i);
			}
		}//j
	}//i

	//所有destoryer每次和一个creator去cutBranch，得到每个creator的3cell
	for(int i=desN;i<m_2cells.size();i++)
	{
		int _2cell=i;    
		//考虑此creator与所有destoryer
		for(unsigned int j=0;j<m_2cells[_2cell]->m_boundary.size();j++)
		{//边加入vboundary
			int curve=LocateSegment(vboundary,m_2cells[_2cell]->m_boundary[j]);
			vboundary[curve]->degree++;
			vboundary[curve]->incident2cell.push_back(_2cell);
		}//j

		for(unsigned int j=0;j<vboundary.size();j++)
		{//重置tmpdegree
			vboundary[j]->ResetDegreee();
		}

		for(unsigned int j=0;j<vboundary.size();j++)
		{//剪边操作
			if(vboundary[j]->tmpdegree==1)
				cutBranch(vboundary,*vboundary[j]);
		}

		CP_3cell *p3cell=new CP_3cell();
		m_3cells.push_back(p3cell);
		p3cell->m_2cells.push_back(m_2cells[_2cell]->index);
		m_2cells[_2cell]->p3cell.push_back(m_2cells[_2cell]->index);
		for(unsigned int j=0;j<desN;j++)
		{//记录剩下的2cell为构成此creator的paired 3cell的面片
			if(m_2cells[j]->flag){
				m_2cells[_2cell]->p3cell.push_back(m_2cells[j]->index);
				p3cell->m_2cells.push_back(m_2cells[j]->index);
			}
		}

		//重置
		for(unsigned int j=0;j<m_2cells[_2cell]->m_boundary.size();j++)
		{//重置vboundary
			int curve=LocateSegment(vboundary,m_2cells[_2cell]->m_boundary[j]);
			vboundary[curve]->degree--;
			vboundary[curve]->incident2cell.pop_back();
		}//j
		//重置
		Reset2cellFlag(_2cell);
	}//i

	//所有vboundary的数据归零
	for(unsigned int i=0;i<vboundary.size();i++)
	{
		vboundary[i]->degree=0;
		vboundary[i]->tmpdegree=0;
		vector<int>().swap(vboundary[i]->incident2cell);
	}//i


	//对相互包含的3cells进行分割
	//for(unsigned int i=0;i<m_3cells.size();i++)
	//{
	//	for(int m=0;m<m_3cells.size();m++)
	//	{
	//		if(i!=m){//相互处理过的不再处理
	//			vector<int>::iterator it=find(m_3cells[i]->cmp3cells.begin(),m_3cells[i]->cmp3cells.end(),m);
	//			if(it==m_3cells[i]->cmp3cells.end()){
	//				m_3cells[m]->cmp3cells.push_back(i);
	//				if(m_3cells[i]->m_2cells.size()<m_3cells[m]->m_2cells.size())
	//					split3cells(*m_3cells[i],*m_3cells[m]);
	//				else
	//					split3cells(*m_3cells[m],*m_3cells[i]);
	//			}
	//		}
	//	}
	//}
}

void CP_FlowComplex::split3cells(CP_3cell& s,CP_3cell& l)
{
	bool splitEnable=false;
	for(unsigned int i=0;i<s.m_2cells.size();i++)
	{
		for(unsigned int j=0;j<l.m_2cells.size();j++)
		{
			if(s.m_2cells[i]==l.m_2cells[j])
				splitEnable=true;
		}
	}//i
	if(splitEnable)
	{
		for(unsigned int i=0;i<s.m_2cells.size();i++)
		{
			vector<int>::iterator it=find(l.m_2cells.begin(),l.m_2cells.end(),s.m_2cells[i]);
			if(it==l.m_2cells.end()) //没找到
				;
			else //找到
				l.m_2cells.erase(it);
		}
		l.m_2cells.push_back(s.m_2cells[0]);
		////加上封口
		//vector<CurveSegment *> vboundary;
		//for(unsigned int i=0;i<s.m_2cells.size();i++)
		//{//s
		//	int _2cell=Locate2cell(s.m_2cells[i]);
		//	for(unsigned int j=0;j<m_2cells[_2cell]->m_boundary.size();j++)
		//	{
		//		int curve=LocateSegment(vboundary,m_2cells[_2cell]->m_boundary[j]);
		//		if(curve==-1)
		//		{
		//			vboundary.push_back(&m_2cells[_2cell]->m_boundary[j]);
		//			m_2cells[_2cell]->m_boundary[j].degree=1;
		//			m_2cells[_2cell]->m_boundary[j].incident2cell.push_back(_2cell);
		//		}else
		//		{
		//			vboundary[curve]->degree++;
		//			vboundary[curve]->incident2cell.push_back(_2cell);
		//		}
		//	}//j
		//}//i

		//for(unsigned int i=0;i<l.m_2cells.size();i++)
		//{//l
		//	int _2cell=Locate2cell(l.m_2cells[i]);
		//	for(unsigned int j=0;j<m_2cells[_2cell]->m_boundary.size();j++)
		//	{
		//		int curve=LocateSegment(vboundary,m_2cells[_2cell]->m_boundary[j]);
		//		if(curve==-1)
		//		{
		//			vboundary.push_back(&m_2cells[_2cell]->m_boundary[j]);
		//			m_2cells[_2cell]->m_boundary[j].degree=1;
		//			m_2cells[_2cell]->m_boundary[j].incident2cell.push_back(_2cell);
		//		}else
		//		{
		//			vboundary[curve]->degree++;
		//			vboundary[curve]->incident2cell.push_back(_2cell);
		//		}
		//	}//j
		//}//i
		//cout<<"correct"<<endl;
		////扩展s的creator 2cell，然后扩展面加入l
		//CP_Patch pPatch;
		//pPatch.m_2cells.push_back(s.m_2cells[0]);
		//expand2cell(*m_2cells[Locate2cell(s.m_2cells[0])],vboundary,pPatch);
		//
		//for(unsigned int i=0;i<pPatch.m_2cells.size();i++)
		//{
		//	l.m_2cells.push_back(m_2cells[pPatch.m_2cells[i]]->index);
		//}
		////boundary复原
		//for(unsigned int i=0;i<vboundary.size();i++)
		//{
		//	vboundary[i]->degree=0;
		//	vector<int>().swap(vboundary[i]->incident2cell);
		//}//i
	}//splitEnable
}

int CP_FlowComplex::CheckClosedVoid(vector<CurveSegment*> &vboundary,int len)
{
	////剪枝法判断边集
	for(unsigned int i=0;i<vboundary.size();i++)
	{
		if(vboundary[i]->tmpdegree==1)
			cutBranch(vboundary,*vboundary[i]);
	}
	//如果0-i 2cell有存在，返回存在的patch数
	int num=0;
	for(unsigned int i=0;i<=len;i++)
	{
		if(m_2cells[i]->flag)
			num++;
	}
	
	return num;
}

void CP_FlowComplex::Reset2cellFlag(int len)
{
	for(unsigned int i=0;i<=len;i++)
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

bool CP_FlowComplex::ExistTriangle(vector<CP_Triganle3D*> visitedtri,CP_Triganle3D &tri)
{
	for (int i = 0; i < visitedtri.size(); i++)
	{
		int abc = 0;
		for (unsigned int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				if (visitedtri[i]->m_points[j]==tri.m_points[k])
					abc++;
			}

		}
		if (abc == 3)
			return true;
	}
	return false;
}

double CP_FlowComplex::calcTriVolume_projection(const CP_Triganle3D &tri)
{
	double average_height;
	double projected_area;
	double projected_vec1_x,projected_vec1_y;
	double projected_vec2_x,projected_vec2_y;
	double temp_volume;
	average_height=(m_0cells[tri.m_points[0]].m_z+m_0cells[tri.m_points[1]].m_z+m_0cells[tri.m_points[2]].m_z)/3.0;

	average_height=fabs(average_height);

	projected_vec1_x=m_0cells[tri.m_points[1]].m_x-m_0cells[tri.m_points[0]].m_x;
	projected_vec1_y=m_0cells[tri.m_points[1]].m_y-m_0cells[tri.m_points[0]].m_y;
	projected_vec2_x=m_0cells[tri.m_points[2]].m_x-m_0cells[tri.m_points[0]].m_x;
	projected_vec2_y=m_0cells[tri.m_points[2]].m_y-m_0cells[tri.m_points[0]].m_y;

	projected_area=(projected_vec1_x*projected_vec2_y-projected_vec2_x*projected_vec1_y)/2.0;

	temp_volume=projected_area*average_height;
	return fabs(temp_volume);
}

void CP_FlowComplex::calc3cellVolume(CP_2cell& pc2cell)
{//计算creator 3cell的大小
	double sum_volume=0;
	//
	int tri_num=pc2cell.m_triangle.size();
	for(unsigned int j = 0; j < tri_num; j++)
	{
		sum_volume+=calcTriVolume_projection(*tricells[pc2cell.m_triangle[j]]);
	}
	for(unsigned int i=0;i<pc2cell.p3cell.size();i++)
	{
		int nowi=Locate2cell(pc2cell.p3cell[i]);
		CP_2cell *p2cell = m_2cells[nowi];
		tri_num=p2cell->m_triangle.size();
		for(unsigned int j=0;j<tri_num;j++)
		{
			sum_volume+=calcTriVolume_projection(*tricells[p2cell->m_triangle[j]]);
		}
	}//i
	pc2cell.dis3cell=sum_volume;
}

void CP_FlowComplex::expand2cell(CP_2cell& p2cell,vector<CurveSegment*> vb,CP_Patch &pPatch)
{
	m_2cells[Locate2cell(p2cell.index)]->visited=true;
	for(unsigned int i=0;i<p2cell.m_boundary.size();i++)
	{
		int curve=LocateSegment(vb,p2cell.m_boundary[i]);
		if(vb[curve]->degree==2)
		{
			//incident 2cell 加入到p2cell中
			for(unsigned int j=0;j<vb[curve]->incident2cell.size();j++)
			{
				int _2cell=vb[curve]->incident2cell[j];
				if(m_2cells[_2cell]->index!=p2cell.index&&!m_2cells[_2cell]->visited)
				{
					pPatch.m_2cells.push_back(_2cell);
					expand2cell(*m_2cells[_2cell],vb,pPatch);
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
void CP_FlowComplex::spreadPatchColor(CP_Patch& pPatch)
{
	pPatch.visited=true;
	vector<int> colored;
	for(unsigned int i=0;i<pPatch.m_adjPatch.size();i++)
	{
		if(m_patches[pPatch.m_adjPatch[i]]->visited)
			colored.push_back(m_patches[pPatch.m_adjPatch[i]]->color);
	}
	const int NUM_OF_COLORS=6;
	for(unsigned int i=0;i<NUM_OF_COLORS;i++)
	{
		vector<int>::iterator it=find(colored.begin(),colored.end(),i);
		if(it==colored.end())
		{
			pPatch.color=i;
			break;
		}
	}
	//spread
	for(unsigned int i=0;i<pPatch.m_adjPatch.size();i++)
	{
		if(!m_patches[pPatch.m_adjPatch[i]]->visited)
			spreadPatchColor(*m_patches[pPatch.m_adjPatch[i]]);
	}
}
void CP_FlowComplex::SetPatchColor()
{
	vector<CP_Patch*> vP;
	for(unsigned int i=0;i<m_patches.size();i++)
		vP.push_back(m_patches[i]);

	sort(vP.begin(),vP.end(),colorNumCmp);
	//从vp中第一个，即相邻patch最多的开始
	for(int i=vP.size()-1;i>=0;i--)
	{
		if(!vP[i]->visited)
			spreadPatchColor(*vP[i]);
	}
}

void CP_FlowComplex::clearAll()
{
	desN=0;
	show=0;
	inputPoints=0;
	inputCurves=0;
	oripatches=0;

	vector<CP_Point3D>().swap(m_0cells);
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
	vector<CP_PolyLine3D>().swap(m_PolyLine);
	for(unsigned int i=0;i<tricells.size();i++)
		delete tricells[i];
	vector<CP_Triganle3D*>().swap(tricells);
	for(unsigned int i=0;i<ctri.size();i++)
		delete ctri[i];
	vector<CP_Triganle3D*>().swap(ctri);
	for(unsigned int i=0;i<visitedtri.size();i++)
		delete visitedtri[i];
	vector<CP_Triganle3D*>().swap(visitedtri);
	vector<CP_Point3D>().swap(vjoint);
}

void CP_FlowComplex::DrawPoints()
{
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(3.0f);
	for (unsigned int j = 0; j < inputPoints; j++)
	{
		glBegin(GL_POINTS);//必须是加上s，要不然显示不了
		glVertex3f(m_0cells[j].m_x, m_0cells[j].m_y,m_0cells[j].m_z);
		glEnd();
	}
}

void CP_FlowComplex::DrawDelaunyTriangles()
{
	glColor4f(0.7,0.7,0.7,1);
	for (unsigned int i = 0; i <delauny2cells.size(); i++)
	{
		CP_Triganle3D *pTri = delauny2cells[i];
		DrawTriangle(*pTri);
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

	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH, GL_NICEST);
	glLineWidth(0.5f);
	glColor3f(0.0, 0.0, 0.0);
	for(unsigned int k=0;k<p2cell.m_triangle.size();k++)
	{
		CP_Triganle3D *pTri = tricells[p2cell.m_triangle[k]];
		glBegin(GL_LINE_LOOP);
		for (unsigned int j = 0; j < 3; j++)
			glVertex3f(m_0cells[pTri->m_points[j]].m_x, m_0cells[pTri->m_points[j]].m_y, m_0cells[pTri->m_points[j]].m_z);
		glEnd();
	}//k
	glDisable(GL_LINE_SMOOTH);

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

	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH, GL_NICEST);
	glLineWidth(1.5f);glColor3f(0.0f, 0.0f, 1.0f);
	for(unsigned int j=0;j<p2cell.m_boundary.size();j++)
	{
		glBegin(GL_LINE_STRIP);
		glVertex3d(m_0cells[p2cell.m_boundary[j].sp].m_x,m_0cells[p2cell.m_boundary[j].sp].m_y,m_0cells[p2cell.m_boundary[j].sp].m_z);
		glVertex3d(m_0cells[p2cell.m_boundary[j].ep].m_x,m_0cells[p2cell.m_boundary[j].ep].m_y,m_0cells[p2cell.m_boundary[j].ep].m_z);
		glEnd();
	}
	glDisable(GL_LINE_SMOOTH);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPopAttrib();
}

void CP_FlowComplex::DrawPatchBoundary(const CP_Patch &pPatch)
{
	glPushAttrib (GL_ALL_ATTRIB_BITS);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.5f, -1.0f);
	for(unsigned int j=0;j<pPatch.m_boundary.size();j++)
	{
		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH, GL_NICEST);
		glLineWidth(2.5f);glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_LINE_STRIP);
		glVertex3d(m_0cells[pPatch.m_boundary[j].sp].m_x,m_0cells[pPatch.m_boundary[j].sp].m_y,m_0cells[pPatch.m_boundary[j].sp].m_z);
		glVertex3d(m_0cells[pPatch.m_boundary[j].ep].m_x,m_0cells[pPatch.m_boundary[j].ep].m_y,m_0cells[pPatch.m_boundary[j].ep].m_z);
		glEnd();
		glDisable(GL_LINE_SMOOTH);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPopAttrib();
}

void CurveSegment::ResetDegreee()
{
	tmpdegree=degree;
}

CurveSegment::CurveSegment(int lp,int rp)
{
	_triangle=-1;
	sp=lp;
	ep=rp;
}


int CurveSegment::GetPointIndex(int i)
{
	if(i==0)
		return sp;
	else
		return ep;
}

CP_2cell::CP_2cell(void)
{
	index=-1;
	visited=false;
	distance=0.0;
	dis3cell=0.0;
	persistence=0.0;
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
	color=-1;
}


CP_Patch::~CP_Patch(void)
{
}


CP_3cell::CP_3cell(void)
{
}


CP_3cell::~CP_3cell(void)
{
}
