#include "glprocs.h"

static PROC GetGLProcAddress(const char *name)
{
	PROC proc = wglGetProcAddress(name);
	if (proc == NULL || proc == (PROC)0x1 || proc == (PROC)0x2 || proc == (PROC)0x3 || proc == (PROC)-1)
	{
		HMODULE module = GetModuleHandleA("opengl32.dll");
		if (module != NULL)
			proc = GetProcAddress(module, name);
	}
	return proc;
}

PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;
PFNGLGENPROGRAMSARBPROC glGenProgramsARB = NULL;
PFNGLBINDPROGRAMARBPROC glBindProgramARB = NULL;
PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB = NULL;
PFNGLPROGRAMSTRINGARBPROC glProgramStringARB = NULL;
PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC glGetCompressedTexImageARB = NULL;

PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB = NULL;
PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB = NULL;
PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB = NULL;
PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB = NULL;
PFNWGLQUERYPBUFFERARBPROC wglQueryPbufferARB = NULL;
PFNWGLBINDTEXIMAGEARBPROC wglBindTexImageARB = NULL;
PFNWGLRELEASETEXIMAGEARBPROC wglReleaseTexImageARB = NULL;
PFNWGLSETPBUFFERATTRIBARBPROC wglSetPbufferAttribARB = NULL;

int InitGLProcs(void)
{
	int ok = 1;

	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)GetGLProcAddress("glActiveTextureARB");
	if (!glActiveTextureARB) ok = 0;
	glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)GetGLProcAddress("glClientActiveTextureARB");
	if (!glClientActiveTextureARB) ok = 0;
	glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)GetGLProcAddress("glGenProgramsARB");
	if (!glGenProgramsARB) ok = 0;
	glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)GetGLProcAddress("glBindProgramARB");
	if (!glBindProgramARB) ok = 0;
	glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)GetGLProcAddress("glDeleteProgramsARB");
	if (!glDeleteProgramsARB) ok = 0;
	glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)GetGLProcAddress("glProgramStringARB");
	if (!glProgramStringARB) ok = 0;
	glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)GetGLProcAddress("glProgramEnvParameter4fARB");
	if (!glProgramEnvParameter4fARB) ok = 0;
	glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)GetGLProcAddress("glProgramLocalParameter4fARB");
	if (!glProgramLocalParameter4fARB) ok = 0;
	glCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)GetGLProcAddress("glCompressedTexImage2DARB");
	if (!glCompressedTexImage2DARB) ok = 0;
	glGetCompressedTexImageARB = (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)GetGLProcAddress("glGetCompressedTexImageARB");
	if (!glGetCompressedTexImageARB) ok = 0;

	wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)GetGLProcAddress("wglGetExtensionsStringARB");
	if (!wglGetExtensionsStringARB) ok = 0;
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)GetGLProcAddress("wglChoosePixelFormatARB");
	if (!wglChoosePixelFormatARB) ok = 0;
	wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)GetGLProcAddress("wglGetPixelFormatAttribivARB");
	if (!wglGetPixelFormatAttribivARB) ok = 0;
	wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)GetGLProcAddress("wglCreatePbufferARB");
	if (!wglCreatePbufferARB) ok = 0;
	wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)GetGLProcAddress("wglGetPbufferDCARB");
	if (!wglGetPbufferDCARB) ok = 0;
	wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)GetGLProcAddress("wglReleasePbufferDCARB");
	if (!wglReleasePbufferDCARB) ok = 0;
	wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)GetGLProcAddress("wglDestroyPbufferARB");
	if (!wglDestroyPbufferARB) ok = 0;
	wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)GetGLProcAddress("wglQueryPbufferARB");
	if (!wglQueryPbufferARB) ok = 0;
	wglBindTexImageARB = (PFNWGLBINDTEXIMAGEARBPROC)GetGLProcAddress("wglBindTexImageARB");
	if (!wglBindTexImageARB) ok = 0;
	wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC)GetGLProcAddress("wglReleaseTexImageARB");
	if (!wglReleaseTexImageARB) ok = 0;
	wglSetPbufferAttribARB = (PFNWGLSETPBUFFERATTRIBARBPROC)GetGLProcAddress("wglSetPbufferAttribARB");
	if (!wglSetPbufferAttribARB) ok = 0;

	return ok;
}
