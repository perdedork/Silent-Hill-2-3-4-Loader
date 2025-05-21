/*ALL UNIVERSAL DATA TYPES FOR EACH FILE'S USE.  If at all possible, use only *
 *  one lightmap that ccovers all possible shadow forms.  If this is the case *
 *  remove all lightmap fields from data types, except the lightmap texcoords *
 *****************************************************************************/


#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glprocs.h>

#include <iostream>
#include <stdlib.h>
//#include "glut.h"
#include "vertex.h"
#include "quat.h"
#include "matrix.h"
//#include "LinkedList.H"



//####################################################################
//##########   A WHOLE BUNCH OF SHIT THAT NEEDS TO CHANGE IN HERE
//####  MOST OF IT ISN'T USED ANYWAYS... CHECK TO SEE, AND CHANGE ACCORDINGLY, AND
//############## PLACE IN APPROPRIATE OTHER CLASSES, ie CollisionDetection, Texture,
//#######  Geometry Data Structs, ect..




typedef struct{
	WORD w,h;
}dim2d;

const dim2d dScreenRes[]={{640,480},{800,600},{1024,768},{1280,1024},{1600,1200},{1280,800}};

enum resScreen {_640x480 = 0, _800x600, _1024x768, _1280x960, _1280x1024, _1600x1200, _1280x800 };
enum refreshScreen {r60 = 60 ,r70 = 70 ,r72 = 72 ,r75 = 75 ,r85 = 85 ,r100 = 100};



#ifndef LOG_OUTPUT
#define LOG_OUTPUT

#define ERROR_LOG   1
#define DATA_LOG    2
#define TEST_LOG    3

static void LogFile(int logType, char *s, ...)
{
	static int e1st = 1, d1st = 0, t1st = 1;
	char *filename = NULL;
	char errorFile[] = "Error.txt", dataFile[] = "Data.txt", testFile[] = "Test.txt";
	char openString[3];
	va_list args;
	char writeBuf[4096];
	FILE *logfile;
	size_t res;

	switch(logType)
	{
	case DATA_LOG: filename = dataFile;
		if(d1st == 0)
		{
			strcpy(openString,"w+");
			d1st = 1;
		}
		else strcpy(openString,"a+");
		break;
	case TEST_LOG: filename = testFile;
		if(t1st == 0)
		{
			strcpy(openString,"w+");
			t1st = 1;
		}
		else strcpy(openString,"a+");
		break;
	case ERROR_LOG: filename = errorFile;
		if(e1st == 0)
		{
			strcpy(openString,"w+");
			e1st = 1;
		}
		else strcpy(openString,"a+");
		break;
	}

	logfile = fopen(filename, openString);

	if(logfile == NULL)
		return;
	va_start( args, s );
	vsprintf( writeBuf, s, args );
	va_end( args );

	if((res = fwrite( writeBuf,sizeof(char), strlen(writeBuf), logfile)) != strlen(writeBuf))
	{
		char msg[45];
		sprintf(msg,"\nLOG ERROR: Only wrote %d/%d bytes\n",res,strlen(writeBuf));
		fwrite(msg,sizeof(char),strlen(msg),logfile);
	}
	if(fwrite("\n",sizeof(char),1,logfile)!=1)
	{
		char msg[45];
		sprintf(msg,"\nLOG ERROR: Unable to write out EOL");
		fwrite(msg,sizeof(char),strlen(msg),logfile);
	}

	fclose(logfile);
}


static void checkGLerror(int lineNum, char *filename,char *module)
{
	GLenum errorCode;
	int    eFlag = 0;
	while((errorCode=glGetError())!=GL_NO_ERROR && eFlag < 10)
	{	
		LogFile(ERROR_LOG,"'%s'(%d) openGL ERROR::%s  :%s (%d)\n",filename,lineNum,module,gluErrorString(errorCode),eFlag);
		eFlag ++;
	}

	if(eFlag) PostQuitMessage( 1 ); //exit(1);
}

static void HexDump(int numBytes,char *buf,FILE *fp)
{
  int i,j,k;

  fprintf(fp,"\nReceived %d bytes::\n",numBytes);

  for(i=0;i<numBytes;i+=16)
    {
      int place=0;
      char tempOut[18];
      for(j=0;j<16 && (numBytes-(i+j));j++)
        {
          fprintf(fp,"%.2x ",(unsigned char)buf[i+j]);
          if(isgraph(buf[i+j]))
            tempOut[place]=buf[i+j];
          else tempOut[place]='.';
          place++;

          if(j==7)
            {
              fprintf(fp,"   ");
              tempOut[place]=' ';
              place++;
            }
        }
      tempOut[place]='\0';
	        for(k=0;k<16-j;k++)
        {
          fprintf(fp,"   ");
          if(k==7)
            fprintf(fp,"   ");
        }
      fprintf(fp,"%s\n",tempOut);
    }
}

