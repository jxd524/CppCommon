#pragma once

#ifdef linux
#include "JxdTypedef.h"
#endif

DWORD CalcCRC32(byte* pBuf, DWORD len);
WORD CalcCRC16(byte* pBuf, DWORD len);
void  EncodeBuffer(byte* pBuf, const DWORD &ALen, const DWORD &dwCRC32);
bool  DecodeBuffer(DWORD dwCRC32, byte* pBuf, DWORD len);