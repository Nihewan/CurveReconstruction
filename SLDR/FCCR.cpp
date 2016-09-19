#include "stdafx.h"
#include "CP_FlowComplex.h"
#include "FCCR.h"
#include <iostream>
#include <fstream>
#include<functional> 
#include<algorithm>
FCCR::FCCR(void)
{
	maxhd=0;
	voids=0;
	epsilon=0.055;
	feasa=0.7;

	showResult=false;
	IsProcess=false;
}


FCCR::~FCCR(void)
{
}

void FCCR::ReSet()
{
	maxhd=0;
	voids=0;
	epsilon=0.055;
	feasa=0.7;
	filename="";

	showResult=false;
	IsProcess=false;

	T.clear();
	m_FlowComplex->ClearAll();
}

double FCCR::getHausdorffDistance(const CP_PolyLine3D &curveA,const CP_PolyLine3D &curveB)
{
	double maxAB = 0;//A���е㵽B���е���Сֵ������
	for (unsigned int i = 0; i < curveA.m_points.size();i++)
	{
		double min = dist(curveA.m_points[i], curveB.m_points[0]);//��i��B���е����Сֵ
		for (unsigned int j = 1; j < curveB.m_points.size(); j++)
		{
			double dis_ij = dist(curveA.m_points[i], curveB.m_points[j]);
			if (min>dis_ij)
			{
				min = dis_ij;
			}
		}
		if (maxAB<min)
		{
			maxAB = min;
		}
	}
	double maxBA = 0;//A���е㵽B���е���Сֵ������
	for (unsigned int i = 0; i < curveB.m_points.size(); i++)
	{
		double min = dist(curveB.m_points[i], curveA.m_points[0]);//��i��A���е����Сֵ
		for (unsigned int j = 1; j < curveA.m_points.size(); j++)
		{
			double dis_ij = dist(curveB.m_points[i], curveA.m_points[j]);
			if (min>dis_ij)
			{
				min = dis_ij;
			}
		}
		if (maxBA<min)
		{
			maxBA = min;
		}
	}

	return max(maxAB, maxBA);
}

void FCCR::ToPolyLine()
{
	//������
	for (unsigned int i = 0; i < m_VT_PolyLine->size(); i++)
	{
		//������i����������k��k��=i���ĺ�˹������
		double minhd = MAX_DISTANCE;
		for (unsigned int k = 0; k < m_VT_PolyLine->size(); k++)
		{
			if (k != i)
			{
				double hd = getHausdorffDistance((*m_VT_PolyLine)[i], (*m_VT_PolyLine)[k]);
				if (hd<minhd)
				{
					minhd = hd;
				}
			}//if
		}//k

		if (minhd / 2>epsilon)
		minhd = epsilon;
		else
		minhd = minhd/2;

		int start = 0;
		CP_PolyLine3D poly;
		poly.tag=true;
		for (unsigned int j = 0; j < (*m_VT_PolyLine)[i].m_points.size(); j++)
		{
			if (j == 0){
				start = 0;
				poly.m_points.push_back((*m_VT_PolyLine)[i].m_points[j]);
			}
			else if (j == (*m_VT_PolyLine)[i].m_points.size() - 1)//�����յ㲻��minhd���Ǳ���������
			{
				poly.m_points.push_back((*m_VT_PolyLine)[i].m_points[j]);
				m_FlowComplex->m_PolyLine.push_back(poly);
			}
			else{
				/*if((*m_VT_PolyLine)[i].m_points.size()<23)
				{
					if(j%2==0){
					poly.m_points.push_back((*m_VT_PolyLine)[i].m_points[j]);
					start = j;
					}
				}else{*/
				if (dist((*m_VT_PolyLine)[i].m_points[start], (*m_VT_PolyLine)[i].m_points[j])>minhd)
				{
					poly.m_points.push_back((*m_VT_PolyLine)[i].m_points[j]);
					start = j;
				}
			//}
			}//else
		}//j
	//	std::cout << minhd << endl;
		if (minhd > maxhd)
			maxhd = minhd;
	}
   //std::cout << maxhd << endl;
	//Ԥ��������,ʹ��������Gariel����
	m_FlowComplex->Gabrielize();
	int num=0;
	//����0-cells
	for (unsigned int i = 0; i < m_FlowComplex->m_PolyLine.size(); i++)
	{
		int s=0;
		for (unsigned int j = 0; j < m_FlowComplex->m_PolyLine[i].m_points.size(); j++)
		{
			num++;
			int ipoint=m_FlowComplex->LocatePoint(m_FlowComplex->m_PolyLine[i].m_points[j]);
			if (ipoint==-1)
			{
				m_FlowComplex->m_0cells.push_back(m_FlowComplex->m_PolyLine[i].m_points[j]);
				m_FlowComplex->IsBoundBox(m_FlowComplex->m_0cells.back());
				if(j!=0)
				{
					CurveSegment* curve=new CurveSegment(s,m_FlowComplex->m_0cells.size()-1);
					m_FlowComplex->m_1cells.push_back(curve);
					curve->isBoundary=i;
				}
				s=m_FlowComplex->m_0cells.size()-1;
				m_FlowComplex->m_PolyLine[i].idx.push_back(s);
				ipoint=s;//�±�
			}
			else
			{
				if(j!=0)
				{
					CurveSegment* curve=new CurveSegment(s,ipoint);
					m_FlowComplex->m_1cells.push_back(curve);
					curve->isBoundary=i;
				}
				s=ipoint;
				m_FlowComplex->m_PolyLine[i].idx.push_back(s);
			}

			//������ߵĶ˵��±�
			if(j==0)
				m_FlowComplex->m_PolyLine[i].s=ipoint;
			else if(j==m_FlowComplex->m_PolyLine[i].m_points.size()-1)
				m_FlowComplex->m_PolyLine[i].e=ipoint;
		}//j
	}//i
	m_FlowComplex->inputPoints=m_FlowComplex->m_0cells.size();
	m_FlowComplex->inputCurveSegments=m_FlowComplex->m_1cells.size();

	double x=0,y=0,z=0;
	for(unsigned int i=0;i<m_FlowComplex->m_0cells.size();i++)
	{
		x+=m_FlowComplex->m_0cells[i].m_x;
		y+=m_FlowComplex->m_0cells[i].m_y;
		z+=m_FlowComplex->m_0cells[i].m_z;
	}
	m_FlowComplex->cp.m_x=x/m_FlowComplex->m_0cells.size();
	m_FlowComplex->cp.m_y=y/m_FlowComplex->m_0cells.size();
	m_FlowComplex->cp.m_z=z/m_FlowComplex->m_0cells.size();
}

void FCCR::ImprovedPolyline()
{
	ToPolyLine();
	//Ѱ����С����һ���ж�
	//1.����ͼ
	m_FlowComplex->graph.numVertexes=m_FlowComplex->inputPoints;
	m_FlowComplex->graph.numEdges=m_FlowComplex->m_PolyLine.size();
	//��������
	m_FlowComplex->graph.adjList.resize(m_FlowComplex->graph.numVertexes);
	for (unsigned int i = 0; i < m_FlowComplex->graph.numVertexes; i++)
	{
		m_FlowComplex->graph.adjList[i].data=i;
		m_FlowComplex->graph.adjList[i].firstedge=NULL;
	}
	//�����߱�
	EdgeNode *e;
	for (unsigned int i = 0; i < m_FlowComplex->m_PolyLine.size(); i++)
	{
		int start=m_FlowComplex->m_PolyLine[i].s;//����start��end
		int end=m_FlowComplex->m_PolyLine[i].e;

		e =new EdgeNode;
		e->adjvex = end;//�ڽ����Ϊend
		e->polyindex=i;
		e->next = m_FlowComplex->graph.adjList[start].firstedge;//��eָ��ָ��ǰ����ָ��Ľṹ
		m_FlowComplex->graph.adjList[start].firstedge = e;//����ǰ�����ָ��ָ��e,�±߷ŵ�ͷ��

		e = new EdgeNode;
		e->adjvex =start;
		e->polyindex=i;
		e->next = m_FlowComplex->graph.adjList[end].firstedge;
		m_FlowComplex->graph.adjList[end].firstedge = e;
	}
	
	//��¼�����
	for (unsigned int i = 0; i < m_FlowComplex->m_PolyLine.size(); i++)
	{
		int start=m_FlowComplex->m_PolyLine[i].s;
		int end=m_FlowComplex->m_PolyLine[i].e;
		vector<int>::iterator result;
		result= find( m_FlowComplex->vjoint.begin(), m_FlowComplex->vjoint.end(), start );
		if(result == m_FlowComplex->vjoint.end()) 
			m_FlowComplex->vjoint.push_back(start);

		result= find( m_FlowComplex->vjoint.begin(), m_FlowComplex->vjoint.end(), end );
		if(result == m_FlowComplex->vjoint.end()) 
			m_FlowComplex->vjoint.push_back(end);
	}
	
}

void FCCR::ShortestCycle()
{
	for (unsigned int i = 0; i < m_FlowComplex->m_PolyLine.size(); i++)
	{
		if(!m_FlowComplex->m_PolyLine[i].tag)
		{
			m_FlowComplex->FindShortestCycle(i);
		}
	}
}

void FCCR::ConfirmClassification()
{
	for (unsigned int i = 0; i < m_FlowComplex->m_PolyLine.size(); i++)
	{
		if(m_FlowComplex->m_PolyLine[i].cycle.size()%2==0&&!m_FlowComplex->m_PolyLine[i].tag)
		    m_FlowComplex->m_PolyLine[i].tag=true;
	}
}

