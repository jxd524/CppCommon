#include "stdafx.h"
#include "JxdListCtrl.h"
#include "JxdDebugInfo.h"

#include <algorithm>

CxdListCtrl::CxdListCtrl(): CxdGraphicsControl(), CtDeleteCtrlID( GetUnionID() ) 
{
    m_BorderLineColor = 0;
    m_ItemUpDownSpace = 0;
    m_nAllCtrlsHeight = 0;
    m_SubItemLeftSpace = 20;
    m_PerPosPiexl = 20;
    m_ClickItemAutoExpand = true;
    m_pVerScrollBar = NULL;
	m_pCurActiveCtrl = NULL;
	m_pCurSubActiveCtrl = NULL;
    Position()->AddEventPositionChanged( NewInterfaceByClass(CxdListCtrl, CxdPosition*, DoPositionChanged) );
    InitInterface( CreateVerBar );
    InitInterface( ListItemClick );//鼠标点击单项的事件
    InitInterface( MoveToTop );
    InitInterface( MoveToBottom );
    InitInterface( SubItemVisible );

    m_pItItemClick = NewInterfaceByClass( CxdListCtrl, CxdUIComponent*, DoItemClick );
    m_pItItemStateChanged = NewInterfaceByClass( CxdListCtrl, CxdUIComponent*, DoItemStateChanged );
    m_pItSubItemClick = NewInterfaceByClass( CxdListCtrl, CxdUIComponent*, DoSubItemClick );
    m_pItSubItemStateChanged = NewInterfaceByClass( CxdListCtrl, CxdUIComponent*, DoSubItemStateChanged );
}

CxdListCtrl::~CxdListCtrl()
{
    for ( TSubItemListIT it = m_ltListSubItems.begin(); it != m_ltListSubItems.end(); it++ )
    {
        TSubItemInfo *pSubInfo = *it;
        delete pSubInfo;
    }
    m_ltListSubItems.clear();

    DeleteInterface( CreateVerBar );
    DeleteInterface( ListItemClick );//鼠标点击单项的事件
    DeleteInterface( MoveToTop );
    DeleteInterface( MoveToBottom );
    DeleteInterface( SubItemVisible );
    SafeDeleteObject( m_pItItemClick );
    SafeDeleteObject( m_pItItemStateChanged );
    SafeDeleteObject( m_pItSubItemClick );
    SafeDeleteObject( m_pItSubItemStateChanged );
}

bool CxdListCtrl::AddListItem( CxdGraphicsControl *ApItemCtrl, int ADefaultHeight )
{
    if ( m_pParent == NULL ) return false;
    if ( m_ltListItems.end() != find(m_ltListItems.begin(), m_ltListItems.end(), ApItemCtrl) ) return false;

    int x(0);
    int y(0);
    int w( Position()->GetWidth() );
    int h( ADefaultHeight );
    if ( h < 0 )
    {
        int nH = ApItemCtrl->ImageInfo()->GetBitmapItemHeight();
        if ( nH > 0 ) h = nH;
    }

    if ( !m_ltListItems.empty() )
    {
        m_nAllCtrlsHeight += m_ItemUpDownSpace;
        CxdGraphicsControl *pLast = m_ltListItems.back();
        y = pLast->Position()->GetBottom() + m_ItemUpDownSpace;
        w = pLast->Position()->GetWidth();
        TSubItemInfo *pSubInfo = FindSubItemInfo( pLast, false );
        if ( Assigned(pSubInfo) && pSubInfo->FIsVisibleSubCtrls && !pSubInfo->FSubCtrlsList.empty() )
        {
            pLast = pSubInfo->FSubCtrlsList.back();
            y = pLast->Position()->GetBottom() + m_ItemUpDownSpace;
        }
    }

    if ( ApItemCtrl->IsCreated() )
    {
        if ( this != ApItemCtrl->GetComponentParent() )
        {
            ApItemCtrl->ReSetParent( this );
        }
        ApItemCtrl->Position()->SetLeft( x );
        ApItemCtrl->Position()->SetTop( y );
        ApItemCtrl->Position()->SetWidth( w );
        ApItemCtrl->Position()->SetHeight( h );
        ApItemCtrl->Position()->NotifyPositionChanged();
    }
    else
    {
        ApItemCtrl->xdCreate(x, y, w, h, this);
    }
    ApItemCtrl->SetItClick( m_pItItemClick, true );
    ApItemCtrl->SetItUIStateChanged( m_pItItemStateChanged, true );
    ApItemCtrl->SetVisible( true );
    ApItemCtrl->Anchors()->SetValue( akLeft | akTop | akRight );
    ApItemCtrl->Anchors()->NotifyAnchorChanged();
    m_ltListItems.push_back( ApItemCtrl );
    m_nAllCtrlsHeight += ApItemCtrl->Position()->GetHeight();
    CheckVerScrollBar();
    ApItemCtrl->Invalidate();
    return true;
}

