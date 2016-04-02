/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdStringCoder.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-10-8 15:55:16
	LastModify : 
	Description: 字符串编码相关
**************************************************************************/
#pragma once
#ifndef _JxdStringCoder_Lib
#define _JxdStringCoder_Lib

#include <string>
#include <vector>
using namespace std;

#ifdef linux
#include "JxdTypedef.h"
#endif

//====================================================================
// 暂时只实现Window下的函数
//====================================================================
//
#ifdef WIN32
wstring AsciiToUnicode(const char *buf);
string  UnicodeToAscii(const wchar_t *buf);

wstring Utf8ToUnicode(const char *buf);
string  UnicodeToUtf8(const wchar_t *buf);

string  GBKToUTF8(const char *buf);
string  UTF8ToGBK(const char *buf);

CString  IntToHex(const long &AValue);
CStringA IntToHexA(const long &AValue);
#endif

//====================================================================
// 以下函数可跨平台使用
//====================================================================
//
char      Dec2HexA(WORD n);
wchar_t   Dec2HexW(WORD n);

WORD    Hex2Dec(char c);
WORD    Hex2Dec(wchar_t c);

//BASE64编码, 解码
string Base64Encode(const char *pUtf8Buf, int len);
string Base64Decode(const char *pBase64Buf, int len);

string  UrlParamEncode(const char *buf, const int len); //编码
wstring UrlParamEncode(const wchar_t *buf, const int len); //编码

string  UrlParamDecodeA(const char *buf, const int len); //还原
wstring UrlParamDecodeW(const wchar_t *buf, const int len); //还原

string  StrToHex(const char *srcBuf, int len, bool bLowerCase = false);

#endif