void FCCR::SetSymmetricCurveTagTrue()
{
	vector<int> toexam;
	for (unsigned int i = 0; i < m_FlowComplex->m_PolyLine.size(); i++)
	{
		if(!m_FlowComplex->m_PolyLine[i].tag)
			toexam.push_back(i);
	}

	for(unsigned int i=0;i<toexam.size();++i)
	{
		CP_PolyLine3D *lpoly=&m_FlowComplex->m_PolyLine[toexam[i]];
		if(m_FlowComplex->ConnectToPolyBothEnds(toexam[i])==-1){
			for(unsigned int j=i+1;j<toexam.size();++j)
			{
				if (m_FlowComplex->ConnectToPolyBothEnds(toexam[j])==-1)
				{
					CP_PolyLine3D *rpoly=&m_FlowComplex->m_PolyLine[toexam[j]];
					//ֻ��һ����
					if(lpoly->s==rpoly->s&&lpoly->e!=rpoly->e)
					{
						if(lpoly->sharppointv.size()==rpoly->sharppointv.size()&&fabs(lpoly->GetLength()-rpoly->GetLength())<0.02)
						{//�۽���һ���ҳ���һ��
						//ÿ���۽ǵ�������ĳ���һ��
							lpoly->tag=true;
							rpoly->tag=true;
						}
					}else if(lpoly->s==rpoly->e&&lpoly->e!=rpoly->s)
					{
						if(lpoly->sharppointv.size()==rpoly->sharppointv.size()&&fabs(lpoly->GetLength()-rpoly->GetLength())<0.02)
						{
							lpoly->tag=true;
							rpoly->tag=true;
						}
					}else if(lpoly->e==rpoly->e&&lpoly->s!=rpoly->s)
					{
						if(lpoly->sharppointv.size()==rpoly->sharppointv.size()&&fabs(lpoly->GetLength()-rpoly->GetLength())<0.02)
						{
							lpoly->tag=true;
							rpoly->tag=true;
						}
					}
					else if(lpoly->e==rpoly->s&&lpoly->s!=rpoly->e)
					{
						if(lpoly->sharppointv.size()==rpoly->sharppointv.size()&&fabs(lpoly->GetLength()-rpoly->GetLength())<0.02)
						{
							lpoly->tag=true;
							rpoly->tag=true;
						}
					}
				}//j connection
			}//j
		}//i connection
	}//i
}

void FCCR::OnDelaunyTriangulation()
{//Delauny�ʷֲ�����voronoi
	vector<Point> P;

	for (unsigned int i = 0; i < m_FlowComplex->m_0cells.size(); i++)
	{
		Point p(m_FlowComplex->m_0cells[i].m_x, m_FlowComplex->m_0cells[i].m_y, m_FlowComplex->m_0cells[i].m_z);
		P.push_back(p);
	}
	T.insert(P.begin(), P.end());

	Facets_iterator fit;
	Delaunay::Finite_cells_iterator cit;
	Delaunay::Vertex_handle v;
	Delaunay::Cell_handle cell;
	for(fit = T.finite_facets_begin(); fit != T.finite_facets_end(); ++fit)
	{
		Triangle tri=T.triangle(*fit);
		CP_Point3D p0(to_double(tri.vertex(0).hx()), to_double(tri.vertex(0).hy()), to_double(tri.vertex(0).hz()));
		CP_Point3D p1(to_double(tri.vertex(1).hx()), to_double(tri.vertex(1).hy()), to_double(tri.vertex(1).hz()));
		CP_Point3D p2(to_double(tri.vertex(2).hx()), to_double(tri.vertex(2).hy()), to_double(tri.vertex(2).hz()));
		int v0=m_FlowComplex->LocatePoint(p0);
		int v1=m_FlowComplex->LocatePoint(p1);
		int v2=m_FlowComplex->LocatePoint(p2);
		CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);
		m_FlowComplex->delauny2cells.push_back(pTriangle);
		m_FlowComplex->non_gabriel_triangles.push_back(pTriangle);
	}
}

void FCCR::addrFacet(const Facet &f)
{//�˲�ֻ��ע�ཻ���棬�������ظ�������ཻ��
	Cell_handle  c=f.first;
	Triangle tri=T.triangle(f);//delaunay������
	if(!tri.is_degenerate())
	{
		CGAL::Object t;
		t=T.dual(f);
		if (CGAL::object_cast<Segment>(&t))
		{
			Segment s;
			assign(s, t);	
			if(!s.is_degenerate())
			{
				if(CGAL::do_intersect(s,tri))
				{//���ཻ�����һ������2-cells
					CGAL::Object intersecObject=CGAL::intersection(s,tri);
					if (CGAL::object_cast<Point>(&intersecObject))
					{
						CP_Point3D p0(to_double(tri.vertex(0).hx()), to_double(tri.vertex(0).hy()), to_double(tri.vertex(0).hz()));
						CP_Point3D p1(to_double(tri.vertex(1).hx()), to_double(tri.vertex(1).hy()), to_double(tri.vertex(1).hz()));
						CP_Point3D p2(to_double(tri.vertex(2).hx()), to_double(tri.vertex(2).hy()), to_double(tri.vertex(2).hz()));
						int v0=m_FlowComplex->LocatePoint(p0);
						int v1=m_FlowComplex->LocatePoint(p1);
						int v2=m_FlowComplex->LocatePoint(p2);
						CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);

						deleteTriangle(m_FlowComplex->non_gabriel_triangles,tri);
						Point intersec;
						assign(intersec, intersecObject);

						CP_Point3D pIntersec(to_double(intersec.hx()), to_double(intersec.hy()), to_double(intersec.hz()));

						m_FlowComplex->m_0cells.push_back(pIntersec);
						m_FlowComplex->m_critical.push_back(pIntersec);//critical points
						int vIntersect=m_FlowComplex->m_0cells.size()-1; 
						CP_2cell *p2cell=new CP_2cell();
						m_FlowComplex->m_2cells.push_back(p2cell);
						p2cell->p_critical=vIntersect;
						int _2cell=m_FlowComplex->m_2cells.size()-1;
						p2cell->index=_2cell;
						p2cell->pTri=pTriangle;
						p2cell->distance=dist(pIntersec,p0);
						p2cell->m_circulator.tri=(*pTriangle);
						p2cell->delaunytri.push_back(m_FlowComplex->LocateTriangle(m_FlowComplex->delauny2cells,*pTriangle));

						//����3������,����õ�EDGE
						int trivertice[3]={0};
						int verindex=0;
						for(int i=0;i<4;i++)
						{
							if(i!=f.second)
								trivertice[verindex++]=i;
						}

						for(int i=0;i<3;i++)
						{//��ÿ��edge,���ڽ���dual voronoi edge������ཻ������cells�����ཻ������cells
							Edge e(c,trivertice[i%3],trivertice[(i+1)%3]);
							addrVoroFace(e,vIntersect,tri,_2cell,&p2cell->m_circulator);
						}//for(int i=0;i<3;i++)
					}//if (CGAL::object_cast<Point>(&intersecObject))
				}//intersect
			}//is_degenerate
		}else
		{
			Ray r;
			assign(r, t);
			if(CGAL::do_intersect(r,tri))
			{
				CGAL::Object intersecObject=CGAL::intersection(r,tri);
				if (CGAL::object_cast<Point>(&intersecObject))
				{
					CP_Point3D p0(to_double(tri.vertex(0).hx()), to_double(tri.vertex(0).hy()), to_double(tri.vertex(0).hz()));
					CP_Point3D p1(to_double(tri.vertex(1).hx()), to_double(tri.vertex(1).hy()), to_double(tri.vertex(1).hz()));
					CP_Point3D p2(to_double(tri.vertex(2).hx()), to_double(tri.vertex(2).hy()), to_double(tri.vertex(2).hz()));
					int v0=m_FlowComplex->LocatePoint(p0);
					int v1=m_FlowComplex->LocatePoint(p1);
					int v2=m_FlowComplex->LocatePoint(p2);
					CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);
					
					deleteTriangle(m_FlowComplex->non_gabriel_triangles,tri);
					Point intersec;
					assign(intersec, intersecObject);
					CP_Point3D pIntersec(to_double(intersec.hx()), to_double(intersec.hy()), to_double(intersec.hz()));

					m_FlowComplex->m_0cells.push_back(pIntersec);
					m_FlowComplex->m_critical.push_back(pIntersec);
					int vIntersect=m_FlowComplex->m_0cells.size()-1;

					CP_2cell *p2cell=new CP_2cell();
					m_FlowComplex->m_2cells.push_back(p2cell);
					p2cell->p_critical=vIntersect;
					int _2cell=m_FlowComplex->m_2cells.size()-1;
					p2cell->index=_2cell;
					p2cell->pTri=pTriangle;
					p2cell->distance=dist(pIntersec,p0);
					p2cell->m_circulator.tri=(*pTriangle);
					p2cell->delaunytri.push_back(m_FlowComplex->LocateTriangle(m_FlowComplex->delauny2cells,*pTriangle));
					//����3������,����õ�EDGE
					int trivertice[3]={0};
					int verindex=0;
					for(int i=0;i<4;i++)
					{
						if(i!=f.second)
							trivertice[verindex++]=i;
					}

					for(int i=0;i<3;i++)
					{//��ÿ��edge,���ڽ���dual voronoi edge������ཻ������cells�����ཻ������cells
						Edge e(c,trivertice[i%3],trivertice[(i+1)%3]);
						addrVoroFace(e,vIntersect,tri,_2cell,&p2cell->m_circulator);
					}//for(int i=0;i<3;i++)
				}//if (CGAL::object_cast<Point>(&intersecObject))
			}//if(CGAL::do_intersect(r,tri))
		}//else Ray
	}//if(!tri.is_degenerate())
}