bool CxdListCtrl::AddSubListItem( CxdGraphicsControl *ApItemParent, CxdGraphicsControl *ApItemCtrl, int ADefaultHeight /*= -1*/ )
{
    TxdGraphicsCtrlListIT itParent = find( m_ltListItems.begin(), m_ltListItems.end(), ApItemParent );
    if ( itParent == m_ltListItems.end() ) return false;

    TSubItemInfo *pInfo = FindSubItemInfo( ApItemParent, false );
    if ( !Assigned(pInfo) )
    {
        pInfo = new TSubItemInfo;
        pInfo->FIsVisibleSubCtrls = false;
        pInfo->FpParentCtrl = ApItemParent;
        m_ltListSubItems.push_back( pInfo );
    }
    
    int x( m_SubItemLeftSpace );
    int y( ApItemParent->Position()->GetBottom() + m_ItemUpDownSpace );
    int w( Position()->GetWidth() - m_SubItemLeftSpace - 
        (Assigned(m_pVerScrollBar) && m_pVerScrollBar->GetVisible() ? m_pVerScrollBar->Position()->GetWidth() : 0) );
    int h( ADefaultHeight );
    if ( h < 0 )
    {
        int nH = ApItemCtrl->ImageInfo()->GetBitmapItemHeight();
        if ( nH > 0 )
        {
            h = nH;
        }
    }
    if ( !pInfo->FSubCtrlsList.empty() )
    {
        if ( pInfo->FIsVisibleSubCtrls )
        {
            m_nAllCtrlsHeight += m_ItemUpDownSpace;
        }
        CxdGraphicsControl *pLast = pInfo->FSubCtrlsList.back();
        y = pLast->Position()->GetBottom() + m_ItemUpDownSpace;
        x = pLast->Position()->GetLeft();
        w = pLast->Position()->GetWidth();
    }

    if ( Assigned(ApItemCtrl->GetComponentParent()) )
    {
        if ( this != ApItemCtrl->GetComponentParent() )
        {
            ApItemCtrl->ReSetParent( this );
        }
        ApItemCtrl->Position()->SetLeft( x );
        ApItemCtrl->Position()->SetTop( y );
        ApItemCtrl->Position()->SetWidth( w );
        ApItemCtrl->Position()->SetHeight( h );
        ApItemCtrl->Position()->NotifyPositionChanged();
    }
    else
    {
        ApItemCtrl->xdCreate(x, y, w, h, this);
    }
    ApItemCtrl->SetItClick( m_pItItemClick, true );
    ApItemCtrl->SetItUIStateChanged( m_pItSubItemStateChanged, true );
    ApItemCtrl->Anchors()->SetValue( akLeft | akTop | akRight );
    ApItemCtrl->SetVisible( pInfo->FIsVisibleSubCtrls );
    pInfo->FSubCtrlsList.push_back( ApItemCtrl );

    if ( pInfo->FIsVisibleSubCtrls )
    {
        m_nAllCtrlsHeight += ApItemCtrl->Position()->GetHeight();
        ApItemCtrl->Invalidate();
        y += h + m_ItemUpDownSpace;
        while ( ++itParent != m_ltListItems.end() )
        {
            CxdGraphicsControl *pItem = *itParent;
            y = MoveItem( pItem, y );
        }
        CheckVerScrollBar();
    }
    return true;
}

CxdBar* CxdListCtrl::DoCreateVerScrollBar( void )
{
    CxdBar *pBar = NULL;
    NotifyInterface( CreateVerBar, &pBar );
    return pBar;
}

void CxdListCtrl::CheckVerScrollBar( void )
{
    int nUnVisibleH = m_nAllCtrlsHeight - Position()->GetHeight();
    if ( nUnVisibleH <= 0 )
    {
        if ( Assigned(m_pVerScrollBar) && m_pVerScrollBar->GetVisible() )
        {
            m_pVerScrollBar->SetVisible( false );
            SetAllCtrlWidth( m_pVerScrollBar->Position()->GetWidth() );
            int nY = 0;
            for ( TxdGraphicsCtrlListIT it = m_ltListItems.begin(); it != m_ltListItems.end(); it++ )
            {
                CxdGraphicsControl *pObj = *it;
                nY = MoveItem( pObj, nY );
            }
        }
    }
    else
    {
        if ( !Assigned(m_pVerScrollBar) )
        {
            m_pVerScrollBar = DoCreateVerScrollBar();
            if ( !Assigned(m_pVerScrollBar) ) return;

            m_pVerScrollBar->SetBarStyle( bsVer );
            m_pVerScrollBar->SetBarPos( 0 );
            m_pVerScrollBar->SetMin( 0 );    
            m_pVerScrollBar->Anchors()->SetValue( akTop | akRight | akBottom );
            m_pVerScrollBar->SetItBarPosChanged( NewInterfaceEx3ByClass(CxdListCtrl, CxdBar*, bool, int, DoVerBarScroll) );
            m_pVerScrollBar->CalcBarInfo();
            int nW = m_pVerScrollBar->ImageInfo()->GetBitmap()->GetWidth();
            m_pVerScrollBar->xdCreate( Position()->GetWidth() - nW, 0, nW, Position()->GetHeight(), this );            
            SetAllCtrlWidth( -m_pVerScrollBar->Position()->GetWidth() );
            m_pVerScrollBar->SetVisible( true );
        }
        else if ( !m_pVerScrollBar->GetVisible() )
        {
            SetAllCtrlWidth( -m_pVerScrollBar->Position()->GetWidth() );
            m_pVerScrollBar->SetVisible( true );
        }
        m_pVerScrollBar->SetMax( ( nUnVisibleH + m_PerPosPiexl - 1) / m_PerPosPiexl );
        m_pVerScrollBar->CalcBarInfo();
		m_pVerScrollBar->Invalidate();
    }
}

