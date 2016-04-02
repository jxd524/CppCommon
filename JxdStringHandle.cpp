#include "stdafx.h"
#include <algorithm>
#include <time.h>

#ifdef linux
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#endif

#include "JxdStringHandle.h"
#include "JxdMacro.h"

#define MaxFormatBufferLen 1024 * 4

/*常量定义*/
static const char * const CtVisibleLetterA = 
    "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const wchar_t * const CtVisibleLetterW = 
    L"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const char * const CtMainDomainA = 
    "ac,ah,biz,bj,cc,com,cq,edu,fj,gd,gov,gs,gx,gz,ha,hb,he,hi,hk,hl,hn,info,io,jl,js,jx,ln,mo,mobi,net,\
    nm,nx,org,qh,sc,sd,sh,sn,sx,tj,tm,travel,tv,tw,ws,xj,xz,yn,zj,";
static const wchar_t * const CtMainDomainW = 
    L"ac,ah,biz,bj,cc,com,cq,edu,fj,gd,gov,gs,gx,gz,ha,hb,he,hi,hk,hl,hn,info,io,jl,js,jx,ln,mo,mobi,net,\
    nm,nx,org,qh,sc,sd,sh,sn,sx,tj,tm,travel,tv,tw,ws,xj,xz,yn,zj,";

/*==============Pos==============*/
#define _PosImpl(FuncName, ParamType, lenFunc, cmpFunc, cmpWith0) \
    int FuncName(const ParamType* ApText, const ParamType* ApSubText, int AFindPos ) {  \
        if ( ApSubText == NULL ) return -1;                                             \
        int nSubLen = (int)lenFunc(ApSubText);                                          \
        if ( nSubLen <= 0 ) return -1;                                                  \
        int nTextLen = (int)lenFunc(ApText);                                            \
        if ( AFindPos < 0 ) AFindPos = 0;                                               \
        if ( AFindPos + nSubLen > nTextLen ) return -1;                                 \
        nTextLen -= nSubLen;                                                            \
        int i = (AFindPos >= 0) && (AFindPos <= nTextLen) ? AFindPos : 0;               \
        for ( ; i <= nTextLen; i++ ) {                                                  \
            if ( 0 cmpWith0 cmpFunc(ApSubText, ApText + i, nSubLen) ) return i;         \
        }                                                                               \
        return -1;                                                                      \
    }
_PosImpl( Pos, char, strlen, _strnicmp, == );
_PosImpl( Pos, wchar_t, wcslen, _wcsnicmp, == );
_PosImpl( PosCase, char, strlen, strncmp, == );
_PosImpl( PosCase, wchar_t, wcslen, wcsncmp, == );
_PosImpl( PosNotOf, char, strlen, _strnicmp, != );
_PosImpl( PosNotOf, wchar_t, wcslen, _wcsnicmp, != );
_PosImpl( PosCaseNotOf, char, strlen, strncmp, != );
_PosImpl( PosCaseNotOf, wchar_t, wcslen, wcsncmp, != );

/*==============PosBack==============*/
#define _PosBackImpl(FuncName, ParamType, lenFunc, cmpFunc, cmpWith0)                   \
    int FuncName( const ParamType *ApText, const ParamType *ApSubText, int AFindPos ){  \
        if ( ApSubText == NULL ) return -1;                                             \
        int nSubLen = (int)lenFunc(ApSubText);                                          \
        if ( nSubLen <= 0 ) return -1;                                                  \
        int nTextLen = (int)lenFunc(ApText);                                            \
        if ( AFindPos <= 0 ) AFindPos = nTextLen;                                       \
        if ( AFindPos - nSubLen < 0 ) return -1;                                        \
        for ( int i = AFindPos; i >= nSubLen; i-- ) {                                   \
            if ( 0 cmpWith0 cmpFunc(ApSubText, ApText + i - nSubLen, nSubLen) )         \
                return i - nSubLen;                                                     \
        }                                                                               \
        return -1;                                                                      \
    }
