# Microsoft Developer Studio Project File - Name="TAR32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TAR32 - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "TAR32.MAK".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "TAR32.MAK" CFG="TAR32 - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "TAR32 - Win32 Release" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "TAR32 - Win32 Debug" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TAR32 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "TAR32 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "TAR32 - Win32 Release"
# Name "TAR32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\arcbz2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\arcfile.cpp
# End Source File
# Begin Source File

SOURCE=..\src\arcgz.cpp
# End Source File
# Begin Source File

SOURCE=..\src\arcz.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Cmdline.c
# End Source File
# Begin Source File

SOURCE=..\src\dlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Minitar.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\normal.cpp
# End Source File
# Begin Source File

SOURCE=..\src\stdafx.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tar32.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Tar32.def
# End Source File
# Begin Source File

SOURCE=..\src\tar32.rc
# End Source File
# Begin Source File

SOURCE=..\src\tar32api.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tar32dll.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tarcmd.cpp
# End Source File
# Begin Source File

SOURCE=..\src\util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\arcbz2.h
# End Source File
# Begin Source File

SOURCE=..\src\arcfile.h
# End Source File
# Begin Source File

SOURCE=..\src\arcgz.h
# End Source File
# Begin Source File

SOURCE=..\src\arcz.h
# End Source File
# Begin Source File

SOURCE=..\src\Bzlib.h
# End Source File
# Begin Source File

SOURCE=..\src\Cmdline.h
# End Source File
# Begin Source File

SOURCE=..\src\dlg.h
# End Source File
# Begin Source File

SOURCE=..\src\normal.h
# End Source File
# Begin Source File

SOURCE=..\src\stdafx.h
# End Source File
# Begin Source File

SOURCE=..\src\Tar.h
# End Source File
# Begin Source File

SOURCE=..\src\tar32.h
# End Source File
# Begin Source File

SOURCE=..\src\tar32api.h
# End Source File
# Begin Source File

SOURCE=..\src\tar32dll.h
# End Source File
# Begin Source File

SOURCE=..\src\tar32res.h
# End Source File
# Begin Source File

SOURCE=..\src\tarcmd.h
# End Source File
# Begin Source File

SOURCE=..\src\util.h
# End Source File
# Begin Source File

SOURCE=..\src\Zconf.h
# End Source File
# Begin Source File

SOURCE=..\src\Zlib.h
# End Source File
# Begin Source File

SOURCE=..\src\Zutil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\src\pointer.cur
# End Source File
# End Group
# Begin Group "doc"

# PROP Default_Filter "doc;txt"
# Begin Source File

SOURCE=..\history.txt
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
# End Group
# Begin Source File

SOURCE=..\src\Libbzip2.lib
# End Source File
# Begin Source File

SOURCE=..\src\Zlib.lib
# End Source File
# Begin Source File

SOURCE=..\src\zlib_d.lib
# End Source File
# End Target
# End Project
