#ifndef QUAT_H
#define QUAT_H

#include <windows.h>
#ifndef __INTRIN_H_
//#include <intrin.h>
#endif /*__INTRIN_H_*/
#include <math.h>
//#include <iostream>
#include "vertex.h"
class quat;
class matrix;

#include "matrix.h"

class quat{
public:
  vert_type x,y,z,w;

  quat(){}
  quat(vert_type X,vert_type Y,vert_type Z,vert_type W):x(X),y(Y),z(Z),w(W){}
  quat(const quat & Q){operator=(Q);}
  quat( const matrix & mat ){ matToQuat( mat ); }
  ~quat(){}

  void computeR();
  vertex rotateVertex(const vertex &v)const;
  void getAxisAngles( float *xRes, float *yRes, float *zRes );
  void matToQuat( const matrix & mat );
  void matToQuat( matrix & mat );


  const quat & operator=(const quat &q);
  vert_type &operator[](int index);                // Mutator
  vert_type  operator[](int index) const;          // Accessor

  static quat UNIT;

};


//--------------------------/
//   Overloaded Operators   /
//--------------------------/

quat operator+(const quat & a,const quat &b);
quat operator-(const quat & a,const quat &b);
quat operator*(const quat & b,const vert_type &a);
quat operator*(const vert_type & a,const quat &b);
quat operator*(const quat & a,const quat &b);
quat operator/(const quat & a,const vert_type &b);
//ostream & operator<<(ostream & os,quat &q);


//  QUARTERION MACROS & FUNCTIONS
#define QNORM(q) (q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w)
#define QDOT(q1,q2) (q1.x*q2.x+q1.y*q2.y+q1.z*q2.z+q1.w*q2.w)
#define QROTANGLE(q) (acos(q.w)*114.591559)
#define QCONJ(q,res) (res.x=-q.x,res.y=-q.y,res.z=-q.z,res.w=q.w)
#define QDIVF(q,f,res) (res.x=q.x/f,res.y=q.y/f,res.z=q.z/f,res.w=q.w/f)
#define QMULTF(q,f,res) (res.x=q.x*f,res.y=q.y*f,res.z=q.z*f,res.w=q.w*f) 
#define QLENGTH(q) (sqrt(QNORM(q)))
#define QADD(q1,q2,res) (res.x=q1.x+q2.x,res.y=q1.y+q2.y,res.z=q1.z+q2.z,\
                         res.w=q1.w+q2.w)
#define QSUBQ(q1,q2,res) (res.x=q1.x-q2.x,res.y=q1.y-q2.y,res.z=q1.z-q2.z,\
                         res.w=q1.w-q2.w)

#define copyQuat(a,b)          (a.x=b.x,a.y=b.y,a.z=b.z,a.w=b.w)
#define setQuat(q,a,b,c,d)     (q.x=a,q.y=b,q.z=c,q.w=d)


void QINVERSE(quat *q,quat *resl);
void QROTAXIS(quat *q,vertex *res);
void QMULT(quat *q1,quat *q2,quat *res);
void QSQUARE(quat *q,quat *res);

void QSLERP(quat *q1,quat *q2,vert_type t,quat *res);

void QUnit(quat * _v);
void QROTATE(vert_type ang,vert_type x,vert_type y,vert_type z,quat *res);

#endif
