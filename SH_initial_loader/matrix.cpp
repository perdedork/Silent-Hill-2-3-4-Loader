#include <windows.h>
#include <math.h>
//#include <iostream>
#include "mathOpts.h"
#include "vertex.h"
#include "quat.h"
#include "matrix.h"
#include "typedefs.h"




matrix3x3::matrix3x3()
{
	int i;
	for(i = 0; i < 9; i++)
		mat[i] = 0;

	mat[0]= mat[4]= mat[8]= 1;
}


matrix3x3 & matrix3x3::operator=(const matrix &v)
{
	mat[0] = v.mat[0];  mat[1] = v.mat[1];  mat[2] = v.mat[2];
	mat[3] = v.mat[4];  mat[4] = v.mat[5];  mat[5] = v.mat[6];
	mat[6] = v.mat[8];  mat[7] = v.mat[9];  mat[8]= v.mat[10];

	return *this;
}


matrix3x3 & matrix3x3::operator=(const matrix3x3 &v)
{
	if(&v!=this)
	{
		int x;
		for(x=0;x<9;x++)
			mat[x]=v.mat[x];
	}
	return *this;
}


float * matrix3x3::getRow(int row)
{
	int place=(row & 0x0003) * 3;
	if(place==9)
		place=0;
	return &(mat[place]);
}


void matrix3x3::transpose()
{
	float temp;
	temp = mat[2];
	mat[2] = mat[6];
	mat[6] = temp;
}


void matrix3x3::swapRows( int row1, int row2 )
{
	float tempRow[3];

	if( row1 < 0 || row1 > 2)
		return;
	if( row2 < 0 || row2 > 2)
		return;
	if( row1 == row2 )
		return;

	tempRow[0] = mat[ row1 * 3 + 0 ];
	tempRow[1] = mat[ row1 * 3 + 1 ];
	tempRow[2] = mat[ row1 * 3 + 2 ];

	mat[ row1 * 3 + 0 ] = mat[ row2 * 3 + 0 ];
	mat[ row1 * 3 + 1 ] = mat[ row2 * 3 + 1 ];
	mat[ row1 * 3 + 2 ] = mat[ row2 * 3 + 2 ];

	mat[ row2 * 3 + 0 ] = tempRow[0];
	mat[ row2 * 3 + 1 ] = tempRow[1];
	mat[ row2 * 3 + 2 ] = tempRow[2];
}

vert_type &matrix3x3::operator[](int index)
{
	int place=index & 0x000f;
	if(place > 8)
		place = 8;
	if(place < 0)
		place = 0;
	return mat[place];
}


vert_type matrix3x3::operator[](int index) const
{
	int place=index & 0x000f;
	if(place > 8)
		place = 8;
	if(place < 0)
		place = 0;
	return mat[place];
}


matrix3x3 & matrix3x3::operator *=(matrix3x3 &m)             
{
	matrix3x3 res;
  
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			res.mat[i*3+j]=0;

			for(int k=0;k<3;k++)
			{
				res.mat[i*3+j]+= mat[i*3+k] * m.mat[k*3+j];
			}
		}
	}

	(*this)=res;
	return (*this);
}


matrix3x3 & matrix3x3::operator *=(const float &f)
{
	int c;
	for(c=0;c<9;c++)
		mat[c]=mat[c]*f;
	return *this;
}


matrix3x3 operator+(const matrix3x3 & a,const matrix3x3 &b)
{
	matrix3x3 r;
	int i;

	for(i=0; i < 9; i++)
		r[i] = a[i] + b[i];
	return r;
}


matrix3x3 operator-(const matrix3x3 & a,const matrix3x3 &b)
{
	matrix3x3 r;
	int i;

	for(i=0; i < 9; i++)
		r[i] = a[i] - b[i];
	return r;
}


vertex operator*(const matrix3x3 & m,const vertex &v)/**/
{
	vertex res;
  
	res.x=v.x * m.mat[0] + v.y * m.mat[1] + v.z * m.mat[2];
	res.y=v.x * m.mat[3] + v.y * m.mat[4] + v.z * m.mat[5];
	res.z=v.x * m.mat[6] + v.y * m.mat[7] + v.z * m.mat[8];

	return res;
}


