
// wbClient.cpp : 定义应用程序的类行为。

#include "stdafx.h"
#include "JxdApplication.h"
#include "JxdGdiPlusBasic.h"
#include "JxdSysSub.h"
#include "JxdHintWindow.h"
#include "JxdDebugInfo.h"
#include "JxdFileSub.h"

#include <DbgHelp.h>
#pragma comment(lib,"Dbghelp.lib")

BEGIN_MESSAGE_MAP(CxdApplication, CWinApp)
END_MESSAGE_MAP()

CString CxdApplication::m_strDumpFileName;

CxdApplication::CxdApplication(): CWinApp()
{
    InitGdiplus();
    m_pfrmHintManage = NULL;
    m_pCommond = NULL;
    m_pUpdate = NULL;
    m_CurUpState = auNULL;
    m_hMutex = 0;
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
    m_pNotifyCreateHintManage = NewInterfaceByClass(CxdApplication, LPVOID, DoCreateHintManage);
    SetNotifyHintInterface( m_pNotifyCreateHintManage );
}

void CxdApplication::DoCreateHintManage( LPVOID )
{
    m_pfrmHintManage = new CxdHintManage;
    m_pfrmHintManage->SetItCreateHintWindow( NewInterfaceByClass(CxdApplication, CxdHintWindow**, DoCreateHintWindow) );
    SetHintInterface( m_pfrmHintManage );
}

CxdApplication::~CxdApplication()
{
    if ( 0 != m_hMutex )
    {
        CloseHandle( m_hMutex );
        m_hMutex = 0;
    }
    SafeDeleteObject( m_pUpdate );
    SafeDeleteObject( m_pNotifyCreateHintManage );
    SetHintInterface( NULL );
    SafeDeleteObject( m_pfrmHintManage );
    SafeDeleteObject( m_pCommond );
}

BOOL CxdApplication::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);
    CWinApp::InitInstance();
	return TRUE;
}

int CxdApplication::ExitInstance()
{
	SafeDeleteObject( m_pfrmHintManage );
    return CWinApp::ExitInstance();
}

BOOL CxdApplication::CheckRunOnes( const TCHAR* const ApMainClassName, bool bShow )
{
    //返回 FALSE 表示已经有实例在运行
    if ( OpenMutex(MUTEX_ALL_ACCESS, false, ApMainClassName) == 0 )
    {
        m_hMutex = CreateMutex( NULL , true, ApMainClassName);
        return TRUE;
    }
	if ( bShow )
	{
        HWND hWnd = FindWindow( ApMainClassName,NULL );
		BringWindowToShow ( hWnd );
	}
    return FALSE;
}

void CxdApplication::SetUnHandleExceptionFileName( const TCHAR* const ApFileName )
{
    if ( ApFileName == NULL )
    {
        m_strDumpFileName.Format( TEXT("%s_%d.dmp"), GetCurProcessName(), GetTickCount() );
    }
    else
    {
        m_strDumpFileName = ApFileName;
    }
    SetUnhandledExceptionFilter(UnhandledExceptionFilter);
}

LONG WINAPI CxdApplication::UnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
{
    try
    {
        HANDLE hFile = ::CreateFile( m_strDumpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL ); 
        if ( hFile!=INVALID_HANDLE_VALUE ) 
        { 
            MINIDUMP_EXCEPTION_INFORMATION ExInfo; 

            ExInfo.ThreadId = ::GetCurrentThreadId(); 
            ExInfo.ExceptionPointers = ExceptionInfo; 
            ExInfo.ClientPointers = NULL; 
            MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL ); 
            ::CloseHandle(hFile); 
        } 
        if ( ((CxdApplication*)AfxGetApp())->OnUnHandleException(ExceptionInfo) )
        {
            ((CxdApplication*)AfxGetApp())->PostThreadMessage(WM_QUIT,0,0);
        }
    }
    catch(...)
    {
    	
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void CxdApplication::DoCreateHintWindow( CxdHintWindow** AWnd )
{
    //创建一个用于提示的窗口
    *AWnd = new CxdHintWindow();
    (*AWnd)->xdCreate( 10, 10, 1, 1, NULL, NULL );
    (*AWnd)->SetHintBKColor( 0xFFFF0000, 0xFF00FF00, LinearGradientModeBackwardDiagonal );
}

void CxdApplication::CheckCommonList( void )
{
    if ( IsNull(m_pCommond) )
    {
        m_pCommond = new TStringList;
        for (int i = 1; i < __argc; i++)
        {
            m_pCommond->push_back( __targv[i] );
        }
    }
}

bool CxdApplication::IsExistCommond( const TCHAR* const ApName )
{
    CheckCommonList();
    TxdParseInfo info;
    const TCHAR* pSrc;
    for (TStringListIT it = m_pCommond->begin(); it != m_pCommond->end(); it++)
    {
        CxdString str = *it;
        pSrc = str.c_str();
        if ( CompareString(pSrc, ApName) || CompareString(pSrc + 1, ApName) )
        {
            return true;
        }
    }
    return !GetCommondValue(ApName).IsEmpty();
}

CString CxdApplication::GetCommondValue( const TCHAR* const ApName )
{
    CheckCommonList();
    TxdParseInfo info;
    const TCHAR* pSrc;
    for (TStringListIT it = m_pCommond->begin(); it != m_pCommond->end(); it++)
    {
        CxdString str = *it;
        if ( ParseFormatString(str.c_str(), TEXT("="), info) )
        {
            pSrc = info.First.c_str();
            if ( CompareString(pSrc, ApName) || CompareString(pSrc + 1, ApName) )
            {
                return info.Second.c_str();
            }
        }
    }
    return TEXT("");
}

TxdAppUpdate CxdApplication::GetAppUpdateState( void )
{
    if ( Assigned(m_pUpdate) )
    {
        m_CurUpState = m_pUpdate->GetUpdateState();
    }
    return m_CurUpState;
}

void CxdApplication::CheckAppUpdate( const TCHAR* ApConfigURL, int ADelaySecond /*= 300*/, const TCHAR* ApUpdateDirName )
{
    if ( Assigned(m_pUpdate) ) 
    {
        if ( ADelaySecond <= 0 )
        {
            m_pUpdate->CheckNow();
        }
        return;
    }
    m_CurUpState = auNULL;
    m_pUpdate = new CxdAppUpdate( ApConfigURL, ADelaySecond, ApUpdateDirName );
    m_pUpdate->SetItDownZip( NewInterfaceExByClass(CxdApplication, const TxdUpConfigInfo&, bool&, DoUpdateDownZip) );
    m_pUpdate->SetItUnZipFile( NewInterfaceExByClass(CxdApplication, const TxdUpConfigInfo&, bool&, DoUpdateUnZipFile) );
    m_pUpdate->SetItRunApp( NewInterfaceEx3ByClass(CxdApplication, const TxdUpConfigInfo&, bool&, CString&, DoUpdateRunApp) );
    m_pUpdate->SetItCloseSelf( NewInterfaceByClass(CxdApplication, const CxdAppUpdate*, DoUpdateCloseSelf) );
    m_pUpdate->SetItFinished( NewInterfaceByClass(CxdApplication, const CxdAppUpdate*, DoUpdateFinished) );
}