TSubItemInfo* CxdListCtrl::FindSubItemInfo( CxdGraphicsControl *ApItemParent, bool bDel )
{
    for ( TSubItemListIT it = m_ltListSubItems.begin(); it != m_ltListSubItems.end(); it++ )
    {
        TSubItemInfo *pInfo = *it;
        if ( (DWORD_PTR)ApItemParent == (DWORD_PTR)pInfo->FpParentCtrl )
        {
            if ( bDel )
            {
                m_ltListSubItems.erase( it );
            }
            return pInfo;
        }
    }
    return NULL;
}

int CxdListCtrl::MoveItem( CxdGraphicsControl *ApItem, int AItemY, TSubItemInfo *pInfo )
{
    ApItem->Position()->SetTop( AItemY );
    int y = ApItem->Position()->GetBottom();

    if ( !Assigned(pInfo) )
    {
        pInfo = FindSubItemInfo( ApItem, false );
    }
    if ( !Assigned(pInfo) || !pInfo->FIsVisibleSubCtrls || pInfo->FSubCtrlsList.empty() ) return y + m_ItemUpDownSpace;

    for ( TxdGraphicsCtrlListIT it = pInfo->FSubCtrlsList.begin(); it != pInfo->FSubCtrlsList.end(); it++ )
    {
        y += m_ItemUpDownSpace;
        CxdGraphicsControl *pSubItem = *it;
        pSubItem->Position()->SetTop( y );
        y = pSubItem->Position()->GetBottom();
    }
    return y + m_ItemUpDownSpace;
}

void CxdListCtrl::SetAllCtrlWidth( int AOffsetWidth )
{
    for ( TxdGraphicsCtrlListIT it = m_ltListItems.begin(); it != m_ltListItems.end(); it++ )
    {
        CxdGraphicsControl *pItem = *it;
        pItem->Position()->SetWidth( pItem->Position()->GetWidth() + AOffsetWidth );
        pItem->Position()->NotifyPositionChanged();
        TSubItemInfo *pSubInfo = FindSubItemInfo( pItem, false );
        if ( Assigned(pSubInfo) )
        {
            for ( TxdGraphicsCtrlListIT itSub = pSubInfo->FSubCtrlsList.begin(); itSub != pSubInfo->FSubCtrlsList.end(); itSub++ )
            {
                CxdGraphicsControl *pSubItem = *itSub;
                pSubItem->Position()->SetWidth( pSubItem->Position()->GetWidth() + AOffsetWidth );
                pSubItem->Position()->NotifyPositionChanged();
            }
        }
    }
}

void CxdListCtrl::DoVerBarScroll( CxdBar* pSender, bool bChangedByMouse, int nOldBarPos )
{
    if ( m_ltListItems.empty() ) return;

    ClearBufferBitmap();
    if ( m_pVerScrollBar->GetBarPos() == m_pVerScrollBar->GetMax() - 1 )
    {
        //最后
        int nY = Position()->GetHeight();
        int nH = 0;
        for ( TxdGraphicsCtrlList::reverse_iterator it = m_ltListItems.rbegin(); it != m_ltListItems.rend(); it++ )
        {
            CxdGraphicsControl *pObj = *it;
            nH = pObj->Position()->GetHeight();
            TSubItemInfo *pInfo = FindSubItemInfo( pObj, false );
            if ( Assigned(pInfo) && pInfo->FIsVisibleSubCtrls )
            {
                int nCount(0);
                for ( TxdGraphicsCtrlListIT itSub = pInfo->FSubCtrlsList.begin(); itSub != pInfo->FSubCtrlsList.end(); itSub++ )
                {
                    nCount++;
                    CxdGraphicsControl *pObj = *itSub;
                    nH += pObj->Position()->GetHeight();
                }
                nH += nCount * m_ItemUpDownSpace;
            }
            nY -= nH;
            MoveItem( pObj, nY );
            nY -= m_ItemUpDownSpace;
        }
    }
    else
    {
        int nY = -m_pVerScrollBar->GetBarPos() * m_PerPosPiexl;
        for ( TxdGraphicsCtrlListIT it = m_ltListItems.begin(); it != m_ltListItems.end(); it++ )
        {
            CxdGraphicsControl *pObj = *it;
            nY = MoveItem( pObj, nY );
        }
    }
    if ( bChangedByMouse )
    {
        if ( m_pVerScrollBar->GetBarPos() == m_pVerScrollBar->GetMax() - 1 )
        {
            NotifyInterface( MoveToBottom, this );
        }
        else if ( 0 == m_pVerScrollBar->GetBarPos() )
        {
            NotifyInterface( MoveToTop, this );
        }
    }
    Invalidate();
}

