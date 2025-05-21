#include <math.h>
//#include <iostream>
#include "typedefs.h"
#include "vertex.h"
#include "quat.h"
#include "mathOpts.h"

 quat quat::UNIT(0,0,0,1);

void quat::computeR()
{
	float t= 1.0f-(x*x)-(y*y)-(z*z);
	w=(t < 0.0f) ? 0.0f : -sqrtf(t);
	QUnit( this );
}

const quat & quat::operator=(const quat &v)
{
  if(&v!=this)
    {x=v.x;y=v.y;z=v.z;w=v.w;}
  return *this;
}
      
inline vert_type &quat::operator[](int index)        // first
{
  return (*this)[(index & 0x0003)];
}

inline vert_type quat::operator[](int index)const    // second
{
  return (*this)[(index & 0x0003)];
}

vertex quat::rotateVertex(const vertex &v)const
{
	static bool printOnce = true;
	//vertex4f invq,qTemp,qTemp2;
	//setVertex4f(qTemp2,x,y,z,w);
	//QUnit((quat *)&qTemp2);
	//QCONJ(qTemp2,invq);
	quat qTemp( *this ), invq, res;
	quat tVert = quat( v.x, v.y, v.z, 0.0f );
	QUnit( &qTemp );
	QCONJ( qTemp, invq );
	//QINVERSE( &qTemp, &invq );
	if(printOnce)
	LogFile(TEST_LOG,"This: %f  %f  %f  %f\nOrig: %f  %f  %f  %f\nConj: %f  %f  %f  %f\ntVert: %f  %f  %f  %f\n",
		x,y,z,w,qTemp.x,qTemp.y,qTemp.z,qTemp.w,invq.x,invq.y,invq.z,invq.w,tVert.x,tVert.y,tVert.z,tVert.w);
	res = (qTemp * tVert) * invq;
printOnce=false;
	return vertex(res.x, res.y, res.z );
//result=q * (vertex * q^-1)  --first parentheses  --Since v.w is 0, remove those terms...
//	qTemp.w =/* v.w1*inv.w*/ - v.x*invq.x - v.y*invq.y - v.z*invq.z;

//	qTemp.x =/* v.w1*inv.x +*/ v.x*invq.w + v.y*invq.z - v.z*invq.y;
 
//	qTemp.y =/* v.w1*inv.y +*/ v.y*invq.w + v.z*invq.x - v.x*invq.z;
 
//	qTemp.z =/* v.w1*inv.z +*/ v.z*invq.w + v.x*invq.y - v.y*invq.x;

//result=( q * (vertex * q^-1))  --2nd parentheses for final result
 //	invq.w = qTemp2.w*qTemp.w - qTemp2.x*qTemp.x - qTemp2.y*qTemp.y - qTemp2.z*qTemp.z;

//	invq.x = qTemp2.w*qTemp.x + qTemp2.x*qTemp.w + qTemp2.y*qTemp.z - qTemp2.z*qTemp.y;
 
//	invq.y = qTemp2.w*qTemp.y + qTemp2.y*qTemp.w + qTemp2.z*qTemp.x - qTemp2.x*qTemp.z;
 
//	invq.z = qTemp2.w*qTemp.z + qTemp2.z*qTemp.w + qTemp2.x*qTemp.y - qTemp2.y*qTemp.x;
	
//	return vertex(invq.x,invq.y,invq.z);
}


void quat::getAxisAngles( float *xRes, float *yRes, float *zRes )
{
	float sinY = -2.0f * ( x * z - w * y );

	float tanX = 2.0f * ( y * z + w * x );
	float tanX2= w * w - x * x - y * y + z * z;
	
	float tanZ = 2.0f * ( y * x + w * z );
	float tanZ2= w * w + x * x - y * y - z * z;

	if( sinY > 1.0f )
		sinY = 1.0f;
	if( sinY < -1.0f )
		sinY = -1.0f;

	*xRes = atan2( tanX, tanX2 );
	*yRes = asinf( sinY );
	*zRes = atan2( tanZ, tanZ2 );
}


