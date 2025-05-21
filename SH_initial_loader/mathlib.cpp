#include <iostream>
#include <math.h>
#include "typedefs.h"
#include "mathlib.h"



//######################################### FIX STUFF AFTER HERE
// CLEAN THIS WHOLE DAMN FILE!!!!!!!!!!!!!!!


int arrayMax(void *array,int numEl,e_type t)
{
  int k,res=0;
  switch(t)
    {
    case tCHAR:{char *temp=(char *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]>temp[res])
	    res=k;
	}break;}
    case tUCHAR:{unsigned char *temp=(unsigned char *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]>temp[res])
	    res=k;
	}break;}
    case tSHORT:{short *temp=(short *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]>temp[res])
	    res=k;
	}break;}
    case tUSHORT:{unsigned short *temp=(unsigned short *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]>temp[res])
	    res=k;
	}break;}
    case tINT:{int *temp=(int *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]>temp[res])
	    res=k;
	}break;}
    case tUINT:{unsigned int *temp=(unsigned int *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]>temp[res])
	    res=k;
	}break;}
    case tFLOAT:{float *temp=(float *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]>temp[res])
	    res=k;
	}break;}
    case tDOUBLE:{double *temp=(double *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]>temp[res])
	    res=k;
	}break;}
    }
    return res;
}


int arrayMin(void *array,int numEl,e_type t)
{
  int k,res=0;
  switch(t)
    {
    case tCHAR:{char *temp=(char *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]<temp[res])
	    res=k;
	}break;}
    case tUCHAR:{unsigned char *temp=(unsigned char *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]<temp[res])
	    res=k;
	}break;}
    case tSHORT:{short *temp=(short *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]<temp[res])
	    res=k;
	}break;}
    case tUSHORT:{unsigned short *temp=(unsigned short *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]<temp[res])
	    res=k;
	}break;}
    case tINT:{int *temp=(int *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]<temp[res])
	    res=k;
	}break;}
    case tUINT:{unsigned int *temp=(unsigned int *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]<temp[res])
	    res=k;
	}break;}
    case tFLOAT:{float *temp=(float *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]<temp[res])
	    res=k;
	}break;}
    case tDOUBLE:{double *temp=(double *)array;
      for(k=1;k<numEl;k++)
	{
	  if(temp[k]<temp[res])
	    res=k;
	}break;}
    }
    return res;
}







void PolyNormal(vertex *v1,vertex *v2,vertex *v3,vertex *res)
{
  vertex t1,t2;          //This implementation gives a positive normal to verts
  VSUBV((*v2),(*v1),t1);  //arranged clockwise.  If you need it negative,
  VSUBV((*v3),(*v2),t2);  //change the place of the v1, and v3 in the VSubV()
  VCROSS(t1,t2,(*res));
  VUnit(res);
}


float PolyArea(vertex *v1,vertex *v2,vertex *v3)
{
  vertex r1,r2,r3;
  VSUBV((*v1),(*v2),r1);
  VSUBV((*v1),(*v3),r2);
  VCROSS(r1,r2,r3);
  return ABS( VMAG( r3 ) ) / 2.0f;
}


//----------- ConeFrustrum -------------------------------//
//- This function checks to see if a point is in/outside -//
//- of a cone.  NEEDS TO BEMODIFIED (like many functions -//
//- in this file...                              
float ConeFrustrum(vertex *from,vertex *at,float aCof,vertex *p)
{
  vertex n;
  VSUBV((*at),(*from),n);
  float dx=p->x-from->x;
  float dy=p->y-from->y;
  float dz=p->z-from->z;
  float termA=n.x*dx+n.y*dy+n.z*dz;
  termA*=termA;
  float termB=aCof*(dx*dx+dy*dy+dz*dz);
  return termA-termB;
}




int TriBoxInt(vertex *p1,vertex *p2,vertex *p3,vertex *min,vertex *max)
{
  float boxCenter[3]={(max->x+min->x)/2.0f,(max->y+min->y)/2.0f,
			(max->z+min->z)/2.0f};
  float triVerts[3][3]={{p1->x,p1->y,p1->z},{p2->x,p2->y,p2->z},
    {p3->x,p3->y,p3->z}};
  float boxHalfSize[3]={max->x-boxCenter[0],max->y-boxCenter[1],
			  max->z-boxCenter[2]};
  return triBoxOverlap(boxCenter,boxHalfSize,triVerts);
}

