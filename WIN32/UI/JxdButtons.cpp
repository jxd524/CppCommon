#include "stdafx.h"
#include "JxdButtons.h"
#include "JxdGdiPlusBasic.h"

//CxdCheckBox

TxdCheckButtonList CxdCheckBox::m_ltGroupButtonList;

CxdCheckBox::CxdCheckBox(): CxdLabelEx()
{
    m_pImgCheck = NULL;
    m_pImgUnCheck = NULL;
    m_ImageOffsetX = 0;
    m_ImageOffsetY = 5;
    m_nGroupIndex = -1;
    m_bRadioButton = false;
    m_bCheck = false;
    m_GroupID = 0;
    m_AutoChangedCheckState = true;
}

CxdCheckBox::~CxdCheckBox()
{
    SafeDeleteObject( m_pImgCheck );
    SafeDeleteObject( m_pImgUnCheck );
    DeleteFromGroup();
}

void CxdCheckBox::DoDrawSubjoinBK( Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt )
{
    CxdImageInfo *pImg = m_bCheck ? m_pImgCheck : m_pImgUnCheck;
    Rect R;
    if ( GetImgRectOnUI(pImg, R) )
    {
        R.X += ADestRect.X;
        R.Y += ADestRect.Y;
        DrawCommonImage( ADestG, R, pImg, GetCurUISate(), ApImgAtt );
    }
}

bool CxdCheckBox::GetImgRectOnUI( CxdImageInfo *ApImage, Rect &ARect )
{
    if ( Assigned(ApImage) && ApImage->IsExsitsBitmap() )
    {
        ARect.X = GetImageOffsetX();
        ARect.Y = GetImageOffsetY(); 
        ARect.Width = ApImage->GetBitmap()->GetWidth();
        ARect.Height = ApImage->GetBitmap()->GetHeight() / ApImage->GetBitmapItemCount();
        return true;
    }
    return false;
}

void CxdCheckBox::OnClick( const int &x /* = 0 */, const int &y /* = 0 */ )
{
    if ( !GetAutoChangedCheckState() || (m_bRadioButton && m_bCheck) )
    {
        CxdGraphicsControl::OnClick(x, y);
        return;
    }
    SetCheck( !m_bCheck );
    CxdGraphicsControl::OnClick(x, y);
}

void CxdCheckBox::AddToGroup( const int &AGroupIndex )
{
    if ( AGroupIndex < 0)
    {
        return;
    }

    bool bAdd = true;
    for ( TxdCheckButtonListIT it = m_ltGroupButtonList.begin(); it != m_ltGroupButtonList.end(); it++ )
    {
        if ( (DWORD_PTR)*it == (DWORD_PTR)this )
        {
            bAdd = false;
            break;
        }
    }
    if ( bAdd )
    {
        m_nGroupIndex = AGroupIndex; 
        m_ltGroupButtonList.push_back( this );
    }
}

void CxdCheckBox::DeleteFromGroup( void )
{
    for ( TxdCheckButtonListIT it = m_ltGroupButtonList.begin(); it != m_ltGroupButtonList.end(); it++ )
    {
        if ( (DWORD_PTR)*it == (DWORD_PTR)this )
        {
            m_ltGroupButtonList.erase( it );
            break;
        }
    }
}

void CxdCheckBox::SetCheck( bool bCheck, bool bReDraw )
{
    if ( m_bCheck != bCheck )
    {
        m_bCheck = bCheck;
        if ( GetGroupIndex() >= 0 )
        {
            bool bOtherCheckState( !bCheck );
            for ( TxdCheckButtonListIT it = m_ltGroupButtonList.begin(); it != m_ltGroupButtonList.end(); it++ )
            {
                CxdCheckBox *pObj = *it;
                if ( (DWORD_PTR)pObj != (DWORD_PTR)this && pObj->GetGroupIndex() == GetGroupIndex() && 
                     (0 == GetGroupID() ? ((DWORD_PTR)pObj->m_pParent == (DWORD_PTR)m_pParent) : (pObj->GetGroupID() == GetGroupID()) ))
                {
                    if ( pObj->m_bCheck != bOtherCheckState )
                    {
                        pObj->m_bCheck = bOtherCheckState;
                        pObj->ClearBufferBitmap();
                        pObj->Invalidate();
                    }
                }
            }
        }
        if ( bReDraw )
        {
            ClearBufferBitmap();
            Invalidate();
        }
    }
}