void	quat::matToQuat(const matrix &mat) 
{
	float trace,sVal;

	trace=1.0f + mat[0]+mat[5]+mat[10];
  
	if(trace > 0.00000001f)
	{
		sVal=0.50f * rsqrtf(trace);
		x=(mat[9]-mat[6] ) * sVal;
		y=(mat[2]-mat[8] ) * sVal;
		z=(mat[4]-mat[1] ) * sVal;
		w= 0.25f / sVal;
	}
	else if(mat[0] > mat[5] && mat[0] > mat[10] )
	{
		sVal= 0.50f * rsqrtf(1.0f + mat[0] - mat[5] - mat[10]);
		x= 0.25f / sVal;
		y=(mat[4] + mat[1] ) * sVal;
		z=(mat[2] + mat[8] ) * sVal;
		w=(mat[9] - mat[6] ) * sVal;   
	}
	else if(mat[5] > mat[10] )
	{
		sVal=0.50f * rsqrtf(1.0f + mat[5] - mat[0] - mat[10]);
		x=(mat[4] + mat[1] ) * sVal;
		y= 0.25f / sVal;
		z=(mat[9] + mat[6]) * sVal;
		w=(mat[2] - mat[8]) * sVal;
	}
	else
	{
		sVal=0.50f * rsqrtf(1.0f + mat[10] - mat[0] - mat[5]);
		x= (mat[2] + mat[8] ) * sVal;
		y=(mat[9] + mat[6] ) * sVal;
		z= 0.25f / sVal;
		w=(mat[4] - mat[1] ) * sVal;
	}
}


void	quat::matToQuat( matrix &mat) 
{
	float trace,sVal;

	trace=mat[0]+mat[5]+mat[10];
  
	if(trace > 0.00000001f)
	{
		sVal=0.50f * rsqrtf(1.0f + trace);
		x=(mat[9]-mat[6] ) * sVal;
		y=(mat[2]-mat[8] ) * sVal;
		z=(mat[4]-mat[1] ) * sVal;
		w= 0.25f / sVal;
	}
	else if(mat[0] > mat[5] && mat[0] > mat[10] )
	{
		sVal= 0.50f * rsqrtf(1.0f + mat[0] - mat[5] - mat[10]);
		x= 0.25f / sVal;
		y=(mat[4] + mat[1] ) * sVal;
		z=(mat[2] + mat[8] ) * sVal;
		w=(mat[9] - mat[6] ) * sVal;   
	}
	else if(mat[5] > mat[10] )
	{
		sVal=0.50f * rsqrtf(1.0f + mat[5] - mat[0] - mat[10]);
		x=(mat[4] + mat[1] ) * sVal;
		y= 0.25f / sVal;
		z=(mat[9] + mat[6]) * sVal;
		w=(mat[2] - mat[8]) * sVal;
	}
	else
	{
		sVal=0.50f * rsqrtf(1.0f + mat[10] - mat[0] - mat[5]);
		x= (mat[2] + mat[8] ) * sVal;
		y=(mat[9] + mat[6] ) * sVal;
		z= 0.25f / sVal;
		w=(mat[4] - mat[1] ) * sVal;
	}
}



void QUnit(quat * _v)
{
 vert_type mag= rsqrtf(_v->x*_v->x+_v->y*_v->y+_v->z*_v->z+_v->w*_v->w);
 _v->x*=mag;_v->y*=mag;_v->z*=mag;_v->w*=mag;
}


void QROTATE(vert_type ang,vert_type x,vert_type y,vert_type z,quat *res)
{
  vert_type angle= ang * 0.01745329252f;
  vertex axis(x,y,z);
  VUnit(&axis);

  vert_type halfTheta = angle * 0.5f;
  vert_type cosHalfTheta = cosf(halfTheta);
  vert_type sinHalfTheta = sinf(halfTheta);
  res->x = axis.x*sinHalfTheta;
  res->y = axis.y*sinHalfTheta;
  res->z = axis.z*sinHalfTheta;
  res->w = cosHalfTheta;
}



void QINVERSE(quat *q,quat *resl)
{
  quat tQuat;
  vert_type qnorm=QNORM((*q));
  resl->x=-q->x/qnorm;
  resl->y=-q->y/qnorm;
  resl->z=-q->z/qnorm;
  resl->w=q->w/qnorm;
}

