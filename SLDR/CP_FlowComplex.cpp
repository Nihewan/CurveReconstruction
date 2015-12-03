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
	for(int i=0;i<m_1cells.size();i++)
		delete m_1cells[i];
	for(int i=0;i<m_2cells.size();i++)
		delete m_2cells[i];
	for(int i=0;i<delauny2cells.size();i++)
		delete delauny2cells[i];
	for(int i=0;i<tricells.size();i++)
		delete tricells[i];
	for(int i=0;i<ctri.size();i++)
		delete ctri[i];
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
	for(int i=0;i<m_PolyLine.size();i++)
	{
		if(m_PolyLine[i].m_points.size()>2)
		{
			for(int j=1;j<m_PolyLine[i].m_points.size()-1;j++)
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
	for(int i=0;i<m_PolyLine.size();i++)
	{
		for(int j=0;j<m_PolyLine[i].m_points.size()-1;j++)
		{//线段j，j+1
			if(j==0||j==m_PolyLine[i].m_points.size()-2){
			double m_x=(m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2;
			double m_y=(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2;
			double m_z=(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2;
			CP_Point3D midPoint(m_x,m_y,m_z);//圆心
			double radius_pq=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1])/2;//半径

			for(int m=0;m<m_PolyLine.size();m++)
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

	for(int i=0;i<vjoint.size();i++)
		subdivideSegsJointV(vjoint[i]);

	//gabriel and 1-cells 可在三角形中表示
	for(int i=0;i<m_PolyLine.size();i++)
	{
		for(int j=0;j<m_PolyLine[i].m_points.size()-1;j++)
		{//线段j，j+1
			double m_x=(m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2;
			double m_y=(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2;
			double m_z=(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2;
			CP_Point3D midPoint(m_x,m_y,m_z);//圆心
			double radius_pq=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1])/2;//半径
		
			//同一条曲线非Gabriel需投影
			//对j，j+1线段检查其他点
			for(int n=0;n<m_PolyLine[i].m_points.size();n++)
			{
				if(n!=j&&n!=(j+1))
				{
					if(!IsGabriel(midPoint,m_PolyLine[i].m_points[n],radius_pq))
					{
						m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j+1,ProjectionPoint(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1],m_PolyLine[i].m_points[n]));
						j++;
						midPoint.m_x=(m_PolyLine[i].m_points[j].m_x+m_PolyLine[i].m_points[j+1].m_x)/2;
						midPoint.m_y=(m_PolyLine[i].m_points[j].m_y+m_PolyLine[i].m_points[j+1].m_y)/2;
						midPoint.m_z=(m_PolyLine[i].m_points[j].m_z+m_PolyLine[i].m_points[j+1].m_z)/2;
						radius_pq=dist(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1])/2;
					}//while Gabriel
				}
			}//n

			//曲线间非Gabriel需投影
			for(int m=0;m<m_PolyLine.size();m++)
			{
				if(i==m)
					;
				else 
				{
					for(int n=0;n<m_PolyLine[m].m_points.size();n++)
					{
						//别的点不在线段内
						if(!IsGabriel(midPoint,m_PolyLine[m].m_points[n],radius_pq))
						{
							//在交汇点冲突时应该以此为joint点等距细分，这样后边不会再需要投影了
							m_PolyLine[i].m_points.insert(m_PolyLine[i].m_points.begin()+j+1,ProjectionPoint(m_PolyLine[i].m_points[j],m_PolyLine[i].m_points[j+1],m_PolyLine[m].m_points[n]));
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
	for(int i=0;i<m_PolyLine.size();i++)
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
	for (int i = 0; i < v.size(); i++)
	{
		if (fabs(v[i].m_x - p.m_x) < TOL&&fabs(v[i].m_y - p.m_y) < TOL&&fabs(v[i].m_z - p.m_z) < TOL)
			return true;
	}
	return false;
}

int CP_FlowComplex::LocatePoint(const CP_Point3D &p)
{
	for (int i = 0; i < m_0cells.size(); i++)
	{
		if (fabs(m_0cells[i].m_x - p.m_x) < TOL&&fabs(m_0cells[i].m_y - p.m_y) < TOL&&fabs(m_0cells[i].m_z - p.m_z) < TOL)
			return i;
	}
	return -1;
}

int CP_FlowComplex::ExistTriangle(vector<CP_Triganle3D*> &v,CP_Triganle3D &t)
{
	for (int i = 0; i < v.size(); i++)
	{
		int abc = 0;
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				if (t.m_points[j]==v[i]->m_points[k])
					abc++;
			}

		}
		if (abc == 3)
		return i;
	}
	return -1;
}

bool CP_FlowComplex::noCover(CP_Triganle3D &ltri,CP_Triganle3D &rtri)
{
	int abc = 0;
	int sp[2]={-1,-1};//公共边顶点的编号
	int lsr=-1,rsr=-1;//ltri 和rtri的源点（计算法向）
	for (int j = 0; j < 3; j++)
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
		for (int j=0;j<3;j++)
		{
			int tmp=-1;
			for(int k=0;k<2;k++)
			{
				if(ltri.m_points[j]==sp[k])
					tmp=j;
			}
			if(tmp==-1)
				lsr=ltri.m_points[j];
		}

		for (int j=0;j<3;j++)
		{
			int tmp=-1;
			for(int k=0;k<2;k++)
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
	for(int i=0;i<m_2cells.size();i++)
	{
		if(m_2cells[i]->index==_2cell)
			return i;
	}
	return -1;
}

vector<int> CP_FlowComplex::GetIncidentTri(const CP_Point3D& v1,const CP_Point3D& v2)
{
	vector<int> v;
	for(int i=0;i<v1.m_adjTriangle.size();i++)
	{
		for(int j=0;j<v2.m_adjTriangle.size();j++)
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
	for(int i=0;i<tricells.size();i++)
	{
		if(m_2cells[Locate2cell(tricells[i]->_2cell)]->flag)
		for(int k=0;k<3;k++)
			m_0cells[tricells[i]->m_points[k]].m_adjTriangle.push_back(i);
	}

	for(int i=0;i<tricells.size();i++)
	{
		if(m_2cells[Locate2cell(tricells[i]->_2cell)]->flag)
		for(int j=0;j<3;j++)
		{
			vector<int> v=GetIncidentTri(m_0cells[tricells[i]->m_points[j%3]],m_0cells[tricells[i]->m_points[(j+1)%3]]);
			for(int k=0;k<v.size();k++)
			{
				if(i!=v[k])
					tricells[i]->m_adjTriangle.push_back(v[k]);
			}
		}
	}
}

void CP_FlowComplex::convert2cellNormal(CP_2cell& _2cell)
{
	for(int i=0;i<_2cell.m_triangle.size();i++)
	{
		int temp=tricells[_2cell.m_triangle[i]]->m_points[0];
		tricells[_2cell.m_triangle[i]]->m_points[0]=tricells[_2cell.m_triangle[i]]->m_points[2];
		tricells[_2cell.m_triangle[i]]->m_points[2]=temp;
	}
}

int CommonEdgeReverse(CP_Triganle3D &ltri,CP_Triganle3D &rtri)
{//若方向相同返回rtri的源点，否则返回-1
	int abc=0;
	int sp[2]={-1,-1};//公共边顶点
	int rsri=-1,lsri=-1;;
	for (int j = 0; j < 3; j++)
	{
		for (int k = 0; k < 3; k++)
		{//公共边在ltri中j是按序的，如果相应k反序，则方向相逆
			if (ltri.m_points[j]==rtri.m_points[k])
			{   
				sp[abc++]=rtri.m_points[k];
			}
		}
	}

	for (int j=0;j<3;j++)
	{
		int tmp=-1;
		for(int k=0;k<2;k++)
		{
			if(ltri.m_points[j]==sp[k])
				tmp=j;
		}
		if(tmp==-1)
			lsri=j;
	}

	for (int j=0;j<3;j++)
	{
		int tmp=-1;
		for(int k=0;k<2;k++)
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
	for(int i=0;i<tri->m_adjTriangle.size();i++)
	{//邻接三角形公共边的方向应与tri相同
		if(!tricells[tri->m_adjTriangle[i]]->normalsetted&&tricells[tri->m_adjTriangle[i]]->_2cell==_2cell)
		{//同属一个2cell且未被访问,测试法向
			tricells[tri->m_adjTriangle[i]]->normalsetted=true;
			int rsri=CommonEdgeReverse(*tri,*tricells[tri->m_adjTriangle[i]]);
			if(rsri!=-1)
			{
				int temp=tricells[tri->m_adjTriangle[i]]->m_points[(rsri+1)%3];
				tricells[tri->m_adjTriangle[i]]->m_points[(rsri+1)%3]=tricells[tri->m_adjTriangle[i]]->m_points[(rsri+2)%3];
				tricells[tri->m_adjTriangle[i]]->m_points[(rsri+2)%3]=temp;
			}
			spread2cellTri(_2cell,tricells[tri->m_adjTriangle[i]]);
		}//normalsetted
	}//i
}

void CP_FlowComplex::_2cellNormalConsensus()
{
	for(int i=0;i<m_2cells.size();i++)
	{
		//每个2cell内三角面片一致
		tricells[m_2cells[i]->m_triangle[0]]->normalsetted=true;
		spread2cellTri(m_2cells[i]->index,tricells[m_2cells[i]->m_triangle[0]]);
	}
}

void CP_FlowComplex::spread(CP_Triganle3D* tri)
{
	for(int i=0;i<tri->m_adjTriangle.size();i++)
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
	for(int i=0;i<tricells.size();i++)
	{
		if(!tricells[i]->normalsetted&&m_2cells[Locate2cell(tricells[i]->_2cell)]->flag)
		{
			tricells[i]->normalsetted=true;
			spread(tricells[i]);
		}
	}
}  

void CP_FlowComplex::Set2cellNormal()
{
	for(int i=desN-1;i>=0;i--)
	{
		if(m_2cells[i]->flag&&!m_2cells[i]->normalsetted)
		{
			m_2cells[i]->normalsetted=true;
			CP_Vector3D ntri = (m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[1]] - m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[0]]) ^ 
				(m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[2]] - m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[0]]);
			CP_Vector3D ncp=m_0cells[tricells[m_2cells[i]->m_triangle[0]]->m_points[0]]-cp;
			if(ntri*ncp<0)
				convert2cellNormal(*m_2cells[i]);
			spread2cellNormal(*m_2cells[i]);
			/*sort(m_2cells[i]->m_adj2cell.begin(),m_2cells[i]->m_adj2cell.end());
			vector<int>::iterator iter = unique(m_2cells[i]->m_adj2cell.begin(),m_2cells[i]->m_adj2cell.end());
			m_2cells[i]->m_adj2cell.erase(iter,m_2cells[i]->m_adj2cell.end());*/
		}//flag normalsetted
	}//i
}

void CP_FlowComplex::spread2cellNormal(CP_2cell& p2cell)
{
	for(int j=0;j<p2cell.m_boundary.size();j++)
	{
		CP_Triganle3D* ptri=tricells[p2cell.m_boundary[j]._triangle];//本2cell中与此边界相关的triganle
		for(int k=0;k<ptri->m_adjTriangle.size();k++)
		{
			CP_2cell* p2celladj=m_2cells[Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)];
			if(tricells[ptri->m_adjTriangle[k]]->_2cell!=p2cell.index&&p2celladj->flag&&!p2celladj->normalsetted)
			{//与此三角面片相邻的另一个2cell Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell)中的三角形tricells[ptri->m_adjTriangle[k]]
				p2celladj->normalsetted=true;
				int rsri=CommonEdgeReverse(*ptri,*tricells[ptri->m_adjTriangle[k]]);
				if(rsri!=-1)
				{
					convert2cellNormal(*p2celladj);
				}
				spread2cellNormal(*p2celladj);
			}
			p2cell.m_adj2cell.push_back(Locate2cell(tricells[ptri->m_adjTriangle[k]]->_2cell));
		}//k
	}//j
}

void CP_FlowComplex::SetCreatorAndDestoryer()
{
	int pNumOfVoid=0;
	vector<CurveSegment*> vboundary;
	for(int i=0;i<m_2cells.size();i++)
	{
		bool exist=true;
		for(int j=0;j<m_2cells[i]->m_boundary.size();j++)
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
			for(int j=0;j<m_2cells[icell]->m_boundary.size();j++)
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

int CP_FlowComplex::CheckClosedVoid(vector<CurveSegment*> &vboundary,int len)
{
	////剪枝法判断边集
	for(int i=0;i<vboundary.size();i++)
	{
		if(vboundary[i]->tmpdegree==1)
			cutBranch(vboundary,*vboundary[i]);
	}
	//如果0-i 2cell有存在，返回存在的patch数
	int num=0;
	for(int i=0;i<=len;i++)
	{
		if(m_2cells[i]->flag)
			num++;
	}
	
	//重置所有curve degree
	for(int  i=0;i<vboundary.size();i++)
	{
		vboundary[i]->ResetDegreee();
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

//CurveSegment::CurveSegment(const CP_Point3D& s, const CP_Point3D& e,int d)
//{
//	m_startPt.m_x=s.m_x;
//	m_startPt.m_y = s.m_y;
//	m_startPt.m_z = s.m_z;
//
//	m_endPt.m_x = e.m_x;
//	m_endPt.m_y = e.m_y;
//	m_endPt.m_z = e.m_z;
//
//	degree = d;
//}

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
	normalsetted=false;
	flag=true;
	type=0;
}


CP_2cell::~CP_2cell(void)
{
}

