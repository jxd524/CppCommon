#include "StdAfx.h"

#include "JxdNetwork.h"
#include "JxdHttp.h"
#include "JxdMacro.h"
#include "JxdDebugInfo.h"

#include <WinInet.h>
#include <UrlMon.h>
//#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Wininet.lib")

bool DownFile( const TCHAR *ApSrcURL, const TCHAR *ApSaveFileName )
{
    try
    {
        DeleteUrlCacheEntry( ApSrcURL ); 
        HRESULT hr = URLDownloadToFile( NULL, ApSrcURL, ApSaveFileName, 0, NULL );
        if ( FAILED(hr) )
        {
            CxdFileStream f( ApSaveFileName, true );
            CxdHttp http;
            http.Get( ApSrcURL, &f );
            if ( (http.GetResponseID() == 200) && (f.GetFileSize() > 0) )
            {
                return true;
            }
            return false;
        }
        return SUCCEEDED( hr );
    }
    catch(...)
    {
    	return false;
    }
}

CString GetCacheEntryInfo(LPCTSTR lpszUrl)
{
    CString strPath;
    DWORD dwEntrySize=0;
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry;

    if (!GetUrlCacheEntryInfo(lpszUrl,NULL,&dwEntrySize))
    {
        if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
        {
            TRACE("GetUrlCacheEntryInfo Error:%d\n",GetLastError());
            TRACE("本地没有指定网页的缓存\n");
            return _T("");
        }
        else
            lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwEntrySize];
    }
    else
        return _T(""); // should not be successful w/ NULL buffer
    // and 0 size

    if (!GetUrlCacheEntryInfo(lpszUrl,lpCacheEntry,&dwEntrySize))
    {
        TRACE("GetUrlCacheEntryInfo Error:%d\n",GetLastError());
        TRACE("本地没有指定网页的缓存\n");
        return _T("");
    }
    else
    {
        strPath = lpCacheEntry->lpszLocalFileName;
    }
    delete []lpCacheEntry;
    return strPath;
}

CString CombineUrl( const TCHAR *ApBaseURL, const TCHAR *ApCalcURL )
{
    TCHAR buf[1024 * 64] = {0};
    DWORD len = 1024 * 64;
    InternetCombineUrl( ApBaseURL, ApCalcURL, buf, &len, 0);
    return CString( buf );
}

CStringA AutoJumpHttp( CxdHttp *ApHttp, const TCHAR *ApBaseURL)
{
    if ( !Assigned(ApHttp) )
    {
        return "";
    }
    CString strURL( ApBaseURL ); 
    CStringA strHTML;
    while ( true )
    {
        strHTML = ApHttp->Get( strURL );
        if ( ApHttp->GetResponseID() / 100 != 3 ) break;
		
        strURL = CombineUrl( strURL, ApHttp->GetLocation() );
        if ( strURL.IsEmpty() ) break;
    }
    return strHTML;
}

bool AutoJumpHttp( CxdHttp *ApHttp, const TCHAR *ApBaseURL, CxdStreamBasic* ApStream )
{
    if ( !Assigned(ApHttp) ) return false;

    CString strURL( ApBaseURL ); 
    while ( true )
    {
        ApHttp->Get( strURL, ApStream );
        if ( ApHttp->GetResponseID() / 100 != 3 ) break;

        strURL = CombineUrl( strURL, ApHttp->GetLocation() );
        if ( strURL.IsEmpty() ) break;
    }
    return ApStream->GetSize() > 0;
}

bool UrlParse( const CString &AURL, TURLInfo &AInfo, DWORD AdwFlags )
{
    static const TCHAR* const CtHttpHeadInfo = TEXT("http://");
    static const int CtHttpHeadInfoLength = _tcslen(CtHttpHeadInfo);

    if ( AURL.GetLength() < CtHttpHeadInfoLength )
    {
        return FALSE;
    }
    CString strURL( AURL );
    CString strTemp( strURL.Mid(0, CtHttpHeadInfoLength) );
    if ( strTemp.CompareNoCase(CtHttpHeadInfo) != 0 )
    {
        strURL = CtHttpHeadInfo + strURL;
    }

    TCHAR cScheme[INTERNET_MAX_SCHEME_LENGTH + 1] = {0},
        cHostName[INTERNET_MAX_HOST_NAME_LENGTH + 1] = {0},
        cUserName[INTERNET_MAX_USER_NAME_LENGTH + 1] = {0},
        cPassword[INTERNET_MAX_PASSWORD_LENGTH + 1] = {0},
        cUrlPath[INTERNET_MAX_URL_LENGTH + 1] = {0},
        cExtraInfo[INTERNET_MAX_URL_LENGTH + 1] = {0};

    URL_COMPONENTS c = {0};
    c.dwStructSize = sizeof(URL_COMPONENTS);
    c.dwExtraInfoLength = INTERNET_MAX_URL_LENGTH;
    c.lpszExtraInfo = cExtraInfo;
    c.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;
    c.lpszHostName = cHostName;
    c.dwPasswordLength = INTERNET_MAX_PASSWORD_LENGTH;
    c.lpszPassword = cPassword;
    c.dwSchemeLength = INTERNET_MAX_URL_LENGTH;
    c.lpszScheme = cScheme;
    c.dwUrlPathLength = INTERNET_MAX_URL_LENGTH;
    c.lpszUrlPath = cUrlPath;
    c.dwUserNameLength = INTERNET_MAX_USER_NAME_LENGTH;
    c.lpszUserName = cUserName;

    BOOL bOK = InternetCrackUrl(strURL, strURL.GetLength(), AdwFlags, &c);
    if ( bOK )
    {
        AInfo.FScheme = c.nScheme;
        AInfo.FHost = c.lpszHostName;
        AInfo.FUserName = c.lpszUserName;
        AInfo.FPassword = c.lpszPassword;
        AInfo.FUrlPath = c.lpszUrlPath;
        AInfo.FExtraInfo = c.lpszExtraInfo;
        AInfo.FPort = c.nPort;
        AInfo.FDomain = GetUrlDomain(AInfo.FHost).c_str();
        switch ( AInfo.FScheme )
        {
        case INTERNET_SCHEME_FTP:
            AInfo.FUrlHead = TEXT("ftp");
            break;
        case INTERNET_SCHEME_HTTPS:
            AInfo.FUrlHead = TEXT("https");
            break;
        case INTERNET_SCHEME_MAILTO:
            AInfo.FUrlHead = TEXT("mailto");
            break;
        default:
            AInfo.FUrlHead = TEXT("http");
        }
    }
    return TRUE == bOK;
}