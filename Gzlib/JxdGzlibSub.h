#pragma once;

#include "zlib.h"

enum TGzipResult{ grSuccess, grDataError, grNoEnoughMem };
//��ѹHTTP����Gzip��
TGzipResult GzipDeCompressHttp(Byte *ApGzipBuffer, uLong AGzipBufferLen, Byte *ApOutBuffer, uLong &AInOutLen);
// ѹ�����ݳ�Gzip��HTTP�� [2012-7-26 15:52 by Terry]
bool        GzipCompressHttp(Byte *ApSrcBuffer, uLong ASrcBufferLen, Byte *ApOutGzipBuffer, uLong &AInOutLen);