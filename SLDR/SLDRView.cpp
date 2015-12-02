// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// SLDRView.cpp : CSLDRView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "SLDR.h"
#endif

#include "SLDRDoc.h"
#include "SLDRView.h"
#include "CF_BodyUtils.h"
#include "CP_Topology.h"
#include "Axis3D.h"
#include "MainFrm.h"
#include <fstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WIN_EXPAND_RADIO 1.4

const float SPHERE_RADIUS = 50.0f;
// CSLDRView

IMPLEMENT_DYNCREATE(CSLDRView, CView)

BEGIN_MESSAGE_MAP(CSLDRView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSLDRView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_POLYLINE, &CSLDRView::OnPolyline)
	ON_COMMAND(ID_DELAUNY, &CSLDRView::OnDelauny)
	ON_COMMAND(ID_FLOWCOMPLEX, &CSLDRView::OnFlowcomplex)
	ON_COMMAND(ID_COLLAPSE, &CSLDRView::OnCollapse)
	ON_WM_KEYUP()
END_MESSAGE_MAP()

// CSLDRView 构造/析构

CSLDRView::CSLDRView()
{
	// TODO: 在此处添加构造代码
	m_rotate[0] = 0;
	m_rotate[1] = 0;
	m_xPos = 0.0f;
	m_yPos = 0.0f;
	m_Scale = 4.0f;
	IsDelauny=false;
	selectedTriangle=-1;
	selectedPolyline=-1;
	IsFC=false;
	IsPoly=false;
	ctrlDown=false;
	zDown=false;
	
}

CSLDRView::~CSLDRView()
{
}

void CSLDRView::ReSet()
{
	m_rotate[0] = 0;
	m_rotate[1] = 0;
	m_xPos = 0.0f;
	m_yPos = 0.0f;
	m_Scale = 4.0f;
	IsDelauny=false;
	selectedTriangle=-1;
	IsFC=false;
	IsPoly=false;
}

BOOL CSLDRView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

