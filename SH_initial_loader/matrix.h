#ifndef MATRIX_H
#define MATRIX_H

#include <windows.h>
#ifndef __INTRIN_H_
//#include <intrin.h>
#endif /*__INTRIN_H_*/

//Forward Declaration
class matrix;
class quat;

#include "vertex.H"
#include "quat.H"



class matrix3x3{
public:
	float mat[9];

	matrix3x3();
	matrix3x3(const matrix &r){operator=(r);}
	matrix3x3(const matrix3x3 &r){operator=(r);}

	matrix3x3 & operator=(const matrix3x3 &v);
	matrix3x3 & operator=(const matrix &v);

	float * getRow(int row);
	void transpose();
	void swapRows( int row1, int row2 );

	void identity( ){	mat[0] = mat[4] = mat[8] = 1.0f;
						mat[1] = mat[2] = mat[3] = 
						mat[5] = mat[6] = mat[7] = 0.0f;} 


	vert_type &operator[](int index);                // Mutator
	vert_type  operator[](int index) const;          // Accessor
	matrix3x3 & operator *=(matrix3x3 &m);
	matrix3x3 & operator *=(const float &f);

};


matrix3x3 operator+(const matrix3x3 & a,const matrix3x3 &b);
matrix3x3 operator-(const matrix3x3 & a,const matrix3x3 &b);
vertex    operator*(const matrix3x3 & m,const vertex & v);
matrix3x3 operator*(const matrix3x3 & a,const matrix3x3 &b);
matrix3x3 operator*(const float & a,const matrix3x3 &b);
matrix3x3 operator*(const matrix3x3 & a,const float &b);

matrix3x3 operator/(const matrix3x3 & a,const float &b);



class matrix{
public:
	float mat[16];

	matrix();
	matrix(const float *r){int j;for(j=0;j<16;j++)mat[j]=r[j];}
	matrix(const matrix3x3 &r){operator=(r);}

	void Inverse(void);

	matrix & operator=(const matrix &v);
	matrix & operator=(const matrix3x3 &v);

	void copyRot( matrix & rhs ){	mat[0] = rhs.mat[0]; mat[1] = rhs.mat[1]; mat[2] = rhs.mat[2]; 
									mat[4] = rhs.mat[4]; mat[5] = rhs.mat[5]; mat[6] = rhs.mat[6]; 
									mat[8] = rhs.mat[8]; mat[9] = rhs.mat[9]; mat[10] = rhs.mat[10]; }

	void clearRot( ){	mat[0] = mat[5] = mat[10]= 1.0f;
						mat[1] = mat[2] = mat[4] = 
						mat[6] = mat[8] = mat[9] = 0.0f;} 

	void copyTrans( matrix & rhs ){	mat[3] = rhs.mat[3]; mat[7] = rhs.mat[7]; mat[10] = rhs.mat[10]; }

	void clearTrans( ){	mat[3] = mat[7] = mat[11]= 0.0f; }

	void identity( ){	mat[0] = mat[5] = mat[10]= mat[15]= 1.0f;
						mat[1] = mat[2] = mat[3] = mat[4] = mat[6] =
						mat[7] = mat[8] = mat[9] = mat[11]= mat[12]=
						mat[13]= mat[14]= 0.0f;} 

	void transpose();
	void swapRow( int row1, int row2 );
	float * getRow(int row);
	void quatToMat( const quat  & q );
	void quatToMat( quat  & q );

	vert_type &operator[](int index);                // Mutator
	vert_type  operator[](int index) const;          // Accessor
	matrix & operator *=(matrix &m);
	matrix & operator *=(const float &f);
	matrix & operator *(const matrix &m);
 
};


matrix operator+(matrix & a,matrix &b);
matrix operator-(matrix & a,matrix &b);
vertex operator*(matrix & b,vertex &a);
vertex4f operator*(matrix & b, vertex4f & a);
matrix operator*(matrix & a,matrix &b);
matrix operator*(float & a,matrix &b);
matrix operator*(matrix & a,float &b);

matrix operator/(matrix & a,float &b);
//ostream & operator <<(ostream & os,matrix &m);




/*multiplies matrix by another, in order res=m1*m2 */
void MMultM(matrix* m1,matrix* m2,matrix *res);

/*Returns the transpose for 4x4 matrix "m" */
void MTRANSPOSE(matrix *m);

/*Returns the matrix "res" that rotates "ang" degrees around axis defined by */
/* the variables "x","y", and "z" */
void MROTATE(float ang,float x,float y,float z,matrix *res);

/*Same as above, but also returns the quat representing the same Rotation*/
void MROTATE(float ang,float x,float y,float z,matrix *res,quat *qres);

void QTOM(quat *q,matrix *m);
void MTOQ(quat *q,matrix *m);

void quatToMat( quat *q, matrix *m );

