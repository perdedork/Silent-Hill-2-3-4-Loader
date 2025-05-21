#include <windows.h>
#include "vertex.h"
#include "typedefs.h"
#include "plane.h"
#include "mathOpts.h"



inline  const plane & plane::operator=(const plane &p)
{
  if(&p!=this)
    {x=p.x;y=p.y;z=p.z;w=p.w;}
  return *this;
}
     
/*
vert_type &plane::operator[](int index)        // first
{
  return (*this)[(index & 0x0003)];
}

const vert_type &plane::operator[](int index)const    // second
{
  return (*this)[(index & 0x0003)];
}
*/

inline void plane::norm()
{
  float invLen=rsqrtf(x*x+y*y+z*z);
  x*=invLen;
  y*=invLen;
  z*=invLen;
  w*=invLen;
}


inline void plane::SetPlaneSign()
{
  signBits=0;
  signBits|=(x<0.0)?PL_XSIGN:0;
  signBits|=(y<0.0)?PL_YSIGN:0;
  signBits|=(z<0.0)?PL_ZSIGN:0;
}


void plane::MakePlane(vertex &t0, vertex & t1, vertex & t2)
{
	vertex _t1 = t1, _t2 = t2, _n;
	_t1 -= t0;
	_t2 -= t0;
	VCROSS(_t1,_t2,_n);
	 VUnit(&_n);          //Since the plane's norm is already of unit len, no need to normalize the "d" value
	PLANE(t0,_n,(*this));
}


vert_type &plane::operator[](int index)        // first
{
  //assert(index<=2 && index>=0);
  //return (*this)[index];
//	return ((float *)this)[index%NUM_VERTEX_ELEMENTS];
	
	index=index%NUM_VERTEX4F_ELEMENTS;
	switch(index){
	case 0:return x;
	case 1:return y;
	case 2:return z;
		}
	return w;
}

const vert_type &plane::operator[](int index)const    // second
{
 // assert(index<=2 && index>=0);
  //return ((float *)this)[index%NUM_VERTEX_ELEMENTS];
	index=index%NUM_VERTEX4F_ELEMENTS;
	switch(index){
	case 0:return x;
	case 1:return y;
	case 2:return z;
		}
	return w;
}


void PlaneEq(vertex *pnts,plane *result)
{
  vertex t1 = pnts[1], t2 = pnts[2], n;
  t1 -= pnts[0];
  t2 -= pnts[0];
  VCROSS(t1,t2,n);
  VUnit(&n);          //Since the plane's norm is already of unit len, no need to normalize the "d" value
  PLANE(pnts[0],n,(*result));
}


void PointOnPlane(plane &p,vertex &res)
{
  copyVertex(res,p);
  res*=p.w;
}


inline int pointPlaneSide(vertex &v,plane &p)
{
  float t=pointPlaneDist(p,v);
  if(t>0.0)return FRONT;
  return BACK;
}


inline int pointPlaneEpsSide(vertex &v,plane &p,float epsilon)
{
  float t=pointPlaneDist(p,v);
  if(t>epsilon)return FRONT;
  else if(t<-epsilon)return BACK;
  return ON;
}


inline float pointPlaneDist(plane &p,vertex &v)
{
  return VDOT(p,v)+p.w;
}


int planeBoxOverlap(plane &p,AABB &b)
{
  enum {LOW=0,HIGH=1};
  float bX[] ={ b.min.x * p.x, b.max.x * p.x },
        bY[] ={ b.min.y * p.y, b.max.y * p.y },
        bZ[] ={ b.min.z * p.z, b.max.z * p.z };

  int c,side=((bX[LOW]+bY[LOW]+bZ[LOW]+p.w)>0.0)?FRONT:BACK;
  int xP=LOW,yP=LOW,zP=LOW;
  for(c=1;c<8;c++)
    {
      if(xP)            //NOTE:Since we go in at iteration #1 and
	{               //    not at #0, we update the values 
	  if(zP&&xP)    //    before the work of the loop is done.
	    {           //    This way, it should have the correct
	      yP=!yP;   //    results... If everything worked right
	    }
	  zP=!zP;
	}
      xP=!xP; 

      int cSide=((bX[xP]+bY[yP]+bZ[zP]+p.w)>0.0)?FRONT:BACK;
      if(cSide!=side)return TRUE;
    }
  return FALSE;
}


int planeBoxIntersect(plane *p,AABB *b)
{
  float t1,t2;
  switch (p->signBits)
    {
    case 0:t1 = p->x*b->max.x + p->y*b->max.y + p->z*b->max.z+p->w;
           t2 = p->x*b->min.x + p->y*b->min.y + p->z*b->min.z+p->w;
           break;
    case 1:t1 = p->x*b->min.x + p->y*b->max.y + p->z*b->max.z+p->w;
           t2 = p->x*b->max.x + p->y*b->min.y + p->z*b->min.z+p->w;
	   break;
    case 2:t1 = p->x*b->max.x + p->y*b->min.y + p->z*b->max.z+p->w;
           t2 = p->x*b->min.x + p->y*b->max.y + p->z*b->min.z+p->w;
	   break;
    case 3:t1 = p->x*b->min.x + p->y*b->min.y + p->z*b->max.z+p->w;
           t2 = p->x*b->max.x + p->y*b->max.y + p->z*b->min.z+p->w;
	   break;
    case 4:t1 = p->x*b->max.x + p->y*b->max.y + p->z*b->min.z+p->w;
           t2 = p->x*b->min.x + p->y*b->min.y + p->z*b->max.z+p->w;
	   break;
    case 5:t1 = p->x*b->min.x + p->y*b->max.y + p->z*b->min.z+p->w;
           t2 = p->x*b->max.x + p->y*b->min.y + p->z*b->max.z+p->w;
	   break;
    case 6:t1 = p->x*b->max.x + p->y*b->min.y + p->z*b->min.z+p->w;
           t2 = p->x*b->min.x + p->y*b->max.y + p->z*b->max.z+p->w;
	   break;
    case 7:t1 = p->x*b->min.x + p->y*b->min.y + p->z*b->min.z+p->w;
           t2 = p->x*b->max.x + p->y*b->max.y + p->z*b->max.z+p->w;
	   break;
    }
  if( t1 > 0.0 && t2 > 0.0) return FRONT;
  if( t1 < 0.0 && t2 < 0.0) return BACK;
  return ON;
}