_PosBackImpl( PosBack, char, strlen, _strnicmp, == );
_PosBackImpl( PosBack, wchar_t, wcslen, _wcsnicmp, == );
_PosBackImpl( PosBackCase, char, strlen, strncmp, == );
_PosBackImpl( PosBackCase, wchar_t, wcslen, wcsncmp, == );
_PosBackImpl( PosBackNotOf, char, strlen, _strnicmp, != );
_PosBackImpl( PosBackNotOf, wchar_t, wcslen, _wcsnicmp, != );
_PosBackImpl( PosBackCaseNotOf, char, strlen, strncmp, != );
_PosBackImpl( PosBackCaseNotOf, wchar_t, wcslen, wcsncmp, != );

/*==============SearchMatchInfo==============*/
#define _SearchMatchInfoImpl( funcName, paramType, lenFunc, PosFunc )                                           \
    int funcName(const paramType* ApText, const paramType* ApSubText, int ABeginPos,                            \
                 TxdMatchInfo &AInfo, int AMaxMatchCount) {                                                     \
        int nCount = 0; int nPos;                                                                               \
        if ( AMaxMatchCount <= 0 ) AMaxMatchCount = MAXINT;                                                     \
        int nSubTextLen = lenFunc(ApSubText);                                                                   \
        while ( (nPos = PosFunc(ApText, ApSubText, ABeginPos)) != -1 ){                                         \
            if ( AMaxMatchCount-- <= 0 ) break;                                                                 \
            if ( ++AInfo.CurCount >= AInfo.MaxCount ){                                                          \
                AInfo.MaxCount = AInfo.MaxCount > 0 ? AInfo.MaxCount + AInfo.MaxCount : 5;                      \
                int *pNewItems = new int[AInfo.MaxCount];                                                       \
                if ( AInfo.MatchInfo != NULL && AInfo.CurCount > 0 ) {                                          \
                    memcpy( pNewItems, AInfo.MatchInfo, (AInfo.CurCount - 1) * sizeof(int) );                   \
                    delete AInfo.MatchInfo;                                                                     \
                }                                                                                               \
                AInfo.MatchInfo = pNewItems;                                                                    \
            }                                                                                                   \
            AInfo.MatchInfo[AInfo.CurCount - 1] = nPos;                                                         \
            ABeginPos = nPos + nSubTextLen;                                                                     \
            nCount++;                                                                                           \
        }                                                                                                       \
        return nCount;                                                                                          \
    }
_SearchMatchInfoImpl(SearchMatchInfo, char, strlen, Pos );
_SearchMatchInfoImpl(SearchMatchInfo, wchar_t, wcslen, Pos );
_SearchMatchInfoImpl(SearchMatchInfoCase, char, strlen, PosCase );
_SearchMatchInfoImpl(SearchMatchInfoCase, wchar_t, wcslen, PosCase );


/*==============ReplaceString==============*/
#define _ReplaceStringImpl(funcName, paramType, vType, lenFunc, matchFunc )                                                 \
    int funcName(const paramType* ApText, const paramType* ApOldSubText, const paramType* ApNewSubText,                     \
                 vType& ANewString, int ABeginPos, int AReplaceCount){                                                      \
        TxdMatchInfo info = {0};                                                                                            \
        matchFunc( ApText, ApOldSubText, ABeginPos, info, AReplaceCount );                                                  \
        if ( info.CurCount > 0 ){                                                                                           \
            int nOldTextLen = lenFunc( ApText );                                                                            \
            int nOldSubTextLen = lenFunc( ApOldSubText );                                                                   \
            int nNewSubTextLen = lenFunc( ApNewSubText );                                                                   \
            int nTextPos = 0, nCpyLen;                                                                                      \
            for ( int i = 0; i < info.CurCount; i++ ){                                                                      \
                nCpyLen = info.MatchInfo[i] - nTextPos;                                                                     \
                if ( nCpyLen > 0 ){                                                                                         \
                    ANewString.append( ApText + nTextPos, nCpyLen );                                                        \
                    nTextPos += nCpyLen;                                                                                    \
                }                                                                                                           \
                ANewString.append( ApNewSubText, nNewSubTextLen );                                                          \
                nTextPos += nOldSubTextLen;                                                                                 \
            }                                                                                                               \
            nCpyLen = nOldTextLen - nTextPos;                                                                               \
            if ( nCpyLen > 0 ) {                                                                                            \
                ANewString.append( ApText + nTextPos, nOldTextLen - nTextPos );                                             \
            }                                                                                                               \
        } else {                                                                                                            \
            ANewString = ApText;                                                                                            \
        }                                                                                                                   \
        int nResult = info.CurCount;                                                                                        \
        SafeDeleteNewBuffer( info.MatchInfo );                                                                              \
        return nResult;                                                                                                     \
    }

