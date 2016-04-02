#include "stdafx.h"
#include "JxdComboBox.h"
#include "JxdGraphicsControl.h"
#include "JxdPopupMenu.h"

CxdComboBox::CxdComboBox(): CxdEdit()
{
    m_pBtnDrawDown = new CxdGraphicsControl;
    m_pBtnDrawDown->SetItClick( NewInterfaceByClass(CxdComboBox, CxdUIComponent*, DoBtnDrawDownClick) );
    m_pBtnDrawDown->Cursor()->SetCursorStyle( crHand );
    m_pBtnDrawDown->Anchors()->AddAll();
    m_bOnRight = true;
    m_nCtrlSpace = 2;
    m_nCtrlWidth = 0;
	m_nMaxHeight = 0;
    InitInterface( ListSelected );
}

CxdComboBox::~CxdComboBox()
{
    SafeDeleteObject( m_pBtnDrawDown );
    DeleteInterface( ListSelected );
}

void CxdComboBox::DrawEditSubComponentBK( Graphics &G )
{
    CRect R( CalcEditRect() );
    Rect rBK;
    m_bOnRight ? rBK.X = R.right : rBK.X = R.left - m_nCtrlSpace;
    rBK.Y = R.top;
    rBK.Width = m_nCtrlSpace * 2 + m_nCtrlWidth + 2;
    rBK.Height = R.Height();
    G.FillRectangle( &SolidBrush(m_dwSpaceColor), rBK );
}

CRect CxdComboBox::CalcEditRect( void )
{
    CRect R( CxdEdit::CalcEditRect() );
    m_bOnRight ? R.right -= m_nCtrlWidth + m_nCtrlSpace * 2 : R.left += m_nCtrlWidth + m_nCtrlSpace;
    return R;
}

void CxdComboBox::OnCreated()
{
    m_nCtrlWidth = m_pBtnDrawDown->ImageInfo()->GetBitmap()->GetWidth();
    CRect R( CalcEditRect() );
    m_bOnRight ? R.left = R.right + m_nCtrlSpace * 2 : R.left -= m_nCtrlWidth + m_nCtrlSpace;
    m_pBtnDrawDown->xdCreate( R.left, R.top, m_nCtrlWidth, R.Height(), this );
}

CxdImageInfo* CxdComboBox::ImageDrawDown( void )
{
    return m_pBtnDrawDown->ImageInfo();
}

void CxdComboBox::SetDrawDownPosition( bool AbOnRight )
{
    if ( m_bOnRight != AbOnRight )
    {
        m_bOnRight = AbOnRight;
        Position()->NotifyPositionChanged();
    }
}

void CxdComboBox::DoBtnDrawDownClick( CxdUIComponent* ApBtn )
{
    CRect R( Position()->GetCRect() );
    ClientToScreen( &R );
	
    m_oComboMenu.SetFixedWidth( R.Width() );
    m_oComboMenu.SetMinHeight( 10 );
    TPopupMenuItem *pItem = m_oComboMenu.PopuMenu( this, R.left - Position()->GetLeft(), R.top - Position()->GetTop() + R.Height() + 2 );
    if ( Assigned(pItem) )
    {
        if ( !IsExsitsItListSelected() || !m_pItListSelected->Notify(pItem) )
        { 
            SetCaption( pItem->FCaption );
            SetSel(0, pItem->FCaption.GetLength() );
        }
    }
}
