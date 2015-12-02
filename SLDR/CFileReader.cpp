#include "StdAfx.h"
#include "CFileReader.h"
#include "CP_PointVector.h"
#include "Cp_Topology.h"
#include "CP_Curve.h"
#include "CP_TopologyExt.h"
#include "propertiesnames.h"

CFileReader::CFileReader(void)
{
}


CFileReader::~CFileReader(void)
{
}

void CFileReader::ReadDXF(LPCTSTR lpszPathName, CP_Body *pBody) {
	CStdioFile sfile;
	sfile.Open(lpszPathName,CFile::modeRead,NULL);

	CString strValue;
	int color = 0;
	CP_Edge *pEdge = NULL;

	// Get the start line
	while(sfile.ReadString(strValue)
		&& strValue.Trim().CompareNoCase(_T(DXF_START)) != 0);

	while(sfile.ReadString(strValue) && strValue.Trim().CompareNoCase(_T(DXF_END)) != 0)
	{
		if (strValue.Trim().CompareNoCase(_T(DXF_ENTITY)) == 0) {
			color = ReadColor(sfile);
		}
		if(strValue.Trim().CompareNoCase(_T(DXF_DASHED)) == 0)
		{
			pEdge = ReadLine(sfile, pBody, true);
			if (pEdge && color != DXF_COLOR_BY_BLOCK && color != DXF_COLOR_BY_LAYER)
				static_cast<CP_EdgeExt*>(pEdge)->SetProperties(PROPERTY_NAME_ADDITIONAL_LINE, 1);
		}
		if(strValue.Trim().CompareNoCase(_T(DXF_LINE)) == 0)
		{
			pEdge = ReadLine(sfile, pBody, false);
			if (pEdge && color != DXF_COLOR_BY_BLOCK && color != DXF_COLOR_BY_LAYER)
				static_cast<CP_EdgeExt*>(pEdge)->SetProperties(PROPERTY_NAME_ADDITIONAL_LINE, 1);
		}
		if(strValue.Trim().CompareNoCase(_T(DXF_ARC)) == 0)
		{
			pEdge = ReadArc(sfile, pBody);
			if (pEdge && color != DXF_COLOR_BY_BLOCK && color != DXF_COLOR_BY_LAYER)
				static_cast<CP_EdgeExt*>(pEdge)->SetProperties(PROPERTY_NAME_ADDITIONAL_LINE, 1);
		}
		if(strValue.Trim().CompareNoCase(_T(DXF_ELLIPSE)) == 0)
		{
			pEdge = ReadEllipse(sfile, pBody);
			if (pEdge && color != DXF_COLOR_BY_BLOCK && color != DXF_COLOR_BY_LAYER)
				static_cast<CP_EdgeExt*>(pEdge)->SetProperties(PROPERTY_NAME_ADDITIONAL_LINE, 1);
		}
		if(strValue.Trim().CompareNoCase(_T(DXF_SPLINE)) == 0)
		{
			pEdge = ReadSpline(sfile, pBody);
			if (pEdge && color != DXF_COLOR_BY_BLOCK && color != DXF_COLOR_BY_LAYER)
				static_cast<CP_EdgeExt*>(pEdge)->SetProperties(PROPERTY_NAME_ADDITIONAL_LINE, 1);
		}
		
	}
	sfile.Close();
	return;
}
/******************************************************************/
/*分割读到的每一行CString*/
/*Cstring是按换行符tab空格等不均匀分割的所以只能按char解决了*/
/******************************************************************/
vector<CString> CFileReader::SplitCString(CString strSource)
{
	vector <CString> vecString;
	int start = 0, end = 0;
	CString word;
	for (int i = 0; i<strSource.GetLength(); i++)
	{
		if (strSource[i] == '\t' || strSource[i] == ' '||strSource[i] == ',' || i == strSource.GetLength() - 1)
		{
			int end = i-1;
			if (i == strSource.GetLength() - 1)//i有可能是行尾index也可能是tab的index
				end = i;
			word = strSource.Mid(start, end - start+1);//substr第二个参数是截取的长度而不是终点
			if (word != "")
				vecString.push_back(word);
			start=i;
			start++;
		}

	}
	return vecString;
}

void CFileReader::ReadCURVE(LPCTSTR lpszPathName, vector<CP_PolyLine3D>* VT_PolyLine)
{
	CStdioFile sfile;
	sfile.Open(lpszPathName,CFile::modeRead,NULL);
	CString str;
	int row = 0;
	int numOfCurves = 0, degree = 3,numOfPoints=0;
	sfile.ReadString(str);
	numOfCurves = atof(str);
	CP_PolyLine3D poly;
	while (sfile.ReadString(str))
	{
		vector <CString> vecString;
		vecString = SplitCString(str);
		if (row == 0)
		{
			numOfPoints = atoi(vecString[0]);
			row++;
		}
		else if (row>0 && row<numOfPoints)
		{
			CP_Point3D p(atof(vecString[0]), atof(vecString[1]), atof(vecString[2]));
			poly.m_points.push_back(p);
			row++;
		}
		else{
			CP_Point3D p(atof(vecString[0]), atof(vecString[1]), atof(vecString[2]));
			poly.m_points.push_back(p);
			VT_PolyLine->push_back(poly);
			vector <CP_Point3D>().swap(poly.m_points);
			row = 0;
		}

	}
	sfile.Close();
	return;
}

