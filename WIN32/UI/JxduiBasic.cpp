#include "stdafx.h"
#include "JxduiBasic.h"
#include "JxdGdiPlusBasic.h"
#include <algorithm>

//CxdUIComponent
CxdUIComponent::CxdUIComponent(): CtAnimateUnVisibleTimerID( GetUnionID() ), CtClearBufferResTimerID( GetUnionID() )
{
    m_bDrawByParent = false;
    m_DrawGrayOnUnEnable = false;
    m_bForceDrawText = false;
    m_nAnimateunVisibleElapse = 80;
    m_bCurAnimateUnVisible = false;
    m_AnimateUnVisible = false;
    m_fUnVisibleTranslate = 1.0f;
    m_bMouseInWidget = false;
    m_VisibleOnParentActive = false;
    m_bIsWndObject = false;
    Tag = 0;
    m_nOldParentWidth = 0;
    m_nOldParentHeight = 0;
    m_bAutoMoveForm = false;
    m_pAutoMoveForm = NULL;
    m_pCursor = NULL;
    m_pImgInfo = NULL;   
    m_pAnchors = NULL;
    m_pParent = NULL;
    m_eUIState = uiNormal;
    m_BufferBitmapTimeout = 1;
    m_pBufBitmap = NULL;
    m_pActiveWidget = NULL;
    m_pFontInfo = NULL;
    m_pStrHintText = NULL;
    m_CaptureOnMouseDown = false;
    m_nCaptionTop = 0;
    m_nCaptionLeft = 0;
    m_bCreated = false;

    InitInterface( Click );  //点击
    InitInterface( RButtonClick );
    InitInterface( DblClick ); //双击
    InitInterface( UIStateChanged ); //UI状态改变时
    InitInterface( OnHintMessage ); //当显示提示信息时
}

CxdUIComponent::~CxdUIComponent()
{
    ClearTimerInfo();

    if ( Assigned(m_pAnchors) ) m_pAnchors->ClearEventByOwner( (INT64)this );
    if ( Assigned(m_pParent) )
    {
        m_pParent->DeleteSubComponent( this );
        m_pParent->Position()->ClearEventByOwner( (INT64)this );
    }
    m_pParent = NULL;
   
    ClearSubComponents();
    SafeDeleteObject( m_pCursor );
    SafeDeleteObject( m_pImgInfo );
    SafeDeleteObject( m_pAnchors );
    SafeDeleteObject( m_pFontInfo );
    SafeDeleteObject( m_pStrHintText );
    SafeDeleteObject( m_pBufBitmap );
    DeleteInterface( Click );  //点击
    DeleteInterface( RButtonClick );
    DeleteInterface( DblClick ); //双击
    DeleteInterface( UIStateChanged ); //UI状态改变时
    DeleteInterface( OnHintMessage ); //当显示提示信息时
}

void CxdUIComponent::OnPaintComponent( Graphics &G, const Rect &ADestR )
{
    DoDrawUI( G, ADestR );
    DoDrawSubComponent( G, ADestR );
}

void CxdUIComponent::DoDrawUI( Graphics &G, const Rect &ADestR )
{
    ImageAttributes *pImgAtt = GetDrawBKImageAttributes();
    if ( Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() )
    {
        DrawCommonImage( G, ADestR, m_pImgInfo, GetCurUISate(), pImgAtt );
    }
    DoDrawSubjoinBK( G, ADestR, pImgAtt );
    DoDrawText( G, ADestR );
    SafeDeleteObject( pImgAtt );
}

