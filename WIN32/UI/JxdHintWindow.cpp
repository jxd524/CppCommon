#include "stdafx.h"
#include "JxdHintWindow.h"
#include "JxduiDefines.h"

/*CxdHintWindow*/

IMPLEMENT_DYNAMIC(CxdHintWindow, CxdForm)

BEGIN_MESSAGE_MAP(CxdHintWindow, CxdForm)
    ON_WM_NCHITTEST()
    ON_WM_SIZE()
END_MESSAGE_MAP()

CxdHintWindow::CxdHintWindow(): CxdForm()
{
    m_MouseTransParent = true;
    SetBufferBitmapTimeout( 1000 );
    SetDoubleBuffer( true );    
    SetBroderColor( 0x666666 );
	SetTopMost( true );
    SetCaptionTopSpace( 2 );
    FontInfo()->SetFontColor( 0xFF666666 );
    SetMaxSize( 350, 0 );

    m_AnimateTime = 200;
	m_lgMode = LinearGradientModeVertical;
	m_dwStartColor = 0xFFFFFFFF;
	m_dwEndColor = 0xFFE4E5F0;
}

CxdHintWindow::~CxdHintWindow()
{
	
}

void CxdHintWindow::OnCreated()
{
    if ( !ImageInfo()->IsExsitsBitmap() )
    {
        ImageInfo()->SetBitmap( CreateGradientBmp(), false );
        ImageInfo()->SetBitmapItemCount( 1 );
        ImageInfo()->SetBitmapDrawStyle( dsStretchAll );
    }
}

void CxdHintWindow::OnGetCreateInfo( CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle )
{
    ANewClassName = CtxdHitWindow;
    AdwStyle = WS_POPUP;
    AdwExStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
    if ( GetMouseTransParent() )
    {
        AdwExStyle |= WS_EX_TRANSPARENT;
    }
}

void CxdHintWindow::ShowHintMessage( const TCHAR *ApHintText, const int &ALeft, const int &ATop)
{
    SetCaption( ApHintText );
    SetAutoSizeByCaption( 3, 3, GetMaxWidth() );

    Rect R( Position()->GetRect() );
    R.X = ALeft;
    R.Y = ATop;
    CalcScreenPos( R );
    Position()->SetLeft( R.X );
    Position()->SetTop( R.Y );
    Position()->NotifyPositionChanged();

    if ( !GetVisible() )
    {
        AnimateWindow( GetAnimateTime(), AW_BLEND | AW_SLIDE );
    }
    
    ShowWindow(SW_SHOWNOACTIVATE);
    Invalidate();
}

LRESULT CxdHintWindow::OnNcHitTest( CPoint point )
{
    if ( GetMouseTransParent() )
    {
        return HTTRANSPARENT;
    }
    return HTCLIENT;
}

LRESULT CxdHintWindow::WindowProc( UINT nMessage, WPARAM wParam, LPARAM lParam )
{
    switch ( nMessage )
    {
    case WM_ERASEBKGND:
        return OnEraseBkgnd( (HDC)wParam );
    	break;
    }
    return CxdForm::WindowProc(nMessage, wParam, lParam);
}

LRESULT CxdHintWindow::OnEraseBkgnd( HDC dc )
{
    int nW = Position()->GetWidth();
    int nH = Position()->GetHeight();
    Bitmap bmp( nW, nH );
    m_bForceDrawText = true;
    CopyBufferBitmap( &bmp, Rect(0, 0, nW, nH) );
    m_bForceDrawText = false;

    HBITMAP hBmp;
    bmp.GetHBITMAP( 0, &hBmp );
    HBRUSH m_hBkBrush = CreatePatternBrush( hBmp );
    DeleteObject( hBmp );

    FillRect(dc, CRect(0, 0, nW, nH), m_hBkBrush );

    return TRUE;
}

void CxdHintWindow::HideHintMessage( void )
{
    AnimateWindow( GetAnimateTime(), AW_BLEND | AW_SLIDE | AW_HIDE );
}

void CxdHintWindow::OnSize( UINT nType, int ANewWidth, int ANewHeight )
{
    if ( Assigned(m_pImgFormStyle) && m_pImgFormStyle->IsExsitsBitmap() )
    {
        CxdForm::OnSize(nType, ANewWidth, ANewHeight);
    }
    else
    {
        HRGN hRgn = CreateRoundRectRgn( 0, 0, ANewWidth, ANewHeight, 3, 3 );
        SetWindowRgn( hRgn, TRUE );
        DeleteObject( hRgn );
    }
}

