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
#include "ArcBall.h"    
#include "CP_FlowComplex.h"
#include "SurfaceOptimization.h"

#include <fstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WIN_EXPAND_RADIO 1.4

const float SPHERE_RADIUS = 50.0f;
ArcBallT arcBall(600.0f,400.0f);
ArcBallT*    ArcBall =&arcBall;
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
	ON_COMMAND(ID_DELAUNY, &CSLDRView::OnPruning)
	ON_COMMAND(ID_FLOWCOMPLEX, &CSLDRView::OnFlowcomplex)
	ON_COMMAND(ID_COLLAPSE, &CSLDRView::OnSpreadAndMerge)
	ON_WM_KEYUP()
	ON_COMMAND(ID_BUTTON2, &CSLDRView::OnFindingCycles)
	ON_COMMAND(ID_BUTTON5, &CSLDRView::OnFCReconstruction)
	ON_UPDATE_COMMAND_UI(ID_POLYLINE, &CSLDRView::OnUpdatePolyline)
	ON_UPDATE_COMMAND_UI(ID_DELAUNY, &CSLDRView::OnUpdateDelauny)
	ON_UPDATE_COMMAND_UI(ID_FLOWCOMPLEX, &CSLDRView::OnUpdateFlowcomplex)
	ON_UPDATE_COMMAND_UI(ID_COLLAPSE, &CSLDRView::OnUpdateCollapse)
	ON_UPDATE_COMMAND_UI(ID_BUTTON2, &CSLDRView::OnUpdateButton2)
	ON_COMMAND(ID_SLIDER_SPECULAR, &CSLDRView::OnSliderSpecular)
	ON_COMMAND(ID_SLIDER_Diffuse, &CSLDRView::OnSliderDiffuse)
	ON_COMMAND(ID_SLIDER_SHININESS, &CSLDRView::OnSliderShininess)
	ON_COMMAND(ID_CHECK_LIGHT0, &CSLDRView::OnCheckLight0)
	ON_COMMAND(ID_CHECK_LIGHT1, &CSLDRView::OnCheckLight1)
	ON_COMMAND(ID_CHECK_LIGHT2, &CSLDRView::OnCheckLight2)
	ON_UPDATE_COMMAND_UI(ID_CHECK_LIGHT0, &CSLDRView::OnUpdateCheckLight0)
	ON_UPDATE_COMMAND_UI(ID_CHECK_LIGHT1, &CSLDRView::OnUpdateCheckLight1)
	ON_UPDATE_COMMAND_UI(ID_CHECK_LIGHT2, &CSLDRView::OnUpdateCheckLight2)
	ON_COMMAND(ID_SLIDER_AMBIENT0, &CSLDRView::OnSliderAmbient0)
	ON_COMMAND(ID_SLIDER_SPECULAR0, &CSLDRView::OnSliderSpecular0)
	ON_COMMAND(ID_SLIDER_DIFFUSE0, &CSLDRView::OnSliderDiffuse0)
	ON_COMMAND(ID_SLIDER_AMBIENT1, &CSLDRView::OnSliderAmbient1)
	ON_COMMAND(ID_SLIDER_SPECULAR1, &CSLDRView::OnSliderSpecular1)
	ON_COMMAND(ID_SLIDER_DIFFUSE1, &CSLDRView::OnSliderDiffuse1)
	//	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_SLIDER_SPECULAR2, &CSLDRView::OnSliderSpecular2)
	ON_COMMAND(ID_SLIDER_CUTOFF2, &CSLDRView::OnSliderCutoff2)
	ON_COMMAND(ID_SLIDER_EXPONENT2, &CSLDRView::OnSliderExponent2)
	ON_WM_RBUTTONDOWN()
	ON_UPDATE_COMMAND_UI(ID_BUTTON5, &CSLDRView::OnUpdateFCReconstruction)
	ON_COMMAND(ID_BUTTON_IMPROVEDFC, &CSLDRView::OnButtonImprovedfc)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_IMPROVEDFC, &CSLDRView::OnUpdateButtonImprovedfc)
	ON_COMMAND(ID_BUTTON_PREPROCESS, &CSLDRView::OnButtonPreprocess)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PREPROCESS, &CSLDRView::OnUpdateButtonPreprocess)
	ON_COMMAND(ID_CHECK_TWOSIDE, &CSLDRView::OnCheckTwoside)
	ON_UPDATE_COMMAND_UI(ID_CHECK_TWOSIDE, &CSLDRView::OnUpdateCheckTwoside)
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
	selected2cell=-1;
	selectedpatch=-1;
	selectedpoly=-1;
	moveonpoly=-1;

	fcEnable=true;
	ifcEnable=true;
	showPruning=false;
	showFC=false;
	showInputP=false;
	showTop=false;
	showCycles=false;

	showPolyEnable=true;
	showFCEnable=false;
	showMergeEnable=false;
	showCyclesEnable=false;
	showPruningEnable=false;
	showPreprocessEnable=false;

	shiftDown=false;
	CtrlDown=false;
	CKeyDown=false;
    which=0;
	twoSide=false;
	mSpecular=0.45;
	mShine=45;
	mDiffuse=0.8;
	light0=true;
	light1=true;
	light2=true;
	mAmbient0=0.16;
	mSpecular0=0.23;
	mDiffuse0=0.7;

	mAmbient1=0.0;
	mSpecular1=0.7;
	mDiffuse1=0.21;

	mCutOff2=10;
	mSpecular2=0.4;
	mExponent2=30;

	ArcBall->setBounds(swidth,sheight);
	step=0;
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
	//m_Scale = 4.0f;
	selected2cell=-1;
	selectedpatch=-1;
	selectedpoly=-1;
	moveonpoly=-1;

	fcEnable=true;
	ifcEnable=true;
	showPruning=false;
	showFC=false;
	showInputP=false;
	showTop=false;
	showCycles=false;

	showPolyEnable=true;
	showFCEnable=false;
	showMergeEnable=false;
	showCyclesEnable=false;
	showPruningEnable=false;
	showPreprocessEnable=false;

	which=0;
	twoSide=false;
	mSpecular=0.45;
	mShine=45;
	mDiffuse=0.8;
	light0=true;
	light1=true;
	light2=true;
	mAmbient0=0.16;
	mSpecular0=0.23;
	mDiffuse0=0.7;

	mAmbient1=0.0;
	mSpecular1=0.7;
	mDiffuse1=0.21;

	mCutOff2=10;
	mSpecular2=0.4;
	mExponent2=30;

	ArcBall->setBounds(swidth,sheight);
	step=0;
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
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
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
	glColor3f(0.2f, 0.2f, 0.2f);
	glNormal3f(1.0f,1.0f,1.0f);
	glRasterPos3f(-halfSize-300,-halfSize+100,0);
	char c[8];
	_itoa_s(pDoc->m_FlowComplex->inputCurves,c,10);
	drawString("Curves: ");drawString(c);drawString("              ");

	_itoa_s(pDoc->m_FlowComplex->inputPoints,c,10);
	drawString("Vertices: ");drawString(c);drawString("              ");

	if(pMain->m_ctrlPaneFCCR->m_dialog.fccr.IsProcess)
		_itoa_s(pDoc->m_FlowComplex->res_triangle_num,c,10);
	else
		_itoa_s(pDoc->m_FlowComplex->tricells.size(),c,10);
	drawString("Faces: ");drawString(c);drawString("              ");
	
	_itoa_s(pDoc->m_FlowComplex->m_2cells.size(),c,10);
	drawString("2cells: ");drawString(c);drawString("              ");

	_itoa_s(pDoc->m_FlowComplex->m_3cells.size(),c,10);
	drawString("3cells: ");drawString(c);drawString("              ");

	_itoa_s(pDoc->m_FlowComplex->mergededge,c,10);
	drawString("mergededge: ");drawString(c);drawString("              ");

	_itoa_s(pDoc->m_FlowComplex->respatches,c,10);
	drawString("Patches: ");drawString(c);drawString("              ");

	if(selectedpatch!=-1)
	{
		if(pDoc->m_FlowComplex->m_patches[selectedpatch]->cycle.size()>0)
		    _itoa_s(which+1,c,10);
		else
			_itoa_s(0,c,10);
		drawString("cycle: ");drawString(c);drawString(":");
		_itoa_s(pDoc->m_FlowComplex->m_patches[selectedpatch]->cycle.size(),c,10);
		drawString(c);
	}
	//半透明框
	glEnable(GL_BLEND);
	//glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glRasterPos3f(-halfSize* radio,-halfSize+200,0.0);
	glColor4f(1.0, 0.82, 1.0, 0.4f);
	glNormal3f(1.0f,1.0f,1.0f);
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
	glColor3f(0.7,0.8,0.8);
	glVertex3f (-1.0f, -1.0f, -1.0f); 
	glVertex3f (1.0f, -1.0f, -1.0f); 

	glColor3f(0.5,0.6,0.6);
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
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,twoSide);//双面光照
	InitMaterial();

	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);
	if(light0)
		InitLight0();
	if(light1)
		InitLight1();
	if(light2)
		InitLight2();
	// Rotate coordinate system
	glPushMatrix();
	glLoadIdentity();
	SetModelViewMatrix();

	// Draw body solid
	glEnable(GL_LIGHTING);
	if(light0)
		glEnable(GL_LIGHT0);
	if(light1)
		glEnable(GL_LIGHT1);
	if(light2)
		glEnable(GL_LIGHT2);
	//drawScene();
	
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
		
		//CAxis3D::DrawAxis(&pDoc->m_boundBox, m_rotate);
	}else if(pDoc->format=="curve")
	{  
		glScalef(m_Scale, m_Scale, m_Scale);
		//0cells
		if(showInputP){
			pDoc->m_FlowComplex->DrawPoints();
		}

		if(pMain->m_ctrlPaneFCCR->m_dialog.showdarts)
		{
			pDoc->m_FlowComplex->DrawDarts();
		}

		//flow complex
		if(showFC)
		{
			if(pMain->m_ctrlPaneFCCR->m_dialog.showDelaunay){
				pDoc->m_FlowComplex->DrawDelaunyTriangles();
			}else if(pMain->m_ctrlPaneFCCR->m_dialog.showvoids){
				if(pMain->m_ctrlPaneFCCR->m_dialog.sel3cell!=-1)
				{
					int _3cell=pMain->m_ctrlPaneFCCR->m_dialog.sel3cell;
					pDoc->m_FlowComplex->DrawVoids(_3cell,pMain->m_ctrlPaneFCCR->m_dialog.sel2cell,
						pMain->m_ctrlPaneFCCR->m_dialog.seltriangle,pMain->m_ctrlPaneFCCR->m_dialog._2cellboundary,
						pMain->m_ctrlPaneFCCR->m_dialog.triboundary,pMain->m_ctrlPaneFCCR->m_dialog.mTrans);
				}
			}else
			{
				if(pMain->m_ctrlPaneFCCR->m_dialog.showcreators)//半透明效果
					glDepthMask(GL_FALSE);
				pDoc->m_FlowComplex->DrawFlowComplex(pMain->m_ctrlPaneFCCR->m_dialog.showcreators,pMain->m_ctrlPaneFCCR->m_dialog.selcreator,
					pMain->m_ctrlPaneFCCR->m_dialog._2cellboundary,pMain->m_ctrlPaneFCCR->m_dialog.triboundary);
				if(pMain->m_ctrlPaneFCCR->m_dialog.showcreators)
					glDepthMask(GL_TRUE);
			}
		}//showFC

		if(showTop)
		{
			for (unsigned int i = 0; i <pDoc->m_FlowComplex->topo.size(); i++)
			{
				CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[pDoc->m_FlowComplex->topo[i]];
				glColor4f(0.7f,0.7f,0.7f,1.0f);
				pDoc->m_FlowComplex->Draw2cell(*p2cell);
				if(pMain->m_ctrlPaneFCCR->m_dialog.triboundary)
					pDoc->m_FlowComplex->DrawTriangleBoundary(*p2cell);

				if(pMain->m_ctrlPaneFCCR->m_dialog._2cellboundary)
					pDoc->m_FlowComplex->Draw2cellBoundary(*p2cell);
			}//2cell
		}//showTop

		if(showCycles)
		{
			if(selectedpatch!=-1)
			{
				CP_Patch *pPatch = pDoc->m_FlowComplex->m_patches[selectedpatch];
				for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
				{
					glColor4f(pDoc->r[2],pDoc->g[2],pDoc->b[2],1.0);
					CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
					pDoc->m_FlowComplex->Draw2cell(*p2cell);
					glColor4f(0.1,0.6,0.1,1.0);
					glLineWidth(1.0f);
					if(pMain->m_ctrlPaneFCCR->m_dialog.triboundary)
						pDoc->m_FlowComplex->DrawTriangleBoundary(*p2cell);
				}
				pDoc->m_FlowComplex->DrawPatchBoundary(*pPatch,CtrlDown,CKeyDown,which,pMain->m_ctrlPaneFCCR->m_dialog.showRMF);
			}
			for(int i=0;i<pDoc->m_FlowComplex->oripatches;i++)
			{
				if(pDoc->m_FlowComplex->m_patches[i]->flag&&i!=selectedpatch){
					CP_Patch *pPatch = pDoc->m_FlowComplex->m_patches[i];
					for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
					{
						if(selectedpatch==-1)
							glColor4f(0.7,0.7,0.7,1.0);
						else if(selectedpatch!=-1)
							glColor4f(0.7,0.7,0.7,1.0-(CtrlDown||CKeyDown||(selectedpatch!=-1))*0.4);//按下ctrl显示环

						CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
						pDoc->m_FlowComplex->Draw2cell(*p2cell);
						if(pMain->m_ctrlPaneFCCR->m_dialog.triboundary)
							pDoc->m_FlowComplex->DrawTriangleBoundary(*p2cell);

						if(pMain->m_ctrlPaneFCCR->m_dialog._2cellboundary)
							pDoc->m_FlowComplex->Draw2cellBoundary(*p2cell);
					}//j
				}
			}//i
			for(unsigned int i=0;i<pDoc->m_FlowComplex->connectedPatches.size();++i){
				int _patch=pDoc->m_FlowComplex->connectedPatches[i];
				if(_patch!=selectedpatch){
					CP_Patch *pPatch = pDoc->m_FlowComplex->m_patches[_patch];
					for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
					{
						if(selectedpatch==-1)
							glColor4f(0.7,0.5,0.5,1.0);
						else if(selectedpatch!=-1)
							glColor4f(0.7,0.7,0.7,1.0-(CtrlDown||CKeyDown||(selectedpatch!=-1))*0.4);//按下ctrl显示环
						CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
						pDoc->m_FlowComplex->Draw2cell(*p2cell);
						if(pMain->m_ctrlPaneFCCR->m_dialog.triboundary)
							pDoc->m_FlowComplex->DrawTriangleBoundary(*p2cell);

						if(pMain->m_ctrlPaneFCCR->m_dialog._2cellboundary)
							pDoc->m_FlowComplex->Draw2cellBoundary(*p2cell);
					}//j
				}
			}
		}//showThicken

		if(pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult)
		{
			if(selectedpatch!=-1)
			{
				CP_Patch *pPatch = pDoc->m_FlowComplex->m_patches[selectedpatch];
				for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
				{
					glColor4f(pDoc->r[2],pDoc->g[2],pDoc->b[2],1.0);
					CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
					pDoc->m_FlowComplex->Draw2cell(*p2cell);
					glColor4f(0.1,0.6,0.1,1.0);
					glLineWidth(1.0f);
					if(pMain->m_ctrlPaneFCCR->m_dialog.triboundary)
						pDoc->m_FlowComplex->DrawTriangleBoundary(*p2cell);
				}
				pDoc->m_FlowComplex->DrawPatchBoundary(*pPatch,CtrlDown,CKeyDown,which,pMain->m_ctrlPaneFCCR->m_dialog.showRMF);
			}

			if(pMain->m_ctrlPaneFCCR->m_dialog.showinteriorpatches)
			{
				for(unsigned int i=0;i<pDoc->m_FlowComplex->interior_patches.size();++i){
					int _patch=pDoc->m_FlowComplex->interior_patches[i];
					CP_Patch *pPatch = pDoc->m_FlowComplex->m_patches[_patch];
					for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
					{
						glColor4f(1.0,0.5,0.5,0.6);
						CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
						pDoc->m_FlowComplex->Draw2cell(*p2cell);
					}
					pDoc->m_FlowComplex->DrawPatchBoundary(*pPatch,true,CKeyDown,which,pMain->m_ctrlPaneFCCR->m_dialog.showRMF);
				}
			}

			for(unsigned int i=0;i<pDoc->m_FlowComplex->m_patches.size();i++)
			{
				CP_Patch *pPatch = pDoc->m_FlowComplex->m_patches[i];
				if(pPatch->flag&&i!=selectedpatch){
					for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
					{
						CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
						if(pMain->m_ctrlPaneFCCR->m_dialog.selectpatchEnable||pMain->m_ctrlPaneFCCR->m_dialog.showinteriorpatches)
						{//突出选取的patch
							if(selectedpatch==-1&&!pMain->m_ctrlPaneFCCR->m_dialog.showinteriorpatches)
								glColor4f(pDoc->r[pPatch->color],pDoc->g[pPatch->color],pDoc->b[pPatch->color],1.0);
							else if(selectedpatch!=-1||pMain->m_ctrlPaneFCCR->m_dialog.showinteriorpatches)
								glColor4f(0.7,0.7,0.7,1.0-(CtrlDown||CKeyDown||(selectedpatch!=-1)||pMain->m_ctrlPaneFCCR->m_dialog.showinteriorpatches)*0.4);//按下ctrl显示环

						}else if(pMain->m_ctrlPaneFCCR->m_dialog.select2cellEnable)
						{//突出选取的2cell
							if(selected2cell==-1||selected2cell==p2cell->index)
								glColor4f(pDoc->r[pPatch->color],pDoc->g[pPatch->color],pDoc->b[pPatch->color],1.0);
							else
								glColor4f(0.7,0.7,0.7,1.0);
						}else{
							if(pMain->m_ctrlPaneFCCR->m_dialog.fccr.IsProcess&&pMain->m_ctrlPaneFCCR->m_dialog.randomColor&&!pPatch->wrong)//错误的面片显示为白色
								glColor4f(pDoc->r[pPatch->color],pDoc->g[pPatch->color],pDoc->b[pPatch->color],1.0);
							else
								glColor4f(0.7,0.7,0.7,1.0);
						}
						pDoc->m_FlowComplex->Draw2cell(*p2cell);
						//边界
						if(pMain->m_ctrlPaneFCCR->m_dialog.triboundary){
							glColor4f(0.0,0.0,0.0,1.0);
							pDoc->m_FlowComplex->DrawTriangleBoundary(*p2cell);
						}

						if(pMain->m_ctrlPaneFCCR->m_dialog._2cellboundary)
							pDoc->m_FlowComplex->Draw2cellBoundary(*p2cell);
					}//j

					if(pMain->m_ctrlPaneFCCR->m_dialog.patchboundary)
						pDoc->m_FlowComplex->DrawPatchBoundary(*pPatch);
				}//flag
			}//i

			if(pMain->m_ctrlPaneFCCR->m_dialog.selpatch!=-1)
			{
				CP_Patch *pPatch = pDoc->m_FlowComplex->m_patches[pMain->m_ctrlPaneFCCR->m_dialog.selpatch];
				pDoc->m_FlowComplex->DrawPatchBoundary(*pPatch);
			}
			if(pMain->m_ctrlPaneFCCR->m_dialog.sel2cell!=-1)
			{
				CP_2cell *p2cell=pDoc->m_FlowComplex->m_2cells[pDoc->m_FlowComplex->Locate2cell(pMain->m_ctrlPaneFCCR->m_dialog.sel2cell)];
				pDoc->m_FlowComplex->Draw2cellBoundary(*p2cell);
			}
		}//showResult
		

		//pruning
		if(showPruning){
			glColor3f(0.9, 0.0, 0.0);
			glLineWidth(4);
			for(unsigned int i=0;i<pDoc->m_FlowComplex->cycles.size();++i){
				for(unsigned int j=0;j<pDoc->m_FlowComplex->cycles[i].size();++j){
					pDoc->m_FlowComplex->m_PolyLine[pDoc->m_FlowComplex->cycles[i][j]].Draw();
				}
			}
		}

		//polyline
		if(showFCEnable||pMain->m_ctrlPaneFCCR->m_dialog.fccr.IsProcess)
		{

			for (unsigned int i = 0; i < pDoc->m_FlowComplex->m_PolyLine.size(); i++)
			{
				if(selectedpoly==i||moveonpoly==i){
					glColor4f(0.33, 0.47, 0.93,1.0);
					glLineWidth(4);
				}
				else{
					glColor4f(0.0, 0.0, 0.0,1.0);
					glLineWidth(3.0);
				}
				if(!pMain->m_ctrlPaneFCCR->m_dialog.showpoly){
					if(!pDoc->m_FlowComplex->m_PolyLine[i].tag&&(moveonpoly!=i)&&(selectedpoly!=i))
					{	
						glColor4f(0.0, 1.0, 0.0,1.0);

					}
					pDoc->m_FlowComplex->m_PolyLine[i].Draw();
				}
			}
		}else
		{
			//没有折线预处理时显示原始线框
			for (unsigned int i = 0; i < pDoc->VT_PolyLine->size(); i++)
			{
				if(selectedpoly==i||moveonpoly==i){
					glColor4f(0.33, 0.47, 0.93,1.0);
					glLineWidth(4);
				}
				else{
					glColor4f(0.0, 0.0, 0.0,1.0); 
					glLineWidth(2.0);
				}/*修改模型，polyline中根据cut输出新模型*/
				(*pDoc->VT_PolyLine)[i].Draw();
			}
		}

		if(pMain->m_ctrlPaneFCCR->m_dialog.edgeByDegree)
		{//2cell只在边界处相交，因为在非边界相交的地方degree==1会被消去
			glDepthMask(GL_FALSE);
			glEnable (GL_LINE_SMOOTH);
			glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
			for (unsigned int i = 0; i <pDoc->m_FlowComplex->m_1cells.size(); i++)
			{
				CurveSegment* pcurve=pDoc->m_FlowComplex->m_1cells[i];
				glLineWidth(3);
				//if (pcurve->degree>2&&!pcurve->isBoundary)
				if (pcurve->newdegree>2/*(int)pcurve->incidentpatch.size()>2&&pcurve->isBoundary==-1*/)
				{ 
					glColor3f(0.173, 0.51, 1.0);
					glBegin(GL_LINE_STRIP);
					glNormal3f(0, 0, 1);
					glVertex3d(pDoc->m_FlowComplex->m_0cells[pcurve->sp].m_x,pDoc->m_FlowComplex->m_0cells[pcurve->sp].m_y,pDoc->m_FlowComplex->m_0cells[pcurve->sp].m_z);
					glVertex3d(pDoc->m_FlowComplex->m_0cells[pcurve->ep].m_x,pDoc->m_FlowComplex->m_0cells[pcurve->ep].m_y,pDoc->m_FlowComplex->m_0cells[pcurve->ep].m_z);
					glEnd();
				}else if(pcurve->newdegree==1){
					glColor3f(1.0, 0.447, 0.463);
					glBegin(GL_LINE_STRIP);
					glNormal3f(0, 0, 1);
					glVertex3d(pDoc->m_FlowComplex->m_0cells[pcurve->sp].m_x,pDoc->m_FlowComplex->m_0cells[pcurve->sp].m_y,pDoc->m_FlowComplex->m_0cells[pcurve->sp].m_z);
					glVertex3d(pDoc->m_FlowComplex->m_0cells[pcurve->ep].m_x,pDoc->m_FlowComplex->m_0cells[pcurve->ep].m_y,pDoc->m_FlowComplex->m_0cells[pcurve->ep].m_z);
					glEnd();
				}

			}//i

			glDepthMask(GL_TRUE);
		}

		drawData();
	}//curve

	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	
	glPopMatrix();

	SwapBuffers(pDC->m_hDC);
	wglMakeCurrent(NULL, NULL);
	
}

