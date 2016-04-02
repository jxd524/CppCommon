#include "StdAfx.h"
#include "JxdApiHook.h"
#include "JxdMacro.h"



bool IsExsitsHookApi( const TApiHookInfoList *ApList, const TApiHookInfo *ApItem )
{
    for ( TApiHookInfoListCIT it = ApList->cbegin(); it != ApList->cend(); it++ )
    {
        const TApiHookInfo *pInfo = *it;
        if ( pInfo == ApItem )
        {
            return true;
        }
    }
    return false;
}

const TApiHookInfo* FindHookItem( TApiHookInfoList *ApList, const char *ApApiName, bool AbDelItem )
{
    for ( TApiHookInfoListCIT it = ApList->cbegin(); it != ApList->cend(); it++ )
    {
        const TApiHookInfo *pInfo = *it;
        if ( pInfo->ApiName.CompareNoCase(ApApiName) == 0 )
        {
            if ( AbDelItem )
            {
                ApList->erase( it );
            }
            return pInfo;
        }
    }
    return NULL;
}

const TApiHookInfo* FindHookItem( TApiHookInfoList *ApList, const LPVOID ApApiAddress, bool AbDelItem )
{
    for ( TApiHookInfoListCIT it = ApList->cbegin(); it != ApList->cend(); it++ )
    {
        const TApiHookInfo *pInfo = *it;
        if ( pInfo->ApiSrcAddr == ApApiAddress )
        {
            if ( AbDelItem )
            {
                ApList->erase( it );
            }
            return pInfo;
        }
    }
    return NULL;
}

const TApiHookInfo* FindHookItem( const TApiHookInfo ApBasic[], const int &ACount, const char *ApApiName )
{
    for (int i = 0; i < ACount; i++)
    {
    	const TApiHookInfo *pInfo = &ApBasic[i];
        if ( 0 == pInfo->ApiName.CompareNoCase(ApApiName) )
        {
            return pInfo;
        }
    }
    return NULL;
}

const TApiHookInfo* FindHookItem( const TApiHookInfo ApBasic[], const int &ACount, const LPVOID ApApiAddress )
{
    for (int i = 0; i < ACount; i++)
    {
        const TApiHookInfo *pInfo = &ApBasic[i];
        if ( ApApiAddress == pInfo->ApiSrcAddr )
        {
            return pInfo;
        }
    }
    return NULL;
}

static TxdApiHookBasicList _GHookList;
TLdrLoadDll CxdApiHookBasic::m_psLdrLoadDll = NULL;  
TLdrGetProcedureAddress CxdApiHookBasic::m_psLdrGetProcedureAddress = NULL;  

IMPLEMENT_DYNAMIC(CxdApiHookBasic, CObject)

CxdApiHookBasic::CxdApiHookBasic(HANDLE AProcess)
{
    m_bActiveHook = false;
    m_hProcess = AProcess;
    if ( m_hProcess == 0 )
    {
        m_hProcess = ::GetCurrentProcess();
    }
    _GHookList.push_back( this );

    m_pHoldApiInfos = NULL;
    m_nHoldApiCount = 0;
}

CxdApiHookBasic::~CxdApiHookBasic(void)
{
    SetActiveHook( false );
    for ( TxdApiHookBasicListIT it = _GHookList.begin(); it != _GHookList.end(); it++ )
    {
        CxdApiHookBasic *pObj = *it;
        if ( pObj == this )
        {
            _GHookList.erase( it );
            break;
        }
    }
}

void CxdApiHookBasic::InitApiHookParam( const TApiHookInfo AAllHoldApiInfo[], int ACount,
    const char *ApDllNameA, const TCHAR *ApDllNameT )
{
    m_pHoldApiInfos = AAllHoldApiInfo;
    m_nHoldApiCount = ACount;
    m_strDllNameA = ApDllNameA;
    m_strDllNameT = ApDllNameT;
}

