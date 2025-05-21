#include <windows.h>
#include <string>
#include <vector>
#include <gl\gl.h>
#include <gl\glext.h>
#include <gl\wglext.h>
#include <gl\glprocs.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iterator>

#include "vertex.h"
#include "plane.h"
#include "Renderer.h"
#include "tgaload.h"
#include "PerPixelLighting.h"



TexMgr::TexMgr()
{}

TexMgr::~TexMgr()
{
	DeleteAllTextures( );		
}


void TexMgr::DeleteAllTextures( )
{
	int k;

	for( k = 0; k < textures.size(); k++)
		glDeleteTextures(1,&textures[k].texID);
	for( k = 0; k < renderTargets.size(); k++ )
	{
		wglDeleteContext(renderTargets[k].hglrc);
		wglReleasePbufferDCARB(renderTargets[k].hPBuffer, renderTargets[k].hdc);
		wglDestroyPbufferARB(renderTargets[k].hPBuffer);
		glDeleteTextures(1, &renderTargets[k].texID );
	}

	textures.clear();
	renderTargets.clear();
}

void TexMgr::checkResident( )
{
	GLuint *texIDs;
	int numTexID;
	GLboolean *results;
	int k;

	
	

	numTexID = textures.size();

	texIDs = new GLuint [numTexID];
	results = new GLboolean [ numTexID ];

	for( k = 0; k < numTexID; k ++ )
	{
		texIDs[k] = textures[k].texID;
		//LogFile(ERROR_LOG,"TEXID=%d",texIDs[k]);
	}

	if(!glAreTexturesResident( numTexID, (const GLuint *)texIDs, results))
		LogFile(ERROR_LOG,"------------------------------------\nERROR: Could not get information on resident textures\n---------------------------------");

	for( k = 0; k < numTexID; k ++ )
	{
		if( !results[k] )
			LogFile(ERROR_LOG,"TexMgr::checkResident - Texture ID is no longer resident (%s - ID: %d )",textures[k].texName.c_str(),textures[k].texID);
	}

	delete [] texIDs;
	delete [] results;
}



int TexMgr::DeleteTex( const char * texName )
{
	int k;

	vector<texHandle>::iterator tItr;
	vector<texRenderTarget>::iterator tItr2;
	
	for( k = 0; k < textures.size(); k++ )
	{
	
	//	LogFile( ERROR_LOG, "Got Here");
	//	LogFile( ERROR_LOG, "texname = %s", texName);
		if( strcmp( texName, textures[k].texName.c_str() ) == 0 )
		{
			tItr = textures.begin();
		//	 LogFile( ERROR_LOG, "\tTEST: %s(%d): Size of textures before erase(1):%d\n",__FILE__,__LINE__,textures.size());
		//	 LogFile (ERROR_LOG, "Tex ID is %ld",textures[k].texID);
			glDeleteTextures( 1, &textures[k].texID );
		//	LogFile( ERROR_LOG, "Got Here");
		    advance( tItr, k );
		//	LogFile( ERROR_LOG, "Got Here 3");
			textures.erase( tItr );
			 //LogFile( TEST_LOG, "\tTEST: %s(%d): Size of textures AFTER erase(1):%d\n",__FILE__,__LINE__,textures.size());
			return 1;
		}
	}

	for( k = 0; k < renderTargets.size(); k++ )
	{
	
		if( strcmp( texName, renderTargets[k].texName.c_str() ) == 0 )
		{
			tItr2 = renderTargets.begin();
		//	 LogFile( TEST_LOG,"\tTEST: %s(%d): Size of renderTargets before erase(1):%d\n",__FILE__,__LINE__,renderTargets.size());
			wglDeleteContext(renderTargets[k].hglrc);
			wglReleasePbufferDCARB(renderTargets[k].hPBuffer, renderTargets[k].hdc);
			wglDestroyPbufferARB(renderTargets[k].hPBuffer);
			glDeleteTextures( 1, &renderTargets[k].texID );
		    advance( tItr2, k );
			renderTargets.erase( tItr2 );
		//	LogFile( TEST_LOG, "\tTEST: %s(%d): Size of renderTargets AFTER erase(1):%d\n",__FILE__,__LINE__,renderTargets.size()); 
			return 1;
		}
	}
	return 0;
}