#endif

static GLboolean IsExtensionSupported(char *checkExtension)
{
    const GLubyte  *s;
	const GLubyte *glString=glGetString(GL_EXTENSIONS);
	GLint len;
	HDC tHdc = wglGetCurrentDC();
	s = glString;
	len = strlen (checkExtension);
	LogFile(ERROR_LOG,(char *)glString);

	while ((s =(GLubyte *) strstr ((char *)s,checkExtension)) != NULL) {
		s += len;

		if ((*s == ' ') || (*s == '\0')) {
			return (GL_TRUE);
		}
	}

	glString = (const GLubyte *)wglGetExtensionsStringARB(tHdc);
	LogFile(ERROR_LOG,(char *)glString);
	s = glString;
	len = strlen (checkExtension);

	while ((s =(GLubyte *) strstr ((char *)s,checkExtension)) != NULL) {
		s += len;

		if ((*s == ' ') || (*s == '\0')) {
			return (GL_TRUE);
		}
	}

	return (GL_FALSE);
}

/*****************************************************
 *                    BASIC TYPES                    *
 *    USED FOR BUILDING MORE ADVANCED STRUCTS AND    *
 *    HOLDING SIMPLE DATA                            *
 *****************************************************/



#ifndef UNDEFINED
#define UNDEFINED -1
#endif

#ifndef UNDEF
#define UNDEF -1
#endif



const double F_EPS = 0.0000001;


#define mDEBUG 1









typedef enum e_clip{ tOUT=0,tIN=1};  //typedef for clipping results
typedef enum e_type{tCHAR,tUCHAR,tSHORT,tUSHORT,tINT,tUINT,tFLOAT,tDOUBLE};
typedef enum e_list{tINCLUDE_LIST,tEXCLUDE_LIST,tBITWISE_LIST};
typedef enum eBoundingShape{tSPHERE,tCUBE,tCONE,tELIPSOID,tLINE,tPOLY,tPOINT};


/**************************************************************************/
/*              G E O M E T R I C   P R I M I T I V E S                   */
/* These are typedefs for geometric primitives used in collision detection*/
/* bounding, occlusion, ect.                                              */
/**************************************************************************/

typedef struct{
  vertex c;        //Center of Sphere
  float rad;       //Radius of the Spere
}sphere;


typedef struct{
  vertex c;        //Center of Box
  float halfWidth; //Half of Width  -like the X val
  float halfHeight;//Half of Height -like the Y val
  float halfDepth; //Half of Depth  -like the Z val
}t_box;

typedef struct{
  vertex min,max;  //Minimum & maximum extents
}AABB;

typedef struct{
  vertex origin;   //Start of line
  vertex dir;      //direction of line -a 2nd point on the line,or the endpoint
}t_line;


typedef struct{
  vertex origin;  //start of ray
  vertex dir;     //normalized direction (a normal)
}ray;   


typedef struct{
	vertex origin;	//Origin/tip of cone
	vertex dir;	//Unit Vec for direction of center axis of cone
	float  length;	//Length of cone
	float  unitRad;	//Radius of cone at length of 1
}cone;


/**************************************************************************/
/* BASIC GRAPHIC TYPES, BITMAP TYPES, RENDERING STRUCTS                   */
/*    Used for storage of graphical structures, loaded from OBJ files, or */
/*    BASIC GRAPHICS FILE TYPES ready to render w/ open GL                */
/**************************************************************************/


       //DESC:t_tristrip_base - stores a triangle strip for group surface that 
       //     uses one type of texture.  Used for preprocessing before being
       //     made into a display list, using vert/texcoord arrays, before
       //     final openGL processing is done
typedef struct{
  int *v;       //verticies
  int *t;       //texcoords
  int *l;       //lightmap coords
  int numTri;   //number of tri in tri-strip: remeber # of v=t=numTri+2
  int texMap;   //index to texture map
  int lightMap; //index to light map
}t_tristrip_base;



/*--------------------------------------------------------------*
 | NOTE: Eventually change this class to "static", depending on |
 |       how it is used in the programs.  If only one instance  |
 |       is used at a time, change it to static, otherwise, let |
 |       it stay as it is now.                                  |
 *--------------------------------------------------------------*/
class Attrib{
public:
  Attrib(){aVal=0;}
  Attrib(long a){aVal=a;}
  ~Attrib(){}
  inline void setAttrib(long aV){aVal=aV;}
  inline int has(long valToCheck){return valToCheck&aVal;}
   inline long getAll(){return aVal;}
private:
  long aVal;
};


