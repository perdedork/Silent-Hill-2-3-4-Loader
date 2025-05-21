/* math.H 
 * 
 * THIS FILE CONTAINS ALL THE MATH FUNCTIONS TO BE USED.  IT ALSO ENCOMPASSES
 * THE OLD FILE "lookup"
 */


#ifndef MATHLIB_H_
#define MATHLIB_H_

#include <windows.h>
#include <math.h>
#include "typedefs.h"


/******************************/
/*       linear algebra       */
/* matrix,vertex,plane,det,ect*/
/******************************/

//  TRIG CONSTANTS   - turn into float/long stuff
#define TORAD(x) (float)(x)*(float)0.01745329252
#define TODEG(x) (float)(x)*(float)57.2957795131




//  GENERAL MATH FUNCTIONS
int arrayMax(void *array,int numEl,e_type t); //Return index of the max element
int arrayMin(void *array,int numEl,e_type t); //return index of the min element

#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define SIGN(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

//  MATRIX FUNCTIONS



          //Faster Linear Algebra funcs 

#define BOXHALFSIZE(box,res1) VHALFSIZE((box->max),(box->min),res1)
#define TCMIDPOINT(t1,t2,tes) (res.s=(t1.s+t2.s)/2.0,res.t=(t1.t+t2.t)/2.0)

#define ABS(x) ((x)<0?-(x):(x))


#define CENTROID(v1,v2,v3,res) (res.x=(v1.x+v2.x+v3.x)/3.0,\
				res.y=(v1.y+v2.y+v3.y)/3.0,\
				res.z=(v1.z+v2.z+v3.z)/3.0)

float DISTANCE(vertex *p1,vertex *p2);
inline float D_DISTANCE(vertex *v1,vertex *v2);


//  GEOMETRIC FUNCTIONS


void PolyNormal(vertex *v1,vertex *v2,vertex *v3,vertex *res);
float PolyArea(vertex *v1,vertex *v2,vertex *v3);
float ConeFrustrum(vertex *from,vertex *at,float aCof,vertex *p);
int triBoxOverlap(float boxcenter[3],float boxhalfsize[3],
		  float triverts[3][3]);
int TriBoxInt(vertex *v1,vertex *v2,vertex *v3,vertex *min,vertex *max);
int raySphereInt(ray *line,sphere * sp,float *res);
int rayBoxInt(AABB *box, ray *ray,vertex *intNear,vertex *intFar);
int rayBoxInt(AABB *box, ray *ray,float &tN,float &tF);



//-- Itersection, and other --/
float pointOnLine( vertex *point, ray * line);



//template< class T >
//inline void swap( T& a, T& b )
//{
// T t=a;
//  a=b;
//  b=t;
//}

/* OTHER MACROS */

inline void swap(int &a,int &b){int t=a;a=b;b=t;}
inline void swap(float &a,float &b){float t=a;a=b;b=t;}
inline void swap(char &a,char &b){char t=a;a=b;b=t;}
inline void swap(long &a,long &b){long t=a;a=b;b=t;}
inline void swap(double &a,double &b){double t=a;a=b;b=t;}


inline float setABrangeToCD(float val,float a,float b,float c, float d)
{
  return c+((val-a)*(b-a))/(d-c);
}

void line3d(int x1,int y1,int z1,int x2,int y2,int z2);

int quadratic(float a, float b, float c, float &t1,float &t2);






__forceinline void FloatToInt(int *int_pointer, float f);

#endif
