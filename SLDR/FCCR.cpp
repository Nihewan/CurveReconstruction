#include "stdafx.h"
#include "FCCR.h"
#include <iostream>
#include <fstream>
FCCR::FCCR(void)
{
	maxhd=0;
	voids=0;
	epsilon=0.055;
	feasa=0.05;
}


FCCR::~FCCR(void)
{
}

double FCCR::getHausdorffDistance(CP_PolyLine3D &curveA, CP_PolyLine3D &curveB)
{
	double maxAB = 0;//A所有点到B所有点最小值里最大的
	for (int i = 0; i < curveA.m_points.size();i++)
	{
		double min = dist(curveA.m_points[i], curveB.m_points[0]);//点i到B所有点的最小值
		for (int j = 1; j < curveB.m_points.size(); j++)
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
	for (int i = 0; i < curveB.m_points.size(); i++)
	{
		double min = dist(curveB.m_points[i], curveA.m_points[0]);//点i到A所有点的最小值
		for (int j = 1; j < curveA.m_points.size(); j++)
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
	for (int i = 0; i < m_VT_PolyLine->size(); i++)
	{
		//求曲线i与其他曲线k（k！=i）的豪斯多夫距离
		double minhd = MAX_DISTANCE;
		for (int k = 0; k < m_VT_PolyLine->size(); k++)
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
		for (int j = 0; j < (*m_VT_PolyLine)[i].m_points.size(); j++)
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
				if (dist((*m_VT_PolyLine)[i].m_points[start], (*m_VT_PolyLine)[i].m_points[j])>minhd)
				{
					poly.m_points.push_back((*m_VT_PolyLine)[i].m_points[j]);
					start = j;
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
	for (int i = 0; i < m_FlowComplex->m_PolyLine.size(); i++)
	{
		int s=0;
		for (int j = 0; j < m_FlowComplex->m_PolyLine[i].m_points.size(); j++)
		{
			num++;
			int ipoint=m_FlowComplex->LocatePoint(m_FlowComplex->m_PolyLine[i].m_points[j]);
			if (ipoint==-1)
			{
				m_FlowComplex->m_0cells.push_back(m_FlowComplex->m_PolyLine[i].m_points[j]);
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
	for(int i=0;i<m_FlowComplex->m_0cells.size();i++)
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

	for (int i = 0; i < m_FlowComplex->m_0cells.size(); i++)
	{
		Point p(m_FlowComplex->m_0cells[i].m_x, m_FlowComplex->m_0cells[i].m_y, m_FlowComplex->m_0cells[i].m_z);
		P.push_back(p);
	}
	T.insert(P.begin(), P.end());
	
	Facets_iterator fit;
	Delaunay::Finite_cells_iterator cit;
	Delaunay::Vertex_handle v;
	Delaunay::Cell_handle cell;
	for(fit = T.finite_facets_begin(); fit != T.finite_facets_end(); fit++)
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

bool FCCR::sharpBoundTri(Triangle tri)
{
	int times=0;
	CP_Point3D p0(to_double(tri.vertex(0).hx()), to_double(tri.vertex(0).hy()), to_double(tri.vertex(0).hz()));
	CP_Point3D p1(to_double(tri.vertex(1).hx()), to_double(tri.vertex(1).hy()), to_double(tri.vertex(1).hz()));
	CP_Point3D p2(to_double(tri.vertex(2).hx()), to_double(tri.vertex(2).hy()), to_double(tri.vertex(2).hz()));

	return true;
}

void FCCR::addrFacet(Facet f)
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
						bool nocover=true;
						for(int j=0;j<m_FlowComplex->ctri.size();j++)
						{
							if(!m_FlowComplex->noCover(*m_FlowComplex->ctri[j],*pTriangle))
								nocover=false;
						}
						if(nocover){
							m_FlowComplex->ctri.push_back(pTriangle);

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

						}
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
					bool nocover=true;
					for(int j=0;j<m_FlowComplex->ctri.size();j++)
					{
						if(!m_FlowComplex->noCover(*m_FlowComplex->ctri[j],*pTriangle))
							nocover=false;
					}
					if(nocover){
						m_FlowComplex->ctri.push_back(pTriangle);
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

					}
				}//if (CGAL::object_cast<Point>(&intersecObject))
			}//if(CGAL::do_intersect(r,tri))
		}//else Ray
	}//if(!tri.is_degenerate())
}

bool  segEqual(Segment s1,Segment s2)
{
	if(s1==s2||(s1.target()==s2.source()&&s1.source()==s2.target()))
		return true;
	else
		return false;
}

bool triEqual(Triangle &tri,Triangle &tricir)
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

void FCCR::addrVoroFace(Edge e,int vIntersect,Triangle tri,int _2cell)
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
	//narrowly miss的三角形数据
	bool narrowmis=false;
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

								for(int i=0;i<3;i++)
								{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
									if(i!=lenMaxEdgeIndex)
									{
										Edge ecir(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
										addrVoroFace(ecir,vIntersectp,tricir,_2cell);
									}	
								}//for(int i=0;i<3;i++)
							}//if (CGAL::object_cast<Point>(&intersecObject))
						}//if(CGAL::do_intersect(scir,tricir))
						else
						{
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
									//num++;
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

							for(int i=0;i<3;i++)
							{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
								if(i!=lenMaxEdgeIndex)
								{
									Edge ecir(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
									addrVoroFace(ecir,vIntersectp,tricir,_2cell);
								}	
							}//for(int i=0;i<3;i++)
						}//if (CGAL::object_cast<Point>(&intersecObject))
					}//if(CGAL::do_intersect(rcir,tricir))
					else
						{
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
		num++;
		for(int i=0;i<3;i++)
		{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
			if(i!=MaxEdgeIndexmis)
			{
				Edge ecir(cmis,trivermis[i%3],trivermis[(i+1)%3]);
				addrVoroFace(ecir,vIntersect,trimis,_2cell);
			}	
		}//for(int i=0;i<3;i++)
	}else if(num==0&&!narrowmis)
	{
		CP_Triganle3D *pTriangle =new CP_Triganle3D(ciindex, ci1index, vIntersect);
		m_FlowComplex->tricells.push_back(pTriangle);
		pTriangle->_2cell=_2cell;
	}
}

bool FCCR::obtusetri(Triangle &tri)
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
		if(i1*i2<0)
			return true;
	}
	return false;
}