Bitmap* CxdHintWindow::CreateGradientBmp()
{
	Bitmap *pBmp = new Bitmap( 10, 20, PixelFormat32bppARGB );
	LinearGradientBrush brush( Rect(0, 0, pBmp->GetWidth(), pBmp->GetHeight()), m_dwStartColor, m_dwEndColor, m_lgMode );
	Graphics G( pBmp );
	G.FillRectangle( &brush, Rect(0, 0, pBmp->GetWidth(), pBmp->GetHeight()) );
	return pBmp;
}

void CxdHintWindow::SetHintBKColor(DWORD startColor, DWORD endColor, LinearGradientMode lgMode)
{
	m_dwStartColor = startColor;
	m_dwEndColor = endColor;
	m_lgMode = lgMode;
    ImageInfo()->SetBitmap( CreateGradientBmp(), false );
    ImageInfo()->SetBitmapItemCount( 1 );
    ImageInfo()->SetBitmapDrawStyle( dsStretchAll );
}

//CxdHintManage
static CxdHintManage* _GHintManage = NULL;
CxdHintManage::CxdHintManage()
{
    if ( Assigned(_GHintManage) )
    {
        throw "only can create one CxdHintManage";
    }
    _GHintManage = this;
    m_hTimerHandle = 0;
    m_DeferShowTimeElapse = 800;
    m_PauseTimeElapse = 3000;
    m_pCurHintComponent = NULL;
    m_pHintWnd = NULL;
    InitInterface( CreateHintWindow );
}

CxdHintManage::~CxdHintManage()
{
    _GHintManage = NULL;
    SafeDeleteObject( m_pHintWnd );
    DeleteInterface( CreateHintWindow );
}

void CxdHintManage::ActivateHint( CxdUIComponent *ApObject )
{
    if ( m_pCurHintComponent != ApObject )
    {
        StopHintTimer();
    }
    m_pCurHintComponent = ApObject;
    m_tmMode = tmShow;
    if ( Assigned(m_pCurHintComponent) )
    {
        StartHintTimer( Assigned(m_pHintWnd) ? 100 : GetDeferShowTimeElapse() );   
    } 
}

void CxdHintManage::UnActiveHint( CxdUIComponent *ApObject )
{
    if ( Assigned(m_pCurHintComponent) && (INT64)m_pCurHintComponent == (INT64)ApObject )
    {
        m_tmMode = tmHide;
        StartHintTimer( Assigned(m_pHintWnd) ? 100 : GetDeferShowTimeElapse() );
    }
}

VOID CALLBACK CxdHintManage::TimerProc( __in HWND hwnd, __in UINT uMsg, __in UINT_PTR idEvent, __in DWORD dwTime )
{
    if ( Assigned(_GHintManage) )
    {
        _GHintManage->StopHintTimer();
        _GHintManage->BeginHandleHintWindow();
    }
}

void CxdHintManage::StartHintTimer( const DWORD &AValue )
{
    StopHintTimer();
    m_hTimerHandle = SetTimer(0, 0, AValue, TimerProc );
}

void CxdHintManage::BeginHandleHintWindow( void )
{
    if ( m_tmMode == tmHide )
    {
        if ( Assigned(m_pHintWnd) )
        {
            m_pHintWnd->HideHintMessage();
        }
        SafeDeleteObject(m_pHintWnd);
    }
    else
    {
        if ( Assigned(m_pCurHintComponent) && m_pCurHintComponent->OnHintMessage() )
        {
            CString *pInfo = m_pCurHintComponent->GetHintText();
            if ( !Assigned(pInfo) )
            {
                if ( Assigned(m_pHintWnd) )
                {
                    m_pHintWnd->HideHintMessage();
                }
                SafeDeleteObject(m_pHintWnd);
                return;
            }
            if ( !Assigned(m_pHintWnd) )
            {
                DoCreateHintWindow();
            }
            CPoint pt;
            GetCursorPos( &pt );
            m_pHintWnd->ShowHintMessage( *pInfo, pt.x + 5, pt.y + 15 );

            m_tmMode = tmHide;
            StartHintTimer( GetPauseTimeElapse() );
        }
        else if ( Assigned(m_pHintWnd) )
        {
            m_pHintWnd->HideHintMessage();
            SafeDeleteObject(m_pHintWnd);
        }
    }
}

void CxdHintManage::DoCreateHintWindow( void )
{
    m_pHintWnd = NULL;
    NotifyInterface( CreateHintWindow, &m_pHintWnd );
    if ( IsNull(m_pHintWnd) )
    {
        m_pHintWnd = new CxdHintWindow;
        m_pHintWnd->xdCreate( 10, 10, 1, 1, NULL, NULL );
    }
}

void CxdHintManage::StopHintTimer( void )
{
    if ( m_hTimerHandle != 0 )
    {
        KillTimer( 0, m_hTimerHandle );
        m_hTimerHandle = 0;
    }
}