#define MSET(m,f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15)(m.mat[0]=f0,m.mat[1]=f1,m.mat[2]=f2,m.mat[3]=f3,\
					 m.mat[4]=f4,m.mat[5]=f5,m.mat[6]=f6,m.mat[7]=f7,m.mat[8]=f8,\
                     m.mat[9]=f9,m.mat[10]=f10,m.mat[11]=f11,m.mat[12]=f12,m.mat[13]=f13,m.mat[14]=f14,m.mat[15]=f15) 
#define MIDENTITY(m)(m.mat[0]=m.mat[5]=m.mat[10]=m.mat[15]=1,m.mat[1]=\
                     m.mat[2]=m.mat[3]=m.mat[4]=m.mat[6]=m.mat[7]=m.mat[8]=\
                     m.mat[9]=m.mat[11]=m.mat[12]=m.mat[13]=m.mat[14]=0) 
#define MTRANSLATE(m,x,y,z)(m.mat[0]=m.mat[5]=m.mat[10]=m.mat[15]=1,\
                            m.mat[1]=m.mat[2]=m.mat[4]=m.mat[6]=m.mat[8]=\
                            m.mat[9]=m.mat[12]=m.mat[13]=m.mat[14]=0,\
                            m.mat[3]=x,m.mat[7]=y,m.mat[11]=z)
#define MSCALE(m,x,y,z)(m.mat[0]=x,m.mat[5]=y,m.mat[10]=z,m.mat[15]=1,\
                        m.mat[1]=m.mat[2]=m.mat[3]=m.mat[4]=m.mat[6]=\
                        m.mat[7]=m.mat[8]=m.mat[9]=m.mat[11]=m.mat[12]=\
                        m.mat[13]=m.mat[14]=0) 
#define MROTATEX(m,x)(m.mat[0]=m.mat[15]=1,m.mat[5]=m.mat[10]=cos(x),\
                      m.mat[9]=sin(x),m.mat[6]=-m.mat[9],m.mat[1]=m.mat[2]= \
                      m.mat[3]=m.mat[4]=m.mat[7]=m.mat[8]=m.mat[11]= \
                      m.mat[12]=m.mat[13]=m.mat[14]=0) 
#define MROTATEY(m,y)(m.mat[5]=m.mat[15]=1,m.mat[0]=m.mat[10]=cos(y), \
                      m.mat[2]=sin(y),m.mat[8]=-m.mat[2], \
                      m.mat[1]=m.mat[3]=m.mat[4]=m.mat[6]=m.mat[7]=m.mat[9]= \
                      m.mat[11]=m.mat[12]=m.mat[13]=m.mat[14]=0) 
#define MROTATEZ(m,z)(m.mat[10]=m.mat[15]=1,m.mat[0]=m.mat[5]=cos(z), \
                      m.mat[4]=sin(z),m.mat[1]=-m.mat[4],m.mat[2]=m.mat[3]= \
                      m.mat[6]=m.mat[7]=m.mat[8]=m.mat[9]=m.mat[11]= \
                      m.mat[12]=m.mat[13]=m.mat[14]=0) 
#define MEMPTY(m)(m.mat[0]=m.mat[5]=m.mat[10]=m.mat[15]=m.mat[1]=m.mat[2]= \
                  m.mat[3]=m.mat[4]=m.mat[6]=m.mat[7]=m.mat[8]=m.mat[9]= \
                  m.mat[11]=m.mat[12]=m.mat[13]=m.mat[14]=0) 

#define MMULTV(m,v,res) (res.x=v.x*m.mat[0]+v.y*m.mat[1]+v.z*m.mat[2],\
                         res.y=v.x*m.mat[4]+v.y*m.mat[5]+v.z*m.mat[6],\
                         res.z=v.x*m.mat[8]+v.y*m.mat[9]+v.z*m.mat[10]) 
#define MMULTVT(m,v,res) (res.x=v.x*m.mat[0]+v.y*m.mat[1]+v.z*m.mat[2]\
			  +m.mat[3],\
                         res.y=v.x*m.mat[4]+v.y*m.mat[5]+v.z*m.mat[6]\
			  +m.mat[7],\
                         res.z=v.x*m.mat[8]+v.y*m.mat[9]+v.z*m.mat[10]\
                          +m.mat[11]) 
#define MMULTC(m,c,res) {static int mc;\
                         for(mc=0;mc<16;mc++)\
			    {res.mat[mc]=m.mat[mc]*c;}}
#define MSUBM(a,b,res) {static int mc;\
                         for(mc=0;mc<16;mc++)\
			    {res.mat[mc]=a.mat[mc]-b.mat[mc];}}
#define MADDM(a,b,res) {static int mc;\
                         for(mc=0;mc<16;mc++)\
			    {res.mat[mc]=a.mat[mc]+b.mat[mc];}}

#endif