void CxdUIComponent::DoDrawText( Graphics &G, const Rect &AuiDestR )
{
    if ( IsDrawCaption() )
    {
        CString strCaption = GetCaption();
        if ( !strCaption.IsEmpty() )
        {
            REAL f( m_bForceDrawText || GetVisible() ? 1.0f : 0.0f );
            if ( !m_bForceDrawText && IsCurAnimateUnVisible() )
            {
                f = GetCurAnimateTranslate();
            }
            DWORD fontColor = FontInfo()->GetFontColor(); 
            m_DrawGrayOnUnEnable && !IsEnable() ? fontColor = 0xFF808080 : DoChangedFontColor( fontColor );
            RectF R( REAL(AuiDestR.X + m_nCaptionLeft), REAL(AuiDestR.Y + m_nCaptionTop), 
                REAL(AuiDestR.Width - m_nCaptionLeft), REAL(AuiDestR.Height - m_nCaptionTop) );
            DoChangedFontRect( R );
            DrawCommonText( G, strCaption, FontInfo(), R, fontColor, GetCurUISate(), f );
        }
    }
}

void CxdUIComponent::DoDrawSubComponent( Graphics &G, const Rect &AuiDestR )
{
    CRect rReal;
    for ( TxdComponentListIT it = m_ltComponents.begin(); it != m_ltComponents.end(); it++ )
    {
        CxdUIComponent *pObj = *it;
        if ( pObj->m_bDrawByParent && pObj->GetVisible() || pObj->m_bCurAnimateUnVisible || (pObj->DoGetVisible() && pObj->IsCurAnimateUnVisible()) )
        {
            rReal = pObj->GetRealRectOnUI();
            pObj->m_bHasSubUINotDraw = false;
            if ( IsValideCRect(rReal) && IsNeedDraw(rReal) )
            {
                //if ( Tag != 0 )
                //{
                    //CString strInfo;
                    //strInfo.Format( TEXT("left: %d, top: %d, right: %d, bottom: %d\r\n"), 
                    //    rReal.left, rReal.top, rReal.right, rReal.bottom);
                    //OutputDebugString( strInfo );
                //}
                int nX = 0;
                int nY = 0;
                int nW = pObj->Position()->GetWidth();
                int nH = pObj->Position()->GetHeight();
                Rect R( pObj->Position()->GetRect() );
                if ( R.X < 0 )
                {
                    int nSpace( -R.X );
                    R.X = 0;
                    R.Width -= nSpace;
                    nX = nSpace;
                }
                if ( R.Y < 0 )
                {
                    int nSpace( -R.Y );
                    R.Y = 0;
                    R.Height -= nSpace;
                    nY = nSpace;
                }
                G.DrawImage( pObj->GetBufferBitmap(), R, nX, nY, R.Width, R.Height, UnitPixel );

                pObj->IsHasSubUINotDraw() ? pObj->ClearBufferBitmap() : pObj->DeferClearBufferBitmap();
            }
            else
            {
                pObj->m_bHasSubUINotDraw = true;
            }
        }
    }
}

bool CxdUIComponent::OnUIStateChanged( const TxdGpUIState &ANewSate )
{
    if ( m_eUIState != ANewSate )
    {
        m_eUIState = ANewSate;
        ClearBufferBitmap();
        bool bVisibleOnActive = m_bMouseInWidget;
        for ( TxdComponentListIT it = m_ltComponents.begin(); it != m_ltComponents.end(); it++ )
        {
            CxdUIComponent *pObj = *it;
            if ( pObj->GetVisibleOnParentActive() && pObj->IsEnable() )
            {
                pObj->SetVisible( bVisibleOnActive );
            }
        }
        NotifyInterface( UIStateChanged, this );
        return true;
    }
    return false;
}

ImageAttributes* CxdUIComponent::GetDrawBKImageAttributes( void )
{
    ImageAttributes *pImgAtt = NULL;
    if ( m_DrawGrayOnUnEnable && !IsEnable() )
    {
        pImgAtt = new ImageAttributes;
        ColorMatrix cm;
        ColorMatrix_ToIdentity( cm );
        ColorMatrix_Gray( cm );
        pImgAtt->SetColorMatrix( &cm );
    }
    else if ( IsCurAnimateUnVisible() )
    {
        if ( m_bCurAnimateUnVisible )
        {
            m_fUnVisibleTranslate -= 0.2f;
        }
        REAL fV = GetCurAnimateTranslate();

        pImgAtt = new ImageAttributes;
        ColorMatrix cm;
        ColorMatrix_ToIdentity( cm );
        ColorMatrix_TranslateValue( cm, fV );
        pImgAtt->SetColorMatrix( &cm );
        if ( fV - 0.001f > 0.0f )
        {
            if ( m_bCurAnimateUnVisible )
            {
                StartTimer( CtAnimateUnVisibleTimerID, m_nAnimateunVisibleElapse );
            }                
        }
        else
        {
            if ( m_bCurAnimateUnVisible )
            {
                m_bCurAnimateUnVisible = false;
            }
        }
    }
    return pImgAtt;
}

