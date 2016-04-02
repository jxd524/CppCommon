#pragma once

#ifndef _JxdZipSub_Lib
#define _JxdZipSub_Lib

//压缩指定目录, ApExceptItems指定项将不加入压缩包中. 格式: TEXT("a.txt,dir\\b.bmp")
bool ZipFiles(const TCHAR* ApDir, const TCHAR* ApZipFileName, const TCHAR* ApExceptItems = NULL);

//解压ZIP包到指定目录, ApExceptItems指定项将不进行解压, 格式 TEXT("a.txt,dir/b.bmp")
bool UnZipFiles(const TCHAR* ApDir, const TCHAR* ApZipFileName, const TCHAR* ApExceptItems = NULL);

#endif