void CxdListCtrl::DoItemClick( CxdUIComponent* ApItem )
{
    CxdGraphicsControl *pObj = dynamic_cast<CxdGraphicsControl*>(ApItem);
	if ( m_ClickItemAutoExpand )
	{
		TSubItemInfo *pInfo = FindSubItemInfo( pObj, false );
		if ( Assigned(pInfo) )
		{
			TxdGraphicsCtrlListIT itParent = find( m_ltListItems.begin(), m_ltListItems.end(), pObj );
			SetSubItemCtrlsVisible( itParent, pInfo, !pInfo->FIsVisibleSubCtrls );
		}
	}
    NotifyInterfaceEx( ListItemClick, pObj, NULL );
}

void CxdListCtrl::SetSubItemCtrlsVisible( TxdGraphicsCtrlListIT &AitParent, TSubItemInfo *ApSubInfo, bool bSetVisible )
{
    if ( ApSubInfo == NULL || ApSubInfo->FIsVisibleSubCtrls == bSetVisible ) return;

    CxdGraphicsControl *pItemParent = *AitParent;
    ApSubInfo->FIsVisibleSubCtrls = bSetVisible;
    for ( TxdGraphicsCtrlListIT it = ApSubInfo->FSubCtrlsList.begin(); it != ApSubInfo->FSubCtrlsList.end(); it++ )
    {
        CxdGraphicsControl *pSubItem = *it;
		if ( ApSubInfo->FIsVisibleSubCtrls != pSubItem->GetVisible() )
		{
            NotifyInterfaceEx( SubItemVisible, pItemParent, bSetVisible );
		}
        pSubItem->SetVisible( ApSubInfo->FIsVisibleSubCtrls );
    }
    int nTop = pItemParent->Position()->GetTop();
    int nY = MoveItem( pItemParent, pItemParent->Position()->GetTop(), ApSubInfo );
    
    while ( ++AitParent != m_ltListItems.end() )
    {
        CxdGraphicsControl *pItem = *AitParent;
        TSubItemInfo *pSubInfo = FindSubItemInfo( pItem, false );
        nY = MoveItem( pItem, nY );
    }
    ReSettingScrollInfo( nTop );
}

void CxdListCtrl::CalcAllCtrlsHeight( void )
{
    m_nAllCtrlsHeight = 0;
    int nCount(0);
    for ( TxdGraphicsCtrlListIT it = m_ltListItems.begin(); it != m_ltListItems.end(); it++ )
    {
        CxdGraphicsControl *pObj = *it;
        if ( pObj->GetVisible() )
        {
            nCount++;
            m_nAllCtrlsHeight += pObj->Position()->GetHeight();
        }
    }
    for ( TSubItemListIT it = m_ltListSubItems.begin(); it != m_ltListSubItems.end(); it++ )
    {
        TSubItemInfo *pInfo = *it;
        if ( pInfo->FIsVisibleSubCtrls )
        {
            for ( TxdGraphicsCtrlListIT itItem = pInfo->FSubCtrlsList.begin(); itItem != pInfo->FSubCtrlsList.end(); itItem++ )
            {
                CxdGraphicsControl *pObj = *itItem;
                nCount++;
                m_nAllCtrlsHeight += pObj->Position()->GetHeight();
            }
        }
    }
    m_nAllCtrlsHeight += (nCount - 1) * m_ItemUpDownSpace;
}

void CxdListCtrl::DoPositionChanged( CxdPosition *ApPosition )
{
    CalcAllCtrlsHeight();
    CheckVerScrollBar();
}

void CxdListCtrl::DoSubItemClick( CxdUIComponent* ApItem )
{
    CxdGraphicsControl *pObj =dynamic_cast<CxdGraphicsControl*>(ApItem);
    if ( IsExsitsItListItemClick() )
    {
        TSubItemInfo *pInfo = FindSubItemInfoBySub( pObj );
        if ( Assigned(pInfo) )
        {
            NotifyInterfaceEx( ListItemClick, pObj, pInfo->FpParentCtrl );
        }
    }
}

TSubItemInfo* CxdListCtrl::FindSubItemInfoBySub( CxdGraphicsControl *ApSubItem )
{
    for ( TSubItemListIT it = m_ltListSubItems.begin(); it != m_ltListSubItems.end(); it++ )
    {
        TSubItemInfo *pInfo = *it;
        for ( TxdGraphicsCtrlListIT itItem = pInfo->FSubCtrlsList.begin(); itItem != pInfo->FSubCtrlsList.end(); itItem++ )
        {
            CxdGraphicsControl *pItem = *itItem;
            if ( (DWORD_PTR)ApSubItem == (DWORD_PTR)pItem )
            {
                return pInfo;
            }
        }
    }
    return NULL;
}