bool CxdUIComponent::CopyBufferDC( HDC AhDestDC, const Rect &ACopyRect )
{
    GetBufferBitmap();
    int nW = ACopyRect.Width <= Position()->GetWidth() ? ACopyRect.Width : Position()->GetWidth();
    int nH = ACopyRect.Height <= Position()->GetHeight() ? ACopyRect.Height : Position()->GetHeight();

    Graphics G( AhDestDC );
    Rect R(0, 0, nW, nH);
    G.DrawImage( m_pBufBitmap, R, ACopyRect.X, ACopyRect.Y, nW, nH, UnitPixel );

    return true;
}

bool CxdUIComponent::CopyBufferBitmap( Bitmap *ApDestBmp, const Rect &ACopyRect )
{
    GetBufferBitmap();
    int nW = ACopyRect.Width <= Position()->GetWidth() ? ACopyRect.Width : Position()->GetWidth();
    int nH = ACopyRect.Height <= Position()->GetHeight() ? ACopyRect.Height : Position()->GetHeight();

    Graphics G( ApDestBmp );
    Rect R(0, 0, nW, nH);
    G.DrawImage( m_pBufBitmap, R, ACopyRect.X, ACopyRect.Y, nW, nH, UnitPixel );

    return true;
}

void CxdUIComponent::Invalidate( void )
{
    if ( Assigned(m_pParent) && m_pParent->GetBufferBitmapTimeout() > 0 )
    {
        m_pParent->ClearBufferBitmap();
    }
    Rect oldR( Position()->GetOldRect() );
    Position()->ClearOldPosition();
    if ( oldR.X == 0 && oldR.Y == 0 && oldR.Width == 0 && oldR.Height == 0 )
    {
        InvalidateRect( NULL );
        return;
    }
    Rect curR( Position()->GetRect() );
    CRect R( oldR.X > 0 && oldR.X < curR.X ? oldR.X : curR.X, 
             oldR.Y > 0 && oldR.Y < curR.Y ? oldR.Y : curR.Y, 
             0, 0 );
    
    int nW = oldR.Width < curR.Width ? curR.Width : oldR.Width;
    R.right = R.left + nW;

    if ( oldR.X + nW > R.right )
    {
        R.right = oldR.X + nW;
    }
    int nH = oldR.Height < curR.Height ? curR.Height : oldR.Height;
    R.bottom = R.top + nH;
    if ( oldR.Y + nH > R.bottom )
    {
        R.bottom = oldR.Y + nH;
    }
    m_pParent->InvalidateRect( &R );
}

bool CxdUIComponent::AddSubComponent( CxdUIComponent *ApObject )
{
    bool bAdd( true );
    for ( TxdComponentListIT it = m_ltComponents.begin(); it != m_ltComponents.end(); it++ )
    {
        CxdUIComponent *pObj = *it;
        if ( pObj == ApObject )
        {
            bAdd = false;
            break;
        }
    }
    if ( bAdd )
    {
        m_ltComponents.push_back( ApObject );
    }
    return true;
}

void CxdUIComponent::OnClick( const int &x /*= 0*/, const int &y /*= 0*/ )
{
    NotifyInterface( Click, this );
}