matrix3x3 operator*(const matrix3x3 & a,const matrix3x3 &b)
{
	matrix3x3 tempResMat;

	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			tempResMat.mat[i*3+j]=0;

			for(int k=0;k<3;k++)
			{
				tempResMat.mat[i*3+j]+=a.mat[i*3+k] * b.mat[k*3+j];
			}
		}
	}
	return tempResMat;
}


matrix3x3 operator*(const float & a,const matrix3x3 &b)
{
	matrix3x3 r;
	int c;

	for(c=0;c<9;c++)
		r.mat[c] = b.mat[c] * a;

	return r;
}


matrix3x3 operator*(const matrix3x3 & a,const float &b)
{
	matrix3x3 r;
	int c;

	for(c=0;c<9;c++)
		r.mat[c] = a.mat[c] * b;
	return r;
}


matrix3x3 operator/(const matrix3x3 & a,const float &b)
{
	matrix3x3 r;
	float t=1/b;
	int c;

	for(c=0;c<9;c++)
		r.mat[c] = a.mat[c] * b;

	return r;
}


//-------------------------------------------/
//           OVERLOADED  OPERATORS           /
//-------------------------------------------/

matrix::matrix()
{
	int i;
	for(i = 0; i < 16; i++)
		mat[i] = 0;
	mat[0]= mat[5]= mat[10]= mat[15]= 1;
}


matrix & matrix::operator=(const matrix &v)
{
	if(&v!=this)
    {
		int x;
		for(x=0;x<16;x++)
			mat[x]=v.mat[x];
	}
	return *this;
}


matrix & matrix::operator=(const matrix3x3 &v)
{
	mat[0] = v.mat[0];  mat[1] = v.mat[1];  mat[2] = v.mat[2];
	mat[4] = v.mat[3];  mat[5] = v.mat[4];  mat[6] = v.mat[5];
	mat[8] = v.mat[6];  mat[9] = v.mat[7];  mat[10]= v.mat[8];

	return *this;
}


void matrix::transpose()
{
	float temp;
	temp	=	mat[1];
	mat[1]	=	mat[4];
	mat[4]	=	temp;
	temp	=	mat[2];
	mat[2]	=	mat[8];
	mat[8]	=	temp;
	temp	=	mat[3];
	mat[3]	=	mat[12];
	mat[12]	=	temp;
	temp	=	mat[6];
	mat[6]	=	mat[9];
	mat[9]	=	temp;
	temp	=	mat[7];
	mat[7]	=	mat[13];
	mat[13]	=	temp;
	temp	=	mat[11];
	mat[11]	=	mat[14];
	mat[14]	=	temp;
}

void matrix::swapRow( int row1, int row2 )
{
	float tempRow[4];

	if( row1 < 0 || row1 > 3)
		return;
	if( row2 < 0 || row2 > 3)
		return;
	if( row1 == row2 )
		return;

	tempRow[0] = mat[ row1 * 4 + 0 ];
	tempRow[1] = mat[ row1 * 4 + 1 ];
	tempRow[2] = mat[ row1 * 4 + 2 ];
	tempRow[3] = mat[ row1 * 4 + 3 ];

	mat[ row1 * 4 + 0 ] = mat[ row2 * 4 + 0 ];
	mat[ row1 * 4 + 1 ] = mat[ row2 * 4 + 1 ];
	mat[ row1 * 4 + 2 ] = mat[ row2 * 4 + 2 ];
	mat[ row1 * 4 + 3 ] = mat[ row2 * 4 + 3 ];

	mat[ row2 * 4 + 0 ] = tempRow[0];
	mat[ row2 * 4 + 1 ] = tempRow[1];
	mat[ row2 * 4 + 2 ] = tempRow[2];
	mat[ row2 * 4 + 3 ] = tempRow[3];
}


float * matrix::getRow(int row)
{
	return &(mat[(row & 0x0003)<<2]);
}


