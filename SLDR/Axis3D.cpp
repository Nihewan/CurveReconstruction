#include "stdafx.h"
#include "Axis3D.h"
#include "gl/GL.h"
#include "gl/GLU.h"
#include "glut.h"
#include "CBoundBox2D.h"

CAxis3D::CAxis3D(void)
{
}


CAxis3D::~CAxis3D(void)
{
}

void CAxis3D::DrawAxis(CBoundBox2D *bBox, int rotate[]) {
	double halfSize = bBox->m_width > bBox->m_height? bBox->m_width/2 : bBox->m_height/2;
	halfSize *= 1.2 * 1.2;
	double axisCenterX = halfSize + halfSize/12;
	double axisCenterY = halfSize - halfSize/2.7;
	double len = halfSize/6.5;
	double triLen = len/4;

	glLineWidth(1.5f);
	glTranslated(abs(axisCenterX), abs(axisCenterY), 0);
	//glTranslated(-5, -5, 0);

	glColor3f(1.0,0.2,0.2);
	glRotatef(rotate[0], 1.0, 0.0, 0.0);
	glRotatef(rotate[1], 0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0 + len, 0, 0);
	glEnd();
	glTranslated(len, 0, 0);
	glRasterPos2f(0, triLen/2);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'X');
	glRotatef(90, 0, 1.0, 0);
	glutSolidCone(triLen/3, triLen, 20, 1);
	glRotatef(-90, 0, 1.0, 0);
	glTranslated(-len, 0, 0);

	glColor3f(0.3,1.0,0.3);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0 + len, 0);
	glEnd();
	glTranslated(0, len, 0);
	glRasterPos2f(triLen/2, 0);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'Y');
	glRotatef(-90, 1.0, 0, 0);
	glutSolidCone(triLen/3, triLen, 20, 1);
	glRotatef(+90, 1.0, 0.0, 0);
	glTranslated(0, -len, 0);

	glColor3f(0.2,0.4,1.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, len);
	glEnd();
	glTranslated(0, 0, len);
	glRasterPos2f(0, triLen/2);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'Z');
	glRotatef(-90, 0, 0, 1.0);
	glutSolidCone(triLen/3, triLen, 20, 1);
	glRotatef(+90, 0, 0, 1.0);
	glTranslated(0, 0, -len);

	glRotatef(-rotate[1], 0.0, 1.0, 0.0);
	glRotatef(-rotate[0], 1.0, 0.0, 0.0);
	glTranslated(-abs(axisCenterX), -abs(axisCenterY), 0);
}