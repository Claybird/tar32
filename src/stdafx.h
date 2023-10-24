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
#include <algorithm>
#include <memory>

#ifndef COUNTOF
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))
#endif//COUNTOF

//VC2005 on x86‚É‚Äsize_t‚ª64bit‚Å‚È‚¢‚½‚ß
#ifndef size64
typedef __int64 size64;
#endif//size64

class CAutoFile {
protected:
	FILE* _fp;
	std::string _path;
	CAutoFile(const CAutoFile&) = delete;
	const CAutoFile& operator=(const CAutoFile&) = delete;
public:
	CAutoFile() :_fp(NULL) {}
	virtual ~CAutoFile() {
		close();
	}
	operator FILE* () { return _fp; }
	bool is_opened() const { return _fp != NULL; }
	void close() {
		if (_fp) {
			fclose(_fp);
			_fp = NULL;
			_path.clear();
		}
	}
	void open(const std::string& fname, const std::string& mode = "rb") {
		close();
		_path = fname;
		auto err = fopen_s(&_fp, fname.c_str(), mode.c_str());
		if (err == 0 && _fp) {
			//set buffer size
			setvbuf(_fp, NULL, _IOFBF, 1024 * 1024);
		}
	}
	const std::string get_path()const { return _path; }
};

#endif
