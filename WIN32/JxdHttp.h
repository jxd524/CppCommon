/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdHttp.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-11-19 14:43:10
	LastModify : 
	Description: 使用WIN API实现HTTP, 支持Cookie管理
**************************************************************************/
#pragma once

#include <afxinet.h>
#include <string>
#include "JxdDataStream.h"
#include "JxdStringHandle.h"
#include "JxdStringCoder.h"
#include "JxdTypedef.h"
#include "JxdEvent.h"

using namespace std;

template<typename TClassType, typename TParamType1, typename TParamType2> class CNotifyEventExT;
typedef CNotifyEventExT<CObject, const LPVOID&, const LPVOID&> CNotifyEventEx;

enum THttpMethod { emGet, emPost };
enum THttpEncode { heUtf8, heGb2313, heGzip, heGBK, heUnknow };

CString GetWebContenType(CString AFileExt);

struct TCookieInfo 
{
    CString FCookieName;
    CString FCookieValue;
    CString FDomain;
    CString FPath;
    CString FExpires; 
};

typedef vector<TCookieInfo*> TCookieInfoList;
typedef TCookieInfoList::iterator TCookieInfoListIT;

CString StreamToString(THttpEncode he, CxdMemoryHandle *ApStream);  

class CxdCookieManage
{
public:
    static void CheckURL(CString &AURL);
    static CString CheckDomainName(const TCHAR *ApURL);
    static CString GetSysCookies(const TCHAR *ApURL, bool IsHttponly = false);
    static CString GetIE6SysCookiePath(void);

    CxdCookieManage();
    ~CxdCookieManage();

    //IE缓存相关
    void    AddSysCookies(const TCHAR *ApURL); //系统保存的URL
    void    ClearSysCookies(const TCHAR *ApURL = NULL);
    void    SynToSystem(const TCHAR *ApDefaultDomain); //同步COOKIE到系统 默认将COOKIE保存 500 天

    //类似: SUS=SID-2624453585-1341385273-JA-x6q20-1f6f0f34f31686f9a02d67dd7d2c0327; expires=Wed, 11-Jul-2012 07:01:10 GMT; path=/; domain=.weibo.com
    bool    AddCookie(CString ACookie, const TCHAR *ApDomainName = NULL);
    //整个HTTP头 Set_Cookie   
    void    AddHttpHeader(CString AHttpHead, const CString &AURL);  
    //多个类似: USRHAWB=usrmdins211172; _s_tentry=-; Apache=818995716383.738.1352424625154;
    void    AddCookies(CString ACookies, const CString &AURL);

    //获取信息
    void    GetDomainList(TStringWList &AList);
    CString GetCookies(const TCHAR *ApURL = NULL); //
    //Test
    CString GetAllCookieInfo(void);

    //添加时间
    void AddExpiresToCookie(const TCHAR *ApExpires);

    //删除
    void    DeleteCookie(const TCHAR *ApCookieName, const TCHAR *ApDomainName = NULL); //删除
    void    Clear(void);
private:
    typedef IxdNotifyEventExT< TCookieInfo*, LPVOID > IxdLoopCookieInfo;

    TCookieInfoList m_ltCookies;
    TCookieInfo* FindCookie(const CString &ACookieName, const CString &ADomainName, bool bCreateOnNull, bool bDelete);
    void LoopFindCookieInfo(const TCHAR *ApURL, IxdLoopCookieInfo* ApIt, LPVOID ApParam);

    void DoBuildCookies(TCookieInfo* ApCookieInfo, LPVOID ApParam);
    void DoClearCookies(TCookieInfo* ApCookieInfo, LPVOID ApParam);
};

class CxdHttp
{
public:
    static void ReadHttpRawHeader(HINTERNET hHttp, CString &ARawText, bool AbResponse = true);