void CSLDRView::drawString(const char* str)
{
	static int isFirstCall = 1;
	static GLuint lists;

	if( isFirstCall ) { // 如果是第一次调用，执行初始化
		// 为每一个ASCII字符产生一个显示列表
		isFirstCall = 0;

		// 申请MAX_CHAR个连续的显示列表编号
		lists = glGenLists(MAX_CHAR);

		// 把每个字符的绘制命令都装到对应的显示列表中
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	// 调用每个字符对应的显示列表，绘制每个字符
	for(; *str!='\0'; ++str)
		glCallList(lists + *str);
}

void  CSLDRView::drawData()
{
	CSLDRDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	double halfSize = swidth > sheight? swidth/3*2 : sheight/3*2;
	halfSize *= WIN_EXPAND_RADIO;

	const double radio = (double)swidth / sheight;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D((- halfSize * radio), ( + halfSize* radio),( - halfSize), ( + halfSize));//裁剪范围(左, 右, 下, 上)如果不希望变形，裁剪范围要和窗口成比例

	//然后回到模型视图矩阵模式
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//显示数据
	glColor3f(0.5f, 0.5f, 0.5f);
	glRasterPos3f(-halfSize+100,-halfSize+100,0);
	char c[8];
	_itoa_s(pDoc->m_FlowComplex->m_0cells.size(),c,10);
	drawString("Vertices: ");drawString(c);drawString("              ");

	_itoa_s(pDoc->m_FlowComplex->tricells.size(),c,10);
	drawString("Faces: ");drawString(c);drawString("              ");

	_itoa_s(pDoc->m_FlowComplex->m_2cells.size(),c,10);
	drawString("2cells: ");drawString(c);drawString("              ");

	//半透明框
	glEnable(GL_BLEND);
	//glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glRasterPos3f(-halfSize* radio,-halfSize+200,0.0);
	glColor4f(1.0, 0.82, 1.0, 0.5f);
	glRectf(-halfSize* radio, -halfSize+200, halfSize* radio, -halfSize);
	glDisable(GL_BLEND);

	//然后回到3D。将投影矩阵设置成3D的投影矩阵
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	SetProjectionMatrix(swidth,sheight);
	glMatrixMode(GL_MODELVIEW);
	//glViewport(0, 0, swidth, sheight);

	//glLoadIdentity();
}

void  CSLDRView::drawScene() {
	float colorSpan=0.0005f;
	float color=0.0f;
	float pixelSize=2.0f;
	float posY=-1.0f;
	float posX=-1.0f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode (GL_MODELVIEW); 
	glPushMatrix (); 
	glLoadIdentity ();
	glMatrixMode (GL_PROJECTION);
	glPushMatrix (); 
	glLoadIdentity ();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glBegin (GL_QUADS);
	glColor3f(0.6,0.7,0.7);
	glVertex3f (-1.0f, -1.0f, -1.0f); 
	glVertex3f (1.0f, -1.0f, -1.0f); 

	glColor3f(0.4,0.5,0.5);
	glVertex3f (1.0f, 1.0f, -1.0f); 
	glVertex3f (-1.0f, 1.0f, -1.0f); 
	glEnd ();

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix (); 
	glMatrixMode (GL_MODELVIEW); 
	glPopMatrix ();
	// 交换缓冲区
	//glfwSwapBuffers();
}

// CSLDRView 绘制
#include "Reconstruction.h"
void CSLDRView::OnDraw(CDC* pDC)
{
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	
	wglMakeCurrent(pDC->m_hDC, m_hRC);

	// InitLightAndMaterial();
	//glClearColor(0.13, 0.15, 0.19, 1);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClearDepth(1.0);

	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	//glEnable (GL_LINE_SMOOTH);
	//glHint (GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

	//InitMaterial();

	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);

	InitLight0();
	InitLight1();
	//InitLight2();
	// Rotate coordinate system
	glPushMatrix();
	glLoadIdentity();
	SetModelViewMatrix();

	// Draw body solid
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	//glEnable(GL_LIGHT2);
	drawScene();

	if(pDoc->format=="dxf")
	{
		CP_AssembledBody *pAsmbBody = GetDocument()->m_pAsmbBody;
		if (pAsmbBody != NULL) {
			CBoundBox2D box = pDoc->m_boundBox;

			CP_Body *pBody;
			double halfSize = box.m_width > box.m_height? box.m_width/2 : box.m_height/2;
			halfSize *= 1.2 * 1.2;
			double scale = halfSize/6;

			// Draw highlight
			for (int i = 0; i < pAsmbBody->GetBodyNumber(); i++) {
				if (pDoc->m_selectedItem.m_bodyIndex == i) {
					pBody = pAsmbBody->GetBody(i);
					glColor3f(0.3f,0.3f,0.3f);
					CF_BodyUtils::DrawSelectedHighLight(pBody, pDoc->m_selectedItem.m_strSelectedItem);
				}
			}
			
			// Draw edge for aligning
			if (pMain->GetUCSEditorPane()->GetDialog()->GetAlignAxisIndex() < 0) 
				;
			else {
				if (pMain->GetUCSEditorPane()->GetDialog()->GetAlignAxisIndex() == 0)
					glColor3d(0.8, 0.1, 0.1);
				else if (pMain->GetUCSEditorPane()->GetDialog()->GetAlignAxisIndex() == 1)
					glColor3d(0.1, 0.1, 0.8);
				if (pDoc->m_mouseSelEdge[0] >= 0)
					CF_BodyUtils::DrawSelectedHighLightEdge(pAsmbBody->GetBody(pDoc->m_mouseSelEdge[0]), pDoc->m_mouseSelEdge[1]);
			}


			// Draw body edge
			// glColor3f(1, 1, 1);
			glLineWidth(1.5);
			//glColor3f(0.0, 0.0, 0.0);
			glColor3f(0.5, 0.5, 0.5);
			int beginIndex = pAsmbBody->GetBodyNumber() == 1? 0 : 1;
			for (int i = beginIndex; i < pAsmbBody->GetBodyNumber(); ++i) {
				CF_BodyUtils::DrawEdges(pAsmbBody->GetBody(i));
			}

			pBody = pAsmbBody->GetOriginalBody();
			CBoundBox2D tmpBox;
			tmpBox.InitBoundBox(pBody);
			CF_BodyUtils::DrawUCS(pBody, &tmpBox.m_centerPt, scale);

			// CF_BodyUtils::DrawWireframeBody(pAsmbBody->GetBody(0));
			}

		glColor3d(1.0, 1.0, 1.0);
		CF_BodyUtils::DrawSolidBody(pAsmbBody->GetBody(0));
		
	}else if(pDoc->format=="curve")
	{  
		glColor3f(0.9, 0.68, 0.24);
		glScalef(m_Scale, m_Scale, m_Scale);
		
		if(IsPoly)
		{
			for (int i = 0; i < pDoc->m_FlowComplex->m_PolyLine.size(); i++)
			{
				if(i==selectedPolyline&&ctrlDown)
					{
						glLineWidth(4);
						glColor3f(0.0, 0.9, 0.0);
					}
				else{
					glLineWidth(2);
					glColor3f(0.9, 0.68, 0.24);
				}
				
				pDoc->m_FlowComplex->m_PolyLine[i].Draw();
			}
		}else
		{
			//没有折线预处理时显示原始线框
			for (int i = 0; i < pDoc->VT_PolyLine->size(); i++)
			{
				/*修改模型，polyline中根据cut输出新模型*/
				/*if(i==selectedPolyline&&ctrlDown)
				{
					glLineWidth(4);
					glColor3f(0.0, 0.9, 0.0);
				}
				else{
					glLineWidth(2);
					glColor3f(0.9, 0.68, 0.24);
				}

				if((*pDoc->VT_PolyLine)[i].cut==1)
					glColor3f(0.9, 0.0, 0.0);
				else
					glColor3f(0.9, 0.68, 0.24);*/
				(*pDoc->VT_PolyLine)[i].Draw();
			}

		}

		glColor3f(0.0, 1.0, 1.0);
		glPointSize(10.0f);
		for (int j = 0; j < pDoc->m_FlowComplex->vjoint.size(); j++)
		{
			glBegin(GL_POINTS);//必须是加上s，要不然显示不了
			glVertex3f(pDoc->m_FlowComplex->vjoint[j].m_x, pDoc->m_FlowComplex->vjoint[j].m_y,pDoc->m_FlowComplex->vjoint[j].m_z);
			glEnd();
		}

		//0cells!IsDelauny
		if(!IsDelauny){
			glColor3f(1.0, 0.0, 0.0);
			glPointSize(4.0f);
			for (int j = 0; j < pDoc->m_FlowComplex->m_0cells.size(); j++)
			{
				glBegin(GL_POINTS);//必须是加上s，要不然显示不了
				glVertex3f(pDoc->m_FlowComplex->m_0cells[j].m_x, pDoc->m_FlowComplex->m_0cells[j].m_y,pDoc->m_FlowComplex->m_0cells[j].m_z);
				glEnd();
			}
			
		}

		if(!IsFC&&IsDelauny)
		{
			glColor4f(0.7,0.7,0.7,1);
			for (int i = 0; i <pDoc->m_FlowComplex->delauny2cells.size(); i++)
			{
				CP_Triganle3D *pTri = pDoc->m_FlowComplex->delauny2cells[i];
				drawMeshTri(pTri);
			}
		}

		glColor4f(0.7,0.0,0.0,1);
		for (int i = 0; i <pDoc->m_FlowComplex->visitedtri.size(); i++)
		{
			CP_Triganle3D *pTri = pDoc->m_FlowComplex->visitedtri[i];
			drawMeshTri(pTri);
		}
		int numm=0;
		for (int i = 0; i <pDoc->m_FlowComplex->m_2cells.size(); i++)
		{
			CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[i];
			if(p2cell->flag){
			if(p2cell->type==1){
					glColor4f(0.24,0.74,0.24,1);
					cout<<i<<":"<<p2cell->distance<<endl;
					numm++;
				}
			for(int j=0;j<p2cell->m_triangle.size();j++)
			{
				CP_Triganle3D *pTri = pDoc->m_FlowComplex->tricells[p2cell->m_triangle[j]];
				if((i+pDoc->VT_PolyLine->size())==selectedTriangle)
					glColor4f(0.24,0.74,0.24,1);
				else
					glColor4f(0.74,0.74,0.74,1);
				if(p2cell->type==1){
					glColor4f(0.24,0.24,0.74,1);
				}
				if(pMain->m_ctrlPaneFCCR->m_dialog.showNum==i)
					glColor4f(0.74,0.0,0.0,1);
				/*if(i==pDoc->m_FlowComplex->desN-1)
					glColor4f(0.74,0.74,0.0,1);*/
				drawMeshTri(pTri);
			}

			if(pMain->m_ctrlPaneFCCR->m_dialog._2cellboundary)
			{
				glPushAttrib (GL_ALL_ATTRIB_BITS);
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(-1.0f, -1.0f);
				for(int j=0;j<p2cell->m_boundary.size();j++)
				{
					glLineWidth(5);glColor3f(0.8, 1.0, 0.0);
					glBegin(GL_LINE_STRIP);
					glVertex3d(pDoc->m_FlowComplex->m_0cells[p2cell->m_boundary[j].sp].m_x,pDoc->m_FlowComplex->m_0cells[p2cell->m_boundary[j].sp].m_y,pDoc->m_FlowComplex->m_0cells[p2cell->m_boundary[j].sp].m_z);
					glVertex3d(pDoc->m_FlowComplex->m_0cells[p2cell->m_boundary[j].ep].m_x,pDoc->m_FlowComplex->m_0cells[p2cell->m_boundary[j].ep].m_y,pDoc->m_FlowComplex->m_0cells[p2cell->m_boundary[j].ep].m_z);
					glEnd();
				}
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDisable(GL_POLYGON_OFFSET_LINE);
				glPopAttrib();
			}
			}//flag
		}

		for(int i=0;i<pDoc->m_FlowComplex->seg.size();i++)
		{
			glLineWidth(3);glColor3f(1, 0, 0);
			glBegin(GL_LINE_STRIP);
			glVertex3d(pDoc->m_FlowComplex->seg[i].m_startPt.m_x,pDoc->m_FlowComplex->seg[i].m_startPt.m_y,pDoc->m_FlowComplex->seg[i].m_startPt.m_z);
			glVertex3d(pDoc->m_FlowComplex->seg[i].m_endPt.m_x,pDoc->m_FlowComplex->seg[i].m_endPt.m_y,pDoc->m_FlowComplex->seg[i].m_endPt.m_z);
			glEnd();
		}

		drawData();
	}

	//glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	
	glPopMatrix();

	CAxis3D::DrawAxis(&pDoc->m_boundBox, m_rotate);

	SwapBuffers(pDC->m_hDC);
	wglMakeCurrent(NULL, NULL);
	
}

