# Microsoft Developer Studio Generated NMAKE File, Based on tar32.dsp
!IF "$(CFG)" == ""
CFG=tar32 - Win32 Debug
!MESSAGE 構成が指定されていません。ﾃﾞﾌｫﾙﾄの tar32 - Win32 Debug を設定します。
!ENDIF 

!IF "$(CFG)" != "tar32 - Win32 Release" && "$(CFG)" != "tar32 - Win32 Debug"
!MESSAGE 指定された ﾋﾞﾙﾄﾞ ﾓｰﾄﾞ "$(CFG)" は正しくありません。
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
!ERROR 無効な構成が指定されています。
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "tar32 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\tar32.dll"


CLEAN :
	-@erase "$(INTDIR)\arcbz2.obj"
	-@erase "$(INTDIR)\arcfile.obj"
	-@erase "$(INTDIR)\arcgz.obj"
	-@erase "$(INTDIR)\arcz.obj"
	-@erase "$(INTDIR)\Cmdline.obj"
	-@erase "$(INTDIR)\dlg.obj"
	-@erase "$(INTDIR)\normal.obj"
	-@erase "$(INTDIR)\tar32.obj"
	-@erase "$(INTDIR)\tar32.res"
	-@erase "$(INTDIR)\tar32api.obj"
	-@erase "$(INTDIR)\tar32dll.obj"
	-@erase "$(INTDIR)\tarcmd.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\tar32.exp"
	-@erase "$(OUTDIR)\tar32.lib"
	-@erase "..\tar32.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TAR32_EXPORTS" /Fp"$(INTDIR)\tar32.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x411 /fo"$(INTDIR)\tar32.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tar32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib zlib.lib libbzip2.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\tar32.pdb" /machine:I386 /def:".\Tar32.def" /out:"../tar32.dll" /implib:"$(OUTDIR)\tar32.lib" 
DEF_FILE= \
	".\Tar32.def"
LINK32_OBJS= \
	"$(INTDIR)\arcbz2.obj" \
	"$(INTDIR)\arcfile.obj" \
	"$(INTDIR)\arcgz.obj" \
	"$(INTDIR)\arcz.obj" \
	"$(INTDIR)\Cmdline.obj" \
	"$(INTDIR)\dlg.obj" \
	"$(INTDIR)\normal.obj" \
	"$(INTDIR)\tar32.obj" \
	"$(INTDIR)\tar32api.obj" \
	"$(INTDIR)\tar32dll.obj" \
	"$(INTDIR)\tarcmd.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\tar32.res" \
	".\Libbzip2.lib" \
	".\Zlib.lib"

"..\tar32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tar32 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "c:\windows\system\tar32.dll"


CLEAN :
	-@erase "$(INTDIR)\arcbz2.obj"
	-@erase "$(INTDIR)\arcfile.obj"
	-@erase "$(INTDIR)\arcgz.obj"
	-@erase "$(INTDIR)\arcz.obj"
	-@erase "$(INTDIR)\Cmdline.obj"
	-@erase "$(INTDIR)\dlg.obj"
	-@erase "$(INTDIR)\normal.obj"
	-@erase "$(INTDIR)\tar32.obj"
	-@erase "$(INTDIR)\tar32.res"
	-@erase "$(INTDIR)\tar32api.obj"
	-@erase "$(INTDIR)\tar32dll.obj"
	-@erase "$(INTDIR)\tarcmd.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\tar32.exp"
	-@erase "$(OUTDIR)\tar32.lib"
	-@erase "$(OUTDIR)\tar32.pdb"
	-@erase "c:\windows\system\tar32.dll"
	-@erase "c:\windows\system\tar32.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TAR32_EXPORTS" /Fp"$(INTDIR)\tar32.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x411 /fo"$(INTDIR)\tar32.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tar32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib zlib_d.lib libbzip2.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\tar32.pdb" /debug /machine:I386 /def:".\Tar32.def" /out:"c:\windows\system\tar32.dll" /implib:"$(OUTDIR)\tar32.lib" /pdbtype:sept 
DEF_FILE= \
	".\Tar32.def"
LINK32_OBJS= \
	"$(INTDIR)\arcbz2.obj" \
	"$(INTDIR)\arcfile.obj" \
	"$(INTDIR)\arcgz.obj" \
	"$(INTDIR)\arcz.obj" \
	"$(INTDIR)\Cmdline.obj" \
	"$(INTDIR)\dlg.obj" \
	"$(INTDIR)\normal.obj" \
	"$(INTDIR)\tar32.obj" \
	"$(INTDIR)\tar32api.obj" \
	"$(INTDIR)\tar32dll.obj" \
	"$(INTDIR)\tarcmd.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\tar32.res" \
	".\Libbzip2.lib" \
	".\Zlib.lib"

"c:\windows\system\tar32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("tar32.dep")
!INCLUDE "tar32.dep"
!ELSE 
!MESSAGE Warning: cannot find "tar32.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "tar32 - Win32 Release" || "$(CFG)" == "tar32 - Win32 Debug"
SOURCE=.\arcbz2.cpp

"$(INTDIR)\arcbz2.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\arcfile.cpp

"$(INTDIR)\arcfile.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\arcgz.cpp

"$(INTDIR)\arcgz.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\arcz.cpp

"$(INTDIR)\arcz.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Cmdline.c

"$(INTDIR)\Cmdline.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dlg.cpp

"$(INTDIR)\dlg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\normal.cpp

"$(INTDIR)\normal.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tar32.cpp

"$(INTDIR)\tar32.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tar32.rc

"$(INTDIR)\tar32.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\tar32api.cpp

"$(INTDIR)\tar32api.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tar32dll.cpp

"$(INTDIR)\tar32dll.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tarcmd.cpp

"$(INTDIR)\tarcmd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\util.cpp

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

