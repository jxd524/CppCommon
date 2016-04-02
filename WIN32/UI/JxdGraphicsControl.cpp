#include "stdafx.h"
#include "JxdGraphicsControl.h"

#include <algorithm>

#include "JxdGdiPlusBasic.h"

//////////////////////////////////////////////////////////////////////////
/// CxdControl

CxdGraphicsControl::CxdGraphicsControl(): CxdUIComponent()
{
    m_bCreated = false;
    m_bDrawByParent = true;
    SetBufferBitmapTimeout( 0 );
    SetCaptureOnMouseDown( true );
    SetAutoMoveForm( false );
    m_bIsWndObject = false;
    m_bVisible = true;
    m_bEnable = true;
    m_pOwner = NULL;
    m_pstrCaption = NULL;
    m_pImgIconInfo = NULL;
    Position()->AddEventPositionChanged( NewInterfaceByClass(CxdGraphicsControl, CxdPosition*, DoPositionChanged) );
}

CxdGraphicsControl::~CxdGraphicsControl()
{
    ClearIconInfos();
    SafeDeleteObject( m_pstrCaption );
    SafeDeleteObject( m_pImgIconInfo );
}

bool CxdGraphicsControl::xdCreate( const int &Ax, const int &Ay, const int &AWidth, const int &AHeight, CxdUIComponent *ApParent /*= NULL*/, const TCHAR *ApWindowName /*= NULL*/ )
{
    if ( m_bCreated || !Assigned(ApParent) )
    {
        return false;
    }
    m_bCreated = true;
    m_pParent = ApParent;   
    m_pOwner = m_pParent->IsWndObject() ? m_pParent : m_pParent->GetOwner();
    m_pParent->AddSubComponent( this );
    Position()->SetLeft( Ax );
    Position()->SetTop( Ay );
    Position()->SetWidth( AWidth );
    Position()->SetHeight( AHeight );
    if ( Assigned(ApWindowName) && _tcslen(ApWindowName) )
    {
        *xdCreateObject(m_pstrCaption) = ApWindowName;
    }    
    CheckAnchors( NULL );
    if ( GetVisibleOnParentActive() )
    {
        m_bVisible = false;
    }
    OnCreated();
    return true;
}

void CxdGraphicsControl::DoPositionChanged( CxdPosition* ApSender )
{
    InvalidateRect( NULL );
}

void CxdGraphicsControl::InvalidateRect( CRect *ApRect /*= NULL*/ )
{
    if ( !Assigned(m_pParent) ) return;

    CRect R( 0, 0, Position()->GetWidth(), Position()->GetHeight() );
    if ( Assigned(ApRect) )
    {
        R = *ApRect;
    }
    int nW = R.Width();
    int nH = R.Height();

    if ( nW <= 0 || nH <= 0 ) return;
    if ( R.top < 0 ) R.top = 0;
    if ( R.left < 0 ) R.left = 0;
    if ( R.bottom > R.top + nH ) R.bottom = R.top + nH;
    if ( R.right > R.left + nW ) R.right = R.left + nW;

    R.left = R.left >= 0 ? R.left + Position()->GetLeft() : Position()->GetLeft();
    R.top = R.top >= 0 ? R.top + Position()->GetTop() : Position()->GetTop();
    R.right = R.left + nW;
    R.bottom = R.top + nH;

    if ( R.bottom > Position()->GetBottom() )
    {
        R.bottom = Position()->GetBottom();
    }
    if ( R.right > Position()->GetRight() )
    {
        R.right = Position()->GetRight();
    }
    m_pParent->InvalidateRect( &R );
}

bool CxdGraphicsControl::OnUIStateChanged( const TxdGpUIState &ANewSate )
{
    bool bNeedReDraw = CxdUIComponent::OnUIStateChanged(ANewSate);
    if ( bNeedReDraw && ((Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() && m_pImgInfo->GetBitmapItemCount() > 1) || IsDrawCaption()) )
    {
        InvalidateRect( NULL );
    }
    return bNeedReDraw;
}

CRect CxdGraphicsControl::GetRealRectByParent( void )
{
    return Position()->GetCRect();
}

CRect CxdGraphicsControl::GetRealRectOnUI( void )
{   
    CRect uiR(0, 0, 0, 0);
    if ( !Assigned(m_pParent) || !Assigned(m_pOwner) )
    {
        return uiR;
    }
    CxdUIComponent *pFind = m_pParent;
    int nRight(0), nBottom(0);
    if ( m_pParent == m_pOwner )
    {
        nRight = m_pOwner->Position()->GetWidth();
        nBottom = m_pOwner->Position()->GetHeight();
    }
    else
    {
        CRect uiParent = m_pParent->GetRealRectOnUI();
        nRight = uiParent.right;
        nBottom = uiParent.bottom;
        while ( pFind != m_pOwner )
        {
            uiR.left += pFind->Position()->GetLeft();// > 0 ? pFind->Position()->GetLeft() : 0 ;
            uiR.top += pFind->Position()->GetTop();// > 0 ? pFind->Position()->GetTop() : 0;            
            pFind = pFind->GetComponentParent();
        }
    }

    if ( Position()->GetLeft() >= 0 )
    {
        uiR.left += Position()->GetLeft();
        uiR.right = uiR.left + Position()->GetWidth();
    }
    else
    {
        uiR.right = uiR.left + Position()->GetWidth() + Position()->GetLeft();
    }
    if ( Position()->GetTop() >= 0 )
    {
        uiR.top += Position()->GetTop();
        uiR.bottom = uiR.top + Position()->GetHeight();
    }
    else
    {
        uiR.bottom = uiR.top + Position()->GetHeight() + Position()->GetTop();
    }

    if ( uiR.left < 0 ) uiR.left = 0;
    if ( uiR.top < 0 ) uiR.top = 0;
    if ( uiR.right > nRight ) uiR.right = nRight;
    if ( uiR.bottom > nBottom ) uiR.bottom = nBottom;

    return uiR;
}

