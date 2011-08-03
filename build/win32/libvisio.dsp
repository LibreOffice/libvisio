# Microsoft Developer Studio Project File - Name="libvisio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libvisio - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libvisio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libvisio.mak" CFG="libvisio - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libvisio - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libvisio - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libvisio - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "libwpd-0.9" /I "libwpg-0.2" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "libwpd-0.9" /I "libwpg-0.2" /D "NDEBUG" /D "WIN32" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\lib\libvisio-0.0.lib"

!ELSEIF  "$(CFG)" == "libvisio - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "libwpd-0.9" /I "libwpg-0.2" /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GR /GX /ZI /Od /I "libwpd-0.9" /I "libwpg-0.2" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\lib\libvisio-0.0.lib"

!ENDIF 

# Begin Target

# Name "libvisio - Win32 Release"
# Name "libvisio - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\lib\libvisio_utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VisioDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSD11Parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSD6Parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDInternalStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDSVGGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXCharacterList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXCollector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXContentCollector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXGeometryList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXOutputElementList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXShapeList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXStyles.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXStylesCollector.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\lib\libvisio.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\libvisio_utils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VisioDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSD11Parser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSD6Parser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDInternalStream.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDSVGGenerator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXCharacterList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXCollector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXContentCollector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXDocumentStructure.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXGeometryList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXOutputElementList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXParser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXShapeList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXStyles.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXStylesCollector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXTypes.h
# End Source File
# End Group
# End Target
# End Project