int TexMgr::DeleteTex( GLuint texID )
{
	int k;
    
	vector<texHandle>::iterator tItr;		
	vector<texRenderTarget>::iterator tItr2;		

	for( k = 0; k < textures.size(); k++ )
	{
			
		if( textures[k].texID == texID )
		{
			tItr = textures.begin();
			/*T*/char testStr[100]; sprintf(testStr,"\tTEST: %s(%d): Size of textures before erase(1):%d\n",__FILE__,__LINE__,textures.size()); LogFile( TEST_LOG, testStr);
			glDeleteTextures( 1, &textures[k].texID );
	    	advance( tItr, k );
			textures.erase( tItr );
			///*T*/sprintf(testStr,"\tTEST: %s(%d): Size of textures AFTER erase(1):%d\n",__FILE__,__LINE__,textures.size()); LogFile( TEST_LOG, testStr);
			return 1;
		}
	}
	

	
	

	for( k = 0; k < renderTargets.size(); k++ )
	{	
		if( renderTargets[k].texID == texID )
		{
			tItr2 = renderTargets.begin();
			/*T*/char testStr[100]; sprintf(testStr,"\tTEST: %s(%d): Size of renderTargets before erase(1):%d\n",__FILE__,__LINE__,renderTargets.size()); LogFile( TEST_LOG, testStr);
			wglDeleteContext(renderTargets[k].hglrc);
			wglReleasePbufferDCARB(renderTargets[k].hPBuffer, renderTargets[k].hdc);
			wglDestroyPbufferARB(renderTargets[k].hPBuffer);
			glDeleteTextures( 1, &renderTargets[k].texID );
			advance( tItr2, k );
			renderTargets.erase( tItr2 );
			/*T*/sprintf(testStr,"\tTEST: %s(%d): Size of renderTargets AFTER erase(1):%d\n",__FILE__,__LINE__,renderTargets.size()); LogFile( TEST_LOG, testStr);
			return 1;
		}
	}
	return 0;
}


int TexMgr::DeleteTex( int index )
{	
	vector<texHandle>::iterator tItr;		
	
	tItr = textures.begin( );
	LogFile( ERROR_LOG, "\tTEST: %s(%d): Size of textures before erase(1):%d\n",__FILE__,__LINE__,textures.size());
	LogFile( ERROR_LOG, "CHECK - Index = %d",index);
	glDeleteTextures( 1, &textures[index].texID );
	LogFile( ERROR_LOG, "Here?");
	advance( tItr, index );
	LogFile( ERROR_LOG, "Here?");
	textures.erase( tItr );
	LogFile( ERROR_LOG, "Here?");
	///*T*/sprintf(testStr,"\tTEST: %s(%d): Size of textures AFTER erase(1):%d\n",__FILE__,__LINE__,textures.size()); LogFile( TEST_LOG, testStr);
	return 1;
}


int TexMgr::DeleteRenderTarget( int index )
{
	vector<texRenderTarget>::iterator tItr;		

	tItr = this->renderTargets.begin( );
	/*T*/char testStr[100]; sprintf(testStr,"\tTEST: %s(%d): Size of renderTargets before erase(1):%d\n",__FILE__,__LINE__,renderTargets.size()); LogFile( TEST_LOG, testStr);
	wglDeleteContext(renderTargets[index].hglrc);
	wglReleasePbufferDCARB(renderTargets[index].hPBuffer, renderTargets[index].hdc);
	wglDestroyPbufferARB(renderTargets[index].hPBuffer);
	glDeleteTextures( 1, &renderTargets[index].texID );
	advance( tItr, index );
	renderTargets.erase( tItr );
	/*T*/sprintf(testStr,"\tTEST: %s(%d): Size of renderTargets AFTER erase(1):%d\n",__FILE__,__LINE__,renderTargets.size()); LogFile( TEST_LOG, testStr);
	return 1;
}

GLuint TexMgr::GetTexture( string & texName )
{
	int k, place = texName.find_last_of("/");
	string tempString = texName.substr( (place < 0)?0:place );

	for( k = 0; k < textures.size(); k++)
	{
		if( textures[k].texName == tempString )
			return textures[k].texID;
	}

	texHandle newTex;

	newTex.texName = tempString;
	if( (newTex.texID = tgaLoadAndBind( texName.c_str(), TGA_FREE | TGA_NO_MIPMAPS, &(newTex.flags) ))==0)
		return 0;
LogFile(ERROR_LOG,"TexMgr::GetTexture - what the hell did it load?");
	newTex.flags |= TEXM_2D_TEX;

	textures.push_back( newTex );
	return newTex.texID;
}


GLuint TexMgr::GetEnvironmentMap()
{
	LogFile(ERROR_LOG, "This Function TexMgr::GetEnvironmentMap Needs to be Written\n");
	return 0;
}

         
GLuint TexMgr::GetReflectionMap()
{
	LogFile(ERROR_LOG, "This Function TexMgr::GetReflectionMap Needs to be Written\n");
	return 0;
}


GLuint TexMgr::SetReflectionMap(plane *reflectPlane, string & reflectTex)
{
	LogFile(ERROR_LOG, "This Function TexMgr::SetReflectionMap Needs to be Written\n");
	return 0;
}