void CSLDRView::SetModelViewMatrix() 
{
	CBoundBox2D box = GetDocument()->m_boundBox;
	glTranslatef(m_xPos, m_yPos, 0.0f);//先平移后旋转
	glMultMatrixf(ArcBall->Transform.M); 
	glTranslated(-box.m_centerPt.m_x, -box.m_centerPt.m_y, -box.m_centerPt.m_z);
}


void CSLDRView::InitMaterial() {
	GLfloat matAmbient[] = {0.8, 0.8, 0.8, 1.0};
	GLfloat matDiffuse[]   = {mDiffuse, mDiffuse, mDiffuse, 1.0};
	GLfloat matAmbDif[]   = {0.5, 0.5, 0.5, 1.0};
	GLfloat matSpecular[] = {mSpecular, mSpecular, mSpecular, 1.0};
	GLfloat shine[] = {mShine}; 
	glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse); 
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbDif);  
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shine);
}

void CSLDRView::InitLight0() {
	const GLfloat pos = 400;
	GLfloat light_ambient [] = { mAmbient0, mAmbient0, mAmbient0, 1.0 };
	GLfloat light_diffuse [] = { mDiffuse0, mDiffuse0, mDiffuse0, 1.0 };
	GLfloat light_specular[] = { mSpecular0, mSpecular0, mSpecular0, 1.0 };
	GLfloat light_position[] = { 0.0, 0, 250, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT , light_ambient );
	glLightfv(GL_LIGHT0, GL_DIFFUSE , light_diffuse );
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

}

