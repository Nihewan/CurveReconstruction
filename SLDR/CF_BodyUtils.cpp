#include "StdAfx.h"
#include "CF_BodyUtils.h"
#include "gl/GL.h"
#include "propertiesnames.h"


CF_BodyUtils::CF_BodyUtils(void)
{
}


CF_BodyUtils::~CF_BodyUtils(void)
{
}


bool CF_BodyUtils::ValidateBody(CP_Body* pBody) {
	bool no_error = true;

#ifdef _DEBUG
	for (int face_index = 0; face_index < pBody->GetFaceNumber(); face_index++)
	{
		CP_Face *face = pBody->GetFace(face_index);
		// 每个面的外环都不为空
		assert(face->GetLoopNumber() >= 1);

		for (int loop_index = 0; loop_index < face->GetLoopNumber(); loop_index++)
		{
			CP_Loop *loop = face->GetLoop(loop_index);

			// 每个环至少有一条半边
			assert(loop->GetHalfNumber() >= 1);

			for (int half_index = 0; half_index < loop->GetHalfNumber(); half_index++)
			{
				CP_Half *current_half = loop->GetHalf(half_index);

				// 每一条edge至少要有两条半边
				assert(current_half->m_pEdge->GetHalf(0) != NULL);
				assert(current_half->m_pEdge->GetHalf(1) != NULL);

				int next_half_index = (half_index + 1) % loop->GetHalfNumber();
				CP_Half *next_half = loop->GetHalf(next_half_index);

				CP_Vertex *current_half_end = current_half->m_pEndVt;
				CP_Vertex *next_half_start = next_half->m_pStartVt;

				// 每一个loop的half要成环
				assert(current_half_end == next_half_start);
			}
		}
	}
#endif // _DEBUG

	return no_error;
}

void CF_BodyUtils::DrawSolidBody(CP_Body* pBody) {
	CP_Face*      tf;
	CP_Surface3D* s;
	int n = pBody->GetFaceNumber( );
	for (int i = 0; i < n; i++)
	{
		tf = pBody->GetFace(i);
		s = tf->m_surface;
		if (s)
			s->DrawSolid(tf->m_sameNormalDirection);
	} 
}

void CF_BodyUtils::DrawWireframeBody(CP_Body* pBody) {
	CP_Face*      tf;
	CP_Surface3D* s;
	int n = pBody->GetFaceNumber( );
	for (int i = 0; i < n; i++)
	{
		tf = pBody->GetFace(i);
		s = tf->m_surface;
		s->DrawMesh();
	} 
}

void CF_BodyUtils::DrawEdges(CP_Body *pBody) {
	int num = pBody->GetEdgeNumber();
	CP_Edge *edge = NULL;
	for (int i = 0; i < num; ++i) {
		glPushName(i);
		edge = pBody->GetEdge(i);
		CP_EdgeExt *pEdgeExt = dynamic_cast<CP_EdgeExt*>(edge);
		if (pEdgeExt && pEdgeExt->HasProperty(PROPERTY_NAME_ADDITIONAL_LINE)) {
			continue;
		}
		edge->m_pCurve3D->Draw();
		glPopName();
		//glColor3d(1, 1, 1);
	}

	num = pBody->GetAILineNumber();
	CP_ArtificialLine *aiLine = NULL;
	glLineStipple(1, 0x0F0F);
	glEnable(GL_LINE_STIPPLE);
	for (int i = 0; i < num; ++i) {
		aiLine = pBody->GetAILine(i);
		aiLine->m_pLineSeg->Draw();
	}
	glDisable(GL_LINE_STIPPLE);
}

void CF_BodyUtils::DrawSelectedHighLight(CP_Body *pBody, CString strSelectedItem) {
	
	if(strSelectedItem.Find("VERTICES")==0 || strSelectedItem.Find("EDGES")==0 || strSelectedItem.Find("AILINES")==0 || strSelectedItem.Find("FACES")==0)
		return;

	if(strSelectedItem.Left(6).Compare("Vertex")==0||strSelectedItem.Mid(3,6).Compare("Vertex")==0)
	{
		int pos = strSelectedItem.Find(' ');
		strSelectedItem = strSelectedItem.Mid(pos+1);
		int index = atoi(strSelectedItem.GetBuffer(strSelectedItem.GetLength()));
		CP_Vertex *pVert = pBody->GetVertex(index);
		glPointSize(12.0f);
		glBegin(GL_POINTS);
		glVertex3d(pVert->m_pPoint->m_x, pVert->m_pPoint->m_y, pVert->m_pPoint->m_z);
		glEnd();
	}
	else if(strSelectedItem.Left(4).Compare("Edge")==0||strSelectedItem.Mid(3,4).Compare("Edge")==0)
	{
		int pos = strSelectedItem.Find(' ');
		strSelectedItem = strSelectedItem.Mid(pos+1);
		int index = atoi(strSelectedItem.GetBuffer(strSelectedItem.GetLength()));
		CP_Edge *pEdge = pBody->GetEdge(index);
		glLineWidth(4.0f);
		pEdge->m_pCurve3D->Draw();
	} else if (strSelectedItem.Left(4).Compare("Face") == 0 || strSelectedItem.Mid(3,4).Compare("Face") == 0) {
		int pos = strSelectedItem.Find(' ');
		strSelectedItem = strSelectedItem.Mid(pos+1);
		int index = atoi(strSelectedItem.GetBuffer(strSelectedItem.GetLength()));
		CP_Face *pFace = pBody->GetFace(index);
		glLineWidth(3.0f);
		for (int i = 0; i < pFace->GetLoopNumber(); ++i) {
			int edgeNum = pFace->GetLoop(i)->GetHalfNumber();
			double color[3] = {0.4, 1, 0.4};
			if (pFace->GetSurfaceType() == TYPE_SURFACE_NURBS || pFace->GetSurfaceType() == TYPE_SURFACE_COONS) {
				color[2] = 1;
				color[1] = 0.5;
				color[0] = 0.3;
// 				color[2] = 0.66;
// 				color[1] = 0.45;
// 				color[0] = 0.4;
			}
			if (pFace->GetSurfaceType() == TYPE_SURFACE_UNDEFINED) {
				color[0] = 1;
				color[1] = 1;
				color[2] = 0.0;
// 				color[0] = 0.54;
// 				color[1] = 0.75;
// 				color[2] = 0.41;
			}
			for (int j = 0; j < edgeNum; ++j) {
				//glColor3d((double)(j+1)/edgeNum * color[0], (double)(j+1)/edgeNum * color[1], (double)(j+1)/edgeNum * color[2]);
				//glColor3d(0, 0, 0);
				glColor3d(color[0], color[1], color[2]);
				pFace->GetLoop(i)->GetHalf(j)->m_pEdge->m_pCurve3D->Draw();
			}
		}
		
 	} else if (strSelectedItem.Left(4).Compare("Body") == 0 || strSelectedItem.Mid(3,4).Compare("Body") == 0) {
		glLineWidth(4.0f);
		DrawEdges(pBody);
	}
}

void CF_BodyUtils::DrawSelectedHighLightEdge(CP_Body *pBody, int edgeIndex) 
{
	ASSERT(edgeIndex >= 0);
	CP_Edge *pEdge = pBody->GetEdge(edgeIndex);
	glLineWidth(4.0f);
	pEdge->m_pCurve3D->Draw();
}

void CF_BodyUtils::DrawUCS(CP_Body *pBody, CP_Point3D *pCenter, double scale) {
	((CP_BodyExt *)pBody)->m_ucs.DrawUCS(*pCenter, scale);
}