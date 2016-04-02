#include "stdafx.h"
#include "JxdTabControl.h"

#include <algorithm>

CxdTabControl::CxdTabControl(): CxdGraphicsControl(), m_nCtDeleteItem( GetUnionID() )
{    
    m_LeftSpace = 2;
    m_RightSpace = 2;
    m_TopSpace = 2;
    m_DownSpace = 2;
    m_SpaceSize = 1;
    m_ItemMaxWidth = 120;
    m_pImgTabItem = NULL;
    m_pImgTabItemClose = NULL;
    m_pBtnAddTab = new CxdGraphicsControl;
}

CxdTabControl::~CxdTabControl()
{
    SafeDeleteObject( m_pImgTabItemClose );
    SafeDeleteObject( m_pImgTabItem );
    m_ltItems.clear();
}

void CxdTabControl::OnCreated()
{
    m_pBtnAddTab->xdCreate( m_pBtnAddTab->Position()->GetLeft(), m_pBtnAddTab->Position()->GetTop(), 
        m_pBtnAddTab->Position()->GetWidth(), m_pBtnAddTab->Position()->GetHeight(), this );
}

void CxdTabControl::SetAddTabButtonPos( const int &ATop, const int& AWidth, const int &AHeight )
{
    m_pBtnAddTab->Position()->SetTop( ATop );
    m_pBtnAddTab->Position()->SetWidth( AWidth );
    m_pBtnAddTab->Position()->SetHeight( AHeight );
    m_pBtnAddTab->Position()->NotifyPositionChanged();
}

int CxdTabControl::AddTab( const TCHAR *ApCaption, const UINT64 &ATag )
{
    CxdButton *pItem = DoNewTabItem();
    if ( !Assigned(pItem) ) return -1;

    pItem->SetImageInfo( m_pImgTabItem );
    pItem->SetItClick( NewInterfaceByClass(CxdTabControl, CxdUIComponent*, DoItemClick) );
    pItem->AddToGroup( (int)this );

    int x( GetLeftSpace() );
    int y( GetTopSpace() );
    int w( GetItemMaxWidth() );
    int h( Position()->GetHeight() - GetTopSpace() - GetDownSpace() );

    if ( !m_ltItems.empty() )
    {
        CxdButton *pLast = m_ltItems[ m_ltItems.size() - 1 ];
        x = pLast->Position()->GetRight() + GetSpaceSize();
        w = pLast->Position()->GetWidth();
    }

    pItem->SetImageInfo( m_pImgTabItem );
    pItem->xdCreate( x, y, w, h, this, ApCaption );
    pItem->Tag = ATag;

    if ( Assigned(m_pImgTabItemClose) && m_pImgTabItemClose->IsExsitsBitmap() )
    {
        CxdGraphicsControl *pClose = DoNewTabItemClose();
        if ( Assigned(pClose) )
        {
            int nCloseW = m_pImgTabItemClose->GetBitmap()->GetWidth();
            int nCloseH = m_pImgTabItemClose->GetBitmap()->GetHeight() / m_pImgTabItemClose->GetBitmapItemCount();
            int nCloseX = pItem->Position()->GetWidth() - nCloseW - 2;
            int nCloseY = (pItem->Position()->GetHeight() - nCloseH) / 2;

            pClose->Anchors()->SetValue( akRight );
            pClose->SetItClick( NewInterfaceByClass(CxdTabControl, CxdUIComponent*, DoItemCloseClick) );
            pClose->SetAnimateUnVisible( true );
            pClose->SetVisibleOnParentActive( true );            
            pClose->SetImageInfo( m_pImgTabItemClose );
            pClose->xdCreate( nCloseX, nCloseY, nCloseW, nCloseH, pItem );
            //pClose->SetVisible( false );
        }
    }

    pItem->Invalidate();

    if ( m_ltItems.empty() )
    {
        pItem->SetPushDown( true );
    }
    m_ltItems.push_back( pItem );

    int nMaxWidth = Position()->GetWidth() - GetLeftSpace() - GetRightSpace();
    int nCurWidth = w * m_ltItems.size() + GetSpaceSize() * (m_ltItems.size() - 1);
    if ( nCurWidth > nMaxWidth )
    {
        x = GetLeftSpace();
        w = ( nMaxWidth - GetSpaceSize() * (m_ltItems.size() - 1) ) / m_ltItems.size();
        for ( TxdButtonListIT it = m_ltItems.begin(); it != m_ltItems.end(); it++ )
        {
            CxdButton *pObj = *it;
            pObj->Position()->SetLeft( x );
            pObj->Position()->SetWidth( w );
            pObj->Position()->NotifyPositionChanged();
            x += w + GetSpaceSize();
        }
        Invalidate();
    }
    return m_ltItems.size() - 1;
}

