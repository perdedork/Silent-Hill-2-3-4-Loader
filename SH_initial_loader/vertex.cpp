#include <windows.h>
#include <stdio.h>
//#include <iostream>
#include <math.h>
//#include <assert.h>

#include "mathOpts.h"

#include "vertex.h"
//#include "typedefs.H"

extern fsqrt FSQRT;


const vertex vertex::ZERO(0,0,0);
const vertex vertex::AXIS_X(1,0,0);
const vertex vertex::AXIS_Y(0,1,0);
const vertex vertex::AXIS_Z(0,0,1);

vertex & vertex::operator=(const vertex &v)
{
  if(&v!=this)
    {x=v.x;y=v.y;z=v.z;}
  return *this;
}


vertex & vertex::operator=(const vertex4f &v)
{
	x=v.x;
	y=v.y;
	z=v.z;

	return *this;
}


vertex & vertex::norm( )
{
	float invLen = 1.0f/sqrtf( x * x + y * y + z * z );
	x *= invLen;
	y *= invLen;
	z *= invLen;

	return *this;
}


vert_type &vertex::operator[](int index)        // first
{
  //assert(index<=2 && index>=0);
  //return (*this)[index];
//	return ((float *)this)[index%NUM_VERTEX_ELEMENTS];
	
	index=index%NUM_VERTEX_ELEMENTS;
	switch(index){
	case 0:return x;
	case 1:return y;
		}
	return z;
}

const vert_type &vertex::operator[](int index)const    // second
{
 // assert(index<=2 && index>=0);
  //return ((float *)this)[index%NUM_VERTEX_ELEMENTS];
	index=index%NUM_VERTEX_ELEMENTS;
	switch(index){
	case 0:return x;
	case 1:return y;
		}
	return z;
}


int operator==(const vertex &v,const vertex &v2)
{
  if(v2.x!=v.x)return FALSE;
  if(v2.y!=v.y)return FALSE;
  if(v2.z!=v.z)return FALSE;
  return TRUE;
}


int operator!=(const vertex &v,const vertex &v2)
{
  return ((v2.x!=v.x)||(v2.y!=v.y)||(v2.z!=v.z));
}


void vertex::operator+= (const vertex &v)
{
  x+=v.x;
  y+=v.y;
  z+=v.z;
 // return *this;
}


void vertex::operator-= (const vertex &v)
{
  x-=v.x;
  y-=v.y;
  z-=v.z;
 // return *this;
}


void vertex::operator*= (const vertex &v)
{
  x*=v.x;
  y*=v.y;
  z*=v.z;
 // return *this;
}


void vertex::operator*= (const vert_type f)
{
  x*=f;
  y*=f;
  z*=f;
 // return *this;
}


void vertex::operator&= (const vertex &v)
{
	x = y * v.z - z * v.y;
	y = z * v.x - x * v.z;
	z = x * v.y - y * v.x;
//	return *this;
}


void vertex::operator/= (const vertex &v)
{
  x/=v.x;
  y/=v.y;
  z/=v.z;
 // return *this;
}


void vertex::operator/= (const vert_type f)
{
  float _f,p=f;
  FP_INV(_f,p);
  x*=_f;
  y*=_f;
  z*=_f;
  //return *this;
}


vertex & vertex::operator-(void)
{
  x=-x;y=-y;z=-z;
  return *this;
}


void VUnit(vertex * _v)
{
 vert_type mag= (1.0f/sqrtf(_v->x*_v->x+_v->y*_v->y+_v->z*_v->z));
//	vert_type mag= rsqrtf(_v->x*_v->x+_v->y*_v->y+_v->z*_v->z);
 _v->x*=mag;_v->y*=mag;_v->z*=mag;
}


ostream & operator<<(ostream & O,vertex &v)
{
	O<<	v.x	<<	" "	<<	v.y	<<	" "	<<	v.z;
	return O;
}

/*
vertex operator+(vertex const &a,vertex const &b)
{
	vertex tmp;
	copyVertex(tmp,a);
	tmp+=b;
	return tmp;
}
vertex operator-(vertex const &a,vertex const &b)
{
	vertex tmp;
	tmp.x=a.x-b.x;
	tmp.y=a.y-b.y;
	tmp.z=a.z-b.z;

	return tmp;
}
*/

double VertLen(vertex *p1,vertex *p2)
{
  double ddist=VertLenSqr(p1,p2);
 //return( FSQRT.fastsqrt(ddist));
  return sqrt(ddist);
}

