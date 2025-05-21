#ifndef PLANE_H
#define PLANE_H

//#include "OpenGL
#include <windows.h>
#include "vertex.h"
#include "typedefs.h"

enum {FRONT,BACK,ON};

#define PL_XSIGN      0x1
#define PL_YSIGN      0x2
#define PL_ZSIGN      0x4

#define FLOAT_SIGN    0x80000000


class plane{
public:
  plane(){}
  plane(const plane & P){operator=(P);}
  plane(float X,float Y,float Z,float W):x(X),y(Y),z(Z),w(W){SetPlaneSign();}


  void MakePlane(vertex &t0, vertex & t1, vertex & t2);
  ~plane(){}

  float x,y,z,w;
  char signBits;    //For some quick tests

  const plane & operator=(const plane &p);
  vert_type &operator[](int index);                // Mutator
  const vert_type &operator[](int index) const;          // Accessor
  void norm();
  void SetPlaneSign();
};


inline char SetSignBits(vertex &v)
{
  char sRes=0;
  sRes|=(v.x<0.0)?PL_XSIGN:0;
  sRes|=(v.y<0.0)?PL_YSIGN:0;
  sRes|=(v.z<0.0)?PL_ZSIGN:0;
  return sRes;
}

#define PLANE(v1,norm,res) (res.x=norm.x,res.y=norm.y,res.z=norm.z,\
			    res.w=-VDOT(norm,v1), \
                            res.signBits=SetSignBits((norm)))


void PlaneEq(vertex *pnts,plane *result);
void PointOnPlane(plane &p,vertex &res);
float pointPlaneDist(plane &p,vertex &v);
int pointPlaneSide(vertex &v,plane &p);
int pointPlaneEpsSide(vertex &v,plane &p,float epsilon);
int planeBoxOverlap(plane &p,AABB &b);
int planeBoxIntersect(plane *p,AABB *b);



#endif