#define X 0
#define Y 1
#define Z 2

#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0]; 

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 


int planeBoxOverlap(float normal[3],float d, float maxbox[3])
{
  int q;
  float vmin[3],vmax[3];
  for(q=X;q<=Z;q++)
  {
    if(normal[q]>0.0f)
    {
      vmin[q]=-maxbox[q];
      vmax[q]=maxbox[q];
    }
    else
    {
      vmin[q]=maxbox[q];
      vmax[q]=-maxbox[q];
    }
  }
  if(DOT(normal,vmin)+d>0.0f) return 0;
  if(DOT(normal,vmax)+d>=0.0f) return 1;
  
  return 0;
}


/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)                         \
        p0 = a*v0[Y] - b*v0[Z];                            \
        p2 = a*v2[Y] - b*v2[Z];                            \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
        rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
        if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)                          \
        p0 = a*v0[Y] - b*v0[Z];                            \
        p1 = a*v1[Y] - b*v1[Z];                            \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
        rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
        if(min>rad || max<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)                         \
        p0 = -a*v0[X] + b*v0[Z];                           \
        p2 = -a*v2[X] + b*v2[Z];                           \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
        rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
        if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)                          \
        p0 = -a*v0[X] + b*v0[Z];                           \
        p1 = -a*v1[X] + b*v1[Z];                           \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
        rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
        if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)                         \
        p1 = a*v1[X] - b*v1[Y];                            \
        p2 = a*v2[X] - b*v2[Y];                            \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
        rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
        if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)                          \
        p0 = a*v0[X] - b*v0[Y];                            \
        p1 = a*v1[X] - b*v1[Y];                            \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
        rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
        if(min>rad || max<-rad) return 0;

int triBoxOverlap(float boxcenter[3],float boxhalfsize[3],float triverts[3][3])
{

  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
   float v0[3],v1[3],v2[3];
   float min,max,d,p0,p1,p2,rad,fex,fey,fez;  
   float normal[3],e0[3],e1[3],e2[3];

   /* This is the fastest branch on Sun */
   /* move everything so that the boxcenter is in (0,0,0) */
   SUB(v0,triverts[0],boxcenter);
   SUB(v1,triverts[1],boxcenter);
   SUB(v2,triverts[2],boxcenter);

   /* compute triangle edges */
   SUB(e0,v1,v0);      /* tri edge 0 */
   SUB(e1,v2,v1);      /* tri edge 1 */
   SUB(e2,v0,v2);      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 9 tests first (this was faster) */
   fex = ABS(e0[X]);
   fey = ABS(e0[Y]);
   fez = ABS(e0[Z]);
   AXISTEST_X01(e0[Z], e0[Y], fez, fey);
   AXISTEST_Y02(e0[Z], e0[X], fez, fex);
   AXISTEST_Z12(e0[Y], e0[X], fey, fex);

   fex = ABS(e1[X]);
   fey = ABS(e1[Y]);
   fez = ABS(e1[Z]);
   AXISTEST_X01(e1[Z], e1[Y], fez, fey);
   AXISTEST_Y02(e1[Z], e1[X], fez, fex);
   AXISTEST_Z0(e1[Y], e1[X], fey, fex);

   fex = ABS(e2[X]);
   fey = ABS(e2[Y]);
   fez = ABS(e2[Z]);
   AXISTEST_X2(e2[Z], e2[Y], fez, fey);
   AXISTEST_Y1(e2[Z], e2[X], fez, fex);
   AXISTEST_Z12(e2[Y], e2[X], fey, fex);

   /* Bullet 1: */
   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0[X],v1[X],v2[X],min,max);
   if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;

   /* test in Y-direction */
   FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
   if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;

   /* test in Z-direction */
   FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
   if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;

   /* Bullet 2: */
   /*  test if the box intersects the plane of the triangle */
   /*  compute plane equation of triangle: normal*x+d=0 */
   CROSS(normal,e0,e1);
   d=-DOT(normal,v0);  /* plane eq: normal.x+d=0 */
   if(!planeBoxOverlap(normal,d,boxhalfsize)) return 0;

   return 1;   /* box and triangle overlaps */
}

 