double VertLenSqr(vertex *v1,vertex *v2)
{
  double tX,tY,tZ;
  tX=v1->x-v2->x;
  tY=v1->y-v2->y;
  tZ=v1->z-v2->z;
  return tX*tX+tY*tY+tZ*tZ;
}


void VertMax(vertex *a,vertex *b,vertex *res)
{
  res->x=(a->x>b->x)?a->x:b->x;
  res->y=(a->y>b->y)?a->y:b->y;
  res->z=(a->z>b->z)?a->z:b->z;
}


void VertMin(vertex *a,vertex *b,vertex *res)
{
  res->x=(a->x<b->x)?a->x:b->x;
  res->y=(a->y<b->y)?a->y:b->y;
  res->z=(a->z<b->z)?a->z:b->z;
}

void makePerpAxis(vertex &inZ, vertex *xAxis, vertex *yAxis)
{
	vertex temp(0,0,0);

	int smallestComp = VMINORCOMPONENT(inZ);

	if(smallestComp == 2)
	{
		smallestComp = VMAJORCOMPONENT(inZ);
		smallestComp = (smallestComp == 1)?0:1;
	}

	if(smallestComp)
	{
		temp.y = 1.0f;
		VCROSS(temp,inZ,(*xAxis));
		VCROSS(inZ,(*xAxis),(*yAxis));
	}
	else
	{
		temp.x = 1.0f;
		VCROSS(inZ,temp,(*yAxis));
		VCROSS((*yAxis),inZ,(*xAxis));
	}

	VUnit(&inZ);
	VUnit(xAxis);
	VUnit(yAxis);
}

void makePerpAxis(vertex &in,vertex *xAxis, vertex *yAxis, vertex *zAxis)
{

}

//Operators
vertex operator+(const vertex & a,const vertex &b)
{
  vertex res;
  res.x=a.x+b.x;res.y=a.y+b.y;res.z=a.z+b.z;
  return res;
}

vertex operator-(const vertex & a,const vertex &b)
{
  vertex res;
  res.x=a.x-b.x;res.y=a.y-b.y;res.z=a.z-b.z;
  return res;
}

vertex operator*(const vertex & b,const vert_type &a)
{
  vertex res;
  res.x=b.x*a;res.y=b.y*a;res.z=b.z*a;
  return res;
}

vertex operator*(const vert_type & a,const vertex &b)
{
  vertex res;
  res.x=b.x*a;res.y=b.y*a;res.z=b.z*a;
  return res;
}

vertex operator/(const vertex & a,const vert_type &b)
{
  vertex res;
  vert_type inv=1/b;
  res.x=a.x*inv;res.y=a.y*inv;res.z=a.z*inv;
  return res;
}




//---------------------------------------------------------------------/
//--           Vertex4f                                              --/
//--  This is the section for the vertex4f shit                      --/
//---------------------------------------------------------------------/


const vertex4f vertex4f::ZERO_V4F(0,0,0,0);
const vertex4f vertex4f::AXIS_X_V4F(1,0,0,0);
const vertex4f vertex4f::AXIS_Y_V4F(0,1,0,0);
const vertex4f vertex4f::AXIS_Z_V4F(0,0,1,0);

vertex4f & vertex4f::operator=(const vertex4f &v)
{
  if(&v!=this)
    {x=v.x;y=v.y;z=v.z;w=v.w;}
  return *this;
}
      
vertex4f & vertex4f::norm( )
{
	float invLen = 1.0f/sqrtf( x * x + y * y + z * z + w * w );
	x *= invLen;
	y *= invLen;
	z *= invLen;
	w *= invLen;
	return *this;
}


vert_type &vertex4f::operator[](int index)        // first
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

const vert_type &vertex4f::operator[](int index)const    // second
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


int operator==(const vertex4f &v,const vertex4f &v2)
{
  if(v2.x!=v.x)return FALSE;
  if(v2.y!=v.y)return FALSE;
  if(v2.z!=v.z)return FALSE;
  if(v2.w!=v.w)return FALSE;
  return TRUE;
}


int operator!=(const vertex4f &v,const vertex4f &v2)
{
  return ((v2.x!=v.x)||(v2.y!=v.y)||(v2.z!=v.z)||(v2.w!=v.w));
}


void vertex4f::operator+= (const vertex4f &v)
{
  x+=v.x;
  y+=v.y;
  z+=v.z;
  w+=v.w;
 // return *this;
}


void vertex4f::operator-= (const vertex4f &v)
{
  x-=v.x;
  y-=v.y;
  z-=v.z;
  w-=v.w;
 // return *this;
}


