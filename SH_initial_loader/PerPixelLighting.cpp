#include <windows.h>
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
#include "PerPixelLighting.h"
#include "Renderer.h"


CubeMapNorm::CubeMapNorm()
{
	texID=0;
}

CubeMapNorm::~CubeMapNorm()
{
	glDeleteTextures(1,&texID);
}

//-------------------------------------------/
//-- CreateCubeMapNormalizer: the only     --/
//--     function that is actually needed  --/
//--     to be used in this class.  I'd    --/
//--     like to make it automatic, maybe  --/
//--     w/ a #define after some testing.. --/
//--     i.e. #define SIDE_DIM   256       --/
//--       CubeMapNorm(){size=SIDE_DIM;    --/
//--       CreateCubeMapNormalizer(size);} --/
//-------------------------------------------/
GLuint CubeMapNorm::CreateCubeMapNormalizer(int dim)
{
	GLuint cubemapID;
	glGenTextures(1, &cubemapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cubemapID);
	glEnable(GL_TEXTURE_CUBE_MAP_ARB);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
/*
	  glTexParameterf(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, 
                  GL_NEAREST);
*/				  

		MakeSide(dim, CUBE_X_POS);
		MakeSide(dim, CUBE_Y_POS);
		MakeSide(dim, CUBE_Z_POS);
		MakeSide(dim, CUBE_Y_NEG);
		MakeSide(dim, CUBE_X_NEG);
		MakeSide(dim, CUBE_Z_NEG);

  texID = cubemapID;
  //  LogFile(TEST_LOG,"Here Twat\n");
	return texID;
}


//-------------------------------------------/
//-- MakeSide: this is an aux function to  --/
//--    help in the creation of the cube-  --/
//--    map normalizer.  It is called by   --/
//--    'CreateCubeMapNormalizer'          --/
//-------------------------------------------/
void CubeMapNorm::MakeSide(int dim, int side)
{

	unsigned char *rgbBuffer=new unsigned char[dim * dim * 3];
//	float *rgbBuffer = new float [dim * dim * 3];
//	float iVal, jVal;
	float middle = 0.5f * (float)(dim - 1);
	int iDim, jDim, place=0;
	float dLess=((float)dim) - 1.0f;
	vertex temp;
	for(iDim=0; iDim < dim; iDim++)
	{
		//iVal = -1.0f + (((float)iDim + 0.5f) * 2.0f / (float)dim);

		for(jDim=0; jDim < dim; jDim++)
		{
			//jVal = -1.0f + (((float)jDim + 0.5f) * 2.0f / (float)dim);



			switch(side)
			{
			case CUBE_X_NEG: temp.x = -middle; temp.y = middle - iDim; temp.z = jDim - middle;//temp.x = -1.0f; temp.y = -iVal; temp.z = jVal;
				break;
			case CUBE_X_POS: temp.x =  middle; temp.y = middle - iDim; temp.z = middle - jDim;//1.0f; temp.y = -iVal; temp.z = -jVal;
				break;
			case CUBE_Y_NEG: temp.x = jDim - middle; temp.y = -middle; temp.z = middle - iDim;//jVal; temp.y = -1.0f; temp.z = -iVal;
				break;
			case CUBE_Y_POS: temp.x = jDim - middle; temp.y =  middle; temp.z = iDim - middle;//jVal; temp.y =  1.0f; temp.z = iVal;
				break;
			case CUBE_Z_NEG: temp.x = middle - jDim; temp.y = middle - iDim; temp.z = -middle;//-jVal; temp.y = -iVal; temp.z = -1.0f;
				break;
			case CUBE_Z_POS: temp.x = jDim - middle; temp.y = middle - iDim; temp.z =  middle;//jVal; temp.y = -iVal; temp.z =  1.0f;
				break;
			}
			VUnit(&temp);

		
				rgbBuffer[place++/*(iDim * dim + jDim) * 3*/ ]    = floatToByte(temp.x);//(temp.x+1.0f)/2.0f;//(unsigned char)(127.5f * (temp.x + 1.0f));//128 + (unsigned char)(127.0f * temp.x);/*PackFloatInByte(temp.x);*/
				rgbBuffer[place++/*(iDim * dim + jDim) * 3 + 1*/] = floatToByte(temp.y);//(temp.y+1.0f)/2.0f;//(unsigned char)(127.5f * (temp.y + 1.0f));//128 + (unsigned char)(127.0f * temp.y);/*PackFloatInByte(temp.y);*/
				rgbBuffer[place++/*(iDim * dim + jDim) * 3 + 2*/] = floatToByte(temp.z);//(temp.z+1.0f)/2.0f;//(unsigned char)(127.5f * (temp.z + 1.0f));//128 + (unsigned char)(127.0f * temp.z);/*PackFloatInByte(temp.z);*/
		}
	}

	switch(side)
	{
	case CUBE_X_NEG:glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_RGB8, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer);
		break;
	case CUBE_X_POS:glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_RGB8, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer);
		break;
	case CUBE_Y_NEG:glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_RGB8, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer);
		break;
	case CUBE_Y_POS:glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_RGB8, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer);
		break;
	case CUBE_Z_NEG:glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_RGB8, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer);
		break;
	case CUBE_Z_POS:glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_RGB8, dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer);
		break;
	}
	delete [] rgbBuffer;
}