_ReplaceStringImpl(ReplaceString, char, string, strlen, SearchMatchInfo );
_ReplaceStringImpl(ReplaceString, wchar_t, wstring, wcslen, SearchMatchInfo );
_ReplaceStringImpl(ReplaceStringCase, char, string, strlen, SearchMatchInfo );
_ReplaceStringImpl(ReplaceStringCase, wchar_t, wstring, wcslen, SearchMatchInfo );

#define _ReplaceStringExImpl( funcName, paramType, vType )                                                  \
    vType funcName(const paramType* ApText, const paramType* ApOldSubText, const paramType* ApNewSubText,   \
                   int ABeginPos, int AReplaceCount){                                                       \
        vType s;                                                                                            \
        funcName( ApText, ApOldSubText, ApNewSubText, s, ABeginPos, AReplaceCount );                        \
        return s;                                                                                           \
    }

_ReplaceStringExImpl( ReplaceString, char, string);
_ReplaceStringExImpl( ReplaceString, wchar_t, wstring);
_ReplaceStringExImpl( ReplaceStringCase, char, string);
_ReplaceStringExImpl( ReplaceStringCase, wchar_t, wstring);

/*==============GetTheString==============*/
#define _GetTheStringImpl(funcName, paramType, vType, lenFunc, pos1, pos2)                                      \
    int funcName(const paramType* ApText, const paramType* ApForward, const paramType* ApBack,                  \
                 vType& ANewString, TxdThePos AWhenBackNotFind, int ABeginPos){                                 \
        vType strResult;                                                                                        \
        int nPos1 = pos1( ApText, ApForward, ABeginPos );                                                       \
        if ( nPos1 < 0 ) return -1;                                                                             \
        int nForwardLen = lenFunc( ApForward );                                                                 \
        int nPos2 = pos2( ApText, ApBack, nPos1 + nForwardLen );                                                \
        if ( -1 == nPos2 ){                                                                                     \
            switch ( AWhenBackNotFind ){                                                                        \
                case tpForward:{         																		\
                    strResult.append( ApText, nPos1 );                                                          \
                    ANewString = strResult;                                                                     \
                    return nPos1 + nForwardLen;                                                                 \
                }                                                                                               \
                case tpBack:{                                                                                   \
                    nPos2 = nPos1 + nForwardLen;                                                                \
                    int nlen = lenFunc( ApText ) - nPos2;                                                       \
                    strResult.append( ApText + nPos2, nlen );                                                   \
                    ANewString = strResult;                                                                     \
                    return nPos1 + nForwardLen;                                                                 \
                }                                                                                               \
                default: return -1;                                                                             \
            }                                                                                                   \
        }                                                                                                       \
        int nPos = nPos1 + nForwardLen;                                                                         \
        int nlen = nPos2 - nPos;                                                                                \
        strResult.append( ApText + nPos, nlen );                                                                \
        ANewString = strResult;                                                                                 \
        return nPos + nlen;                                                                                     \
    }

_GetTheStringImpl(GetTheString, char, string, strlen, Pos, Pos);
_GetTheStringImpl(GetTheString, wchar_t, wstring, wcslen, Pos, Pos);
_GetTheStringImpl(GetTheStringCase, char, string, strlen, PosCase, PosCase);
_GetTheStringImpl(GetTheStringCase, wchar_t, wstring, wcslen, PosCase, PosCase);

