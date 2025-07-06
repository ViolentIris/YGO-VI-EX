#ifndef __CONFIG_H
#define __CONFIG_H

#pragma once

#define _IRR_STATIC_LIB_
#define IRR_COMPILE_WITH_DX9_DEV_PACK
#ifdef _WIN32

#define NOMINMAX
#include <WinSock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#ifdef _MSC_VER
#define mywcsncasecmp _wcsnicmp
#define mystrncasecmp _strnicmp
#else
#define mywcsncasecmp wcsncasecmp
#define mystrncasecmp strncasecmp
#endif

#define socklen_t int

#else //_WIN32

#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <locale.h>

#define SD_BOTH 2
#define SOCKET int
#define closesocket close
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKADDR_IN sockaddr_in
#define SOCKADDR sockaddr
#define SOCKET_ERRNO() (errno)

#include <wchar.h>
#define mywcsncasecmp wcsncasecmp
#define mystrncasecmp strncasecmp
inline int _wtoi(const wchar_t * s) {
	wchar_t * endptr;
	return (int)wcstol(s, &endptr, 10);
}
#endif

template<size_t N, typename... TR>
inline int myswprintf(wchar_t(&buf)[N], const wchar_t* fmt, TR... args) {
	return swprintf(buf, N, fmt, args...);
}

#include <irrlicht.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else //__APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#endif //__APPLE__
#include "CGUITTFont.h"
#include "CGUIImageButton.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <algorithm>
#include <string>
#include "bufferio.h"
#include "myfilesystem.h"
#include "mysignal.h"
#include "../ocgcore/ocgapi.h"
#include "../ocgcore/common.h"

inline FILE* mywfopen(const wchar_t* filename, const char* mode) {
	FILE* fp{};
#ifdef _WIN32
	wchar_t wmode[20]{};
	BufferIO::CopyCharArray(mode, wmode);
	fp = _wfopen(filename, wmode);
#else
	char fname[1024]{};
	BufferIO::EncodeUTF8(filename, fname);
	fp = std::fopen(fname, mode);
#endif
	return fp;
}

#if !defined(_WIN32)
#define myfopen std::fopen
#elif defined(WDK_NTDDI_VERSION) && (WDK_NTDDI_VERSION >= 0x0A000005) // Redstone 4, Version 1803, Build 17134.
#define FOPEN_WINDOWS_SUPPORT_UTF8
#define myfopen std::fopen
#else
inline FILE* myfopen(const char* filename, const char* mode) {
	wchar_t wfilename[256]{};
	BufferIO::DecodeUTF8(filename, wfilename);
	wchar_t wmode[20]{};
	BufferIO::CopyCharArray(mode, wmode);
	return _wfopen(wfilename, wmode);
}
#endif

#include <irrlicht.h>
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

typedef int BOOL;
extern unsigned short PRO_VERSION;
extern unsigned int enable_log;
extern bool exit_on_return;
extern bool auto_watch_mode;
extern bool open_file;
extern wchar_t open_file_name[256];
extern bool bot_mode;

#endif