bool  segEqual(const Segment &s1,const Segment &s2)
{
	if(s1==s2||(s1.target()==s2.source()&&s1.source()==s2.target()))
		return true;
	else
		return false;
}

bool triEqual(const Triangle &tri,const Triangle &tricir)
{
	int abc = 0;
	for (int j = 0; j < 3; j++)
	{
		for (int k = 0; k < 3; k++)
		{
			if (tri.vertex(k)==tricir.vertex(j))
				abc++;
		}

	}
	if (abc == 3)
		return true;
	else
		return false;
}

void FCCR::addrVoroFace(const Edge &e,int vIntersect,const Triangle &tri,int _2cell,CircuAndTri* pTrcirculator)
{
	Delaunay::Facet_circulator  facet_cir ;
	Cell_handle c=e.first;
	int estart=e.second;
	int eend=e.third;
	facet_cir= T.incident_facets (e);
	//��ߵ�����
	Point ci=c->vertex(estart)->point();
	Point ci1=c->vertex(eend)->point();
	Point cvIntersect(m_FlowComplex->m_0cells[vIntersect].m_x,m_FlowComplex->m_0cells[vIntersect].m_y,m_FlowComplex->m_0cells[vIntersect].m_z);
	CP_Point3D pci(to_double(ci.hx()), to_double(ci.hy()), to_double(ci.hz()));
	CP_Point3D pci1(to_double(ci1.hx()), to_double(ci1.hy()), to_double(ci1.hz()));

	Delaunay::Facet_circulator done(facet_cir) ; 
	Triangle tric(c->vertex(estart)->point(),c->vertex(eend)->point(),cvIntersect);

	int ciindex=m_FlowComplex->LocatePoint(pci);
	int ci1index=m_FlowComplex->LocatePoint(pci1);
	int num=0;
	int number=0;
	//narrowly miss������������
	bool narrowmis=false;
	int _2cellmis=-1;
	int MaxEdgeIndexmis=-1;
	Cell_handle cmis;
	Triangle trimis;
	int trivermis[3]={-1,-1,-1};
	vector<CP_LineSegment3D> circulator;
	CP_Triganle3D tricirculator;
	do
	{
		if(!T.is_infinite(*facet_cir)&&!tric.is_degenerate())
		{
			Triangle tricir=T.triangle(*facet_cir);
			if(!triEqual(tri,tricir)&&obtusetri(tricir)){
				CGAL::Object tcir;
				tcir=T.dual(*facet_cir);
				if (CGAL::object_cast<Segment>(&tcir))
				{
					Segment scir;
					assign(scir, tcir);
					if(!scir.is_degenerate())
					{
						if(CGAL::do_intersect(scir,tric))
						{
							CGAL::Object intersecObject=CGAL::intersection(scir,tric);
							if (CGAL::object_cast<Point>(&intersecObject))
							{
								deleteTriangle(m_FlowComplex->non_gabriel_triangles,tricir);
								CP_Point3D p0(to_double(tricir.vertex(0).hx()), to_double(tricir.vertex(0).hy()), to_double(tricir.vertex(0).hz()));
								CP_Point3D p1(to_double(tricir.vertex(1).hx()), to_double(tricir.vertex(1).hy()), to_double(tricir.vertex(1).hz()));
								CP_Point3D p2(to_double(tricir.vertex(2).hx()), to_double(tricir.vertex(2).hy()), to_double(tricir.vertex(2).hz()));
								int v0=m_FlowComplex->LocatePoint(p0);
								int v1=m_FlowComplex->LocatePoint(p1);
								int v2=m_FlowComplex->LocatePoint(p2);
								CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);
								//�����µ�critical point
								Point intersec;
								assign(intersec, intersecObject);
								CP_Point3D pIntersec(to_double(intersec.hx()), to_double(intersec.hy()), to_double(intersec.hz()));
								
								m_FlowComplex->m_0cells.push_back(pIntersec);
								int vIntersectp=m_FlowComplex->m_0cells.size()-1;
								
								//����chevron
								CP_Triganle3D *pTriangle0 =new CP_Triganle3D(ciindex, vIntersect, vIntersectp);
								CP_Triganle3D *pTriangle1 =new CP_Triganle3D(ci1index, vIntersect, vIntersectp);

								m_FlowComplex->tricells.push_back(pTriangle0);
								m_FlowComplex->tricells.push_back(pTriangle1);
								pTriangle0->_2cell=_2cell;
								pTriangle1->_2cell=_2cell;

								num++;
								//�ҳ�����������������
								int trivertice[3]={0};
								int verindex=0;
								for(int i=0;i<4;i++)
								{
									if(i!=facet_cir->second)
										trivertice[verindex++]=i;
								}

								////�ҳ����
								double lenMax=0;
								int lenMaxEdgeIndex=0;
								for(int i=0;i<3;i++)
								{//��ÿ��edge,���ڽ���dual voronoi edge������ཻ������cells�����ཻ������cells
									Segment stemp=T.segment(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
									double len= to_double(stemp.squared_length());
									if(len>lenMax){
										lenMaxEdgeIndex=i;lenMax=len;
									}
								}//for(int i=0;i<3;i++)

								CP_2cell *p2cell=new CP_2cell();
								m_FlowComplex->m_2cells.push_back(p2cell);
								p2cell->p_critical=vIntersectp;
								int _2cellcir=m_FlowComplex->m_2cells.size()-1;
								p2cell->index=_2cellcir;
								p2cell->pTri=NULL;
								p2cell->distance=dist(pIntersec,pci);
								p2cell->delaunytri.push_back(m_FlowComplex->LocateTriangle(m_FlowComplex->delauny2cells,*pTriangle));

								for(int i=0;i<3;i++)
								{//��ÿ��edge,���ڽ���dual voronoi edge������ཻ������cells�����ཻ������cells
									if(i!=lenMaxEdgeIndex)
									{
										Edge ecir(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
										addrVoroFace(ecir,vIntersectp,tricir,_2cellcir,NULL);
									}	
								}//for(int i=0;i<3;i++)
							}//if (CGAL::object_cast<Point>(&intersecObject))
						}//if(CGAL::do_intersect(scir,tricir))
						else
						{//û���ཻ��ԭ������Ǿ������⵼��tricir̫С���������ڵ�tri�ĺ���ཻ�������˶����ǹ�������
							if(obtusetri(tricir)&&CGAL::do_intersect(scir,tri))
							{
								//������ڱ�����ߣ��������θü���
								int trivertice[3]={0};
								int verindex=0;
								for(int i=0;i<4;i++)
								{
									if(i!=facet_cir->second)
										trivertice[verindex++]=i;
								}

								////�ҳ����
								double lenMax=0;
								int lenMaxEdgeIndex=0;
								for(int i=0;i<3;i++)
								{//��ÿ��edge,���ڽ���dual voronoi edge������ཻ������cells�����ཻ������cells
									Segment stemp=T.segment(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
									double len= to_double(stemp.squared_length());
									if(len>lenMax){
										lenMaxEdgeIndex=i;lenMax=len;
									}
								}//for(int i=0;i<3;i++)
								if(segEqual(T.segment(e),T.segment(facet_cir->first,trivertice[lenMaxEdgeIndex%3],trivertice[(lenMaxEdgeIndex+1)%3])))
								{
									narrowmis=true;
									//num++;//narrowmis�൱��num++
									MaxEdgeIndexmis=lenMaxEdgeIndex;
									cmis=facet_cir->first;
									trimis=tricir;
									for(int i=0;i<3;i++)
										trivermis[i]=trivertice[i];
								}//segequal
							}//obtuse
						}//intersect else
					}//if(!scir.is_degenerate())
				}else
				{
					Ray rcir;
					assign(rcir, tcir);
					if(CGAL::do_intersect(rcir,tric))
					{
						CGAL::Object intersecObject=CGAL::intersection(rcir,tric);
						if (CGAL::object_cast<Point>(&intersecObject))
						{
							deleteTriangle(m_FlowComplex->non_gabriel_triangles,tricir);
							CP_Point3D p0(to_double(tricir.vertex(0).hx()), to_double(tricir.vertex(0).hy()), to_double(tricir.vertex(0).hz()));
							CP_Point3D p1(to_double(tricir.vertex(1).hx()), to_double(tricir.vertex(1).hy()), to_double(tricir.vertex(1).hz()));
							CP_Point3D p2(to_double(tricir.vertex(2).hx()), to_double(tricir.vertex(2).hy()), to_double(tricir.vertex(2).hz()));
							int v0=m_FlowComplex->LocatePoint(p0);
							int v1=m_FlowComplex->LocatePoint(p1);
							int v2=m_FlowComplex->LocatePoint(p2);
							CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);
							//�����µ�critical point
							Point intersec;
							assign(intersec, intersecObject);
							CP_Point3D pIntersec(to_double(intersec.hx()), to_double(intersec.hy()), to_double(intersec.hz()));
							
							m_FlowComplex->m_0cells.push_back(pIntersec);
							int vIntersectp=m_FlowComplex->m_0cells.size()-1;

							//����chevron
							CP_Triganle3D *pTriangle0 =new CP_Triganle3D(ciindex, vIntersect, vIntersectp);
							CP_Triganle3D *pTriangle1 =new CP_Triganle3D(ci1index, vIntersect, vIntersectp);
						
							m_FlowComplex->tricells.push_back(pTriangle0);
							m_FlowComplex->tricells.push_back(pTriangle1);
							pTriangle0->_2cell=_2cell;
							pTriangle1->_2cell=_2cell;

							num++;
							//�ҳ�����������������
							int trivertice[3]={0};
							int verindex=0;
							for(int i=0;i<4;i++)
							{
								if(i!=facet_cir->second)
									trivertice[verindex++]=i;
							}

							////�ҳ����
							double lenMax=0;
							int lenMaxEdgeIndex=0;
							for(int i=0;i<3;i++)
							{//��ÿ��edge,���ڽ���dual voronoi edge������ཻ������cells�����ཻ������cells
								Segment stemp=T.segment(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
								double len= to_double(stemp.squared_length());
								if(len>lenMax){
									lenMaxEdgeIndex=i;lenMax=len;
								}
							}//for(int i=0;i<3;i++)

							CP_2cell *p2cell=new CP_2cell();
							m_FlowComplex->m_2cells.push_back(p2cell);
							p2cell->p_critical=vIntersectp;
							int _2cellcir=m_FlowComplex->m_2cells.size()-1;
							p2cell->index=_2cellcir;
							p2cell->pTri=NULL;
							p2cell->distance=dist(pIntersec,pci);
							p2cell->delaunytri.push_back(m_FlowComplex->LocateTriangle(m_FlowComplex->delauny2cells,*pTriangle));

							for(int i=0;i<3;i++)
							{//��ÿ��edge,���ڽ���dual voronoi edge������ཻ������cells�����ཻ������cells
								if(i!=lenMaxEdgeIndex)
								{
									Edge ecir(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
									addrVoroFace(ecir,vIntersectp,tricir,_2cellcir,NULL);
								}	
							}//for(int i=0;i<3;i++)
						}//if (CGAL::object_cast<Point>(&intersecObject))
					}//if(CGAL::do_intersect(rcir,tricir))
					else
					{//
						if(obtusetri(tricir)&&CGAL::do_intersect(rcir,tri))
						{
							//������ڱ�����ߣ��������θü���
							int trivertice[3]={0};
							int verindex=0;
							for(int i=0;i<4;i++)
							{
								if(i!=facet_cir->second)
									trivertice[verindex++]=i;
							}

							////�ҳ����
							double lenMax=0;
							int lenMaxEdgeIndex=0;
							for(int i=0;i<3;i++)
							{//��ÿ��edge,���ڽ���dual voronoi edge������ཻ������cells�����ཻ������cells
								Segment stemp=T.segment(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
								double len= to_double(stemp.squared_length());
								if(len>lenMax){
									lenMaxEdgeIndex=i;lenMax=len;
								}
							}//for(int i=0;i<3;i++)
							if(segEqual(T.segment(e),T.segment(facet_cir->first,trivertice[lenMaxEdgeIndex%3],trivertice[(lenMaxEdgeIndex+1)%3])))
							{
								narrowmis=true;
								//num++;
								MaxEdgeIndexmis=lenMaxEdgeIndex;
								cmis=facet_cir->first;
								trimis=tricir;
								for(int i=0;i<3;i++)
									trivermis[i]=trivertice[i];
							}//segequal
						}//obtuse
					}//intersect else
				}//else Ray
			}//if(!triEqual(tri,tricir))
		}//if(T.is_infinite(*facet_cir))
	}while(++facet_cir!=done);//��Voronoi�����ֻ��һ����������

	if(num==0&&narrowmis)
	{
		deleteTriangle(m_FlowComplex->non_gabriel_triangles,trimis);
		CP_Point3D p0(to_double(trimis.vertex(0).hx()), to_double(trimis.vertex(0).hy()), to_double(trimis.vertex(0).hz()));
		CP_Point3D p1(to_double(trimis.vertex(1).hx()), to_double(trimis.vertex(1).hy()), to_double(trimis.vertex(1).hz()));
		CP_Point3D p2(to_double(trimis.vertex(2).hx()), to_double(trimis.vertex(2).hy()), to_double(trimis.vertex(2).hz()));
		int v0=m_FlowComplex->LocatePoint(p0);
		int v1=m_FlowComplex->LocatePoint(p1);
		int v2=m_FlowComplex->LocatePoint(p2);
		CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);

		CP_Point3D pIntersec((pci.m_x+pci1.m_x)/2, (pci.m_y+pci1.m_y)/2, (pci.m_z+pci1.m_z)/2);

		m_FlowComplex->m_0cells.push_back(pIntersec);
		int vIntersectp=m_FlowComplex->m_0cells.size()-1;

		//����chevron
		CP_Triganle3D *pTriangle0 =new CP_Triganle3D(ciindex, vIntersect, vIntersectp);
		CP_Triganle3D *pTriangle1 =new CP_Triganle3D(ci1index, vIntersect, vIntersectp);

		m_FlowComplex->tricells.push_back(pTriangle0);
		m_FlowComplex->tricells.push_back(pTriangle1);
		pTriangle0->_2cell=_2cell;
		pTriangle1->_2cell=_2cell;

		num++;
		CP_2cell *p2cell=new CP_2cell();
		m_FlowComplex->m_2cells.push_back(p2cell);
		p2cell->p_critical=vIntersectp;
		int _2cellcir=m_FlowComplex->m_2cells.size()-1;
		p2cell->index=_2cellcir;
		p2cell->pTri=NULL;
		p2cell->distance=dist(pIntersec,pci);
		p2cell->delaunytri.push_back(m_FlowComplex->LocateTriangle(m_FlowComplex->delauny2cells,*pTriangle));

		for(int i=0;i<3;i++)
		{//��ÿ��edge,���ڽ���dual voronoi edge������ཻ������cells�����ཻ������cells
			if(i!=MaxEdgeIndexmis)
			{
				Edge ecir(cmis,trivermis[i%3],trivermis[(i+1)%3]);
				addrVoroFace(ecir,vIntersectp,trimis,_2cellcir,NULL);
			}	
		}//for(int i=0;i<3;i++)
	}else if(num==0&&!narrowmis)
	{
		CP_Triganle3D *pTriangle =new CP_Triganle3D(ciindex, ci1index, vIntersect);
		m_FlowComplex->tricells.push_back(pTriangle);
		pTriangle->_2cell=_2cell;
	}
}

void FCCR::deleteTriangle(vector<CP_Triganle3D*> &non_gabriel_triangles,const Triangle &tri)
{
	CP_Point3D p0(to_double(tri.vertex(0).hx()), to_double(tri.vertex(0).hy()), to_double(tri.vertex(0).hz()));
	CP_Point3D p1(to_double(tri.vertex(1).hx()), to_double(tri.vertex(1).hy()), to_double(tri.vertex(1).hz()));
	CP_Point3D p2(to_double(tri.vertex(2).hx()), to_double(tri.vertex(2).hy()), to_double(tri.vertex(2).hz()));
	int v0=m_FlowComplex->LocatePoint(p0);
	int v1=m_FlowComplex->LocatePoint(p1);
	int v2=m_FlowComplex->LocatePoint(p2);
	CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);
	//�������non_gabriel_triangles�а���tri����ɾ��
	for (unsigned int i=0;i<non_gabriel_triangles.size();i++)
	{
		unsigned int num=0;
		for(unsigned int m=0;m<3;m++)
		{
			for(unsigned int n=0;n<3;n++)
			{
				if(non_gabriel_triangles[i]->m_points[m]==pTriangle->m_points[n])
				{
					num++;
				}
			}//n
		}//m
		if(num==3)
		{
			non_gabriel_triangles.erase(non_gabriel_triangles.begin()+i);
			break;
		}
	}//i
}

bool FCCR::obtusetri(const Triangle &tri)
{
	CP_Point3D p[3];
	p[0].m_x=to_double(tri.vertex(0).hx());
	p[0].m_y=to_double(tri.vertex(0).hy());
	p[0].m_z=to_double(tri.vertex(0).hz());

	p[1].m_x=to_double(tri.vertex(1).hx());
	p[1].m_y=to_double(tri.vertex(1).hy());
	p[1].m_z=to_double(tri.vertex(1).hz());

	p[2].m_x=to_double(tri.vertex(2).hx());
	p[2].m_y=to_double(tri.vertex(2).hy());
	p[2].m_z=to_double(tri.vertex(2).hz());
	for(int i=0;i<3;i++)
	{
		CP_Vector3D i1(p[(i+1)%3].m_x-p[i%3].m_x,p[(i+1)%3].m_y-p[i%3].m_y,p[(i+1)%3].m_z-p[i%3].m_z);
		CP_Vector3D i2(p[(i+2)%3].m_x-p[i%3].m_x,p[(i+2)%3].m_y-p[i%3].m_y,p[(i+2)%3].m_z-p[i%3].m_z);
		if(i1*i2<=1e-5)
			return true;
	}
	return false;
}


int ExistLineSeg(const vector<CurveSegment> &lvec,const CurveSegment &l)
{
	for(unsigned int i=0;i<lvec.size();i++)
	{
		if(lvec[i].sp==l.sp&&lvec[i].ep==l.ep)
			return i;
		else if(lvec[i].sp==l.ep&&lvec[i].ep==l.sp)
			return i;
	}
	return -1;
}

bool distanceCmp(CP_2cell* l,CP_2cell* r)
{
	return l->distance<r->distance;
}

bool typeCmp(CP_2cell* l,CP_2cell* r)
{
	return l->type<r->type;
}

bool dis3cellCmp(CP_3cell* l,CP_3cell* r)
{
	return l->dis3cell<r->dis3cell;
	//return l->persistence<r->persistence;
}

void FCCR::ToFlowcomplex()
{
	//Delauny���й���Flow Complex�ṹ�����¼��㷨��
	Facets_iterator fit;
	Facet f;
	for(fit = T.finite_facets_begin(); fit != T.finite_facets_end(); ++fit)
	{
		f.first=fit->first;
		f.second=fit->second;
		addrFacet(f);
	}

	//���ڼ���3cell����ĵ�
	Cells_iterator cit;
	Tetrahedron tet;
	CircumPoint cirp;
	CP_Point3D p;
	for(cit=T.finite_cells_begin();cit!=T.finite_cells_end();++cit)
	{
		tet=T.tetrahedron(cit);
		cirp.SetMember(to_double(cit->circumcenter().hx()),to_double(cit->circumcenter().hy()),to_double(cit->circumcenter().hz()),to_double(tet.volume()));
		if((cirp.m_x>=m_FlowComplex->minx)&&(cirp.m_x<=m_FlowComplex->maxx)&&(cirp.m_y>=m_FlowComplex->miny)&&(cirp.m_y<=m_FlowComplex->maxy)&&(cirp.m_z>=m_FlowComplex->minz)&&(cirp.m_z<=m_FlowComplex->maxz))
		{
			p.m_x=to_double(tet.vertex(0).hx()); 
			p.m_y=to_double(tet.vertex(0).hy()); 
			p.m_z=to_double(tet.vertex(0).hz());
			cirp.SetDistance(dist(p,cirp));
			m_FlowComplex->m_circums.push_back(cirp);
			//cout<<m_FlowComplex->m_circums[m_FlowComplex->m_circums.size()-1].distance<<endl;
		}
	}
	cout<<"cir:"<<m_FlowComplex->m_circums.size()<<endl;

	//��tricell����2cells
	for (unsigned int i = 0; i < m_FlowComplex->tricells.size(); i++)
	{
		m_FlowComplex->m_2cells[m_FlowComplex->tricells[i]->_2cell]->m_triangle.push_back(i);
	}
	m_FlowComplex->SetTriangleBound();//Ϊÿ��triangle�����Χ�У������ж�vorocircum���Ƿ���3cell�еļ���

	////����ÿ��2-cell�ı߽�
	vector<CurveSegment* > lvec;
	for (unsigned int i = 0; i < m_FlowComplex->m_2cells.size(); i++)
	{//2-cell�߽�
		for(unsigned int j=0;j<m_FlowComplex->m_2cells[i]->m_triangle.size();j++)
		{
			CP_Triganle3D* ptri=m_FlowComplex->tricells[m_FlowComplex->m_2cells[i]->m_triangle[j]];
			for(int k=0;k<3;k++)
			{
				CurveSegment* l=new CurveSegment(ptri->m_points[k%3],ptri->m_points[(k+1)%3]);
				l->_triangle=m_FlowComplex->m_2cells[i]->m_triangle[j];
				int lindex=m_FlowComplex->LocateSegment(lvec,*l);
				if(lindex!=-1)
				{
					delete l;
					lvec.erase(lvec.begin()+lindex);
				}else
					lvec.push_back(l);
			}//k
		}//j
		
		for(unsigned int j=0;j<lvec.size();j++)
			m_FlowComplex->m_2cells[i]->m_boundary.push_back(*lvec[j]);
		vector<CurveSegment* >().swap(lvec);
	}//i

	//��distance��С����ӵ�������
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.end(),distanceCmp);
	//decide creator and destoryer
	m_FlowComplex->SetCreatorAndDestoryer();
	m_FlowComplex->_3cellN=m_FlowComplex->m_3cells.size();
	m_FlowComplex->Reset2cellVisited();

	int desN=0;
	for (unsigned int i = 0; i <m_FlowComplex->m_2cells.size(); i++)
	{
		CP_2cell *p2cell=m_FlowComplex->m_2cells[i];
		if(p2cell->type==0)
			desN++;
	}
	m_FlowComplex->desN=desN;
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.end(),typeCmp);

	//����3cell�Ĵ�С
	m_FlowComplex->Calculate3cellvolume();
	
	//2-cells��distance����
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.begin()+desN,distanceCmp);
	sort(m_FlowComplex->m_2cells.begin()+desN,m_FlowComplex->m_2cells.end(),distanceCmp);//��dis3cell���������

	//creator ordered by the persistance/ѡ������
	m_FlowComplex->Set3cellDistance();//m3cell��distance cicurm����������Сֵ,������persistence
	sort(m_FlowComplex->m_3cells.begin(),m_FlowComplex->m_3cells.end(),dis3cellCmp);

	vector<int> v;
	for(unsigned int i=0;i<m_FlowComplex->m_3cells.size();i++)
	{//Ϊ����ʾ3cellsʱ����
		for(unsigned int j=0;j<m_FlowComplex->m_3cells[i]->m_2cells.size();j++)
		{
			v.push_back(m_FlowComplex->Locate2cell(m_FlowComplex->m_3cells[i]->m_2cells[j]));
		}
		v.swap(m_FlowComplex->m_3cells[i]->m_2cells);
		vector<int>().swap(v);
	}

	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->SetAdjTriangle();
	m_FlowComplex->Set2cellNormalConsensus();
}

