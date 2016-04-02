
#pragma once;

#include <string>
using namespace std;

class CxdStreamBasic;

//#define SHA1MaxSize 20
extern const int CtSHA1MaxSize;
extern const int CtMD5MaxSize;

//SHA1 �㷨 ApOut��С: SHA1MaxSize
void     SHA1Buffer(const char *ApBuf, int ALen, unsigned char *ApOut);
string   SHA1ToStr(unsigned char *ApSHA1, bool bLowerCase = false);
CStringA SHA1StringA(const char *ApText, bool bLowserCase = true);

//MD5 �㷨
void    MD5Buffer(const char *ApBuf, int ALen, unsigned char *ApOut);
string  MD5ToStr(const unsigned char *ApMD5, bool bLowerCase = false ) ;
CString MD5String(const CString &AText, bool bLowerCase = false);
CStringA MD5StringA(const CStringA &AText, bool bLowerCase = false);

// �Զ���ӽ��ܺ��� [2012-6-30 09:36 by Terry]
CString  EncryptString(const TCHAR *ApText, const int &ATextLen, const TCHAR *ApPassword); //����
CStringA EncryptStringA(const char *ApText, const int &ATextLen, const char *ApPassword); //����
void    DecryptString(const TCHAR *ApEncrptText, const TCHAR *ApPassword, CxdStreamBasic *AStream); //����
void    DecryptStringA(const char *ApEncrptText, const char *ApPassword, CxdStreamBasic *AStream); //����