void CxdListCtrl::AllExpand( void )
{
    for ( TSubItemListIT it = m_ltListSubItems.begin(); it != m_ltListSubItems.end(); it++ )
    {
        TSubItemInfo *pInfo = *it;
        if ( !pInfo->FIsVisibleSubCtrls )
        {
            TxdGraphicsCtrlListIT itParent = find( m_ltListItems.begin(), m_ltListItems.end(), pInfo->FpParentCtrl );
            SetSubItemCtrlsVisible( itParent, pInfo, true );
        }
    } 
	if ( Assigned(m_pVerScrollBar) )
	{
		m_pVerScrollBar->SetBarPos( 0 );
	}
}

void CxdListCtrl::AllContract( void )
{
    for ( TSubItemListIT it = m_ltListSubItems.begin(); it != m_ltListSubItems.end(); it++ )
    {

        TSubItemInfo *pInfo = *it;
        if ( pInfo->FIsVisibleSubCtrls )
        {
            TxdGraphicsCtrlListIT itParent = find( m_ltListItems.begin(), m_ltListItems.end(), pInfo->FpParentCtrl );
            SetSubItemCtrlsVisible( itParent, pInfo, false );
        }
    }
	if ( Assigned(m_pVerScrollBar) )
	{
		m_pVerScrollBar->SetBarPos( 0 );
	}
}

void CxdListCtrl::DoItemStateChanged( CxdUIComponent* ApItem )
{
    CxdGraphicsControl *pItem = dynamic_cast<CxdGraphicsControl*>(ApItem);
    if ( pItem->GetCurUISate() == uiNormal )
    {
        if ( m_pCurActiveCtrl != NULL )
        {
            m_pCurActiveCtrl = NULL;
            ResetActiveParent( m_ltListItemActive, NULL );
        }
    }
	else if ( pItem->GetCurUISate() == uiActive || pItem->GetCurUISate() == uiDown )
	{
		m_pCurActiveCtrl = pItem;
		ResetActiveParent( m_ltListItemActive, m_pCurActiveCtrl );
	}
}

void CxdListCtrl::DoSubItemStateChanged( CxdUIComponent* ApItem )
{
    CxdGraphicsControl *pItem = dynamic_cast<CxdGraphicsControl*>(ApItem);
    if ( pItem->GetCurUISate() == uiNormal )
    {
        if ( m_pCurSubActiveCtrl != NULL )
        {
            m_pCurSubActiveCtrl = NULL;
            ResetActiveParent( m_ltListSubItemActive, NULL );
        }
    }
    else if ( pItem->GetCurUISate() == uiActive || pItem->GetCurUISate() == uiDown )
    {
        m_pCurSubActiveCtrl = pItem;
        ResetActiveParent( m_ltListSubItemActive, m_pCurSubActiveCtrl );
    }
}

void CxdListCtrl::ReSettingScrollInfo( int ATop )
{
    int nOldUnH = m_nAllCtrlsHeight;
    CalcAllCtrlsHeight();
    CheckVerScrollBar();
    Invalidate();
}

void CxdListCtrl::ClearAllItem( void )
{
    m_pCurActiveCtrl = NULL;
    m_pCurSubActiveCtrl = NULL;
    ResetActiveParent( m_ltListSubItemActive, NULL );
    ResetActiveParent( m_ltListItemActive, NULL );
    for ( TxdGraphicsCtrlListIT it = m_ltListItems.begin(); it != m_ltListItems.end(); it++ )
    {
        CxdGraphicsControl *pObj = *it;
        DoDeleteItem( pObj );
        delete pObj;
    }
    m_ltListItems.clear();
    for ( TSubItemListIT it = m_ltListSubItems.begin(); it != m_ltListSubItems.end(); it++ )
    {
        TSubItemInfo *pInfo = *it;
        for ( TxdGraphicsCtrlListIT itItem = pInfo->FSubCtrlsList.begin(); itItem != pInfo->FSubCtrlsList.end(); itItem++ )
        {
            CxdGraphicsControl *pObj = *itItem;
            DoDeleteSubItem( pObj );
            delete pObj;
        }
        delete pInfo;
    }
    m_ltListSubItems.clear();
	if ( Assigned(m_pVerScrollBar) )
	{
		m_pVerScrollBar->SetBarPos( 0 );
		m_pVerScrollBar->SetVisible( false );
	}
	m_nAllCtrlsHeight = 0;
    Invalidate();
}