int TexMgr::CreateCubeReflectMap( char *rName, rgbaf *reflectColor, vertex * pos, int _flags )
{
	int sRes, texNum; 
	texRenderTarget tempCubeReflect;
	int flags;
	GLuint texID;
	reflectSetup *eVal;
char messageStr[300];

	
	flags = _flags | TEXM_CUBE_RENDER | TEXM_CUBEMAP | TEXM_32_BPP | TEXM_REFLECT_COLOR;
	sRes = (_flags & TEXM_RES_128)?128:(_flags & TEXM_RES_512)?512:(_flags & TEXM_RES_1024)?1024:512;
	eVal = new reflectSetup;
	copyRGBA((*eVal).reflectColor,(*reflectColor));
	copyVertex((*eVal).centerPos, (*pos));

	if(IsExtensionSupported("WGL_ARB_render_texture"))
	{
		flags |=TEXM_PBUFFER;
		LogFile(ERROR_LOG,"EXTENSION: WGL_ARB_render_texture is supported!!");
	}
	else
	{
		flags |=TEXM_PBUFFER;
		flags ^=TEXM_PBUFFER;
		/*TEST*/if(flags|TEXM_PBUFFER)LogFile(ERROR_LOG,"\n\tPBUFFER IS SET, IT SHOULD NOT BE!!\n\n");
		LogFile(ERROR_LOG,"MISSING EXTENSION: WGL_ARB_render_texture is NOT supported!!");
	}
	
	texID = SetupRender( rName, sRes, sRes, flags );

	texNum = GetRenderTex( rName);

	renderTargets[texNum].extra = (void *)eVal;

	sprintf(messageStr," DIAG: The texID is %d, and the texNum is %d",texID, texNum);
	if(TestLost(texNum)==GL_TRUE)
		strcat(messageStr,"\nERROR - CreateCubeReflectionMap::TestLost - PBuff lost in the Init Func");
	LogFile(ERROR_LOG,messageStr);

	return texNum;
}
		


	



GLuint TexMgr::SetEnvironmentMap(vertex *location,int res, string & envTex)
{
	texHandle cubeTex;
	GLubyte *buff = new GLubyte[ res * res * 3 ];
	int j, k, size = res >> 2;

	glGenTextures( 1, &cubeTex.texID );
	glEnable( GL_TEXTURE_CUBE_MAP );
	glBindTexture( GL_TEXTURE_CUBE_MAP, cubeTex.texID );

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);//REPEAT );
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);//REPEAT );
	//glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
	glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
	glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );
	glEnable( GL_TEXTURE_GEN_R ); 

	for( j = 0; j < res; j++)
	{
		for( k = 0; k < res; k++)
		{
			buff[ (j * res + k) * 3 ] = j;
			buff[ (j * res + k) * 3 + 1] = 255 - ((j + k) >> 1) ;
			buff[ (j * res + k) * 3 + 2] = k;
		}
	}

	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_RGB8, res, res, 0, GL_RGB, GL_UNSIGNED_BYTE, buff );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_RGB8, res, res, 0, GL_RGB, GL_UNSIGNED_BYTE, buff );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_RGB8, res, res, 0, GL_RGB, GL_UNSIGNED_BYTE, buff );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_RGB8, res, res, 0, GL_RGB, GL_UNSIGNED_BYTE, buff );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_RGB8, res, res, 0, GL_RGB, GL_UNSIGNED_BYTE, buff );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_RGB8, res, res, 0, GL_RGB, GL_UNSIGNED_BYTE, buff );

	delete [] buff;

	textures.push_back( cubeTex );
	return cubeTex.texID;
}



void   TexMgr::AddTex( string & texName, GLuint id, int flags)
{
	texHandle newTex;

	newTex.texName = texName;
	newTex.texID = id;	
	if( flags == 0 )
		newTex.flags |= TEXM_2D_TEX | TEXM_32_BPP;
	else
		newTex.flags = flags;

	textures.push_back( newTex );
}

int    TexMgr::BindTex( char * texName, int texUnit )
{
	int index=0;

	while( index < textures.size() )
	{
		if( strcmp( texName, textures[index].texName.c_str() )==0)
			return BindTex( index, texUnit );
		index++;
	}
	return -1;
}


int    TexMgr::BindTex( int texIndex, int texUnit )
{
	GLenum texType = (textures[texIndex].flags & TEXM_2D_TEX )? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_ARB;
	
	glActiveTextureARB(GL_TEXTURE0_ARB + texUnit );
	glBindTexture( texType, textures[ texIndex ].texID );
	glEnable( texType );

	return texIndex;
}


