#ifndef RENDEROBJS
#define RENDEROBJS

#define MAX_SHADER_PARAMS 24

#include <windows.h>
#include <gl\gl.h>
#include <gl\glext.h>
#include <gl\wglext.h>
#include <gl\glprocs.h>

class ShaderObj{
public:
	ShaderObj();
	ShaderObj(char *filename,GLenum type);
	~ShaderObj(){glDeleteProgramsARB( 1, &progID);}

	//-- Shader Use Stuff --/
	void SetLocalParam(int place,float X=0.0, float Y = 0.0, float Z = 0.0, float W = 0.0);
	void SetGlobalParam(int place,float X=0.0, float Y = 0.0, float Z = 0.0, float W = 0.0);
	void EnableShader();
	void DisableShader();
	void BindTextureUnit(int unit, GLenum texType, GLuint texID);

	//-- Shader Creation --/
	bool   CheckProgramExtensions();
	GLuint LoadProgram(char *filename,GLenum type);

	GLuint progID;
	GLenum progType;

	static GLint texUnits;
};



#endif