void CSLDRView::drawMeshTri(CP_Triganle3D* pTri)
{
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	/*glBegin(GL_POLYGON);
	glNormal3d(n0.m_x, n0.m_y, n0.m_z);
	glVertex3d(pVt0->x(), pVt0->y(), pVt0->z());
	glNormal3d(n1.m_x, n1.m_y, n1.m_z);
	glVertex3d(pVt1->x(), pVt1->y(), pVt1->z());
	glNormal3d(n2.m_x, n2.m_y, n2.m_z);
	glVertex3d(pVt2->x(), pVt2->y(), pVt2->z());
	glEnd();*/
	glPushAttrib (GL_ALL_ATTRIB_BITS);
	CP_Vector3D ntmp = (pDoc->m_FlowComplex->m_0cells[pTri->m_points[1]] - pDoc->m_FlowComplex->m_0cells[pTri->m_points[0]]) ^ (pDoc->m_FlowComplex->m_0cells[pTri->m_points[2]] -pDoc->m_FlowComplex->m_0cells[pTri->m_points[0]]);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
	glNormal3f(ntmp.m_x, ntmp.m_y, ntmp.m_z);
	glVertex3f(pDoc->m_FlowComplex->m_0cells[pTri->m_points[0]].m_x, pDoc->m_FlowComplex->m_0cells[pTri->m_points[0]].m_y, pDoc->m_FlowComplex->m_0cells[pTri->m_points[0]].m_z);
	glNormal3f(ntmp.m_x, ntmp.m_y, ntmp.m_z);
	glVertex3f(pDoc->m_FlowComplex->m_0cells[pTri->m_points[1]].m_x, pDoc->m_FlowComplex->m_0cells[pTri->m_points[1]].m_y, pDoc->m_FlowComplex->m_0cells[pTri->m_points[1]].m_z);
	glNormal3f(ntmp.m_x, ntmp.m_y, ntmp.m_z);
	glVertex3f(pDoc->m_FlowComplex->m_0cells[pTri->m_points[2]].m_x, pDoc->m_FlowComplex->m_0cells[pTri->m_points[2]].m_y, pDoc->m_FlowComplex->m_0cells[pTri->m_points[2]].m_z);
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);
	
	if(!pMain->m_ctrlPaneFCCR->m_dialog.triboundary)
	{
		glLineWidth(1);glColor3f(0.0, 0.0, 0.0);
		glBegin(GL_LINE_LOOP);
		for (int j = 0; j < 3; j++)
			glVertex3f(pDoc->m_FlowComplex->m_0cells[pTri->m_points[j]].m_x, pDoc->m_FlowComplex->m_0cells[pTri->m_points[j]].m_y, pDoc->m_FlowComplex->m_0cells[pTri->m_points[j]].m_z);
		glEnd();
		glPopAttrib ();
	}
}

