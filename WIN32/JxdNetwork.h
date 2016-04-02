#pragma once

class CxdHttp;
#include <afxinet.h>
#include "JxdDataStream.h"

struct TUrlCookiesParam
{
    CString FURL;
    CString FCookies;
    void *FpData;
};

struct TURLInfo
{
    INTERNET_SCHEME FScheme;
    CString FHost;
    CString FDomain;
    CString FUserName;
    CString FPassword;
    CString FUrlPath;
    CString FExtraInfo;
    WORD    FPort;
    CString FUrlHead;
};

//����ָ��URL��ָ���ļ��У��ⲿ����ȷ��·������
bool DownFile(const TCHAR *ApSrcURL, const TCHAR *ApSaveFileName);

CString CombineUrl(const TCHAR *ApBaseURL, const TCHAR *ApCalcURL);
CString GetCacheEntryInfo(LPCTSTR lpszUrl);

CStringA AutoJumpHttp(CxdHttp *ApHttp, const TCHAR *ApBaseURL);
bool     AutoJumpHttp(CxdHttp *ApHttp, const TCHAR *ApBaseURL, CxdStreamBasic* ApStream);

bool    UrlParse(const CString &AURL, TURLInfo &AInfo, DWORD AdwFlags = 0);