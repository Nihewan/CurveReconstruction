#include "StdAfx.h"
#include "CBoundBox2D.h"


CBoundBox2D::CBoundBox2D(void):m_height(10), m_width(10)
{
}


CBoundBox2D::~CBoundBox2D(void)
{
}

void CBoundBox2D::InitBoundBox(CP_Body *pBody) {
	int num = pBody->GetEdgeNumber();
	CP_Point3D *pPt = pBody->GetVertex(0)->m_pPoint;
	m_rightPos = pPt->m_x, m_leftPos = pPt->m_x;
	m_topPos = pPt->m_y, m_bottomPos = pPt->m_y;

	for (int i = 0; i < num; ++i) {
		GetEdgeExtreme(pBody->GetEdge(i));
	}

	m_width = m_rightPos - m_leftPos;
	m_height = m_topPos - m_bottomPos;
	m_centerPt.m_x = (m_rightPos + m_leftPos) / 2;
	m_centerPt.m_y = (m_topPos + m_bottomPos) / 2;
}

// 计算当前Loop的BoundBox
void CBoundBox2D::InitBoundBox(CP_Loop * pLoop)
{
	CP_Point3D *pPt = pLoop->GetFirstHalf()->m_pStartVt->m_pPoint;
	m_rightPos = pPt->m_x, m_leftPos = pPt->m_x;
	m_topPos = pPt->m_y, m_bottomPos = pPt->m_y;

	int halfsize = pLoop->GetHalfNumber();
	for(int j = 0; j < halfsize; j++) {
		GetEdgeExtreme(pLoop->GetHalf(j)->m_pEdge);
	}
}

bool CBoundBox2D::Contain(const CBoundBox2D &box) const {
	return m_rightPos > box.m_rightPos && m_leftPos < box.m_leftPos
		&& m_topPos > box.m_topPos && m_bottomPos < box.m_bottomPos;
}

void CBoundBox2D::GetExtreme(CP_Point3D *pPt) {
	if (pPt->m_x > m_rightPos) m_rightPos = pPt->m_x;
	else if (pPt->m_x < m_leftPos) m_leftPos = pPt->m_x;
	if (pPt->m_y > m_topPos) m_topPos = pPt->m_y;
	else if (pPt->m_y < m_bottomPos) m_bottomPos = pPt->m_y;
}

void CBoundBox2D::GetEdgeExtreme(CP_Edge *pEdge) {
	for (double t = 0; t <= 1; t += 0.2) {
		CP_Point3D *pPt = &(pEdge->m_pCurve3D->GetPoint(t));
		GetExtreme(pPt);
	}
}

