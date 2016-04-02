/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdDebugInfo.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 14:53:02
	LastModify : 
	Description: 简单Debug处理, 一般记录到当前运行程序的目录下, 
                 默认以.debug为文件扩展名
**************************************************************************/
#pragma once

#ifndef _JxdDebugInfo_Lib
#define _JxdDebugInfo_Lib

#include <string.h>
#include <stdio.h>
#include "JxdStringHandle.h"
#ifdef WIN32
#include "JxdStringCoder.h"
#endif

#define WriteDebugToFile

void SetDebugFileName(const char *ApFileName);
void LogToFile(const char *ApFileName, const char *ApFunctionName, const int &ALineName, const char *ApText);
void LogToFile(const char *ApFileName, const char *ApFunctionName, const int &ALineName, const wchar_t *ApText);


#define DebugAO(info) LogToFile( __FILE__, __FUNCTION__, __LINE__, info )
#define DebugA(info, ...) LogToFile( __FILE__, __FUNCTION__, __LINE__, Format(info, ##__VA_ARGS__).c_str() )

//====================================================================
// Windows下使用
//====================================================================
//
#ifdef WIN32
#define DebugWO(info) LogToFile( __FILE__, __FUNCTION__, __LINE__, UnicodeToAscii(info).c_str() )
#define DebugW(info, ...) LogToFile( __FILE__, __FUNCTION__, __LINE__, Format(info, ##__VA_ARGS__).c_str() ) 

#ifdef UNICODE
#define Debug(info, ...) DebugW(info, ##__VA_ARGS__)
#else
#define Debug(info, ...) DebugA(info, ##__VA_ARGS__)
#endif

#endif 

//====================================================================
// Linux下使用
//====================================================================
//
#ifdef linux
#define Debug(info, ...) DebugA(info, ##__VA_ARGS__)
#endif



#endif
