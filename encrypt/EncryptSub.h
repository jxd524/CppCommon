
#pragma once;

#include <string>
using namespace std;

class CxdStreamBasic;

//#define SHA1MaxSize 20
extern const int CtSHA1MaxSize;
extern const int CtMD5MaxSize;

//SHA1 算法 ApOut大小: SHA1MaxSize
void     SHA1Buffer(const char *ApBuf, int ALen, unsigned char *ApOut);
string   SHA1ToStr(unsigned char *ApSHA1, bool bLowerCase = false);
CStringA SHA1StringA(const char *ApText, bool bLowserCase = true);

//MD5 算法
void    MD5Buffer(const char *ApBuf, int ALen, unsigned char *ApOut);
string  MD5ToStr(const unsigned char *ApMD5, bool bLowerCase = false ) ;
CString MD5String(const CString &AText, bool bLowerCase = false);
CStringA MD5StringA(const CStringA &AText, bool bLowerCase = false);

// 自定义加解密函数 [2012-6-30 09:36 by Terry]
CString  EncryptString(const TCHAR *ApText, const int &ATextLen, const TCHAR *ApPassword); //加密
CStringA EncryptStringA(const char *ApText, const int &ATextLen, const char *ApPassword); //加密
void    DecryptString(const TCHAR *ApEncrptText, const TCHAR *ApPassword, CxdStreamBasic *AStream); //解密
void    DecryptStringA(const char *ApEncrptText, const char *ApPassword, CxdStreamBasic *AStream); //解密