void CSLDRView::InitLight1() {
	const GLfloat pos = 200; 
	GLfloat light_ambient [] = { mAmbient1, mAmbient1, mAmbient1, 1.0 };
	GLfloat light_diffuse [] = { mDiffuse1, mDiffuse1, mDiffuse1, 1.0 };
	GLfloat light_specular[] = { mSpecular1, mSpecular1, mSpecular1, 1.0 };
	GLfloat light_position[] = { 0, pos,0 , 0.0 };
	glLightfv(GL_LIGHT1, GL_POSITION, light_position);
	glLightfv(GL_LIGHT1, GL_AMBIENT , light_ambient );
	glLightfv(GL_LIGHT1, GL_DIFFUSE , light_diffuse );
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
}

void CSLDRView::InitLight2() {
	const GLfloat pos = 300; 
	GLfloat light_ambient [] = { 0.05, 0.05, 0.05, 1.0 };
	GLfloat light_diffuse [] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat light_specular[] = {mSpecular2, mSpecular2, mSpecular2, 1.0 };
	GLfloat light_position[] = { pos, pos, pos, 1.0 };
	float spotlightDirection[]={-1.0f,-1.0f,-1.0f};  //聚光灯方向
	glLightfv(GL_LIGHT2, GL_POSITION, light_position);
	glLightfv(GL_LIGHT2, GL_AMBIENT , light_ambient );
	glLightfv(GL_LIGHT2, GL_DIFFUSE , light_diffuse );
	glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
	glLightf(GL_LIGHT2,GL_SPOT_CUTOFF,mCutOff2);
	glLightf(GL_LIGHT2,GL_SPOT_EXPONENT,mExponent2); //聚光灯指数
	glLightfv(GL_LIGHT2,GL_SPOT_DIRECTION,spotlightDirection); //聚光灯方向
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
	ArcBall->isRClicked = false;
	ArcBall->upstate();

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

	CClientDC dc(this);
	wglMakeCurrent(dc.m_hDC, m_hRC);

	CRect view_rect;
	GetClientRect(view_rect);
	swidth = view_rect.Width();
	sheight = view_rect.Height();
	ArcBall->setBounds(swidth,sheight);

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
	CSLDRDoc* pDoc = GetDocument();
	
	int step = 5;

	BOOL bCtrlPressed = ::GetKeyState(VK_CONTROL) & 0Xf0;
	if(bCtrlPressed)
	{
		 CtrlDown=true;
	}

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
	case VK_SHIFT: shiftDown=true;
		break;
	case 'C':    CKeyDown=true;
		break;
	case 'K':    which--;
		         if(which<0)which=0;
		break;
	case 'I':    which++;
		         if(selectedpatch!=-1&&which>=(int)pDoc->m_FlowComplex->m_patches[selectedpatch]->cycle.size()-1)
					 which=(int)pDoc->m_FlowComplex->m_patches[selectedpatch]->cycle.size()-1;
		break;
	case 'M':
		CSLDRDoc* pDoc = GetDocument();
		pDoc->m_FlowComplex->TopologyComplete();
		pDoc->m_FlowComplex->ProcessingInteriorPatch();
		break;
	}
	
	this->Invalidate();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
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
	ArcBall->MousePt.s.X = point.x;
	ArcBall->MousePt.s.Y = point.y;
	ArcBall->upstate();

	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	if (pMain->GetCursor() == SEL)
	{
		FindSelNode(point);
	}

	if(shiftDown)
	{//选择曲线
		FindPolyline(point);
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

void CSLDRView::FindPolyline(CPoint point)
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
	
	glLineWidth(2);
	if(pMain->m_ctrlPaneFCCR->m_dialog.fccr.IsProcess)
	{
		for (unsigned int i = 0; i < pDoc->m_FlowComplex->m_PolyLine.size(); i++)
		{
			glPushName(i);
			pDoc->m_FlowComplex->m_PolyLine[i].Draw();
			glPopName();
		}
	}else
	{
		//没有折线预处理时显示原始线框
		for (unsigned int i = 0; i < pDoc->VT_PolyLine->size(); i++)
		{
			glPushName(i);
			(*pDoc->VT_PolyLine)[i].Draw();
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

void CSLDRView::Find2cell(CPoint point)
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
	for(unsigned int i=0;i<pDoc->m_FlowComplex->m_patches.size();i++)
	{
		CP_Patch *pPatch = pDoc->m_FlowComplex->m_patches[i];
		if(pPatch->flag){
			for (unsigned int j = 0; j <pPatch->m_2cells.size(); j++)
			{
				CP_2cell *p2cell = pDoc->m_FlowComplex->m_2cells[pPatch->m_2cells[j]];
				glPushName(p2cell->index);
				pDoc->m_FlowComplex->Draw2cell(*p2cell);
				glPopName();
			}//j
		}
	}//i

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

void CSLDRView::FindPatch(CPoint point)
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
	if(pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult){
		for(unsigned int i=0;i<pDoc->m_FlowComplex->m_patches.size();i++)
		{
			CP_Patch *pPatch = pDoc->m_FlowComplex->m_patches[i];
			if(pPatch->flag){
				glPushName(i);
				pDoc->m_FlowComplex->DrawPatch(*pDoc->m_FlowComplex->m_patches[i]);
				glPopName();
			}
		}//i
	}else if(showCycles){
		for(int i=0;i<pDoc->m_FlowComplex->oripatches;i++)
		{
			if(pDoc->m_FlowComplex->m_patches[i]->flag){
				glPushName(i);
				pDoc->m_FlowComplex->DrawPatch(*pDoc->m_FlowComplex->m_patches[i]);
				glPopName();
			}
		}//i
		for(unsigned int i=0;i<pDoc->m_FlowComplex->connectedPatches.size();++i){
			int _patch=pDoc->m_FlowComplex->connectedPatches[i];
			glPushName(_patch);
			pDoc->m_FlowComplex->DrawPatch(*pDoc->m_FlowComplex->m_patches[_patch]);
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
		int i;
		unsigned int j;
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
			if(pMain->m_ctrlPaneFCCR->m_dialog.fccr.IsProcess&&pMain->m_ctrlPaneFCCR->m_dialog.select2cellEnable)
				selected2cell=*ptr;
			else if(showPruningEnable&&pMain->m_ctrlPaneFCCR->m_dialog.selectpatchEnable)
				selectedpatch=*ptr;
			else if(shiftDown){
				//selectedpoly=*ptr;
				moveonpoly=*ptr;
				//cout<<selectedpoly<<endl;
			}
		}
		//printf ("\n");
	} else
	{
		GetDocument()->m_mouseSelEdge[0] = -1;
		GetDocument()->m_mouseSelEdge[1] = -1;
	}
	Invalidate(TRUE);
}

void CSLDRView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ArcBall->isClicked = true;
	ArcBall->MousePt.s.X = point.x;
	ArcBall->MousePt.s.Y = point.y;
	ArcBall->upstate();

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

	if (pMain->m_ctrlPaneFCCR)//指针非空时才可判断选取
	{
		if(pMain->m_ctrlPaneFCCR->m_dialog.fccr.IsProcess&&pMain->m_ctrlPaneFCCR->m_dialog.select2cellEnable)
		{
			Find2cell(point);
		}
		else if(showPruningEnable&&pMain->m_ctrlPaneFCCR->m_dialog.selectpatchEnable)
		{
			FindPatch(point);
		}
		if(shiftDown&&moveonpoly!=-1)
		{//当ctrl并移动到曲线上点击后才确认选中
			selectedpoly=moveonpoly;
			pMain->m_ctrlPaneFCCR->m_dialog.selpoly=selectedpoly;
			cout<<selectedpoly<<":"<<endl;
			int s=pDoc->m_FlowComplex->m_PolyLine[selectedpoly].s;
			cout<<s<<":"<<pDoc->m_FlowComplex->m_0cells[s].m_x<<" "<<pDoc->m_FlowComplex->m_0cells[s].m_y<<" "<<pDoc->m_FlowComplex->m_0cells[s].m_z<<endl;
			int e=pDoc->m_FlowComplex->m_PolyLine[selectedpoly].e;
			cout<<e<<":"<<pDoc->m_FlowComplex->m_0cells[e].m_x<<" "<<pDoc->m_FlowComplex->m_0cells[e].m_y<<" "<<pDoc->m_FlowComplex->m_0cells[e].m_z<<endl;

			pDoc->m_FlowComplex->GenerateCycle(selectedpoly);
		}

		if(selected2cell!=-1)
		{
			CP_2cell* p2cell=pDoc->m_FlowComplex->m_2cells[pDoc->m_FlowComplex->Locate2cell(selected2cell)];
			
			pMain->m_ctrlPaneFCCR->m_dialog.SetTreeItems(selected2cell);//2cell在集合中的下标，非标号
			for(auto e:p2cell->delaunytri)
				cout<<e<<",";
			cout<<endl;
		}
		else if(selectedpatch!=-1)
		{
			which=0;
			pMain->m_ctrlPaneFCCR->m_dialog.SetTreePatch(selectedpatch);//2cell在集合中的下标，非标号
			if(shiftDown){
				pDoc->m_FlowComplex->temporaryfalse.push_back(selectedpatch);
			    pDoc->m_FlowComplex->m_patches[selectedpatch]->flag=false;
			}
			//pDoc->m_FlowComplex->PatchNormalConsensus(selectedpatch);
			cout<<selectedpatch<<endl;
			for(unsigned int i=0;i<pDoc->m_FlowComplex->m_patches[selectedpatch]->m_boundary.size();++i){
				if(pDoc->m_FlowComplex->m_patches[selectedpatch]->m_boundary[i]->isBoundary!=-1)
				cout<<pDoc->m_FlowComplex->m_patches[selectedpatch]->m_boundary[i]->newdegree<<",";
			}//i
			cout<<endl;
		}
	}

	CView::OnLButtonDown(nFlags, point);
}


double dist(const CP_Point3D &x,const CP_Point3D &y)
{
	return sqrt((x.m_x - y.m_x)*(x.m_x - y.m_x) + (x.m_y - y.m_y)*(x.m_y - y.m_y) + (x.m_z - y.m_z)*(x.m_z - y.m_z));
}

double Area(double a, double b, double c)
{//海伦公式计算三角形面积
	double s = (a+b+c)/2;
	return (double)sqrt(s*(s-a)*(s-b)*(s-c));
}

BOOL CSLDRView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_Scale += 0.05 *zDelta/2 ;
	if(m_Scale<0)
		m_Scale=0.2;
	InvalidateRect(NULL, FALSE);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CSLDRView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ArcBall->isClicked = false;
	ArcBall->upstate();

	m_MouseDownPoint = CPoint(0, 0);
	ReleaseCapture();
	CView::OnLButtonUp(nFlags, point);
}