int ExistLineSeg(vector<CurveSegment> &lvec,CurveSegment &l)
{
	for(int i=0;i<lvec.size();i++)
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

bool dis3cellCmp(CP_2cell* l,CP_2cell* r)
{
	return l->dis3cell<r->dis3cell;
}

void FCCR::ToFlowcomplex()
{
	//Delauny面中构造Flow Complex结构并重新计算法向
	Facets_iterator fit;
	for(fit = T.finite_facets_begin(); fit != T.finite_facets_end(); fit++)
	{
		Facet f(fit->first,fit->second);
		addrFacetDelauny(f);
	//	addrFacet(f);
	}

	//由tricell构造2cells
	for (int i = 0; i < m_FlowComplex->tricells.size(); i++)
	{
		m_FlowComplex->m_2cells[m_FlowComplex->tricells[i]->_2cell]->m_triangle.push_back(i);
	}
	
	//计算每个2-cell的distance function
	for (int i = 0; i < m_FlowComplex->m_2cells.size(); i++)
	{//2-cell边界
			vector<CurveSegment> lvec;
			for(int j=0;j<m_FlowComplex->m_2cells[i]->m_triangle.size();j++)
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
			for(int j=0;j<lvec.size();j++)
				m_FlowComplex->m_2cells[i]->m_boundary.push_back(lvec[j]);

			//distance function
			double dismmin=MAX_DISTANCE;
			for(int j=0;j<lvec.size();j++)
			{
				for(int k=0;k<2;k++)
				{
					if(m_FlowComplex->m_2cells[i]->p_critical!=lvec[j].GetPointIndex(k))
					{
						double dis=dist(m_FlowComplex->m_0cells[m_FlowComplex->m_2cells[i]->p_critical],m_FlowComplex->m_0cells[lvec[j].GetPointIndex(k)]);
						if(dis<dismmin)
						{
							dismmin=dis;
						}
					}
				}//k
			}//j
			m_FlowComplex->m_2cells[i]->distance=dismmin;
	}//i

	//按distance从小到大加到集合中
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.end(),distanceCmp);
	//decide creator and destoryer
	m_FlowComplex->SetCreatorAndDestoryer();
	
	int desN=0;
	for (int i = 0; i <m_FlowComplex->m_2cells.size(); i++)
	{
		CP_2cell *p2cell=m_FlowComplex->m_2cells[i];
		if(p2cell->type==0)
			desN++;
	}
	m_FlowComplex->desN=desN;
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.end(),typeCmp);

	////2-cells按distance增序
	sort(m_FlowComplex->m_2cells.begin(),m_FlowComplex->m_2cells.begin()+desN,distanceCmp);
	sort(m_FlowComplex->m_2cells.begin()+desN,m_FlowComplex->m_2cells.end(),distanceCmp);
	//为creator设置paired 3cells
	m_FlowComplex->setpaired3cells();
	m_FlowComplex->Reset2cellVisited();
	//计算3cell的大小
	for (int i = m_FlowComplex->desN; i < m_FlowComplex->m_2cells.size(); i++)
	{
		m_FlowComplex->calc3cellVolume(*m_FlowComplex->m_2cells[i]);
	}

	//直接选个最大的，不必再排序了
	sort(m_FlowComplex->m_2cells.begin()+desN,m_FlowComplex->m_2cells.end(),dis3cellCmp);
	
	for(int i=0;i<m_FlowComplex->m_3cells.size();i++)
	{//为了显示3cells时方便
		vector<int> v;
		for(int j=0;j<m_FlowComplex->m_3cells[i]->m_2cells.size();j++)
		{
			v.push_back(m_FlowComplex->Locate2cell(m_FlowComplex->m_3cells[i]->m_2cells[j]));
		}
		v.swap(m_FlowComplex->m_3cells[i]->m_2cells);
	}

	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->SetAdjTriangle();
	m_FlowComplex->_2cellNormalConsensus();
}