void CxdListCtrl::DeleteItem( CxdGraphicsControl *ApCtrl )
{
    TxdGraphicsCtrlList *pDelList = NULL; 
    TxdGraphicsCtrlListIT itParent = find( m_ltListItems.begin(), m_ltListItems.end(), ApCtrl );
    if ( itParent != m_ltListItems.end() )
    {
        //删除项
        pDelList = new TxdGraphicsCtrlList;
        int nTop = ApCtrl->Position()->GetTop();
        ApCtrl->SetVisible( false );
        itParent = m_ltListItems.erase( itParent );
        pDelList->push_back( ApCtrl );
        if ( ApCtrl == m_pCurActiveCtrl )
        {
            if ( Assigned(m_pCurSubActiveCtrl) )
            {
                ResetActiveParent( m_ltListSubItemActive, NULL );
                m_pCurSubActiveCtrl = NULL;
            }
            ResetActiveParent( m_ltListItemActive, NULL );
            m_pCurActiveCtrl = NULL;
        }
        DoDeleteItem( ApCtrl );
        TSubItemInfo *pSubInfo = FindSubItemInfo( ApCtrl, true );
        if ( Assigned(pSubInfo) )
        {
            for ( TxdGraphicsCtrlListIT it = pSubInfo->FSubCtrlsList.begin(); it != pSubInfo->FSubCtrlsList.end(); it++ )
            {
                CxdGraphicsControl *pCtrl = *it;
                DoDeleteSubItem( pCtrl );
                delete pCtrl;
            }
            delete pSubInfo;
        }

        int nPos( nTop );
        while ( itParent != m_ltListItems.end() )
        {
            CxdGraphicsControl *pObj = *itParent++;
            nPos = MoveItem( pObj, nPos );
        }
        ReSettingScrollInfo( nTop );
    }
    else
    {
        TSubItemInfo *pInfo = FindSubItemInfoBySub(ApCtrl);
        if ( Assigned(pInfo) )
        {
            pDelList = new TxdGraphicsCtrlList;
            if ( pInfo->FIsVisibleSubCtrls )
            {
                ApCtrl->SetVisible( false );
            }
            pDelList->push_back( ApCtrl );
            if ( ApCtrl == m_pCurSubActiveCtrl )
            {
                ResetActiveParent( m_ltListSubItemActive, NULL );
                m_pCurSubActiveCtrl = NULL;
            }
            DoDeleteSubItem( ApCtrl );
            int nPos( ApCtrl->Position()->GetTop() );

            TxdGraphicsCtrlListIT itItem = find( pInfo->FSubCtrlsList.begin(), pInfo->FSubCtrlsList.end(), ApCtrl );
            itItem = pInfo->FSubCtrlsList.erase( itItem );
            while ( itItem != pInfo->FSubCtrlsList.end() )
            {
                CxdGraphicsControl *pObj = *itItem++;
                pObj->Position()->SetTop( nPos );
                nPos = pObj->Position()->GetBottom() + m_ItemUpDownSpace;
            }

            if ( pInfo->FIsVisibleSubCtrls )
            {
                itItem = find( m_ltListItems.begin(), m_ltListItems.end(), pInfo->FpParentCtrl );
                while ( ++itItem != m_ltListItems.end() )
                {
                    CxdGraphicsControl *pObj = *itItem;
                    nPos = MoveItem( pObj, nPos );
                }
                ReSettingScrollInfo( pInfo->FpParentCtrl->Position()->GetTop() );
            }

            if ( pInfo->FSubCtrlsList.empty() )
            {
                FindSubItemInfo( pInfo->FpParentCtrl, true );
                delete pInfo;
            }
        }
    }
    if ( Assigned(pDelList) )
    {
        CxdCustomMessageInfo *pPostInfo = new CxdCustomMessageInfo;
        pPostInfo->MessageID = CtDeleteCtrlID;
        pPostInfo->MessageData = pDelList;
        PostCustomMessage( pPostInfo );
    }
}

bool CxdListCtrl::OnCustomMessage( CxdCustomMessageInfo *pInfo )
{
    if ( !CxdGraphicsControl::OnCustomMessage(pInfo) )
    {
        if ( pInfo->MessageID == CtDeleteCtrlID )
        {
            TxdGraphicsCtrlList *pDelList = (TxdGraphicsCtrlList*)pInfo->MessageData;
            for ( TxdGraphicsCtrlListIT it = pDelList->begin(); it != pDelList->end(); it++ )
            {
                CxdGraphicsControl *pObj = *it;
                delete pObj;
            }
            delete pDelList;
            delete pInfo;
            return true;
        }
    }
    return false;
}

bool CxdListCtrl::AddListItemActiveCtrl( CxdGraphicsControl *ApItemCtrl )
{
    return AddActiveCtrl( m_ltListItemActive, ApItemCtrl );
}

void CxdListCtrl::ResetActiveParent( TxdGraphicsCtrlList &AList, CxdGraphicsControl *ApParent )
{
    bool bVisible = Assigned( ApParent );
    for ( TxdGraphicsCtrlListIT it = AList.begin(); it != AList.end(); it++ )
    {
        CxdGraphicsControl *pObj = *it;
        pObj->SetVisible( bVisible );
        pObj->ReSetParent( bVisible ? ApParent : this );
		if ( bVisible )
		{
			DoSetActiveCtrlPos( pObj, ApParent->Position()->GetHeight() );
		}
    }
}

