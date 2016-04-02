#pragma once


DWORD CalcCRC32(void* pBuf, DWORD len);
WORD CalcCRC16(void* pBuf, DWORD len);
DWORD EncodeBuffer(void* pBuf, DWORD len);
bool DecodeBuffer(DWORD dwCRC32, void* pBuf, DWORD len);