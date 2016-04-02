#include "stdafx.h"
#include "JxdPopupMenu.h"
#include "JxdGraphicsControl.h"
#include "JxdGdiPlusBasic.h"
#include "JxdBar.h"

//CxdPopuItemCtrl

CxdPopuItemCtrl::CxdPopuItemCtrl( CxdPopupMenu *ApOwner )
{
    m_pOwnerMenu = ApOwner;
}

void CxdPopuItemCtrl::DoDrawSubjoinBK( Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt )
{
    TPopupMenuItem *pItem = (TPopupMenuItem*)Tag;
    if ( Assigned(pItem) && m_pOwnerMenu->GetDrawIconWidth() > 0 && 
        Assigned(m_pOwnerMenu->m_pImgIcons) && m_pOwnerMenu->m_pImgIcons->IsExsitsBitmap() && 
        Assigned(pItem) && !pItem->IsSeparator() && m_pOwnerMenu->m_pImgIcons->GetBitmapItemCount() > 0 )
    {
        if ( !pItem->FIsCheck && -1 == pItem->FImgIndex )
        {
            return;
        }
        int nCount =  m_pOwnerMenu->m_pImgIcons->GetBitmapItemCount();
        if ( pItem->FImgIndex < m_pOwnerMenu->m_pImgIcons->GetBitmapItemCount() )
        {
            Rect R( ADestRect );
            R.X += m_pOwnerMenu->GetDrawIconLeftSpace();
            R.Y += m_pOwnerMenu->GetDrawIconTopSpace();
            R.Width = m_pOwnerMenu->GetDrawIconWidth();
            R.Height = m_pOwnerMenu->m_pImgIcons->GetBitmap()->GetHeight() / m_pOwnerMenu->m_pImgIcons->GetBitmapItemCount();
            DrawCommonImage(ADestG, R, m_pOwnerMenu->m_pImgIcons, pItem->FIsCheck ? 0 : pItem->FImgIndex, ApImgAtt );
        }
    }
}

void CxdPopuItemCtrl::DoChangedFontColor( DWORD &AFontColor )
{
    if ( GetCurUISate() != uiNormal ) AFontColor = m_pOwnerMenu->GetItemActiveFontColor();
}

//CxdPoupForm
BEGIN_MESSAGE_MAP(CxdPopupForm, CxdForm)
    ON_WM_ACTIVATE()
END_MESSAGE_MAP()

CxdPopupForm::CxdPopupForm(): CxdForm()
{
	SetTopMost( true );
    SetDoubleBuffer( true );
    SetAutoChangedSize( false );
    SetAutoMoveForm( false );
    SetBroderColor( 0x666666 );
	m_CtrlRightSpace = m_CtrlLeftSpace = m_CtrlTopSpace = m_CtrlBottomSpace = 5;
    InitInterface( WndActiveChanged );
    InitInterface( ItemClick );
}

CxdPopupForm::~CxdPopupForm()
{
    DeleteInterface( WndActiveChanged );
    DeleteInterface( ItemClick );
}

void CxdPopupForm::OnGetCreateInfo( CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle )
{
    CxdForm::OnGetCreateInfo(ADefaultWndClassName, ANewClassName, AdwStyle, AdwExStyle);
    AdwStyle = WS_POPUP;
    ANewClassName = CtxdPopupForm;
}

void CxdPopupForm::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized )
{
    CxdForm::OnActivate(nState, pWndOther, bMinimized);
    if ( !IsExsitsItWndActiveChanged() || !m_pItWndActiveChanged->Notify(this, nState, pWndOther) )
	{
		if(WA_INACTIVE == nState)
		{
			CWnd *pWnd = pWndOther;
			if ( !Assigned(pWnd) || (pWnd->GetSafeHwnd() != GetSafeHwnd() ) )
			{
				if ( GetVisible() ) Close(10);
			}
		}
	}
}