void FCCR::OnCollapse()
{
	for (int i = m_FlowComplex->desN; i < m_FlowComplex->m_2cells.size()-voids; i++)
	{
		m_FlowComplex->m_2cells[i]->flag=false;
	}
	//除creator之外的都加入1cell
	for(int i=0;i<m_FlowComplex->m_2cells.size();i++)
	{
		if(m_FlowComplex->m_2cells[i]->flag)
		{
			for(int j=0;j<m_FlowComplex->m_2cells[i]->m_boundary.size();j++)
			{
				int curve=m_FlowComplex->LocateSegment(m_FlowComplex->m_1cells,m_FlowComplex->m_2cells[i]->m_boundary[j]);
				if(curve==-1)
				{
					CurveSegment *c=new CurveSegment(m_FlowComplex->m_2cells[i]->m_boundary[j].GetPointIndex(0),m_FlowComplex->m_2cells[i]->m_boundary[j].GetPointIndex(1));
					m_FlowComplex->m_1cells.push_back(c);
					c->degree=1;
					c->incident2cell.push_back(i);
				}else
				{
					if(m_FlowComplex->m_1cells[curve]->degree!=-1)
					{
						m_FlowComplex->m_1cells[curve]->degree++;
					}
					m_FlowComplex->m_1cells[curve]->incident2cell.push_back(i);
				}
			}//j
		}//if type
	}//i

	//其他creator依次扩展
	for (int i = m_FlowComplex->m_2cells.size()-voids-1; i >=m_FlowComplex->desN ; i--)
	{
		for(int j=0;j<m_FlowComplex->m_2cells[i]->m_boundary.size();j++)
		{//边加入m_1cell
			int curve=m_FlowComplex->LocateSegment(m_FlowComplex->m_1cells,m_FlowComplex->m_2cells[i]->m_boundary[j]);
			m_FlowComplex->m_1cells[curve]->degree++;
			m_FlowComplex->m_1cells[curve]->incident2cell.push_back(i);
		}//j

		CP_Patch *pPatch=new CP_Patch();
		m_FlowComplex->m_cpatches.push_back(pPatch);
		pPatch->m_2cells.push_back(i);
		m_FlowComplex->expand2cell(*m_FlowComplex->m_2cells[i],m_FlowComplex->m_1cells,*pPatch);
		
		double r = (double)(rand() % 256) / 256;
		double g = (double)(rand() % 256) / 256;
		double b = (double)(rand() % 256) / 256;
		pPatch->r=r;
		pPatch->g=g;
		pPatch->b=b;

		for(int j=0;j<m_FlowComplex->m_2cells[i]->m_boundary.size();j++)
		{//重置m_1cell
			int curve=m_FlowComplex->LocateSegment(m_FlowComplex->m_1cells,m_FlowComplex->m_2cells[i]->m_boundary[j]);
			m_FlowComplex->m_1cells[curve]->degree--;
			m_FlowComplex->m_1cells[curve]->incident2cell.pop_back();
		}//j
	}//i
	//for(int i=0;i<m_FlowComplex->m_cpatches.size();i++)
	//{
	//	CP_Patch *pPatch = m_FlowComplex->m_cpatches[i];
	//	for(int j=0;j<pPatch->m_2cells.size();j++)
	//	{//重置加入的destoryer,destoryer扩展时相互之间访问2cell互斥，不能有相同2cell
	//		m_FlowComplex->m_2cells[pPatch->m_2cells[j]]->visited=false;
	//	}
	//}

	for (int i = 0; i < m_FlowComplex->m_1cells.size(); i++)
	{
		if(m_FlowComplex->m_1cells[i]->degree == 1)
			collapse(*m_FlowComplex->m_1cells[i]);
		//消除destoryer毛边和除最大creator 3cell的destoryer
	}//collapse后边应该做相应改变

	if(voids>0)
	{
		//扩展最大的creator
		CP_Patch *pPatch=new CP_Patch();
		m_FlowComplex->m_patches.push_back(pPatch);
		pPatch->m_2cells.push_back(m_FlowComplex->m_2cells.size()-1);
		//如果非封闭模型却要expand最大creator，边界没有加入导致消去destoryer面
		m_FlowComplex->expand2cell(*m_FlowComplex->m_2cells[m_FlowComplex->m_2cells.size()-1],m_FlowComplex->m_1cells,*pPatch);

		pPatch->index=0;
		for(int i=0;i<pPatch->m_2cells.size();i++)
		{
			m_FlowComplex->m_2cells[pPatch->m_2cells[i]]->patch=0;
		}

		double r = (double)(rand() % 256) / 256;
		double g = (double)(rand() % 256) / 256;
		double b = (double)(rand() % 256) / 256;
		pPatch->r=r;
		pPatch->g=g;
		pPatch->b=b;
	}

	//destoryer patches
	for (int i =m_FlowComplex->desN-1; i >= 0; i--)
	{
		if(m_FlowComplex->m_2cells[i]->flag&&!m_FlowComplex->m_2cells[i]->visited)
		{
			CP_Patch *pPatch=new CP_Patch();
			m_FlowComplex->m_patches.push_back(pPatch);
			pPatch->m_2cells.push_back(i);
			m_FlowComplex->expand2cell(*m_FlowComplex->m_2cells[i],m_FlowComplex->m_1cells,*pPatch);

			int _patch=m_FlowComplex->m_patches.size()-1;
			pPatch->index=_patch;
			for(int i=0;i<pPatch->m_2cells.size();i++)
			{
				m_FlowComplex->m_2cells[pPatch->m_2cells[i]]->patch=_patch;
			}

			double r = (double)(rand() % 256) / 256;
			double g = (double)(rand() % 256) / 256;
			double b = (double)(rand() % 256) / 256;
			pPatch->r=r;
			pPatch->g=g;
			pPatch->b=b;
		}
	}//i
	//for(int i=0;i<m_FlowComplex->m_patches.size();i++)
	//{
	//	CP_Patch *pPatch = m_FlowComplex->m_patches[i];
	//	for(int j=0;j<pPatch->m_2cells.size();j++)
	//	{//重置加入的destoryer,destoryer扩展时相互之间访问2cell互斥，不能有相同2cell
	//		m_FlowComplex->m_2cells[pPatch->m_2cells[j]]->visited=false;
	//	}
	//}
	m_FlowComplex->oripatches=m_FlowComplex->m_patches.size();

	m_FlowComplex->ResetTriNormalset();
	m_FlowComplex->Reset2cellVisited();
	//m_FlowComplex->Set2cellNormal();
}


