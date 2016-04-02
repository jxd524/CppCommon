#include "stdafx.h"
#include "JxdEdit.h"
#include "JxdGdiPlusBasic.h"
#include "JxdSysSub.h"


IMPLEMENT_DYNAMIC(CxdEdit, CEdit)

BEGIN_MESSAGE_MAP(CxdEdit, CEdit)
    ON_WM_GETDLGCODE()
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR_REFLECT()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_WM_CHAR()
    ON_WM_KEYDOWN()
	ON_CONTROL_REFLECT( EN_CHANGE, OnEditChanged )
END_MESSAGE_MAP()

CxdEdit::CxdEdit(): CxdWinControl<CEdit>()
{
    m_bMulitLine = false;
    m_bTransParent = false;
    m_bPassword = false;
    m_bOnlyNumber = false;
    m_bTextInLeft = true;
    m_ShowVScroll = false;
    m_hBkBrush = 0;
    m_nBrightness = 80;
    m_fTranslate = 0.5f;
    m_nBroderWidth = 1;
    m_nSpace = 3;
    m_dwNormalBroderColor = 0xFF0F0F0F;
    m_dwActiveBroderColor = 0xFF013FAF;
    m_dwSpaceColor = 0xFFFEFEFE;
    m_dwNormatTextColor = 0;
    m_dwActiveTextColor = 0xFF666666;
    m_TipTextColor = RGB(128, 128, 128);
    m_pHitFont = NULL;
    m_bResetFont = false;
    m_bCurIsHitText = false;  
    m_bEnterDown = false;
	m_nStart = 0;
	m_nEnd = 0;
	m_bNoChooseState = false;
    InitInterface( OnPaste );
    InitInterface( OnChar );
    InitInterface( OnMidMouseWheel );
	InitInterface( OnEnEditChange );
}

CxdEdit::~CxdEdit()
{
    ClearHitFont();
    ClearBkBrush();

    if ( Assigned(m_pParent) )
    {
        m_pParent->Position()->ClearEventByOwner( (INT64)this );   
    }
    DeleteInterface( OnPaste );
    DeleteInterface( OnChar );
    DeleteInterface( OnMidMouseWheel );
	DeleteInterface( OnEnEditChange );
}

UINT CxdEdit::OnGetDlgCode()
{
    if ( m_bTransParent )
    {
        RedrawWindow( NULL, NULL, RDW_ERASE | RDW_INVALIDATE );
    }
    else
    {
        SetEditRect();
    }    
    return CEdit::OnGetDlgCode();
}

HBRUSH CxdEdit::CtlColor( CDC* pDC, UINT nCtlColor )
{
    pDC->SetBkMode( m_bTransParent ? TRANSPARENT : OPAQUE );
    pDC->SetTextColor( m_bCurIsHitText ? m_TipTextColor : (IsCurNormal() ? m_dwNormatTextColor : m_dwActiveTextColor) );
    CreateBkBrush();
    return m_hBkBrush;
}

void CxdEdit::OnGetCreateInfo( CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle )
{
#define CheckToAddStyle(bValue, AStyleOnTrue, AStyleOnFalse) \
    bValue ? AdwStyle |= AStyleOnTrue : AdwStyle |= AStyleOnFalse;

    ADefaultWndClassName = TEXT("EDIT");
    ANewClassName = TEXT("xdEdit");

    AdwStyle = WS_CHILD | ES_MULTILINE | WS_TABSTOP;
    if ( m_ShowVScroll ) AdwStyle |= WS_VSCROLL;

    CheckToAddStyle( m_bMulitLine, ES_AUTOVSCROLL | ES_WANTRETURN, ES_AUTOHSCROLL );
    CheckToAddStyle( m_bPassword, ES_PASSWORD, 0 );
    CheckToAddStyle( m_bOnlyNumber, ES_NUMBER, 0 );	
    CheckToAddStyle( m_bTextInLeft, ES_LEFT, ES_RIGHT );
    AdwExStyle = 0;
}