void CxdPopupForm::AddExtendCtrlItem(CxdGraphicsControl *pItem)
{
	if(pItem)
	{
		if(!pItem->IsCreated())
		{
			pItem->xdCreate(pItem->Position()->GetLeft(), pItem->Position()->GetTop(),
				pItem->Position()->GetWidth(), pItem->Position()->GetHeight(), this);
		}

		pItem->ReSetParent(this);
		int nATop = pItem->Position()->GetTop();
		int nALeft = pItem->Position()->GetLeft();
		DWORD dwAk(0);
		if (nATop < 0)
		{
			pItem->Position()->SetTop(Position()->GetHeight() + nATop);
			dwAk |= akBottom;
		}
		if(nALeft < 0)
		{
			pItem->Position()->SetLeft(Position()->GetWidth() + nALeft);
			dwAk |= akRight;
		}
		pItem->Anchors()->SetValue(dwAk);
		pItem->Position()->NotifyPositionChanged();
	}
}

void CxdPopupForm::AddListCtrlItem(CxdGraphicsControl *pItem, int nHeight)
{
	if(pItem && m_pPopupCtrlList)
	{
		m_pPopupCtrlList->AddListItem(pItem, nHeight);

		CSize si = pItem->CalcStringSize( pItem->GetCaption() );
		int nW = si.cx + pItem->GetCaptionLeftSpace() + GetCtrlLeftSpace() + GetCtrlRightSpace() + 5;
		int nH = m_pPopupCtrlList->GetAllCtrlsHeight() + GetCtrlTopSpace() + GetCtrlBottomSpace();

        if ( nW > Position()->GetWidth() ) Position()->SetWidth( nW );
        Position()->SetHeight( nH );
        Position()->NotifyPositionChanged();

        m_pPopupCtrlList->Position()->SetLeft( GetCtrlLeftSpace() );
        m_pPopupCtrlList->Position()->SetTop( GetCtrlTopSpace() );
        m_pPopupCtrlList->Position()->SetWidth( Position()->GetWidth() - GetCtrlLeftSpace() - GetCtrlRightSpace() );
        m_pPopupCtrlList->Position()->SetHeight( Position()->GetHeight() - GetCtrlTopSpace() - GetCtrlBottomSpace() );
        m_pPopupCtrlList->Position()->NotifyPositionChanged();
	}
}

void CxdPopupForm::OnCreated()
{
	m_pPopupCtrlList = new CxdListCtrl;
    m_pPopupCtrlList->SetItemUpDownSpace( 0 );
    m_pPopupCtrlList->SetSubItemLeftSpace( 0 );
    m_pPopupCtrlList->SetClickItemAutoExpand( true );
    m_pPopupCtrlList->SetCaptureOnMouseDown(true);
    m_pPopupCtrlList->SetItListItemClick( NewInterfaceExByClass( CxdPopupForm, CxdGraphicsControl*, CxdGraphicsControl*, DoItemClick) );
    m_pPopupCtrlList->Anchors()->AddAll();
    m_pPopupCtrlList->xdCreate( GetCtrlLeftSpace(), GetCtrlTopSpace(), 
        Position()->GetWidth() - GetCtrlLeftSpace() - GetCtrlRightSpace(),
        Position()->GetHeight() - GetCtrlTopSpace() - GetCtrlBottomSpace(), this );
}

void CxdPopupForm::DoItemClick( CxdGraphicsControl* ApCtrl, CxdGraphicsControl* ApCtrlParent )
{
    NotifyInterface( ItemClick, ApCtrl );
}

void CxdPopupForm::NotifySpaceChanged( void )
{
    if ( Assigned(m_pPopupCtrlList) )
    {
        m_pPopupCtrlList->Position()->SetLeft( GetCtrlLeftSpace() );
        m_pPopupCtrlList->Position()->SetTop( GetCtrlTopSpace() );
        m_pPopupCtrlList->Position()->SetWidth( Position()->GetWidth() - GetCtrlLeftSpace() - GetCtrlRightSpace() );
        m_pPopupCtrlList->Position()->SetHeight( Position()->GetHeight() - GetCtrlTopSpace() - GetCtrlBottomSpace() );
        m_pPopupCtrlList->Position()->NotifyPositionChanged();
    }
}

//CxdPopupMenu
CxdPopupMenu::CxdPopupMenu()
{
    m_pCtrlOld = NULL;
    m_pMainPopupForm = NULL;
    m_pImgStyle = NULL;
    m_pImgBK = NULL;
    m_pImgNormalItem = NULL;
    m_pImgSeparator = NULL;
    m_pImgIcons = NULL;
    m_pFontInfo = NULL;
	m_pImgSubItems = NULL;
    m_DrawIconWidth = 16;
    m_DrawIconLeftSpace = 4;
    m_DrawIconTopSpace = 6;
    m_CaptionIconSpace = 10;
    m_CaptionTopSpace = 4;
    m_FormSpace = 2;
    m_FixedWidth = 0;
    m_MinHeight = 0;
}

