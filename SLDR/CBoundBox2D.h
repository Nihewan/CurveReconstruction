#pragma once
#include "CP_Topology.h"
#include "CP_PointVector.h"

// Class CBoundBox can be used to calculate the view port, so the view can show object properly
// 
class CBoundBox2D
{
public:
	CBoundBox2D(void);
	~CBoundBox2D(void);

public:
	void		InitBoundBox(CP_Body *pBody);
	void		InitBoundBox(CP_Loop *pLoop);

	bool		Contain(const CBoundBox2D &box) const;

public:
	CP_Point3D m_centerPt;
	double m_width;
	double m_height;
	double m_rightPos;
	double m_leftPos;
	double m_bottomPos;
	double m_topPos;

private:
	void		GetExtreme(CP_Point3D *pPt);
	void		GetEdgeExtreme(CP_Edge *pEdge);
};

