#include <windows.h>
#include <stdio.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "vertex.h"
#include "matrix.h"
#include "typedefs.h"
#include "mathlib.h"
//#include "glut.h"
#include "Camera.h"





Camera::Camera()
{
	xAng=0;yAng=0; 
	frustum=NULL;
	keypadMoveSpeed=CM_MIN_KEYPAD_SPEED;
	mouseSpeed=CM_MIN_MOUSE_SPEED;
	invertUp = false;
}


void Camera::createCamView( float viewAng, float aspectRat, float nearPlane, float farPlane, bool _invertUp )
{
	if( viewAng < F_EPS || aspectRat < F_EPS )
	{
		char eStr[128];
		sprintf(eStr,"ERROR: Camera::createCamView -Illegal Values: viewAng: %f  aspectRat: %f",viewAng, aspectRat);
		LogFile(ERROR_LOG,eStr);
		LogFile(ERROR_LOG,"ERROR: Camera::createCamView - set to default values: viewAng: 75.0  aspectRat: 1.0");
		viewAng = 75.0;
		aspectRat = 1.0;
	}
	viewCone.length = farPlane;
	viewCone.unitRad = tanf(TORAD((viewAng*0.5)));

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(viewAng, aspectRat, nearPlane, farPlane);
	invertUp = _invertUp;
}

void Camera::mouseSpeedX(float scale)
{
	mouseSpeed+=scale;
	if(mouseSpeed<CM_MIN_MOUSE_SPEED)            //Adjusted to match screen Hz
		mouseSpeed=CM_MIN_MOUSE_SPEED;
	else if(mouseSpeed>CM_MAX_MOUSE_SPEED)       
		mouseSpeed=CM_MAX_MOUSE_SPEED;
}

void Camera::keypadSpeed(float scale)
{
	keypadMoveSpeed+=scale;
  
	if(keypadMoveSpeed<CM_MIN_KEYPAD_SPEED)
		keypadMoveSpeed=CM_MIN_KEYPAD_SPEED;
	else if(keypadMoveSpeed>CM_MAX_KEYPAD_SPEED)
		keypadMoveSpeed=CM_MAX_KEYPAD_SPEED;
}

/*NOTE: Add in an additional scaling value to mRate to take into account the *
 *      Time step between each steps of movement,for the next 3 functions    */
void Camera::moveFB(float mRate)
{
	vertex tAt((at.x-from.x)*mRate,(at.y-from.y)*mRate,(at.z-from.z)*mRate);
	from.x+=tAt.x;
	from.y+=tAt.y;
	from.z+=tAt.z;
	at.x+=tAt.x;
	at.y+=tAt.y;
	at.z+=tAt.z;
}


void Camera::moveLR(float mRate)
{
	vertex tAt(viewAxis.x*mRate,viewAxis.y*mRate,viewAxis.z*mRate);
	from.x+=tAt.x;
	from.y+=tAt.y;
	from.z+=tAt.z;
	at.x+=tAt.x;
	at.y+=tAt.y;
	at.z+=tAt.z;
}


void Camera::rotateView(int xRot,int yRot)
{
	float xRad=TORAD(xRot),yRad=TORAD(yRot);
	//Compute the Axis Vectors
	if( invertUp )
	{
		xRad = -xRad;
		yRad = -yRad;
	}

	float xCos=cosf(xRad),yCos=cosf(yRad),xSin=sinf(xRad),ySin=sinf(yRad);

	vertex tAt(xSin*yCos,-ySin,yCos*xCos);                         //set TEMP AT
	setVertex(up,(xSin*ySin),yCos,ySin*xCos);                      //set UP
	setVertex(at,(tAt.x+from.x),(tAt.y+from.y),(tAt.z+from.z));    //Set AT

	if( invertUp )
		setVertex(viewAxis,-xCos,0,xSin);  //This replaces "VCROSS(tAt,up,viewAxis);"
	else
		setVertex(viewAxis,xCos,0,-xSin);  //This replaces "VCROSS(tAt,up,viewAxis);"

	int k;
	MIDENTITY(viewMat);

	for(k=0;k<3;k++)
	{
		viewMat.mat[k]=((float *)&viewAxis)[k];
		viewMat.mat[k+4]=((float *)&up)[k];
		viewMat.mat[k+8]=((float *)&tAt)[k];
	}
  
	if( invertUp )
	{
		viewMat.mat[ 4 ] = -viewMat.mat[ 4 ];
		viewMat.mat[ 5 ] = -viewMat.mat[ 5 ];
		viewMat.mat[ 6 ] = -viewMat.mat[ 6 ];
	}
	MTRANSPOSE(&viewMat);

}



void Camera::camView()
{ 
	vertex tempAt = at - from;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if( invertUp )
		gluLookAt( 0,0,0,tempAt.x, tempAt.y, tempAt.z, -up.x, -up.y, -up.z);
	else
		gluLookAt( 0,0,0,tempAt.x, tempAt.y, tempAt.z, up.x, up.y, up.z);
	glGetFloatv(GL_MODELVIEW_MATRIX,glViewRot.mat);
	glLoadIdentity();
	if( invertUp )
		gluLookAt( from.x,from.y,from.z,at.x,at.y,at.z, -up.x, -up.y, -up.z);
	else
		gluLookAt(from.x,from.y,from.z,at.x,at.y,at.z,up.x,up.y,up.z);
	viewCone.origin= from;
	viewCone.dir   = tempAt;
}


void Camera::movePos( float fX, float fY, float fZ )
{
	vertex tempAt = at - from;
	from = vertex( fX, fY, fZ );
	at = tempAt + from;
}

void Camera::setPos( float fX, float fY, float fZ )
{
	from = vertex( fX, fY, fZ );
	at = vertex( fX, fY, fZ + 1.0f );
	if( invertUp ) setVertex(up,0,-1,0); else setVertex(up,0,1,0);
	viewAxis = vertex( 1.0f, 0.0f, 0.0f );
	xAng = yAng = 0;
}