BumpMapTex::BumpMapTex(){texID=0;}
BumpMapTex::~BumpMapTex(){glDeleteTextures(1,&texID);}

GLuint BumpMapTex::LoadBumpMap( char *filename )
{
	GLuint tempTexID;

	if((tempTexID=tgaLoadAndBind(filename,TGA_FREE | TGA_NO_MIPMAPS))==0)
	{
		texID=0;
		return 0;
	}
	texID=tempTexID;
	return texID;
}


bool   BumpMapTex::CheckBumpExtentions()
{

	if(!IsExtensionSupported("GL_ARB_multitexture"))
		return false;
	if(!IsExtensionSupported("GL_ARB_texture_cube_map"))
	{
		if(!IsExtensionSupported("GL_EXT_texture_cube_map"))
			return false;
	}
	if(!IsExtensionSupported("GL_ARB_texture_env_combine"))
		return false;
	if(!IsExtensionSupported("GL_ARB_texture_env_dot3"))
		return false;

	return true;
}

void   BumpMapTex::AddRenderStage( CubeMapNorm & cMap )
{
}




//--==<[ M A T E R I A L   F I L E   F O R M A T  ]>==--/
//--                                                  --/
//--Everything in "" are text, and things in < >'s are--/
//--parameters, but don't put in the < >'s.  Needed   --/
//--lines start with a ':', and optional lines start  --/
//--with a '['. Neither symbol is needed in the actual--/
//--material.  Surfaces have different colors, so mat --/
//--erials don't have any color specified.  As new    --/
//--types of material properties are added the format --/
//--will change.                                      --/
//-------------==<[ V E R S I O N   01 ]>==-------------/
//--:"MAT_V< 2 digit number for version >"            --/
//--:"<internal material name>"                       --/
//--["DIFFUSE <diffuse texture path and name>"        --/ although the textures are listed as "optional," there must be
//--["BUMP <bump map path and name>"                  --/ a diffuse, and if not, the mirror, or full environment flag must be set
//--["SPECULAR <specular color map path and name>"    --/
//--["HEIGHT <height map path and name>"              --/
//--["scale_bias <scale value> <bias value>"          --/
//--:"flags <number = flag0 | flag1 | ... | flagN >"  --/
//--["vspecial <special vertex shader path and name>" --/
//--["fspecial <special frag shader path and name>"   --/


Material::Material(){}
Material::~Material(){}