void matrix::Inverse()
{
	float src[16], tmp[12], dst[16], det;
	long k;
	
	for( k = 0; k < 4; k++)
	{
		src[k]   = mat [k * 4];
		src[k+4] = mat [k * 4 + 1];
		src[k+8] = mat [k * 4 + 2];
		src[k+12]= mat [k * 4 + 3];
	}

	tmp[0]  = src[10] * src[15];
	tmp[1]  = src[11] * src[14];
	tmp[2]  = src[9]  * src[15];
	tmp[3]  = src[11] * src[13];
	tmp[4]  = src[9]  * src[14];
	tmp[5]  = src[10] * src[13];
	tmp[6]  = src[8]  * src[15];
	tmp[7]  = src[11] * src[12];
	tmp[8]  = src[8]  * src[14];
	tmp[9]  = src[10] * src[12];
	tmp[10] = src[8]  * src[13];
	tmp[11] = src[9]  * src[12];

	dst[0]  = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4]  * src[7];
	dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5]  * src[7];
	dst[1]  = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9]  * src[7];
	dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8]  * src[7];
	dst[2]  = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
	dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
	dst[3]  = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
	dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
	dst[4]  = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5]  * src[3];
	dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4]  * src[3];
	dst[5]  = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8]  * src[3];
	dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9]  * src[3];
	dst[6]  = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
	dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
	dst[7]  = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
	dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

	tmp[0]  = src[2] * src[7];
	tmp[1]  = src[3] * src[6];
	tmp[2]  = src[1] * src[7];
	tmp[3]  = src[3] * src[5];
	tmp[4]  = src[1] * src[6];
	tmp[5]  = src[2] * src[5];
	tmp[6]  = src[0] * src[7];
	tmp[7]  = src[3] * src[4];
	tmp[8]  = src[0] * src[6];
	tmp[9]  = src[2] * src[4];
	tmp[10] = src[0] * src[5];
	tmp[11] = src[1] * src[4];

	dst[8]   = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
	dst[8]  -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
	dst[9]   = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
	dst[9]  -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
	dst[10]  = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10]* src[15];
	dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11]* src[15];
	dst[11]  = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11]* src[14];
	dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10]* src[14];
	dst[12]  = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
	dst[12] -= tmp[4] * src[11] + tmp[0] * src[9]  + tmp[3] * src[10];
	dst[13]  = tmp[8] * src[11] + tmp[0] * src[8]  + tmp[7] * src[10];
	dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
	dst[14]  = tmp[6] * src[9]  + tmp[11]* src[11] + tmp[3] * src[8];
	dst[14] -= tmp[10]* src[11] + tmp[2] * src[8]  + tmp[7] * src[9];
	dst[15]  = tmp[10]* src[10] + tmp[4] * src[8]  + tmp[9] * src[9];
	dst[15] -= tmp[8] * src[9]  + tmp[11]* src[10] + tmp[5] * src[8];

	det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];

	det = 1.0f / det;

	for(k=0; k < 16; k++)
		mat[k] = dst[k] * det;
}


void matrix::quatToMat( const quat  & q )
{
	float xx = q.x * q.x;
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float xw = q.x * q.w;
	float yy = q.y * q.y;
	float yz = q.y * q.z;
	float yw = q.y * q.w;
	float zz = q.z * q.z;
	float zw = q.z * q.w;

	mat[ 0 ]  = 1.0f - 2.0f * (yy + zz);
	mat[ 1 ]  =        2.0f * (xy - zw);
	mat[ 2 ]  =        2.0f * (xz + yw);
	mat[ 4 ]  =        2.0f * (xy + zw);
	mat[ 5 ]  = 1.0f - 2.0f * (xx + zz);
	mat[ 6 ]  =        2.0f * (yz - xw);
	mat[ 8 ]  =        2.0f * (xz - yw);
	mat[ 9 ]  =        2.0f * (yz + xw);
	mat[ 10 ]  = 1.0f - 2.0f * (xx + yy);

	mat[ 3 ] = mat[ 7 ] = mat[ 11 ] = mat[ 12 ] = mat[ 13 ] = mat[14] = 0.0f;
	mat[ 15 ] = 1.0f;
}


