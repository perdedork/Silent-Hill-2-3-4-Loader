# Microsoft Developer Studio Generated NMAKE File, Based on SH_initial_loader.dsp
!IF "$(CFG)" == ""
CFG=SH_initial_loader - Win32 Debug
!MESSAGE No configuration specified. Defaulting to SH_initial_loader - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "SH_initial_loader - Win32 Release" && "$(CFG)" != "SH_initial_loader - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SH_initial_loader.mak" CFG="SH_initial_loader - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SH_initial_loader - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SH_initial_loader - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\SH2-SH3_Viewer.exe"


CLEAN :
	-@erase "$(INTDIR)\Camera.obj"
	-@erase "$(INTDIR)\glprocs.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathOpts.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\quat.obj"
	-@erase "$(INTDIR)\Renderer.obj"
	-@erase "$(INTDIR)\RenderObjs.obj"
	-@erase "$(INTDIR)\RShadow.obj"
	-@erase "$(INTDIR)\RVertex.obj"
	-@erase "$(INTDIR)\SH2_Loader.obj"
	-@erase "$(INTDIR)\SH3_Loader.obj"
	-@erase "$(INTDIR)\SH_initial_loader.obj"
	-@erase "$(INTDIR)\SH_Model_Anim.obj"
	-@erase "$(INTDIR)\tgaload.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vertex.obj"
	-@erase "$(OUTDIR)\SH2-SH3_Viewer.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\OpenTest" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SH_initial_loader.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\SH2-SH3_Viewer.pdb" /machine:I386 /out:"$(OUTDIR)\SH2-SH3_Viewer.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Camera.obj" \
	"$(INTDIR)\glprocs.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\mathOpts.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\quat.obj" \
	"$(INTDIR)\Renderer.obj" \
	"$(INTDIR)\RenderObjs.obj" \
	"$(INTDIR)\RShadow.obj" \
	"$(INTDIR)\RVertex.obj" \
	"$(INTDIR)\SH2_Loader.obj" \
	"$(INTDIR)\SH3_Loader.obj" \
	"$(INTDIR)\SH_initial_loader.obj" \
	"$(INTDIR)\SH_Model_Anim.obj" \
	"$(INTDIR)\tgaload.obj" \
	"$(INTDIR)\vertex.obj"

"$(OUTDIR)\SH2-SH3_Viewer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\SH_initial_loader.exe" "$(OUTDIR)\SH_initial_loader.bsc"


CLEAN :
	-@erase "$(INTDIR)\Camera.obj"
	-@erase "$(INTDIR)\Camera.sbr"
	-@erase "$(INTDIR)\glprocs.obj"
	-@erase "$(INTDIR)\glprocs.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\mathOpts.obj"
	-@erase "$(INTDIR)\mathOpts.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\quat.obj"
	-@erase "$(INTDIR)\quat.sbr"
	-@erase "$(INTDIR)\Renderer.obj"
	-@erase "$(INTDIR)\Renderer.sbr"
	-@erase "$(INTDIR)\RenderObjs.obj"
	-@erase "$(INTDIR)\RenderObjs.sbr"
	-@erase "$(INTDIR)\RShadow.obj"
	-@erase "$(INTDIR)\RShadow.sbr"
	-@erase "$(INTDIR)\RVertex.obj"
	-@erase "$(INTDIR)\RVertex.sbr"
	-@erase "$(INTDIR)\SH2_Loader.obj"
	-@erase "$(INTDIR)\SH2_Loader.sbr"
	-@erase "$(INTDIR)\SH3_Loader.obj"
	-@erase "$(INTDIR)\SH3_Loader.sbr"
	-@erase "$(INTDIR)\SH_initial_loader.obj"
	-@erase "$(INTDIR)\SH_initial_loader.sbr"
	-@erase "$(INTDIR)\SH_Model_Anim.obj"
	-@erase "$(INTDIR)\SH_Model_Anim.sbr"
	-@erase "$(INTDIR)\tgaload.obj"
	-@erase "$(INTDIR)\tgaload.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vertex.obj"
	-@erase "$(INTDIR)\vertex.sbr"
	-@erase "$(OUTDIR)\SH_initial_loader.bsc"
	-@erase "$(OUTDIR)\SH_initial_loader.exe"
	-@erase "$(OUTDIR)\SH_initial_loader.ilk"
	-@erase "$(OUTDIR)\SH_initial_loader.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "H:\pro\OpenGLStuff\OpenTest" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "SILENT_HILL3" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SH_initial_loader.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Camera.sbr" \
	"$(INTDIR)\glprocs.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\mathOpts.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\quat.sbr" \
	"$(INTDIR)\Renderer.sbr" \
	"$(INTDIR)\RenderObjs.sbr" \
	"$(INTDIR)\RShadow.sbr" \
	"$(INTDIR)\RVertex.sbr" \
	"$(INTDIR)\SH2_Loader.sbr" \
	"$(INTDIR)\SH3_Loader.sbr" \
	"$(INTDIR)\SH_initial_loader.sbr" \
	"$(INTDIR)\SH_Model_Anim.sbr" \
	"$(INTDIR)\tgaload.sbr" \
	"$(INTDIR)\vertex.sbr"