_GetTheStringImpl(GetTheStringFromBack, char, string, strlen, PosBack, Pos);
_GetTheStringImpl(GetTheStringFromBack, wchar_t, wstring, wcslen, PosBack, Pos);
_GetTheStringImpl(GetTheStringFromBackCase, char, string, strlen, PosBackCase, PosCase);
_GetTheStringImpl(GetTheStringFromBackCase, wchar_t, wstring, wcslen, PosBackCase, PosCase);

#define _GetTheStringExImpl(funcName, paramType, vType) \
    vType funcName(const paramType* ApText, const paramType* ApForward, const paramType* ApBack, \
                   TxdThePos AWhenBackNotFind, int ABeginPos){\
        vType s; \
        funcName(ApText, ApForward, ApBack, s, AWhenBackNotFind, ABeginPos);\
        return s;\
    }
_GetTheStringExImpl( GetTheString, char, string );
_GetTheStringExImpl( GetTheString, wchar_t, wstring );
_GetTheStringExImpl( GetTheStringCase, char, string );
_GetTheStringExImpl( GetTheStringCase, wchar_t, wstring );

_GetTheStringExImpl( GetTheStringFromBack, char, string );
_GetTheStringExImpl( GetTheStringFromBack, wchar_t, wstring );
_GetTheStringExImpl( GetTheStringFromBackCase, char, string );
_GetTheStringExImpl( GetTheStringFromBackCase, wchar_t, wstring );

string Format( const char* AFrmText, ... )
{
    char buf[MaxFormatBufferLen] = {0};                   
    va_list arglist;                     
    va_start( arglist, AFrmText );              
    int n = vsprintf(buf, AFrmText, arglist);    
    va_end( arglist );                          
    buf[n > MaxFormatBufferLen ? MaxFormatBufferLen : n] = 0;           
    return string(buf);
}

wstring Format( const wchar_t* AFrmText, ... )
{
    wchar_t buf[MaxFormatBufferLen] = {0};                   
    va_list arglist;                     
    va_start( arglist, AFrmText );              
    int n = vswprintf(buf, MaxFormatBufferLen, AFrmText, arglist);    
    va_end( arglist );                          
    buf[n > MaxFormatBufferLen ? MaxFormatBufferLen : n] = 0;           
    return wstring(buf);
}

#define _GetRandomStringImpl(type, value)                           \
    static const type CtStringInfo( value );                        \
    int nLen = AOnlyNumber ? 10 : CtStringInfo.length();            \
    srand( static_cast<unsigned int>(time(NULL)) );                 \
    int nMax = (AMinCount < AMaxCount) ? AMaxCount : AMinCount;     \
    nMax = rand() % nMax;                                           \
    if ( nMax < AMinCount ) nMax = AMinCount;                       \
    type strResult;                                                 \
    for ( int i = nMax; i > 0; i-- ){                               \
        strResult.append( 1, CtStringInfo.at(rand() % nLen) );      \
    }                                                               \
    return strResult;

string GetRandomStringA( const int AMinCount /*= 10*/, const int AMaxCount /*= 30*/, const bool AOnlyNumber /*= false*/ )
{
    _GetRandomStringImpl( string, CtVisibleLetterA );
}

wstring GetRandomStringW( const int AMinCount /*= 10*/, const int AMaxCount /*= 30*/, const bool AOnlyNumber /*= false*/ )
{
    _GetRandomStringImpl( wstring, CtVisibleLetterW );
}

#define _ChangedCase( type, func ) \
    type strResult( s ); \
    transform( s.begin(), s.end(), strResult.begin(), func );\
    return strResult;

string LowerCase( const string &s )
{
    _ChangedCase( string, ::tolower );
}

wstring LowerCase( const wstring &s )
{
    _ChangedCase( wstring, ::tolower );
}

string UpperCase( const string &s )
{
    _ChangedCase( string, ::toupper );
}

wstring UpperCase( const wstring &s )
{
    _ChangedCase( wstring, ::toupper );
}

void TrimString( string &AText )
{
    if ( !AText.empty() ) 
    {
        AText.erase( 0,AText.find_first_not_of(" ") );
        AText.erase( AText.find_last_not_of(" ") + 1 );
    }
}