void CxdUIComponent::DoHandleParentSizeChanged( const int &ANewWidth, const int &ANewHeight )
{
    if ( !Assigned(m_pAnchors) )
    {
        return;
    }
    if ( 0 == m_nOldParentWidth || 0 == m_nOldParentHeight )
    {
        m_nOldParentWidth = ANewWidth;
        m_nOldParentHeight = ANewHeight;
    }
    int nSpaceW = ANewWidth - m_nOldParentWidth;
    int nSpaceH = ANewHeight - m_nOldParentHeight;

    if ( nSpaceW == 0 && nSpaceH == 0 )
    {
        return;
    }

    if ( m_pAnchors->IsExsits(akLeft) && m_pAnchors->IsExsits(akRight) )
    {
        Position()->SetWidth( Position()->GetWidth() + nSpaceW );
    }
    else if ( m_pAnchors->IsExsits(akRight) )
    {
        Position()->SetLeft( Position()->GetLeft() + nSpaceW );
    }
    
    if ( m_pAnchors->IsExsits(akTop) && m_pAnchors->IsExsits(akBottom) )
    {
        Position()->SetHeight( Position()->GetHeight() + nSpaceH );
    }
    else if ( m_pAnchors->IsExsits(akBottom) )
    {
        Position()->SetTop( Position()->GetTop() + nSpaceH );
    }
    m_nOldParentWidth = ANewWidth;
    m_nOldParentHeight = ANewHeight;
    Position()->NotifyPositionChanged();
}

void CxdUIComponent::ClearSubComponents( void )
{
    TxdComponentList::reverse_iterator it = m_ltComponents.rbegin();
    while ( it != m_ltComponents.rend() )
    {
        CxdUIComponent *pObject = *it;
        m_ltComponents.pop_back();
        pObject->m_pParent = NULL;
        delete pObject;
        it = m_ltComponents.rbegin();
    }
}

bool CxdUIComponent::DeleteSubComponent( CxdUIComponent *ApObject )
{
    for ( TxdComponentListIT it = m_ltComponents.begin(); it != m_ltComponents.end(); it++ )
    {
        CxdUIComponent *pObject = *it;
        if ( (DWORD_PTR)pObject == (DWORD_PTR)ApObject )
        {
            if ( (DWORD_PTR)m_pActiveWidget == (DWORD_PTR)ApObject ) m_pActiveWidget = NULL;
            m_ltComponents.erase( it );
            return true;
        }
    }
    return false;
}

void CxdUIComponent::OnMouseMoveOnWidget( WPARAM AFlags, int x, int y )
{
    //CString strInfo;
    //strInfo.Format( TEXT("TAG: %I64d; x: %d; y: %d\r\n"), Tag, x, y );
    //OutputDebugString( strInfo );
    bool bReSetEnter = false;
    if ( Assigned(m_pActiveWidget) )
    {
        CRect R( m_pActiveWidget->Position()->GetCRect() );
        if ( m_pActiveWidget->GetVisible() && m_pActiveWidget->IsEnable() && PtInRect(&R, CPoint(x, y)) )
        {
            int nNewX( x - m_pActiveWidget->Position()->GetLeft() );
            int nNewY( y - m_pActiveWidget->Position()->GetTop() );
            m_pActiveWidget->OnMouseMoveOnWidget(AFlags, nNewX, nNewY);   
        }
        else
        {
            m_pActiveWidget->OnMouseLeaveWidget();
            m_pActiveWidget = NULL;
            bReSetEnter = true;
        }
    }

    if ( !Assigned(m_pActiveWidget) && !IsCurCapture(this) )
    {
        CPoint pt(x, y);
        for ( TxdComponentListIT it = m_ltComponents.begin(); it != m_ltComponents.end(); it++ )
        {
            CxdUIComponent *pObj = *it;
            if ( pObj->m_bDrawByParent && (pObj->GetVisible() || pObj->IsCurAnimateUnVisible()) )
            {
                CRect R( pObj->Position()->GetCRect() );
                if ( PtInRect(&R, pt) )
                {
                    m_pActiveWidget = pObj;
                    if ( m_pActiveWidget->IsEnable() )
                    {
                        m_pActiveWidget->OnMouseEnterWidget();
                        OnMouseMoveOnWidget( AFlags, x, y );
                        return;
                    }
                }
            }
        }

        //add by Terr at 2013-10-30
        if ( bReSetEnter )
        {
            CRect R( 0, 0, Position()->GetWidth(), Position()->GetHeight() );
            if ( PtInRect(&R, pt) )
            {
                OnMouseEnterWidget(); 
            }
        }
    }
}

