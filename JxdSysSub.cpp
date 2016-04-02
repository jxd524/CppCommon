#include "stdafx.h"
#include "JxdSysSub.h"
#include "JxdFileSub.h"
#include <atlconv.h>
#include "JxdDebugInfo.h"
#include "JxdDataStream.h"

/***********************************************************************
    Window 下相关
************************************************************************/
#ifdef WIN32

#include <Shlobj.h>
#pragma comment(lib, "Version.lib")


void OpenIE( const TCHAR* ApURL, bool ATryOpenIE /*= true*/ )
{
    bool bOK( false );
    if ( ATryOpenIE )
    {
        bOK = ::ShellExecute(0, TEXT("open"), TEXT("iexplore"), ApURL, 0, SW_NORMAL) > (HINSTANCE)32;
    }
    if ( !bOK )
    {
        ::ShellExecute(0, TEXT("open"), ApURL, 0, 0, SW_NORMAL);
    }
}

void BringWindowToShow( HWND hWnd )
{
    if ( IsWindow(hWnd) )
    {
        if ( IsIconic(hWnd) )
        {
            ShowWindow(hWnd, SW_RESTORE);
        }
        else if ( !IsWindowVisible(hWnd) )
        {
            ShowWindow(hWnd, SW_SHOW);
        }
        ::SetForegroundWindow(hWnd);
    }
}

CString GetAppVersion( const TCHAR* AppName )
{
    struct   LANGANDCODEPAGE
    { 
        WORD   wLanguage; 
        WORD   wCodePage; 
    }   *TranslationPtr; 

    CString   AppVersion; 
    DWORD   RessourceVersionInfoSize; 
    DWORD   JustAJunkVariabel; 
    WCHAR*   VersionInfoPtr; 
    WCHAR*     InformationPtr; 
    UINT      VersionInfoSize; 
    WCHAR     VersionValue[255]; 
    RessourceVersionInfoSize=GetFileVersionInfoSize(AppName,&JustAJunkVariabel); 
    if(0!=RessourceVersionInfoSize) 
    {
        VersionInfoPtr = new WCHAR[RessourceVersionInfoSize];
        if(!GetFileVersionInfo(AppName,0,RessourceVersionInfoSize,VersionInfoPtr)) 
        {
            delete[]   VersionInfoPtr; 
            return TEXT("1.0.0.0"); 
        } 
        if(!VerQueryValue( VersionInfoPtr, TEXT("VarFileInfo\\Translation"), (LPVOID*)&TranslationPtr, &VersionInfoSize)) 
        { 
            delete[]   VersionInfoPtr; 
            return TEXT("1.0.0.0"); 
        } 
        // retrieve product version
        wsprintf(VersionValue, TEXT("\\StringFileInfo\\%04x%04x\\ProductVersion"), TranslationPtr[0].wLanguage, TranslationPtr[0].wCodePage); 
        if(!VerQueryValue( VersionInfoPtr, VersionValue, (LPVOID*)&InformationPtr, &VersionInfoSize)) 
        { 
            delete[]   VersionInfoPtr; 
            return TEXT("1.0.0.0"); 
        } 
        if( wcslen(InformationPtr)> 0 )   //Not   Null 
        { 
            AppVersion=CString(InformationPtr); 
        } 
        delete[]VersionInfoPtr; 
    } 
    return   AppVersion; 
} 

CString GetVersionMark()
{
    OSVERSIONINFOEX os; 
    os.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX); 
    if(GetVersionEx((OSVERSIONINFO *)&os)){ 
        CString vmark; 
        switch(os.dwMajorVersion){                //先判断操作系统版本 
        case 5: 
            switch(os.dwMinorVersion){ 
            case 0:                  //Windows 2000 
                if(os.wSuiteMask==VER_SUITE_ENTERPRISE) 
                    vmark=_T("Advanced Server"); 
                break; 
            case 1:                  //Windows XP 
                if(os.wSuiteMask==VER_SUITE_EMBEDDEDNT) 
                    vmark=_T("Embedded"); 
                else if(os.wSuiteMask==VER_SUITE_PERSONAL) 
                    vmark=_T("Home Edition"); 
                else 
                    vmark=_T("Professional"); 
                break; 
            case 2: 
                if(GetSystemMetrics(SM_SERVERR2)==0 && os.wSuiteMask==VER_SUITE_BLADE)  //Windows Server 2003 
                    vmark=_T("Web Edition"); 
                else if(GetSystemMetrics(SM_SERVERR2)==0 && os.wSuiteMask==VER_SUITE_COMPUTE_SERVER) 
                    vmark=_T("Compute Cluster Edition"); 
                else if(GetSystemMetrics(SM_SERVERR2)==0 && os.wSuiteMask==VER_SUITE_STORAGE_SERVER) 
                    vmark=_T("Storage Server"); 
                else if(GetSystemMetrics(SM_SERVERR2)==0 && os.wSuiteMask==VER_SUITE_DATACENTER) 
                    vmark=_T("Datacenter Edition"); 
                else if(GetSystemMetrics(SM_SERVERR2)==0 && os.wSuiteMask==VER_SUITE_ENTERPRISE) 
                    vmark=_T("Enterprise Edition"); 
                else if(GetSystemMetrics(SM_SERVERR2)!=0 && os.wSuiteMask==VER_SUITE_STORAGE_SERVER)  
                    vmark=_T("Storage Server"); 
                break; 
            } 
            break; 
        case 6: 
            switch(os.dwMinorVersion){ 
            case 0: 
                if(os.wProductType!=VER_NT_WORKSTATION && os.wSuiteMask==VER_SUITE_DATACENTER)   
                    vmark=_T("Datacenter Server"); 
                else if(os.wProductType!=VER_NT_WORKSTATION && os.wSuiteMask==VER_SUITE_ENTERPRISE) 
                    vmark=_T("Enterprise"); 
                else if(os.wProductType==VER_NT_WORKSTATION && os.wSuiteMask==VER_SUITE_PERSONAL)  //Windows Vista
                    vmark=_T("Home"); 
                break; 
            } 
            break; 
        default: 
            vmark=_T(""); 
        } 
        return vmark; 
    } 
    else 
        return _T("");
}