bool CxdListCtrl::AddActiveCtrl( TxdGraphicsCtrlList &AList, CxdGraphicsControl *ApItemCtrl )
{
    if ( m_pParent == NULL ) return false;
    if ( AList.end() != find(AList.begin(), AList.end(), ApItemCtrl) ) return false;

    if ( ApItemCtrl->IsCreated() )
    {
        if ( this != ApItemCtrl->GetComponentParent() )
        {
            ApItemCtrl->ReSetParent( this );
        }
    }
    else
    {
        if ( !ApItemCtrl->xdCreate( ApItemCtrl->Position()->GetLeft(), ApItemCtrl->Position()->GetTop(), 
             ApItemCtrl->Position()->GetWidth(), ApItemCtrl->Position()->GetHeight(), this) )
        {
            return false;
        }
    }
    ApItemCtrl->SetVisible( false );
    AList.push_back( ApItemCtrl );
    return true;
}

bool CxdListCtrl::AddListSubItemActiveCtrl( CxdGraphicsControl *ApItemCtrl )
{
    return AddActiveCtrl( m_ltListSubItemActive, ApItemCtrl );
}

void CxdListCtrl::DeleteBySubCtrl( CxdGraphicsControl *ApSubCtrl )
{
    CxdUIComponent* pParent = ApSubCtrl->GetComponentParent();
    if ( Assigned(pParent) )
    {
        DeleteItem( dynamic_cast<CxdGraphicsControl*>(pParent) );
    }
}

bool CxdListCtrl::IsExsitsItem( CxdGraphicsControl *ApCtrl )
{
    if ( m_ltListItems.end() != find(m_ltListItems.begin(), m_ltListItems.end(), ApCtrl) ) return true;
    return Assigned( FindSubItemInfoBySub(ApCtrl) );
}

bool CxdListCtrl::IsItemHasSubItem(CxdGraphicsControl *ApCtrl)
{
	if ( m_ltListItems.end() == find(m_ltListItems.begin(), m_ltListItems.end(), ApCtrl) ) return false;
	TSubItemInfo *pInfo = FindSubItemInfo( ApCtrl, false );
	if ( Assigned(pInfo) )
	{
		return true;
	}
	return false;
}

int CxdListCtrl::GetAllCtrlsHeight()
{
	int nHeight = m_nAllCtrlsHeight;
	if(m_ltListItems.size())
	{
		nHeight += GetItemUpDownSpace()*(m_ltListItems.size()-1);
	}
	return m_nAllCtrlsHeight;
}

int CxdListCtrl::GetScrollPos( void )
{
	if ( Assigned(m_pVerScrollBar) && m_pVerScrollBar->GetVisible() )
	{
		return m_pVerScrollBar->GetBarPos();
	}
	return -1;
}

void CxdListCtrl::SetScrollPos( const int &APos )
{
	if ( Assigned(m_pVerScrollBar) && m_pVerScrollBar->GetVisible() )
	{
		m_pVerScrollBar->SetBarPos( APos );
	}
}

CxdGraphicsControl* CxdListCtrl::GetSubItemParentCtrl( CxdGraphicsControl *ApSubCtrl )
{
	TSubItemInfo *pInfo = FindSubItemInfoBySub( ApSubCtrl );
	if ( Assigned(pInfo) )
	{
		return pInfo->FpParentCtrl;
	}
	return NULL;
}

void CxdListCtrl::SetItemExpand( CxdGraphicsControl *ApCtrl)
{
	if ( m_ltListItems.end() == find(m_ltListItems.begin(), m_ltListItems.end(), ApCtrl) ) return;
	TSubItemInfo *pInfo = FindSubItemInfo( ApCtrl, false );
	if ( Assigned(pInfo) )
	{
		TxdGraphicsCtrlListIT itParent = find( m_ltListItems.begin(), m_ltListItems.end(), ApCtrl );
		SetSubItemCtrlsVisible( itParent, pInfo, true );
	}
}

void CxdListCtrl::SetScrollBarPos( CxdGraphicsControl *ApCtrl )
{
	int nY = 0;
	int nX = 0;
	if ( m_ltListItems.end() != find(m_ltListItems.begin(), m_ltListItems.end(), ApCtrl) )
	{
		nY += ApCtrl->Position()->GetTop();
		nX += ApCtrl->Position()->GetLeft();
	}
	int nUnVisibleH = nY;
	int nUnVisibleW = nX;
	if ( Assigned(m_pVerScrollBar) && m_pVerScrollBar->GetVisible() )
	{
		if ( nUnVisibleH > (m_nAllCtrlsHeight - Position()->GetHeight() + m_PerPosPiexl - 1) )
		{
			nUnVisibleH = m_nAllCtrlsHeight - Position()->GetHeight();
		}
		m_pVerScrollBar->SetBarPos( nUnVisibleH / m_PerPosPiexl );
	}
}

CxdGraphicsControl* CxdListCtrl::GetItem( const int &AID )
{
	for ( TxdGraphicsCtrlListIT it = m_ltListItems.begin(); it != m_ltListItems.end(); it++ )
	{
		CxdGraphicsControl *pItem = *it;
		if ( AID == (int)pItem->Tag )
		{
			return pItem;
		}
	}
	return NULL;
}