/*raySphereInt
 *  A collision detection function to check to see if a line and a sphere
 *  Intersect.  Returns the minimum of the two result values from eq
 * (-b(+/-)(b^2-4ac)^.5)/2a in the 'res' parameters.  The return value is
 * either 0, or 1, if the ray misses, or intersects the sphere
 */
int raySphereInt(ray *line, sphere * sp,float *res)
{
  vertex l(sp->c.x-line->origin.x,sp->c.y-line->origin.y,
	      sp->c.z-line->origin.z);
  float q,s,l_2,m_2,r_2,s_2;

  r_2=sp->rad*sp->rad;
  s=VDOT(l,(line->dir));
  l_2=VDOT(l,l);

  if(s<0 && l_2>r_2) return 0;
  
  s_2=s*s;
  m_2=l_2-s_2;

  if(m_2>r_2) return 0;
  
  q = sqrtf(r_2-m_2);
  if(l_2>r_2)*res=s-q;
  else *res=s+q;
  return 1;
}
  /*
  float i=line->dir.x-line->origin.x,j=line->dir.y-line->origin.y,
    k=line->dir.z-line->origin.z;
  float a=i*i+j*j+k*k;
  float lsX=(line->origin.x-sp->c.x),lsY=(line->origin.y-sp->c.y),
    lsZ=(line->origin.z-sp->c.z);
  float b=2*i*lsX+2*j*lsY+2*k*lsZ;
  float c=lsX*lsX+lsY*lsY+lsZ*lsZ-sp->rad*sp->rad;
  float disc=b*b-4*a*c,res1,res3;
  if(disc<0)
    return -1;
  if(disc==0)
    return 0;
  disc=sqrt(disc);
  res1=(-b+disc)/(2*a);
  res3=(-b-disc)/(2*a);
  if(res1<res3)
    {
      *res=res1;
      *res2=res3;
    }
  else
    {
      *res=res3;
      *res2=res1;
    }
  return 1;
}
  */


#define NUMDIM	3
#define RIGHT	0
#define LEFT	1
#define MIDDLE	2

int rayBoxInt(AABB *box, ray *ray,vertex *intCoord)
{
  float minB[NUMDIM]={box->min.x,box->min.y,box->min.z};
  float maxB[NUMDIM]={box->max.x,box->max.y,box->max.z};	  /*box */
  float origin[NUMDIM]={ray->origin.x,ray->origin.y,ray->origin.z},
    dir[NUMDIM]={ray->dir.x,ray->dir.y,ray->dir.z};  		  /*ray */
  float coord[NUMDIM];				           /* hit point */

  char inside = TRUE;
  char quadrant[NUMDIM];
  register int i;
  int whichPlane;
  float maxT[NUMDIM];
  float candidatePlane[NUMDIM];

  /* Find candidate planes; this loop can be avoided if
   	rays cast all from the eye(assume perpsective view) */
  for (i=0; i<NUMDIM; i++)
    if(origin[i] < minB[i]) {
      quadrant[i] = LEFT;
      candidatePlane[i] = minB[i];
      inside = FALSE;
    }else if (origin[i] > maxB[i]) {
      quadrant[i] = RIGHT;
      candidatePlane[i] = maxB[i];
      inside = FALSE;
    }else	{
      quadrant[i] = MIDDLE;
    }

  /* Ray origin inside bounding box */
  if(inside)	{
    intCoord->x=origin[0];
    intCoord->y=origin[1];
    intCoord->z=origin[2];
    return (TRUE);
  }


  /* Calculate T distances to candidate planes */
  for (i = 0; i < NUMDIM; i++)
    if (quadrant[i] != MIDDLE && dir[i] !=0.)
      maxT[i] = (candidatePlane[i]-origin[i]) / dir[i];
    else
      maxT[i] = -1.;

	/* Get largest of the maxT's for final choice of intersection */
  whichPlane = 0;
  for (i = 1; i < NUMDIM; i++)
    if (maxT[whichPlane] < maxT[i])
      whichPlane = i;

	/* Check final candidate actually inside box */
  if (maxT[whichPlane] < 0.) return (FALSE);
  for (i = 0; i < NUMDIM; i++)
    if (whichPlane != i) {
      coord[i] = origin[i] + maxT[whichPlane] *dir[i];
      if (coord[i] < minB[i] || coord[i] > maxB[i])
	return (FALSE);
    } else {
      coord[i] = candidatePlane[i];
    }
  intCoord->x=coord[0];
  intCoord->y=coord[1];
  intCoord->z=coord[2];

  return (TRUE);				/* ray hits box */
}

