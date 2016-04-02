#include "stdafx.h"
#include "JxdForm.h"

#include "JxdEvent.h"
#include "JxduiCommon.h"
#include "JxduiDefines.h"
#include "JxdGdiPlusBasic.h"
#include "JxdDebugInfo.h"

IMPLEMENT_DYNAMIC(CxdForm, CDialog)

BEGIN_MESSAGE_MAP(CxdForm, CDialog)
    ON_WM_KEYDOWN()
    ON_WM_CLOSE()
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_NCHITTEST()
END_MESSAGE_MAP()

CxdForm::CxdForm(const TCHAR *ApClassName):CxdWinControl(), m_CtClassName(ApClassName), CtCloseTimerID( GetUnionID() )
{
    m_WindowState = wsNormal;
    m_bTopMost = false;
    m_bAutoChangedSize = false;
    m_bNeedSaveOldCursorStyle = true;
    m_csOldStyle = crDefault;
    m_pImgFormStyle = NULL;
    m_nCurStyleWidth = 0;
    m_nCurStyleHeight = 0;
    m_dwBorderColor = 0;
    m_nMinWidth = m_nMinHeight = 0;
    m_nMaxWidth = m_nMaxHeight = 0;
    m_hBroderBrush = 0;
    m_RoundValue = 0;
    SetAutoMoveForm( true );
    SetBufferBitmapTimeout( 0 );
    SetDoubleBuffer( true );
    //SetBroderColor( RGB(255, 0, 0) );
}

CxdForm::~CxdForm()
{
    if ( m_hBroderBrush != 0 )
    {
        DeleteObject( m_hBroderBrush );
        m_hBroderBrush = 0;
    }
    SafeDeleteObject( m_pImgFormStyle );
}

void CxdForm::OnInitWindowSize( const int &Ax, const int &Ay, const int &AWidth, const int &AHeight )
{
    int w( AWidth );
    if ( w <= 0 )
    {
        if ( Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() )
        {
            w = m_pImgInfo->GetBitmap()->GetWidth();
        }
        else if ( Position()->GetWidth() > 0 )
        {
            w = Position()->GetWidth();
        }
        else
        {
            w = 800;
        }
    }
    int h( AHeight );
    if ( h <= 0 )
    {
        if ( Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() )
        {
            h = m_pImgInfo->GetBitmap()->GetHeight();
        }
        else if ( Position()->GetHeight() > 0 )
        {
            h = Position()->GetHeight();
        }
        else
        {
            h = 600;
        }
    }
    int x( Ax );
    if ( x == -1 )
    {
        if ( Assigned(m_pParent) )
        {
            x = m_pParent->Position()->GetLeft() + (m_pParent->Position()->GetWidth() - w) / 2;
        }
        else
        {
            x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
        }
    }
    int y( Ay );
    if ( y == -1 )
    {
        if ( Assigned(m_pParent) )
        {
            y = m_pParent->Position()->GetTop() + (m_pParent->Position()->GetHeight() - h) / 2;
        }
        else
        {
            y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
        }
    }
    if ( x + Ax < 0 )
    {
         x = 0;
    }
    if ( y + Ay < 0 )
    {
         y = 0;
    }
    CxdWinControl<CDialog>::OnInitWindowSize( x, y, w, h );
}

void CxdForm::OnCancel()
{
    Close();
}

void CxdForm::OnClose()
{
    switch ( OnCloseForm() )
    {
    case csDestroy:
        if (this == AfxGetApp()->m_pMainWnd )
        {
//			_Animate_close_window( m_hWnd );
            PostQuitMessage(0);
        }
        else
        {
            EndDialog(0);
        }
    	break;
    case csHide:
        SetVisible( false );
        break;
    case csMini:
        PostMessage( WM_SYSCOMMAND, SC_MINIMIZE );
        break;
    }
}

TxdGpCloseStyle CxdForm::OnCloseForm( void )
{
    return csDestroy;
}

