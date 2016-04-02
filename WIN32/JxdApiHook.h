/**************************************************************************
Copyright  : pp.cc 深圳指掌时代 珊瑚组
File       : JxdApiHook.h & .cpp 
Author     : Terry
Email      : jxd524@163.com
CreateTime : 2012-7-25 09:37
Description: CxdApiHookBasic, CxdWininetHook, CxdWinSockHook
**************************************************************************/
#pragma once

#include <WinInet.h>  
#include <Dbghelp.h>  
#include <DelayImp.h>  
#include <Psapi.h>  
#include <WinSock2.h>
#include <vector>
#include <iterator>

#pragma comment(lib, "WinInet.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Psapi.lib")

using namespace std;

class CxdApiHookBasic;
typedef vector<CxdApiHookBasic*> TxdApiHookBasicList;
typedef TxdApiHookBasicList::iterator TxdApiHookBasicListIT;

typedef struct _STRING {  
    USHORT  Length;  
    USHORT  MaximumLength;  
    PCHAR   Buffer;  
} ANSI_STRING, *PANSI_STRING;  

typedef struct _LSA_UNICODE_STRING {  
    USHORT Length;  
    USHORT MaximumLength;  
    PWSTR  Buffer;  
}LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef LONG NTSTATUS;  

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L) 
#endif