void TrimString( wstring &AText )
{
    if ( !AText.empty() ) 
    {
        AText.erase( 0,AText.find_first_not_of(L" ") );
        AText.erase( AText.find_last_not_of(L" ") + 1 );
    }
}

#define _ParseFormatStringImpl( lenFunc, matchFunc, type )              \
    TxdMatchInfo info = {0};                                            \
    int nCount = matchFunc( ApText, ApSign, 0, info, AParseCount );     \
    int nSingLen( lenFunc(ApSign) );                                    \
    int nPos = 0;                                                       \
    for ( int i = 0; i < nCount; i++ ){                                 \
        Alt.push_back( type(ApText + nPos, info.MatchInfo[i] - nPos) ); \
        nPos = info.MatchInfo[i] + nSingLen;                            \
    }                                                                   \
    SafeDeleteNewBuffer( info.MatchInfo );                              \
    if ( nPos < (int)lenFunc(ApText) ){                                 \
        nCount++;                                                       \
        Alt.push_back( type(ApText + nPos) );                           \
    }                                                                   \
    return nCount;

int ParseFormatString( const char *ApText, const char *ApSign, TStringAList &Alt, int AParseCount /*= -1*/ )
{
    _ParseFormatStringImpl( strlen, SearchMatchInfo, string );
}

int ParseFormatString( const wchar_t *ApText, const wchar_t *ApSign, TStringWList &Alt, int AParseCount /*= -1*/ )
{
    _ParseFormatStringImpl( wcslen, SearchMatchInfo, wstring );
}

int ParseFormatStringCase( const char *ApText, const char *ApSign, TStringAList &Alt, int AParseCount /*= -1*/ )
{
    _ParseFormatStringImpl( strlen, SearchMatchInfoCase, string );
}

int ParseFormatStringCase( const wchar_t *ApText, const wchar_t *ApSign, TStringWList &Alt, int AParseCount /*= -1*/ )
{
    _ParseFormatStringImpl( wcslen, SearchMatchInfoCase, wstring );
}

#define _ParseInfoImpl(type, parseFunc) \
    type lt;\
    if ( 2 == parseFunc(ApText, ApSign, lt, 1) ){\
        AInfo.First = lt.at(0);\
        AInfo.Second = lt.at(1);\
        return true;\
    }\
    return false;

bool ParseFormatString( const char *ApText, const char *ApSign, TxdParseInfoA &AInfo )
{
    _ParseInfoImpl( TStringAList, ParseFormatString );
}

bool ParseFormatString( const wchar_t *ApText, const wchar_t *ApSign, TxdParseInfoW &AInfo )
{
    _ParseInfoImpl( TStringWList, ParseFormatString );
}

bool ParseFormatStringCase( const char *ApText, const char *ApSign, TxdParseInfoA &AInfo )
{
    _ParseInfoImpl( TStringAList, ParseFormatStringCase );
}

bool ParseFormatStringCase( const wchar_t *ApText, const wchar_t *ApSign, TxdParseInfoW &AInfo )
{
    _ParseInfoImpl( TStringWList, ParseFormatStringCase );
}

#define _GetUrlDomainImpl(lenFunc, returnType, strListType, CtDomains )             \
    returnType str1( 2, '/' );                                                      \
    returnType str2( 1, '/' );                                                      \
    int nPos1 = Pos( ApURL, str1.c_str(), 0 );                                      \
    -1 == nPos1 ? nPos1 = 0 : nPos1 += 2;                                           \
    int nPos2 = Pos( ApURL, str2.c_str(), nPos1 );                                  \
    if ( -1 == nPos2 ) nPos2 = (int)lenFunc(ApURL);                                 \
    returnType strResult( ApURL + nPos1, nPos2 - nPos1 );                           \
    returnType strSign( 1, '.' );                                                   \
    strListType lt;                                                                 \
    ParseFormatString( strResult.c_str(), strSign.c_str(), lt );                    \
    int nMinCount(2);                                                               \
    strResult.clear();                                                              \
    for ( strListType::reverse_iterator it = lt.rbegin(); it != lt.rend(); it++ ){  \
        returnType strTemp = *it;                                                   \
        if ( !strResult.empty() ){                                                  \
            strResult.insert( strResult.begin(), 1, '.' );                          \
        }                                                                           \
        strResult = strTemp + strResult;                                            \
        --nMinCount;                                                                \
        if ( nMinCount == 0 ) {                                                     \
            strTemp.append( 1, ',' );                                               \
            if ( -1 == Pos(CtDomains, strTemp.c_str(), 0) ) {                       \
                break;                                                              \
            }                                                                       \
        } else if ( nMinCount < 0 ) {                                               \
            break;                                                                  \
        }                                                                           \
    }                                                                               \
    return strResult;