void CxdUIComponent::OnMouseEnterWidget( void )
{
    DoHandleHintMessage( true, this );
    m_bMouseInWidget = true;
    OnUIStateChanged( uiActive );    
}

void CxdUIComponent::OnMouseLeaveWidget( void )
{    
    DoHandleHintMessage( false, this );
	try
	{
		for ( TxdComponentListIT it = m_ltComponents.begin(); it != m_ltComponents.end(); it++ )
		{
			CxdUIComponent *pObj = *it;
			if ( Assigned(pObj) && pObj->IsMouseActive() )
			{
				pObj->OnMouseLeaveWidget();
			}
		}
	}
	catch (...)
	{
		
	}
    m_bMouseInWidget = false;
    m_pActiveWidget = NULL;
    OnUIStateChanged( uiNormal );  
    if ( Assigned(m_pParent) && m_pParent->GetCurUISate() != uiNormal )
    {
        DoHandleHintMessage( true, m_pParent );
    }
}

void CxdUIComponent::OnLButtonDownWidget( WPARAM AFlags, int x, int y )
{
    OnUIStateChanged( uiDown );
}

void CxdUIComponent::OnLButtonUpWidget( WPARAM AFlags, int x, int y )
{
    OnUIStateChanged( uiActive );
}

void CxdUIComponent::SetVisible( bool bVisible )
{
    ClearBufferBitmap();
    DoSettingVisible( bVisible );
    if ( bVisible )
    {
        if ( m_bCurAnimateUnVisible )
        {
            m_bCurAnimateUnVisible = false;
            EndTimer( CtAnimateUnVisibleTimerID );
        }
    }
    else
    {
        if ( GetAnimateUnVisible() )
        {
            m_bCurAnimateUnVisible = true;
            m_fUnVisibleTranslate = 1.0f;            
            StartTimer( CtAnimateUnVisibleTimerID, m_nAnimateunVisibleElapse );
        }
    }
}

bool CxdUIComponent::GetVisible()
{
    if ( Assigned(m_pParent) )
    {
        if ( m_pParent->IsWndObject() )
        {
            return DoGetVisible();
        }
        return DoGetVisible() && m_pParent->GetVisible();
    }
    return DoGetVisible();
}

bool CxdUIComponent::OnTimerMessage( int ATimeID, LPVOID ApData )
{
    if ( CtAnimateUnVisibleTimerID == ATimeID )
    {
        EndTimer( CtAnimateUnVisibleTimerID );   
        ClearBufferBitmap();
        Invalidate();
    }
    else if ( CtClearBufferResTimerID == ATimeID )
    {
        EndTimer( CtClearBufferResTimerID );
        ClearBufferBitmap();
    }
    return false;
}

bool CxdUIComponent::IsCurAnimateUnVisible( void )
{
    return m_bCurAnimateUnVisible || (Assigned(m_pParent) && m_pParent->IsCurAnimateUnVisible());
}

REAL CxdUIComponent::GetCurAnimateTranslate()
{
    if ( m_bCurAnimateUnVisible )
    {
        return m_fUnVisibleTranslate;
    }
    else if ( Assigned(m_pParent) )
    {
        return m_pParent->GetCurAnimateTranslate();
    }
    else
    {
        return 0.0f;
    }
}

void CxdUIComponent::OnCreated( void )
{
    if ( Assigned(m_pParent) )
    {
        m_nOldParentWidth = m_pParent->Position()->GetWidth();
        m_nOldParentHeight = m_pParent->Position()->GetHeight();
    }
}

HWND CxdUIComponent::GetOwnerHandle( void )
{
    CxdUIComponent *pOwner = GetOwner();
    if ( !Assigned(pOwner) )
    {
        return 0;
    }
    CWnd* pWnd = dynamic_cast<CWnd*>( pOwner );
    return pWnd->GetSafeHwnd();
}


