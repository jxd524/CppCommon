#include "stdafx.h"
#include "JxdStringCoder.h"

#ifdef linux
#include <stdio.h>
#endif

const static char *CtpDataSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#ifdef WIN32
#pragma warning(disable:4996)
wstring AsciiToUnicode( const char *buf )
{
    if ( NULL == buf || strlen(buf) == 0 ) return L"";

    int nlen = MultiByteToWideChar( CP_ACP, 0, buf, -1, NULL, 0 );
    wchar_t *pBuf = new wchar_t[nlen + 1];
    memset( pBuf, 0, (nlen + 1) * sizeof(wchar_t) );
    MultiByteToWideChar( CP_ACP, 0, buf, -1, pBuf, nlen );

    wstring strResult( pBuf );
    delete []pBuf;

    return strResult;
}

string UnicodeToAscii( const wchar_t *buf )
{
    if ( buf == NULL || 0 == wcslen(buf) ) return "";

    int nlen = WideCharToMultiByte( CP_ACP, 0, buf, -1, NULL, 0, NULL, NULL );
    char *pBuf = new char[nlen + 1];
    memset( pBuf, 0, nlen + 1 );
    WideCharToMultiByte( CP_ACP, 0, buf, -1, pBuf, nlen, NULL, NULL );

    string strResult( pBuf );
    delete []pBuf;

    return strResult;
}

wstring Utf8ToUnicode( const char *buf )
{
    if ( buf == NULL || 0 == strlen(buf) ) return L"";

    int nlen = MultiByteToWideChar( CP_UTF8, 0, buf, -1, NULL, 0 );
    wchar_t *pBuf = new wchar_t[nlen];
    memset( pBuf, 0, nlen * sizeof(wchar_t) );
    MultiByteToWideChar( CP_UTF8, 0, buf, -1, pBuf, nlen );

    wstring strResult( pBuf );
    delete []pBuf;

    return strResult;
}

string UnicodeToUtf8( const wchar_t *buf )
{
    if ( NULL == buf || 0 == wcslen(buf) )
    {
        return "";
    }
    int nlen = WideCharToMultiByte( CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL );
    char *pTempBuf = new char[nlen + 1];
    memset( pTempBuf, 0, nlen + 1 );
    WideCharToMultiByte( CP_UTF8, 0, buf, -1, pTempBuf, nlen, NULL, NULL );

    string strResult( pTempBuf );
    delete []pTempBuf;

    return strResult;
}


string GBKToUTF8( const char *buf )
{
    string strOutUTF8 = "";  
    WCHAR * str1;  
    int n = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);  
    str1 = new WCHAR[n];  
    MultiByteToWideChar(CP_ACP, 0, buf, -1, str1, n);  
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);  
    char * str2 = new char[n];  
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);  
    strOutUTF8 = str2;  
    delete[]str1;  
    str1 = NULL;  
    delete[]str2;  
    str2 = NULL;  
    return strOutUTF8;  
}

string UTF8ToGBK( const char *buf )
{
    int len = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);  
    unsigned short * wszGBK = new unsigned short[len + 1];  
    memset(wszGBK, 0, len * 2 + 2);  
    MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buf, -1, (LPWSTR)wszGBK, len);  

    len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, NULL, 0, NULL, NULL);  
    char *szGBK = new char[len + 1];  
    memset(szGBK, 0, len + 1);  
    WideCharToMultiByte(CP_ACP,0, (LPCWSTR)wszGBK, -1, szGBK, len, NULL, NULL);  
    //strUTF8 = szGBK;  
    std::string strTemp(szGBK);  
    delete[]szGBK;  
    delete[]wszGBK;  
    return strTemp;  
}

CString IntToHex( const long &AValue )
{
    CString str;
    str.Format( TEXT("%%%X"), AValue );
    return str;
}

CStringA IntToHexA( const long &AValue )
{
    CStringA str;
    str.Format( "%%%X", AValue );
    return str;
}

#endif

#define _UrlParamEncodeImpl(Type, returnType, CtEncodeInfo)         \
    returnType strResult;                                           \
    Type c;                                                         \
    for( int i = 0; i < len; i++ ){                                 \
        c = buf[i];                                                 \
        if ( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||    \
           (c >= 'A' && c <= 'Z') || ('.' == c) || ('_' == c) ||    \
           ('-' == c) || ('*' == c) ){                              \
            strResult += c;                                         \
        } else {                                                    \
            strResult += '%' ;                                      \
            strResult += CtEncodeInfo[(c & 0xF0) >> 4];             \
            strResult += CtEncodeInfo[c & 0x0F];                    \
        }                                                           \
    }                                                               \
    return strResult;
string UrlParamEncode( const char *buf, const int len )
{
    _UrlParamEncodeImpl( char, string, "0123456789ABCDEF" );
}
wstring UrlParamEncode( const wchar_t *buf, const int len )
{
    _UrlParamEncodeImpl( wchar_t, wstring, L"0123456789ABCDEF" );
}