bool Material::LoadMaterial(char *material, TexMgr *tPtr, ShaderMgr *sPtr)
{
	int version;
	std::ifstream matFile;
	std::string buffer;

	for( int k = 0; k < TEX_COUNT; k++ )        //Set all of the texture object IDs to 0
		textures[k] = 0;
	vShader = fShader = 0;                      //Set the vertex and fragment program IDs to 0
	parallaxScale = parallaxBias = 0.0f;        //Set the scale and bias for parallax mapping


	matFile.open(material);
	if(matFile.fail())
	{
		char mess[200];
		sprintf(mess,"ERROR: PerPixelLighting{Material::LoadMaterial} - Unable to open material '%s' -Ignored\n",material);
		LogFile(ERROR_LOG,mess);
		return false;
	}

	matFile >> buffer;
	if(strncmp(buffer.c_str(),"MAT_V",5) != 0 )
	{
		char mess[200];
		sprintf(mess,"ERROR: PerPixelLighting{Material::LoadMaterial} - Invalid material identifier '%s' -Ignored\n",material);
		LogFile(ERROR_LOG,mess);
		matFile.close();
		return false;
	}
	version = atoi((buffer.substr(5)).c_str());  //Get Material File Version
	matFile >> materialName;                     //Read in next line of file
	
	do{
		string temp;
		matFile >> buffer;

		if( buffer == "DIFFUSE" )
		{
			matFile >> temp;
			textures[TEX_DIFF] = tPtr->GetTexture(temp);
		}
		else if( buffer == "BUMP")
		{
			matFile >> temp;
			textures[TEX_BUMP] = tPtr->GetTexture(temp);
		}
		else if( buffer == "SPECULAR")
		{
			matFile >> temp;
			textures[TEX_SPEC] = tPtr->GetTexture(temp);
		}	
		else if( buffer == "HEIGHT")
		{
			matFile >> temp;
			textures[TEX_HEIGHT] = tPtr->GetTexture(temp);
		}	
		else if( buffer == "scale_bias")
		{
			matFile >> parallaxScale >> parallaxBias;
		}	
	}
	while( buffer != "Flags" && !matFile.eof() );

	if( buffer == "Flags")
		matFile >> materialFormat;
	else
	{
		int j, fSet = 0x0001;
		char mess[200];
		sprintf(mess,"ERROR: PerPixelLighting{Material::LoadMaterial} - Missing required field 'Flags <#>' in '%s', will try to Set \n",material);
		LogFile(ERROR_LOG,mess);
		
		materialFormat = 0;

		for( j = 0; j < TEX_COUNT; j++)
		{
			if( textures[j] != 0 )
				materialFormat |= fSet;
			fSet <<= 4;
		}
	}

	if( materialFormat & MAT_ALT_ENV )
		textures[TEX_DIFF] = tPtr->GetEnvironmentMap();
	else if( materialFormat & MAT_ALT_MIRROR )
		textures[TEX_DIFF] = tPtr->GetReflectionMap();
	else if( materialFormat & MAT_TEX_D_ENV )
		textures[TEX_SPECIAL] = tPtr->GetEnvironmentMap();
	else if( materialFormat & MAT_TEX_MIRROR )
		textures[TEX_SPECIAL] = tPtr->GetReflectionMap();

	if( materialFormat & MAT_SPECIAL )
	{
		string temp;
		matFile >> buffer;

		if( buffer == "vspecial")
		{
			matFile >> temp;
			matFile >> buffer;

			vShader = sPtr->CreateVertexShader(temp);

			if( buffer == "fspecial")
			{
				matFile >> temp;
				fShader = sPtr->CreateFragmentShader(temp);
			}
		}
		else if( buffer == "fspecial" )
		{
			matFile >> temp;
			matFile >> buffer;

			fShader = sPtr->CreateFragmentShader(temp);

			if( buffer == "vspecial" )
			{
				matFile >> temp;
				vShader = sPtr->CreateVertexShader(temp);
			}
		}
		else
		{
			char mess[200];
			sprintf(mess,"ERROR: PerPixelLighting{Material::LoadMaterial} - Flags set for special shader in '%s', none specified\n",material);
			LogFile(ERROR_LOG,mess);
		}
	}
	else
	{
		vShader = sPtr->GetVertexShader(materialFormat);
		fShader = sPtr->GetFragmentShader(materialFormat);
	}
	//-- PUT ADDITIONAL STUFF FOR VERSIONS ABOVE 01 HERE: --/

	matFile.close();
	return true;
}	


WORD Material::ApplyMat()
{

	glEnable( GL_VERTEX_PROGRAM_ARB );
	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, vShader );

	glEnable( GL_FRAGMENT_PROGRAM_ARB );
	glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, fShader );

	if(materialFormat & MAT_TEX_DIFF)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB);
		if( materialFormat & MAT_ALT_ENV )
		{
			glEnable (GL_TEXTURE_CUBE_MAP_ARB);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARB,textures[TEX_DIFF]);
		}
		else
		{
			glEnable (GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,textures[TEX_DIFF]);
		}
	}
	if(materialFormat & MAT_TEX_BUMP)
	{
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable (GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,textures[TEX_BUMP]);
	}
	if(materialFormat & MAT_TEX_SPEC)
	{
		glActiveTextureARB(GL_TEXTURE3_ARB);
		glEnable (GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,textures[TEX_SPEC]);
	}
	if(materialFormat & MAT_TEX_HEIGHT)
	{
		glActiveTextureARB(GL_TEXTURE2_ARB);
		glEnable (GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,textures[TEX_HEIGHT]);
		glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, parallaxScale, parallaxBias, 0.0f,1.0f);
	}
	if(materialFormat & MAT_TEX_D_ENV)
	{
		glActiveTextureARB(GL_TEXTURE4_ARB);
		glEnable (GL_TEXTURE_CUBE_MAP_ARB);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB,textures[TEX_SPECIAL]);
	}
	else if(materialFormat & MAT_TEX_MIRROR)
	{
		glActiveTextureARB(GL_TEXTURE4_ARB);
		glEnable (GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,textures[TEX_SPECIAL]);
	}

	return materialFormat & 0x9111;
}

  