CSize CxdUIComponent::CalcStringSize( const TCHAR *ApString, const int &AMaxWidth /*= 0*/, const int &AMaxHeight /*= 0*/ )
{
    HWND hParent = GetOwnerHandle();
    HDC dc = ::GetDC( hParent );
    Graphics g( dc );
    CSize si = ::CalcStringSize( g, FontInfo()->GetFont(), ApString, AMaxWidth, AMaxHeight );
    ::ReleaseDC( hParent, dc );
    return si;
}


void CxdUIComponent::SetAutoSizeByCaption( const int &ALeftRightSpace /*= 0*/, const int &ATopDownSpace /*= 0*/, const int &AMaxWidth /*= 0*/, const int &AMaxHeight /*= 0*/ )
{
    if ( IsDrawCaption() )
    {
        CString strCaption = GetCaption();
        HWND hParent = GetOwnerHandle();
        if ( strCaption.IsEmpty() || 0 == hParent )
        {
            return;
        }
        CSize si = CalcStringSize( strCaption, AMaxWidth, AMaxHeight );

        Position()->SetWidth( ALeftRightSpace * 2 + si.cx + m_nCaptionLeft );
        Position()->SetHeight( ATopDownSpace * 2 + si.cy + m_nCaptionTop );
        Position()->NotifyPositionChanged();
    }
}

void CxdUIComponent::SetHintText( const TCHAR *ApHintText )
{
    if ( Assigned(ApHintText) && _tcslen(ApHintText) )
    {
        if ( !Assigned(m_pStrHintText) )
        {
            m_pStrHintText = new CString;
        }
        *m_pStrHintText = ApHintText;
    }
    else
    {
        SafeDeleteObject( m_pStrHintText );
    }
}

void CxdUIComponent::SetAutoSizeByImage( void )
{
    if ( Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() )
    {
        Position()->SetWidth( m_pImgInfo->GetBitmap()->GetWidth() );
        Position()->SetHeight( m_pImgInfo->GetBitmap()->GetHeight() / m_pImgInfo->GetBitmapItemCount() );
        Position()->NotifyPositionChanged();
        Position()->NotifyPositionChanged();
    }
}

bool CxdUIComponent::IsNeedDraw( const Gdiplus::Rect ASrcRect )
{
    return true;
}

void CxdUIComponent::InvalidateGpRect( const Rect &ARect )
{
    InvalidateRect( &CRect(ARect.X, ARect.Y, ARect.GetRight(), ARect.GetBottom()) );
}

bool CxdUIComponent::OnHintMessage( void )
{
    NotifyInterface( OnHintMessage, this );
    return Assigned( GetHintText() );
}

void CxdUIComponent::OnLButtonDbclickWidget( WPARAM AFlags, int x, int y )
{
    NotifyInterface( DblClick, this );
}

bool CxdUIComponent::IsEnable( void )
{
    if ( IsWndObject() )
    {
        return GetEnable();
    }
    return GetEnable() &&  ( Assigned(m_pParent) ? (m_pParent->IsWndObject() ? m_pParent->GetEnable() : m_pParent->IsEnable()) : true );
}

void CxdUIComponent::CalcScreenPos( Rect &R )
{
    CxdScreen scr;
    if ( R.X + R.Width > scr.GetScreenWidth() )
    {
        R.X -= R.Width;
    }
    if ( R.Y + R.Height > scr.GetScreenHeight() )
    {
        R.Y -= R.Height;
    }
}

void CxdUIComponent::SetImageInfo( CxdImageInfo *ApNewImg, bool AbResOnly )
{
    UpdatePointer( m_pImgInfo, ApNewImg );
    if ( !AbResOnly )
    {
        if ( Assigned(m_pImgInfo) )
        {
            m_pImgInfo->ForceDelBmpOnFree();
        }
    }
}

void CxdUIComponent::SetFontInfo( CxdFontInfo *ApNewFont, bool AbResOnly /*= true*/ )
{
    UpdatePointer( m_pFontInfo, ApNewFont );
    if ( !AbResOnly )
    {
        if ( Assigned(m_pFontInfo) )
        {
            m_pFontInfo->ForceDelFontOnFree();
        }
    }
}