void CxdEdit::CreateBkBrush( void )
{
    if ( m_hBkBrush == 0 )
    {
        Bitmap *pBmp = NULL;
        if ( m_bTransParent )
        {
            pBmp = new Bitmap( Position()->GetWidth(), Position()->GetHeight(), PixelFormat32bppARGB );
            m_pParent->CopyBufferBitmap( pBmp, Position()->GetRect() );
            ChangedBmpBrightness( pBmp, m_nBrightness );
        }
        if ( Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() )
        {
            bool bSetBK = false;
            if ( !Assigned(pBmp) )
            {
                pBmp = new Bitmap( Position()->GetWidth(), Position()->GetHeight(), PixelFormat32bppARGB );   
                bSetBK = true;
            }
            Graphics G( pBmp );
            if ( bSetBK )
            {
                G.FillRectangle( &SolidBrush(m_dwSpaceColor), 0, 0, pBmp->GetWidth(), pBmp->GetHeight() );
            }
            if ( m_bTransParent )
            {
                ImageAttributes imageAtt;
                InitTranslateImgAtt( imageAtt, m_fTranslate );
                DrawCommonImage( G, Rect(0, 0, Position()->GetWidth(), Position()->GetHeight()), m_pImgInfo, GetCurUISate(), &imageAtt );
            }
            else
            {
                DrawCommonImage( G, Rect(0, 0, Position()->GetWidth(), Position()->GetHeight()), m_pImgInfo, GetCurUISate() );
            }            
        }

        if ( Assigned(pBmp) )
        {
            HBITMAP hBmp;
            pBmp->GetHBITMAP( 0, &hBmp );
            m_hBkBrush = CreatePatternBrush( hBmp );
            DeleteObject( hBmp );
            delete pBmp;
        }
        else
        {
            m_hBkBrush = CreateSolidBrush( RGB(255, 255, 255) );
        }
    }
}

void CxdEdit::ClearBkBrush( void )
{
    if ( m_hBkBrush != 0 )
    {
        DeleteObject( m_hBkBrush );
        m_hBkBrush = 0;
    }
}

void CxdEdit::_OnWndPaint( bool &ACallDefault )
{
    SetEditRect();    
    ACallDefault = false;
    CEdit::OnPaint();

    HDC dc = ::GetDC( GetSafeHwnd() );
    GetClientRect( &m_rCurReDrawRect );
    Graphics G(dc);        
    if ( m_nSpace > 0 )
    {
        REAL nW( REAL(m_nBroderWidth + m_nSpace) );
        Pen pen( m_dwSpaceColor, nW );
        pen.SetAlignment( PenAlignmentInset );
        Rect R(0, 0, m_rCurReDrawRect.Width(), m_rCurReDrawRect.Height() );
        if ( nW == 1 )
        {
            R.Width -= 1; 
            R.Height -= 1;
        }
        G.DrawRectangle( &pen, R );
    }
    if ( m_nBroderWidth > 0 )
    {
        Pen pen( IsCurNormal() ? m_dwNormalBroderColor : m_dwActiveBroderColor, (REAL)m_nBroderWidth );
        pen.SetAlignment( PenAlignmentInset );
        Rect R(0, 0, m_rCurReDrawRect.Width(), m_rCurReDrawRect.Height() );
        if ( m_nBroderWidth == 1 )
        {
            R.Width -= 1;
            R.Height -= 1;
        }
        G.DrawRectangle( &pen, R );
    }

    DrawEditSubComponentBK( G );
    DoDrawSubComponent( G, Rect(0, 0, Position()->GetWidth(), Position()->GetHeight()) );
    ::ReleaseDC( GetSafeHwnd(), dc );
    m_rCurReDrawRect = CRect(0, 0, 0, 0);
}

void CxdEdit::SetEditRect( void )
{
    CRect R = CalcEditRect();
    SendMessage( EM_SETRECT/*EM_SETRECTNP*/, 0, LPARAM(&R) );  //EM_SETRECTNP
}