void CSLDRView::OnPolyline()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	if(pDoc->format=="curve")
	{
		if(!showFCEnable){
			fcEnable=false;
			ifcEnable=false;
			pMain->m_ctrlPaneFCCR->m_dialog.OnImprovedFCPolyline();
		}else{
			if(showCycles||showPruning||showFC||showTop)
			{
				showCycles=false;
				showPruning=false;
				showFC=false;
				showTop=false;
			}
			if(showInputP)
			{
				showInputP=false;
				pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=true;
				pMain->m_ctrlPaneFCCR->m_dialog.SetTreePatches();
			}else
			{
				showInputP=true;
				pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=false;
			}
		}
	}else
		MessageBox("请先打开文件或清空后再操作");
	Invalidate(); 
}

void CSLDRView::OnFlowcomplex()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	if(!showMergeEnable){
		pMain->m_ctrlPaneFCCR->m_dialog.OnImprovedFCFlowComplex();
	}else{
		if(showInputP||showPruning||showCycles||showTop)
		{
			showInputP=false;
			showPruning=false;
			showCycles=false;
			showTop=false;
		}
		if(showFC)
		{
			showFC=false;
			pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=true;
			pMain->m_ctrlPaneFCCR->m_dialog.SetTreePatches();
		}else
		{
			showFC=true;
			pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=false;
		}
	}
	Invalidate(); 
}


