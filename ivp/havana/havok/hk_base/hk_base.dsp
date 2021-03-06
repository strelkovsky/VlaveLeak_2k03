# Microsoft Developer Studio Project File - Name="hk_base" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=hk_base - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "hk_base.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hk_base.mak" CFG="hk_base - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hk_base - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "hk_base - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "hk_base - Win32 SDK SingleThreaded" (based on "Win32 (x86) Static Library")
!MESSAGE "hk_base - Win32 SDK MultiThreaded" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "hk_base"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "hk_base - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../hk_library/win32/release"
# PROP Intermediate_Dir "../../hk_tmp/win32/hk_base/release"
# PROP Target_Dir ""
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /Zi /O2 /I "../" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"hk_base/base.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Custom Build
TargetPath=\tf2\src\ivp\havana\hk_library\win32\release\hk_base.lib
InputPath=\tf2\src\ivp\havana\hk_library\win32\release\hk_base.lib
SOURCE="$(InputPath)"

"..\..\..\..\lib\common\hk_base.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	if exist ..\..\..\..\lib\common\hk_base.lib attrib -r ..\..\..\..\lib\common\hk_base.lib 
	copy $(TargetPath) ..\..\..\..\lib\common\hk_base.lib 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "hk_base - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../hk_library/win32/debug"
# PROP Intermediate_Dir "../../hk_tmp/win32/hk_base/debug"
# PROP Target_Dir ""
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /W3 /Gm /ZI /Od /I "../" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"hk_base/base.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Custom Build
TargetPath=\tf2\src\ivp\havana\hk_library\win32\debug\hk_base.lib
InputPath=\tf2\src\ivp\havana\hk_library\win32\debug\hk_base.lib
SOURCE="$(InputPath)"

"..\..\..\..\lib\common\hk_base.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	if exist ..\..\..\..\lib\common\hk_base.lib attrib -r ..\..\..\..\lib\common\hk_base.lib 
	copy $(TargetPath) ..\..\..\..\lib\common\hk_base.lib 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "hk_base - Win32 SDK SingleThreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "hk_base___Win32_SDK_SingleThreaded"
# PROP BASE Intermediate_Dir "hk_base___Win32_SDK_SingleThreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../hk_library/win32/sdk_singlethreaded"
# PROP Intermediate_Dir "../../hk_tmp/win32/hk_base/sdk_singlethreaded"
# PROP Target_Dir ""
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Zi /O2 /I "../" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"hk_base/base.h" /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /W3 /O2 /I "../" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"hk_base/base.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "hk_base - Win32 SDK MultiThreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "hk_base___Win32_SDK_MultiThreaded"
# PROP BASE Intermediate_Dir "hk_base___Win32_SDK_MultiThreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../hk_library/win32/sdk_multithreaded"
# PROP Intermediate_Dir "../../hk_tmp/win32/hk_base/sdk_multithreaded"
# PROP Target_Dir ""
MTL=midl.exe
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Zi /O2 /I "../" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"hk_base/base.h" /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MT /W3 /Zi /O2 /I "../" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"hk_base/base.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../hk_library/win32/sdk_multithreaded\hk_base_mt.lib"

!ENDIF 

# Begin Target

# Name "hk_base - Win32 Release"
# Name "hk_base - Win32 Debug"
# Name "hk_base - Win32 SDK SingleThreaded"
# Name "hk_base - Win32 SDK MultiThreaded"
# Begin Group "memory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\memory\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\memory\memory.h
# End Source File
# Begin Source File

SOURCE=.\memory\memory.inl
# End Source File
# Begin Source File

SOURCE=.\memory\memory_managed_virtual_class.h
# End Source File
# Begin Source File

SOURCE=.\memory\memory_util.cpp
# End Source File
# Begin Source File

SOURCE=.\memory\memory_util.h
# End Source File
# End Group
# Begin Group "array"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\array\array.cpp
# End Source File
# Begin Source File

SOURCE=.\array\array.h
# End Source File
# Begin Source File

SOURCE=.\array\array.inl
# End Source File
# Begin Source File

SOURCE=.\array\pre_allocated_array.h
# End Source File
# End Group
# Begin Group "hash"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hash\hash.cpp
# End Source File
# Begin Source File

SOURCE=.\hash\hash.h
# End Source File
# Begin Source File

SOURCE=.\hash\hash.inl
# End Source File
# End Group
# Begin Group "stopwatch"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\stopwatch\stopwatch.cpp
# End Source File
# Begin Source File

SOURCE=.\stopwatch\stopwatch.h
# End Source File
# Begin Source File

SOURCE=.\stopwatch\stopwatch_qpt.inl
# End Source File
# End Group
# Begin Group "string"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\string\string.cpp
# End Source File
# Begin Source File

SOURCE=.\string\string.h
# End Source File
# End Group
# Begin Group "id_server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\id_server\id_server.cpp
# End Source File
# Begin Source File

SOURCE=.\id_server\id_server.h
# End Source File
# Begin Source File

SOURCE=.\id_server\id_server.inl
# End Source File
# End Group
# Begin Source File

SOURCE=.\base.h
# End Source File
# Begin Source File

SOURCE=.\base_types.cpp
# ADD CPP /Yc"hk_base/base.h"
# End Source File
# Begin Source File

SOURCE=.\base_types.h
# End Source File
# Begin Source File

SOURCE=.\console.cpp
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# End Target
# End Project