bool CxdApiHookBasic::AddHookApi( const char * AApiName )
{
    if ( !IsInitApiHookParam() )
    {
        return false;
    }
    const TApiHookInfo *pFind = FindHookItem( m_pHoldApiInfos, m_nHoldApiCount, AApiName );
    if ( Assigned(pFind) )
    {
        DoAddHookInfo( pFind );
        return true;
    }
    else
    {
        bool bFind = false;
        CStringA strApi( AApiName );
        const TApiHookInfo *pFind = FindHookItem( m_pHoldApiInfos, m_nHoldApiCount, strApi + "A" );
        if ( Assigned(pFind) )
        {
            bFind = true;
            DoAddHookInfo( pFind );
        }
        pFind = FindHookItem( m_pHoldApiInfos, m_nHoldApiCount, strApi + "W" );
        if ( Assigned(pFind) )
        {
            bFind = true;
            DoAddHookInfo( pFind );
        }
        return bFind;
    }
    return false;
}

bool CxdApiHookBasic::AddHookApi( LPVOID ApApiAddress )
{
    if ( !IsInitApiHookParam() )
    {
        return false;
    }
    const TApiHookInfo *pFind = FindHookItem( m_pHoldApiInfos, m_nHoldApiCount, ApApiAddress );
    if ( Assigned(pFind) )
    {
        DoAddHookInfo( pFind );
        return true;
    }
    return false;
}

void CxdApiHookBasic::DoAddHookInfo( const TApiHookInfo *ApInfo )
{
    if ( !IsExsitsHookApi(&m_CurHookApiList, ApInfo) )
    {
        m_CurHookApiList.push_back( ApInfo );
        if ( GetActiveHook() )
        {
            ReplaceAllModuleIATEntry( m_hProcess, m_strDllNameA, ApInfo->ApiSrcAddr, ApInfo->ApiNewAddr );
        }
    }
}

void CxdApiHookBasic::DelHookApi( const char* AApiName )
{
    const TApiHookInfo* pInfo = FindHookItem( &m_CurHookApiList, AApiName, true );
    if ( Assigned(pInfo) && GetActiveHook() )
    {
        ReplaceAllModuleIATEntry( m_hProcess, m_strDllNameA, pInfo->ApiNewAddr, pInfo->ApiSrcAddr );
    }
}

void CxdApiHookBasic::DelHookApi( LPVOID ApApiAddress )
{
    const TApiHookInfo* pInfo = FindHookItem( &m_CurHookApiList, ApApiAddress, true );
    if ( Assigned(pInfo) && GetActiveHook() )
    {
        ReplaceAllModuleIATEntry( m_hProcess, m_strDllNameA, pInfo->ApiNewAddr, pInfo->ApiSrcAddr );
    }
}

bool CxdApiHookBasic::SetActiveHook( bool bActive )
{
    if ( m_bActiveHook != bActive )
    {
        if ( bActive )
        {
            if ( ActiveHook(0) )
            {
                if ( !Assigned(m_psLdrLoadDll) )
                {
                    m_psLdrLoadDll = (TLdrLoadDll)::GetProcAddress( ::GetModuleHandle(_T("NTDLL.DLL")), "LdrLoadDll" );
                    ReplaceAllModuleIATEntry( m_hProcess, "NTDLL.DLL", m_psLdrLoadDll, &CxdApiHookBasic::_LdrLoadDll);  
                }
                if ( !Assigned(m_psLdrGetProcedureAddress) )
                {
                    m_psLdrGetProcedureAddress = (TLdrGetProcedureAddress)::GetProcAddress( ::GetModuleHandle(_T("NTDLL.DLL")), "LdrGetProcedureAddress" );
                    ReplaceAllModuleIATEntry( m_hProcess, "NTDLL.DLL", m_psLdrGetProcedureAddress, &CxdApiHookBasic::_LdrGetProcedureAddress);  
                }
                m_bActiveHook = Assigned(m_psLdrLoadDll) && Assigned(m_psLdrGetProcedureAddress);
            }
        }
        else
        {
            if ( _GHookList.size() <= 1 )
            {
                if ( Assigned(m_psLdrLoadDll) )
                {
                    ReplaceAllModuleIATEntry( m_hProcess, "NTDLL.DLL", &CxdApiHookBasic::_LdrLoadDll, m_psLdrLoadDll );  
                    m_psLdrLoadDll = NULL;
                }
                if ( Assigned(m_psLdrGetProcedureAddress) )
                {
                    ReplaceAllModuleIATEntry( m_hProcess, "NTDLL.DLL", &CxdApiHookBasic::_LdrGetProcedureAddress, m_psLdrGetProcedureAddress );
                    m_psLdrGetProcedureAddress = NULL;
                }
            }
            UnActiveHook();
            m_bActiveHook = false;
        }
    }
    return m_bActiveHook == bActive;
}

