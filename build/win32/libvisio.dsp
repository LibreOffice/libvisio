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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "libwpd-0.9" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "libwpd-0.9" /D "NDEBUG" /D "WIN32" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\lib\libvisio-0.2.lib"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "libwpd-0.9" /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GR /GX /ZI /Od /I "libwpd-0.9" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\lib\libvisio-0.2.lib"

!ENDIF 

# Begin Target

# Name "libvisio - Win32 Release"
# Name "libvisio - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\lib\WPG1Parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPG2Parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGBinaryData.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGBitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGBrush.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGColor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGGradient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGInternalStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGPath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGPen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGPoint.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGraphics.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGRect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGSVGGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGXParser.cpp
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

SOURCE=..\..\src\lib\WPG1Parser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPG2Parser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGBinaryData.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGBitmap.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGBrush.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGColor.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGGradient.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGInternalStream.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGPaintInterface.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGPath.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGPen.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGPoint.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGraphics.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGRect.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGString.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGSVGGenerator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\WPGXParser.h
# End Source File
# End Group
# End Target
# End Project