void CxdUIComponent::DoHandleHintMessage( bool bActive, CxdUIComponent *ApObject )
{
    if ( IsExsitsHintInterface() )
    {
        IHintMessage *pHitInterface = GetHintInterface();
        if ( Assigned(pHitInterface))
        {
            bActive ? pHitInterface->ActivateHint( ApObject ) : pHitInterface->UnActiveHint( ApObject );
        }
    }
    else
    {
        if ( ApObject->OnHintMessage() && Assigned(ApObject->GetHintText()) )
        {
            IHintMessage *pHitInterface = GetHintInterface();
            if ( Assigned(pHitInterface))
            {
                bActive ? pHitInterface->ActivateHint( ApObject ) : pHitInterface->UnActiveHint( ApObject );
            }
        }
    }
}

Gdiplus::Bitmap* CxdUIComponent::GetBufferBitmap( void )
{
    if ( Assigned(m_pBufBitmap) )
    {
        return m_pBufBitmap;
    }
    int nW = Position()->GetWidth();
    int nH = Position()->GetHeight();
    m_pBufBitmap = new Bitmap( nW, nH, PixelFormat32bppARGB );
    Graphics bmpG( m_pBufBitmap );
    //bmpG.SetSmoothingMode( SmoothingModeHighQuality );
    bmpG.SetTextRenderingHint( TextRenderingHintAntiAliasGridFit );
    //bmpG.FillRectangle( &SolidBrush(0), 0, 0, nW, nH );
    OnPaintComponent( bmpG, Rect(0, 0, nW, nH) );
    return m_pBufBitmap;
}

void CxdUIComponent::DeferClearBufferBitmap()
{
    if ( m_BufferBitmapTimeout > 0 )
    {
        if ( Assigned(m_pBufBitmap) )
        {
            StartTimer( CtClearBufferResTimerID, m_BufferBitmapTimeout );
        }
    }
    else
    {
        SafeDeleteObject( m_pBufBitmap );
    }
}

void CxdUIComponent::ClearBufferBitmap( void )
{
    SafeDeleteObject( m_pBufBitmap );
    if ( Assigned(m_pParent) && m_pParent->GetBufferBitmapTimeout() > 0 )
    {
        m_pParent->ClearBufferBitmap();
    }
}

bool CxdUIComponent::IsHasSubUINotDraw( void )
{
    if ( m_bHasSubUINotDraw )
    {
        return true;
    }
    for ( TxdComponentListIT it = m_ltComponents.begin(); it != m_ltComponents.end(); it++ )
    {
        CxdUIComponent *pObj = *it;
        if ( pObj->IsHasSubUINotDraw() )
        {
            return true;
        }
    }
    return false;
}

void CxdUIComponent::PostCustomMessage( CxdCustomMessageInfo *ApInfo )
{
    ::PostMessage( GetOwnerHandle(), CtCustomMessageID, (WPARAM)this, (LPARAM)ApInfo );
}

void CxdUIComponent::SendCustomMessage( CxdCustomMessageInfo *ApInfo )
{
    ::SendMessage( GetOwnerHandle(), CtCustomMessageID, (WPARAM)this, (LPARAM)ApInfo );
}

CxdAnchors* CxdUIComponent::Anchors( void )
{
    if ( !Assigned(m_pAnchors) )
    {
        m_pAnchors = new CxdAnchors;
        if ( Assigned(m_pParent) )
        {
            IxdPositionInterface *pIt = NewInterfaceByClass(CxdUIComponent, CxdPosition*, OnParentPositionChanged);
            if ( !m_pParent->Position()->AddEventPositionChanged(pIt) )
            {
                delete pIt;
            }
        }
    }
    return m_pAnchors;
}

