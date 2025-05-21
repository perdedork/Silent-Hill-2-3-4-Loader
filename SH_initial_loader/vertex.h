#ifndef VERTEX_H
#define VERTEX_H

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include <windows.h>
//#ifndef __INTRIN_H_
//#include <intrin.h>
//#endif /*__INTRIN_H_*/
#include <iostream>

typedef float vert_type;
#define NUM_VERTEX_ELEMENTS     3
#define NUM_VERTEX4F_ELEMENTS   4

using std::ostream;

class vertex4f;

class vertex{
public:
  vertex(){}
  vertex(const vertex &v){operator=(v);}
  vertex( float *fVals){ if( fVals ){ x = fVals[0]; y = fVals[1]; z = fVals[2]; }else{ x=y=z=0.0f; }}
  ~vertex(){}
  vertex(vert_type X,vert_type Y,vert_type Z):x(X),y(Y),z(Z){}

  vert_type x,y,z;
 
  vertex & operator=(const vertex &v);
  vertex & operator=(const vertex4f &v);
  vert_type &operator[](int index);                // Mutator
  const vert_type & operator[](int index) const;          // Accessor
  int getNumElements(){return 3;}
  vertex & norm( );
  vertex & setMins( vertex & v ){ x = (x < v.x )?x:v.x; y = (y < v.y )?y:v.y; z = (z < v.z )?z:v.z; return *this;}
  vertex & setMaxs( vertex & v ){ x = (x > v.x )?x:v.x; y = (y > v.y )?y:v.y; z = (z > v.z )?z:v.z; return *this;}

  //--------------------------/
  //   Overloaded Operators   /
  //--------------------------/

 
  friend vertex operator*(const vertex & b,const vert_type &a);
  friend vertex operator*(const vert_type & a,const vertex &b);
  friend vertex operator/(const vertex & a,const vert_type &b);
  friend ostream& operator<<(ostream& O,vertex &v);
  friend vertex   operator+ (vertex const &a,vertex const &b);
  friend vertex   operator- (vertex const &a,vertex const &b);

  void operator+= (const vertex &v);
  void  operator-= (const vertex &v);
  void operator*= (const vertex &v);
  void operator*= (const vert_type f);
  void operator&= (const vertex &v);
  void operator/= (const vertex &v);
  void operator/= (const vert_type f);
  vertex & operator-(void);

  static const vertex ZERO;
  static const vertex AXIS_X;
  static const vertex AXIS_Y;
  static const vertex AXIS_Z;

};

void makePerpAxis(vertex &inZ, vertex *xAxis, vertex *yAxis);
void makePerpAxis(vertex &in,vertex *xAxis, vertex *yAxis, vertex *zAxis);

int operator==(const vertex &v,const vertex &v2);               // Comparison
int operator!=(const vertex &v,const vertex &v2);

//------------------------------/
//      Vertex Operations       /
//------------------------------/

void VUnit(vertex * _v);
double VertLen(vertex *p1,vertex *p2);
double VertLenSqr(vertex *v1,vertex *v2);
void VertMax(vertex *a,vertex *b,vertex *res);
void VertMin(vertex *a,vertex *b,vertex *res);

#define copyVertex(a,b)        (a.x=b.x,a.y=b.y,a.z=b.z)
#define copyVertex4f(a,b)      (a.x=b.x,a.y=b.y,a.z=b.z,a.w=b.w)
#define copy3to4(a,b)          (copyVertex(a,b),a.w=1)
#define copy4to3div(a,b)       (a.x=b.x/b.w,a.y=b.y/b.w,a.z=b.z/b.w)
#define setVertex(v,a,b,c)     (v.x=a, v.y=b, v.z=c)
#define setVertex4f(v,a,b,c,d) (v.x=a, v.y=b, v.z=c,v.w=d)

#define VMINORCOMPONENT(v1) ((abs(v1.x)<=abs(v1.y))?((abs(v1.x)<=abs(v1.z))?0:2):(abs(v1.y)<=abs(v1.z))?1:2)
#define VMAJORCOMPONENT(v1) ((abs(v1.x)>=abs(v1.y))?((abs(v1.x)>=abs(v1.z))?0:2):(abs(v1.y)>=abs(v1.z))?1:2)

#define VSUBV(v1,v2,res) (res.x=v1.x-v2.x,res.y=v1.y-v2.y,res.z=v1.z-v2.z) 
#define VADDV(v1,v2,res) (res.x=v2.x+v1.x,res.y=v2.y+v1.y,res.z=v2.z+v1.z) 

#define VDOT(v1,v2) (v1.x*v2.x+v1.y*v2.y+v1.z*v2.z) 
#define VCROSS(v1,v2,res) (res.x=v1.y*v2.z-v1.z*v2.y,\
                           res.y=v1.z*v2.x-v1.x*v2.z,\
                           res.z=v1.x*v2.y-v1.y*v2.x) 
#define VMAG(v) sqrtf(VDOT(v,v)) 
#define VDIVF(v,f,res)(res.x=v.x/f,res.y=v.y/f,res.z=v.z/f) 
#define VMULTF(v,f,res) (res.x=v.x*f,res.y=v.y*f,res.z=v.z*f) 
#define VMIDPOINT(v1,v2,res) (res.x=(v1.x+v2.x)/2.0f,\
                              res.y=(v1.y+v2.y)/2.0f,\
                              res.z=(v1.z+v2.z)/2.0f)