string GetUrlDomain( const char *ApURL )
{
    _GetUrlDomainImpl( strlen, string, TStringAList, CtMainDomainA );
}

wstring GetUrlDomain( const wchar_t *ApURL )
{
    _GetUrlDomainImpl( wcslen, wstring, TStringWList, CtMainDomainW );
}

#define _GetLineEndSignImpl(lenFunc, returnType)                \
    int nCount( lenFunc(ApText) );                              \
    returnType s;                                               \
    for ( int i = 0; i < nCount; i++ ){                         \
        if ('\r' == ApText[i]){                                 \
            if ( (i + 1 < nCount) && ('\n' == ApText[i + 1]) ){ \
                s.append( 1, '\r' );                            \
                s.append( 1, '\n' );                            \
                return s;                                       \
            } else {                                            \
                s.append( 1, '\r' );                            \
                return s;                                       \
            }                                                   \
        } else if ( '\n' == ApText[i] ) {                       \
            s.append( 1, '\n' );                                \
            return s;                                           \
        }                                                       \
    }                                                           \
    return s;

string GetLineEndSign( const char *ApText )
{
    _GetLineEndSignImpl( strlen, string );
}

wstring GetLineEndSign( const wchar_t *ApText )
{
    _GetLineEndSignImpl( wcslen, wstring );
}

//CompareString
#define _CompareStringImpl(FuncName, ParamType, lenFunc)\
    bool FuncName(ParamType AParam1, ParamType AParam2) {\
    return (lenFunc(AParam1) == lenFunc(AParam2)) && (Pos(AParam1, AParam2, 0) == 0);\
}
_CompareStringImpl(CompareString, const char*, strlen );
_CompareStringImpl(CompareString, const wchar_t*, wcslen );
_CompareStringImpl(CompareStringCase, const char*, strlen );
_CompareStringImpl(CompareStringCase, const wchar_t*, wcslen );

#ifdef linux

#define strnicmpImpl(type)\
    if(count){\
        int f=0, l=0;\
        do{\
            if ( ((f = (type)(*(first++))) >= 'A') && (f <= 'Z') )\
                f -= 'A' - 'a';\
            if ( ((l = (type)(*(last++))) >= 'A') && (l <= 'Z') )\
                l -= 'A' - 'a';\
        }\
        while ( --count && f && (f == l) );\
        return ( f - l );\
    }\
    return 0;
int _strnicmp( const char * first, const char * last, size_t count )
{
    strnicmpImpl( unsigned char );
}

int _wcsnicmp( const wchar_t * first, const wchar_t * last, size_t count )
{
    strnicmpImpl( unsigned wchar_t );
}

#endif

INT64 StrToInt64( const char* AStr )
{
    return atoi(AStr);
}

INT64 StrToInt64( const wchar_t* AStr )
{
    return wcstol(AStr, NULL, 10 );
}

string Int64ToStrA( const INT64 &AValue )
{
#ifdef WIN32
    return Format( "%I64d", AValue );
#endif
#ifdef linux
    return Format( "%lld", AValue );
#endif    
}

wstring Int64ToStrW( const INT64 &AValue )
{
#ifdef WIN32
    return Format( L"%I64d", AValue );
#endif
#ifdef linux
    return Format( L"%lld", AValue );
#endif
}