int TexMgr::TestLost(int index)
{
	int lost=0;

	if( index > -1 && index < renderTargets.size())
	{
		if(!wglQueryPbufferARB(renderTargets[index].hPBuffer, WGL_PBUFFER_LOST_ARB, &lost))
			checkGLerror(__LINE__,__FILE__,"Problem w/ PBuffer in TexMgr::TestLost()");
		return lost;
	}
	return -1;
}

void   TexMgr::TexRender( int target )
{

	if( target == -1 && lastDC != (struct HDC__ *)-1 && lastRC != (struct HGLRC__ *)-1)
	{
		if(!wglMakeCurrent( lastDC, lastRC ))
			LogFile(ERROR_LOG,"\n\nERROR: Unable to set the last DC as the render target\n");
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		return;
	}

	if( target >= renderTargets.size() || target < -1 )
	{
		char msg[128];
		sprintf( msg, "'%s'(%d): Render target '%d' out of range - current max is %d.\n\tDoing nothing.\n"
			,__FILE__,__LINE__,	target, renderTargets.size() - 1 );
		LogFile( ERROR_LOG, msg );
		return;
	}
	
	if(renderTargets[target].hPBuffer == 0 || renderTargets[target].hdc == 0 || renderTargets[target].hglrc == 0 )
	{
		char msg[128];
		sprintf( msg, "WARNING: Render Tex[%d] does not have a PBUFFER, HDC, or HGLRC",target);
		LogFile( ERROR_LOG,msg);
		return;
	}
	lastDC = (struct HDC__ *)-1;
	lastRC = (struct HGLRC__ *)-1;

	lastDC = wglGetCurrentDC();
	lastRC = wglGetCurrentContext();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);

checkGLerror(__LINE__,__FILE__,"After getting Current DC and Context in TexMgr::TexRender()");
	if(!wglMakeCurrent( renderTargets[ target ].hdc, renderTargets[ target ].hglrc ))
		LogFile(ERROR_LOG,"\n\nERROR: Unable to set the PBUFFER as the render target\n");
checkGLerror(__LINE__,__FILE__,"After setting DC and Render Context  to new values in TexMgr::TexRender()");

}

