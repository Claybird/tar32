# Microsoft Developer Studio Project File - Name="tar32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=tar32 - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "tar32.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "tar32.mak" CFG="tar32 - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "tar32 - Win32 Release" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "tar32 - Win32 Debug" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tar32 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TAR32_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "zlib" /I "bzip2" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TAR32_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../tar32.dll" /implib:"../tar32.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "tar32 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TAR32_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "zlib" /I "bzip2" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TAR32_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"f:\windows\system32\tar32.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "tar32 - Win32 Release"
# Name "tar32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\arcbz2.cpp
# End Source File
# Begin Source File

SOURCE=.\arcfile.cpp
# End Source File
# Begin Source File

SOURCE=.\arcgz.cpp
# End Source File
# Begin Source File

SOURCE=.\arcz.cpp
# End Source File
# Begin Source File

SOURCE=.\Cmdline.c
# End Source File
# Begin Source File

SOURCE=.\dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\normal.cpp
# End Source File
# Begin Source File

SOURCE=.\rpm.cpp
# End Source File
# Begin Source File

SOURCE=.\susie.cpp
# End Source File
# Begin Source File

SOURCE=.\tar32.cpp
# End Source File
# Begin Source File

SOURCE=.\Tar32.def
# End Source File
# Begin Source File

SOURCE=.\tar32.rc
# End Source File
# Begin Source File

SOURCE=.\tar32api.cpp
# End Source File
# Begin Source File

SOURCE=.\tar32dll.cpp
# End Source File
# Begin Source File

SOURCE=.\tarcmd.cpp
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ar.h
# End Source File
# Begin Source File

SOURCE=.\arcbz2.h
# End Source File
# Begin Source File

SOURCE=.\arcfile.h
# End Source File
# Begin Source File

SOURCE=.\arcgz.h
# End Source File
# Begin Source File

SOURCE=.\arcz.h
# End Source File
# Begin Source File

SOURCE=.\Cmdline.h
# End Source File
# Begin Source File

SOURCE=.\cpio.h
# End Source File
# Begin Source File

SOURCE=.\dlg.h
# End Source File
# Begin Source File

SOURCE=.\fast_stl.h
# End Source File
# Begin Source File

SOURCE=.\normal.h
# End Source File
# Begin Source File

SOURCE=.\rpm.h
# End Source File
# Begin Source File

SOURCE=.\Tar.h
# End Source File
# Begin Source File

SOURCE=.\tar32.h
# End Source File
# Begin Source File

SOURCE=.\tar32api.h
# End Source File
# Begin Source File

SOURCE=.\tar32dll.h
# End Source File
# Begin Source File

SOURCE=.\tarcmd.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\pointer.cur
# End Source File
# End Group
# Begin Source File

SOURCE=..\history.txt
# End Source File
# Begin Source File

SOURCE=..\html\index.html
# End Source File
# Begin Source File

SOURCE=..\tar32_2.txt
# End Source File
# Begin Source File

SOURCE=..\sdk\Tar_api.txt
# End Source File
# Begin Source File

SOURCE=..\sdk\Tar_cmd.txt
# End Source File
# Begin Source File

SOURCE=..\sdk\Tar_fmt.txt
# End Source File
# Begin Source File

SOURCE=.\bzip2\libbz2.lib
# End Source File
# Begin Source File

SOURCE=.\zlib\zlib.lib
# End Source File
# End Target
# End Project