/************    R E N D E R A B L E   T Y P E S *******************/

             /*THESE ARE THE TYPES THAT HAVE BEEN PROCESSED, */
                    /* AND ARE READY FOR OPEN GL*/

//DEFINES FOR FLAGS FOR DATA ELEMENTS CONTAINED IN RENDERABLE TYPES
       /*These say what kind of data is in group types*/
#define R_TEXCOORDS    0x00000001    //group has texcoords
#define R_NORMALS      0x00000002    //group has vertex normals
#define R_VERTEX_CLR   0x00000004    //group has vertex colors
#define R_LIGHTMAP     0x00000008    //group has a lightmap, & texcoords for it

       /*These say how the data is stored to be rendered*/
#define R_TRISTRIP     0x00000100    //group is stored as tristrips
#define R_TRISTRIP_MT  0x00000200    //group is stored as multitex'd tristrips
#define R_INDEXED_OBJ  0x00000400    //group is stored as indexed obj
#define R_INDEXED_MT   0x00000800    //group is stored as multitex'd index obj

       /*These pertain to texture info, like wether it is a texOBJ or index*/
#define R_GL_TEX_OBJ   0x00010000    //Texture is stored as an glTexObj
#define R_TEXMAP_IND   0x00020000    //Texture is just a texmap to be registerd

       /*These determine the type of t_group_dat struct is used in the group*/
#define R_GROUP_DAT    0x01000000    //Group data has a single texture
#define R_GROUP_DAT_MT 0x02000000    //Group data w/ data for mltitexturing



/**************************************************
 *                    MACROS                      *
 **************************************************/

/*BASIC TYPE MACROS
 *    these are used to do simple things w/ the basic
 *    types defined at the beginning of this file
 */
#define copyTexcoord(a,b)      (a.t=b.t,a.s=b.s)
#define copyRGB(x,y)           (x.r=y.r,x.g=y.g,x.b=y.b)
#define copyRGBA(x,y)          (x.r=y.r,x.g=y.g,x.b=y.b,x.a=y.a)
#define copy2RGBA(x,y)         (x.r=y.r,x.g=y.g,x.b=y.b,x.a=255)
//#define setTexcoord(v,a,b)     (v.s=a, v.t=b)
#define setRGB(p,d,e,f)        (p.r=d, p.g=e, p.b=f)


/*HELPERS
 *  auxillary definitions and types to help readability, coding, and
 *  to prevent errors
 */

#define SAFEDELETE(x){if((x)!=NULL){delete [] (x);(x)=NULL;}}
#define MEMALLOCATE(x,s){if(x==NULL){cout<<s<<":Memory allocation error..."<< \
                                        "\texiting.."<<endl;exit(1);}}
#define ARRAYLEN(x)  (sizeof(x)/sizeof(x[0]))
#define FILEREAD(d,s,f){int fres; if((fres = fread(d,1,s,f)) != s ){ LogFile(ERROR_LOG,"");}}



#define MWHEEL_UP          3
#define MWHEEL_DOWN        4



//---------------------------------------------------------------------------/
//--                          Printing Macros                              --/
//--   These macros print out names of variables, seperate lines, and will --/
//--   print to strings, and message boxes, when it is ported to windows.  --/
//  NOTE: Set #ifdef DEBUG symbols to pass variables and print out values    /
//  in certain cases


//---  Prints on same line, and leaves end there ---/
#define print(x)        cout<<x

//---  Prints on a new line, and advances to next line ---/
#define printnewline(x) cout<<"\n"<<x<<endl

//---  Prints on a new line, and leaves cursor at end ---/
#define printline(x)    cout<<"\n"<<x

//---  Prints on same line, and advaces to next line ---/
#define printendline(x) cout<<x<<endl

//---  This macro creates a section with a variable name, = sign, and val ---/
#define c_Var(x)        (#x)<<(" = ")<<(x)

//---  Makes a variable name string  ---/
#define c_Str(a)     #a

//---  Concatanates x and y based on what they are ---/
#define c_Cat(x,y)   x##y

//---  Concatonates the name of x, and the val of y ---/
#define c_Name_Cat_Val(x,y)  c_Str(x)##y

//---  Returns the value of a variable  ---/
#define c_Xstr(x)   c_Str(x)

//---  Simple macro to print out a variable and it's name on a newline ---/
#define printVar(x)    printnewline(c_Var(x))

//#define mDEBUG 1