void FCCR::OnCollapse()
{//�����ع�
	//����voids��3cell
	for(unsigned int i=0;i<m_FlowComplex->m_3cells.size();i++)
	{
		CP_3cell *p3cell=m_FlowComplex->m_3cells[i];
		if(i<m_FlowComplex->m_3cells.size()-voids)
		{
			m_FlowComplex->m_2cells[p3cell->m_2cells[0]]->flag=false;
		}else
		{//��Ӧ��3cell������������circum flagΪtrue
			p3cell->flag=true;
			for(unsigned int j=0;j<p3cell->m_circums.size();j++)
			{
				m_FlowComplex->m_circums[p3cell->m_circums[j]].flag=true;
			}//j
		}
	}//i
	//��creator֮��Ķ�����1cell
	for(unsigned int i=0;i<m_FlowComplex->m_2cells.size();i++)
	{
		if(m_FlowComplex->m_2cells[i]->flag)
		{
			m_FlowComplex->Insert2cellInto1cell(*m_FlowComplex->m_2cells[i]);
		}//if type
	}//i

	for (unsigned int i = 0; i < m_FlowComplex->m_1cells.size(); i++)
	{
		if(m_FlowComplex->m_1cells[i]->degree == 1&&m_FlowComplex->m_1cells[i]->isBoundary==-1)
			collapse(*m_FlowComplex->m_1cells[i]);

		//����destoryerë�ߺͳ����creator 3cell��destoryer
	}//collapse���Ӧ������Ӧ�ı�

	for(unsigned int i=0;i<m_FlowComplex->m_2cells.size();i++)
	{
		if(m_FlowComplex->m_2cells[i]->flag)
		{
			m_FlowComplex->topo.push_back(i);
		}//if type
	}//i
}