void CSLDRView::OnSpreadAndMerge()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	if(!showCyclesEnable){
		pMain->m_ctrlPaneFCCR->m_dialog.OnImprovedFCSpreadMerge();
	}else{
		if(showInputP||showPruning||showFC||showCycles)
		{
			showInputP=false;
			showPruning=false;
			showFC=false;
			showCycles=false;
		}
		if(showTop)
		{
			showTop=false;
			pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=true;
			pMain->m_ctrlPaneFCCR->m_dialog.SetTreePatches();
		}else
		{
			showTop=true;
			pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=false;
		}
	}
	Invalidate(); 
}


void CSLDRView::OnFindingCycles()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	if(!showPruningEnable){
		pMain->m_ctrlPaneFCCR->m_dialog.OnImprovedFCFindingCycles();
	}else{
		if(showInputP||showPruning||showFC||showTop)
		{
			showInputP=false;
			showPruning=false;
			showFC=false;
			showTop=false;
		}
		if(showCycles)
		{
			showCycles=false;
			pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=true;
			pMain->m_ctrlPaneFCCR->m_dialog.SetTreePatches();
		}else
		{
			showCycles=true;
			pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=false;
		}
	}
	Invalidate(); 
}


void CSLDRView::OnPruning()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	if(!pMain->m_ctrlPaneFCCR->m_dialog.fccr.IsProcess){
		showCycles=false;
		pMain->m_ctrlPaneFCCR->m_dialog.OnImprovedFCPruning();
	}else{
		if(showInputP||showCycles||showFC||showTop)
		{
			showInputP=false;
			showCycles=false;
			showFC=false;
			showTop=false;
		}
		if(showPruning)
		{
			showPruning=false;
			pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=true;
			pMain->m_ctrlPaneFCCR->m_dialog.SetTreePatches();
		}else
		{
			showPruning=true;
			pMain->m_ctrlPaneFCCR->m_dialog.fccr.showResult=false;
		}
	}
	Invalidate(); 
}

