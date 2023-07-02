/*	pre compiled header */
#ifndef __TAR32_STDAFX_H
#define __TAR32_STDAFX_H

#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>	// chmod
#include <crtdbg.h>		// _CrtMemCheckpoint, _CrtMemDumpAllObjectsSince
#include <ctype.h>
#include <wtypes.h>
#include <direct.h> // _mkdir
#include <mbstring.h>
#include <fcntl.h>
#include <assert.h>

#include <windows.h>
#include <winbase.h>
#include <winnls.h>
#include <winuser.h>
#include <shlwapi.h>
#include <process.h> // _beginthread
#include <commctrl.h>

#include <sys/types.h>	// stat
#include <sys/utime.h> // utime
#include <sys/stat.h>	// S_IWRITE

#include <cstdio>
#include <string>
#include <vector>
#include <list>
#include <strstream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <memory>

#ifndef COUNTOF
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))
#endif//COUNTOF

//VC2005 on x86‚É‚Äsize_t‚ª64bit‚Å‚È‚¢‚½‚ß
#ifndef size64
typedef __int64 size64;
#endif//size64

#ifdef UNIT_TEST
#include <gtest/gtest.h>
inline std::string PROJECT_DIR() {
	char path[_MAX_PATH] = {};
	strncpy(path, __FILE__, _MAX_PATH);
	PathRemoveFileSpec(path);
	return path;
}

#endif

#endif