void   TexMgr::CubeRender( int target, int side)
{
	glFlush();
	vertex from, at;
	reflectSetup *pRS;

	if( side == 0 )
		TexRender( target );

	int iPBufferAttributes[] = {
		WGL_CUBE_MAP_FACE_ARB, WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side,
		0, 0
	};

	wglSetPbufferAttribARB(renderTargets[ target ].hPBuffer, iPBufferAttributes);

	pRS = (reflectSetup *)renderTargets[ target ].extra;

	from = pRS->centerPos;

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(90.0f, 1.0f, 1.0f, 500.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	switch( WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side )
	{
		case WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:at = from;
												 at.x += 1.0f;
												 gluLookAt(from.x,from.y, from.z,at.x,at.y,at.z,0,1.0f, 0);
												 break;
		case WGL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:at = from;
												 at.x -= 1.0f;
												 gluLookAt(from.x,from.y, from.z,at.x,at.y,at.z,0,1.0f, 0);
												 break;
		case WGL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:at = from;
												 at.y += 1.0f;
												 gluLookAt(from.x,from.y, from.z,at.x,at.y,at.z,0,0,1.0f);
												 break;
		case WGL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:at = from;
												 at.y -= 1.0f;
												 gluLookAt(from.x,from.y, from.z,at.x,at.y,at.z,0,0,1.0f);
												 break;
		case WGL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:at = from;
												 at.z += 1.0f;
												 gluLookAt(from.x,from.y, from.z,at.x,at.y,at.z,0,1.0f, 0);
												 break;
		case WGL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:at = from;
												 at.z -= 1.0f;
												 gluLookAt(from.x,from.y, from.z,at.x,at.y,at.z,0,1.0f, 0);
												 break;
	}

/*	glProgramLocalParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, fpIndex, pRS->reflectColor.r, pRS->reflectColor.g,
									pRS->reflectColor.b, pRS->reflectColor.a );
*/
}

void TexMgr::RefreshAndRenew(int texnum)
{
	wglReleaseTexImageARB((HPBUFFERARB) renderTargets[texnum].hPBuffer, WGL_FRONT_LEFT_ARB);
	wglBindTexImageARB((HPBUFFERARB) renderTargets[texnum].hPBuffer, WGL_FRONT_LEFT_ARB);
}


const char *TexMgr::FindTexName( unsigned int texID )
{
	int k;
	static char notFound[]="NAME NOT FOUND";

	for( k = 0; k < textures.size(); k++ )
	{
		if( textures[k].texID = texID )
			return textures[k].texName.c_str();
	}

	return notFound;
}

GLuint TexMgr::SetupRender( char * texName, int xDim, int yDim, int flags)
{
	bool pbuff = (flags & TEXM_PBUFFER)?true:false;
	bool floatPix = (flags & TEXM_FLOAT_TYPE)?true:false;
	HDC tHdc = wglGetCurrentDC();
	HGLRC hglrc = wglGetCurrentContext();
	float fAttribs[] = { 0 };
	texRenderTarget tempRenderTarget;

/*T*/char msg[300];
	int pixelFormat;
	unsigned int numFormats;	

	tempRenderTarget.texName = texName;

	if( tHdc == NULL )
	{
		char errorMsg[128];
		sprintf(errorMsg, "%s(%d)TexMgr::SetupRender - wglGetCurrentDC() failed!!\n",__FILE__,__LINE__);
		LogFile(ERROR_LOG, errorMsg );
		return 0;
	}
	
	int curFormat = GetPixelFormat(tHdc);

	
	int attribsQuery[] = {
		WGL_SUPPORT_OPENGL_ARB,        //0
		WGL_DRAW_TO_PBUFFER_ARB,	   //1
		WGL_BIND_TO_TEXTURE_RGBA_ARB,
		WGL_PIXEL_TYPE_ARB,			   //3
		WGL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,              //5
		WGL_ALPHA_BITS_ARB,
		WGL_DEPTH_BITS_ARB,            //7
		WGL_STENCIL_BITS_ARB,          //8
	/*	WGL_TYPE_RGBA_FLOAT_ARB, */
		0,0};
	int results[ARRAYLEN(attribsQuery)];

	wglGetPixelFormatAttribivARB( tHdc, curFormat, 0, ARRAYLEN(attribsQuery), 
								  attribsQuery, results );
	
		char testMsg[300];
		sprintf(testMsg,"Current Pixel Format::OpenGL: %s  PBuff: %s  Bind2Tex: %s  PixType: %d Accel: %s  Color Bits: %d  Alp: %d  Depth: %d  Stencil:  %d\n",
			(results[0])?"Yes":"NO",(results[1])?"Yes":"NO",(results[2])?"Yes":"NO",results[3],(results[4])?"Yes":"NO",results[5],results[6],
			results[7],results[8]);
		LogFile( TEST_LOG, testMsg );
	

	if(pbuff)
	{
		/*
		int attribsSet[] = {
			WGL_SUPPORT_OPENGL_ARB,       true,
			WGL_DRAW_TO_PBUFFER_ARB,      true,
			WGL_COLOR_BITS_ARB,			  results[5], 
			WGL_ALPHA_BITS_ARB,           results[6], 
			WGL_DEPTH_BITS_ARB,           results[7],
			WGL_BIND_TO_TEXTURE_RGBA_ARB, true,
			0,0          //List terminated w/ zero
			};
		*/
		int attribsSet[] = { 
				WGL_DRAW_TO_PBUFFER_ARB,GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
				WGL_COLOR_BITS_ARB, (floatPix)?128:32,
				WGL_RED_BITS_ARB, (floatPix)?32:8,
				WGL_GREEN_BITS_ARB, (floatPix)?32:8,
				WGL_BLUE_BITS_ARB, (floatPix)?32:8,
				WGL_ALPHA_BITS_ARB, (floatPix)?32:8,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, (floatPix)?0:8,
				WGL_BIND_TO_TEXTURE_RGBA_ARB, true,
				WGL_PIXEL_TYPE_ARB, (floatPix)?WGL_TYPE_RGBA_FLOAT_ARB:WGL_TYPE_RGBA_ARB,
				0, 0 };

		if(!wglChoosePixelFormatARB(tHdc, attribsSet, fAttribs, 1, &pixelFormat, &numFormats))
		{
			char errorMsg[128];
			sprintf(errorMsg, "%s(%d)TexMgr::SetupRender - wglChoosePixelFormatARB() failed!!\n",__FILE__,__LINE__);
			LogFile(ERROR_LOG, errorMsg );
			checkGLerror(__LINE__,__FILE__,"TexMgr::SetupRender()");
			return 0;
		}
		if(numFormats == 0 )
		{
			char errorMsg[128];
			sprintf(errorMsg, "%s(%d)TexMgr::SetupRender - No formats supported!!\n",__FILE__,__LINE__);
			LogFile(ERROR_LOG, errorMsg );
			checkGLerror(__LINE__,__FILE__,"TexMgr::SetupRender()");
			return 0;
		}


		wglGetPixelFormatAttribivARB( tHdc, pixelFormat, 0, ARRAYLEN(attribsQuery), 
								  attribsQuery, results );
		{
			char testMsg[300];
			sprintf(testMsg,"PBUFF Format::OpenGL: %s  PBuff: %s  Bind2Tex: %s  PixType: %x Accel: %s  ColorBits: %d  Alp: %d  Depth: %d  Stencil:  %d  Float: %s\n",
				(results[0])?"Yes":"NO",(results[1])?"Yes":"NO",(results[2])?"Yes":"NO",results[3],(results[4])?"Yes":"NO",results[5],results[6],
				results[7],results[8],(results[9])?"YES":"No...");
			LogFile( TEST_LOG, testMsg );
		}

		int renderTexAttribs[]={ 
			WGL_TEXTURE_FORMAT_ARB,  WGL_TEXTURE_RGBA_ARB,
			WGL_TEXTURE_TARGET_ARB,  (flags & TEXM_CUBE_RENDER)?WGL_TEXTURE_CUBE_MAP_ARB:WGL_TEXTURE_2D_ARB,
			0,0  };


		tempRenderTarget.hPBuffer = wglCreatePbufferARB(tHdc, pixelFormat, xDim, yDim, renderTexAttribs);
		checkGLerror(__LINE__,__FILE__,"TexMgr::SetupRender() - after the CreatePbuffer");
		tempRenderTarget.hdc = wglGetPbufferDCARB( tempRenderTarget.hPBuffer );
		checkGLerror(__LINE__,__FILE__,"TexMgr::SetupRender()");
		tempRenderTarget.hglrc = wglCreateContext( tempRenderTarget.hdc );
		wglQueryPbufferARB(tempRenderTarget.hPBuffer, WGL_PBUFFER_WIDTH_ARB, &tempRenderTarget.w);
		wglQueryPbufferARB(tempRenderTarget.hPBuffer, WGL_PBUFFER_HEIGHT_ARB, &tempRenderTarget.h);
		wglShareLists(hglrc, tempRenderTarget.hglrc);

		sprintf(msg,"The values of the HDC and HGLRC are %d, %d\nThe dims are %d, %d\n The P-Buffer "
				"is %d\nThe type is %s",tempRenderTarget.hdc,tempRenderTarget.hglrc,tempRenderTarget.w,
				tempRenderTarget.h,tempRenderTarget.hPBuffer, (flags & TEXM_CUBE_RENDER)?"CUBE":"2D" );
LogFile( ERROR_LOG, msg );
	}
	else
	{
		tempRenderTarget.hPBuffer = 0;
		tempRenderTarget.hdc = 0;
		tempRenderTarget.hglrc = 0;
		tempRenderTarget.w = xDim;
		tempRenderTarget.h = yDim;
	}

	tempRenderTarget.flags = flags;
	//glEnable(GL_TEXTURE_2D);
	glGenTextures( 1, & tempRenderTarget.texID );
	glBindTexture((flags & TEXM_CUBE_RENDER)? GL_TEXTURE_CUBE_MAP_ARB : GL_TEXTURE_2D, tempRenderTarget.texID );

	if(!pbuff)
	{
		unsigned int *renderTex = NULL;

		renderTex = new unsigned int[xDim * yDim * 4 * sizeof(int)];
		
		if(!(flags & TEXM_CUBE_RENDER))
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 4, xDim, yDim, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderTex);
		}
		else
		{
			glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, 4, yDim, yDim, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderTex );
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, 4, xDim, yDim, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderTex );
			glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, 4, xDim, yDim, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderTex );
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, 4, xDim, yDim, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderTex );
			glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, 4, xDim, yDim, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderTex );
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, 4, xDim, yDim, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderTex );
		}
		delete [] renderTex;
	}

	glTexParameteri((flags & TEXM_CUBE_RENDER)? GL_TEXTURE_CUBE_MAP_ARB : GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri((flags & TEXM_CUBE_RENDER)? GL_TEXTURE_CUBE_MAP_ARB : GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri((flags & TEXM_CUBE_RENDER)? GL_TEXTURE_CUBE_MAP_ARB : GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (floatPix)?GL_NEAREST:GL_LINEAR);
	glTexParameteri((flags & TEXM_CUBE_RENDER)? GL_TEXTURE_CUBE_MAP_ARB : GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (floatPix)?GL_NEAREST:GL_LINEAR);

	renderTargets.push_back( tempRenderTarget );
	sprintf(msg,"The value of the textureID is %d\n",tempRenderTarget.texID);
	LogFile(ERROR_LOG,msg);
	

	return tempRenderTarget.texID;

}



int    TexMgr::BindRenderTex( char * texName, int texUnit )
{
	int index=0;

	while( index < renderTargets.size() )
	{
		if( strcmp( texName, renderTargets[index].texName.c_str() )==0)
			return BindRenderTex( index, texUnit );
		index++;
	}
	return -1;
}


int    TexMgr::BindRenderTex( int texIndex, int texUnit )
{
	GLenum texType = (renderTargets[texIndex].flags & TEXM_2D_TEX )? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_ARB;
	
	glActiveTextureARB(GL_TEXTURE0_ARB + texUnit );
	glBindTexture( texType, renderTargets[ texIndex ].texID );
	glEnable( texType );

	return texIndex;
}


int    TexMgr::GetRenderTex( char * texName )
{
	int index=0;

	while( index < renderTargets.size() )
	{

		if( strcmp( texName, renderTargets[index].texName.c_str() )==0)
			return index;
		index++;
	}
	return -1;
}


void TexMgr::SetRenderTargetDefault( int texNum )
{
	TexRender(texNum); 

	glViewport(0, 0, renderTargets[texNum].w, renderTargets[texNum].h);
	glCullFace(GL_FRONT);
	//Depth states
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	//glShadeModel(GL_FLAT);
	//	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	TexRender( -1 );
}

ShaderMgr::ShaderMgr()
{}


ShaderMgr::~ShaderMgr()
{
	int k;
	for( k = 0; k < vShaders.size(); k++)
		glDeleteProgramsARB( 1, & vShaders[k].shaderID );
	for( k = 0; k < fShaders.size(); k++)
		glDeleteProgramsARB( 1, & fShaders[k].shaderID );
}

GLuint ShaderMgr::CreateVertexShader(char * vertShader)
{
	string temp(vertShader);
	return CreateVertexShader(temp);
}

GLuint ShaderMgr::CreateFragmentShader(char * fragShader)
{
	string temp(fragShader);
	return CreateFragmentShader(temp);
}


GLuint ShaderMgr::CreateVertexShader(string &vertShader)
{
	int k, place = vertShader.find_last_of("/");
	string tempShader = vertShader.substr( (place < 0)?0:place );

	for( k = 0; k < vShaders.size(); k++)
	{
		if( vShaders[k].shaderName == tempShader )
			return vShaders[k].shaderID;
	}


	struct _stat buf;
	shaderHandle newShader;
	int res;
	FILE *shaderFile;
    char *buffer;
	char errorString[250];
	int errorPos;

    if((res = _stat( vertShader.c_str(), &buf )) != 0)
	{
		sprintf(errorString,"ShaderMgr::CreateVertexShader()::Couldn't get data about file %s.\n", vertShader.c_str());
		LogFile(ERROR_LOG, errorString);
		exit(1);
	}

	if((shaderFile = fopen(vertShader.c_str(), "r")) == NULL)
	{
		sprintf(errorString,"ShaderMgr::CreateVertexShader()::Couldn't open %s\n", vertShader.c_str());
		LogFile(ERROR_LOG, errorString);
		exit(1);
	}
   
	buffer = new char [buf.st_size];

    res = fread(buffer, sizeof(char),  buf.st_size, shaderFile);
	buffer[res]='\0';
	if(res != buf.st_size)
	{
		sprintf(errorString,"ShaderMgr::CreateVertexShader() - Didn't read enough bytes from %s: (%d / %d read)\n", vertShader.c_str(), res,buf.st_size);
		LogFile(ERROR_LOG, errorString);
	}

	fclose(shaderFile);

	glGenProgramsARB( 1, &newShader.shaderID);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, newShader.shaderID);
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, res, buffer);


	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	if(errorPos != -1)
	{
		sprintf(errorString,"ShaderMgr::CreateVertexShader()->'%s' Error at pos %d: %s\n", vertShader.c_str(),errorPos,
			glGetString(GL_PROGRAM_ERROR_STRING_ARB));
		LogFile(ERROR_LOG, errorString);
		exit(1);
	}
    glEnable(GL_VERTEX_PROGRAM_ARB);

	newShader.shaderName = tempShader;
	vShaders.push_back( newShader );
	return newShader.shaderID;	
}