/*
int rayBoxInt(t_AABB *box,t_ray *ray,float &tN,float &tF)
{
  float tNear=-99999999999,tFar=99999999999;
  int k;

  for(k=0;k<3;k++)
    {
      if(ABS
      */

int rayBoxInt(AABB *box, ray *ray,float &tN,float &tF)
{
  float boxExtents[3],p[3],tNear=-999999,tFar=999999,
    d[3]={ray->dir.x,ray->dir.y,ray->dir.z},t1,t2;
  vertex temp;
  int counter;

  VMIDPOINT((box->min),(box->max),temp);
  p[0]=ray->origin.x-temp.x;
  p[1]=ray->origin.y-temp.y;
  p[2]=ray->origin.z-temp.z;

  BOXHALFSIZE(box,temp);
  boxExtents[0]=temp.x;
  boxExtents[1]=temp.y;
  boxExtents[2]=temp.z;
  
  for(counter=0;counter<3;counter++)
    {
      if(d[counter]==0)
	{
	  if(p[counter]>boxExtents[counter]||p[counter]<-boxExtents[counter])
	    return FALSE;
	}
      t1=(-boxExtents[counter]-p[counter])/d[counter];
      t2=(boxExtents[counter]-p[counter])/d[counter];
      if(t1>t2)
	swap(t1,t2);
      if(t1>tNear)
	tNear=t1;
      if(t2<tFar)
	tFar=t2;
      if(tNear>tFar)
	return FALSE;
      if(tFar<0)
	return FALSE;
    }
  tN=tNear;
  tF=tFar;
  return TRUE;
}


int rayBoxInt( AABB *box, ray *ray,vertex *intNear,vertex *intFar)
{
  float tNear,tFar;
  vertex temp;
  if(rayBoxInt(box,ray,tNear,tFar)==FALSE)
    return FALSE;
  VMULTF((ray->dir),tNear,temp);
  VADDV(temp,(ray->origin),(*intNear));
  VMULTF((ray->dir),tFar,temp);
  VADDV(temp,(ray->origin),(*intFar));
  return TRUE;
}


void line3d(int x1,int y1,int z1,int x2,int y2,int z2)
  {
    int xd, yd, zd;
    int x, y, z;
    int ax, ay, az;
    int sx, sy, sz;
    int dx, dy, dz;

    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;

    ax = ABS(dx) << 1;
    ay = ABS(dy) << 1;
    az = ABS(dz) << 1;

    sx = SIGN(dx);
    sy = SIGN(dy);
    sz = SIGN(dz);

    x = x1;
    y = y1;
    z = z1;

    if (ax >= MAX(ay, az))            /* x dominant */
    {
        yd = ay - (ax >> 1);
        zd = az - (ax >> 1);
        for (;;)
        {
          //  point3d(x, y, z);
            if (x == x2)
            {
                return;
            }

            if (yd >= 0)
            {
                y += sy;
                yd -= ax;
            }

            if (zd >= 0)
            {
                z += sz;
                zd -= ax;
            }

            x += sx;
            yd += ay;
            zd += az;
        }
    }
    else if (ay >= MAX(ax, az))            /* y dominant */
    {
        xd = ax - (ay >> 1);
        zd = az - (ay >> 1);
        for (;;)
        {
          //  point3d(x, y, z);
            if (y == y2)
            {
                return;
            }

            if (xd >= 0)
            {
                x += sx;
                xd -= ay;
            }

            if (zd >= 0)
            {
                z += sz;
                zd -= ay;
            }

            y += sy;
            xd += ax;
            zd += az;
        }
    }
    else if (az >= MAX(ax, ay))            /* z dominant */
    {
        xd = ax - (az >> 1);
        yd = ay - (az >> 1);
        for (;;)
        {
          //  point3d(x, y, z);
            if (z == z2)
            {
                return;
            }

            if (xd >= 0)
            {
                x += sx;
                xd -= az;
            }

            if (yd >= 0)
            {
                y += sy;
                yd -= az;
            }

            z += sz;
            xd += ax;
            yd += ay;
        }
    }
}