CxdPopupMenu::~CxdPopupMenu()
{
    ClearPopupItemList( &m_ltPopupItems, false );
    SafeDeleteObject( m_pImgStyle );
    SafeDeleteObject( m_pImgBK );
    SafeDeleteObject( m_pImgNormalItem );
    SafeDeleteObject( m_pImgSubItems );
    SafeDeleteObject( m_pImgSeparator );
    SafeDeleteObject( m_pImgIcons );
    SafeDeleteObject( m_pFontInfo );
}

TPopupMenuItem* CxdPopupMenu::PopuMenu( CxdUIComponent *ApOwner, const int &ALeft, const int &ATop, IxdMenuItemInterface *ApBeforePopupItem )
{
    m_pOnPopupEvent = ApBeforePopupItem;

    m_pMainPopupForm = CreatePopupForm();
    if ( IsNull(m_pMainPopupForm) ) return NULL;

	m_pMainPopupForm->SetImageInfo(m_pImgBK);
	m_pMainPopupForm->SetFontInfo(m_pFontInfo);
	m_pMainPopupForm->SetImageFormStyle(m_pImgStyle);
    m_pMainPopupForm->SetItItemClick( NewInterfaceByClass(CxdPopupMenu, CxdGraphicsControl*, DoItemClick) );
    m_pMainPopupForm->SetItWndActiveChanged(NewInterfaceEx3ByClass(CxdPopupMenu, LPVOID, UINT, CWnd*, DoMainItemActiveChanged) );
    m_pMainPopupForm->xdCreate( ALeft, ATop, 20, 20, ApOwner );

    DoAddItemToForm( m_pMainPopupForm, &m_ltPopupItems );
	CalcPopupFormPos( m_pMainPopupForm, &m_pMainPopupForm->Position()->GetCRect() );

	m_pClickItem = NULL;
    m_pMainPopupForm->ShowModel(false);

    m_pOnPopupEvent = NULL;
    ClearSubPopupMenu( NULL );
    SafeDeleteObject( m_pMainPopupForm );
    return m_pClickItem;
}

void CxdPopupMenu::DoMainItemActiveChanged( LPVOID ApSender, UINT AState, CWnd* ApWndOther )
{    
    if ( !Assigned(m_pMainPopupForm) ) return;

    if ( AState == WA_INACTIVE )
    {
        CxdPopupForm *pfrm = (CxdPopupForm*)ApSender;
        CWnd *pWnd = ApWndOther;
        if ( !Assigned(pWnd) || (pWnd->GetSafeHwnd() != m_pMainPopupForm->GetSafeHwnd() && !IsSubPopupForm(pWnd->GetSafeHwnd())) )
        {
            if ( m_pMainPopupForm->GetVisible() )
            {
                m_pMainPopupForm->Close();
            }
        }
    }
}

void CxdPopupMenu::DoItemClick( CxdGraphicsControl* ApCtrl )
{
    if ( !Assigned(m_pMainPopupForm) ) return;

    CxdGraphicsControl *pCtrl = dynamic_cast<CxdGraphicsControl*>(ApCtrl);
    if ( Assigned(pCtrl) )
    {
        m_pClickItem = (TPopupMenuItem*)pCtrl->Tag;
        if ( m_pClickItem->IsHashSubItem() || m_pClickItem->IsSeparator() )
        {
            m_pClickItem = NULL;
            return;
        }
        SetSubPopupUnVisible();
        m_pMainPopupForm->Close();
    }
}

