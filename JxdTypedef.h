/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdTypedef.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-10-18 15:46:10
	LastModify : 
	Description: 类型定义,用于Linux系统, VS下已经有相关定义
**************************************************************************/
#pragma once
#ifndef _JxdTypedef_Lib
#define _JxdTypedef_Lib

#ifdef WIN32

class CxdObject

#ifdef _MFC_VER
    : public CObject
#endif

{
public:
    virtual ~CxdObject(){}
};

typedef int socklen_t;
typedef int ssize_t;

#endif

#ifdef linux

#include <stddef.h>
typedef void*               LPVOID;
typedef float               FLOAT;
typedef bool                boolean;
typedef signed char         INT8;
typedef signed short        INT16;
typedef signed int          INT32;
typedef long int            INT64;
typedef unsigned char       byte;
typedef unsigned int        UINT;
typedef unsigned char       byte;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned long int   UINT64;
typedef int                 SOCKET;
typedef TCHAR               char;

class CxdObject
{
public:
    virtual ~CxdObject(){}
};

#endif

#endif