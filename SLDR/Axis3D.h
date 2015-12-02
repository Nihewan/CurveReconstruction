#pragma once

class CBoundBox2D;

class CAxis3D
{
private:
	CAxis3D(void);
	~CAxis3D(void);

public:
	static void DrawAxis(CBoundBox2D *bBox, int rotate[]);

private:
};