void CxdEdit::OnCreated( void )
{
    if ( m_bTransParent )
    { 
        m_pParent->Position()->AddEventPositionChanged( NewInterfaceByClass(CxdEdit, CxdPosition*, OnEdtParentPositionChanged) );
    }
    if ( m_bPassword )
    {
        SetPasswordChar( '*' );
    }
}
 
bool CxdEdit::OnUIStateChanged( const TxdGpUIState &ANewSate )
{
    if ( Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() && m_pImgInfo->GetBitmapItemCount() > 1 )
    {
        ClearBkBrush();
        return CxdWinControl::OnUIStateChanged(ANewSate);
    }
    else
    {
        return CxdUIComponent::OnUIStateChanged(ANewSate);
    }
}

void CxdEdit::OnEdtParentPositionChanged( CxdPosition* ApPosition )
{
    if ( m_bTransParent )
    {
        ClearBkBrush();
    }
    Invalidate();
}

bool CxdEdit::IsCurNormal( void )
{
    HWND hFocus = ::GetFocus();
    return /*( GetCurUISate() == uiNormal ) && */( m_hWnd != hFocus );
}

void CxdEdit::OnKillFocus( CWnd* pNewWnd )
{
    if ( !m_strTipTextAtNull.IsEmpty() )
    {
        CString strText;
        GetWindowText( strText );
        if ( strText.IsEmpty() )
        {
            if ( Assigned(m_pHitFont) )
            {
                m_bResetFont = true;
                m_pOldFont = GetFont();
                SetFont(m_pHitFont);
            }
            m_bCurIsHitText = true;
            if ( m_bPassword )
            {
                SetPasswordChar( 0 );
            }
            SetWindowText( m_strTipTextAtNull );
        }
    } 
    Default();
 	GetSel( m_nStart, m_nEnd );
}

void CxdEdit::OnSetFocus( CWnd* pOldWnd )
{
    m_bCurIsHitText = false;
    if ( !m_strTipTextAtNull.IsEmpty() )
    {
        CString strText;
        GetWindowText( strText );
        if ( strText == m_strTipTextAtNull )
        {
            SetWindowText( TEXT("") );
        }
        if ( m_bResetFont && Assigned(m_pOldFont) )
        {
            SetFont( m_pOldFont );
        }
        m_bResetFont = false;
        if ( m_bPassword )
        {
            SetPasswordChar( '*' );
        }
    }
    Default();
 	SetSel( m_nStart, m_nEnd );
}

CString CxdEdit::GetCaption( void )
{
    CString strCaption;
    GetWindowText( strCaption );
    if ( !m_strTipTextAtNull.IsEmpty() && strCaption.Compare(m_strTipTextAtNull) == 0 )
    {
        return TEXT("");
    }
    return strCaption;
}

void CxdEdit::SetCaption( const TCHAR* ApCaption )
{
    CxdWinControl<CEdit>::SetCaption( ApCaption );

    if ( GetFocus() == this )
    {
        SetSel(0xFFFFFF, 0xFFFFFFFF );
    }
}

void CxdEdit::SetTipTextAtNull( const TCHAR *ApHitText )
{
    if ( m_strTipTextAtNull.Compare(ApHitText) != 0 )
    {
        m_strTipTextAtNull = ApHitText;
        OnKillFocus( NULL );
    }
}

void CxdEdit::SetTipTextFont( CFont *ApFont, bool AOnlyRes /*= true*/ )
{
    ClearHitFont();
    m_pHitFont = ApFont;
    m_bResetFont = AOnlyRes;
}

void CxdEdit::ClearHitFont( void )
{
    if ( !m_bResHitFont )
    {
        SafeDeleteObject( m_pHitFont );
    }
    m_pHitFont = NULL;
    m_bResHitFont = true;
}

bool CxdEdit::SettingWndStyle( const bool &ANewValue, bool &ABeSetValue, int AStyleValue )
{
    bool bOK = true;
    if ( ANewValue != ABeSetValue )
    {
        if ( IsWindow(m_hWnd) )
        {
            bOK = SetWndStyle( AStyleValue, ANewValue );
        }
        else
        {
            bOK = true;
        }
        if ( bOK )
        {
            ABeSetValue = ANewValue;
        }
    }
    return bOK;
}