//---  DEBUG COMMANDS  ---/
#ifdef mDEBUG
#define d_set(v,n) \
     printline(c_Str(The old variable val is:)),printendline(c_Var(v)),\
     v=n,\
     printline(c_Str(The new variable val is:)),printendline(c_Var(v))
#else
#define d_set(v,n) v=n
#endif

#define debugLong( v ) ( LogFile(ERROR_LOG,"\tLong  %s: %ld",#v,v))
#define debugLongArray( v, c ) { LogFile(ERROR_LOG,"\tLong  %s:",#v);int x;for(x=0;x< c;x++)LogFile(ERROR_LOG,"\t\t%d - [ %ld ]",x,v[x]);}
#define debugFloat( v ) ( LogFile(ERROR_LOG,"\tFloat %s: %f",#v,v))
#define debugFloatArray( v, c ) { LogFile(ERROR_LOG,"\tFloat %s:",#v);int x;for(x=0;x< c;x++)LogFile(ERROR_LOG,"\t\t%d - [ %f ]",x,v[x]);}
#define debugShort( v ) ( LogFile(ERROR_LOG,"\tShort %s: %d",#v,v))
#define debugShortArray( v, c ) { LogFile(ERROR_LOG,"\tShort %s:",#v);int x;for(x=0;x< c;x++)LogFile(ERROR_LOG,"\t\t%d - [ %d ]",x,v[x]);}
#define debugChar( v ) ( LogFile(ERROR_LOG,"\tChar  %s: %d",#v,v))
#define debugCharArray( v, c ) { LogFile(ERROR_LOG,"\tChar  %s:",#v);int x;for(x=0;x< c;x++)LogFile(ERROR_LOG,"\t\t%d - [ %d ]",x,v[x]);}
#define debugAll( v ) ( LogFile(ERROR_LOG,"\t%s: [ %08x ][ %.6f ] [ %u ] [ %d %d ] [ %d %d %d %d ]",#v,(unsigned long)(((long*)&(v))[0]),(float)(((float*)&(v))[0]),(unsigned long)(((long*)&(v))[0]),(unsigned short)(((short*)&v)[0]),(unsigned short)(((short*)&v)[1]),(unsigned char)(((char*)&v)[0]),(unsigned char)(((char*)&v)[1]),(unsigned char)(((char*)&v)[2]),(unsigned char)(((char*)&v)[3])))
#define debugAllArray( v, c ) { LogFile(ERROR_LOG,"\t%s:",#v);int x;for(x=0;x< c;x++)LogFile(ERROR_LOG,"\t\t%d - [ %08x ][ %.6f ] [ %u ] [ %d %d ] [ %d %d %d %d ]",x,(unsigned long)(((long*)&(v[x]))[0]),(float)(((float*)&(v[x]))[0]),(unsigned long)(((long*)&(v[x]))[0]),(unsigned short)(((short*)&v[x])[0]),(unsigned short)(((short*)&v[x])[1]),(unsigned char)(((char*)&v[x])[0]),(unsigned char)(((char*)&v[x])[1]),(unsigned char)(((char*)&v[x])[2]),(unsigned char)(((char*)&v[x])[3]));}
#define debugVertex( v ) ( LogFile(ERROR_LOG,"\t      %s: %f %f %f",#v,v.x,v.y,v.z))
#define debugVertex4f( v ) ( LogFile(ERROR_LOG,"\t      %s: %f %f %f %f",#v,v.x,v.y,v.z,v.w))
#define debugMatrix( v ) ( LogFile(ERROR_LOG,"\t%s: \n\t[[ %.3f\t%.3f\t%.3f\t%.3f ]\n\t [ %.3f\t%.3f\t%.3f\t%.3f ]\n\t [ %.3f\t%.3f\t%.3f\t%.3f ]\n\t [ %.3f\t%.3f\t%.3f\t%.3f ]]",#v,v.mat[0],v.mat[1],v.mat[2],v.mat[3],v.mat[4],v.mat[5],v.mat[6],v.mat[7],v.mat[8],v.mat[9],v.mat[10],v.mat[11],v.mat[12],v.mat[13],v.mat[14],v.mat[15]))

//---------------------------------------------------------------------------/
//--                         DataStruct Macros                             --/
//--  These macros ease the use of data structures that aren't classes.    --/
//--  Save space to avoid repeated code.                                   --/
//---------------------------------------------------------------------------/

#define END(x)     ((x)->next==NULL)
//#define NEXT(x)    if(!END(x))(x)=(x)->next;
#define LAST(x)    ((x)->front==NULL||(x)->back==NULL)
#define FRONT(x)   if((x)->front!=NULL)(x)=(x)->front;
#define BACK(x)    if((x)->back!=NULL)(x)=(x)->back;

#endif