void vertex4f::operator*= (const vertex4f &v)
{
  x*=v.x;
  y*=v.y;
  z*=v.z;
  w*=v.w;
 // return *this;
}


void vertex4f::operator*= (const vert_type f)
{
  x*=f;
  y*=f;
  z*=f;
  w*=f;
 // return *this;
}


void vertex4f::operator&= (const vertex4f &v)
{
	x = y * v.z - z * v.y;                 //NOTE: I thinks that the correct cross prod for 4 elements is like the quat multiply
	y = z * v.x - x * v.z;
	z = x * v.y - y * v.x;
//	return *this;
}


void vertex4f::operator/= (const vertex4f &v)
{
  x/=v.x;
  y/=v.y;
  z/=v.z;
  w/=v.w;
 // return *this;
}


void vertex4f::operator/= (const vert_type f)
{
  float _f,p=f;
  FP_INV(_f,p);
  x*=_f;
  y*=_f;
  z*=_f;
  w*=_f;
  //return *this;
}


vertex4f & vertex4f::operator-(void)
{
  x=-x;y=-y;z=-z;w=-w;
  return *this;
}


void VUnit4f(vertex4f * _v)
{
 //vert_type mag= (1.0/sqrt(_v->x*_v->x+_v->y*_v->y+_v->z*_v->z));
	vert_type mag= rsqrtf(_v->x*_v->x+_v->y*_v->y+_v->z*_v->z + _v->w * _v->w);
 _v->x*=mag;_v->y*=mag;_v->z*=mag;_v->w*=mag;
}


//ostream & operator<<(ostream & O,vertex4f &v)
//{
// O<<"---------------------------"<<endl<<"x="<<v.x<<"  y="<<v.y;
//  O<<"  z="<<v.z<<"  v.w="<<v.w<<endl;
//  return O;
//}

/*
vertex operator+(vertex const &a,vertex const &b)
{
	vertex tmp;
	copyVertex(tmp,a);
	tmp+=b;
	return tmp;
}
vertex operator-(vertex const &a,vertex const &b)
{
	vertex tmp;
	tmp.x=a.x-b.x;
	tmp.y=a.y-b.y;
	tmp.z=a.z-b.z;

	return tmp;
}
*/

double VertLen4f(vertex4f *p1,vertex4f *p2)
{
  double ddist=VertLenSqr4f(p1,p2);
 //return( FSQRT.fastsqrt(ddist));
  return sqrt(ddist);
}

double VertLenSqr4f(vertex4f *v1,vertex4f *v2)
{
  double tX,tY,tZ,tW;
  tX=v1->x-v2->x;
  tY=v1->y-v2->y;
  tZ=v1->z-v2->z;
  tW=v1->w-v2->w;
  return tX*tX+tY*tY+tZ*tZ+tW*tW;
}


void VertMax4f(vertex4f *a,vertex4f *b,vertex4f *res)
{
  res->x=(a->x>b->x)?a->x:b->x;
  res->y=(a->y>b->y)?a->y:b->y;
  res->z=(a->z>b->z)?a->z:b->z;
  res->w=(a->w>b->w)?a->w:b->w;
}


void VertMin4f(vertex4f *a,vertex4f *b,vertex4f *res)
{
  res->x=(a->x<b->x)?a->x:b->x;
  res->y=(a->y<b->y)?a->y:b->y;
  res->z=(a->z<b->z)?a->z:b->z;
  res->w=(a->w<b->w)?a->w:b->w;
}


//Operators
vertex4f operator+(const vertex4f & a,const vertex4f &b)
{
  vertex4f res;
  res.x=a.x+b.x;res.y=a.y+b.y;res.z=a.z+b.z;res.w=a.w+b.w;
  return res;
}

vertex4f operator-(const vertex4f & a,const vertex4f &b)
{
  vertex4f res;
  res.x=a.x-b.x;res.y=a.y-b.y;res.z=a.z-b.z;res.w=a.w-b.w;
  return res;
}

vertex4f operator*(const vertex4f & b,const vert_type &a)
{
  vertex4f res;
  res.x=b.x*a;res.y=b.y*a;res.z=b.z*a;res.w=b.w*a;
  return res;
}

vertex4f operator*(const vert_type & a,const vertex4f &b)
{
  vertex4f res;
  res.x=b.x*a;res.y=b.y*a;res.z=b.z*a;res.w=b.w*a;
  return res;
}

vertex4f operator/(const vertex4f & a,const vert_type &b)
{
  vertex4f res;
  vert_type inv=1/b;
  res.x=a.x*inv;res.y=a.y*inv;res.z=a.z*inv;res.w=a.w*inv;
  return res;
}