bool CxdApiHookBasic::ActiveHook( HANDLE hModCaller )
{
    if ( 0 == hModCaller )
    {
        if ( IsInitApiHookParam() && DoActiveHook(hModCaller) )
        {
            RelaceAllModuleIATEnterByList( m_strDllNameA, m_hProcess, m_CurHookApiList );
            return true;
        }
    }
    else if ( IsInitApiHookParam() )
    {
        RelaceIATEnterByList( m_hProcess, hModCaller, m_strDllNameA, m_CurHookApiList );
        return true;
    }
    return false;
}

void CxdApiHookBasic::UnActiveHook( void )
{
    if ( DoUnActiveHook() && IsInitApiHookParam() )
    {
        RestoreAllModuleIATEnterByList( m_strDllNameA, m_hProcess, m_CurHookApiList );
    }
}

PVOID CxdApiHookBasic::FindHookFunctionAddress( const CString &AModName, const CStringA &AFunctionName, const LPVOID AFunctionAddress )
{
    if ( IsInitApiHookParam() && !AFunctionName.IsEmpty() && 
        0 == AModName.CompareNoCase(m_strDllNameT) )
    {
        const TApiHookInfo *pInfo = FindHookItem( &m_CurHookApiList, AFunctionAddress, false );
        if ( Assigned(pInfo) )
        {
            return pInfo->ApiNewAddr;
        }
    }
    return NULL;
}

void CxdApiHookBasic::ReplaceAllModuleIATEntry( HANDLE hProcess, LPCSTR lpszDllName, LPVOID pfnCurrent, LPVOID pfnNew )
{
    HMODULE hMods[1024] = {0};  
    DWORD cbNeeded; 
    if ( 0 == hProcess )
    {
        hProcess = ::GetCurrentProcess();  
    }
    if( ::EnumProcessModules( hProcess, hMods, sizeof(hMods), &cbNeeded))  
    {  
        for ( UINT i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )  
        {  
            ReplaceIATEntryInImageImportTable( hProcess, hMods[i], lpszDllName, pfnCurrent, pfnNew );  
        }  
    }  
}

BOOL CxdApiHookBasic::ReplaceIATEntryInImageImportTable( HANDLE hProcess, HANDLE hModCaller, LPCSTR lpszDllName, LPVOID pfnCurrent, LPVOID pfnNew )
{
    ASSERT(hModCaller && lpszDllName && pfnCurrent && pfnNew );  
    //PIMAGE_IMPORT_DESCRIPTOR -> IMAGE_DIRECTORY_ENTRY_IMPORT
    if ( 0 == hProcess )
    {
        hProcess = ::GetCurrentProcess();
    }
    DWORD dwSize = 0;  
    PIMAGE_SECTION_HEADER pFoundHeader = NULL;  
    PIMAGE_IMPORT_DESCRIPTOR pImgImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToDataEx( hModCaller, TRUE, 
        IMAGE_DIRECTORY_ENTRY_IMPORT, &dwSize, &pFoundHeader );  
    if( pImgImportDescriptor == NULL )
    { 
        return FALSE; 
    }

    while (pImgImportDescriptor->Name)  
    {  
        if ( _strcmpi((CHAR*)((PBYTE)hModCaller+pImgImportDescriptor->Name), lpszDllName) == 0 )  
        {  
            break; // found  
        }  
        ++pImgImportDescriptor;  
    }  
    // NOTE:
    // If the special module can not be found in IMAGE_DIRECTORY_ENTRY_IMPORT
    // Then should try to search it in IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT
    if( !pImgImportDescriptor->Name )  
    {
        return ReplaceIATEntryInDelayImageImportTable( hProcess, hModCaller, lpszDllName, pfnCurrent, pfnNew);  
    }

    // retrieve IAT  
    PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)(((LPBYTE)hModCaller) + pImgImportDescriptor->FirstThunk);  

    // enumerate functions in the IAT
    while( pThunk->u1.Function )
    {    
        PDWORD lpAddr = (PDWORD)&(pThunk->u1.Function);    
        if(*lpAddr == (DWORD)pfnCurrent)    
        {  
            // modify the address
            ::WriteProcessMemory( hProcess, lpAddr, &pfnNew, sizeof(DWORD), NULL );
            return TRUE;  
        }     
        pThunk++;
    }  
    return FALSE;  
}