void matrix::quatToMat( quat  & q )
{
	float xx = q.x * q.x;
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float xw = q.x * q.w;
	float yy = q.y * q.y;
	float yz = q.y * q.z;
	float yw = q.y * q.w;
	float zz = q.z * q.z;
	float zw = q.z * q.w;

	mat[ 0 ]  = 1.0f - 2.0f * (yy + zz);
	mat[ 1 ]  =        2.0f * (xy - zw);
	mat[ 2 ]  =        2.0f * (xz + yw);
	mat[ 4 ]  =        2.0f * (xy + zw);
	mat[ 5 ]  = 1.0f - 2.0f * (xx + zz);
	mat[ 6 ]  =        2.0f * (yz - xw);
	mat[ 8 ]  =        2.0f * (xz - yw);
	mat[ 9 ]  =        2.0f * (yz + xw);
	mat[ 10 ]  = 1.0f - 2.0f * (xx + yy);

	mat[ 3 ] = mat[ 7 ] = mat[ 11 ] = mat[ 12 ] = mat[ 13 ] = mat[14] = 0.0f;
	mat[ 15 ] = 1.0f;
}


vert_type &matrix::operator[](int index)
{
	return mat[(index & 0x000f)];
}


vert_type matrix::operator[](int index) const
{
	return mat[(index & 0x000f)];
}


matrix & matrix::operator *=(matrix &m)
{
	matrix res;
	MMultM( this, &m, &res);
	(*this) = res;
	return (*this);
}


matrix & matrix::operator *=(const float &f)
{
	int c;
	for( c = 0; c < 16; c++)
		mat[c] = mat[c] * f;
	return *this;
}


matrix & matrix::operator *(const matrix &m)
{
	matrix tempMat;
	for(int i=0;i<4;i++)
    {
		for(int j=0;j<4;j++)
		{
			tempMat.mat[i*4+j] = 0;

			for(int k=0;k<4;k++)
			{
				tempMat.mat[ i * 4 + j ] += mat[ i * 4 + k ] * m.mat[ k * 4 + j ];
			}
		}
	}

	*this = tempMat;

	return *this;
}

matrix operator+(matrix & a,matrix &b)
{
	matrix r;
	MADDM(a,b,r);
	return r;
}


matrix operator-(matrix & a,matrix &b)
{
	matrix r;
	MSUBM(a,b,r);
	return r;
}


vertex operator*(matrix & b,vertex &a)
{

	vertex resVec;
	MMULTVT(b,a,resVec);
	return resVec;
}


vertex4f operator*(matrix & b, vertex4f & a)
{
	vertex4f res;

	res.x = b.mat[ 0] * a.x + b.mat[ 1] * a.y + b.mat[ 2] * a.z + b.mat[ 3] * a.w;
	res.y = b.mat[ 4] * a.x + b.mat[ 5] * a.y + b.mat[ 6] * a.z + b.mat[ 7] * a.w;
	res.z = b.mat[ 8] * a.x + b.mat[ 9] * a.y + b.mat[10] * a.z + b.mat[11] * a.w;
	res.w = b.mat[12] * a.x + b.mat[13] * a.y + b.mat[14] * a.z + b.mat[15] * a.w;

	return res;
}

matrix operator*(matrix & a,matrix &b)
{
	matrix tempResMat;
 
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			tempResMat.mat[ i * 4 + j ] = 0;

			for(int k=0;k<4;k++)
			{
				tempResMat.mat[ i * 4 + j ] += a.mat[ i * 4 + k ] * b.mat[ k * 4 + j ];
			}
		}
	}
	return tempResMat;
}


matrix operator*(float & a,matrix &b)
{
	matrix r;
	MMULTC(b,a,r);
	return r;
}


matrix operator*(matrix & a,float &b)
{
	matrix r;
	MMULTC(a,b,r);
	return r;
}


matrix operator/(matrix & a,float &b)
{
	matrix r;
	float t=1/b;
	MMULTC(a,t,r);
	return r;
}


/*ostream & operator <<(ostream & os,matrix &m)
{
	for(int j=0;j<4;j++)
	{
		os<<"[";
		for(int k=0;k<4;k++)
		{
			os<<m.mat[j*4+k]<<" , ";
		}
		os<<"]"<<endl;
	}
	return os;
}
*/