CString CxdGraphicsControl::GetCaption( void )
{
    if ( Assigned(m_pstrCaption) )
    {
        return *m_pstrCaption;
    }
    return TEXT("");
}

void CxdGraphicsControl::SetCaption( const TCHAR* ApCaption )
{
    if ( !Assigned(ApCaption) || _tcslen(ApCaption) == 0 )
    {
        SafeDeleteObject( m_pstrCaption );
    }
    else
    {
        xdCreateObject( m_pstrCaption );
        *m_pstrCaption = ApCaption;
    }
    ClearBufferBitmap();
}

bool CxdGraphicsControl::IsNeedDraw( const Gdiplus::Rect ASrcRect )
{
    return m_pOwner->IsNeedDraw( ASrcRect );
}

void CxdGraphicsControl::ReSetParent( CxdUIComponent *ApParent )
{
    if ( Assigned(m_pParent) )
    {
        m_pParent->Position()->ClearEventByOwner( (INT64)this );
        if ( Assigned(m_pParent->IsExsitsAnchor()) )
        {
            m_pParent->Anchors()->ClearEventByOwner( (INT64)this );
        }
        m_pParent->DeleteSubComponent( this );
        m_pParent->ClearBufferBitmap();
        m_pParent->Invalidate();
    }
    m_pParent = ApParent;
    if ( m_pParent != NULL )
    {
        m_pParent->AddSubComponent( this );
        CheckAnchors( NULL );
        if ( GetVisibleOnParentActive() )
        {
            m_bVisible = false;
        }
    }
}

void CxdGraphicsControl::ClearIconInfos( void )
{
    for ( TImgIconInfoListIT it = m_ltImgIconInfoList.begin(); it != m_ltImgIconInfoList.end(); it++ )
    {
        TImgIconInfo *pInfo = *it;
        delete pInfo;
    }
    m_ltImgIconInfoList.clear();
}

int CxdGraphicsControl::AddIconInfo( const TImgIconInfo &AInfo )
{
    TImgIconInfo *pInfo = new TImgIconInfo;
    *pInfo = AInfo;
    m_ltImgIconInfoList.push_back( pInfo );
    return m_ltImgIconInfoList.size() - 1;
}

bool CxdGraphicsControl::DeleteIconInfo( const UINT &AIconIndex )
{
    if ( AIconIndex >= 0 && AIconIndex < (UINT)m_ltImgIconInfoList.size() )
    {
        TImgIconInfoListIT it = m_ltImgIconInfoList.begin() + AIconIndex;
        TImgIconInfo *pInfo = *it;
        delete pInfo;
        m_ltImgIconInfoList.erase( it );
        return true;
    }
    return false;
}

void CxdGraphicsControl::DoDrawSubjoinBK( Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt )
{
    CxdUIComponent::DoDrawSubjoinBK( ADestG, ADestRect, ApImgAtt );
    if ( !Assigned(m_pImgIconInfo) || !m_pImgIconInfo->IsExsitsBitmap() || m_ltImgIconInfoList.empty() ) return;

    int nH = m_pImgIconInfo->GetBitmapItemHeight();
    Rect ImgR(0, 0, m_pImgIconInfo->GetBitmapItemWidth(), nH );

    for ( TImgIconInfoListIT it = m_ltImgIconInfoList.begin(); it != m_ltImgIconInfoList.end(); it++ )
    {
        TImgIconInfo *pInfo = *it;
        if ( pInfo->FIconIndex >= 0 && pInfo->FIconIndex < m_pImgIconInfo->GetBitmapItemCount() )
        {
            Rect SrcR( ImgR );
            SrcR.Y = pInfo->FIconIndex * nH;

            Rect DestR( ADestRect );
            DestR.X += pInfo->FOffsetX;
            DestR.Y += pInfo->FOffsetY;

            switch ( GetCurUISate() )
            {
            case uiActive:
                DestR.X += pInfo->FOffsetXOnActive;
                DestR.Y += pInfo->FOffsetYOnActive;
            	break;
            case uiDown:
                DestR.X += pInfo->FOffsetXOnActive + pInfo->FOffsetXOnDown;
                DestR.Y += pInfo->FOffsetYOnActive + pInfo->FOffsetYOnDown;
                break;
            }

            DrawCommonImage( ADestG, DestR, m_pImgIconInfo->GetBitmap(), SrcR,
                m_pImgIconInfo->GetBitmapDrawStyle(), m_pImgIconInfo->GetStretchX(), m_pImgIconInfo->GetStretchY(), ApImgAtt );
        }
    }
}

void CxdGraphicsControl::SetImageIconInfo( CxdImageInfo *ApNewImg, bool AbResOnly /*= true*/ )
{
    UpdatePointer( m_pImgIconInfo, ApNewImg );
    if ( !AbResOnly )
    {
        if ( Assigned(m_pImgIconInfo) )
        {
            m_pImgIconInfo->ForceDelBmpOnFree();
        }
    }
}

TImgIconInfo* CxdGraphicsControl::GetIconInfo( const UINT &AIconIndex )
{
    if ( AIconIndex >= 0 && AIconIndex < (UINT)m_ltImgIconInfoList.size() )
    {
        TImgIconInfoListIT it = m_ltImgIconInfoList.begin() + AIconIndex;
        return *it;
    }
    return NULL;
}