void FCCR::collapse(CurveSegment &curve)
{
	//对使得curve存在的唯一一个2cell消去操作
	int icell=curve.incident2cell[0];
	if(m_FlowComplex->m_2cells[icell]->flag)
	{
		m_FlowComplex->m_2cells[icell]->flag=false;
		//所有边界线段度数-1并检查如果度数=1做剪枝操作
		for(int j=0;j<m_FlowComplex->m_2cells[icell]->m_boundary.size();j++)
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
		for(int i=0;i<m_FlowComplex->m_cpatches.size();i++)
		{//对每个cpatch的3cell中的flag检查是否还是true，若为false看其是否在cpatch中
			CP_Patch *pPatch = m_FlowComplex->m_cpatches[i];
			CP_2cell *p2cell=m_FlowComplex->m_2cells[pPatch->m_2cells[0]];
			bool exist=true;
			for(int j=0;j<p2cell->p3cell.size();j++)
			{
				int location=m_FlowComplex->Locate2cell(p2cell->p3cell[j]);
				if(!m_FlowComplex->m_2cells[location]->flag)
				{
					vector<int>::iterator result = find( pPatch->m_2cells.begin( ), pPatch->m_2cells.end( ), location ); //查找3
					if ( result == pPatch->m_2cells.end( ) ) //没找到
						exist=false;
				}
			}//j

			//Pruning the Thickened Reconstruction,用法向检测不去掉不在patch上的面片
			if(exist)
			{
				CP_Triganle3D *pTri=p2cell->pTri;
				double f=0.0;
				CP_Vector3D nor = (m_FlowComplex->m_0cells[pTri->m_points[1]] - m_FlowComplex->m_0cells[pTri->m_points[0]]) ^ (m_FlowComplex->m_0cells[pTri->m_points[2]] -m_FlowComplex->m_0cells[pTri->m_points[0]]);
				nor.Normalize();
				for(int j=0;j<3;j++)
				{
					//在输入线中找三个点
					double minNT=MAX_DISTANCE;
					for(int k=0;k<m_FlowComplex->inputCurves;k++)
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
					m_FlowComplex->m_patches.push_back(pPatch);
					//m_FlowComplex->m_2cells[pPatch->m_2cells[0]]->flag=true;
					int _patch=m_FlowComplex->m_patches.size()-1;
					pPatch->index=_patch;
					for(int i=0;i<pPatch->m_2cells.size();i++)
					{
						m_FlowComplex->m_2cells[pPatch->m_2cells[i]]->patch=_patch;
					}
				}
			}//exist
		}//i
	}//voids>0
	
	//patch边界
	for(int i=0;i<m_FlowComplex->m_patches.size();i++)
	{
		vector<CurveSegment> lvec;
		for(int j=0;j<m_FlowComplex->m_patches[i]->m_2cells.size();j++)
		{
			CP_2cell *p2cell = m_FlowComplex->m_2cells[m_FlowComplex->m_patches[i]->m_2cells[j]];
			for(int k=0;k<p2cell->m_boundary.size();k++)
			{
				int lindex=ExistLineSeg(lvec,p2cell->m_boundary[k]);
				if(lindex!=-1)
				{
					lvec.erase(lvec.begin()+lindex);
				}else
					lvec.push_back(p2cell->m_boundary[k]);
			}
		}//j
		for(int j=0;j<lvec.size();j++)
			m_FlowComplex->m_patches[i]->m_boundary.push_back(lvec[j]);
	}//i
	//patch法向
	m_FlowComplex->ResetTriNormalset();
	m_FlowComplex->Reset2cellVisited();
	m_FlowComplex->patchNormalConsensus();

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

void FCCR::addrFacetDelauny(Facet f)
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
						bool nocover=true;
						for(int j=0;j<m_FlowComplex->ctri.size();j++)
						{
							if(!m_FlowComplex->noCover(*m_FlowComplex->ctri[j],*pTriangle))
								nocover=false;
						}
						if(nocover){
							m_FlowComplex->ctri.push_back(pTriangle);

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

							CP_Triganle3D *pTriangletri =new CP_Triganle3D(v0, v1, v2);
							m_FlowComplex->tricells.push_back(pTriangletri);
							pTriangletri->_2cell=_2cell;
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
								addrVoroFaceDelauny(e,vIntersect,tri,_2cell);
							}//for(int i=0;i<3;i++)

						}
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
					bool nocover=true;
					for(int j=0;j<m_FlowComplex->ctri.size();j++)
					{
						if(!m_FlowComplex->noCover(*m_FlowComplex->ctri[j],*pTriangle))
							nocover=false;
					}
					if(nocover){
						m_FlowComplex->ctri.push_back(pTriangle);
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

						CP_Triganle3D *pTriangletri =new CP_Triganle3D(v0, v1, v2);
						m_FlowComplex->tricells.push_back(pTriangletri);
						pTriangletri->_2cell=_2cell;
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
							addrVoroFaceDelauny(e,vIntersect,tri,_2cell);
						}//for(int i=0;i<3;i++)

					}
				}//if (CGAL::object_cast<Point>(&intersecObject))
			}//if(CGAL::do_intersect(r,tri))
		}//else Ray
	}//if(!tri.is_degenerate())
}

