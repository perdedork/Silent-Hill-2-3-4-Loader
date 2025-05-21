#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>
#include <string>
#include <vector>
#include <gl\gl.h>

#include "vertex.h"
#include "plane.h"
#include "IBaseTypes.h"

using namespace std;



//-- Flags for TexRender --/
#define TEXM_CUBE_RENDER      0x0001
#define TEXM_2D_RENDER        0x0002
#define TEXM_16_BPP           0x0010
#define TEXM_24_BPP           0x0020
#define TEXM_32_BPP           0x0040
#define TEXM_DEPTH_8          0x0100
#define TEXM_DEPTH_16         0x0200
#define TEXM_DEPTH_24         0x0400
#define TEXM_FLOAT_TYPE       0x0800

#define TEXM_RES_128          0x0004
#define TEXM_RES_256          0x0008
#define TEXM_RES_512          0x0080
#define TEXM_RES_1024         0x4000

#define TEXM_HAS_RES		(TEXM_RES_128|TEXM_RES_256|TEXM_RES_512|TEXM_RES_1024)

#define TEXM_PBUFFER		  0x8000

//-- Flags for AddTex --/
#define TEXM_2D_TEX           0x1000
#define TEXM_CUBEMAP          0x2000

//-- Flags for void* variable in texRenderTarget --/
#define TEXM_REFLECT_COLOR	  0x00010000		//Color to represent reflectable area ( from reflect /env map
#define TEXM_ALPHA_COLOR	  0x00020000		//Color for semi transparent color (based on alpha value )
#define TEXM_NULL			  0x80000000		//NOTHING!!!

class texHandle
{
public:
	texHandle( ){ }
	texHandle( const texHandle & rhs ){ operator=( rhs ); }
	~texHandle( ){ }
	texHandle & operator=( const texHandle & rhs ){ if( &rhs != this ){ texName = rhs.texName; texID = rhs.texID; flags = rhs.flags; }return *this; }

	string texName;
	GLuint texID;
	int flags;
};


class TexMgr{
public:
	TexMgr();
	~TexMgr();

	//-- Regular Texture Functions --/
	GLuint GetTexture( string & texName );				//First tries to find the named texture, and if it can't, then it makes it
	GLuint GetEnvironmentMap();							//Returns the
	GLuint GetReflectionMap();
	GLuint SetReflectionMap(plane *reflectPlane,		//Returns the tex object handle for surface used for reflections
		                    string & reflectTex);
	int	   CreateCubeReflectMap( char *rName, rgbaf *relfectColor,	//Takes a texture name, a color to fill w/ reflected images for shaders, center pos of cube, and flags
									vertex * pos, int _flags );		//Generates a PBuffer, or render textures CUBE_MAP
	GLuint SetEnvironmentMap(vertex *location,			//This sets the parameters for the environment map.  There are different ones for
							 int res, string & envTex);	//Different env mapped objects/surfaces, if desired.
	void   AddTex( string & texName, GLuint id,			//Add a texture loaded outside of texMgr, with
				  int flags);                
	int    BindTex( char * texName, int texUnit );		//Bind texture to texunit, returns index #, -1 for failure
    int    BindTex( int texIndex, int texUnit );		//Bind texture[texIndex] to texUnit, returns index #, -1 for failure

	//-- Rendering to a Texture Functions --/
	void   TexRender( int target );						//Render to target, 0 for framebuffer
	void   CubeRender( int target, int side);			//Render to 'side' of cubemap[target]
	GLuint SetupRender( char * texName, int xDim, int yDim, int flags);  //Setup the render target
	int    BindRenderTex( char * texName, int texUnit );//Bind a render texture to a texture unit, returns index # or -1 for failure
	int    BindRenderTex( int texIndex, int texUnit );	//Bind render texture[texIndex] to texUnit, returns index #, -1 for failure
	int    GetRenderTex( char * texName );				//Returns the # of the index of texName, -1 for failure
	int    TestLost(int index);
	void   SetRenderTargetDefault( int texNum );

   //-- Deleting by Name of texID --/
	int DeleteTex( const char * texName );				//Deletes texture by name   returns ok/error
	int DeleteTex( GLuint texID );						//Deletes texture by texID, returns ok/error
	int DeleteTex( int index );							//Deletes textures[k], returns ok/error
	int DeleteRenderTarget( int index );				//Deletes renderTargets[k], returns ok/error
	void DeleteAllTextures( );							//Deletes all textures, and cleans up vectors;
	void RefreshAndRenew(int texnum);
	void checkResident( );
	const char *FindTexName( unsigned int texID );

	
	vector<texHandle>        textures;
	vector<texRenderTarget>  renderTargets;
	HGLRC lastRC;
	HDC   lastDC;
};


class ShaderMgr{
public:
	ShaderMgr();
	~ShaderMgr();

	GLuint CreateVertexShader(string & vertShader);
	GLuint CreateFragmentShader(string & fragShader);
	GLuint CreateVertexShader(char * vertShader);
	GLuint CreateFragmentShader(char * fragShader);
	GLuint GetVertexShader(WORD format);
	GLuint GetFragmentShader(WORD format);

	struct shaderHandle{
		string shaderName;
		GLuint shaderID;
	};

	vector<shaderHandle> vShaders, fShaders;
};

typedef struct{
	int polygonSentCnt;
	float fps;           //Frames per second, based on last frame
	float timeToRender;  //Time to do rendering only - from first rendering command, to last, but no CPU (no AI, physics, ect)
}FrameDat;


class RenderManager{
public:
	RenderManager();
	~RenderManager();

	bool renderFrame(FrameDat &perFrameInfo);

};



class RenderStage{
public:
	RenderStage();
	~RenderStage();

	void render();

int material; 
};

#endif