#pragma once;

#include "zlib.h"

enum TGzipResult{ grSuccess, grDataError, grNoEnoughMem };
//解压HTTP流的Gzip流
TGzipResult GzipDeCompressHttp(Byte *ApGzipBuffer, uLong AGzipBufferLen, Byte *ApOutBuffer, uLong &AInOutLen);
// 压缩数据成Gzip的HTTP流 [2012-7-26 15:52 by Terry]
bool        GzipCompressHttp(Byte *ApSrcBuffer, uLong ASrcBufferLen, Byte *ApOutGzipBuffer, uLong &AInOutLen);