void CSLDRView::SetModelViewMatrix() 
{
	CBoundBox2D box = GetDocument()->m_boundBox;
	glRotatef(m_rotate[0], 1.0, 0.0, 0.0);
	glRotatef(m_rotate[1], 0.0, 1.0, 0.0);
	glTranslatef(m_xPos, m_yPos, 0.0f);
	glTranslated(-box.m_centerPt.m_x, -box.m_centerPt.m_y, -box.m_centerPt.m_z);
}


void CSLDRView::InitMaterial() {
	GLfloat matAmbient[] = {0.2, 0.2, 0.2, 1.0};
	GLfloat matDiffuse[]   = {0.5, 0.5, 0.5, 1.0};
	GLfloat matAmbDif[]   = {0.5, 0.5, 0.5, 1.0};
	GLfloat matSpecular[] = {0.1, 0.1, 0.1, 1.0};
	GLfloat shine[] = {10.0}; 
	GLfloat matEmission[] = {0.1, 0.1, 0.1, 1.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse); 
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbDif);  
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shine);
}

void CSLDRView::InitLight0() {
	const GLfloat pos = 400;
	GLfloat light_ambient [] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse [] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat light_specular[] = { 0.9, 0.9, 0.9, 1.0 };
	GLfloat light_position[] = { 0.0, 110.0, 110.0, 1.0 };
	//glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE , light_diffuse );
	glLightfv(GL_LIGHT0, GL_AMBIENT , light_ambient );
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	float Light_Model_Ambient[] = { 0.2 , 0.2 , 0.2 , 1.0 }; // 缺省值
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT , Light_Model_Ambient );

}