void CxdUIComponent::CheckAnchors( CxdAnchors* ApAnchors )
{
    if ( Assigned(m_pAnchors) && Assigned(m_pParent) )
    {
        IxdPositionInterface *pIt = NewInterfaceByClass(CxdUIComponent, CxdPosition*, OnParentPositionChanged);
        if ( !m_pParent->Position()->AddEventPositionChanged(pIt) )
        {
            delete pIt;
        }
    }
}

void CxdUIComponent::OnParentPositionChanged( CxdPosition *ApPosition )
{
    DoHandleParentSizeChanged( ApPosition->GetWidth(), ApPosition->GetHeight() );
    Position()->NotifyPositionChanged();
}

VOID CALLBACK _TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    TxdTimerInfo *p = (TxdTimerInfo *)idEvent;
    try
    {
        if ( !p->pOwner->OnTimerMessage(p->TimerID, p->Data) )
        {
            ::KillTimer( hwnd, idEvent );
            p->pOwner->FindTimerInfo( p->TimerID, true );
            delete p;
        }
    }
    catch (...)
    {
        ::KillTimer( hwnd, idEvent );
        p->pOwner->FindTimerInfo( p->TimerID, true );
        delete p;
    }
}

void CxdUIComponent::StartTimer( int ATimeID, int ATimeElapse, LPVOID AData )
{
    HWND hwnd = GetOwnerHandle();
    if ( hwnd != 0 )
    {
        TxdTimerInfo *pInfo = FindTimerInfo( ATimeID, false );
        if ( IsNull(pInfo) )
        {
            pInfo = new TxdTimerInfo;
            pInfo->Data = AData;
            pInfo->pOwner = this;
            pInfo->TimerID = ATimeID;
            m_ltTimerInfo.push_back( pInfo );
        }
        SetTimer( hwnd, (UINT_PTR)pInfo, ATimeElapse, _TimerProc );
    }
}

void CxdUIComponent::EndTimer( int ATimeID )
{
    HWND hwnd = GetOwnerHandle();
    if ( hwnd != 0 )
    {
        TxdTimerInfo *pInfo = FindTimerInfo( ATimeID, true );
        if ( Assigned(pInfo) )
        {
            KillTimer( hwnd, (UINT_PTR)pInfo );
            delete pInfo;
        }
    }
}

TxdTimerInfo* CxdUIComponent::FindTimerInfo( int ATimeID, bool ADelete )
{
    for ( TTimerInfoListIT it = m_ltTimerInfo.begin(); it != m_ltTimerInfo.end(); it++ )
    {
        TxdTimerInfo *pInfo = *it;
        if ( pInfo->TimerID == ATimeID )
        {
            if ( ADelete ) m_ltTimerInfo.erase( it );
            return pInfo;
        }
    }
    return NULL;
}

void CxdUIComponent::ClearTimerInfo( void )
{
    HWND hWnd = GetOwnerHandle();
    for ( TTimerInfoListIT it = m_ltTimerInfo.begin(); it != m_ltTimerInfo.end(); it++ )
    {
        TxdTimerInfo *pInfo = *it;
        if ( hWnd > 0 ) KillTimer( hWnd, (UINT_PTR)pInfo );
        delete pInfo;
    }
    m_ltTimerInfo.clear();
}

void CxdUIComponent::OnMidMouseWheel( UINT nFlags, short zDelta, int x, int y )
{
    for ( TxdComponentListIT it = m_ltComponents.begin(); it != m_ltComponents.end(); it++ )
    {
        CxdUIComponent *pObject = *it;
        if ( pObject->GetVisible() && pObject->IsEnable() && pObject->Position()->IsInReect(x, y) )
        {
            pObject->OnMidMouseWheel( nFlags, zDelta, x - pObject->Position()->GetLeft(), y - pObject->Position()->GetTop() );
        }
    }
}

void CxdUIComponent::OnRButtonDownWidget( WPARAM AFlags, int x, int y )
{

}

void CxdUIComponent::OnRButtonUpWidget( WPARAM AFlags, int x, int y )
{

}

void CxdUIComponent::OnRButtonClick( const int &x /*= 0*/, const int &y /*= 0*/ )
{
    NotifyInterface( RButtonClick, this );
}