BOOL CxdApiHookBasic::ReplaceIATEntryInDelayImageImportTable(HANDLE hProcess, HANDLE hModCaller, LPCSTR lpszDllName, LPVOID pfnCurrent, LPVOID pfnNew )
{
    ASSERT(hModCaller && lpszDllName && pfnCurrent && pfnNew );  
    if ( 0 == hProcess )
    {
        hProcess = ::GetCurrentProcess();
    }
    // retrieve IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT  
    DWORD dwSize = 0;  
    PIMAGE_SECTION_HEADER pFoundHeader = NULL;  
    PImgDelayDescr pImgDelayDescr = (PImgDelayDescr)ImageDirectoryEntryToDataEx( hModCaller, TRUE, 
        IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, &dwSize, &pFoundHeader);  
    if( pImgDelayDescr == NULL )
    { 
        return FALSE; 
    }

    while ( pImgDelayDescr->rvaDLLName )  
    {  
        if ( 0 == _strcmpi((CHAR*)((PBYTE)hModCaller+pImgDelayDescr->rvaDLLName), lpszDllName) )  
        {  
            break;  
        }  
        ++pImgDelayDescr;  
    }  
    // Not found in IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT
    if( !pImgDelayDescr->rvaDLLName )  
    {
        return FALSE;  
    }

    // retrieve IAT  
    PIMAGE_THUNK_DATA pThunk = NULL;  
    if( (pImgDelayDescr->grAttrs & dlattrRva) == 0 ) 
    {
        return FALSE;  
    }
    pThunk = (PIMAGE_THUNK_DATA)(((LPBYTE)hModCaller) + pImgDelayDescr->rvaIAT); 

    // enumerate functions in the IAT
    while(pThunk->u1.Function)    
    {    
        PDWORD lpAddr = (PDWORD)&(pThunk->u1.Function);    
        if(*lpAddr == (DWORD)pfnCurrent)    
        {  
            // modify the address
            ::WriteProcessMemory( hProcess, lpAddr, &pfnNew, sizeof(DWORD), NULL);
            return TRUE;
        }     
        pThunk++;    
    }
    return FALSE;  
}

void CxdApiHookBasic::RelaceAllModuleIATEnterByList( LPCSTR lpszDllName, const HANDLE &hProcess, TApiHookInfoList &AList)
{
    for (TApiHookInfoListCIT it = AList.cbegin(); it != AList.cend(); it++ )
    {
        const TApiHookInfo *pInfo = *it;
        ReplaceAllModuleIATEntry( hProcess, lpszDllName, pInfo->ApiSrcAddr, pInfo->ApiNewAddr );
    }
}

void CxdApiHookBasic::RestoreAllModuleIATEnterByList( LPCSTR lpszDllName, const HANDLE &hProcess, TApiHookInfoList &AList )
{
    for (TApiHookInfoListCIT it = AList.cbegin(); it != AList.cend(); it++ )
    {
        const TApiHookInfo *pInfo = *it;
        ReplaceAllModuleIATEntry( hProcess, lpszDllName, pInfo->ApiNewAddr, pInfo->ApiSrcAddr );
    }
}

void CxdApiHookBasic::RelaceIATEnterByList( const HANDLE &hProcess, const HANDLE &hModCaller, LPCSTR lpszDllName, TApiHookInfoList &AList )
{
    for (TApiHookInfoListCIT it = AList.cbegin(); it != AList.cend(); it++ )
    {
        const TApiHookInfo *pInfo = *it;
        ReplaceIATEntryInImageImportTable( hProcess, hModCaller, lpszDllName, pInfo->ApiSrcAddr, pInfo->ApiNewAddr );
    }
}