GLuint ShaderMgr::CreateFragmentShader(string &fragShader)
{
	int k, place = fragShader.find_last_of("/");
	string tempShader = fragShader.substr( (place < 0)?0:place );

	for( k = 0; k < fShaders.size(); k++)
	{
		if( fShaders[k].shaderName == tempShader )
			return fShaders[k].shaderID;
	}


	struct _stat buf;
	shaderHandle newShader;
	int res;
	FILE *shaderFile;
    char *buffer;
	char errorString[250];
	int errorPos;

    if((res = _stat( fragShader.c_str(), &buf )) != 0)
	{
		sprintf(errorString,"ShaderMgr::CreateFragmentShader()::Couldn't get data about file %s.\n", fragShader.c_str());
		LogFile(ERROR_LOG, errorString);
		exit(1);
	}

	if((shaderFile = fopen(fragShader.c_str(), "r")) == NULL)
	{
		sprintf(errorString,"ShaderMgr::CreateFragmentShader()::Couldn't open %s\n", fragShader.c_str());
		LogFile(ERROR_LOG, errorString);
		exit(1);
	}
   
	buffer = new char [buf.st_size];

    res = fread(buffer, sizeof(char),  buf.st_size, shaderFile);
	buffer[res]='\0';
	if(res != buf.st_size)
	{
		sprintf(errorString,"ShaderMgr::CreateFragmentShader() - Didn't read enough bytes from %s: (%d / %d read)\n", fragShader.c_str(), res,buf.st_size);
		LogFile(ERROR_LOG, errorString);
	}

	fclose(shaderFile);

	glGenProgramsARB( 1, &newShader.shaderID);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, newShader.shaderID);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, res, buffer);


	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	if(errorPos != -1)
	{
		sprintf(errorString,"ShaderMgr::CreateVertexShader()->'%s' Error at pos %d: %s\n", fragShader.c_str(),errorPos,
			glGetString(GL_PROGRAM_ERROR_STRING_ARB));
		LogFile(ERROR_LOG, errorString);
		exit(1);
	}
    glEnable(GL_FRAGMENT_PROGRAM_ARB);

	newShader.shaderName = tempShader;
	fShaders.push_back( newShader );
	return newShader.shaderID;	
}



