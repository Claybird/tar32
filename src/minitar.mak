# Microsoft Developer Studio Generated NMAKE File, Based on minitar.dsp
!IF "$(CFG)" == ""
CFG=minitar - Win32 Debug
!MESSAGE 構成が指定されていません。ﾃﾞﾌｫﾙﾄの minitar - Win32 Debug を設定します。
!ENDIF 

!IF "$(CFG)" != "minitar - Win32 Release" && "$(CFG)" != "minitar - Win32 Debug"
!MESSAGE 指定された ﾋﾞﾙﾄﾞ ﾓｰﾄﾞ "$(CFG)" は正しくありません。
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "minitar.mak" CFG="minitar - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "minitar - Win32 Release" ("Win32 (x86) Console Application" 用)
!MESSAGE "minitar - Win32 Debug" ("Win32 (x86) Console Application" 用)
!MESSAGE 
!ERROR 無効な構成が指定されています。
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "minitar - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : "..\minitar.exe"

!ELSE 

ALL : "tar32 - Win32 Release" "..\minitar.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"tar32 - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\Minitar.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "..\minitar.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\minitar.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\minitar.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\minitar.pdb" /machine:I386 /out:"../minitar.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Minitar.obj" \
	"$(OUTDIR)\tar32.lib"

"..\minitar.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "minitar - Win32 Debug"

OUTDIR=.\Debug_minitar
INTDIR=.\Debug_minitar

!IF "$(RECURSE)" == "0" 

ALL : ".\Debug\minitar.exe"

!ELSE 

ALL : "tar32 - Win32 Debug" ".\Debug\minitar.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"tar32 - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\Minitar.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\minitar.pdb"
	-@erase ".\Debug\minitar.exe"
	-@erase ".\Debug\minitar.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\minitar.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\minitar.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\minitar.pdb" /debug /machine:I386 /out:"Debug/minitar.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Minitar.obj" \
	".\Debug\tar32.lib"

".\Debug\minitar.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("minitar.dep")
!INCLUDE "minitar.dep"
!ELSE 
!MESSAGE Warning: cannot find "minitar.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "minitar - Win32 Release" || "$(CFG)" == "minitar - Win32 Debug"
SOURCE=.\Minitar.c

"$(INTDIR)\Minitar.obj" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "minitar - Win32 Release"

"tar32 - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\tar32.mak CFG="tar32 - Win32 Release" 
   cd "."

"tar32 - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\tar32.mak CFG="tar32 - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "minitar - Win32 Debug"

"tar32 - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\tar32.mak CFG="tar32 - Win32 Debug" 
   cd "."

"tar32 - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\tar32.mak CFG="tar32 - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 


!ENDIF 