void CSLDRView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CView::OnKeyUp(nChar, nRepCnt, nFlags);

	BOOL bCtrlPressed = ::GetKeyState(VK_CONTROL) & 0Xf0;
	if(!bCtrlPressed)
	{
		CtrlDown=false;
	}

	switch (nChar)
	{
	case VK_SHIFT:		shiftDown=false;
						moveonpoly=-1;
						break;
	case 'C':           CKeyDown=false;
		                break;
	}
	
	this->Invalidate();
}

void CSLDRView::OnFCReconstruction()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	if(pDoc->format=="curve"&&!pMain->m_ctrlPaneFCCR->m_dialog.fccr.IsProcess)
	{
		fcEnable=false;
		ifcEnable=false;
		showPolyEnable=false;
		pMain->m_ctrlPaneFCCR->m_dialog.OnButtonReconstruction();
	}else
		MessageBox("请先打开文件或清空后再操作");
}

void CSLDRView::OnUpdatePolyline(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(showPolyEnable);
	pCmdUI->SetCheck(showInputP);
}

void CSLDRView::OnUpdateFlowcomplex(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if(!showFCEnable)
		pCmdUI->Enable(showFCEnable);
	else
		pCmdUI->SetCheck(showFC);
}

void CSLDRView::OnUpdateCollapse(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if(!showMergeEnable)
		pCmdUI->Enable(showMergeEnable);
	else
		pCmdUI->SetCheck(showTop);
}