GLuint ShaderMgr::GetVertexShader(WORD format)
{
	char totalName[10];
	int  length = 0, k;

	if( format & MAT_TEX_DIFF ) totalName[ length++ ] = 'D';      //--   Potential Names for Vertex shaders:   --/
	if( format & MAT_TEX_BUMP ) totalName[ length++ ] = 'B';      //-- 'D', 'DB', 'DB_G', 'DBS_G', 'DBS_GS', 'DBS_S', 'DS_S'
	if( format & MAT_TEX_SPEC ) totalName[ length++ ] = 'S';      //-- 'DS', 'DBS', 'DBSH', 'DBSH_G', 'DBSH_GS', 'DBSH_S', 
	if( format & MAT_TEX_HEIGHT ) totalName[ length++ ] = 'H';    //-- QUEER NAMES - probably will never use: 
																  //--  'S', 'S_S', 'BS_S', 'BS_GS', 'BS_G', 'B_G', 'B', 
	if( length > 1)
		totalName[ length++ ] = '_';

	if( format & MAT_TEX_GLOSS ) totalName[ length++ ] = 'G';    
	if( format & MAT_TEX_S_EXP ) totalName[ length++ ] = 'S';
	
	totalName[ length++ ] = '.';
	totalName[ length++ ] = '\0';

	for( k = 0; k < vShaders.size(); k++)
	{
		if( strncmp(totalName, vShaders[k].shaderName.c_str(), length - 1) == 0 )
			return vShaders[k].shaderID;
	}
	return 0;
}



