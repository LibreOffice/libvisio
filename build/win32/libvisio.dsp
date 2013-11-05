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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\inc" /I "librevenge-0.9" /I "libwpg-0.2" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\..\inc" /I "librevenge-0.9" /I "libwpg-0.2" /D "NDEBUG" /D "WIN32" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /FD /c
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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\inc" /I "librevenge-0.9" /I "libwpg-0.2" /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GR /GX /ZI /Od /I "..\..\inc" /I "librevenge-0.9" /I "libwpg-0.2" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_LIB" /D "_CRT_SECURE_NO_WARNINGS" /FD /GZ /c
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

SOURCE=..\..\src\lib\VDXParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VisioDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSD5Parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSD6Parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDCharacterList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDContentCollector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDFieldList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDGeometryList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDInternalStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDOutputElementList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDPages.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDParagraphList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDShapeList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDStencils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\librevenge::RVNGStringVector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDStyles.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDStylesCollector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDSVGGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXMLHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXMLParserBase.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXMLTokenMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXTheme.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDZipStream.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\inc\libvisio\libvisio.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\libvisio\VisioDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\libvisio\librevenge::RVNGStringVector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\libvisio_utils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\tokenhash.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\tokens.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VDXParser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSD5Parser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSD6Parser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDCharacterList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDCollector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDContentCollector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDDocumentStructure.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDFieldList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDGeometryList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDInternalStream.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDOutputElementList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDPages.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDParagraphList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDParser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDShapeList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDStencils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\librevenge::RVNGStringVector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDStyles.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDStylesCollector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDSVGGenerator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXMLHelper.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXMLParserBase.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXMLTokenMap.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXParser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDXTheme.h
# End Source File
# Begin Source File

SOURCE=..\..\src\lib\VSDZipStream.h
# End Source File
# End Group
# End Target
# End Project
