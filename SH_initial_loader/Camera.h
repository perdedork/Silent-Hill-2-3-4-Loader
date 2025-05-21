#ifndef CAMERA_H
#define CAMERA_H

//#include "glut.h"
#include <windows.h>
#ifndef __INTRIN_H_
//#include <intrin.h>
#endif /*__INTRIN_H_*/
#include <gl\gl.h>
#include <gl\glu.h>

#include "vertex.h"
#include "matrix.h"
#include "quat.h"
#include "typedefs.h"
#include "IBaseTypes.h"

/*************************************************************************/
/*  Camera.H rev2.0  -Now uses left handed system completely.            */
/*    This class acts as a "plug-in" GLUT view handler.  The GLUT call-  */
/* back functions are called w/ the appropriate member functions of this */
/* class.  Note that they are all static, so this an obj of this class   */
/* never needs to be instantiated.  This class handles the mouse and key-*/
/* board input, as well as defining some of the rendering order.         */
/*                                                                       */
/* ->REVISION NOTES                                                      */
/*  -Rev 2.0  :-Changed camera system to speed up rendering, and for use */
/*              w/ the "octtree" class.  This ultimately will be changed */
/*              to allow for independent cameras, removing static parts  */
/*              of the class.                                            */
/*  -Rev 2.1  :-Removed 'statc' state of vars and funcs.                 */
/*             -Changed it from Glut-specific to general                 */
/*             -Changed it to a Right handed system                      */
/*************************************************************************/

const float CM_MIN_MOUSE_SPEED =      1.0f;
const float CM_MAX_MOUSE_SPEED =     10.0f;
const float CM_MIN_KEYPAD_SPEED=      1.0f;
const float CM_MAX_KEYPAD_SPEED=     20.0f;

class Camera{
public:
	Camera();
	~Camera(){}

    void mouseSpeedX(float scale);  //Changes the mouse sensetivity
    void keypadSpeed(float scale);  //Changes the movement speed
	inline   void reset(){setVertex(from,0,0,0);setVertex(viewAxis,1,0,0);
							if( invertUp ) setVertex(up,0,-1,0); else setVertex(up,0,1,0);
							setVertex(at,0,0,1);xAng=yAng=0;}
	void createCamView( float viewAng, float aspectRat, float nearPlane, float farPlane, bool _invertUp = false );   //Sets up perspective proj matrix, as well as values for the view cone
  inline   void setCam(float fX,float fY,float fZ,float aX,float aY,float aZ,float uX,float uY,float uZ)
                   {vertex tAt;setVertex(up,uX,uY,uZ);setVertex(from,fX,fY,fZ);setVertex(at,aX,aY,aZ);VSUBV(from,at,tAt);
					VCROSS(up,tAt,viewAxis);VUnit(&viewAxis);}
    void moveFB(float mRate);
    void moveLR(float mRate);
    void rotateView(int xRot,int yRot);
 
	void movePos( float fX, float fY, float fZ );	//Changes the 'from' position of the camera, but doesn't change the direction the camera is looking
	void setPos( float fX, float fY, float fZ );	//Changes the 'from' position of the camera, and resets the view direction
	
    void camView();
 
	void setInvertUp( bool _val ){ invertUp = _val; }
 

  /*NOTE: These function only needs to be called once for the current camera,*
   *     then passed to the octtree.  Since they're pointers, when the camera*
   *     changes the values, the octtree immidiately gets the new value.     */

  //######  THIS OR THE MATRIX ONE
  //inline quat *getViewRot(){return &viewQuat;} 
  inline matrix *getViewRot(){return &viewMat;}
  inline vertex *getViewVertex(){return &from;}
  inline vertex *getAtVertex(){return &at;}
  inline void setFrustum(vertex *fNorms){frustum=fNorms;}

    int xAng,yAng;         //Keeps track of current viewing angles
    float mouseSpeed;      //Mouse sensitivity
    float keypadMoveSpeed; //Keypad movement speed
    vertex from;           //Current location of the camera
    vertex at;             //Current location camera looks at
    vertex up;             //The up vector
    vertex viewAxis;       //Axis of up-down rotation, & left-right moves
    vertex *frustum;       //Pointer to frustum in Octtree

    quat viewQuat;         //Quat for viewing rotation
    matrix viewMat;
	matrix glViewRot;	   //Specifically GL created rotation for Modelview - NO TRANSLATION	

	cone viewCone;		   //Cone representing view frustum - Only variables that are changed are the 'from' and 'dir'
	bool invertUp;


};

#endif