void MMultM(matrix* m1,matrix* m2,matrix *res)
{
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			res->mat[i*4+j]=0.0;

			for(int k=0;k<4;k++)
			{
				res->mat[ i * 4 + j ] += m1->mat[ i * 4 + k ] * m2->mat[ k * 4 + j ];
			}
		}
	}
}


void MTRANSPOSE(matrix *m)
{
	float temp;
	temp=m->mat[1];
	m->mat[1]=m->mat[4];
	m->mat[4]=temp;
	temp=m->mat[2];
	m->mat[2]=m->mat[8];
	m->mat[8]=temp;
	temp=m->mat[3];
	m->mat[3]=m->mat[12];
	m->mat[12]=temp;
	temp=m->mat[6];
	m->mat[6]=m->mat[9];
	m->mat[9]=temp;
	temp=m->mat[7];
	m->mat[7]=m->mat[13];
	m->mat[13]=temp;
	temp=m->mat[11];
	m->mat[11]=m->mat[14];
	m->mat[14]=temp;
}



void MROTATE(float ang,float x,float y,float z,matrix *res)
{
	float angle=ang * 0.01745329252f;
	vertex axis(x,y,z);
	VUnit(&axis);
	vertex4f q;
	float halfTheta = angle * 0.5f;
	float cosHalfTheta = cosf(halfTheta);
	float sinHalfTheta = sinf(halfTheta);
	float xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
	q.x = axis.x*sinHalfTheta;
	q.y = axis.y*sinHalfTheta;
	q.z = axis.z*sinHalfTheta;
	q.w = cosHalfTheta;
	xs = 2.0f * q.x;  ys = 2.0f * q.y;  zs = 2.0f * q.z;
	wx =  q.w * xs; wy =  q.w * ys; wz =  q.w * zs;
	xx =  q.x * xs; xy =  q.x * ys; xz =  q.x * zs;
	yy =  q.y * ys; yz =  q.y * zs; zz =  q.z * zs;
	res->mat[0]=1.0f -(yy + zz); res->mat[1]= xy - wz; res->mat[2]= xz + wy;
	res->mat[4]=xy + wz; res->mat[5]= 1.0f -  (xx + zz); res->mat[6]= yz - wx;
	res->mat[8]= xz - wy; res->mat[9]= yz + wx; res->mat[10]= 1.0f - (xx + yy);
	res->mat[3]=res->mat[7]=res->mat[11]=res->mat[12]=res->mat[13]=
	res->mat[14]=0;
	res->mat[15]= 1.0f;
}


void MROTATE(float ang,float x,float y,float z,matrix *res,quat *qres)
{
	float angle=ang * 0.01745329252f;
	vertex axis(x,y,z);
	VUnit(&axis);
	quat q;
	float halfTheta = angle * 0.5f;
	float cosHalfTheta = cosf(halfTheta);
	float sinHalfTheta = sinf(halfTheta);
	float xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
	qres->x=q.x = axis.x * sinHalfTheta;
	qres->y=q.y = axis.y * sinHalfTheta;
	qres->z=q.z = axis.z * sinHalfTheta;
	qres->w=q.w = cosHalfTheta;
	xs = 2.0f * q.x;  ys = 2.0f * q.y;  zs = 2.0f * q.z;
	wx =  q.w * xs; wy =  q.w * ys; wz =  q.w * zs;
	xx =  q.x * xs; xy =  q.x * ys; xz =  q.x * zs;
	yy =  q.y * ys; yz =  q.y * zs; zz =  q.z * zs;
	res->mat[0]=1.0f - (yy + zz); res->mat[1]= xy - wz; res->mat[2]= xz + wy;
	res->mat[4]=xy + wz; res->mat[5]= 1.0f - (xx + zz); res->mat[6]= yz - wx;
	res->mat[8]= xz - wy; res->mat[9]= yz + wx; res->mat[10]= 1.0f - (xx + yy);
	res->mat[3]=res->mat[7]=res->mat[11]=res->mat[12]=res->mat[13]=
	res->mat[14]=0.0f;
	res->mat[15]=1.0f;
}