void FCCR::addrVoroFaceDelauny(Edge e,int vIntersect,Triangle tri,int _2cell)
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
	//narrowly miss的三角形数据
	bool narrowmis=false;
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

								CP_Point3D p0(to_double(tricir.vertex(0).hx()), to_double(tricir.vertex(0).hy()), to_double(tricir.vertex(0).hz()));
								CP_Point3D p1(to_double(tricir.vertex(1).hx()), to_double(tricir.vertex(1).hy()), to_double(tricir.vertex(1).hz()));
								CP_Point3D p2(to_double(tricir.vertex(2).hx()), to_double(tricir.vertex(2).hy()), to_double(tricir.vertex(2).hz()));
								int v0=m_FlowComplex->LocatePoint(p0);
								int v1=m_FlowComplex->LocatePoint(p1);
								int v2=m_FlowComplex->LocatePoint(p2);
								CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);
								//加入chevron
								//CP_Triganle3D *pTriangle0 =new CP_Triganle3D(ciindex, vIntersect, vIntersectp);
								//CP_Triganle3D *pTriangle1 =new CP_Triganle3D(ci1index, vIntersect, vIntersectp);

								m_FlowComplex->tricells.push_back(pTriangle);
								//m_FlowComplex->tricells.push_back(pTriangle1);
								pTriangle->_2cell=_2cell;
								//pTriangle1->_2cell=_2cell;

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

								for(int i=0;i<3;i++)
								{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
									if(i!=lenMaxEdgeIndex)
									{
										Edge ecir(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
										addrVoroFaceDelauny(ecir,vIntersectp,tricir,_2cell);
									}	
								}//for(int i=0;i<3;i++)
							}//if (CGAL::object_cast<Point>(&intersecObject))
						}//if(CGAL::do_intersect(scir,tricir))
						else
						{
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
									CP_Point3D p0(to_double(tricir.vertex(0).hx()), to_double(tricir.vertex(0).hy()), to_double(tricir.vertex(0).hz()));
									CP_Point3D p1(to_double(tricir.vertex(1).hx()), to_double(tricir.vertex(1).hy()), to_double(tricir.vertex(1).hz()));
									CP_Point3D p2(to_double(tricir.vertex(2).hx()), to_double(tricir.vertex(2).hy()), to_double(tricir.vertex(2).hz()));
									int v0=m_FlowComplex->LocatePoint(p0);
									int v1=m_FlowComplex->LocatePoint(p1);
									int v2=m_FlowComplex->LocatePoint(p2);
									CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);
									m_FlowComplex->tricells.push_back(pTriangle);
									pTriangle->_2cell=_2cell;

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
							CP_Point3D p0(to_double(tricir.vertex(0).hx()), to_double(tricir.vertex(0).hy()), to_double(tricir.vertex(0).hz()));
							CP_Point3D p1(to_double(tricir.vertex(1).hx()), to_double(tricir.vertex(1).hy()), to_double(tricir.vertex(1).hz()));
							CP_Point3D p2(to_double(tricir.vertex(2).hx()), to_double(tricir.vertex(2).hy()), to_double(tricir.vertex(2).hz()));
							int v0=m_FlowComplex->LocatePoint(p0);
							int v1=m_FlowComplex->LocatePoint(p1);
							int v2=m_FlowComplex->LocatePoint(p2);
							CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);
							m_FlowComplex->tricells.push_back(pTriangle);
							pTriangle->_2cell=_2cell;

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

							for(int i=0;i<3;i++)
							{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
								if(i!=lenMaxEdgeIndex)
								{
									Edge ecir(facet_cir->first,trivertice[i%3],trivertice[(i+1)%3]);
									addrVoroFaceDelauny(ecir,vIntersectp,tricir,_2cell);
								}	
							}//for(int i=0;i<3;i++)
						}//if (CGAL::object_cast<Point>(&intersecObject))
					}//if(CGAL::do_intersect(rcir,tricir))
					else
					{
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
								CP_Point3D p0(to_double(tricir.vertex(0).hx()), to_double(tricir.vertex(0).hy()), to_double(tricir.vertex(0).hz()));
								CP_Point3D p1(to_double(tricir.vertex(1).hx()), to_double(tricir.vertex(1).hy()), to_double(tricir.vertex(1).hz()));
								CP_Point3D p2(to_double(tricir.vertex(2).hx()), to_double(tricir.vertex(2).hy()), to_double(tricir.vertex(2).hz()));
								int v0=m_FlowComplex->LocatePoint(p0);
								int v1=m_FlowComplex->LocatePoint(p1);
								int v2=m_FlowComplex->LocatePoint(p2);
								CP_Triganle3D *pTriangle =new CP_Triganle3D(v0, v1, v2);
								m_FlowComplex->tricells.push_back(pTriangle);
								pTriangle->_2cell=_2cell;

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
		num++;
		for(int i=0;i<3;i++)
		{//对每条edge,其邻接面dual voronoi edge与此面相交，加入cells；不相交，加入cells
			if(i!=MaxEdgeIndexmis)
			{
				Edge ecir(cmis,trivermis[i%3],trivermis[(i+1)%3]);
				addrVoroFaceDelauny(ecir,vIntersect,trimis,_2cell);
			}	
		}//for(int i=0;i<3;i++)
	}/*else if(num==0&&!narrowmis)
	{
		CP_Triganle3D *pTriangle =new CP_Triganle3D(ciindex, ci1index, vIntersect);
		m_FlowComplex->tricells.push_back(pTriangle);
		pTriangle->_2cell=_2cell;
	}*/
}