CxdGraphicsControl* CxdListCtrl::GetSubItem( const int &AID, CxdGraphicsControl *ApParentCtrl )
{
	if ( m_ltListItems.end() == find(m_ltListItems.begin(), m_ltListItems.end(), ApParentCtrl) ) return false;
	TSubItemInfo *pInfo = FindSubItemInfo( ApParentCtrl, false );
	if ( Assigned(pInfo) )
	{
		for ( TxdGraphicsCtrlListIT it = pInfo->FSubCtrlsList.begin(); it != pInfo->FSubCtrlsList.end(); it++ )
		{
			CxdGraphicsControl *pSubItem = *it;
			if ( AID == (int)pSubItem->Tag )
			{
				return pSubItem;
			}
		}
	}
	return NULL;
}

void CxdListCtrl::ClearAllActiveItem( void )
{
	m_pCurActiveCtrl = NULL;
	m_pCurSubActiveCtrl = NULL;
	for ( TxdGraphicsCtrlListIT it = m_ltListItemActive.begin(); it != m_ltListItemActive.end(); it++ )
	{
		CxdGraphicsControl *pObj = *it;
		delete pObj;
	}
	m_ltListItemActive.clear();
	for ( TxdGraphicsCtrlListIT it = m_ltListSubItemActive.begin(); it != m_ltListSubItemActive.end(); it++ )
	{
		CxdGraphicsControl *pSubObj = *it;
		delete pSubObj;
	}
	m_ltListSubItemActive.clear();
	Invalidate();
}

void CxdListCtrl::ReCalcAllSize( void )
{
	m_nAllCtrlsHeight = 0;
	int nCount(0);
	int nY(0);
	for ( TxdGraphicsCtrlListIT it = m_ltListItems.begin(); it != m_ltListItems.end(); it++ )
	{
		CxdGraphicsControl *pObj = *it;
		if ( pObj->GetVisible() )
		{
			nCount++;
			pObj->Position()->SetTop( nY );
			nY += pObj->Position()->GetHeight() + m_ItemUpDownSpace;
			m_nAllCtrlsHeight += pObj->Position()->GetHeight();
		}
	}
	for ( TSubItemListIT it = m_ltListSubItems.begin(); it != m_ltListSubItems.end(); it++ )
	{
		TSubItemInfo *pInfo = *it;
		if ( pInfo->FIsVisibleSubCtrls )
		{
			for ( TxdGraphicsCtrlListIT itItem = pInfo->FSubCtrlsList.begin(); itItem != pInfo->FSubCtrlsList.end(); itItem++ )
			{
				CxdGraphicsControl *pObj = *itItem;
				nCount++;
				m_nAllCtrlsHeight += pObj->Position()->GetHeight();
			}
		}
	}
	m_nAllCtrlsHeight += (nCount - 1) * m_ItemUpDownSpace;
	CheckVerScrollBar();
}

int CxdListCtrl::GetVerScrollBarWidth( void )
{
	int nW( 0 );
	if ( Assigned(m_pVerScrollBar) && m_pVerScrollBar->GetVisible() )
	{
		nW = m_pVerScrollBar->ImageInfo()->GetBitmap()->GetWidth();
	}
	return nW;
}

void CxdListCtrl::OnMidMouseWheel( UINT nFlags, short zDelta, int x, int y )
{
    if ( Assigned(m_pVerScrollBar) && m_pVerScrollBar->GetVisible() )
    {
        m_pVerScrollBar->SetBarPos( m_pVerScrollBar->GetBarPos() + (zDelta < 0 ? 1 : -1) );
    }
}

void CxdListCtrl::LoopItem( CxdGraphicsControl* ApItemParent, IxdLoopCtrlInterface* ApLoop )
{
    if ( IsNull(ApItemParent) )
    {
        for ( TxdGraphicsCtrlListIT it = m_ltListItems.begin(); it != m_ltListItems.end(); it++ )
        {
            CxdGraphicsControl* pObj = *it;
            bool bBreak = false;
            ApLoop->Notify( pObj, bBreak );
            if( bBreak ) return;
        }
    }
    else
    {
        TSubItemInfo* pInfo = FindSubItemInfo( ApItemParent, false );
        if ( Assigned(pInfo) )
        {
            for ( TxdGraphicsCtrlListIT it = pInfo->FSubCtrlsList.begin(); it != pInfo->FSubCtrlsList.end(); it++ )
            {
                CxdGraphicsControl* pObj = *it;
                bool bBreak = false;
                ApLoop->Notify( pObj, bBreak );
                if( bBreak ) return;
            }
        }
    }
}

void CxdListCtrl::OnPaintComponent( Graphics &G, const Rect &ADestR )
{
    CxdGraphicsControl::OnPaintComponent( G, ADestR );
    if ( m_BorderLineColor > 0 )
    {
        Rect R = ADestR;
        R.Width -= 1;
        R.Height -= 1;
        G.DrawRectangle( &Pen( m_BorderLineColor, 1), R );
    }
}