void quatToMat( quat *q, matrix *m )
{
	float xx = q->x * q->x;
	float xy = q->x * q->y;
	float xz = q->x * q->z;
	float xw = q->x * q->w;
	float yy = q->y * q->y;
	float yz = q->y * q->z;
	float yw = q->y * q->w;
	float zz = q->z * q->z;
	float zw = q->z * q->w;

	m->mat[ 0 ]  = 1.0f - 2.0f * (yy + zz);
	m->mat[ 1 ]  =        2.0f * (xy - zw);
	m->mat[ 2 ]  =        2.0f * (xz + yw);
	m->mat[ 4 ]  =        2.0f * (xy + zw);
	m->mat[ 5 ]  = 1.0f - 2.0f * (xx + zz);
	m->mat[ 6 ]  =        2.0f * (yz - xw);
	m->mat[ 8 ]  =        2.0f * (xz - yw);
	m->mat[ 9 ]  =        2.0f * (yz + xw);
	m->mat[ 10 ]  = 1.0f - 2.0f * (xx + yy);

	m->mat[ 3 ] = m->mat[ 7 ] = m->mat[ 11 ] = m->mat[ 12 ] = m->mat[ 13 ] = m->mat[14] = 0.0f;
	m->mat[ 15 ] = 1.0f;
}


void QTOM(quat *q,matrix *m)
{
	float xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

	xs = 2.0f * q->x;  ys = 2.0f * q->y;  zs = 2.0f * q->z;
	wx =  q->w * xs; wy =  q->w * ys; wz =  q->w * zs;
	xx =  q->x * xs; xy =  q->x * ys; xz =  q->x * zs;
	yy =  q->y * ys; yz =  q->y * zs; zz =  q->z * zs;
	m->mat[0]=1.0f -(yy + zz); m->mat[1]= xy - wz; m->mat[2]= xz + wy;
	m->mat[4]=xy + wz; m->mat[5]= 1.0f - (xx + zz); m->mat[6]= yz - wx;
	m->mat[8]= xz - wy; m->mat[9]= yz + wx; m->mat[10]= 1.0f - (xx + yy);
	m->mat[3]=m->mat[7]=m->mat[11]=m->mat[12]=m->mat[13]=m->mat[14]=0.0f;
	m->mat[15]=1.0f;
}

void MTOQ(quat *q,matrix *m)
{
	float trace,sVal;

	trace=1.0f + m->mat[0]+m->mat[5]+m->mat[10];
  
	if(trace > 0.00000001f)
	{
		sVal=0.50f * rsqrtf(trace);
		q->x=(m->mat[9]-m->mat[6] ) * sVal;
		q->y=(m->mat[2]-m->mat[8] ) * sVal;
		q->z=(m->mat[4]-m->mat[1] ) * sVal;
		q->w= 0.25f / sVal;
	}
	else if(m->mat[0] > m->mat[5] && m->mat[0] > m->mat[10] )
	{
		sVal= 0.50f * rsqrtf(1.0f + m->mat[0] - m->mat[5] - m->mat[10]);
		q->x= 0.25f / sVal;
		q->y=(m->mat[4] + m->mat[1] ) * sVal;
		q->z=(m->mat[2] + m->mat[8] ) * sVal;
		q->w=(m->mat[9] - m->mat[6] ) * sVal;   
	}
	else if(m->mat[5] > m->mat[10] )
	{
		sVal=0.50f * rsqrtf(1.0f + m->mat[5] - m->mat[0] - m->mat[10]);
		q->x=(m->mat[4] + m->mat[1] ) * sVal;
		q->y= 0.25f / sVal;
		q->z=(m->mat[9] + m->mat[6]) * sVal;
		q->w=(m->mat[2] - m->mat[8]) * sVal;
	}
	else
	{
		sVal=0.50f * rsqrtf(1.0f + m->mat[10] - m->mat[0] - m->mat[5]);
		q->x= (m->mat[2] + m->mat[8] ) * sVal;
		q->y=(m->mat[9] + m->mat[6] ) * sVal;
		q->z= 0.25f / sVal;
		q->w=(m->mat[4] - m->mat[1] ) * sVal;
	}
}