NTSTATUS WINAPI CxdApiHookBasic::_LdrLoadDll( IN PWCHAR PathToFile OPTIONAL, IN ULONG Flags OPTIONAL, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle )
{
    NTSTATUS ntStatus = m_psLdrLoadDll( PathToFile, Flags, ModuleFileName, ModuleHandle);  
    if( ntStatus == STATUS_SUCCESS && (Flags & LOAD_LIBRARY_AS_DATAFILE) == 0 )  
    {
        HANDLE hDll = *ModuleHandle;  
        ReplaceIATEntryInImageImportTable( 0, hDll, "NTDLL.DLL", m_psLdrLoadDll, &CxdApiHookBasic::_LdrLoadDll);  
        ReplaceIATEntryInImageImportTable( 0, hDll, "NTDLL.DLL", m_psLdrGetProcedureAddress, &CxdApiHookBasic::_LdrGetProcedureAddress);  
        
        for ( TxdApiHookBasicListIT it = _GHookList.begin(); it != _GHookList.end(); it++ )
        {
            CxdApiHookBasic *pObj = *it;
            if ( pObj->GetActiveHook() )
            {
                pObj->ActiveHook( hDll );
            }
        }
    }  
    return ntStatus; 
}

NTSTATUS WINAPI CxdApiHookBasic::_LdrGetProcedureAddress( IN HMODULE ModuleHandle, IN PANSI_STRING FunctionName OPTIONAL, IN WORD Oridinal OPTIONAL, OUT PVOID *FunctionAddress )
{
    NTSTATUS ntStatus = m_psLdrGetProcedureAddress( ModuleHandle, FunctionName, Oridinal, FunctionAddress);  
    if( ntStatus == STATUS_SUCCESS )  
    {  
        TCHAR tszPath[MAX_PATH] = {0};  
        if( GetModuleFileName( ModuleHandle, tszPath, MAX_PATH) )  
        {  
            CString strModName( tszPath );  
            int nFind = strModName.ReverseFind( '\\' );  
            if( nFind > 0 )  
            {
                strModName = strModName.Mid( nFind + 1 );  
            }
            CStringA strFunctionName( Assigned(FunctionName) ? FunctionName->Buffer : "NULL" );

            LPVOID func = NULL;
            for ( TxdApiHookBasicListIT it = _GHookList.begin(); it != _GHookList.end(); it++ )
            {
                CxdApiHookBasic *pObj = *it;
                if ( pObj->GetActiveHook() )
                {
                    func = pObj->FindHookFunctionAddress( strModName, strFunctionName, *FunctionAddress );
                    if ( Assigned(func) )
                    {
                        *FunctionAddress = func;
                        break;
                    }
                }
                
            }            
        }  
    }
    return ntStatus;  
}


//CxdWininetHook
CxdWininetHook* CxdWininetHook::SGpCurHook = NULL;
ImplementApiHookAW( CxdWininetHook, HttpSendRequest, WINAPI, BOOL );
ImplementApiHookAW( CxdWininetHook, HttpSendRequestEx, WINAPI, BOOL );
ImplementApiHookAW( CxdWininetHook, HttpEndRequest, WINAPI, BOOL);
ImplementApiHookAW( CxdWininetHook, InternetConnect, WINAPI, HINTERNET );
ImplementApiHookAW( CxdWininetHook, HttpAddRequestHeaders, WINAPI, BOOL );
ImplementApiHookAW( CxdWininetHook, InternetOpenUrl, WINAPI, HINTERNET );
ImplementApiHookAW( CxdWininetHook, InternetOpen, WINAPI, HINTERNET );
ImplementApiHook( CxdWininetHook, InternetCloseHandle, WINAPI, BOOL);
ImplementApiHookAW( CxdWininetHook, InternetSetCookie, WINAPI, BOOL);
ImplementApiHookAW( CxdWininetHook, InternetSetCookieEx, WINAPI, DWORD);
ImplementApiHookAW( CxdWininetHook, InternetGetCookie, WINAPI, BOOL);
ImplementApiHookAW( CxdWininetHook, InternetGetCookieEx, WINAPI, BOOL);
ImplementApiHookAW( CxdWininetHook, HttpQueryInfo, WINAPI, BOOL);