#define _Dec2HexImpl(type)                  \
    if ( 0 <= n && n <= 9 ) {               \
        return type( short('0') + n );      \
    } else if ( 10 <= n && n <= 15 ) {      \
        return type( short('A') + n - 10 ); \
    }                                       \
    return type(0);

char Dec2HexA( WORD n )
{
    _Dec2HexImpl( char );
}

wchar_t   Dec2HexW(WORD n)
{
    _Dec2HexImpl( wchar_t );
}

#define _Hex2DecImpl\
    if ( '0'<=c && c<='9' ){\
        return WORD(c-'0');\
    } else if ( 'a'<=c && c<='f' ) {\
        return ( WORD(c-'a') + 10 );\
    } else if ( 'A'<=c && c<='F' ) {\
        return ( WORD(c-'A') + 10 );\
    } \
    return -1;

WORD Hex2Dec( char c )
{
    _Hex2DecImpl;
}

WORD Hex2Dec( wchar_t c )
{
    _Hex2DecImpl;
}

string Base64Encode( const char *pUtf8Buf, int len )
{
    if ( NULL == pUtf8Buf )
    {
        return "";
    }
    
    string strResult;
    int nModLen = len % 3;

    byte b1, b2, b3;
    byte NewBuf[4];
    for (int i = 0; i < len; ) 
    {
        b1 = pUtf8Buf[i++];
        b2 = (i >= len) ? 0 : pUtf8Buf[i++];
        b3 = (i >= len) ? 0 : pUtf8Buf[i++];

        NewBuf[0] = b1 >> 2;
        NewBuf[1] = (b1 << 6 >> 2 | b2 >> 4 ) & 0x3F;
        NewBuf[2] = (b2 << 4 >> 2 | b3 >> 6 ) & 0x3F;
        NewBuf[3] = b3 & 0x3F;
        for ( int j = 0; j != 4; j++ )
        {
            strResult.push_back( CtpDataSet[ NewBuf[j] ] );
        }
    }
    if ( nModLen >= 1 )
    {
        strResult[ strResult.length() - 1 ] = '=';
    }
    if ( nModLen == 1 )
    {
        strResult[ strResult.length() - 2 ] = '=';
    }
    return strResult;
}

string Base64Decode( const char *pBase64Buf, int len )
{
    //解码表
    const char DecodeTable[] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        62, // '+'
        0, 0, 0,
        63, // '/'
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
        0, 0, 0, 0, 0, 0, 0,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
        0, 0, 0, 0, 0, 0,
        26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
    };
    //返回值
    string strDecode;
    int nValue;
    int i= 0;
    int OutByte = 0;
    while (i < len)
    {
        if (*pBase64Buf != '\r' && *pBase64Buf!='\n')
        {
            nValue = DecodeTable[(int)(*pBase64Buf++)] << 18;
            nValue += DecodeTable[(int)(*pBase64Buf++)] << 12;
            strDecode+=(nValue & 0x00FF0000) >> 16;
            OutByte++;
            if (*pBase64Buf != '=')
            {
                nValue += DecodeTable[(int)(*pBase64Buf++)] << 6;
                strDecode+=(nValue & 0x0000FF00) >> 8;
                OutByte++;
                if (*pBase64Buf != '=')
                {
                    nValue += DecodeTable[(int)(*pBase64Buf++)];
                    strDecode+=nValue & 0x000000FF;
                    OutByte++;
                }
            }
            i += 4;
        }
        else// 回车换行,跳过
        {
            pBase64Buf++;
            i++;
        }
    }
    return strDecode;
}


#define _UrlParamDecodeImpl(type, returnType)       \
    returnType result;                              \
    for ( int i = 0; i < len; i++ ) {               \
        type c = buf[i];                            \
        if ( c != '%' ) {                           \
            result += c;                            \
        } else {                                    \
            type c1 = buf[++i];                     \
            type c0 = buf[++i];                     \
            int num = 0;                            \
            num += Hex2Dec(c1) * 16 + Hex2Dec(c0);  \
            result += type(num);                    \
        }                                           \
    }                                               \
    return result;

string UrlParamDecodeA( const char *buf, const int len )
{
    _UrlParamDecodeImpl( char, string );
}

wstring UrlParamDecodeW( const wchar_t *buf, const int len )
{
    _UrlParamDecodeImpl( wchar_t, wstring );
}


string StrToHex( const char *srcBuf, int len, bool bLowerCase /*= false*/ )
{
    if ( len <= 0 )
    {
        return "";
    }
    const char *pFormat = bLowerCase ? "%02x" : "%02X";

    char buf[64];
    int n;
    string s;
    for (int i = 0; i != len; i++)
    {
        char a = srcBuf[i];
        n = sprintf( buf, pFormat, a );
        buf[n] = 0;
        s += buf;
    }
    return s;
}