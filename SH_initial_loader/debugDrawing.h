#ifndef  __DEBUG_DRAWING_H__
#define  __DEBUG_DRAWING_H__

#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include <stdio.h>


#include "vertex.h"
//#include "quat.h"
#include "matrix.h"

extern GLuint nFontList;

static void drawArrow( vertex *start, vertex *finish )
{
	vertex dir1 = (*start) - (*finish);
	vertex sideX,sideY;
//LogFile(ERROR_LOG,"Dir = (%f, %f, %f)",dir1.x,dir1.y,dir1.z);
	dir1.norm( );
	makePerpAxis( dir1, &sideX, &sideY );

	sideX = (sideX + dir1 ) * 0.5f;
	sideY = (sideY + dir1 ) * 0.5f;

	glBegin( GL_LINES );

	glVertex3fv( &(start->x) );
	glVertex3fv( &(finish->x) );

	dir1 = sideX + (*finish);
	glVertex3fv( &(finish->x) );
	glVertex3fv( &(dir1.x) );

	dir1 = sideY + (*finish);
	glVertex3fv( &(finish->x) );
	glVertex3fv( &(dir1.x) );

	glEnd();
}



static void drawBasisMatrix( matrix *m, float mScale, int colorSet = 0, long matrixNum = -1 )
{
	float cSets [][3]={	{ 1.0f, 0.0f, 0.0f },
						{ 1.0f, 1.0f, 0.0f },
						{ 0.0f, 0.0f, 1.0f },
						{ 1.0f, 0.0f, 1.0f },
						{ 0.0f, 1.0f, 0.0f },
						{ 0.0f, 1.0f, 1.0f }};
	char text[5];
	int cSet = colorSet % 2;

	m->transpose( );
	vertex pos = vertex( m->getRow( 3 ) );
	m->transpose( );
	vertex xAxis = vertex( m->getRow( 0 ) );
	vertex yAxis = vertex( m->getRow( 1 ) );
	vertex zAxis = vertex( m->getRow( 2 ) );
	vertex endPoint;

	xAxis *= mScale;
	yAxis *= mScale;
	zAxis *= mScale;
//LogFile(ERROR_LOG,"POS=(%f,%f,%f)",pos.x,pos.y,pos.z);
	glDisable( GL_TEXTURE_2D );

//	glLineWidth( mScale * .5f );
//	glColor3f( 0.0f, 0.0f, 0.0f );
	endPoint = pos + xAxis;
//	drawArrow( &pos, &endPoint );

	glLineWidth( mScale * .4f );
	glColor3fv( cSets[ cSet * 3 ] );
	drawArrow( &pos, &endPoint );

//	glLineWidth( mScale * .5f );
//	glColor3f( 0.0f, 0.0f, 0.0f );
	endPoint = pos + yAxis;
//	drawArrow( &pos, &endPoint );

	glLineWidth( mScale * .4f );
	glColor3fv( cSets[ cSet * 3 + 1] );
	drawArrow( &pos, &endPoint );
	
//	glLineWidth( mScale * .5f );
//	glColor3f( 0.0f, 0.0f, 0.0f );
	endPoint = pos + zAxis;
//	drawArrow( &pos, &endPoint );

	glLineWidth( mScale * .4f );
	glColor3fv( cSets[ cSet * 3 + 2] );
	drawArrow( &pos, &endPoint );

	if( matrixNum != -1 )
	{
		glRasterPos3f(pos.x,pos.y+10.0f,pos.z);
		sprintf( text,"%4ld",matrixNum);
		glListBase(nFontList);
		glCallLists (strlen(text), GL_UNSIGNED_BYTE, text);
	}

	glEnable( GL_TEXTURE_2D );
}


#endif /*__DEBUG_DRAWING_H__*/