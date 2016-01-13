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
		SubdivideSegsJointV(vjoint[i]);

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
				ReverseForProjection(m_PolyLine[i].m_points[j]);
				done=true;
				break;
			}
		}//j
		if(done)
			break;
	}//i
}

bool CP_FlowComplex::IsGabriel(const CP_Point3D &c,const CP_Point3D &r,double radius_pq)
{//r是否在以c为圆心以radius为半径的球中,在返回false，不在true
	double dis=dist(c,r)-radius_pq;
	if(fabs(dis)>TOL&&dis<0)
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
			if(m_1cells[curve]->degree!=-1)
			{
				m_1cells[curve]->degree++;
			}
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
				if(p2celladj->patch!=-1&&p2celladj->patch!=m_patches[i]->index&&p2celladj->patch<oripatches){
					vector<int>::iterator it=find(m_patches[i]->m_adjPatch.begin(),m_patches[i]->m_adjPatch.end(),p2celladj->patch);
					if(it==m_patches[i]->m_adjPatch.end())
						m_patches[i]->m_adjPatch.push_back(p2celladj->patch);
				}
			}//k
		}//j
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
		m_2cells[m_patches[i]->m_2cells[0]]->visited=true;
		SpreadPatch2cell(i,m_2cells[m_patches[i]->m_2cells[0]]);
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
	//for(int i=m_patches.size()-1;i>=0;i--)
	for(unsigned int i=1;i<m_patches.size();i++)
	{
		if(!m_patches[i]->visited)
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
						ConvertpatchNormal(tricells[ptri->m_adjTriangle[k]]);
					}
					SpreadPatchNormal(*pPatchadj);
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
		if(vb[curve]->degree==2)
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

void CP_FlowComplex::GetPatchBoundary()
{
	for(unsigned int i=0;i<m_patches.size();i++)
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
				}else
					lvec.push_back(p2cell->m_boundary[k]);
			}
		}//j
		for(unsigned int j=0;j<lvec.size();j++)
			m_patches[i]->m_boundary.push_back(lvec[j]);
	}//i
}

void CP_FlowComplex::SeekCreatorPatch()
{
	CP_Patch *pPatch=new CP_Patch();
	m_patches.push_back(pPatch);
	pPatch->m_2cells.push_back(m_3cells[_3cellN-1]->m_2cells[0]);
	//如果非封闭模型却要expand最大creator，边界没有加入导致消去destoryer面
	Expand2cell(*m_2cells[m_3cells[_3cellN-1]->m_2cells[0]],m_1cells,pPatch);

	pPatch->index=m_patches.size()-1;
	int _patch=m_patches.size()-1;
	for(unsigned int i=0;i<pPatch->m_2cells.size();i++)
	{
		m_2cells[pPatch->m_2cells[i]]->patch=_patch;
	}
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
			for(unsigned int i=0;i<pPatch->m_2cells.size();i++)
			{
				m_2cells[pPatch->m_2cells[i]]->patch=_patch;
			}
		}
	}//i
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


int CurveSegment::GetPointIndex(int i) const
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
	flag=true;
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
	return *this;
}