void CSLDRView::InitLight1() {
	const GLfloat pos = 200; 
	GLfloat light_ambient [] = { 0.0, 0.0, 0.0, 1.0 };
	if(format=="curve"){
		light_ambient[0] = 0.6;
		light_ambient[1] = 0.6;
		light_ambient[2] = 0.6;
		light_ambient[3] = 1.0;
	}
	GLfloat light_diffuse [] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat light_specular[] = { 0.9, 0.9, 0.9, 1.0 };
	GLfloat light_position[] = { 0, 0, pos, 0.0 };
	glLightfv(GL_LIGHT1, GL_POSITION, light_position);
	glLightfv(GL_LIGHT1, GL_AMBIENT , light_ambient );
	glLightfv(GL_LIGHT1, GL_DIFFUSE , light_diffuse );
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
}

void CSLDRView::InitLight2() {
	const GLfloat pos = 300; 
	GLfloat light_ambient [] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat light_diffuse [] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat light_specular[] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat light_position[] = { 0, -pos, pos/2, 1.0 };
	glLightfv(GL_LIGHT2, GL_POSITION, light_position);
	glLightfv(GL_LIGHT2, GL_AMBIENT , light_ambient );
	glLightfv(GL_LIGHT2, GL_DIFFUSE , light_diffuse );
	glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
}

// CSLDRView 打印


void CSLDRView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CSLDRView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CSLDRView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CSLDRView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CSLDRView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSLDRView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CSLDRView 诊断

#ifdef _DEBUG
void CSLDRView::AssertValid() const
{
	CView::AssertValid();
}

void CSLDRView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSLDRDoc* CSLDRView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSLDRDoc)));
	return (CSLDRDoc*)m_pDocument;
}
#endif //_DEBUG


// CSLDRView 消息处理程序


void CSLDRView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (cx <=0 || cy <= 0)
		return ;

	swidth = cx;
	sheight = cy;

	CClientDC dc(this);
	wglMakeCurrent(dc.m_hDC, m_hRC);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	SetProjectionMatrix(cx, cy);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, cx, cy);

	glLoadIdentity();

	wglMakeCurrent(NULL, NULL);
}

void CSLDRView::SetProjectionMatrix(int cx, int cy) 
{
	CBoundBox2D *pBBox = &(GetDocument()->m_boundBox);
	double width = pBBox->m_width;
	double height = pBBox->m_height;
	double halfSize = width > height? width/3*2 : height/3*2;
	halfSize *= WIN_EXPAND_RADIO;

	const double radio = (double)cx / cy;

	glOrtho((- halfSize * radio), ( + halfSize* radio) , 
		( - halfSize), ( + halfSize), -DEPTH_RANGE, DEPTH_RANGE);
	//glOrtho(-cx / 2, cx / 2, -cy / 2, cy / 2, -DEPTH_RANGE, DEPTH_RANGE);
}


int CSLDRView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	InitOpenGL();

	return 0;
}

void CSLDRView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	// Set : a specified OpenGL rendering context ==> NULL
	// Set : current rendering context ==> NULL
	wglMakeCurrent(NULL, NULL);

	// Delete the handle to an OpenGL rendering context 
	wglDeleteContext(m_hRC);
	m_hRC=NULL;

}


BOOL CSLDRView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	// return CView::OnEraseBkgnd(pDC);
	return TRUE;
}


void CSLDRView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
	int step = 5;

	switch (nChar)
	{
	case VK_UP:        m_yPos = m_yPos + 1.0f;
		break;
	case VK_DOWN:    m_yPos = m_yPos - 1.0f;
		break;
	case VK_LEFT:    m_xPos = m_xPos - 1.0f;
		break;
	case VK_RIGHT:  m_xPos = m_xPos + 1.0f;
		break;
	case VK_CONTROL: ctrlDown=true;
		break;
	case 'Z':	zDown=true;
		break;
	}
	
	this->Invalidate();

}