LRESULT CxdForm::OnNcHitTest( CPoint point )
{
    if ( GetTopActiveWidget() != this ) return HTCLIENT;

    if ( m_bAutoChangedSize && (wsNormal == m_WindowState) )
    {
        static const int CtSpace = 8;
        ScreenToClient(&point);
        CRect r;
        GetClientRect(&r);

        if ((point.x < CtSpace) && (point.y < CtSpace))
        {
            ChangedCursorStyle( crSizeNWSE );
            return HTTOPLEFT;
        }
        else if ((point.x > r.right - CtSpace) && (point.y < CtSpace))
        {
            ChangedCursorStyle( crSizeNESW );
            return HTTOPRIGHT;
        }
        if ((point.x < CtSpace) && (point.y > r.bottom - CtSpace))
        {
            ChangedCursorStyle( crSizeNESW );
            return HTBOTTOMLEFT;
        }
        else if ((point.x > r.right - CtSpace) && (point.y > r.bottom - CtSpace))
        {
            ChangedCursorStyle( crSizeNWSE );
            return HTBOTTOMRIGHT; 
        }
        else if (point.x < CtSpace)
        {
            ChangedCursorStyle( crSizeWE );
            return HTLEFT;
        }
        else if (point.x > r.right - CtSpace)
        {
            ChangedCursorStyle( crSizeWE );
            return HTRIGHT;
        }
        else if (point.y < CtSpace)
        {
            ChangedCursorStyle( crSizeNS );
            return HTTOP;
        }
        else if (point.y > r.bottom - CtSpace)
        {
            ChangedCursorStyle( crSizeNS );
            return HTBOTTOM;
        }
        else
        {
            m_bNeedSaveOldCursorStyle = false;
            Cursor()->SetCursorStyle( m_csOldStyle );
            //
            return HTCLIENT;
            //
            if ( GetAutoMoveForm() && (m_WindowState != wsMaximized) )
            {
                return HTCAPTION;
            }
            return HTCLIENT;
        }
    }
    //
    return HTCLIENT;
    //
    if ( GetAutoMoveForm() && (m_WindowState != wsMaximized) )
    {
        return HTCAPTION;
    }
    return HTCLIENT;
}

void CxdForm::SetWindowState( TWindowState AState )
{
    if ( AState != m_WindowState )
    {
        if ( !IsWindow(m_hWnd) )
        {
            m_WindowState = AState;
            return;
        }
        int nCmd;
        switch ( AState )
        {
        case wsMaximized:
            nCmd = SW_SHOWMAXIMIZED;
            break;
        case wsMinimized:
            nCmd = SW_MINIMIZE;
            break;
        default:
            nCmd = SW_SHOWNORMAL;
        }        
        ShowWindow( nCmd );
    }
}

void CxdForm::UpdateWindowState( void )
{
    WINDOWPLACEMENT Placement = {0};
    Placement.length = sizeof( WINDOWPLACEMENT );
    GetWindowPlacement( &Placement );
    switch( Placement.showCmd )
    {
    case SW_SHOWMAXIMIZED:
        m_WindowState = wsMaximized;
        break;
    case SW_SHOWMINIMIZED:
        m_WindowState = wsMinimized;
        break;
    default:
        m_WindowState = wsNormal;
        break;
    }
}

void CxdForm::OnWindowPosChanging( WINDOWPOS* lpwndpos )
{
    //if ( (lpwndpos->flags & SWP_NOSIZE) == 0 && !CheckNewSize(lpwndpos->cx, lpwndpos->cy) )
    //{
    //    lpwndpos->flags |= SWP_NOSIZE;
    //}
    static const DWORD SWP_STATECHANGED = 0x8000;
    if ( (lpwndpos->flags & SWP_STATECHANGED) == SWP_STATECHANGED )
    {
        UpdateWindowState();
    }
}

void CxdForm::ChangedCursorStyle( const TCursorStyle &ANewStyle )
{
    TCursorStyle old = Cursor()->GetCursorStyle();
    if ( old != ANewStyle )
    {        
        if ( m_bNeedSaveOldCursorStyle )
        {
            m_bNeedSaveOldCursorStyle = false;
            m_csOldStyle = old;
        }
        Cursor()->SetCursorStyle( ANewStyle );
    }
}

void CxdForm::DoCursorChanged( void* )
{
    if ( m_bNeedSaveOldCursorStyle )
    {
        m_csOldStyle = Cursor()->GetCursorStyle();
    }
}