#define VHALFSIZE(v1,v2,res) (res.x=(v1.x-v2.x)/2.0f,\
                              res.y=(v1.y-v2.y)/2.0f,\
                              res.z=(v1.z-v2.z)/2.0f)


//--------------------------/
//   Overloaded Operators   /
//--------------------------/
/*
vertex operator+(const vertex & a,const vertex &b);
vertex operator-(const vertex & a,const vertex &b);
vertex operator*(const vertex & b,const vert_type &a);
vertex operator*(const vert_type & a,const vertex &b);
vertex operator/(const vertex & a,const vert_type &b);
*/






/*
typedef struct{
  vert_type x,y,z,w;
}vertex4f;
*/

class vertex4f:public vertex{
public:
  vertex4f(){}
  vertex4f(const vertex4f &v){operator=(v);}
  vertex4f( float *fVals){ if( fVals ){ x = fVals[0]; y = fVals[1]; z = fVals[2]; w = fVals[3]; }else{ x=y=z=w=0.0f; }}
  ~vertex4f(){}
  vertex4f(vert_type X,vert_type Y,vert_type Z,vert_type W):w(W){x=X;y=Y;z=Z;}

  vert_type w;
 
  vertex4f & operator=(const vertex4f &v);
  vertex4f & operator=(const vertex &v){if(&v!=this){copyVertex((*this),v);}return *this;}
  vert_type &operator[](int index);                // Mutator
  const vert_type & operator[](int index) const;          // Accessor
  int getNumElements(){return 4;}
  vertex4f & norm( );
  vertex4f & setMins( vertex4f & v ){ x = (x < v.x )?x:v.x; y = (y < v.y )?y:v.y; z = (z < v.z )?z:v.z; w = (w < v.w )?w:v.w; return *this;}
  vertex4f & setMaxs( vertex4f & v ){ x = (x > v.x )?x:v.x; y = (y > v.y )?y:v.y; z = (z > v.z )?z:v.z; w = (w > v.w )?w:v.w; return *this;}

  //--------------------------/
  //   Overloaded Operators   /
  //--------------------------/

 
  friend vertex4f operator*(const vertex4f & b,const vert_type &a);
  friend vertex4f operator*(const vert_type & a,const vertex4f &b);
  friend vertex4f operator/(const vertex4f & a,const vert_type &b);
//  friend ostream& operator<<(ostream& O,vertex4f &v);
  friend vertex4f   operator+ (vertex4f const &a,vertex4f const &b);
  friend vertex4f   operator- (vertex4f const &a,vertex4f const &b);

  void operator+= (const vertex4f &v);
  void  operator-= (const vertex4f &v);
  void operator*= (const vertex4f &v);
  void operator*= (const vert_type f);
  void operator&= (const vertex4f &v);
  void operator/= (const vertex4f &v);
  void operator/= (const vert_type f);
  vertex4f & operator-(void);

  static const vertex4f ZERO_V4F;
  static const vertex4f AXIS_X_V4F;
  static const vertex4f AXIS_Y_V4F;
  static const vertex4f AXIS_Z_V4F;

};


int operator==(const vertex4f &v,const vertex4f &v2);               // Comparison
int operator!=(const vertex4f &v,const vertex4f &v2);

//------------------------------/
//      vertex4f Operations       /
//------------------------------/

void VUnit4f(vertex4f * _v);
double VertLen4f(vertex4f *p1,vertex4f *p2);
__inline double VertLenSqr4f(vertex4f *v1,vertex4f *v2);
void VertMax4f(vertex4f *a,vertex4f *b,vertex4f *res);
void VertMin4f(vertex4f *a,vertex4f *b,vertex4f *res);

#define copy4to3(a,b)          (copyVertex(a,b))

#define VSUBV4F(v1,v2,res) (res.x=v1.x-v2.x,res.y=v1.y-v2.y,res.z=v1.z-v2.z,res.w=v1.w-v2.w) 
#define VADDV4F(v1,v2,res) (res.x=v2.x+v1.x,res.y=v2.y+v1.y,res.z=v2.z+v1.z,res.w=v1.w+v2.w) 

#define VDOT4F(v1,v2) (v1.x*v2.x+v1.y*v2.y+v1.z*v2.z+v1.w*v2.w) 

#define VMAG4F(v) sqrt(VDOT4F(v,v)) 
#define VDIVF4F(v,f,res)(res.x=v.x/f,res.y=v.y/f,res.z=v.z/f,res.w=v.w/f) 
#define VMULTF4F(v,f,res) (res.x=v.x*f,res.y=v.y*f,res.z=v.z*f,res.w=v.w*f) 
#define VMIDPOINT4F(v1,v2,res) (res.x=(v1.x+v2.x)/2.0f,\
                              res.y=(v1.y+v2.y)/2.0f,\
                              res.z=(v1.z+v2.z)/2.0f,\
							  res.w=(v1.w+v2.w)/2.0f)
#define VHALFSIZE4F(v1,v2,res) (res.x=(v1.x-v2.x)/2.0f,\
                              res.y=(v1.y-v2.y)/2.0f,\
                              res.z=(v1.z-v2.z)/2.0f,\
							  res.w=(v1.w-v2.w)/2.0f)


#endif