void CSLDRView::InitOpenGL() {
	// The PIXELFORMATDESCRIPTOR structure describes
	//		the pixel format of a drawing surface.
	PIXELFORMATDESCRIPTOR pfd =
	{ 
		sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd 
		1,                     			// version number 
		PFD_DRAW_TO_WINDOW |   	// support window 
		PFD_SUPPORT_OPENGL |   	// support OpenGL 
		PFD_DOUBLEBUFFER,		// double buffered
		PFD_TYPE_RGBA,
		24,                    	// 24-bit color depth 
		0, 0, 0, 0, 0, 0,       // color bits ignored 
		0,                     	// no alpha buffer 
		0,                     	// shift bit ignored 
		0,                     	// no accumulation buffer 
		0, 0, 0, 0,            	// accum bits ignored 
		32,                    	// 32-bit z-buffer (depth)
		0,                     	// no stencil buffer 
		0,                     	// no auxiliary buffer 
		PFD_MAIN_PLANE,         // main layer 
		0,                     	// reserved 
		0, 0, 0                	// layer masks ignored 
	}; 

	CClientDC dc(this);
	int pixelFormat = ChoosePixelFormat(dc.m_hDC, &pfd);

	if (!SetPixelFormat(dc.m_hDC, pixelFormat, &pfd))
	{
		MessageBox(_T("Error: Unable to Set Pixel Format in CGLTemplate1View::OnCreate( )"),
			_T("Application Error"), MB_ICONERROR);
	}
	m_hRC = wglCreateContext(dc.m_hDC);
}



void CSLDRView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	if (pMain->GetCursor() == SEL)
	{
		FindSelNode(point);
	}
	
	if(pDoc->format=="curve"&&ctrlDown){
		FindPolyLine(point);
	}

	if (GetCapture() == this)
	{
		//Increment the object rotation angles
		m_rotate[0] += (point.y - m_MouseDownPoint.y) / 3.6;
		m_rotate[1] += (point.x - m_MouseDownPoint.x) / 3.6;//鼠标在窗口x轴方向上的增量加到视点绕y轴的角度上，这样就左右转了 
		//Redraw the view
		InvalidateRect(NULL, FALSE);
		//Set the mouse point
		m_MouseDownPoint = point;
	};
	CView::OnMouseMove(nFlags, point);
}

void CSLDRView::FindTriangle(CPoint point)
{
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

#define BUFSIZE 512
	CClientDC dc(this);
	wglMakeCurrent(dc.m_hDC, m_hRC); // 设置当前context，为了获取glGetIntegerv
	GLuint selectBuf[BUFSIZE]={0};//创建一个保存选择结果的数组
	GLint hits; //点击记录的个数
	GLint viewport[4];  //视口,使用glViewport获取

	glSelectBuffer (BUFSIZE, selectBuf);//告诉OpenGL初始化selectbuffer
	glRenderMode(GL_SELECT);	//进入选择模式

	glInitNames();	//初始化名字栈

	glMatrixMode (GL_PROJECTION); //进入投影阶段准备拾取
	glPushMatrix (); //保存以前的投影矩阵
	glLoadIdentity (); //载入单位矩阵
	glGetIntegerv(GL_VIEWPORT, viewport); //获得viewport

	gluPickMatrix((GLdouble) point.x, (GLdouble) (viewport[3] - point.y + viewport[1]), 5, 5,  viewport); // 选择框的大小为12，12
	SetProjectionMatrix(viewport[2], viewport[3]);

	// 绘制待选择的形体，在Select模式下不会复制到帧缓冲区
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	SetModelViewMatrix();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // 必须要Clear buffer
	glScalef(m_Scale, m_Scale, m_Scale);
	for (int i = 0; i <pDoc->m_FlowComplex->m_2cells.size(); i++)
	{
		CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[i];
		if(pDoc->m_FlowComplex->m_2cells[i]->flag){
			glPushName(i+pDoc->VT_PolyLine->size());
			for(int j=0;j<p2cell->m_triangle.size();j++)
			{
				CP_Triganle3D *pTri = pDoc->m_FlowComplex->tricells[p2cell->m_triangle[j]];
				glColor4f(0.74,0.74,0.74,1);
				drawMeshTri(pTri);
			}
			glPopName();
		}
	}

	glPopMatrix();
	// SwapBuffers(dc.m_hDC);
	// 绘制结束

	hits = glRenderMode (GL_RENDER);	// 从选择模式返回正常模式,该函数返回选择到对象的个数
	
	ProcessHits(hits, selectBuf);

	// 恢复矩阵
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	wglMakeCurrent(NULL, NULL);
}