void CxdForm::OnSize( UINT nType, int ANewWidth, int ANewHeight )
{
    if ( ANewHeight != 0 && ANewWidth != 0 )
    {
        if ( Assigned(m_pImgFormStyle) && m_pImgFormStyle->IsExsitsBitmap() &&
            (m_nCurStyleWidth != ANewWidth || m_nCurStyleHeight != ANewHeight) )
        {
            m_nCurStyleWidth = ANewWidth;
            m_nCurStyleHeight = ANewHeight;
            Bitmap bmp( ANewWidth, ANewHeight );
            Graphics bmpG( &bmp );
            DrawCommonImage( bmpG, Rect(0, 0, ANewWidth, ANewHeight), m_pImgFormStyle, uiNormal );
            HRGN hWndRgn = CreateStyleByBitmap( &bmp, 10 );
            SetWindowRgn( hWndRgn, TRUE );
            DeleteObject( hWndRgn );
        }
        else if ( m_RoundValue > 0 )
        {
            HRGN hWndRgn = CreateRoundRectRgn( 0, 0, ANewWidth, ANewHeight, m_RoundValue, m_RoundValue );
            SetWindowRgn( hWndRgn, TRUE );
            DeleteObject( hWndRgn );
        }
    }
}

void CxdForm::OnGetCreateInfo( CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle )
{
    ADefaultWndClassName = TEXT("#32770");
    ANewClassName = m_CtClassName;
    AdwStyle = WS_POPUP | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    AdwExStyle = 0;
    if ( m_bTopMost )
    {
        AdwExStyle |= WS_EX_TOPMOST;
    }
    if ( m_WindowState == wsMaximized )
    {
        AdwStyle |= WS_MAXIMIZE;
    }
    else if ( m_WindowState == wsMinimized )
    {
        AdwStyle |= WS_MINIMIZE;
    }
}

void CxdForm::SetBroderColor( const COLORREF &AColor )
{
    if ( AColor != m_dwBorderColor )
    {
        m_dwBorderColor = AColor;
        if ( 0 != m_hBroderBrush )
        {
            DeleteObject( m_hBroderBrush );
        }
        m_hBroderBrush = CreateSolidBrush( m_dwBorderColor );
    }
}

bool CxdForm::CheckNewSize( int &ANewWidth, int &ANewHeight )
{
    if ( m_nMinWidth > 0 && ANewWidth < m_nMinWidth )
    {
        ANewWidth = m_nMinWidth;
    }
    if ( m_nMinHeight > 0 && ANewHeight < m_nMinHeight )
    {
        ANewHeight = m_nMinHeight;
    }
    if ( m_nMaxWidth > 0 && ANewWidth > m_nMaxWidth )
    {
        ANewWidth = m_nMaxWidth;
    }
    if ( m_nMaxHeight > 0 && ANewHeight > m_nMaxHeight )
    {
        ANewHeight = m_nMaxHeight;
    }
    return true;
}

void CxdForm::SetMainIcon( DWORD AIconID )
{
    SendMessage( WM_SETICON, 1, LPARAM(LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE(AIconID))) );
}

void CxdForm::BringWindowToShow( void )
{
    HWND hWnd = GetSafeHwnd();
    if ( IsWindow(hWnd) )
    {
        if ( IsIconic() )
        {
            ShowWindow( SW_RESTORE );
        }
        else if ( !IsWindowVisible() )
        {
            ShowWindow( SW_SHOW );
        }
        ::SetForegroundWindow(hWnd);
    }
}

void CxdForm::SetTopMost( bool bTopMost )
{
    HWND h = GetSafeHwnd();
    if ( h != NULL )
    {
        if ( GetTopMost() != bTopMost )
        {
            HWND hStyle = bTopMost ? HWND_TOPMOST : HWND_NOTOPMOST;
            ::SetWindowPos( h, hStyle, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER );
            m_bTopMost = GetTopMost();
        }
    }
    else
    {
        m_bTopMost = bTopMost;
    }
}