void FCCR::collapse(const CurveSegment &curve)
{
	//��ʹ��curve���ڵ�Ψһһ��2cell��ȥ����
	int icell=curve.incident2cell[0];
	if(m_FlowComplex->m_2cells[icell]->flag)
	{
		m_FlowComplex->m_2cells[icell]->flag=false;
		//���б߽��߶ζ���-1������������=1����֦����
		for(unsigned int j=0;j<m_FlowComplex->m_2cells[icell]->m_boundary.size();j++)
		{
			int icurve=m_FlowComplex->LocateSegment(m_FlowComplex->m_1cells,m_FlowComplex->m_2cells[icell]->m_boundary[j]);
			if(m_FlowComplex->m_1cells[icurve]->degree>0)
			{
				m_FlowComplex->m_1cells[icurve]->degree--;
				m_FlowComplex->m_1cells[icurve]->incident2cell.erase(remove(m_FlowComplex->m_1cells[icurve]->incident2cell.begin(),m_FlowComplex->m_1cells[icurve]->incident2cell.end(),icell),m_FlowComplex->m_1cells[icurve]->incident2cell.end());
				if(m_FlowComplex->m_1cells[icurve]->degree==1&&m_FlowComplex->m_1cells[icurve]->isBoundary==-1)
					collapse(*m_FlowComplex->m_1cells[icurve]);
			}
		}//j
	}//2cell flag
}