//---------------- quadratic ---------------------------/
//- USE: Computes the quadratic equation (which solves -/
//-		 equations of the form ax^2 + bx + c = 0, or   -/
//-		 of the form (x + t1)(x + t2) = 0.  The whole  -/
//-      eqation is:           ___________             -/
//-                    -b +/- V b^2 - 4ac              -/
//-           t1,t2 = -----------------------          -/
//-                              2a                    -/
//- RETURN: TRUE if t1 and t2 exist - t1 and t2 are    -/
//-		 roots of the quadratic equation.  False if no -/
//-		 solution exists.							   -/
//------------------------------------------------------/
int quadratic(float a, float b, float c, float &t1,float &t2)
{
  float q=b*b-4*a*c;
  if(q<0)
    return FALSE;
  float sq=sqrtf(q), d = 1.0f / (2.0f * a);
  t1=(-b-sq)*d;
  t2=(-b+sq)*d;
  return TRUE;
}




//------------------------ pointOnLine ---------------------------------/
//- USE:This function 'snaps' a point to a line, defined by a starting -/
//-		point, and a unit vector direction.  The point is set at the   -/
//-		closest point on the line.                                     -/
//- RETURN: The x value in [ f(x) = dir * x + origin ] is returned.    -/
//----------------------------------------------------------------------/
float pointOnLine( vertex *point, ray * line)
{
	vertex ptMinusOrigin;

	VSUBV((*point),(line->origin),ptMinusOrigin);
    return VDOT((line->dir),ptMinusOrigin)/VDOT((line->dir),(line->dir));
}





//----------------------- checkSphereInCone -------------------------------/
//- USE: Given a cone defined by a origin, unit vector direction, a total -/
//-		 length, and rad at the unit length, and a sphere defined by a    -/
//-		 center and rad, the function tests the volumes for intersection. -/
//- RETURN: True if the volumes intersect, Fasle if they don't.           -/
//-------------------------------------------------------------------------/
bool checkSphereInCone( sphere *s, cone *c )
{
	float distAlongAxis = pointOnLine( &(s->c), (ray *)c);
	vertex closestPtAxis = c->origin + distAlongAxis * c->dir;
	float distAxisOrigin = (float)VertLen( &closestPtAxis, &(s->c) );

	if( distAlongAxis > c->length )
	{
		vertex endCone = c->origin + c->dir * c->length;
		float radAtConeEnd = c->length * c->unitRad;
		vertex closestPtCone = endCone + (radAtConeEnd * ( s->c - closestPtAxis))/distAxisOrigin;

		if( VertLenSqr( &(s->c), &closestPtCone ) < s->rad * s->rad)
			return TRUE;
		return FALSE;
	}

	float radAtClosestPt = c->unitRad * distAlongAxis;

	if( distAxisOrigin < s->rad + radAtClosestPt )
		return TRUE;
	return FALSE;
}

//----------------------------------------/
//-- computeLightDist                    -/
//--  DESC: Given the atten factors, the -/
//--   maximum distance the light can go -/
//--   is returned.  Use to set Proj mat -/
//--   and other data for shadows.       -/

float computeLightDist( float aC,  float aL, float aQ)
{
	if( aC < F_EPS )
		aC = 1.0f;
	if( aQ < F_EPS )
	{
		if( aL < F_EPS )
			return 999999.99f;
		else
			return ((256 - aC)/aL);
	}
	else
	{
		if( aL < F_EPS )
			return sqrtf  ((256.0f - aC)/ aQ);
		else return ((aL + sqrtf(aL * aL - 4 * aQ * ( aC - 256)))/ ( 2 * aC - 510));
	}

	return 999999.99f;
}