GLuint ShaderMgr::GetFragmentShader(WORD format)
{
	char totalName[14];
	int  length = 0, k;

	if( format & MAT_TEX_TRANSP )        //--   Possible Prefixes for fragment shaders      --/
	{	totalName[ length++ ] = 'T';     //--   'T_', 'T_E_', 'T_M_', 'E_', 'M_'
		totalName[ length++ ] = '_';  }                                 

	if( format & (MAT_TEX_D_ENV | MAT_ALT_ENV) )
	{	totalName[ length++ ] = 'E';    
	    totalName[ length++ ] = '_';   } 
	else if( format & (MAT_TEX_MIRROR | MAT_ALT_MIRROR) )
	{   totalName[ length++ ] = 'M';
	    totalName[ length++ ] = '_';   }


	if( format & MAT_TEX_DIFF ) totalName[ length++ ] = 'D';      //--   Potential Mid - Names for Fragment shaders:   --/
	if( format & MAT_TEX_BUMP ) totalName[ length++ ] = 'B';      //-- 'D', 'DB', 'DB_G', 'DBS_G', 'DBS_GS', 'DBS_S', 'DS_S'
	if( format & MAT_TEX_SPEC ) totalName[ length++ ] = 'S';      //-- 'DS', 'DBS', 'DBSH', 'DBSH_G', 'DBSH_GS', 'DBSH_S', 
	if( format & MAT_TEX_HEIGHT ) totalName[ length++ ] = 'H';    //-- QUEER NAMES - probably will never use: 
																  //--  'S', 'S_S', 'BS_S', 'BS_GS', 'BS_G', 'B_G', 'B', 
	totalName[ length++ ] = '_';
	
	if( format & MAT_TEX_GLOSS )  totalName[ length++ ] = 'G';
	if( format & MAT_TEX_S_EXP )  totalName[ length++ ] = 'S'; 
	
	if( totalName[ length - 1 ] == '_')
		--length;

	totalName[ length++ ] = '.';
	totalName[ length++ ] = '\0';

	for( k = 0; k < fShaders.size(); k++)
	{
		if( strncmp(totalName, fShaders[k].shaderName.c_str(), length - 1) == 0 )
			return fShaders[k].shaderID;
	}
	return 0;
}