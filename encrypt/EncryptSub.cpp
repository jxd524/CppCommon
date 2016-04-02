#include "stdafx.h"
#include "EncryptSub.h"
#include "md5.h"
#include "JxdStringCoder.h"
#include "JxdStringHandle.h"
#include "JxdDataStream.h"
extern "C"
{
    #include "crypt.h"
}

static const int CtSHA1MaxSize = 20;
static const int CtMD5MaxSize = 16;

void SHA1Buffer(const char *ApBuf, int ALen, unsigned char *ApOut)
{
    hash_state md;
    sha1_init( &md );
    sha1_process( &md, (unsigned char*)ApBuf, ALen );
    sha1_done( &md, ApOut );
}

string SHA1ToStr(unsigned char *ApSHA1, bool bLowerCase /*= false*/)
{
    char *pFormat;
    bLowerCase ? pFormat = "%02x" : pFormat = "%02X";

    char buf[8];
    int n;
    string s;
    for (int i = 0; i != CtSHA1MaxSize; i++)
    {
        n = sprintf_s( buf, 8, pFormat, ApSHA1[i] );
        buf[n] = 0;
        s += buf;
    }
    return s;
}

void MD5Buffer( const char *ApBuf, int ALen, unsigned char *ApOut )
{
    MD5_CTX context;

    MD5Init( &context );
    MD5Update( &context, (unsigned char *)ApBuf, ALen );
    MD5Final( &context, ApOut );
}

string MD5ToStr( const unsigned char *ApMD5, bool bLowerCase /*= false */ )
{
    const static char CtUpperDigits[CtMD5MaxSize] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    const static char CtLowerDigits[CtMD5MaxSize] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    const char* const CtpDigits = bLowerCase ? CtLowerDigits : CtUpperDigits;
    string s;
    for ( int i = 0; i != CtMD5MaxSize; i++ )
    {
        s += CtpDigits[(ApMD5[i] >> 4) & 0x0f];
        s += CtpDigits[ApMD5[i] & 0x0f];
    }
    return s;
}

CString MD5String( const CString &AText, bool bLowerCase )
{
    string str = UnicodeToAscii( AText );
    unsigned char md5Buf[16];
    MD5Buffer( str.c_str(), (int)str.length(), md5Buf );
    str = MD5ToStr( md5Buf, bLowerCase );
    return  AsciiToUnicode( str.c_str() ).c_str();
}

CStringA MD5StringA( const CStringA &AText, bool bLowerCase )
{
    string str( AText );
    unsigned char md5Buf[16];
    MD5Buffer( str.c_str(), (int)str.length(), md5Buf );
    str = MD5ToStr( md5Buf, bLowerCase );
    return  str.c_str();
}

CStringA SHA1StringA( const char *ApText, bool bLowserCase )
{
    char buf[CtSHA1MaxSize + 1] = {0};
    SHA1Buffer( ApText, (int)strlen(ApText), (unsigned char *)buf );
    CStringA str( SHA1ToStr((unsigned char *)buf, bLowserCase).c_str() );
    return str;
}

CString EncryptString( const TCHAR *ApText, const int &ATextLen, const TCHAR *ApPassword )
{
    int nPwLen = (int)_tcslen( ApPassword );
    if ( 0 == nPwLen )
    {
        return TEXT("");
    }
    srand( (unsigned)time(NULL) );

    CString strResult;
    int nTextLen = ATextLen;
    int nSeed = rand();
    strResult = IntToHex( nSeed );
    int nTemp;
    for (int i = 0; i < nTextLen; i++)
    {
    	nTemp = ApText[i] ^ ApPassword[ (i + nSeed + 1) % nPwLen ];
        strResult += IntToHex( nTemp );
    }
    return strResult;
}

void DecryptString( const TCHAR *ApEncrptText, const TCHAR *ApPassword, CxdStreamBasic *AStream )
{
    int nPwLen = (int)_tcslen( ApPassword );
    if ( 0 == nPwLen )
    {
        return;
    }
    int nTextLen = (int)_tcslen(ApEncrptText);
    int i(0), j, nTemp, nSeed, nIndex;
    bool bReadSeed( false );
    CString strTemp;
    while ( i < nTextLen )
    {
        if ( '%' == ApEncrptText[i] )
        {
            j = i + 1;
            while ( j < nTextLen )
            {
                if ( '%' == ApEncrptText[j++] )
                {
                    --j;
                    break;
                }
            }
            strTemp.Empty();
            while ( ++i < j )
            {
                strTemp.AppendChar( ApEncrptText[i] );
            }
            if ( !strTemp.IsEmpty() )
            {
                //nTemp = _tcstol( strTemp, NULL, 16 );
                _tcscanf_s( strTemp, "%x", &nTemp );
                if ( bReadSeed )
                {
                    nTemp = nTemp ^ ApPassword[ (nIndex++ + nSeed + 1) % nPwLen ];
                    AStream->WriteWChar( nTemp );
                }
                else
                {
                    bReadSeed = true;
                    nSeed = nTemp;
                    nIndex = 0;
                }
            }
        }
        else
        {
            i++;
        }
    }
}

CStringA EncryptStringA( const char *ApText, const int &ATextLen, const char *ApPassword )
{
    int nPwLen = (int)strlen( ApPassword );
    if ( 0 == nPwLen )
    {
        return "";
    }
    srand( (unsigned)time(NULL) );

    CStringA strResult;
    int nTextLen = ATextLen;
    int nSeed = 22157;//rand();
    strResult = IntToHexA( nSeed );
    int nTemp;
    for (int i = 0; i < nTextLen; i++)
    {
        nTemp = ApText[i] ^ ApPassword[ (i + nSeed + 1) % nPwLen ];
        strResult += IntToHexA( nTemp );
    }
    return strResult;
}

void DecryptStringA( const char *ApEncrptText, const char *ApPassword, CxdStreamBasic *AStream )
{
    int nPwLen = (int)strlen( ApPassword );
    if ( 0 == nPwLen )
    {
        return;
    }
    int nTextLen = (int)strlen(ApEncrptText);
    int i(0), j, nSeed, nIndex;
    int nTemp;
    bool bReadSeed( false );
    CStringA strTemp;
    while ( i < nTextLen )
    {
        if ( '%' == ApEncrptText[i] )
        {
            j = i + 1;
            while ( j < nTextLen )
            {
                if ( '%' == ApEncrptText[j++] )
                {
                    --j;
                    break;
                }
            }
            strTemp.Empty();
            while ( ++i < j )
            {
                strTemp.AppendChar( ApEncrptText[i] );
            }
            if ( !strTemp.IsEmpty() )
            {
                //nTemp = strtol( strTemp, NULL, 16 );
                sscanf_s( strTemp, "%x", &nTemp );
                if ( bReadSeed )
                {
                    nTemp = nTemp ^ ApPassword[ (nIndex++ + nSeed + 1) % nPwLen ];
                    AStream->WriteChar( nTemp );
                }
                else
                {
                    bReadSeed = true;
                    nSeed = nTemp;
                    nIndex = 0;
                }
            }
        }
        else
        {
            i++;
        }
    }
}