void FCCR::OnThicken()
{
	if(voids>0)
	{
		//creator distance<largest persistence creator distance ��2cell creator�������⣬����creatorҲ���ǣ���paired 3cell����Thicken
		for(int i=m_FlowComplex->m_3cells.size()-1;i>=0;i--)
		{
			CP_3cell *p3cell=m_FlowComplex->m_3cells[i];
			//if(m_FlowComplex->m_2cells[p3cell->m_2cells[0]]->distance<m_FlowComplex->m_2cells[m_FlowComplex->m_3cells[m_FlowComplex->m_3cells.size()-1]->m_2cells[0]]->distance)
			if(m_FlowComplex->m_2cells[p3cell->m_2cells[0]]->distance<m_FlowComplex->m_2cells[m_FlowComplex->m_2cells.size()-1]->distance)
			{
				//�鿴�������distance��3cell��circum�Ƿ��Ѵ���
				bool exist=false;
				for(unsigned int j=0;j<p3cell->m_circums.size();j++)
				{
					if(m_FlowComplex->m_circums[p3cell->m_circums[j]].flag)
					{
						exist=true;//�Ѵ��ڣ����迼��
						break;
					}
				}//j
				if(!exist)
				{
					p3cell->flag=true;
					for(unsigned int j=0;j<p3cell->m_circums.size();j++)
						m_FlowComplex->m_circums[p3cell->m_circums[j]].flag=true;
				}
			}//distance
		}//i

		//pruning,���flag Ϊtrue����Щ3cell��2cell��flag��Ϊtrue����ptri��ΪNULL����Щ���Ƿ񶼷���pruning�ķ������,���ϵ�2cell flag=true
		for(unsigned int i=0;i<m_FlowComplex->m_3cells.size()-voids;i++)
		{
			CP_3cell *p3cell=m_FlowComplex->m_3cells[i];
			if(p3cell->flag)
			{
				bool exist=true;
				for(unsigned int j=0;j<p3cell->m_2cells.size();j++)
				{
					CP_2cell* p2cell=m_FlowComplex->m_2cells[p3cell->m_2cells[j]];
					if(!p2cell->flag)
					{//���ڲ�Ϊtrue��2ell,�����������1cell flag����Ϊtrue�����Բ����ظ� 
						if(p2cell->pTri!=NULL){
							CP_Triganle3D *pTri=p2cell->pTri;
							double f=0.0;
							CP_Vector3D nor = (m_FlowComplex->m_0cells[pTri->m_points[1]] - m_FlowComplex->m_0cells[pTri->m_points[0]]) ^ (m_FlowComplex->m_0cells[pTri->m_points[2]] -m_FlowComplex->m_0cells[pTri->m_points[0]]);
							nor.Normalize();
							for(int j=0;j<3;j++)
							{
								//������������������
								double minNT=MAX_DISTANCE;
								for(unsigned int k=0;k<m_FlowComplex->inputCurveSegments;k++)
								{
									if(m_FlowComplex->m_1cells[k]->sp==pTri->m_points[j]||m_FlowComplex->m_1cells[k]->ep==pTri->m_points[j])
									{
										CP_Vector3D tangent=m_FlowComplex->GetTangent(*m_FlowComplex->m_1cells[k]);
										tangent.Normalize();
										double tmpNT=fabs(nor*tangent);
										if(minNT>tmpNT)
											minNT=tmpNT;
									}
								}//inputcurves
								f=f+minNT;
							}//triangle 3 points
							if(f<feasa){
								p2cell->flag=true;
								m_FlowComplex->Insert2cellInto1cell(*p2cell);
							}
						}else{
							p2cell->flag=true;//����Ҳ��Ϊtrue
							m_FlowComplex->Insert2cellInto1cell(*p2cell);
						}
					}//2cell flag
				}//j
			}//3cell flag
		}//i

		for (unsigned int i = 0; i < m_FlowComplex->m_1cells.size(); i++)
		{
			if(m_FlowComplex->m_1cells[i]->degree == 1&&m_FlowComplex->m_1cells[i]->isBoundary==-1)
				collapse(*m_FlowComplex->m_1cells[i]);
			//����destoryerë�ߺͳ����creator 3cell��destoryer
		}//collapse���Ӧ������Ӧ�ı�
	
		//Ѱ��patch
		//��չ����creator
		m_FlowComplex->SeekCreatorPatch(m_FlowComplex->_3cellN-1);
	}

	//destoryer patches
	m_FlowComplex->SeekDestoryerPatch();
	m_FlowComplex->oripatches=m_FlowComplex->m_patches.size();
	
	//patch����
	m_FlowComplex->ResetTriNormalset();
	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->PatchNormalConsensus();

	m_FlowComplex->ResetTriNormalset();
	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->SetPatchNormal();
	//patch��ɫ
	//�ҵ�ÿ��patch������patch
	m_FlowComplex->SetAdjPatch();
	//��ɫ�㷨
	m_FlowComplex->ResetPatchVisited();
	m_FlowComplex->SetPatchColor();
}

bool FCCR::IsExistSharpChange(CP_PolyLine3D &poly)
{
	bool sharp=false;
	int delta=1;//1�����ǿ����и��ݵģ�̫����ܶദ����
	int pnum=static_cast<int>(poly.m_points.size());
	
	for(int i=delta;i<pnum-delta;++i)
	{
		CP_Vector3D v0=poly.m_points[i-delta]-poly.m_points[i];
		CP_Vector3D v1=poly.m_points[i+delta]-poly.m_points[i];
		double cosAlpha=v0*v1/(v0.GetLength()*v1.GetLength());
		if(cosAlpha>=-0.78){//С��140�ȣ�̫�����̫��
			sharp=true;
			poly.sharppointv.push_back(i);
		}
	}//i
	if (sharp)
		return true;
	else
 	    return false;
}

void FCCR::ImprovedFlowcomplex()
{
	//Delauny���й���Flow Complex�ṹ�����¼��㷨��
	Facets_iterator fit;
	Facet f;
	for(fit = T.finite_facets_begin(); fit != T.finite_facets_end(); ++fit)
	{
		f.first=fit->first;
		f.second=fit->second;
		addrFacet(f);
	}

	//��tricell����2cells
	for (unsigned int i = 0; i < m_FlowComplex->tricells.size(); i++)
	{
		m_FlowComplex->m_2cells[m_FlowComplex->tricells[i]->_2cell]->m_triangle.push_back(i);
	}

	////����ÿ��2-cell�ı߽�
	vector<CurveSegment> lvec;
	for (unsigned int i = 0; i < m_FlowComplex->m_2cells.size(); i++)
	{//2-cell�߽�
		for(unsigned int j=0;j<m_FlowComplex->m_2cells[i]->m_triangle.size();j++)
		{
			CP_Triganle3D* ptri=m_FlowComplex->tricells[m_FlowComplex->m_2cells[i]->m_triangle[j]];
			for(int k=0;k<3;k++)
			{
				CurveSegment l(ptri->m_points[k%3],ptri->m_points[(k+1)%3]);
				l._triangle=m_FlowComplex->m_2cells[i]->m_triangle[j];
				int lindex=ExistLineSeg(lvec,l);
				if(lindex!=-1)
				{
					lvec.erase(lvec.begin()+lindex);
				}else
					lvec.push_back(l);
			}//k
		}//j

		for(unsigned int j=0;j<lvec.size();j++)
			m_FlowComplex->m_2cells[i]->m_boundary.push_back(lvec[j]);
		vector<CurveSegment>().swap(lvec);
	}//i

	//��distance��С����ӵ�������
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.end(),distanceCmp);
	//decide creator and destoryer
	m_FlowComplex->SetCreatorAndDestoryer();
	m_FlowComplex->_3cellN=m_FlowComplex->m_3cells.size();
	m_FlowComplex->Reset2cellVisited();

	int desN=0;
	for (unsigned int i = 0; i <m_FlowComplex->m_2cells.size(); i++)
	{
		CP_2cell *p2cell=m_FlowComplex->m_2cells[i];
		if(p2cell->type==0)
			desN++;
	}
	m_FlowComplex->desN=desN;
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.end(),typeCmp);

	//2-cells��distance����
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.begin()+desN,distanceCmp);
	sort(m_FlowComplex->m_2cells.begin()+desN,m_FlowComplex->m_2cells.end(),distanceCmp);//��2cell distance���������

	vector<int> v;
	for(unsigned int i=0;i<m_FlowComplex->m_3cells.size();i++)
	{//Ϊ����ʾ3cellsʱ����
		for(unsigned int j=0;j<m_FlowComplex->m_3cells[i]->m_2cells.size();j++)
		{
			v.push_back(m_FlowComplex->Locate2cell(m_FlowComplex->m_3cells[i]->m_2cells[j]));
		}
		v.swap(m_FlowComplex->m_3cells[i]->m_2cells);
		vector<int>().swap(v);
	}

	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->SetAdjTriangle();
	m_FlowComplex->Set2cellNormalConsensus();
}