int CFileReader::ReadColor(CStdioFile &sfile) {
	int dxfCode;
	double dxfValue;
	CString strCode, strValue;
	int color = 0;

	while(1)
	{
		sfile.ReadString(strCode);
		dxfCode = atoi(strCode.GetBuffer());
		strCode.ReleaseBuffer();
		if(dxfCode == 100 || dxfCode == 6) break;

		sfile.ReadString(strValue);
		dxfValue = atof(strValue.GetBuffer());
		strValue.ReleaseBuffer();
		switch(dxfCode)
		{
		case DXF_COLOR: color = (int)dxfValue; break;
		}
	}
	return color;
}

CP_Edge* CFileReader::ReadLine(CStdioFile &sfile, CP_Body *pBody, bool isDashed) {
	int dxfCode;
	double dxfValue;
	CString strCode, strValue;
	CP_Point3D *pStartPt = new CP_Point3D;
	CP_Point3D *pEndPt = new CP_Point3D;

	while(1)
	{
		sfile.ReadString(strCode);
		sfile.ReadString(strValue);
		dxfCode = atoi(strCode.GetBuffer());
		dxfValue = atof(strValue.GetBuffer());
		strCode.ReleaseBuffer();
		strValue.ReleaseBuffer();
		if(dxfCode == 0) break;
		switch(dxfCode)
		{
		case 10: pStartPt->m_x = dxfValue; break;
		case 20: pStartPt->m_y = dxfValue; break;
		case 30: pStartPt->m_z = dxfValue; break;
		case 11: pEndPt->m_x = dxfValue; break;
		case 21: pEndPt->m_y = dxfValue; break;
		case 31: pEndPt->m_z = dxfValue; break;
		}
	}
	if (isDashed) {
		CP_ArtificialLine * pLine = new CP_ArtificialLine(pStartPt , pEndPt, pBody);
		pBody->AddAILine(pLine);
		return NULL;
	}
	else {
		CP_LineSegment3D *pLineSeg = new CP_LineSegment3D(*pStartPt, *pEndPt);
		CP_Vertex * pVertStr =  pBody->HasVertex(pStartPt);
		if (pVertStr == NULL) {
			pVertStr = new CP_VertexExt(pStartPt);
			pBody->AddVertex(pVertStr);
		}
		
		CP_Vertex * pVertEnd = pBody->HasVertex(pEndPt);
		if (pVertEnd == NULL) {
			pVertEnd = new CP_VertexExt(pEndPt);
			pBody->AddVertex(pVertEnd);
		}

		CP_EdgeExt * pEdge = new CP_EdgeExt(pLineSeg, pVertStr, pVertEnd);
		pBody->AddEdge(pEdge);
		return pEdge;
	}
}

CP_Edge* CFileReader::ReadArc(CStdioFile &sfile, CP_Body *pBody) {
	int dxfCode;
	double dxfValue;
	CString strCode, strValue;
	CP_Point3D centerPt;
	double radius, stangle = 0, edangle = 2*PI;
	CP_Vector3D xaxis;

	while(1)
	{
		sfile.ReadString(strCode);
		sfile.ReadString(strValue);
		dxfCode = atoi(strCode.GetBuffer());
		dxfValue = atof(strValue.GetBuffer());
		strCode.ReleaseBuffer();
		strValue.ReleaseBuffer();
		if(dxfCode == 0) break;
		switch(dxfCode)
		{
		case 10: centerPt.m_x = dxfValue; break;
		case 20: centerPt.m_y = dxfValue; break;
		case 30: centerPt.m_z = dxfValue; break;
		case 11: xaxis.m_x = dxfValue; break;
		case 21: xaxis.m_y = dxfValue; break;
		case 31: xaxis.m_z = dxfValue; break;
		case 40: radius = dxfValue; break;
		case 50: stangle = dxfValue*PI/180; break;
		case 51: edangle = dxfValue*PI/180; break;
		}
	}
	CP_Arc * pArc = new CP_Arc(centerPt, xaxis, radius, radius, stangle, edangle);
	CP_Point3D *pStr = new CP_Point3D(pArc->GetPoint(0));
	CP_Point3D *pEnd = new CP_Point3D(pArc->GetPoint(1));

	CP_Vertex * pVertStr =  pBody->HasVertex(pStr);
	if (pVertStr == NULL) {
		pVertStr = new CP_VertexExt(pStr);
		pBody->AddVertex(pVertStr);
	}

	CP_Vertex * pVertEnd = pBody->HasVertex(pEnd);
	if (pVertEnd == NULL) {
		pVertEnd = new CP_VertexExt(pEnd);
		pBody->AddVertex(pVertEnd);
	}

	CP_Edge *pEdge = new CP_EdgeExt(pArc, pVertStr, pVertEnd);
	pBody->AddEdge(pEdge);
	return pEdge;
}