void CxdTabControl::DoItemClick( CxdUIComponent* ApBtn )
{
    CxdButton *pBtn = dynamic_cast<CxdButton*>(ApBtn);
    if ( DoSelecteTab(pBtn) )
    {
        pBtn->SetPushDown( true );
    }    
}

void CxdTabControl::DoItemCloseClick( CxdUIComponent* ApBtn )
{
    CxdGraphicsControl *pBtnClose = dynamic_cast<CxdGraphicsControl*>( ApBtn );
    CxdButton *pBtn = dynamic_cast<CxdButton*>( pBtnClose->GetComponentParent() );
    _DeleteTab( pBtn );
}

bool CxdTabControl::DoSelecteTab( CxdButton *ApBtn )
{
    return true;
}

bool CxdTabControl::DoCloseTab( CxdButton *ApBtn )
{
    return true;
}

bool CxdTabControl::OnTimerMessage(int ATimeID, LPVOID ApData)
{
    if ( ATimeID == m_nCtDeleteItem )
    {
        for ( TxdButtonListIT it = m_ltWaitDeleteItem.begin(); it != m_ltWaitDeleteItem.end(); it++ )
        {
            CxdButton *pBtn = *it;
            delete pBtn;
        }
        m_ltWaitDeleteItem.clear();
    }
    return false;
}

void CxdTabControl::DelTab( const int &AIndex )
{
    if ( AIndex >= 0 && AIndex < (int)m_ltItems.size() )
    {
        _DeleteTab( m_ltItems[AIndex] );
    }
}

void CxdTabControl::_DeleteTab( CxdButton *ApBtn )
{
    if ( DoCloseTab(ApBtn) )
    {
        TxdButtonListIT it = find( m_ltItems.begin(), m_ltItems.end(), ApBtn );
        m_ltItems.erase( it );
        ApBtn->SetVisible( false );
        m_ltWaitDeleteItem.push_back( ApBtn );

        if ( !m_ltItems.empty() )
        {
            int x = GetLeftSpace();
            int nMaxWidth = Position()->GetWidth() - GetLeftSpace() - GetRightSpace();
            int w = ( nMaxWidth - GetSpaceSize() * (m_ltItems.size() - 1) ) / m_ltItems.size();
            if ( w > GetItemMaxWidth() )
            {
                w = GetItemMaxWidth();
            }
            for ( TxdButtonListIT it = m_ltItems.begin(); it != m_ltItems.end(); it++ )
            {
                CxdButton *pObj = *it;
                pObj->Position()->SetLeft( x );
                pObj->Position()->SetWidth( w );
                pObj->Position()->NotifyPositionChanged();
                x += w + GetSpaceSize();
            }
        }

        Invalidate();

        StartTimer( m_nCtDeleteItem, 100 );
    }
}

void CxdTabControl::SelTab( const int &AIndex )
{
    if ( AIndex >= 0 && AIndex < (int)m_ltItems.size() )
    {
        CxdButton *pBtn = m_ltItems[AIndex];
        if ( !pBtn->GetPushDown() )
        {
            pBtn->Click();
        }
    }
}