void FCCR::ImprovedThicken()
{
	//destoryer patches
	m_FlowComplex->SeekDestoryerPatch();
	m_FlowComplex->oripatches=m_FlowComplex->m_patches.size();

	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();i++)
	{
		CP_Patch *pPatch = m_FlowComplex->m_patches[i];
		for(unsigned int j=0;j<pPatch->m_boundary.size();j++)
		{
			if(pPatch->m_boundary[j]->isBoundary==-1)
			{//�߽粻���������һ���Ƿ����α�
				pPatch->wrong=true;//����չʾ�������patch
				pPatch->nonmanifoldedge++;
				pPatch->m_boundary[j]->incidentpatch.push_back(i);
			}
		}//j
	}//i
	//for(unsigned int i=m_FlowComplex->m_1cells.size()-1;i>=0;--i)
	for(unsigned int i=0;i<m_FlowComplex->m_1cells.size();i++)
	{
		int patchnum=m_FlowComplex->m_1cells[i]->incidentpatch.size();
		if(patchnum>2)
		{
			int p1=-1,p2=-1;//һ��patch�ı��
			double  dihedral=PI*1000;//����ֵ
			int nonmanifolds=MAX_DISTANCE;
			int topoOnTwoSide=0;
			for(int j=0;j<patchnum;j++){
				CP_Patch *pPatchl=m_FlowComplex->m_patches[m_FlowComplex->m_1cells[i]->incidentpatch[j]];
				while(pPatchl->merged!=pPatchl->index)
				{
					pPatchl=m_FlowComplex->m_patches[pPatchl->merged];
				}
				for (int k=j+1;k<patchnum;k++)
				{
					CP_Patch *pPatchr=m_FlowComplex->m_patches[m_FlowComplex->m_1cells[i]->incidentpatch[k]];
					while(pPatchr->merged!=pPatchr->index)
					{
						pPatchr=m_FlowComplex->m_patches[pPatchr->merged];
					}
					if(pPatchl->index!=pPatchr->index){
					
					int tmptopoOnTwoSide=m_FlowComplex->TopologicalEnable(*m_FlowComplex->m_1cells[i],*pPatchl,*pPatchr);
					if(tmptopoOnTwoSide==topoOnTwoSide)
					{
						double tmpdihedral=m_FlowComplex->DihedralOfNeighbourPatch(*m_FlowComplex->m_1cells[i],*pPatchl,*pPatchr);
						if(tmpdihedral<dihedral){
							topoOnTwoSide=tmptopoOnTwoSide;
							dihedral=tmpdihedral;
							p1=pPatchl->index;p2=pPatchr->index;
						}
					}else if(tmptopoOnTwoSide>topoOnTwoSide)
					{
						topoOnTwoSide=tmptopoOnTwoSide;
						double tmpdihedral=m_FlowComplex->DihedralOfNeighbourPatch(*m_FlowComplex->m_1cells[i],*pPatchl,*pPatchr);
						dihedral=tmpdihedral;
						p1=pPatchl->index;p2=pPatchr->index;
					}
					}
				}//k
			}//j
			//�Դ˴�1cell��patch�ϲ�
			if(dihedral>=/*(double)PI/2.0*/2.0*PI){p1=-1;p2=-1;}
			if(p1!=-1&&p2!=-1) {
				m_FlowComplex->MergePatch(*m_FlowComplex->m_1cells[i],*m_FlowComplex->m_patches[p1],*m_FlowComplex->m_patches[p2]);
			    m_FlowComplex->m_patches[p1]->dihedral=dihedral;
			}
		}//>2
	}//i 1cell
	
	vector<CP_Patch*>::iterator iter;
	for (iter=m_FlowComplex->m_patches.begin();iter!=m_FlowComplex->m_patches.end();++iter)
	{
		int interior=true;
		for(unsigned int i=0;i<(*iter)->m_boundary.size();++i)
		{
			if((*iter)->m_boundary[i]->degree<=2)
			{
				interior=false;
				break;
			}
		}
		
		if(interior)
		{//ȥ��2cell����߽�
			for (unsigned int j = 0; j <(*iter)->m_2cells.size(); j++)
			{
				CP_2cell *p2cell=m_FlowComplex->m_2cells[(*iter)->m_2cells[j]];
				p2cell->flag=false;
				for(unsigned int k=0;k<p2cell->m_boundary.size();k++){
					int icurve=m_FlowComplex->LocateSegment(m_FlowComplex->m_1cells,p2cell->m_boundary[k]);
					m_FlowComplex->m_1cells[icurve]->degree--;
					m_FlowComplex->m_1cells[icurve]->incident2cell.erase(remove(m_FlowComplex->m_1cells[icurve]->incident2cell.begin(),m_FlowComplex->m_1cells[icurve]->incident2cell.end(),(*iter)->m_2cells[j]),m_FlowComplex->m_1cells[icurve]->incident2cell.end());
				}
			}
			(*iter)->flag=false;
		}
		if((*iter)->merged!=(*iter)->index)
		    (*iter)->flag=false;
	}
	
	//�ڶ�������������creator patch
	for(int i=(int)m_FlowComplex->m_3cells.size()-1;i>=0;i--)
	{
		//wrong patch 2cell visited���ã������������
		for (unsigned int j=0;j<m_FlowComplex->m_patches.size();++j)
		{
			if(m_FlowComplex->m_patches[j]->flag&&m_FlowComplex->m_patches[j]->wrong)
			{
				CP_Patch *pPatch=m_FlowComplex->m_patches[j];
				for(unsigned int k=0;k<pPatch->m_2cells.size();++k)
				{
					m_FlowComplex->m_2cells[pPatch->m_2cells[k]]->visited=false;
				}
				pPatch->flag=false;//��ֹ���ȥ�ظ�
			}
			for(unsigned int k=0;k<m_FlowComplex->m_patches[j]->m_boundary.size();k++)
			{
				if(m_FlowComplex->m_patches[j]->m_boundary[k]->isBoundary==-1)
				{
					vector<int>().swap(m_FlowComplex->m_patches[j]->m_boundary[k]->incidentpatch);
				}
			}//k
		}//j

		CP_3cell *p3cell=m_FlowComplex->m_3cells[i];
		for(unsigned int j=0;j<p3cell->m_2cells.size();j++)
		{
			CP_2cell* p2cell=m_FlowComplex->m_2cells[p3cell->m_2cells[j]];
			if(!p2cell->flag)
			{
				p2cell->flag=true;//����Ҳ��Ϊtrue
				m_FlowComplex->Insert2cellInto1cell(*p2cell);
			}
		}
		int startidx=m_FlowComplex->m_patches.size();
		for(int j=(int)m_FlowComplex->m_3cells.size()-1;j>=i;--j)
		{
			int _patch=m_FlowComplex->m_2cells[m_FlowComplex->m_3cells[j]->m_2cells[0]]->patch;
			if(_patch==-1)//���µ�patch
				m_FlowComplex->SeekCreatorPatch(j);
			else {//��ǰ��patchδ�����棬Ҫ������
				if(m_FlowComplex->m_patches[_patch]->wrong)
					m_FlowComplex->SeekCreatorPatch(j);
			}
		}
		m_FlowComplex->SeekDestoryerPatch();

		//��¼��patches�ı���Ϣ���ںϲ�
		for(unsigned int j=startidx;j<m_FlowComplex->m_patches.size();++j){
			if(m_FlowComplex->m_patches[j]->flag){
				for(unsigned int k=0;k<m_FlowComplex->m_patches[j]->m_boundary.size();++k)
				{
					if(m_FlowComplex->m_patches[j]->m_boundary[k]->isBoundary==-1)
					{	
						m_FlowComplex->m_patches[j]->wrong=true;
						m_FlowComplex->m_patches[j]->nonmanifoldedge++;
						m_FlowComplex->m_patches[j]->m_boundary[k]->incidentpatch.push_back(j);
					}
				}//k
			}//flag
		}//j
		
		//�ϲ���patches
		for(unsigned int j=0;j<m_FlowComplex->m_1cells.size();j++)
		{
			int patchnum=m_FlowComplex->m_1cells[j]->incidentpatch.size();
			if(patchnum>2)
			{
				int p1=-1,p2=-1;//һ��patch�ı��
				double  dihedral=PI*1000;//����ֵ
				int nonmanifolds=MAX_DISTANCE;
				int topoOnTwoSide=0;
				for(int k=0;k<patchnum;k++){
					CP_Patch *pPatchl=m_FlowComplex->m_patches[m_FlowComplex->m_1cells[j]->incidentpatch[k]];
					while(pPatchl->merged!=pPatchl->index)
					{
						pPatchl=m_FlowComplex->m_patches[pPatchl->merged];
					}
					for (int m=k+1;m<patchnum;m++)
					{
						CP_Patch *pPatchr=m_FlowComplex->m_patches[m_FlowComplex->m_1cells[j]->incidentpatch[m]];
						while(pPatchr->merged!=pPatchr->index)
						{
							pPatchr=m_FlowComplex->m_patches[pPatchr->merged];
						}
						if(pPatchl->index!=pPatchr->index){
						int tmptopoOnTwoSide=m_FlowComplex->TopologicalEnable(*m_FlowComplex->m_1cells[j],*pPatchl,*pPatchr);
						if(tmptopoOnTwoSide==topoOnTwoSide)
						{
							double tmpdihedral=m_FlowComplex->DihedralOfNeighbourPatch(*m_FlowComplex->m_1cells[j],*pPatchl,*pPatchr);
							if(tmpdihedral<dihedral){
								topoOnTwoSide=tmptopoOnTwoSide;
								dihedral=tmpdihedral;
								p1=pPatchl->index;p2=pPatchr->index;
							}
						}else if(tmptopoOnTwoSide>topoOnTwoSide)
						{
							topoOnTwoSide=tmptopoOnTwoSide;
							double tmpdihedral=m_FlowComplex->DihedralOfNeighbourPatch(*m_FlowComplex->m_1cells[j],*pPatchl,*pPatchr);
							dihedral=tmpdihedral;
							p1=pPatchl->index;p2=pPatchr->index;
						}
						}
					}//m
				}//k
				//�Դ˴�1cell��patch�ϲ�
				if(dihedral>=/*(double)PI/2.0*/2.0*PI){p1=-1;p2=-1;}
				if(p1!=-1&&p2!=-1){
					m_FlowComplex->MergePatch(*m_FlowComplex->m_1cells[j],*m_FlowComplex->m_patches[p1],*m_FlowComplex->m_patches[p2]);
					m_FlowComplex->m_patches[p1]->dihedral=dihedral;
				}			
			}//>2
		}//j 1cell
		//���ÿ�ζ�����ڲ������ɾ�����������©��
	}//i 3cell

	//�����ڲ���,�Ⱥϲ���ȥ���ڲ��棬���ͬʱȥ�������ֿ���
	for(unsigned int j=0;j<m_FlowComplex->m_patches.size();++j){
		if(m_FlowComplex->m_patches[j]->flag&&m_FlowComplex->m_patches[j]->wrong){
			//���緲�Ǵ��ڵĶ����ڲ�������⣬����ȥ��Ԥ�ڱ������棨��Torso��
			int interior=true;
			for(unsigned int k=0;k<m_FlowComplex->m_patches[j]->m_boundary.size();++k)
			{
				if(m_FlowComplex->m_patches[j]->m_boundary[k]->degree<=2)
				{
					interior=false;
				}
			}
			if(interior){
				for (unsigned int k = 0;k <m_FlowComplex->m_patches[j]->m_2cells.size(); k++)
				{
					CP_2cell *p2cell=m_FlowComplex->m_2cells[m_FlowComplex->m_patches[j]->m_2cells[k]];
					p2cell->flag=false;
					for(unsigned int m=0;m<p2cell->m_boundary.size();m++){
						int icurve=m_FlowComplex->LocateSegment(m_FlowComplex->m_1cells,p2cell->m_boundary[m]);
						m_FlowComplex->m_1cells[icurve]->degree--;
						m_FlowComplex->m_1cells[icurve]->incident2cell.erase(remove(m_FlowComplex->m_1cells[icurve]->incident2cell.begin(),m_FlowComplex->m_1cells[icurve]->incident2cell.end(),m_FlowComplex->m_patches[j]->m_2cells[k]),m_FlowComplex->m_1cells[icurve]->incident2cell.end());
					}
				}
				m_FlowComplex->m_patches[j]->flag=false;
			}
		}//flag wrong
		else if(m_FlowComplex->m_patches[j]->flag/*&&!m_FlowComplex->m_patches[j]->wrong*/){
			if(m_FlowComplex->m_patches[j]->m_boundary.size()==0)//�ޱ߽統Ȼ������(Boatβ��)
			{m_FlowComplex->m_patches[j]->flag=false;}
		}//flag !wrong
	}//j
	m_FlowComplex->oripatches=m_FlowComplex->m_patches.size();

	//ɾ���ظ���
	for(unsigned int j=0;j<m_FlowComplex->m_patches.size();++j){
		if(m_FlowComplex->m_patches[j]->flag/*&&m_FlowComplex->m_patches[j]->wrong*/){//������˻�ɾ��Torso���ڹ����»���patch
			for(vector<CurveSegment*>::iterator itl=m_FlowComplex->m_patches[j]->m_boundary.begin();itl!=m_FlowComplex->m_patches[j]->m_boundary.end();)
			{
				bool dele=false;
				for(vector<CurveSegment*>::iterator itr=itl+1;itr!=m_FlowComplex->m_patches[j]->m_boundary.end();)
				{
					if((*itr)->EqualToCurvement(*itl)){
						dele=true;
						m_FlowComplex->m_patches[j]->m_boundary.erase(itr);
					}
					else
						++itr;
				}
				if(dele)
					m_FlowComplex->m_patches[j]->m_boundary.erase(itl);
				else
					++itl;
			}
		}//flag
	}//j
}