void CSLDRView::FindSelNode(CPoint point) 
{
	#define BUFSIZE 512
	CClientDC dc(this);
	wglMakeCurrent(dc.m_hDC, m_hRC); // 设置当前context，为了获取glGetIntegerv
	GLuint selectBuf[BUFSIZE]={0};//创建一个保存选择结果的数组
	GLint hits; //点击记录的个数
	GLint viewport[4];  //视口,使用glViewport获取

	glSelectBuffer (BUFSIZE, selectBuf);//告诉OpenGL初始化selectbuffer
	glRenderMode(GL_SELECT);	//进入选择模式

	glInitNames();	//初始化名字栈

	glMatrixMode (GL_PROJECTION); //进入投影阶段准备拾取
	glPushMatrix (); //保存以前的投影矩阵
	glLoadIdentity (); //载入单位矩阵
	glGetIntegerv(GL_VIEWPORT, viewport); //获得viewport

	gluPickMatrix((GLdouble) point.x, (GLdouble) (viewport[3] - point.y + viewport[1]), 5, 5,  viewport); // 选择框的大小为12，12
	SetProjectionMatrix(viewport[2], viewport[3]);

	// 绘制待选择的形体，在Select模式下不会复制到帧缓冲区
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	SetModelViewMatrix();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // 必须要Clear buffer
		CP_AssembledBody *pAsmbBody = GetDocument()->m_pAsmbBody;
		int beginIndex = pAsmbBody->GetBodyNumber() == 1? 0 : 1;
		for (int i = beginIndex; i < pAsmbBody->GetBodyNumber(); ++i) {
			glPushName(BODY_EDGE_MAX_NUM * i);
			CF_BodyUtils::DrawEdges(pAsmbBody->GetBody(i));
			glPopName();
		}
		
	glPopMatrix();
	// SwapBuffers(dc.m_hDC);
	// 绘制结束

	hits = glRenderMode (GL_RENDER);	// 从选择模式返回正常模式,该函数返回选择到对象的个数
	ProcessHits(hits, selectBuf);

	// 恢复矩阵
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	wglMakeCurrent(NULL, NULL);
}

void CSLDRView::ProcessHits(GLint hits, GLuint buffer[])
{
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();

	if (hits) {
		GetDocument()->m_mouseSelEdge[0] = buffer[3] / BODY_EDGE_MAX_NUM;
		GetDocument()->m_mouseSelEdge[1] = buffer[4];
		unsigned int i, j;
		GLuint names, *ptr, minZ,*ptrNames, numberOfNames;

		//printf ("hits = %d\n", hits);
		ptr = (GLuint *) buffer;
		minZ = 0xffffffff;
		for (i = 0; i < hits; i++) {	
			names = *ptr;
			ptr++;
			if (*ptr < minZ) {
				numberOfNames = names;
				minZ = *ptr;
				ptrNames = ptr+2;
			}

			ptr += names+2;
		}
		//printf ("The closest hit names are ");
		ptr = ptrNames;
		for (j = 0; j < numberOfNames; j++,ptr++) {
			if(ctrlDown)
				selectedPolyline=*ptr;
			else if(zDown||pMain->m_ctrlPaneFCCR->m_dialog.selectEnable)
				selectedTriangle=*ptr;
		//	printf ("%d ", *ptr);
		}
		//printf ("\n");
	} else
	{
		GetDocument()->m_mouseSelEdge[0] = -1;
		GetDocument()->m_mouseSelEdge[1] = -1;
	}
	Invalidate(TRUE);
}

void CSLDRView::FindPolyLine(CPoint point)
{
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

#define BUFSIZE 512
	CClientDC dc(this);
	wglMakeCurrent(dc.m_hDC, m_hRC); // 设置当前context，为了获取glGetIntegerv
	GLuint selectBuf[BUFSIZE]={0};//创建一个保存选择结果的数组
	GLint hits; //点击记录的个数
	GLint viewport[4];  //视口,使用glViewport获取

	glSelectBuffer (BUFSIZE, selectBuf);//告诉OpenGL初始化selectbuffer
	glRenderMode(GL_SELECT);	//进入选择模式

	glInitNames();	//初始化名字栈

	glMatrixMode (GL_PROJECTION); //进入投影阶段准备拾取
	glPushMatrix (); //保存以前的投影矩阵
	glLoadIdentity (); //载入单位矩阵
	glGetIntegerv(GL_VIEWPORT, viewport); //获得viewport

	gluPickMatrix((GLdouble) point.x, (GLdouble) (viewport[3] - point.y + viewport[1]), 5, 5,  viewport); // 选择框的大小为12，12
	SetProjectionMatrix(viewport[2], viewport[3]);

	// 绘制待选择的形体，在Select模式下不会复制到帧缓冲区
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	SetModelViewMatrix();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // 必须要Clear buffer
	glScalef(m_Scale, m_Scale, m_Scale);

	//for (int i = 0; i < pDoc->VT_PolyLine->size(); i++)
	//{
	//	glPushName(i);
	//	(*pDoc->VT_PolyLine)[i].Draw();
	//	glPopName();
	//}

	for (int i = 0; i < pDoc->m_FlowComplex->m_PolyLine.size(); i++)
	{
	glPushName(i);
	pDoc->m_FlowComplex->m_PolyLine[i].Draw();
	glPopName();
	}

	glPopMatrix();
	// SwapBuffers(dc.m_hDC);
	// 绘制结束

	hits = glRenderMode (GL_RENDER);	// 从选择模式返回正常模式,该函数返回选择到对象的个数

	ProcessHits(hits, selectBuf);

	// 恢复矩阵
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	wglMakeCurrent(NULL, NULL);
}