void CxdPopupMenu::DoSubItemUIChanged( CxdUIComponent* ApSender )
{
    
    if ( !Assigned(m_pMainPopupForm) ) return;

    CxdGraphicsControl *pCtrl = dynamic_cast<CxdGraphicsControl*>(ApSender);
    
    if ( Assigned(pCtrl) )
    {
        if ( m_pCtrlOld == pCtrl ) return;

        m_pCtrlOld = pCtrl;
        if ( pCtrl->GetCurUISate() == uiActive )
        {
            if ( !Assigned(FindSubPopupItem(pCtrl, false)) )
            {
                CxdPopupForm *pParent = dynamic_cast<CxdPopupForm*>( pCtrl->GetOwner() );
                ClearSubPopupMenu( pParent );
                TPopupMenuItem *pItemInfo = (TPopupMenuItem*)pCtrl->Tag;
                if ( Assigned( pItemInfo->FpSubList) )
                {
                    CreateSubPopupMenu( pCtrl );
                }
            }            
        }
    }
}

void CxdPopupMenu::DoAddItemToForm(CxdPopupForm *ApForm, TxdPopupItemList *ApList)
{
    for ( TxdPopupItemListIT it = ApList->begin(); it != ApList->end(); it++ )
    {
        TPopupMenuItem *pItem = *it;
        if ( Assigned(m_pOnPopupEvent) )
        {
            m_pOnPopupEvent->Notify( pItem );
        }
        if ( !pItem->FIsVisible ) continue;

		AddItemToListCtrl( ApForm, pItem );
    }
}

void CxdPopupMenu::AddItemToListCtrl(CxdPopupForm *AForm, TPopupMenuItem *ApItemInfo)
{
	CxdImageInfo *pImg = NULL;
	int nH=0, nW=0;
	if ( ApItemInfo->IsSeparator() )
	{
		pImg = m_pImgSeparator;
		nH = Assigned(pImg) && pImg->IsExsitsBitmap() ? pImg->GetBitmap()->GetHeight() : 3;
	}
	else
	{
		pImg =  ApItemInfo->IsHashSubItem() ? m_pImgSubItems : m_pImgNormalItem;
		if(Assigned(pImg) && pImg->IsExsitsBitmap() )
			nH = pImg->GetBitmap()->GetHeight() / pImg->GetBitmapItemCount();
	}
	if ( !Assigned(pImg) || nH == 0 ) return;

	//设置添加的ITEM属性
	CxdGraphicsControl *pItem = CreatePopupItem();
    if ( IsNull(pItem) ) return;

	pItem->Tag = (INT64)ApItemInfo;
	pItem->SetImageInfo( pImg );
	pItem->SetFontInfo( m_pFontInfo );
	if ( !ApItemInfo->IsSeparator() )
	{
		pItem->SetCaptionLeftSpace( GetDrawIconWidth() + GetDrawIconLeftSpace() + GetCaptionIconSpace() );
		pItem->SetCaptionTopSpace( GetCaptionTopSpace() );
        pItem->SetCaption( ApItemInfo->FCaption );
	}
	AForm->AddListCtrlItem(pItem, nH);
	pItem->SetItUIStateChanged( NewInterfaceByClass(CxdPopupMenu, CxdUIComponent*, DoSubItemUIChanged) ); 
}

bool CxdPopupMenu::IsSubPopupForm( const HWND &AHandle )
{
    for ( TPopupFormInfoListIT it = m_ltSubPopup.begin(); it != m_ltSubPopup.end(); it++ )
    {
        TPopupFormInfo *pItem = *it;
        if ( Assigned(pItem->FpItemSubForm) && AHandle == pItem->FpItemSubForm->GetSafeHwnd() )
        {
            return true;
        }
    }
    return false;
}