"$(OUTDIR)\SH_initial_loader.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\SH_initial_loader.pdb" /debug /machine:I386 /out:"$(OUTDIR)\SH_initial_loader.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Camera.obj" \
	"$(INTDIR)\glprocs.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\mathOpts.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\quat.obj" \
	"$(INTDIR)\Renderer.obj" \
	"$(INTDIR)\RenderObjs.obj" \
	"$(INTDIR)\RShadow.obj" \
	"$(INTDIR)\RVertex.obj" \
	"$(INTDIR)\SH2_Loader.obj" \
	"$(INTDIR)\SH3_Loader.obj" \
	"$(INTDIR)\SH_initial_loader.obj" \
	"$(INTDIR)\SH_Model_Anim.obj" \
	"$(INTDIR)\tgaload.obj" \
	"$(INTDIR)\vertex.obj"

"$(OUTDIR)\SH_initial_loader.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("SH_initial_loader.dep")
!INCLUDE "SH_initial_loader.dep"
!ELSE 
!MESSAGE Warning: cannot find "SH_initial_loader.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "SH_initial_loader - Win32 Release" || "$(CFG)" == "SH_initial_loader - Win32 Debug"
SOURCE=..\..\OpenTest\Camera.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\Camera.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\Camera.obj"	"$(INTDIR)\Camera.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\glprocs.c

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\glprocs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\glprocs.obj"	"$(INTDIR)\glprocs.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\mathlib.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\mathlib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\mathlib.obj"	"$(INTDIR)\mathlib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\mathOpts.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\mathOpts.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\mathOpts.obj"	"$(INTDIR)\mathOpts.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\matrix.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\matrix.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\matrix.obj"	"$(INTDIR)\matrix.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\quat.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\quat.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\quat.obj"	"$(INTDIR)\quat.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\Renderer.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\Renderer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\Renderer.obj"	"$(INTDIR)\Renderer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\RenderObjs.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\RenderObjs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\RenderObjs.obj"	"$(INTDIR)\RenderObjs.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\RShadow.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\RShadow.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\RShadow.obj"	"$(INTDIR)\RShadow.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\RVertex.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\RVertex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\RVertex.obj"	"$(INTDIR)\RVertex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\SH2_Loader.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\SH2_Loader.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\SH2_Loader.obj"	"$(INTDIR)\SH2_Loader.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\SH3_Loader.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\SH3_Loader.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\SH3_Loader.obj"	"$(INTDIR)\SH3_Loader.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\SH_initial_loader.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\SH_initial_loader.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\SH_initial_loader.obj"	"$(INTDIR)\SH_initial_loader.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\SH_Model_Anim.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\SH_Model_Anim.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\SH_Model_Anim.obj"	"$(INTDIR)\SH_Model_Anim.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\OpenTest\tgaload.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\tgaload.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\tgaload.obj"	"$(INTDIR)\tgaload.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\OpenTest\vertex.cpp

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"


"$(INTDIR)\vertex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"


"$(INTDIR)\vertex.obj"	"$(INTDIR)\vertex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