void CxdCheckBox::SetRadioButton( bool bRadioButton, bool bReDraw )
{
    if ( m_bRadioButton != bRadioButton )
    {
        m_bRadioButton = bRadioButton;
        if ( bReDraw )
        {
            ClearBufferBitmap();
            Invalidate();
        }
    }
}

void CxdCheckBox::SetCheckImageInfo(CxdImageInfo *ApNewImg, bool AbResOnly /* = true */)
{
	UpdatePointer( m_pImgCheck, ApNewImg );
	if ( !AbResOnly )
	{
		if ( Assigned(m_pImgCheck) )
		{
			m_pImgCheck->ForceDelBmpOnFree();
		}
	}
}

void CxdCheckBox::SetUnCheckImageInfo(CxdImageInfo *ApNewImg, bool AbResOnly /* = true */)
{
	UpdatePointer( m_pImgUnCheck, ApNewImg );
	if ( !AbResOnly )
	{
		if ( Assigned(m_pImgUnCheck) )
		{
			m_pImgUnCheck->ForceDelBmpOnFree();
		}
	}
}

bool CxdCheckBox::OnUIStateChanged( const TxdGpUIState &ANewSate )
{
    bool bNeedReDraw = CxdUIComponent::OnUIStateChanged(ANewSate);
    if ( bNeedReDraw )
    {
        if ( IsDrawCaption() || (Assigned(m_pImgInfo) && m_pImgInfo->GetBitmapItemCount() > 1) )
        {
            InvalidateRect( NULL );
        }
        else if ( (Assigned(m_pImgCheck) && m_pImgCheck->IsExsitsBitmap() && m_pImgCheck->GetBitmapItemCount() > 1) ||
                   (Assigned(m_pImgUnCheck) && m_pImgUnCheck->IsExsitsBitmap() && m_pImgUnCheck->GetBitmapItemCount() > 1) )
        {
            ClearBufferBitmap();
            InvalidateRect( NULL );
        }
    }
    return bNeedReDraw;
}


//CxdButton
TxdButtonList CxdButton::G_ltGroupButtonList;

CxdButton::CxdButton(): CxdLabelEx()
{
    m_bPushDown = false;
    m_nGroupIndex = -1;
    m_GroupID = 0;
}

CxdButton::~CxdButton()
{
    DeleteFromGroup();
}

void CxdButton::SetPushDown( bool bPushDown, bool bReDraw )
{
    if ( m_bPushDown != bPushDown )
    {
        m_bPushDown = bPushDown;
        if ( m_nGroupIndex >= 0 )
        {
            bool bOtherCheckState( !bPushDown );
            for ( TxdButtonListIT it = G_ltGroupButtonList.begin(); it != G_ltGroupButtonList.end(); it++ )
            {
                CxdButton *pObj = *it;
                if ( (DWORD_PTR)pObj != (DWORD_PTR)this && pObj->GetGroupIndex() == GetGroupIndex() && 
                     (0 == GetGroupID() ? ((DWORD_PTR)pObj->m_pParent == (DWORD_PTR)m_pParent) : (pObj->GetGroupID() == GetGroupID()) ))
                {
                    if ( pObj->m_bPushDown != bOtherCheckState )
                    {
                        pObj->m_bPushDown = bOtherCheckState;
                        pObj->ClearBufferBitmap();
                        pObj->Invalidate();
                    }
                }
            }
        }
        if ( bReDraw )
        {
            ClearBufferBitmap();
            Invalidate();
        }
    }
}

TxdGpUIState CxdButton::GetCurUISate( void )
{
    if ( m_bPushDown )
    {
        return uiDown;
    }
    return CxdUIComponent::GetCurUISate();
}

void CxdButton::AddToGroup( const int &AGroupIndex )
{
    if ( AGroupIndex < 0)
    {
        return;
    }

    bool bAdd = true;
    for ( TxdButtonListIT it = G_ltGroupButtonList.begin(); it != G_ltGroupButtonList.end(); it++ )
    {
        if ( (DWORD_PTR)*it == (DWORD_PTR)this )
        {
            bAdd = false;
            break;
        }
    }
    if ( bAdd )
    {
        m_nGroupIndex = AGroupIndex; 
        G_ltGroupButtonList.push_back( this );
    }
}

void CxdButton::DeleteFromGroup( void )
{
    for ( TxdButtonListIT it = G_ltGroupButtonList.begin(); it != G_ltGroupButtonList.end(); it++ )
    {
        if ( (DWORD_PTR)*it == (DWORD_PTR)this )
        {
            G_ltGroupButtonList.erase( it );
            break;
        }
    }
}