void QROTAXIS(quat *q,vertex *res)
{
  vert_type halfTheta=q->w;
  vert_type sinHalfTheta=sqrtf( 1.0f - halfTheta*halfTheta);
  if( sinHalfTheta < 0.0005f )
    sinHalfTheta = 1.0f;
  res->x=q->x/sinHalfTheta;
  res->y=q->y/sinHalfTheta;
  res->z=q->z/sinHalfTheta;
}

void QMULT(quat *q1,quat *q2,quat *res)
{
 /* vertex t1,t2,t3,qc1,qc2;
  vert_type w1=q1->w,w2=q2->w;
  copyVertex(qc1,(*q1));
  copyVertex(qc2,(*q2));

  VCROSS(qc1,qc2,t1);
  VMULTF(qc2,w1,t2);
  VADDV(t1,t2,t3);
  VMULTF(qc1,w2,t2);
  VADDV(t3,t2,t1);
  res->x=t1.x;
  res->y=t1.y;
  res->z=t1.z;
  res->w=w1*w2-VDOT(qc1,qc2);*/
float x1 = q1->x, x2 = q2->x, y1 = q1->y, y2 = q2->y,
	  z1 = q1->z, z2 = q2->z, w1 = q1->w, w2 = q2->w;
 
 res->w = w1*w2 - x1*x2 - y1*y2 - z1*z2;

 res->x = w1*x2 + x1*w2 + y1*z2 - z1*y2;
 
 res->y = w1*y2 + y1*w2 + z1*x2 - x1*z2;
 
 res->z = w1*z2 + z1*w2 + x1*y2 - y1*x2;
 
}

void QSQUARE(quat *q,quat *res)
{
  vert_type wMag=2.0f * q->w;
  res->w=q->w*q->w-(q->x*q->x+q->y*q->y+q->z*q->z);
  res->x=q->x*wMag;
  res->y=q->y*wMag;
  res->z=q->z*wMag;
}



void QSLERP(quat *q1,quat *q2,vert_type t,quat *res)
{
  vert_type theta=acosf(QDOT((*q1),(*q2)));
  quat mR1,mR2;
  if(theta==0.0f)
    {
      res->x = res->y = res->z = 0.0f;  res->w=1.0f;
      return;
    }
  vert_type sinTheta = sinf(theta), sinThetaMoreT = sinf(theta*t) / sinTheta,
    sinThetaLessT=((sinf(theta*(1-t)))/sinTheta);
  QMULTF((*q2),sinThetaMoreT,mR1);
  QMULTF((*q1),sinThetaLessT,mR2);
  QADD(mR1,mR2,(*res)); 
}


//Operators
inline quat operator+(const quat & a,const quat &b)
{
  quat res;
  res.x=a.x+b.x;res.y=a.y+b.y;res.z=a.z+b.z;res.w=a.w+b.w;
  return res;
}

inline quat operator-(const quat & a,const quat &b)
{
  quat res;
  res.x=a.x-b.x;res.y=a.y-b.y;res.z=a.z-b.z;res.w=a.w-b.w;
  return res;
}

inline quat operator*(const quat & b,const vert_type &a)
{
  quat res;
  res.x=b.x*a;res.y=b.y*a;res.z=b.z*a;res.w=b.w*a;
  return res;
}

inline quat operator*(const vert_type & a,const quat &b)
{
  quat res;
  res.x=b.x*a;res.y=b.y*a;res.z=b.z*a;res.w=b.w*a;
  return res;
}


quat operator*(const quat & a,const quat &b)
{
  quat r;
  r.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
  r.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
  r.y = a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z;
  r.z = a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x;
  //QUnit( &r );
  return r;
}


inline quat operator/(const quat & a,const vert_type &b)
{
  quat res;
  vert_type inv = 1.0f / b;
  res.x=a.x*inv;res.y=a.y*inv;res.z=a.z*inv;res.w=a.w*inv;
  return res;
}

/*ostream & operator<<(ostream & os,quat &q)
{
  os<<" ------------Quat------------"<<endl<<"x="<<q.x<<"  y="<<q.y;
  os<<"  z="<<q.z<<" w="<<q.w<<endl;
  return os;
}
*/