CP_Edge* CFileReader::ReadEllipse(CStdioFile &sfile, CP_Body *pBody) {
	int dxfCode;
	double dxfValue;
	CString strCode, strValue;
	CP_Point3D centerPt;
	CP_Vector3D xaxis;
	double ratio, stangle = 0, edangle = 2*PI;

	while(1)
	{
		sfile.ReadString(strCode);
		sfile.ReadString(strValue);
		dxfCode = atoi(strCode.GetBuffer());
		dxfValue = atof(strValue.GetBuffer());
		strCode.ReleaseBuffer();
		strValue.ReleaseBuffer();
		if(dxfCode == 0) break;
		switch(dxfCode)
		{
		case 10: centerPt.m_x = dxfValue; break;
		case 20: centerPt.m_y = dxfValue; break;
		case 30: centerPt.m_z = dxfValue; break;
		case 11: xaxis.m_x = dxfValue; break;
		case 21: xaxis.m_y = dxfValue; break;
		case 31: xaxis.m_z = dxfValue; break;
		case 40: ratio = dxfValue; break;
		case 41: stangle = dxfValue; break;
		case 42: edangle = dxfValue; break;
		}
	}
	double major = xaxis.GetLength();
	double minor = major * ratio;
	xaxis.Normalize();
	CP_Arc * pEArc = new CP_Arc(centerPt, xaxis, major, minor, stangle, edangle);
	CP_Point3D *pStr = new CP_Point3D(pEArc->GetPoint(0));
	CP_Point3D *pEnd = new CP_Point3D(pEArc->GetPoint(1));

	CP_Vertex * pVertStr =  pBody->HasVertex(pStr);
	if (pVertStr == NULL) {
		pVertStr = new CP_VertexExt(pStr);
		pBody->AddVertex(pVertStr);
	}

	CP_Vertex * pVertEnd = pBody->HasVertex(pEnd);
	if (pVertEnd == NULL) {
		pVertEnd = new CP_VertexExt(pEnd);
		pBody->AddVertex(pVertEnd);
	}
	
	CP_Edge *pEdge = new CP_EdgeExt(pEArc, pVertStr, pVertEnd);
	pBody->AddEdge(pEdge);
	return pEdge;
}

CP_Edge* CFileReader::ReadSpline(CStdioFile &sfile, CP_Body *pBody) {
	int dxfCode;
	double dxfValue;
	CString strCode, strValue;
	CP_Point3D *pCtrlPt = NULL;
	CP_Nurbs * pSpline = new CP_Nurbs();

	while(1)
	{
		sfile.ReadString(strCode);
		sfile.ReadString(strValue);
		dxfCode = atoi(strCode.GetBuffer());
		dxfValue = atof(strValue.GetBuffer());
		strCode.ReleaseBuffer();
		strValue.ReleaseBuffer();
		if(dxfCode == 0) break;
		switch(dxfCode)
		{
		case 71: pSpline->SetDegree((int)dxfValue); break;
		case 73: pSpline->SetCtrlNumber((int)dxfValue); break;
		case 40: pSpline->AddKnotValue(dxfValue); break;
		case 10: 
			pCtrlPt = new CP_Point3D;
			pCtrlPt->m_x = dxfValue;
			pSpline->AddWeightValue(1);
			break;
		case 20: pCtrlPt->m_y = dxfValue; break;
		case 30: pCtrlPt->m_z = dxfValue; 
			pSpline->AddCtrlPoint(pCtrlPt);
			break;
		case 41: pSpline->SetWeightValue(pSpline->m_ctrlArr.size()-1, dxfValue); break; 
		}
	}
	CP_Point3D* pStr = new CP_Point3D(*pSpline->GetCtrlPoint(0));
	CP_Point3D* pEnd = new CP_Point3D(*pCtrlPt);

	CP_Vertex * pVertStr =  pBody->HasVertex(pStr);
	if (pVertStr == NULL) {
		pVertStr = new CP_VertexExt(pStr);
		pBody->AddVertex(pVertStr);
	}

	CP_Vertex * pVertEnd = pBody->HasVertex(pEnd);
	if (pVertEnd == NULL) {
		pVertEnd = new CP_VertexExt(pEnd);
		pBody->AddVertex(pVertEnd);
	}

	pSpline->NormalizeKnotValue();
	CP_Edge *pEdge = new CP_EdgeExt(pSpline, pVertStr, pVertEnd);
	pBody->AddEdge(pEdge);
	return pEdge;
}