void CxdPopupMenu::CreateSubPopupMenu( CxdGraphicsControl *ApCtrl )
{
    TPopupFormInfo *pItem = FindSubPopupItem( ApCtrl, false );
    if ( Assigned(pItem) )
    {
        pItem->FpItemSubForm->BringWindowToShow();
        return;
    }
    pItem = new TPopupFormInfo;
    pItem->FpItemCtrl = ApCtrl;
    pItem->FpItemParent = dynamic_cast<CxdPopupForm*>( pItem->FpItemCtrl->GetOwner() );
    pItem->FpItemSubForm = CreatePopupForm();

    pItem->FpItemSubForm->SetImageInfo(m_pImgBK);
    pItem->FpItemSubForm->SetFontInfo(m_pFontInfo);
    pItem->FpItemSubForm->SetImageFormStyle(m_pImgStyle);
    pItem->FpItemSubForm->SetItItemClick( NewInterfaceByClass(CxdPopupMenu, CxdGraphicsControl*, DoItemClick) );
    pItem->FpItemSubForm->SetItWndActiveChanged(NewInterfaceEx3ByClass(CxdPopupMenu, LPVOID, UINT, CWnd*, DoMainItemActiveChanged) );
    pItem->FpItemSubForm->xdCreate( 1, 1, 20, 20, m_pMainPopupForm );
    m_ltSubPopup.push_back( pItem );

    TPopupMenuItem *pItemInfo = (TPopupMenuItem*)pItem->FpItemCtrl->Tag;
    DoAddItemToForm( pItem->FpItemSubForm, pItemInfo->FpSubList );
    
    CRect rTemp = pItem->FpItemSubForm->Position()->GetCRect();
    pItem->FpItemSubForm->Position()->SetLeft( pItem->FpItemParent->Position()->GetLeft() + pItem->FpItemParent->Position()->GetWidth() );
    pItem->FpItemSubForm->Position()->SetTop( pItem->FpItemParent->Position()->GetTop() + pItem->FpItemCtrl->Position()->GetTop() );
	CRect rect = pItem->FpItemCtrl->Position()->GetCRect();
    CalcPopupFormPos( pItem->FpItemSubForm, &rect, pItem->FpItemParent );

    pItem->FpItemSubForm->SetVisible( true );
}

TPopupFormInfo* CxdPopupMenu::FindSubPopupItem( CxdGraphicsControl *ApCtrl, bool bDelete )
{
    for ( TPopupFormInfoListIT it = m_ltSubPopup.begin(); it != m_ltSubPopup.end(); it++ )
    {
        TPopupFormInfo *pItem = *it;
        if ( pItem->FpItemCtrl == ApCtrl )
        {            
            if ( bDelete )
            {
                m_ltSubPopup.erase( it );
            }
            return pItem;
        }
    }
    return NULL;
}

void CxdPopupMenu::ClearSubPopupMenu( CxdPopupForm *ApParent )
{
    if ( Assigned(ApParent) )
    {
        for ( TPopupFormInfoListIT it = m_ltSubPopup.begin(); it != m_ltSubPopup.end(); )
        {
            TPopupFormInfo *pItem = *it;
            if ( pItem->FpItemParent == ApParent )
            {
                ClearSubPopupMenu( pItem->FpItemSubForm );
                delete pItem->FpItemSubForm;
                delete pItem;
                it = m_ltSubPopup.erase( it );
            }
            else
            {
                it++;
            }
        }
    }
    else
    {
        for ( TPopupFormInfoList::reverse_iterator it = m_ltSubPopup.rbegin(); it != m_ltSubPopup.rend(); it++ )
        {
            TPopupFormInfo *pItem = *it;
            delete pItem->FpItemSubForm;
            delete pItem;
        }
        m_ltSubPopup.clear();
    }
    
}

void CxdPopupMenu::ClearPopupItemList( TxdPopupItemList *ApList, bool bFreeList )
{
    if ( !Assigned(ApList) )
    {
        return;
    }
    for ( TxdPopupItemListIT it = ApList->begin(); it != ApList->end(); it++ )
    {
        TPopupMenuItem *pItem = *it;
        ClearPopupItemList( pItem->FpSubList, true );
        delete pItem;
    }
    ApList->clear();
    if ( bFreeList )
    {
        delete ApList;
    }
}

void CxdPopupMenu::SetSubPopupUnVisible( void )
{
    for ( TPopupFormInfoListIT it = m_ltSubPopup.begin(); it != m_ltSubPopup.end(); it++ )
    {
        TPopupFormInfo *pItem = *it;
        pItem->FpItemSubForm->SetVisible( false );
    }
}

