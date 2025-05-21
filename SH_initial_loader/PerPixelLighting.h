#ifndef PERPIXELLIGHTING_H_
#define PERPIXELLIGHTING_H_



#include <windows.h>
#include <cstring>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include <stdio.h>

#include "mathOpts.h"
#include "vertex.h"
#include "quat.h"
#include "matrix.h"
#include "typedefs.h"
#include "mathlib.h"
#include "Camera.h"
#include "tgaload.h"
#include "plane.h"

using namespace std;


//-------------------------------------------------/
//-   CubeMap Normalizer - for use in per-pixel   -/
//-    lighting calculations: light vector tex    -/
//-    lookups, bump-mapping in texture space,    -/
//-                               
//-------------------------------------------------/

#define CUBEMAP_TEX_SIZE 256
#define PackFloatInByte(in)  (GLubyte) ((((in)+1.0f) / 2.0f) * 255.0f)

enum CubeMapSides{CUBE_X_NEG = 0, CUBE_X_POS = 1, CUBE_Y_NEG = 2, CUBE_Y_POS= 3, CUBE_Z_NEG= 4, CUBE_Z_POS= 5};

class CubeMapNorm{
public:
	CubeMapNorm();
	~CubeMapNorm();

	GLuint CreateCubeMapNormalizer(int dim);
	void MakeSide(int dim, int side);
	unsigned char floatToByte(float n){return (unsigned char)(127.5f * (n + 1.0f));}

	GLuint texID;
	int size;
};


//-- Texture types --/
enum {TEX_DIFF = 0, TEX_BUMP = 1, TEX_SPEC = 2, TEX_HEIGHT = 3, TEX_SPECIAL = 4, TEX_COUNT};

//-- Material & Texture Formats --/
//             NOTE: These are used to set the appropriate shaders for rendering, and setting & releasing state
//Primary Texture Flags
#define MAT_TEX_DIFF    0x0001        //Diffuse Texture Flag
#define MAT_TEX_BUMP    0x0010        //Bump Map texture Flag
#define MAT_TEX_SPEC    0x0100        //Specular Color Texture Flag
#define MAT_TEX_HEIGHT  0x1000        //Height Map Texture

//Additions & Changes
#define MAT_TEX_GLOSS   0x0002        //Alpha channel of bump map is a gloss map
#define MAT_TEX_S_EXP   0x0020        //Alpha channel of specular texture is specular exponent
#define MAT_TEX_TRANSP  0x0200        //Diffuse Alpha channel is transparency

//Different rendered surfaces
#define MAT_TEX_D_ENV   0x2000        //Texture is combo of diffuse texture and environment map, modulated by alpha channel
#define MAT_ALT_ENV     0x0004        //Entire diffuse is an environment map, w/ alpha blending (ie color showing through = alpha 
#define MAT_TEX_MIRROR  0x0040        //combo of diffuse texture and mirrored env, along the slope of the plane, which is projected into screen space
#define MAT_ALT_MIRROR  0x0400        //Texture mirrors, which is based upon the normal of surface

#define MAT_SPECIAL     0x8000        //Surface has special shader.  In material file, flag is specifyied, along w/ <path>\<name> of shader

class TexMgr;
class ShaderMgr;

class Material{
public:
	Material();
	~Material();

	bool LoadMaterial(char *material,TexMgr *tPtr,  //Gets the textures, as well as sets data for shaders and format
						ShaderMgr *sPtr);           //and the scale and bias if parallax mapping is in this material
	WORD ApplyMat();                                //Applys the current properties of material for rendering,returns format so renderMgr can disable appropriate states.

	string materialName;
	GLuint vShader, fShader;
	WORD   materialFormat;
	GLuint textures[TEX_COUNT];
	float  parallaxScale, parallaxBias;             //Scale and bias for parallax mapping
};

class BumpMapTex{
public:
	BumpMapTex();
	~BumpMapTex();

	GLuint LoadBumpMap( char *filename );
	void   SetExternalBumpMap( GLuint id ){ texID = id; }
	bool   CheckBumpExtentions();          //Checks to see if the hardware can support dot3 tex env, and combiner
	void   AddRenderStage( CubeMapNorm & cMap );   //This function inserts the steps of the multitex env for bump mapping


	GLuint texID;

};

#endif