//Hook API 声明
#define _DefineApiHook(apiName, apiType, apiReturn, val_defines, vals) \
    public:\
        typedef apiReturn (apiType* T##apiName)(val_defines);\
    protected:\
        static T##apiName m_pFunc_##apiName;\
        static apiReturn apiType _##apiName(val_defines);\
        virtual apiReturn apiType new_##apiName(val_defines) { return m_pFunc_##apiName(vals); }

//Hook API 实现
#define _ImplementApiHook(className, apiName, apiType, apiReturn, val_defines, vals) \
    className::T##apiName className::m_pFunc_##apiName = (className::T##apiName)apiName;  \
    apiReturn apiType className::_##apiName(val_defines){ \
        if ( IsValidObject(SGpCurHook) ) return SGpCurHook->new_##apiName(vals); \
        return m_pFunc_##apiName(vals); }

//类支持的HOOK API 函数
#define _A "A"
#define _W "W"
#define ImplementHoldApiInfo(className, SGval) \
    const TApiHookInfo className::SGval[] = {
#define HoldApiItem(className, apiName) \
    { #apiName, className::m_pFunc_##apiName, &className::_##apiName }
#define HoldApiItemAW(className, apiName) \
    { #apiName##_A, className::m_pFunc_##apiName##A, &className::_##apiName##A },\
    { #apiName##_W, className::m_pFunc_##apiName##W, &className::_##apiName##W }
#define EndHoldApiInfo };

#define DefineApiHook(apiName, apiType, apiReturn)\
    _DefineApiHook(apiName, apiType, apiReturn, apiName##Define, apiName##Val);

#define ImplementApiHook(className, apiName, apiType, apiReturn)\
    _ImplementApiHook(className, apiName, apiType, apiReturn, apiName##Define, apiName##Val )

//A与W类型函数
#define DefineApiHookAW(apiName, apiType, apiReturn)\
    _DefineApiHook(apiName##A, apiType, apiReturn, apiName##ADefine, apiName##Val);\
    _DefineApiHook(apiName##W, apiType, apiReturn, apiName##WDefine, apiName##Val);

#define ImplementApiHookAW(className, apiName, apiType, apiReturn)\
    _ImplementApiHook(className, apiName##A, apiType, apiReturn, apiName##A##Define, apiName##Val )\
    _ImplementApiHook(className, apiName##W, apiType, apiReturn, apiName##W##Define, apiName##Val )

//Hook info helper
struct TApiHookInfo 
{
    CStringA ApiName; 
    LPVOID   ApiSrcAddr; //HOOK 前Api 的地址
    LPVOID   ApiNewAddr; //Hook 后要替换的新地址
};
typedef vector<const TApiHookInfo*> TApiHookInfoList;
typedef TApiHookInfoList::const_iterator TApiHookInfoListCIT;
bool IsExsitsHookApi(const TApiHookInfoList *ApList, const TApiHookInfo *ApItem);
const TApiHookInfo* FindHookItem(TApiHookInfoList *ApList, const char *ApApiName, bool AbDelItem);
const TApiHookInfo* FindHookItem(TApiHookInfoList *ApList, const LPVOID ApApiAddress, bool AbDelItem);
const TApiHookInfo* FindHookItem(const TApiHookInfo ApBasic[], const int &ACount, const char *ApApiName);
const TApiHookInfo* FindHookItem(const TApiHookInfo ApBasic[], const int &ACount, const LPVOID ApApiAddress);

//Api define
typedef NTSTATUS (WINAPI* TLdrGetProcedureAddress)(IN HMODULE ModuleHandle, IN PANSI_STRING FunctionName OPTIONAL, IN WORD Oridinal OPTIONAL, OUT PVOID *FunctionAddress );  
typedef NTSTATUS (WINAPI* TLdrLoadDll)(IN PWCHAR PathToFile OPTIONAL, IN ULONG Flags OPTIONAL, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle);  

class CxdApiHookBasic: public CObject
{
public:
    CxdApiHookBasic(HANDLE AProcess = 0);
    virtual ~CxdApiHookBasic(void);

    HANDLE GetHookProcess(void) { return m_hProcess; }
    bool GetActiveHook(void) { return m_bActiveHook; }
    bool SetActiveHook(bool bActive);

    //需要HOOK的API
    bool AddHookApi(const char * AApiName);
    bool AddHookApi(LPVOID ApApiAddress);
    void DelHookApi(const char* AApiName);
    void DelHookApi(LPVOID ApApiAddress);
private:
    HANDLE m_hProcess;
    bool m_bActiveHook;

    //当前支持的HOOK函数
    int m_nHoldApiCount;
    const TApiHookInfo *m_pHoldApiInfos;

    //需要被HOOK的API信息
    TApiHookInfoList m_CurHookApiList;

    CStringA m_strDllNameA;
    CString  m_strDllNameT;
    void DoAddHookInfo(const TApiHookInfo *ApInfo);
protected:    
    void InitApiHookParam(const TApiHookInfo AAllHoldApiInfo[], int ACount, const char *ApDllNameA, const TCHAR *ApDllNameT);
    inline bool IsInitApiHookParam(void) { return m_pHoldApiInfos != NULL; }
    static inline bool IsValidObject(CxdApiHookBasic *pObject) { return pObject != NULL && pObject->GetActiveHook(); }

    virtual bool DoActiveHook(HANDLE hModCaller) { return true; }
    virtual bool DoUnActiveHook(void) { return true; };
    virtual PVOID FindHookFunctionAddress(const CString &AModName, const CStringA &AFunctionName, const LPVOID AFunctionAddress);

    bool  ActiveHook(HANDLE hModCaller);
    void  UnActiveHook(void);    
    
    //HOOK API
    //LoadDll
    static TLdrLoadDll m_psLdrLoadDll;  
    static NTSTATUS WINAPI _LdrLoadDll(IN PWCHAR PathToFile OPTIONAL, IN ULONG Flags OPTIONAL, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle);  
    //GetProcedureAddress
    static TLdrGetProcedureAddress m_psLdrGetProcedureAddress;      
    static NTSTATUS WINAPI _LdrGetProcedureAddress(IN HMODULE ModuleHandle, IN PANSI_STRING FunctionName OPTIONAL, IN WORD Oridinal OPTIONAL, OUT PVOID *FunctionAddress );   

    // process the IAT 参考 WinintHook 类 及 Window核心编程 22 章: 22.9.1 通过改写代码来挂接API
    //替换指定进程中所有模块的函数地址
    //hProcess: 0 表示当前进程
    //lpszDllName: pfnCurrent所在模块名称
    //pfnCurrent: 需要替换的函数地址
    //pfnNew: 新的函数地址
    static void ReplaceAllModuleIATEntry(HANDLE hProcess, LPCSTR lpszDllName, LPVOID pfnCurrent, LPVOID pfnNew);
    //替换指定一个模块中的函数地址: ImageImport
    static BOOL ReplaceIATEntryInImageImportTable(HANDLE hProcess, HANDLE hModCaller, LPCSTR lpszDllName, LPVOID pfnCurrent, LPVOID pfnNew);
    //替换指定一个模块中的函数地址: DelayImageImport
    static BOOL ReplaceIATEntryInDelayImageImportTable(HANDLE hProcess, HANDLE hModCaller, LPCSTR lpszDllName, LPVOID pfnCurrent, LPVOID pfnNew);

    static void RelaceAllModuleIATEnterByList(LPCSTR lpszDllName, const HANDLE &hProcess, TApiHookInfoList &AList);
    static void RestoreAllModuleIATEnterByList(LPCSTR lpszDllName, const HANDLE &hProcess, TApiHookInfoList &AList);
    static void RelaceIATEnterByList(const HANDLE &hProcess, const HANDLE &hModCaller, LPCSTR lpszDllName, TApiHookInfoList &AList);
    DECLARE_DYNAMIC(CxdApiHookBasic)
};

//Winint API Hook
//HttpSendRequest
#define HttpSendRequestADefine HINTERNET hRequest, LPCSTR  lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength
#define HttpSendRequestWDefine HINTERNET hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength
#define HttpSendRequestVal hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength

//HttpSendRequestEx
#define HttpSendRequestExADefine __in HINTERNET hRequest, __in_opt LPINTERNET_BUFFERSA lpBuffersIn, __out_opt LPINTERNET_BUFFERSA lpBuffersOut, __in DWORD dwFlags, __in_opt DWORD_PTR dwContext
#define HttpSendRequestExWDefine __in HINTERNET hRequest, __in_opt LPINTERNET_BUFFERSW lpBuffersIn, __out_opt LPINTERNET_BUFFERSW lpBuffersOut, __in DWORD dwFlags, __in_opt DWORD_PTR dwContext
#define HttpSendRequestExVal hRequest, lpBuffersIn, lpBuffersOut, dwFlags, dwContext

//HttpEndRequest
#define HttpEndRequestADefine __in HINTERNET hRequest, __out_opt LPINTERNET_BUFFERSA lpBuffersOut, __in DWORD dwFlags, __in_opt DWORD_PTR dwContext
#define HttpEndRequestWDefine __in HINTERNET hRequest, __out_opt LPINTERNET_BUFFERSW lpBuffersOut, __in DWORD dwFlags, __in_opt DWORD_PTR dwContext
#define HttpEndRequestVal hRequest, lpBuffersOut, dwFlags, dwContext

//HttpOpenRequest
#define HttpHttpOpenRequestADefine __in HINTERNET hConnect,__in_opt LPCSTR lpszVerb, __in_opt LPCSTR lpszObjectName, __in_opt LPCSTR lpszVersion, __in_opt LPCSTR lpszReferrer, __in_z_opt LPCSTR FAR * lplpszAcceptTypes, __in DWORD dwFlags, __in_opt DWORD_PTR dwContext
#define HttpHttpOpenRequestWDefine __in HINTERNET hConnect,__in_opt LPCWSTR lpszVerb,__in_opt LPCWSTR lpszObjectName,__in_opt LPCWSTR lpszVersion,__in_opt LPCWSTR lpszReferrer,__in_z_opt LPCWSTR FAR * lplpszAcceptTypes,__in DWORD dwFlags, __in_opt DWORD_PTR dwContext
#define HttpHttpOpenRequestVal hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext

//InternetConnect
#define InternetConnectADefine __in HINTERNET hInternet,__in LPCSTR lpszServerName,__in INTERNET_PORT nServerPort,__in_opt LPCSTR lpszUserName,__in_opt LPCSTR lpszPassword,__in DWORD dwService,__in DWORD dwFlags,__in_opt DWORD_PTR dwContext
#define InternetConnectWDefine __in HINTERNET hInternet,__in LPCWSTR lpszServerName,__in INTERNET_PORT nServerPort,__in_opt LPCWSTR lpszUserName,__in_opt LPCWSTR lpszPassword,__in DWORD dwService,__in DWORD dwFlags,__in_opt DWORD_PTR dwContext
#define InternetConnectVal hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext

//HttpAddRequestHeaders
#define HttpAddRequestHeadersADefine __in HINTERNET hRequest,__in_ecount(dwHeadersLength) LPCSTR lpszHeaders,__in DWORD dwHeadersLength,__in DWORD dwModifiers
#define HttpAddRequestHeadersWDefine __in HINTERNET hRequest,__in_ecount(dwHeadersLength) LPCWSTR lpszHeaders,__in DWORD dwHeadersLength,__in DWORD dwModifiers
#define HttpAddRequestHeadersVal hRequest, lpszHeaders, dwHeadersLength, dwModifiers

//InternetOpenUrl
#define InternetOpenUrlADefine HINTERNET hInternet,LPCSTR lpszUrl, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext
#define InternetOpenUrlWDefine HINTERNET hInternet,LPCWSTR lpszUrl,LPCWSTR lpszHeaders,DWORD dwHeadersLength,DWORD dwFlags,DWORD_PTR dwContext
#define InternetOpenUrlVal hInternet, lpszUrl, lpszHeaders, dwHeadersLength, dwFlags, dwContext

//InternetOpen
#define InternetOpenADefine LPCSTR lpszAgent, DWORD dwAccessType, LPCSTR lpszProxy, LPCSTR lpszProxyBypass, DWORD dwFlags
#define InternetOpenWDefine LPCWSTR lpszAgent, DWORD dwAccessType, LPCWSTR lpszProxy, LPCWSTR lpszProxyBypass, DWORD dwFlags
#define InternetOpenVal lpszAgent, dwAccessType, lpszProxy, lpszProxyBypass, dwFlags

//InternetCloseHandle
#define InternetCloseHandleDefine __in HINTERNET hInternet
#define InternetCloseHandleVal hInternet

//InternetSetCookie
#define InternetSetCookieADefine LPCSTR lpszUrl, LPCSTR lpszCookieName, LPCSTR lpszCookieData
#define InternetSetCookieWDefine LPCWSTR lpszUrl, LPCWSTR lpszCookieName, LPCWSTR lpszCookieData
#define InternetSetCookieVal lpszUrl, lpszCookieName, lpszCookieData

//InternetSetCookieEx
#define InternetSetCookieExADefine LPCSTR lpszUrl, LPCSTR lpszCookieName, LPCSTR lpszCookieData, DWORD dwFlags, DWORD_PTR dwReserved
#define InternetSetCookieExWDefine LPCWSTR lpszUrl, LPCWSTR lpszCookieName, LPCWSTR lpszCookieData, DWORD dwFlags, DWORD_PTR dwReserved
#define InternetSetCookieExVal lpszUrl, lpszCookieName, lpszCookieData, dwFlags, dwReserved

//InternetGetCookie
#define InternetGetCookieADefine __in LPCSTR lpszUrl, __in_opt LPCSTR lpszCookieName, __out_ecount_opt(*lpdwSize) LPSTR lpszCookieData, __inout LPDWORD lpdwSize
#define InternetGetCookieWDefine __in LPCWSTR lpszUrl, __in_opt LPCWSTR lpszCookieName, __out_ecount_opt(*lpdwSize) LPWSTR lpszCookieData, __inout LPDWORD lpdwSize
#define InternetGetCookieVal lpszUrl, lpszCookieName, lpszCookieData, lpdwSize

//InternetGetCookieEx
#define InternetGetCookieExADefine __in LPCSTR lpszUrl, __in_opt LPCSTR lpszCookieName, __in_ecount_opt(*lpdwSize) LPSTR lpszCookieData, __inout LPDWORD lpdwSize, __in DWORD dwFlags, __reserved LPVOID lpReserved
#define InternetGetCookieExWDefine __in LPCWSTR lpszUrl, __in_opt LPCWSTR lpszCookieName, __in_ecount_opt(*lpdwSize) LPWSTR lpszCookieData, __inout LPDWORD lpdwSize, __in DWORD dwFlags, __reserved LPVOID lpReserved
#define InternetGetCookieExVal lpszUrl, lpszCookieName, lpszCookieData, lpdwSize, dwFlags, lpReserved

//HttpQueryInfo
#define HttpQueryInfoADefine __in HINTERNET hRequest, __in DWORD dwInfoLevel,__inout_bcount_opt(*lpdwBufferLength) __out_data_source(NETWORK) LPVOID lpBuffer, __inout LPDWORD lpdwBufferLength, __inout_opt LPDWORD lpdwIndex
#define HttpQueryInfoWDefine __in HINTERNET hRequest, __in DWORD dwInfoLevel, __inout_bcount_opt(*lpdwBufferLength) __out_data_source(NETWORK) LPVOID lpBuffer, __inout LPDWORD lpdwBufferLength, __inout_opt LPDWORD lpdwIndex
#define HttpQueryInfoVal hRequest, dwInfoLevel, lpBuffer, lpdwBufferLength, lpdwIndex

//CxdWininetHook
class CxdWininetHook: public CxdApiHookBasic
{
public:
    CxdWininetHook(HANDLE AProcess = 0);

    DefineApiHookAW( HttpSendRequest, WINAPI, BOOL );
    DefineApiHookAW( HttpSendRequestEx, WINAPI, BOOL );
    DefineApiHookAW( HttpEndRequest, WINAPI, BOOL);
    DefineApiHookAW( InternetConnect, WINAPI, HINTERNET );
    DefineApiHookAW( HttpAddRequestHeaders, WINAPI, BOOL );
    DefineApiHookAW( InternetOpenUrl, WINAPI, HINTERNET );
    DefineApiHookAW( InternetOpen, WINAPI, HINTERNET );
    DefineApiHook( InternetCloseHandle, WINAPI, BOOL);
    DefineApiHookAW( InternetSetCookie, WINAPI, BOOL);
    DefineApiHookAW( InternetSetCookieEx, WINAPI, DWORD);
    DefineApiHookAW( InternetGetCookie, WINAPI, BOOL);
    DefineApiHookAW( InternetGetCookieEx, WINAPI, BOOL);
    DefineApiHookAW( HttpQueryInfo, WINAPI, BOOL);
protected:
    static CxdWininetHook* SGpCurHook;
    static const TApiHookInfo CtpHoldApiInfo[];
};

//socket
#define socketDefine __in int af, __in int type, __in int protocol
#define socketVal af, type, protocol

//connect
#define connectDefine __in SOCKET s, __in_bcount(namelen) const struct sockaddr FAR *name, __in int namelen
#define connectVal s, name, namelen

//send
#define sendDefine __in SOCKET s, __in_bcount(len) const char FAR * buf, __in int len, __in int flags
#define sendVal s, buf, len, flags

//recv
#define recvDefine __in SOCKET s, __out_bcount_part(len, return) __out_data_source(NETWORK) char FAR * buf, __in int len, __in int flags
#define recvVal s, buf, len, flags

//sendto
#define sendtoDefine __in SOCKET s, __in_bcount(len) const char FAR * buf, __in int len, __in int flags, \
    __in_bcount_opt(tolen) const struct sockaddr FAR *to, __in int tolen
#define sendtoVal s, buf, len, flags, to, tolen

//recvfrom
#define recvfromDefine __in SOCKET s, __out_bcount_part(len, return) __out_data_source(NETWORK) char FAR * buf, \
    __in int len, __in int flags, __out_bcount_opt(*fromlen) struct sockaddr FAR * from, __inout_opt int FAR * fromlen
#define recvfromVal s, buf, len, flags, from, fromlen

//WSASocket
#define WSASocketADefine __in int af, __in int type, __in int protocol,__in_opt LPWSAPROTOCOL_INFOA lpProtocolInfo,\
    __in GROUP g, __in DWORD dwFlags
#define WSASocketWDefine __in int af, __in int type, __in int protocol,__in_opt LPWSAPROTOCOL_INFOW lpProtocolInfo,\
    __in GROUP g, __in DWORD dwFlags
#define WSASocketVal af, type, protocol, lpProtocolInfo, g, dwFlags

//WSASend
#define WSASendDefine __in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount,\
    __out_opt LPDWORD lpNumberOfBytes, __in DWORD dwFlags, __inout_opt LPWSAOVERLAPPED lpOverlapped,\
    __in_opt LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
#define WSASendVal s, lpBuffers, dwBufferCount, lpNumberOfBytes, dwFlags, lpOverlapped, lpCompletionRoutine

//WSARecv
#define WSARecvDefine __in SOCKET s, __in_ecount(dwBufferCount) LPWSABUF lpBuffers, __in DWORD dwBufferCount,\
    __out_opt LPDWORD lpNumberOfBytes, __in DWORD dwFlags, __inout_opt LPWSAOVERLAPPED lpOverlapped,\
    __in_opt LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
#define WSARecvVal s, lpBuffers, dwBufferCount, lpNumberOfBytes, dwFlags, lpOverlapped, lpCompletionRoutine

#define closesocketDefine IN SOCKET s
#define closesocketVal s

class CxdWinSockHook: public CxdApiHookBasic
{
public:
    CxdWinSockHook(HANDLE AProcess = 0);
    
    DefineApiHook(socket, WSAAPI, int );
    DefineApiHook(connect, WSAAPI, int );
    DefineApiHook(send, WSAAPI, int );
    DefineApiHook(recv, WSAAPI, int );
    DefineApiHook(sendto, WSAAPI, int );
    DefineApiHook( recvfrom, WSAAPI, int );
    DefineApiHook( WSASend, WSAAPI, int );
    DefineApiHook( WSARecv, WSAAPI, int );
    DefineApiHook(closesocket, WSAAPI, int );
    DefineApiHookAW(WSASocket, WSAAPI, SOCKET );
private:
    static CxdWinSockHook* SGpCurHook;
    static const TApiHookInfo CtpHoldApiInfo[];
};