#include "stdafx.h"
#include "CP_FlowComplex.h"
#include "FCCR.h"
#include <iostream>
#include <fstream>
FCCR::FCCR(void)
{
	maxhd=0;
	voids=0;
	epsilon=0.055;
	feasa=0.7;
}


FCCR::~FCCR(void)
{
}

double FCCR::getHausdorffDistance(const CP_PolyLine3D &curveA,const CP_PolyLine3D &curveB)
{
	double maxAB = 0;//A所有点到B所有点最小值里最大的
	for (unsigned int i = 0; i < curveA.m_points.size();i++)
	{
		double min = dist(curveA.m_points[i], curveB.m_points[0]);//点i到B所有点的最小值
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
	double maxBA = 0;//A所有点到B所有点最小值里最大的
	for (unsigned int i = 0; i < curveB.m_points.size(); i++)
	{
		double min = dist(curveB.m_points[i], curveA.m_points[0]);//点i到A所有点的最小值
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
	for (unsigned int i = 0; i < m_VT_PolyLine->size(); i++)
	{
		//求曲线i与其他曲线k（k！=i）的豪斯多夫距离
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
		poly.tag=false;
		for (unsigned int j = 0; j < (*m_VT_PolyLine)[i].m_points.size(); j++)
		{
			if (j == 0){
				start = 0;
				poly.m_points.push_back((*m_VT_PolyLine)[i].m_points[j]);
			}
			else if (j == (*m_VT_PolyLine)[i].m_points.size() - 1)//曲线终点不足minhd但是必须连起来
			{
				poly.m_points.push_back((*m_VT_PolyLine)[i].m_points[j]);
				m_FlowComplex->m_PolyLine.push_back(poly);
			}
			else{
				if((*m_VT_PolyLine)[i].m_points.size()<23)
				{
					if(j%2==0){
					poly.m_points.push_back((*m_VT_PolyLine)[i].m_points[j]);
					start = j;
					}
				}else{
				if (dist((*m_VT_PolyLine)[i].m_points[start], (*m_VT_PolyLine)[i].m_points[j])>minhd)
				{
					poly.m_points.push_back((*m_VT_PolyLine)[i].m_points[j]);
					start = j;
				}
				}
			}//else
		}//j
	//	std::cout << minhd << endl;
		if (minhd > maxhd)
			maxhd = minhd;
	}
//	std::cout << maxhd << endl;
	//预处理折线
	m_FlowComplex->Gabrielize();
	int num=0;
	//加入0-cells
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
				m_FlowComplex->IsBoundBox(m_FlowComplex->m_0cells[m_FlowComplex->m_0cells.size()-1]);
				if(j!=0)
				{
					CurveSegment* curve=new CurveSegment(s,m_FlowComplex->m_0cells.size()-1);
					m_FlowComplex->m_1cells.push_back(curve);
					curve->degree=-1;
				}
				s=m_FlowComplex->m_0cells.size()-1;
			}
			else
			{
				if(j!=0)
				{
					CurveSegment* curve=new CurveSegment(s,ipoint);
					m_FlowComplex->m_1cells.push_back(curve);
					curve->degree=-1;
				}
				s=ipoint;
			}
		}//j
	}//i
	m_FlowComplex->inputPoints=m_FlowComplex->m_0cells.size();
	m_FlowComplex->inputCurves=m_FlowComplex->m_1cells.size();

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

void FCCR::OnDelaunyTriangulation()
{
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
	}
}