CString GetSystemName()
{
    SYSTEM_INFO info;
    GetSystemInfo( &info );

    OSVERSIONINFOEX os;
    os.dwOSVersionInfoSize = sizeof(os);
    if ( GetVersionEx((OSVERSIONINFO *)&os) )
    {
        CString strName;
        switch (os.dwMajorVersion)
        {
        case 4:
            switch (os.dwMinorVersion)
            {
            case 0:
                if ( os.dwPlatformId == VER_PLATFORM_WIN32_NT )
                {
                    strName = TEXT("Microsoft Windows NT 4.0");
                }
                else if ( os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
                {
                    strName = TEXT("Microsoft Windows 95");
                }
                break;
            case 10:
                strName = TEXT("Microsoft Windows 98");
                break;
            case 90:
                strName = TEXT("Microsoft Windows Me");
                break;
            }
            break;
        case 5:
            switch ( os.dwMinorVersion )
            {
            case 0:
                strName = TEXT("Microsoft Windows 2000");
                break;
            case 1:
                strName = TEXT("Microsoft Windows XP");
                break;
            case 2:
                if ( os.wProductType == VER_NT_WORKSTATION && info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
                {
                    strName = TEXT("Microsoft Windows XP Professional x64 Edition");
                }
                else if ( GetSystemMetrics(SM_SERVERR2) == 0 )
                {
                    strName = TEXT("Microsoft Windows Server 2003");
                }
                else if ( GetSystemMetrics(SM_SERVERR2) != 0 )
                {
                    strName = TEXT("Microsoft Windows Server 2003 R2");
                }
                break;
            }
            break;
        case 6:
            switch (os.dwMinorVersion)
            {
            case 0:
                if ( os.wProductType == VER_NT_WORKSTATION )
                {
                    strName = TEXT("Microsoft Windows Vista");
                }
                else
                {
                    strName = TEXT("Microsoft Windows Server 2008");
                }
                break;
            case 1:
                if ( os.wProductType == VER_NT_WORKSTATION )
                {
                    strName = TEXT("Microsoft Windows 7");
                }
                else
                {
                    strName = TEXT("Microsoft Windows Server 2008 R2");
                }
                break;
            }
            break;
        default:
            strName = TEXT("未知操作系统");
            break;
        }
        return strName;
    }
    else
    {
        return TEXT("");
    }
}

CString GetIEVersion()
{
    TCHAR szFilename[] = _T("mshtml.dll");  
    DWORD dwMajorVersion =0, dwMinorVersion = 0;  
    DWORD dwBuildNumber =0, dwRevisionNumber = 0;  
    DWORD dwHandle = 0;  
    DWORD dwVerInfoSize = GetFileVersionInfoSize(szFilename, &dwHandle);  
    if (dwVerInfoSize)  
    {  
        LPVOID lpBuffer = LocalAlloc(LPTR, dwVerInfoSize);  
        if (lpBuffer)  
        {  
            if (GetFileVersionInfo(szFilename, dwHandle, dwVerInfoSize, lpBuffer))  
            {  
                VS_FIXEDFILEINFO * lpFixedFileInfo = NULL;  
                UINT nFixedFileInfoSize = 0;  
                if (VerQueryValue(lpBuffer, TEXT("\\"), (LPVOID*)&lpFixedFileInfo, &nFixedFileInfoSize) &&(nFixedFileInfoSize))  
                {  
                    dwMajorVersion = HIWORD(lpFixedFileInfo->dwFileVersionMS);    
                    dwMinorVersion = LOWORD(lpFixedFileInfo->dwFileVersionMS);    
                    dwBuildNumber = HIWORD(lpFixedFileInfo->dwFileVersionLS);    
                    dwRevisionNumber = LOWORD(lpFixedFileInfo->dwFileVersionLS);  
                }  
            }  
            LocalFree(lpBuffer);  
        }  
    } 
    CString strIEVersion;
    strIEVersion.Format( TEXT("%d.%d.%d.%d"), dwMajorVersion, dwMinorVersion, dwBuildNumber, dwRevisionNumber );
    return strIEVersion;
}

bool IsIE6OrIE7Version( void )
{
    TCHAR szFilename[] = _T("mshtml.dll");  
    DWORD dwMajorVersion =0, dwMinorVersion = 0;  
    DWORD dwBuildNumber =0, dwRevisionNumber = 0;  
    DWORD dwHandle = 0;  
    DWORD dwVerInfoSize = GetFileVersionInfoSize(szFilename, &dwHandle);  
    if (dwVerInfoSize)  
    {  
        LPVOID lpBuffer = LocalAlloc(LPTR, dwVerInfoSize);  
        if (lpBuffer)  
        {  
            if (GetFileVersionInfo(szFilename, dwHandle, dwVerInfoSize, lpBuffer))  
            {  
                VS_FIXEDFILEINFO * lpFixedFileInfo = NULL;  
                UINT nFixedFileInfoSize = 0;  
                if (VerQueryValue(lpBuffer, TEXT("\\"), (LPVOID*)&lpFixedFileInfo, &nFixedFileInfoSize) &&(nFixedFileInfoSize))  
                {  
                    dwMajorVersion = HIWORD(lpFixedFileInfo->dwFileVersionMS);    
                    dwMinorVersion = LOWORD(lpFixedFileInfo->dwFileVersionMS);    
                    dwBuildNumber = HIWORD(lpFixedFileInfo->dwFileVersionLS);    
                    dwRevisionNumber = LOWORD(lpFixedFileInfo->dwFileVersionLS);  
                }  
            }  
            LocalFree(lpBuffer);  
        }  
    }  
    return dwMajorVersion == 6 || dwMajorVersion == 7;
}

void CreateStartShortCut( const TCHAR *ApAppFileName, const char *ApShortCut )
{
	char szStartPath[MAX_PATH] = {0};
	SHGetSpecialFolderPathA(NULL, szStartPath, CSIDL_COMMON_PROGRAMS, 0);
	string strTemp( szStartPath );
	strTemp += ApShortCut;
	if ( FileExist(strTemp.c_str()) ) return;
	
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		IShellLink *pisl;
		hr = CoCreateInstance(CLSID_ShellLink, NULL,
			CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pisl);
		if (SUCCEEDED(hr))
		{
			IPersistFile* pIPF;
			//这里是要创建快捷方式的原始文件地址
			pisl->SetPath(ApAppFileName);
			hr = pisl->QueryInterface(IID_IPersistFile, (void**)&pIPF);
			if (SUCCEEDED(hr))
			{
				string strFolder = ExtractFilePath( ApShortCut );
				string strTempPath = szStartPath + strFolder;
				if ( !FileExist(strTempPath.c_str()) ) ForceDirectories( strTempPath.c_str() );
				strcat_s( szStartPath, ApShortCut );

				USES_CONVERSION;
				LPCOLESTR lpOleStr = A2COLE(szStartPath);

				pIPF->Save(lpOleStr, FALSE);

				pIPF->Release();
			}
			pisl->Release();
		}
		CoUninitialize();
	}
}

bool ExtenResource( HMODULE AResHandle, DWORD AResID, const TCHAR* AResType, const TCHAR* ApFileName )
{
    bool bOK = false;
    HGLOBAL hGlobal = NULL;
    HRSRC hSource = NULL;
    LPVOID lpVoid  = NULL;
    int nSize = 0;

    hSource = FindResource( AResHandle, MAKEINTRESOURCE(AResID), AResType );
    if( NULL == hSource )
        return bOK;

    hGlobal = LoadResource( AResHandle, hSource );
    if( NULL == hGlobal )
    {    
        FreeResource(hGlobal);         
        return bOK;        
    }
    lpVoid = LockResource(hGlobal);
    if( NULL == lpVoid )
    {
        goto Exit;
    }

    nSize = (UINT)SizeofResource( AResHandle, hSource );
    HGLOBAL hGlobal2 = GlobalAlloc( GMEM_MOVEABLE, nSize );
    if( hGlobal2 == NULL )
    {
        goto Exit;
    }

    void* pData = GlobalLock( hGlobal2 );
    memcpy( pData, (void *)hGlobal, nSize );
    GlobalUnlock(hGlobal2);

    {
        CxdFileStream f( ApFileName, true );
        f.FileWrite( pData, nSize );
        bOK = true;
    }
    FreeResource(hGlobal2); 
Exit:
    UnlockResource(hGlobal);    
    FreeResource(hGlobal); 
    return bOK;
}

#endif