bool CxdEdit::SetPassword( bool bValue )
{
    if ( SettingWndStyle(bValue, m_bPassword, ES_PASSWORD) )
    {
        if ( IsWindow(m_hWnd) )
        {
            m_bPassword ? SetPasswordChar( '*' ) : SetPasswordChar( 0 );
            SetEditRect();
        }
        return true;
    }
    return false;
}

CSize CxdEdit::CalcBestSize( const int &ASetWidth, const int &ALineCount /*= 1*/ )
{
    CFont *pFont = GetFont();
    if ( !pFont )
    {
        return 0;
    }

    HDC dc = ::GetDC( GetSafeHwnd() );    
    HDC memDC = CreateCompatibleDC( dc );
    SelectObject( memDC, pFont->GetSafeHandle() );
    SIZE size;
    CString str( TEXT("国") );
    ::GetTextExtentPoint32(memDC, str, str.GetLength(), &size);
    DeleteDC(memDC);
    ::ReleaseDC( GetSafeHwnd(), dc );

    size.cy = size.cy * ( ALineCount >= 1 ? ALineCount : 1) + (m_nSpace + m_nBroderWidth) * 2;
    size.cx = ASetWidth;//(ASetWidth / size.cx) * size.cx;// + (m_nSpace + m_nBroderWidth) * 2 ;
    return size;
}

CRect CxdEdit::CalcEditRect( void )
{
    CRect R;
    R.left = m_nBroderWidth + m_nSpace ;
    R.top = m_nBroderWidth + m_nSpace;    
    R.right = Position()->GetWidth() - m_nBroderWidth - m_nSpace;// - 2;
    R.bottom = Position()->GetHeight() - m_nBroderWidth - m_nSpace;
    return R;
}

bool CxdEdit::SetTextInLeft( bool bValue )
{
    if ( bValue != m_bTextInLeft )
    {        
        if ( IsWindow(m_hWnd) )
        {
            SetWndStyle( bValue ? ES_LEFT : ES_RIGHT, true );
        }
        m_bTextInLeft = bValue;
        return true;
    }
    return false;
}

void CxdEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if ( IsExsitsItOnChar() )
    {
        TxdEditOnChar t;
        t.bCallDefault = true;
        t.nChar = nChar;
        t.nFlags = nFlags;
        t.nRepCnt = nRepCnt;
        m_pItOnChar->Notify( this, &t );
        if ( !t.bCallDefault ) return;
    }
	CxdWinControl<CEdit>::OnChar( nChar, nRepCnt, nFlags );
}

void CxdEdit::OnMidMouseWheel( UINT nFlags, short zDelta, int x, int y )
{
    if ( IsExsitsItOnMidMouseWheel() )
    {
        TxdEditMidMouseWheel t;
        t.nFlags = nFlags;
        t.x = x;
        t.y = y;
        t.zDelta = zDelta;
        m_pItOnMidMouseWheel->Notify( this, &t );
    }
    CxdWinControl<CEdit>::OnMidMouseWheel( nFlags, zDelta, x, y );
}

void CxdEdit::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if ( IsCtrlPressed() && (nChar == 'a' || nChar == 'A') )
    {
        SetSel(0, 0xFFFFFFFF );
        return;
    }
    CxdWinControl<CEdit>::OnKeyDown( nChar, nRepCnt, nFlags );
}

LRESULT CxdEdit::WindowProc( UINT nMessage, WPARAM wParam, LPARAM lParam )
{
    switch ( nMessage )
    {
    case WM_PASTE:
        {
            CxdWinControl<CEdit>::WindowProc(nMessage, wParam, lParam);
            NotifyInterface( OnPaste, this );
            return TRUE;
        }
    default:
        break;
    }
    return CxdWinControl<CEdit>::WindowProc(nMessage, wParam, lParam);
}

void CxdEdit::OnEditChanged()
{
	NotifyInterface( OnEnEditChange, this );
}