void FCCR::addrFacet(const Facet &f)
{//此步只关注相交的面，避免了重复处理非相交面
	Cell_handle  c=f.first;
	Triangle tri=T.triangle(f);//delaunay三角形
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
				{//若相交，则进一步计算2-cells
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

						Point intersec;
						assign(intersec, intersecObject);

						CP_Point3D pIntersec(to_double(intersec.hx()), to_double(intersec.hy()), to_double(intersec.hz()));

						m_FlowComplex->m_0cells.push_back(pIntersec);
						int vIntersect=m_FlowComplex->m_0cells.size()-1; 
						CP_2cell *p2cell=new CP_2cell();
						m_FlowComplex->m_2cells.push_back(p2cell);
						p2cell->p_critical=vIntersect;
						int _2cell=m_FlowComplex->m_2cells.size()-1;
						p2cell->index=_2cell;
						p2cell->pTri=pTriangle;
						p2cell->distance=dist(pIntersec,p0);
						//另外3个顶点,方便得到EDGE
						int trivertice[3]={0};
						int verindex=0;
						for(int i=0;i<4;i++)
						{
							if(i!=f.second)
								trivertice[verindex++]=i;
						}

						for(int i=0;i<3;i++)
						{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
							Edge e(c,trivertice[i%3],trivertice[(i+1)%3]);
							addrVoroFace(e,vIntersect,tri,_2cell);
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
					
					Point intersec;
					assign(intersec, intersecObject);
					CP_Point3D pIntersec(to_double(intersec.hx()), to_double(intersec.hy()), to_double(intersec.hz()));

					m_FlowComplex->m_0cells.push_back(pIntersec);
					int vIntersect=m_FlowComplex->m_0cells.size()-1;

					CP_2cell *p2cell=new CP_2cell();
					m_FlowComplex->m_2cells.push_back(p2cell);
					p2cell->p_critical=vIntersect;
					int _2cell=m_FlowComplex->m_2cells.size()-1;
					p2cell->index=_2cell;
					p2cell->pTri=pTriangle;
					p2cell->distance=dist(pIntersec,p0);
					//另外3个顶点,方便得到EDGE
					int trivertice[3]={0};
					int verindex=0;
					for(int i=0;i<4;i++)
					{
						if(i!=f.second)
							trivertice[verindex++]=i;
					}

					for(int i=0;i<3;i++)
					{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
						Edge e(c,trivertice[i%3],trivertice[(i+1)%3]);
						addrVoroFace(e,vIntersect,tri,_2cell);
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

void FCCR::addrVoroFace(const Edge &e,int vIntersect,const Triangle &tri,int _2cell)
{
	Delaunay::Facet_circulator  facet_cir ;
	Cell_handle c=e.first;
	int estart=e.second;
	int eend=e.third;
	facet_cir= T.incident_facets (e);
	//这边的两点
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
	//narrowly miss的三角形数据
	bool narrowmis=false;
	int _2cellmis=-1;
	int MaxEdgeIndexmis=-1;
	Cell_handle cmis;
	Triangle trimis;
	int trivermis[3]={-1,-1,-1};
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
								//计算新的critical point
								Point intersec;
								assign(intersec, intersecObject);
								CP_Point3D pIntersec(to_double(intersec.hx()), to_double(intersec.hy()), to_double(intersec.hz()));
								
								m_FlowComplex->m_0cells.push_back(pIntersec);
								int vIntersectp=m_FlowComplex->m_0cells.size()-1;
								
								//加入chevron
								CP_Triganle3D *pTriangle0 =new CP_Triganle3D(ciindex, vIntersect, vIntersectp);
								CP_Triganle3D *pTriangle1 =new CP_Triganle3D(ci1index, vIntersect, vIntersectp);

								m_FlowComplex->tricells.push_back(pTriangle0);
								m_FlowComplex->tricells.push_back(pTriangle1);
								pTriangle0->_2cell=_2cell;
								pTriangle1->_2cell=_2cell;

								num++;
								//找出新三角形其余两边
								int trivertice[3]={0};
								int verindex=0;
								for(int i=0;i<4;i++)
								{
									if(i!=facet_cir->second)
										trivertice[verindex++]=i;
								}

								////找出最长边
								double lenMax=0;
								int lenMaxEdgeIndex=0;
								for(int i=0;i<3;i++)
								{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
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

								for(int i=0;i<3;i++)
								{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
									if(i!=lenMaxEdgeIndex)
									{
										Edge ecir(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
										addrVoroFace(ecir,vIntersectp,tricir,_2cellcir);
									}	
								}//for(int i=0;i<3;i++)
							}//if (CGAL::object_cast<Point>(&intersecObject))
						}//if(CGAL::do_intersect(scir,tricir))
						else
						{//
							if(obtusetri(tricir)&&CGAL::do_intersect(scir,tricir.supporting_plane()))
							{
								//如果相邻边是最长边，此三角形该加入
								int trivertice[3]={0};
								int verindex=0;
								for(int i=0;i<4;i++)
								{
									if(i!=facet_cir->second)
										trivertice[verindex++]=i;
								}

								////找出最长边
								double lenMax=0;
								int lenMaxEdgeIndex=0;
								for(int i=0;i<3;i++)
								{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
									Segment stemp=T.segment(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
									double len= to_double(stemp.squared_length());
									if(len>lenMax){
										lenMaxEdgeIndex=i;lenMax=len;
									}
								}//for(int i=0;i<3;i++)
								if(segEqual(T.segment(e),T.segment(facet_cir->first,trivertice[lenMaxEdgeIndex%3],trivertice[(lenMaxEdgeIndex+1)%3])))
								{
									narrowmis=true;
									//num++;//narrowmis相当于num++
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
							//计算新的critical point
							Point intersec;
							assign(intersec, intersecObject);
							CP_Point3D pIntersec(to_double(intersec.hx()), to_double(intersec.hy()), to_double(intersec.hz()));
							
							m_FlowComplex->m_0cells.push_back(pIntersec);
							int vIntersectp=m_FlowComplex->m_0cells.size()-1;

							//加入chevron
							CP_Triganle3D *pTriangle0 =new CP_Triganle3D(ciindex, vIntersect, vIntersectp);
							CP_Triganle3D *pTriangle1 =new CP_Triganle3D(ci1index, vIntersect, vIntersectp);
						
							m_FlowComplex->tricells.push_back(pTriangle0);
							m_FlowComplex->tricells.push_back(pTriangle1);
							pTriangle0->_2cell=_2cell;
							pTriangle1->_2cell=_2cell;

							num++;
							//找出新三角形其余两边
							int trivertice[3]={0};
							int verindex=0;
							for(int i=0;i<4;i++)
							{
								if(i!=facet_cir->second)
									trivertice[verindex++]=i;
							}

							////找出最长边
							double lenMax=0;
							int lenMaxEdgeIndex=0;
							for(int i=0;i<3;i++)
							{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
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

							for(int i=0;i<3;i++)
							{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
								if(i!=lenMaxEdgeIndex)
								{
									Edge ecir(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
									addrVoroFace(ecir,vIntersectp,tricir,_2cellcir);
								}	
							}//for(int i=0;i<3;i++)
						}//if (CGAL::object_cast<Point>(&intersecObject))
					}//if(CGAL::do_intersect(rcir,tricir))
					else
					{//
						if(obtusetri(tricir)&&CGAL::do_intersect(rcir,tricir.supporting_plane()))
						{
							//如果相邻边是最长边，此三角形该加入
							int trivertice[3]={0};
							int verindex=0;
							for(int i=0;i<4;i++)
							{
								if(i!=facet_cir->second)
									trivertice[verindex++]=i;
							}

							////找出最长边
							double lenMax=0;
							int lenMaxEdgeIndex=0;
							for(int i=0;i<3;i++)
							{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
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
	}while(++facet_cir!=done);//此Voronoi面最多只有一边满足条件

	if(num==0&&narrowmis)
	{
		CP_Point3D pIntersec((pci.m_x+pci1.m_x)/2, (pci.m_y+pci1.m_y)/2, (pci.m_z+pci1.m_z)/2);

		m_FlowComplex->m_0cells.push_back(pIntersec);
		int vIntersectp=m_FlowComplex->m_0cells.size()-1;

		//加入chevron
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

		for(int i=0;i<3;i++)
		{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
			if(i!=MaxEdgeIndexmis)
			{
				Edge ecir(cmis,trivermis[i%3],trivermis[(i+1)%3]);
				addrVoroFace(ecir,vIntersectp,trimis,_2cellcir);
			}	
		}//for(int i=0;i<3;i++)
	}else if(num==0&&!narrowmis)
	{
		CP_Triganle3D *pTriangle =new CP_Triganle3D(ciindex, ci1index, vIntersect);
		m_FlowComplex->tricells.push_back(pTriangle);
		pTriangle->_2cell=_2cell;
	}
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
		if(i1*i2<=TOL)
			return true;
	}
	return false;
}


int ExistLineSeg(const vector<CurveSegment> &lvec,CurveSegment &l)
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
}

void FCCR::ToFlowcomplex()
{
	//Delauny面中构造Flow Complex结构并重新计算法向
	Facets_iterator fit;
	Facet f;
	for(fit = T.finite_facets_begin(); fit != T.finite_facets_end(); ++fit)
	{
		f.first=fit->first;
		f.second=fit->second;
		addrFacet(f);
	}

	//用于计算3cell体积的点
	Cells_iterator cit;
	Tetrahedron tet;
	CircumPoint cirp;
	for(cit=T.finite_cells_begin();cit!=T.finite_cells_end();++cit)
	{
		tet=T.tetrahedron(cit);
		cirp.SetMember(to_double(cit->circumcenter().hx()),to_double(cit->circumcenter().hy()),to_double(cit->circumcenter().hz()),to_double(tet.volume()));
		if((cirp.m_x>=m_FlowComplex->minx)&&(cirp.m_x<=m_FlowComplex->maxx)&&(cirp.m_y>=m_FlowComplex->miny)&&(cirp.m_y<=m_FlowComplex->maxy)&&(cirp.m_z>=m_FlowComplex->minz)&&(cirp.m_z<=m_FlowComplex->maxz))
			m_FlowComplex->m_circums.push_back(cirp);
	}
	cout<<"cir:"<<m_FlowComplex->m_circums.size()<<endl;

	//由tricell构造2cells
	for (unsigned int i = 0; i < m_FlowComplex->tricells.size(); i++)
	{
		m_FlowComplex->m_2cells[m_FlowComplex->tricells[i]->_2cell]->m_triangle.push_back(i);
	}
	m_FlowComplex->SetTriangleBound();//为每个triangle计算包围盒，减少判断vorocircum点是否在3cell中的计算

	////计算每个2-cell的边界
	vector<CurveSegment> lvec;
	for (unsigned int i = 0; i < m_FlowComplex->m_2cells.size(); i++)
	{//2-cell边界
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

	//按distance从小到大加到集合中
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

	//计算3cell的大小
	m_FlowComplex->Calculate3cellvolume();

	//2-cells按distance增序
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.begin()+desN,distanceCmp);
	sort(m_FlowComplex->m_2cells.begin()+desN,m_FlowComplex->m_2cells.end(),distanceCmp);

	//creator ordered by the persistance
	sort(m_FlowComplex->m_3cells.begin(),m_FlowComplex->m_3cells.end(),dis3cellCmp);
	
	vector<int> v;
	for(unsigned int i=0;i<m_FlowComplex->m_3cells.size();i++)
	{//为了显示3cells时方便
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
{
	//保留voids个3cell
	for(unsigned int i=0;i<m_FlowComplex->m_3cells.size();i++)
	{
		CP_3cell *p3cell=m_FlowComplex->m_3cells[i];
		if(i<m_FlowComplex->m_3cells.size()-voids)
		{
			m_FlowComplex->m_2cells[p3cell->m_2cells[0]]->flag=false;
		}else
		{//对应的3cell被保留，置其circum flag为true
			p3cell->flag=true;
			for(unsigned int j=0;j<p3cell->m_circums.size();j++)
			{
				m_FlowComplex->m_circums[p3cell->m_circums[j]].flag=true;
			}//j
		}
	}//i
	//除creator之外的都加入1cell
	for(unsigned int i=0;i<m_FlowComplex->m_2cells.size();i++)
	{
		if(m_FlowComplex->m_2cells[i]->flag)
		{
			m_FlowComplex->Insert2cellInto1cell(*m_FlowComplex->m_2cells[i]);
		}//if type
	}//i

	for (unsigned int i = 0; i < m_FlowComplex->m_1cells.size(); i++)
	{
		if(m_FlowComplex->m_1cells[i]->degree == 1)
			collapse(*m_FlowComplex->m_1cells[i]);

		//消除destoryer毛边和除最大creator 3cell的destoryer
	}//collapse后边应该做相应改变

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
	//对使得curve存在的唯一一个2cell消去操作
	int icell=curve.incident2cell[0];
	if(m_FlowComplex->m_2cells[icell]->flag)
	{
		m_FlowComplex->m_2cells[icell]->flag=false;
		//所有边界线段度数-1并检查如果度数=1做剪枝操作
		for(unsigned int j=0;j<m_FlowComplex->m_2cells[icell]->m_boundary.size();j++)
		{
			int icurve=m_FlowComplex->LocateSegment(m_FlowComplex->m_1cells,m_FlowComplex->m_2cells[icell]->m_boundary[j]);
			if(m_FlowComplex->m_1cells[icurve]->degree>0)
			{
				m_FlowComplex->m_1cells[icurve]->degree--;
				m_FlowComplex->m_1cells[icurve]->incident2cell.erase(remove(m_FlowComplex->m_1cells[icurve]->incident2cell.begin(),m_FlowComplex->m_1cells[icurve]->incident2cell.end(),icell),m_FlowComplex->m_1cells[icurve]->incident2cell.end());
				if(m_FlowComplex->m_1cells[icurve]->degree==1)
					collapse(*m_FlowComplex->m_1cells[icurve]);
			}
		}//j
	}//2cell flag
}


void FCCR::OnThicken()
{
	if(voids>0)
	{
		//creator distance<largest persistence creator distance 的2cell creator和paired 3cell考虑Thicken
		for(int i=m_FlowComplex->m_3cells.size()-1;i>=0;i--)
		{
			CP_3cell *p3cell=m_FlowComplex->m_3cells[i];
			if(m_FlowComplex->m_2cells[p3cell->m_2cells[0]]->distance<m_FlowComplex->m_2cells[m_FlowComplex->m_2cells.size()-1]->distance)
			{
				//查看这个符合distance的3cell的circum是否已存在
				bool exist=false;
				for(unsigned int j=0;j<p3cell->m_circums.size();j++)
				{
					if(m_FlowComplex->m_circums[p3cell->m_circums[j]].flag)
					{
						exist=true;//已存在，不予考虑
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

		//pruning,检查flag 为true的那些3cell的2cell（flag不为true，且ptri不为NULL的那些）是否都符合pruning的法向规则,符合的2cell flag=true
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
					{//现在不为true的2ell,遍历过加入过1cell flag会置为true，所以不会重复 
						if(p2cell->pTri!=NULL){
							CP_Triganle3D *pTri=p2cell->pTri;
							double f=0.0;
							CP_Vector3D nor = (m_FlowComplex->m_0cells[pTri->m_points[1]] - m_FlowComplex->m_0cells[pTri->m_points[0]]) ^ (m_FlowComplex->m_0cells[pTri->m_points[2]] -m_FlowComplex->m_0cells[pTri->m_points[0]]);
							nor.Normalize();
							for(int j=0;j<3;j++)
							{
								//在输入线中找三个点
								double minNT=MAX_DISTANCE;
								for(unsigned int k=0;k<m_FlowComplex->inputCurves;k++)
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
							p2cell->flag=true;//其他也置为true
							m_FlowComplex->Insert2cellInto1cell(*p2cell);
						}
					}//2cell flag
				}//j
			}//3cell flag
		}//i

		for (unsigned int i = 0; i < m_FlowComplex->m_1cells.size(); i++)
		{
			if(m_FlowComplex->m_1cells[i]->degree == 1)
				collapse(*m_FlowComplex->m_1cells[i]);
			//消除destoryer毛边和除最大creator 3cell的destoryer
		}//collapse后边应该做相应改变
	
		//寻找patch
		//扩展最大的creator
		m_FlowComplex->SeekCreatorPatch();
	}

	//destoryer patches
	m_FlowComplex->SeekDestoryerPatch();
	m_FlowComplex->oripatches=m_FlowComplex->m_patches.size();
	
	//patch边界
	m_FlowComplex->GetPatchBoundary();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//TODO 目前pruning算法肯定不能消除所有3cell，往后改进中可能使用此段代码
	//m_FlowComplex->ResetTriNormalset();
	//m_FlowComplex->Reset2cellVisited();

	////patch加入新的边集，看哪些是隔板
	////patch重新寻找
	//vector<CurveSegment*> vboundary;
	//for(unsigned int i=0;i<m_FlowComplex->m_patches.size();i++)
	//{
	//	CP_Patch * pPatch=m_FlowComplex->m_patches[i];
	//	for(unsigned int j=0;j<pPatch->m_boundary.size();j++)
	//	{
	//		int curve=m_FlowComplex->LocateSegment(vboundary,pPatch->m_boundary[j]);
	//		if(curve==-1)
	//		{
	//			vboundary.push_back(&pPatch->m_boundary[j]);
	//			pPatch->m_boundary[j].degree=1;
	//			pPatch->m_boundary[j].incident2cell.push_back(i);//incident2cell 存相关的patch编号
	//		}else
	//		{
	//			vboundary[curve]->degree++;
	//			vboundary[curve]->incident2cell.push_back(i);
	//		}
	//	}//j
	//}
	//for(unsigned int i=0;i<m_FlowComplex->m_patches.size();i++)
	//{
	//	bool part=true;
	//	CP_Patch * pPatch=m_FlowComplex->m_patches[i];
	//	for(unsigned int j=0;j<pPatch->m_boundary.size();j++)
	//	{
	//		int curve=m_FlowComplex->LocateSegment(vboundary,pPatch->m_boundary[j]);
	//		if(vboundary[curve]->degree<3)
	//		{
	//			part=false;
	//			break;
	//		}
	//	}
	//	if(part){
	//		//此patch是隔板，所有2cell flag=false
	//		for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
	//		{
	//			CP_2cell *p2cell =m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
	//			p2cell->flag=false;
	//			for(unsigned int k=0;k<p2cell->m_boundary.size();k++)
	//			{
	//				int icurve=m_FlowComplex->LocateSegment(m_FlowComplex->m_1cells,p2cell->m_boundary[k]);
	//				if(m_FlowComplex->m_1cells[icurve]->degree>0)
	//				{
	//					m_FlowComplex->m_1cells[icurve]->degree--;
	//					m_FlowComplex->m_1cells[icurve]->incident2cell.erase(remove(m_FlowComplex->m_1cells[icurve]->incident2cell.begin(),m_FlowComplex->m_1cells[icurve]->incident2cell.end(),pPatch->m_2cells[j]),m_FlowComplex->m_1cells[icurve]->incident2cell.end());
	//				}
	//			}//j
	//		}
	//		for(unsigned int j=0;j<pPatch->m_boundary.size();j++)
	//		{
	//			int curve=m_FlowComplex->LocateSegment(vboundary,pPatch->m_boundary[j]);
	//			vboundary[curve]->degree--;
	//			vboundary[curve]->incident2cell.erase(remove(vboundary[curve]->incident2cell.begin(),vboundary[curve]->incident2cell.end(),i),vboundary[curve]->incident2cell.end());
	//		}//j
	//	}
	//}
	//vector<CP_Patch *>().swap(m_FlowComplex->m_patches);

	////重新构造patch
	//if(voids>0){
	//	//寻找patch
	//	//扩展最大的creator
	//	m_FlowComplex->SeekCreatorPatch();
	//}
	////destoryer patches
	//m_FlowComplex->SeekDestoryerPatch();
	//m_FlowComplex->oripatches=m_FlowComplex->m_patches.size();
	////patch边界
	//m_FlowComplex->GetPatchBoundary();

	//patch法向
	m_FlowComplex->ResetTriNormalset();
	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->PatchNormalConsensus();

	m_FlowComplex->ResetTriNormalset();
	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->SetPatchNormal();
	//patch颜色
	//找到每个patch的相邻patch
	m_FlowComplex->SetAdjPatch();
	//着色算法
	m_FlowComplex->ResetPatchVisited();
	m_FlowComplex->SetPatchColor();
}