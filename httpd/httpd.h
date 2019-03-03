#ifndef __HTTPD_H__
#define __HTTPD_H__

#define _CRT_SECURE_NO_WARNINGS

#ifdef  FD_SETSIZE
#undef  FD_SETSIZE
#define FD_SETSIZE  1024
#else 
#define FD_SETSIZE  1024
#endif

#define BUFFER_UNIT 4096

#define BUFFER_PAGE 65536

#ifdef _DEBUG
#define ASSERT(x)   assert(x)
#else
#define ASSERT(x)
#endif

#include <stdio.h>

#ifdef WIN32
#include <Winsock2.h>
#include <Windows.h>
#endif

#include <assert.h>
#include <time.h>
#include "types.h"
#include "utils.h"
#include "logger.h"
#include "network.h"
#include "event.h"
#include "http.h"

#pragma pack(1)

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

#endif