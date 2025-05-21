
#include <windows.h>
#include <gl\gl.h>
#include <gl\glext.h>
#include <gl\wglext.h>
#include <gl\glprocs.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>

#include "typedefs.h"
#include "RenderObjs.h"


GLint ShaderObj::texUnits = -1;



ShaderObj::ShaderObj()
{
	progID = -1;
	progType = GL_VERTEX_PROGRAM_ARB;
}



ShaderObj::ShaderObj(char *filename,GLenum type)
{
	progType = type;
	progID = -1;

	progID = LoadProgram(filename,type);
}



void ShaderObj::SetLocalParam(int place,float X, float Y, float Z, float W)
{
	glProgramLocalParameter4fARB( progType, place, (GLfloat) X, (GLfloat) Y, (GLfloat) Z, (GLfloat) W);
}



void ShaderObj::SetGlobalParam(int place, float X, float Y, float Z, float W)
{
	glProgramEnvParameter4fARB( progType, place, (GLfloat) X, (GLfloat) Y, (GLfloat) Z, (GLfloat) W);
}



void ShaderObj::EnableShader()
{
	glEnable( progType );
	glBindProgramARB( progType, progID);
}



void ShaderObj::DisableShader()
{
	glDisable( progType );
}



void ShaderObj::BindTextureUnit(int unit, GLenum texType, GLuint texID)
{

	if(texUnits == -1)
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &texUnits);

	if( unit < 0 || unit >= texUnits)
		return;

    glActiveTextureARB(GL_TEXTURE0_ARB + unit);
	glEnable( texType );
	glBindTexture( texType, texID);
}




bool   ShaderObj::CheckProgramExtensions()   //NOTE: also checks multitexture, for use in frag prog
{

	if(!IsExtensionSupported("GL_ARB_fragment_program"))
		return false;
	if(!IsExtensionSupported("GL_ARB_vertex_program"))
		return false;
	if(!IsExtensionSupported("GL_ARB_multitexture"))
	{
		LogFile(ERROR_LOG, "ERROR: Implementation doesn't support multi-texture.  Need to use multi-pass rendering!\n");
	}
	return true;
}


GLuint ShaderObj::LoadProgram(char *filename,GLenum type)
{
	struct _stat buf;
	int res;
	FILE *shaderFile;
    char *buffer;
	char errorString[250];
	int errorPos;

    if((res = _stat( filename, &buf )) != 0)
	{
		sprintf(errorString,"ShaderObj::Couldn't get data about file %s.\n", filename);
		LogFile(ERROR_LOG, errorString);
		exit(1);
	}

	if((shaderFile = fopen(filename, "r")) == NULL)
	{
		sprintf(errorString,"ShaderObj::Couldn't open %s\n", filename);
		LogFile(ERROR_LOG, errorString);
		exit(1);
	}
   
	buffer = new char [buf.st_size];

    res = fread(buffer, sizeof(char),  buf.st_size, shaderFile);
	buffer[res]='\0';
	if(res != buf.st_size)
	{
		sprintf(errorString,"ShaderObj::Didn't read enough bytes from %s: (%d / %d read)\n", filename, res,buf.st_size);
		LogFile(ERROR_LOG, errorString);
	//	LogFile(ERROR_LOG, buffer);
	}

	fclose(shaderFile);

	glGenProgramsARB( 1, &progID);
	glBindProgramARB(type, progID);
	glProgramStringARB(type, GL_PROGRAM_FORMAT_ASCII_ARB, res, buffer);


	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	if(errorPos != -1)
	{
		sprintf(errorString,"ShaderObj::LoadProgram ->'%s' Error at pos %d: %s\n", filename,errorPos,
			glGetString(GL_PROGRAM_ERROR_STRING_ARB));
		LogFile(ERROR_LOG, errorString);
		exit(1);
	}
    glEnable(type);
	progType = type;
	return progID;	
}