bool CxdForm::GetTopMost( void )
{
    HWND h = GetSafeHwnd();
    if ( h != NULL )
    {
        return (::GetWindowLong(h, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
    }
    return m_bTopMost;
}

void CxdForm::OnGetMinMaxInfo( MINMAXINFO* lpMMI )
{
    int nMaxW = GetSystemMetrics( SM_CXFULLSCREEN ) + GetSystemMetrics(SM_CXDLGFRAME);
    int nMaxH = GetSystemMetrics( SM_CYFULLSCREEN ) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME);

    if ( m_nMaxWidth > 0 && m_nMaxWidth < nMaxW)
    {
        nMaxW = m_nMaxWidth;
    }
    if ( m_nMaxHeight > 0 && m_nMaxHeight < nMaxH )
    {
        nMaxH = m_nMaxHeight;
    }

    lpMMI->ptMaxSize.x = nMaxW;
    lpMMI->ptMaxSize.y = nMaxH;

    if ( m_nMinWidth > 0 )
    {
        lpMMI->ptMinTrackSize.x = m_nMinWidth;
    }    
    if ( m_nMinHeight > 0 )
    {
        lpMMI->ptMinTrackSize.y = m_nMinHeight;
    }
    if ( m_nMaxHeight > 0 ) lpMMI->ptMaxTrackSize.x = nMaxW;
    if ( m_nMaxWidth > 0) lpMMI->ptMaxTrackSize.y = nMaxH;
}

void CxdForm::OnLButtonDblClk( UINT, CPoint )
{
    if ( m_WindowState == wsMaximized )
    {
        SetWindowState( wsNormal );
    }
    else
    {
        SetWindowState( wsMaximized );
    }
}

void CxdForm::ShowModel( bool AbSetParentDisable, bool AbSetVisible )
{
    bool bParentEnable( Assigned(m_pParent) && m_pParent->GetEnable() );
    if ( AbSetParentDisable && Assigned(m_pParent) && m_pParent->GetEnable() )
    {
        m_pParent->SetEnable( false );
    }

    if ( AbSetVisible ) SetVisible( true );

    RunModalLoop( 0 );
    if ( Assigned(m_pParent) )
    {
        m_pParent->SetEnable( bParentEnable );
    }
}

void CxdForm::Close( const DWORD &ADeferTime )
{
    if ( ADeferTime == 0 )
    {
        PostMessage( WM_CLOSE, 0, 0 );
    }
    else
    {
        SetVisible( false );
        StartTimer( CtCloseTimerID, ADeferTime );
    }    
}

bool CxdForm::OnTimerMessage( int ATimeID, LPVOID ApData )
{
    if ( CtCloseTimerID == ATimeID )
    {
        PostMessage( WM_CLOSE, 0, 0 );
        return false;
    }
    return CxdWinControl<CDialog>::OnTimerMessage( ATimeID, ApData );;
}

void CxdForm::DoDrawSubjoinBK( Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt )
{
    CxdWinControl::DoDrawSubjoinBK( ADestG, ADestRect, ApImgAtt );

    if ( m_hBroderBrush != 0 )
    {
        HRGN h = CreateRectRgn( 0, 0, Position()->GetWidth(), Position()->GetHeight() );
        GetWindowRgn( h );

        HDC dc = ADestG.GetHDC();
        FrameRgn( dc, h, m_hBroderBrush, 1, 1 );
        ADestG.ReleaseHDC(dc);

        DeleteObject( h );
    }
}

BOOL CxdForm::_Animate_close_window( HWND hWnd )
{
	::SetWindowLong(hWnd, GWL_EXSTYLE,GetWindowLong(hWnd,GWL_EXSTYLE)^0x80000);
	RECT rect;
	::GetWindowRect(hWnd,&rect);
	//::SetWindowLong(m_hWnd, GWL_EXSTYLE,GetWindowLong(m_hWnd,GWL_EXSTYLE)^0x80000);
	for(int i=0;i<11;i++)
	{
		::SetWindowPos(hWnd,NULL,rect.left,rect.top-7*i,0,0,SWP_NOSIZE | SWP_SHOWWINDOW);
		::SetLayeredWindowAttributes(hWnd,0,(10-i)*25,LWA_ALPHA);
		Sleep(1000);
	} 
	return TRUE;
}