void CSLDRView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	m_MouseDownPoint = point;
	SetCapture();
	if (pMain->GetCursor() == SEL)
	{
		FindSelNode(point);
		if ( pDoc->m_mouseSelEdge[0] < 0) // No edge selected
			pMain->GetUCSEditorPane()->GetDialog()->CancelAlign();
		else
			pMain->GetUCSEditorPane()->GetDialog()->AlignWith(pDoc->m_pAsmbBody, pDoc->m_mouseSelEdge[0], pDoc->m_mouseSelEdge[1]);
		pDoc->m_mouseSelEdge[0] = -1;
		pDoc->m_mouseSelEdge[1] = -1;
		Invalidate(TRUE);
	}

	if(IsDelauny&&(pMain->m_ctrlPaneFCCR->m_dialog.selectEnable||zDown))
	{
		FindTriangle(point);
	}

	if(selectedTriangle!=-1)
	{
		pMain->m_ctrlPaneFCCR->m_dialog.SetTreeItems(selectedTriangle-pDoc->VT_PolyLine->size());//2cell在集合中的下标，非标号
		//(*pDoc->VT_PolyLine)[selectedTriangle].cut=1;
		//pDoc->m_FlowComplex->m_2cells[selectedTriangle-pDoc->VT_PolyLine->size()]->flag=false;
		//cout<<"line index:"<<selectedTriangle<<endl;
	}

	if(pDoc->format=="curve"&&ctrlDown){
		FindPolyLine(point);
	}

	if(selectedPolyline!=-1)
	{
		cout<<"select polyline:"<<selectedPolyline<<endl;
	}

	CView::OnLButtonDown(nFlags, point);
}


double dist(const CP_Point3D &x,const CP_Point3D &y)
{
	return sqrt((x.m_x - y.m_x)*(x.m_x - y.m_x) + (x.m_y - y.m_y)*(x.m_y - y.m_y) + (x.m_z - y.m_z)*(x.m_z - y.m_z));
}

BOOL CSLDRView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_Scale += 0.1 *zDelta/2 ;
	InvalidateRect(NULL, FALSE);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CSLDRView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_MouseDownPoint = CPoint(0, 0);
	ReleaseCapture();
	CView::OnLButtonUp(nFlags, point);
}


void CSLDRView::OnPolyline()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	clock_t start,end;
	start = clock();
	pMain->m_ctrlPaneFCCR->m_dialog.fccr.ToPolyLine();
	 end = clock();
	 cout<<"time for polyline: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	IsPoly=true;

	//ofstream out("123.curve");
	//int num=0;
	//for (int i = 0; i < pDoc->VT_PolyLine->size(); i++)
	//{
	//	if((*pDoc->VT_PolyLine)[i].cut==1){
	//	out<<(*pDoc->VT_PolyLine)[i].m_points.size()<<endl;
	//	for (int j = 0; j < (*pDoc->VT_PolyLine)[i].m_points.size(); j++)
	//	{
	//		out<<(*pDoc->VT_PolyLine)[i].m_points[j].m_x<<" "<<(*pDoc->VT_PolyLine)[i].m_points[j].m_y<<" "<< (*pDoc->VT_PolyLine)[i].m_points[j].m_z<<endl;
	//	}
	//	}
	//}

	Invalidate(); 
}


void CSLDRView::OnDelauny()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	IsDelauny=true;
	pMain->m_ctrlPaneFCCR->m_dialog.fccr.nn=0;
	clock_t start,end;
	start = clock();
	pMain->m_ctrlPaneFCCR->m_dialog.fccr.OnDelaunyTriangulation();
	end = clock();
	cout<<"time for Delauny: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	Invalidate(); 
}


void CSLDRView::OnFlowcomplex()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	pMain->m_ctrlPaneFCCR->m_dialog.fccr.filename=pDoc->filename;
	IsFC=true;
	clock_t start,end;
	start = clock();
	pMain->m_ctrlPaneFCCR->m_dialog.fccr.ToFlowcomplex();
	end = clock();
	cout<<"time for compute Flow Complex: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	Invalidate(); 
}


void CSLDRView::OnCollapse()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	clock_t start,end;
	start = clock();
	pMain->m_ctrlPaneFCCR->m_dialog.fccr.OnCollapse();
	end = clock();
	cout<<"time for collapse: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
	Invalidate(); 
}


void CSLDRView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
	switch (nChar)
	{
	case VK_CONTROL:	ctrlDown=false;
		selectedPolyline=-1;
		break;
	case 'Z':	zDown=false;
		break;
	}

	this->Invalidate();
}