void FCCR::ImprovedFindCyclesForAllPatches()
{
	//�߽���ȫ���������߰�Χ��patch��¼darts
	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();++i){
		if(m_FlowComplex->m_patches[i]->flag&&!m_FlowComplex->m_patches[i]->wrong){
			vector<int> poly;
            for(unsigned int j=0;j<m_FlowComplex->m_patches[i]->m_boundary.size();j++){
				if(m_FlowComplex->m_patches[i]->m_boundary[j]->isBoundary!=-1
					&&find(poly.begin(),poly.end(),m_FlowComplex->m_patches[i]->m_boundary[j]->isBoundary)==poly.end())
				{
					poly.push_back(m_FlowComplex->m_patches[i]->m_boundary[j]->isBoundary);
				}
			}//j

			vector<vector<int> > pairs;
			pairs.resize(m_FlowComplex->graph.numVertexes);
			for(unsigned int j=0;j<poly.size();++j)
			{
				pairs[m_FlowComplex->m_PolyLine[poly[j]].s].push_back(poly[j]);
				pairs[m_FlowComplex->m_PolyLine[poly[j]].e].push_back(poly[j]);
			}

			for(unsigned int j=0;j<pairs.size();++j)
			{
				if((int)pairs[j].size()==2)
				{
                     m_FlowComplex->graph.adjList[j].darts.push_back(pair<int,int>(pairs[j][0],pairs[j][1]));
				}
			}

			m_FlowComplex->cycles.push_back(poly);//�߽���ȫ���������߰�Χ��patch��¼cycle
		}//flag wrong-false
	}//i

	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();++i){
		if(m_FlowComplex->m_patches[i]->flag&&!m_FlowComplex->m_patches[i]->wrong){
			for(unsigned int j=0;j<m_FlowComplex->m_patches[i]->m_2cells.size();++j){
				int _2cell=m_FlowComplex->m_patches[i]->m_2cells[j];
				for(unsigned int k=0;k<m_FlowComplex->m_2cells[_2cell]->delaunytri.size();++k){
					m_FlowComplex->delaunyexist[m_FlowComplex->m_2cells[_2cell]->delaunytri[k]]=i;}
			}//j
		}
	}//i
	
	for (unsigned int i = 0; i <m_FlowComplex->m_1cells.size(); i++)
	{
		CurveSegment* pcurve=m_FlowComplex->m_1cells[i];
		if(pcurve->isBoundary!=-1){
			pcurve->tmpdegree=0;
			pcurve->newdegree=0;
		}
		else
			pcurve->ResetDegreee();
	}
	//tmpdegree������ʱ������ֻ��patch wrong Ϊfalse��Ӱ��
	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();++i){
		if(m_FlowComplex->m_patches[i]->flag&&!m_FlowComplex->m_patches[i]->wrong){
			for(unsigned int j=0;j<m_FlowComplex->m_patches[i]->m_boundary.size();j++){
				m_FlowComplex->m_patches[i]->m_boundary[j]->tmpdegree++;
				m_FlowComplex->m_patches[i]->m_boundary[j]->newdegree++;
			}
		}
	}
	//��һ��wrong patch�ҵ�������ͨ����
	for(unsigned int i=0;i<m_FlowComplex->m_patches.size();++i){
		if(m_FlowComplex->m_patches[i]->flag&&m_FlowComplex->m_patches[i]->wrong){
			vector<int> poly;
			for(unsigned int j=0;j<m_FlowComplex->m_patches[i]->m_boundary.size();j++){
				int validDegree=m_FlowComplex->m_patches[i]->m_boundary[j]->tmpdegree;
				if(m_FlowComplex->m_patches[i]->m_boundary[j]->isBoundary!=-1&&validDegree<2//һ����ͨ�����ı߲�����������2������ 
					&&find(poly.begin(),poly.end(),m_FlowComplex->m_patches[i]->m_boundary[j]->isBoundary)==poly.end())
				{
					poly.push_back(m_FlowComplex->m_patches[i]->m_boundary[j]->isBoundary);
				}
			}//j

			GraphList  graphall;
			m_FlowComplex->BuildGraphFromCurves(poly,graphall);
			m_FlowComplex->m_patches[i]->forest=m_FlowComplex->GetConnectedComponents(graphall);
			//���ˣ��ҵ�patch�����г�ʼ��ͨ����
			//����dartsɾ����������ܲ�������ǡ�÷��ģ�͵Ļ�
			
			vector<GraphList*> newforest;
			for(unsigned int j=0;j<m_FlowComplex->m_patches[i]->forest.size();++j)
			{
				m_FlowComplex->AddTreeWithoutbranch(newforest,m_FlowComplex->m_patches[i]->forest[j]);
			}
			m_FlowComplex->m_patches[i]->forest=newforest;
			//Ϊpatch��ÿ����ͨ�����һ�
			for(unsigned int j=0;j<m_FlowComplex->m_patches[i]->forest.size();++j)
			{
				m_FlowComplex->FindShortestCycleForComponent(m_FlowComplex->m_patches[i]->cycle,*m_FlowComplex->m_patches[i]->forest[j],i);
			}
			//ȥ���ظ�

			//�ع�
			m_FlowComplex->ComputeDelaunyPatchForCycles(*m_FlowComplex->m_patches[i]);
			m_FlowComplex->m_patches[i]->flag=false;
		}//flag wrong
	}//i
	cout<<"success"<<endl;
	//patch����
	m_FlowComplex->ResetTriNormalset();
	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->PatchNormalConsensus();

	m_FlowComplex->ResetTriNormalset();
	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->SetPatchNormal();
	//patch��ɫ
	//�ҵ�ÿ��patch������patch
	m_FlowComplex->SetAdjPatch();
	//��ɫ�㷨
	m_FlowComplex->ResetPatchVisited();
	m_FlowComplex->SetPatchColor();

}