void CSLDRView::OnUpdateButton2(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if(!showCyclesEnable)
		pCmdUI->Enable(showCyclesEnable);
	else
		pCmdUI->SetCheck(showCycles);
}

void CSLDRView::OnUpdateDelauny(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if(!showPruningEnable)
		pCmdUI->Enable(showPruningEnable);
	else
		pCmdUI->SetCheck(showPruning);
}

void CSLDRView::OnSliderSpecular()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_SPECULAR));
	int position=pSlider->GetPos();
	mSpecular=position/100.0;
	Invalidate();
}

void CSLDRView::OnSliderDiffuse()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_Diffuse));
	int position=pSlider->GetPos();
	mDiffuse=position/100.0;
	Invalidate();
}

void CSLDRView::OnSliderShininess()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_SHININESS));
	int position=pSlider->GetPos();
	mShine=position;
	Invalidate();
}


void CSLDRView::OnCheckLight0()
{
	// TODO: 在此添加命令处理程序代码
	light0=!light0;
	Invalidate();
}


void CSLDRView::OnCheckLight1()
{
	// TODO: 在此添加命令处理程序代码
	light1=!light1;
	Invalidate();
}


void CSLDRView::OnCheckLight2()
{
	// TODO: 在此添加命令处理程序代码
	light2=!light2;
	Invalidate();
}