void CxdPopupMenu::CalcPopupFormPos( CxdPopupForm *ApCurCalcForm, CRect *ApRect, CxdPopupForm *ApParent/* = NULL*/ )
{
	ApCurCalcForm->FShowOnRight = true;
	ApCurCalcForm->FShowOnTop = true;
	int x = ApCurCalcForm->Position()->GetLeft();
	int y = ApCurCalcForm->Position()->GetTop();
	int w = ApCurCalcForm->Position()->GetWidth();
	int h = ApCurCalcForm->Position()->GetHeight();

    w = m_FixedWidth > 0 ? m_FixedWidth : w;
    h = m_MinHeight > h ? m_MinHeight : h;
    CxdScreen sn;
	int nSpace = 5;
    if ( !Assigned(ApParent) )
    {
        if ( x < 0 )
        {
            x = 0;
        }
        else if ( x + w > sn.GetScreenWidth() )
        {
            x = sn.GetScreenWidth() - w - nSpace - (sn.GetScreenWidth() - x);
			ApCurCalcForm->FShowOnRight = false;
        }
        if ( y < 0 )
        {
             y = 0;
        }
        else if ( y + h > sn.GetScreenHeight() )
        {
            y = sn.GetScreenHeight() - h - nSpace - (sn.GetScreenHeight() - y );
			ApCurCalcForm->FShowOnTop = false;
        }
    }
    else
    {
        int x1 = ApParent->Position()->GetLeft();
        int y1 = ApParent->Position()->GetHeight();
        int w1 = ApParent->Position()->GetWidth();
        int h1 = ApParent->Position()->GetHeight();

		if ( x < 0 )
		{
			x = 0;
		}
		else if ( x + w > sn.GetScreenWidth() || !ApParent->FShowOnRight )
		{
			x -= (w1 + w);
			ApCurCalcForm->FShowOnRight = false;
		}
		if ( y < 0 )
		{
			y = 0;
		}
		else if ( y + h > sn.GetScreenHeight() )
		{
			y -=  ( h - ApRect->Height() + nSpace );
			ApCurCalcForm->FShowOnTop = false;
		}
        x += ApCurCalcForm->FShowOnRight ? GetFormSpace() : -GetFormSpace();
    }
	ApCurCalcForm->Position()->SetLeft( x );
	ApCurCalcForm->Position()->SetTop( y );
    ApCurCalcForm->Position()->SetWidth( w );
    ApCurCalcForm->Position()->SetHeight( h );
	ApCurCalcForm->Position()->NotifyPositionChanged();
}

void CxdPopupMenu::AddPopupItem( const TPopupMenuItem *ApItem, TPopupMenuItem *ApParent /*= NULL*/ )
{
    TPopupMenuItem *pItem = new TPopupMenuItem;
    *pItem = *ApItem;
    pItem->FpSubList = NULL;
    if ( Assigned(ApParent) )
    {
        if ( !Assigned(ApParent->FpSubList) )
        {
            ApParent->FpSubList = new TxdPopupItemList;
        }
        ApParent->FpSubList->push_back( pItem );
    }
    else
    {
        m_ltPopupItems.push_back( pItem );
    }
}

TPopupMenuItem* CxdPopupMenu::FindPopupItem( const INT64 &ATag )
{
    return FindPopupItem( ATag, &m_ltPopupItems );
}

TPopupMenuItem* CxdPopupMenu::FindPopupItem( const INT64 &ATag, TxdPopupItemList *ApList )
{
    for ( TxdPopupItemListIT it = ApList->begin(); it != ApList->end(); it++ )
    {
        TPopupMenuItem *pItem = *it;
        if ( pItem->FTag == ATag )
        {
            return pItem;
        }
        else if ( Assigned(pItem->FpSubList) )
        {
            TPopupMenuItem *pFind = FindPopupItem( ATag, pItem->FpSubList );
            if ( Assigned(pFind) )
            {
                return pFind;
            }
        }
    }
    return NULL;
}

void CxdPopupMenu::InitPopupItems( TPopupMenuItemInfos *ApItems, const int &ACount )
{
    for (int i = 0; i < ACount; i++)
    {
    	TPopupMenuItem *pParent = ApItems[i].FParentTag >= 0 ? FindPopupItem( ApItems[i].FParentTag ) : NULL;
        TPopupMenuItem item;
        item.FCaption = ApItems[i].FCaption;
        item.FImgIndex = ApItems[i].FImgIndex;
        item.FIsCheck = ApItems[i].FIsCheck;
        item.FIsEnable = ApItems[i].FIsEnable;
        item.FIsVisible = ApItems[i].FIsVisible;
        item.FpSubList = NULL;
        item.FTag = ApItems[i].FTag;
        AddPopupItem( &item, pParent );
    }
}

void CxdPopupMenu::Clear( void )
{
    ClearPopupItemList( &m_ltPopupItems, false );
}