ImplementHoldApiInfo(CxdWininetHook, CtpHoldApiInfo)
    HoldApiItemAW( CxdWininetHook, HttpSendRequest),
    HoldApiItemAW( CxdWininetHook, HttpSendRequestEx),
    HoldApiItemAW( CxdWininetHook, HttpEndRequest),
    HoldApiItemAW( CxdWininetHook, InternetConnect),
    HoldApiItemAW( CxdWininetHook, HttpAddRequestHeaders ),
    HoldApiItemAW( CxdWininetHook, InternetOpenUrl ),
    HoldApiItemAW( CxdWininetHook, InternetOpen ),
    HoldApiItem( CxdWininetHook, InternetCloseHandle ),
    HoldApiItemAW( CxdWininetHook, InternetSetCookie ),
    HoldApiItemAW( CxdWininetHook, InternetSetCookieEx ),
    HoldApiItemAW( CxdWininetHook, InternetGetCookie),
    HoldApiItemAW( CxdWininetHook, InternetGetCookieEx),
    HoldApiItemAW( CxdWininetHook, HttpQueryInfo)
EndHoldApiInfo

CxdWininetHook::CxdWininetHook( HANDLE AProcess /*= 0*/ ): CxdApiHookBasic(AProcess)
{
    if ( SGpCurHook ) throw "only can create on CxdWininetHook Object!!!";
    SGpCurHook = this;
    InitApiHookParam( CtpHoldApiInfo, sizeof(CtpHoldApiInfo) / sizeof(TApiHookInfo), "WININET.DLL", TEXT("WININET.DLL") );
}

//CxdWinSockHook
CxdWinSockHook* CxdWinSockHook::SGpCurHook = NULL;
ImplementApiHook(CxdWinSockHook, socket, WSAAPI, int);
ImplementApiHookAW(CxdWinSockHook, WSASocket, WSAAPI, SOCKET);
ImplementApiHook(CxdWinSockHook, connect, WSAAPI, int);
ImplementApiHook(CxdWinSockHook, send, WSAAPI, int);
ImplementApiHook(CxdWinSockHook, recv, WSAAPI, int);
ImplementApiHook(CxdWinSockHook, sendto, WSAAPI, int);
ImplementApiHook(CxdWinSockHook, recvfrom, WSAAPI, int);
ImplementApiHook(CxdWinSockHook, WSASend, WSAAPI, int);
ImplementApiHook(CxdWinSockHook, WSARecv, WSAAPI, int);
ImplementApiHook(CxdWinSockHook, closesocket, WSAAPI, int);

ImplementHoldApiInfo(CxdWinSockHook, CtpHoldApiInfo)
    HoldApiItem(CxdWinSockHook, socket),
    HoldApiItemAW(CxdWinSockHook, WSASocket),
    HoldApiItem(CxdWinSockHook, connect),
    HoldApiItem(CxdWinSockHook, send),
    HoldApiItem(CxdWinSockHook, recv),
    HoldApiItem(CxdWinSockHook, sendto),
    HoldApiItem(CxdWinSockHook, recvfrom),
    HoldApiItem(CxdWinSockHook, WSASend),
    HoldApiItem(CxdWinSockHook, WSARecv),
    HoldApiItem(CxdWinSockHook, closesocket)
EndHoldApiInfo

CxdWinSockHook::CxdWinSockHook( HANDLE AProcess /*= 0*/ ): CxdApiHookBasic(AProcess)
{
    if ( SGpCurHook ) throw "only can create on CxdwinSockHook Object!!!";
    SGpCurHook = this;
    InitApiHookParam( CtpHoldApiInfo, sizeof(CtpHoldApiInfo) / sizeof(TApiHookInfo), "Ws2_32.dll", TEXT("Ws2_32.dll") );
}