void CSLDRView::OnUpdateCheckLight0(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(light0);
}


void CSLDRView::OnUpdateCheckLight1(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(light1);
}


void CSLDRView::OnUpdateCheckLight2(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(light2);
}


void CSLDRView::OnSliderAmbient0()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_AMBIENT0));
	int position=pSlider->GetPos();
	cout<<"ambient0:"<<position<<endl;
	mAmbient0=position/100.0;
	Invalidate();
}


void CSLDRView::OnSliderSpecular0()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_SPECULAR0));
	int position=pSlider->GetPos();
	cout<<"specular0:"<<position<<endl;
	mSpecular0=position/100.0;
	Invalidate();
}


void CSLDRView::OnSliderDiffuse0()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_DIFFUSE0));
	int position=pSlider->GetPos();
	cout<<"diffuse0:"<<position<<endl;
	mDiffuse0=position/100.0;
	Invalidate();
}


void CSLDRView::OnSliderAmbient1()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_AMBIENT1));
	int position=pSlider->GetPos();
	cout<<"ambient1:"<<position<<endl;
	mAmbient1=position/100.0;
	Invalidate();
}


void CSLDRView::OnSliderSpecular1()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_SPECULAR1));
	int position=pSlider->GetPos();
	cout<<"specular1:"<<position<<endl;
	mSpecular1=position/100.0;
	Invalidate();
}


void CSLDRView::OnSliderDiffuse1()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_DIFFUSE1));
	int position=pSlider->GetPos();
	cout<<"diffuse1:"<<position<<endl;
	mDiffuse1=position/100.0;
	Invalidate();
}

//void CSLDRView::OnRButtonDown(UINT nFlags, CPoint point)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//	
//	CView::OnRButtonDown(nFlags, point);
//}

void CSLDRView::OnSliderSpecular2()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_SPECULAR2));
	int position=pSlider->GetPos();
	cout<<"specular2:"<<position<<endl;
	mSpecular2=position/100.0;
	Invalidate();
}

void CSLDRView::OnSliderCutoff2()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_CUTOFF2));
	int position=pSlider->GetPos();
	cout<<"cutoff2:"<<position<<endl;
	mCutOff2=position;
	Invalidate();
}


void CSLDRView::OnSliderExponent2()
{
	// TODO: 在此添加命令处理程序代码
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, pMain->m_wndRibbonBar.FindByID(ID_SLIDER_EXPONENT2));
	int position=pSlider->GetPos();
	cout<<"exponent2:"<<position<<endl;
	mExponent2=position;
	Invalidate();
}


void CSLDRView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	ArcBall->isRClicked = true;
	ArcBall->MousePt.s.X = point.x;
	ArcBall->MousePt.s.Y = point.y;
	ArcBall->upstate();

	for(auto i:pDoc->m_FlowComplex->temporaryfalse)
		pDoc->m_FlowComplex->m_patches[i]->flag=true;
	vector<int>().swap(pDoc->m_FlowComplex->temporaryfalse);
	selectedpatch=-1;
	selected2cell=-1;
	selectedpoly=-1;
	pMain->m_ctrlPaneFCCR->m_dialog.selpatch=-1;
	Invalidate();
	//CView::OnRButtonDown(nFlags, point);
}

void CSLDRView::OnUpdateFCReconstruction(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(fcEnable);
}

void CSLDRView::OnButtonImprovedfc()
{
	// 与FC方法大致相同，修改其中的一些步骤 
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	CSLDRDoc* pDoc = GetDocument();
	if(pDoc->format=="curve"&&!pMain->m_ctrlPaneFCCR->m_dialog.fccr.IsProcess)
	{
		fcEnable=false;
		ifcEnable=false;
		showPolyEnable=false;
		pMain->m_ctrlPaneFCCR->m_dialog.OnImprovedFCReconstruction();
	}else
		MessageBox("请先打开文件或清空后再操作");
}

void CSLDRView::OnUpdateButtonImprovedfc(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(ifcEnable);
}


void CSLDRView::OnButtonPreprocess()
{
	// TODO: 在此添加命令处理程序代码
	CSLDRDoc* pDoc = GetDocument();
	SurfaceOptimization surface_optimization(pDoc->m_FlowComplex);
	surface_optimization.ElimateDegenerateTriangles();
	Invalidate();
}


void CSLDRView::OnUpdateButtonPreprocess(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(showPreprocessEnable);
}


void CSLDRView::OnCheckTwoside()
{
	// TODO: 在此添加命令处理程序代码
	twoSide=!twoSide;
	Invalidate();
}


void CSLDRView::OnUpdateCheckTwoside(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(twoSide);
}
