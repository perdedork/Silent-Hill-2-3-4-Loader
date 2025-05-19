# Microsoft Developer Studio Project File - Name="SH_initial_loader" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SH_initial_loader - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SH_initial_loader.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SH_initial_loader - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /Ot /Oi /Ob2 /I "..\..\OpenTest" /I "C:\Documents and Settings\Mike\Desktop\zlib123" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib zlib.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"msvcrtd.lib" /out:"Release/SH2-SH3_Viewer.exe" /libpath:"C:\Documents and Settings\Mike\Desktop\zlib123\projects\visualc6\Win32_LIB_Debug"

!ELSEIF  "$(CFG)" == "SH_initial_loader - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /Gi /GX /ZI /Od /I "..\..\OpenTest" /I "C:\Documents and Settings\Mike\Desktop\zlib123" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "SILENT_HILL3" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib zlib.lib /nologo /subsystem:windows /profile /debug /machine:I386 /libpath:"C:\Documents and Settings\Mike\Desktop\zlib123\projects\visualc6\Win32_LIB_Debug"

!ENDIF 

# Begin Target

# Name "SH_initial_loader - Win32 Release"
# Name "SH_initial_loader - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\OpenTest\Camera.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\glprocs.c
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\mathlib.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\mathOpts.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\OBJ_Exporter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\quat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\Renderer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\RenderObjs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\RShadow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\RVertex.cpp
# End Source File
# Begin Source File

SOURCE=.\SH2_Loader.cpp
# End Source File
# Begin Source File

SOURCE=.\SH2_Model.cpp
# End Source File
# Begin Source File

SOURCE=.\SH3_ArcFilenames.cpp
# End Source File
# Begin Source File

SOURCE=.\SH3_Loader.cpp
# End Source File
# Begin Source File

SOURCE=.\SH4_Loader.cpp
# End Source File
# Begin Source File

SOURCE=.\SH_Collision.cpp
# End Source File
# Begin Source File

SOURCE=.\SH_initial_loader.cpp
# End Source File
# Begin Source File

SOURCE=.\SH_Model_Anim.cpp
# End Source File
# Begin Source File

SOURCE=.\SH_Msg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\tgaload.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\vertex.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\OpenTest\Camera.h
# End Source File
# Begin Source File

SOURCE=.\debugDrawing.h
# End Source File
# Begin Source File

SOURCE=.\fontSystem.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\IBaseTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\mathlib.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\mathOpts.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\matrix.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\OBJ_Exporter.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\PerPixelLighting.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\quat.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\Renderer.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\RenderObjs.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\RVertex.h
# End Source File
# Begin Source File

SOURCE=.\SH2_Loader.h
# End Source File
# Begin Source File

SOURCE=.\SH2_Model.h
# End Source File
# Begin Source File

SOURCE=.\SH3_ArcFilenames.h
# End Source File
# Begin Source File

SOURCE=.\SH3_Loader.h
# End Source File
# Begin Source File

SOURCE=.\SH3_Mod.h
# End Source File
# Begin Source File

SOURCE=.\SH4_Loader.h
# End Source File
# Begin Source File

SOURCE=.\SH_Collision.h
# End Source File
# Begin Source File

SOURCE=.\SH_Model_Anim.h
# End Source File
# Begin Source File

SOURCE=.\SH_Msg.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\tgaload.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=..\..\OpenTest\vertex.h
# End Source File
# Begin Source File

SOURCE=.\winutil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\fontTest.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