    CxdHttp( const CString &AUserAgent = TEXT("Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E)") );
    ~CxdHttp();
    //setting
    bool GetAutoRedirect(void) { return m_bAutoRedirect; }
    void SetAutoRedirect(bool bAuto) { m_bAutoRedirect = bAuto; }
    //setting timeout
    DWORD GetTimeoutConnect(void) { return m_dwTimeoutConnect; }
    void  SetTimeoutConnect(DWORD AValue) { m_dwTimeoutConnect = AValue; }
    DWORD GetTimeoutRecv(void) { return m_dwTimeoutRecv; }
    void  SetTimeoutRecv(DWORD AValue) { m_dwTimeoutRecv = AValue; }
    DWORD GetTimeoutSend(void) { return m_dwTimeoutSend; }
    void  SetTimeoutSend(DWORD AValue) { m_dwTimeoutSend = AValue; }
    //Accept
    void    SetHeadAccept(const CString &AText);
    CString GetHeadAccept(void);
    //Cookies
    CxdCookieManage* GetCookieManage(void) { return &m_CookieManage; }

	//Proxy
	void SetIsUseProxy(bool bUse) { m_bIsUseProxy = bUse; }
	bool GetIsUseProxy(void) { return m_bIsUseProxy; }
	void SetProxyAddress(const TCHAR *ApAddress) { m_strProxyAddress = ApAddress; }

    //Referer
    void    SetHeadReferer(const CString &AReferer);
    CString GetHeadReferer(void);
    //raw header peroperty
    void    SetRawHead(const CString &AName, const CString &AValue);
    CString GetRawHead(const CString &AName);
	//ContentType
	void SetContentType(const CString &AContentType);

    //Response info
    inline int GetResponseID(void) { return m_nResponseID; }
    inline CString GetContentType(void){ return m_strContentType; }
    inline THttpEncode ResponseType(void) { return m_heEnCode; }
    inline CString GetContentEncoding(void){ return m_strContentEncoding; }
    inline CString GetLocation(void){ return m_strLocation; }
    static THttpEncode CheckContentType(const CString &AContentType);

    //Last Request info
    inline CString GetLastRecvRawHeaders(void) { return m_strLastRecvRawHeaders; }
    inline CString GetLastResquestURL(void) { return m_strLastURL; }
    inline THttpMethod GetLastMethod(void) { return m_LastRequestMethod; }

    //CMD 当提供 ApRawRecvStream 对象时，则函数返回空字符
    CStringA Get(const CString &AURL, CxdStreamBasic *ApRawRecvStream = NULL);
    CStringA Post(const CString &AURL, CxdMemoryHandle *ApPostData, CxdStreamBasic *ApRawRecvStream = NULL);
    CStringA Post(const CString &AURL, const char *ApPostData, const int &Alen);
    CStringA Post(const CString &AURL, const CString &APostData);
    CStringA UploadFile(const CString &AURL, const CString &AFileName, const CString &AHeadName = TEXT("picture") ); 
protected:
    //ACmd: GET 或 POST
    bool    DoRequestCmd(THttpMethod AMethod, const CString &AURL, CxdStreamBasic *ApResponseStream, CxdMemoryHandle *ApRequestStream );
    CString StreamToStr(CxdMemoryHandle *ApStream);
private:
    CString m_strUserAgent;
    //服务器返回信息
    int m_nResponseID;
    CString m_strLastRecvRawHeaders;
    CString m_strContentType;
    CString m_strContentEncoding;
    CString m_strLocation;
    THttpEncode m_heEnCode;

    DWORD m_dwTimeoutConnect,
        m_dwTimeoutRecv,
        m_dwTimeoutSend;
    bool  m_bAutoRedirect;

	//Proxy
	bool m_bIsUseProxy;
	CString m_strProxyAddress;

    THttpMethod m_LastRequestMethod;
    CString m_strLastURL;

    CxdCookieManage m_CookieManage;

    TParseInfoWList m_ltRequestRawHeader;
    bool FindRawHead(const CString &AHeadName, TxdParseInfoW &AInfo);

    void HandleRawHeaders(void);
    void SetOpt(HINTERNET h, DWORD optLeave, DWORD optValue);
};