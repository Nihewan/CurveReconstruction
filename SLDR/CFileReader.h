#pragma once
#include<vector>
using std::vector;
#define DXF_START "ENTITIES"
#define DXF_END   "ENDSEC"

#define DXF_ENTITY "AcDbEntity"

// LINE
#define DXF_LINE    "AcDbLine"
#define DXF_POINT_X 10
#define DXF_POINT_Y 20
#define DXF_POINT_Z 30
#define DXF_END_X   11
#define DXF_END_Y   21
#define DXF_END_Z   31

//ARC
#define DXF_ARC      "AcDbCircle"
#define DXF_RADIUS   40
#define DXF_STR_ANG  50
#define DXF_END_ANG  51

//ELLARC
#define DXF_ELLIPSE  "AcDbEllipse"
#define DXF_RATIO    40
//#define DXF_STR_ANG  41
//#define DXF_END_ANG  42

//SPLINE
#define DXF_SPLINE   "AcDbSpline"
#define DXF_DEGREE   71
#define DXF_KNOT_NUM 72
#define DXF_CTRL_NUM 73
#define DXF_KNOT_POT 40
#define DXF_CTRL_X   10
#define DXF_CTRL_Y   20
#define DXF_CTRL_Z   30
#define DXF_CTRL_W   41

//DASHED
#define DXF_DASHED   "DASHED"

// Line color
#define DXF_COLOR 62
#define DXF_COLOR_BY_BLOCK 0
#define DXF_COLOR_BY_LAYER 256


class CP_Body;
class CP_Edge;
class CP_PolyLine3D;

class CFileReader
{
public:
	CFileReader(void);
	~CFileReader(void);

public:
	static void ReadDXF(LPCTSTR lpszPathName, CP_Body *pBody);
	static void ReadCURVE(LPCTSTR lpszPathName, vector<CP_PolyLine3D>* VT_PolyLine);
	static vector<CString> SplitCString(CString strSource);
private:
	static int  ReadColor(CStdioFile &sfile);
	static CP_Edge* ReadLine(CStdioFile &sfile, CP_Body *pBody, bool isDashed);
	static CP_Edge* ReadArc(CStdioFile &sfile, CP_Body *pBody);
	static CP_Edge* ReadEllipse(CStdioFile &sfile, CP_Body *pBody);
	static CP_Edge* ReadSpline(CStdioFile &sfile, CP_Body *pBody);
};

