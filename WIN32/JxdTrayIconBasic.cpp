#include "stdafx.h"
#include "JxdTrayIconBasic.h"

DWORD CxdTrayIconBasic::WM_JXDTRAYICONMESSAGE = WM_USER + 524;
HHOOK CxdTrayIconBasic::m_hHook = NULL;
map<HWND, CxdTrayIconBasic*> CxdTrayIconBasic::m_hMap;
UINT CxdTrayIconBasic::m_uIDs = 524;

CxdTrayIconBasic::CxdTrayIconBasic(): CtNotifyDataSize( sizeof(NOTIFYICONDATA) )  
{
    CString str( TEXT("PP¾«Áé") );
    memset( &m_NotifyData, 0, CtNotifyDataSize );
    m_NotifyData.cbSize = CtNotifyDataSize;
    m_NotifyData.uCallbackMessage = WM_JXDTRAYICONMESSAGE;
    m_NotifyData.uID = m_uIDs++;
    wmemcpy( m_NotifyData.szTip, str, str.GetLength() );
    InitInterface( TrayClick );
}

CxdTrayIconBasic::~CxdTrayIconBasic()
{
    DeleteInterface( TrayClick );
    UnTrayIcon();
}

LRESULT CALLBACK CxdTrayIconBasic::CallWindowProc(int code, WPARAM wParam, LPARAM lParam)
{
    if ( HC_ACTION == code )
    {
        CWPSTRUCT *pMsg = (CWPSTRUCT*)lParam;
        if ( pMsg->message == WM_JXDTRAYICONMESSAGE )
        {
            map<HWND, CxdTrayIconBasic*>::const_iterator it = m_hMap.find( pMsg->hwnd );
            ASSERT( it != m_hMap.end() );
            CxdTrayIconBasic *pTray = it->second;
            ASSERT( pTray != NULL );
            pTray->OnTrayNotifyEvent( pMsg->wParam, pMsg->lParam );
            return 0;
        }
    }
    return CallNextHookEx( m_hHook, code, wParam, lParam );
}

bool CxdTrayIconBasic::SetTrayIconHandle(HWND hwnd, UINT dwIconID)
{
    if ( NULL == m_hHook )
        m_hHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWindowProc, NULL, GetCurrentThreadId() );
    if ( NULL == m_hHook ) return false;

    m_NotifyData.hWnd = hwnd;
    m_NotifyData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    0 == dwIconID ? m_NotifyData.hIcon = 0 : m_NotifyData.hIcon = AfxGetApp()->LoadIcon( dwIconID );
    if ( !Shell_NotifyIcon(NIM_ADD, &m_NotifyData) ) return false;

    m_hMap[hwnd] = this;
    return true;
}

bool CxdTrayIconBasic::ShowBalloonToolTip(const std::wstring &strText, 
    const std::wstring &strCaption, DWORD btsStyle, UINT uTimeOut)
{
    if ( NULL == m_hHook && m_hMap.find(m_NotifyData.hWnd) == m_hMap.end() ) return false;
    m_NotifyData.uFlags |= NIF_INFO;
    wmemcpy( m_NotifyData.szInfo, strText.c_str(), strText.length() );
    wmemcpy( m_NotifyData.szInfoTitle, strCaption.c_str(), strCaption.length() );
    m_NotifyData.uTimeout = uTimeOut;
    m_NotifyData.dwInfoFlags = btsStyle;
    return TRUE == Shell_NotifyIcon( NIM_MODIFY, &m_NotifyData );
}

bool CxdTrayIconBasic::SetIcon(UINT dwIconID)
{
    if ( NULL == m_hHook && m_hMap.find(m_NotifyData.hWnd) == m_hMap.end() ) return false;
    0 == dwIconID ? m_NotifyData.hIcon = 0 : m_NotifyData.hIcon = AfxGetApp()->LoadIcon( dwIconID );
    return TRUE == Shell_NotifyIcon( NIM_MODIFY, &m_NotifyData );
}

void CxdTrayIconBasic::UnTrayIcon()
{
    if ( m_hMap.size() == 0 )
    {
        UnhookWindowsHookEx( m_hHook );
        m_hHook = NULL;
    }
    if ( 0 != m_NotifyData.hWnd )
    {
        Shell_NotifyIcon( NIM_DELETE, &m_NotifyData );
    }
}

bool CxdTrayIconBasic::SetHintText(const CString &AHintText)
{
    if ( NULL == m_hHook && m_hMap.find(m_NotifyData.hWnd) == m_hMap.end() )
    {
        return false;
    }
    int nlen( AHintText.GetLength() );
    if (nlen > 128) 
    {
        nlen = 128;
    }
    wmemcpy( m_NotifyData.szTip, AHintText, nlen );
    m_NotifyData.szTip[ nlen ] = 0;
    return TRUE == Shell_NotifyIcon( NIM_MODIFY, &m_NotifyData );
}

void CxdTrayIconBasic::OnTrayNotifyEvent(WPARAM wID, LPARAM lEvent)
{
    ASSERT( m_NotifyData.uID == wID );
    TTrayClickType ct = tctLBtnClick;
    switch( lEvent )
    {
    case WM_LBUTTONDBLCLK:
        ct = tctLBtnDbClick;
        break;
    case WM_RBUTTONUP:
        ct = tctRBtnClick;
        break;
    }
    NotifyInterface( TrayClick, ct );
}
