#include "stdafx.h"
#include "JxdBar.h"
#include "JxdGdiPlusBasic.h"

CxdBar::CxdBar(const int &AMinSliderSize): CxdGraphicsControl(), CtAutoChangedPosTimerID( GetUnionID() ), CtMinSliderSize(AMinSliderSize)
{
    SetBufferBitmapTimeout( 0 );
    m_bsStyle = bsHor;
    m_bShowCtrlButton = true;
    m_nMin = 0;
    m_nMax = 100;
    m_nPos = 60;
    m_nSliderSize = 0;
    m_nSliderPos = 0;
    m_nTimerChangedCount = 1;
    m_mpMousePos = mpNone;
    m_bHandleBkState = false;
    m_nSettingSliderSize = 0;
    m_dSettingPosPixel = 1.5f;
    SetCaptureOnMouseDown( true );

    InitInterface( BarPosChanged );
}

CxdBar::~CxdBar()
{
    DeleteInterface( BarPosChanged );
}

void CxdBar::OnCreated( void )
{
    CalcBarInfo();
}

void CxdBar::DoDrawUI( Graphics &G, const Rect &ADestR )
{
    if ( Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() )
    {
        ImageAttributes *pImgAtt = GetDrawBKImageAttributes();
        m_bsStyle == bsVer ? DrawVerBar( G, pImgAtt ) : DrawHorBar( G, pImgAtt );
        SafeDeleteObject( pImgAtt );
    } 
}

void CxdBar::CalcBarInfo( void )
{
    if ( !Assigned(m_pImgInfo) || !m_pImgInfo->IsExsitsBitmap() )
    {
        m_nImgBlockWidth = 0;
        m_nImgBlockHeight = 0;
        return;
    }
    CheckImageBar();
    int nMaxSliderSpaceSize(0);
    if ( bsVer == m_bsStyle )
    {
        //´¹Ö±
        m_nImgBlockWidth = m_pImgInfo->GetBitmap()->GetWidth();
        m_nImgBlockHeight = m_pImgInfo->GetBitmap()->GetHeight() / m_pImgInfo->GetBitmapItemCount() / 4;
        nMaxSliderSpaceSize = Position()->GetHeight() - (m_bShowCtrlButton ? m_nImgBlockHeight * 2 : 0);
    }
    else
    {
        //Ë®Æ½
        m_nImgBlockWidth = m_pImgInfo->GetBitmap()->GetWidth() / m_pImgInfo->GetBitmapItemCount() / 4;
        m_nImgBlockHeight = m_pImgInfo->GetBitmap()->GetHeight();
        nMaxSliderSpaceSize = Position()->GetWidth() - (m_bShowCtrlButton ? m_nImgBlockWidth * 2 : 0);
    }
    if ( m_nSettingSliderSize <= 0 )
    {
        int nTemp = nMaxSliderSpaceSize - int(GetMaxScrllPos() * m_dSettingPosPixel);
        if ( nTemp >= CtMinSliderSize )
        {
            m_nSliderSize = nTemp;
        }
        else
        {
            m_nSliderSize = CtMinSliderSize;
        }
    }
    else
    {
        m_nSliderSize = m_nSettingSliderSize;
    }
    m_dPerPosPixel = (nMaxSliderSpaceSize - m_nSliderSize) / (double)GetMaxScrllPos();
    CalcSliderInfo();
}

void CxdBar::CalcSliderInfo( void )
{
    int nPos(-1);
    if ( m_nPos < m_nMin )
    {
        m_nPos = m_nMin;
        nPos = m_nPos;
    }
    else if ( m_nPos > m_nMax )
    {
        m_nPos = m_nMax - 1;
        nPos = m_nPos;
    }
    if ( m_nPos == m_nMin )
    {
        m_nSliderPos = m_bShowCtrlButton ? (bsVer == m_bsStyle ? m_nImgBlockHeight : m_nImgBlockWidth) : 0;
    }
    else if ( m_nPos == m_nMax - 1 )
    {
        m_nSliderPos = bsVer == m_bsStyle ? (Position()->GetHeight() - m_nSliderSize - (m_bShowCtrlButton ? m_nImgBlockHeight : 0)) :
            (Position()->GetWidth() - m_nSliderSize - (m_bShowCtrlButton ? m_nImgBlockWidth : 0));
    }
    else
    {
        m_nSliderPos = int( m_dPerPosPixel * m_nPos ) + (m_bShowCtrlButton ? (bsVer == m_bsStyle ? m_nImgBlockHeight : m_nImgBlockWidth) : 0);
    }
    if ( nPos != -1 )
    {
        NotifyInterfaceEx3( BarPosChanged, this, true, 0 );
    }
}

void CxdBar::CheckImageBar( void )
{
    if ( bsVer == m_bsStyle )
    {
        if ( m_pImgInfo->GetBitmap()->GetWidth() > m_pImgInfo->GetBitmap()->GetHeight() )
        {
            m_pImgInfo->GetBitmap()->RotateFlip( Rotate90FlipNone );
        }
    }
    else
    {
        if ( m_pImgInfo->GetBitmap()->GetWidth() < m_pImgInfo->GetBitmap()->GetHeight() )
        {
            m_pImgInfo->GetBitmap()->RotateFlip( Rotate270FlipNone );
        }
    }
}

void CxdBar::DrawVerBar( Graphics &G, ImageAttributes *ApImgAtt )
{
    //±³¾°
    Rect destR(0, 0, Position()->GetWidth(), Position()->GetHeight() );
    Rect rSrcBK(0, m_nImgBlockHeight, m_nImgBlockWidth, m_nImgBlockHeight - 1);

    Rect rUpDest( 0, 0, m_nImgBlockWidth, m_nImgBlockHeight );
    Rect rUpButton( rUpDest );

    Rect rDownDest( 0, Position()->GetHeight() - m_nImgBlockHeight, m_nImgBlockWidth, m_nImgBlockHeight );
    Rect rDownButton( 0, m_nImgBlockHeight * 3, m_nImgBlockWidth, m_nImgBlockHeight );

    Rect rSliderDest( GetSliderRect() );
    Rect rSliderButton( 0, m_nImgBlockHeight * 2, m_nImgBlockWidth, m_nImgBlockHeight - 1 );

    int nItemSize = m_pImgInfo->GetBitmap()->GetHeight() / m_pImgInfo->GetBitmapItemCount();
    int nSize = 0;
    switch ( GetCurUISate() )
    {
    case uiActive:
        if ( m_pImgInfo->GetBitmapItemCount() > 1 )
        {
            nSize = nItemSize;
        }
    	break;
    case uiDown:
        {
            if ( m_pImgInfo->GetBitmapItemCount() > 2 )
            {
                nSize = nItemSize * 2;
            }
            else if ( m_pImgInfo->GetBitmapItemCount() > 1 )
            {
                nSize = nItemSize;
            }
        }
        break;
    }

    switch ( m_mpMousePos )
    {
    case mpBK:
        if (m_bHandleBkState) rSrcBK.Y += nSize;
        break;
    case mpFirstButton:
        rUpButton.Y += nSize;
        break;
    case mpLastButton:
        rDownButton.Y += nSize;
        break;
    case mpSlider:
        rSliderButton.Y += nSize;
        break;
    }          

    CRect uiR = GetRealRectOnUI();
    Rect R;

    DrawCommonImage( G, destR, m_pImgInfo->GetBitmap(), rSrcBK, m_pImgInfo->GetBitmapDrawStyle(),
        m_pImgInfo->GetStretchX(), m_pImgInfo->GetStretchY(), ApImgAtt );

    if ( m_bShowCtrlButton )
    {
        //UpButton
        R.X = uiR.left + rUpDest.X;
        R.Y = uiR.top + rUpDest.Y;
        R.Width = rUpDest.Width;
        R.Height = rUpDest.Height;
        if ( IsNeedDraw(R) )
        {
            DrawCommonImage( G, rUpDest, m_pImgInfo->GetBitmap(), rUpButton, dsPaste, 
                m_pImgInfo->GetStretchX(), m_pImgInfo->GetStretchY(), ApImgAtt );
        }

        //DownButton
        R.X = uiR.left + rDownDest.X;
        R.Y = uiR.top + rDownDest.Y;
        R.Width = rDownDest.Width;
        R.Height = rDownDest.Height;
        if ( IsNeedDraw(R) )
        {
            DrawCommonImage( G, rDownDest, m_pImgInfo->GetBitmap(), rDownButton, dsPaste, 
                m_pImgInfo->GetStretchX(), m_pImgInfo->GetStretchY(), ApImgAtt );
        }
    }

    //Slider
    R.X = uiR.left + rSliderDest.X;
    R.Y = uiR.top + rSliderDest.Y;
    R.Width = rSliderDest.Width;
    R.Height = rSliderDest.Height;
    if ( IsNeedDraw(R) )
    {
        DrawCommonImage( G, rSliderDest, m_pImgInfo->GetBitmap(), rSliderButton, m_pImgInfo->GetBitmapDrawStyle(),
            m_pImgInfo->GetStretchX(), m_pImgInfo->GetStretchY(), ApImgAtt );
    }
    
}

void CxdBar::DrawHorBar( Graphics &G, ImageAttributes *ApImgAtt )
{
    //±³¾°
    Rect destR(0, 0, Position()->GetWidth(), Position()->GetHeight() );
    Rect rSrcBK(m_nImgBlockWidth, 0, m_nImgBlockWidth, m_nImgBlockHeight);

    Rect rLeftDest( 0, 0, m_nImgBlockWidth, m_nImgBlockHeight );
    Rect rLeftButton( rLeftDest );
    rLeftButton.Width += 1;
    rLeftButton.Height += 1;

    Rect rRightDest( Position()->GetWidth() - m_nImgBlockWidth, 0, m_nImgBlockWidth, m_nImgBlockHeight );
    Rect rRightButton( m_nImgBlockWidth * 3, 0, m_nImgBlockWidth + 1, m_nImgBlockHeight + 1 );

    Rect rSliderDest( GetSliderRect() );
    Rect rSliderButton( m_nImgBlockWidth * 2, 0, m_nImgBlockWidth, m_nImgBlockHeight );

    int nItemSize = m_pImgInfo->GetBitmap()->GetWidth() / m_pImgInfo->GetBitmapItemCount();
    int nSize = 0;
    switch ( GetCurUISate() )
    {
    case uiActive:
        if ( m_pImgInfo->GetBitmapItemCount() > 1 )
        {
            nSize = nItemSize;
        }
        break;
    case uiDown:
        {
            if ( m_pImgInfo->GetBitmapItemCount() > 2 )
            {
                nSize = nItemSize * 2;
            }
            else if ( m_pImgInfo->GetBitmapItemCount() > 1 )
            {
                nSize = nItemSize;
            }
        }
        break;
    }

    switch ( m_mpMousePos )
    {
    case mpBK:
        if (m_bHandleBkState) rSrcBK.X += nSize;
        break;
    case mpFirstButton:
        rLeftButton.X += nSize;
        break;
    case mpLastButton:
        rRightButton.X += nSize;
        break;
    case mpSlider:
        rSliderButton.X += nSize;
        break;
    }          

    CRect uiR = GetRealRectOnUI();
    Rect R;

    //Draw BK
    DrawCommonImage( G, destR, m_pImgInfo->GetBitmap(), rSrcBK, m_pImgInfo->GetBitmapDrawStyle(), 
        m_pImgInfo->GetStretchX(), m_pImgInfo->GetStretchY(), ApImgAtt );

    if ( m_bShowCtrlButton )
    {
        //m_pImgInfo->GetBitmap()->Save( L"a.png", &GetEncoderClsid(ImgFormatPng) );
        //LeftButton
        R.X = uiR.left + rLeftDest.X;
        R.Y = uiR.top + rLeftDest.Y;
        R.Width = rLeftDest.Width;
        R.Height = rLeftDest.Height;
        if ( IsNeedDraw(R) )
        {
            DrawCommonImage( G, rLeftDest, m_pImgInfo->GetBitmap(), rLeftButton, dsStretchAll, 
                m_pImgInfo->GetStretchX(), m_pImgInfo->GetStretchY(), ApImgAtt );
        }
        

        //DownButton
        R.X = uiR.left + rRightDest.X;
        R.Y = uiR.top + rRightDest.Y;
        R.Width = rRightDest.Width;
        R.Height = rRightDest.Height;
        if ( IsNeedDraw(R) )
        {
            DrawCommonImage( G, rRightDest, m_pImgInfo->GetBitmap(), rRightButton, dsStretchAll, 
                m_pImgInfo->GetStretchX(), m_pImgInfo->GetStretchY(), ApImgAtt );
        }
    }

    //Slider
    R.X = uiR.left + rSliderDest.X;
    R.Y = uiR.top + rSliderDest.Y;
    R.Width = rSliderDest.Width;
    R.Height = rSliderDest.Height;
    if ( IsNeedDraw(R) )
    {
        DrawCommonImage( G, rSliderDest, m_pImgInfo->GetBitmap(), rSliderButton, m_pImgInfo->GetBitmapDrawStyle(), 
            m_pImgInfo->GetStretchX(), m_pImgInfo->GetStretchY(), ApImgAtt );
    }
}

bool CxdBar::SetBarPos( const int &APos )
{
    return DoChangedBarPos( APos, false );
}

void CxdBar::OnMouseMoveOnWidget( UINT AFlags, int x, int y )
{
    CxdGraphicsControl::OnMouseMoveOnWidget( AFlags, x, y );

    if ( GetCurUISate() != uiDown )
    {
        CalcMousePos(x, y);
    }    
    else if ( m_mpMousePos == mpSlider )
    {
        int nSize = bsVer == m_bsStyle ? y - m_ptDown.y : x - m_ptDown.x;
        int nAddPos = int( nSize / m_dPerPosPixel);
        if ( nAddPos != 0 )
        {
            m_ptDown.x = x;
            m_ptDown.y = y;
            DoChangedBarPos( m_nPos + nAddPos, true );
        }
    }
}

void CxdBar::InvalieByMP( TxdBarMousePos Amp )
{
    switch ( Amp )
    {
    case mpFirstButton:
        {
            Rect R(0, 0, m_nImgBlockWidth, m_nImgBlockHeight);
            InvalidateGpRect( R );
        }
        break;
    case mpLastButton:
        {
            Rect R( 0, 0, m_nImgBlockWidth, m_nImgBlockHeight );
            m_bsStyle == bsVer ?  R.Y = Position()->GetHeight() - m_nImgBlockHeight : 
                R.X = Position()->GetWidth() - m_nImgBlockWidth;
            InvalidateGpRect( R );
        }
        break;
    case mpSlider:
        {
            Rect R;
            m_bsStyle == bsVer ? R = Rect( 0, m_nSliderPos, m_nImgBlockWidth, m_nSliderSize ) :
                R = Rect( m_nSliderPos, 0, m_nSliderSize,  m_nImgBlockHeight);
            InvalidateGpRect( R );
            //Invalidate();
        }
        break;
    }    
}

void CxdBar::ChangedNewMousePos( TxdBarMousePos Amp )
{
    if ( Amp != m_mpMousePos  )
    {
        if ( Amp == mpBK && m_bHandleBkState )
        {
            m_mpMousePos = Amp;
            Invalidate();
            return;
        }
        TxdBarMousePos oldMP = m_mpMousePos;
        m_mpMousePos = Amp;
        if ( oldMP == mpBK && m_bHandleBkState )
        {
            Invalidate();
        }
        else
        {
            InvalieByMP( m_mpMousePos );
            InvalieByMP( oldMP );
        }        
    }
}

void CxdBar::OnMouseLeaveWidget()
{
    CxdGraphicsControl::OnMouseLeaveWidget();
    ChangedNewMousePos( mpNone );
}

void CxdBar::OnClick( const int &x /* = 0 */, const int &y /* = 0 */ )
{
    switch ( m_mpMousePos )
    {
    case mpFirstButton:
        DoChangedBarPos( m_nPos - 1, true);
    	break;
    case mpLastButton:
        DoChangedBarPos( m_nPos + 1, true );
        break;
    }
}

void CxdBar::OnLButtonDownWidget( UINT AFlags, int x, int y )
{
    CxdGraphicsControl::OnLButtonDownWidget(AFlags, x, y);
    switch ( m_mpMousePos )
    {
    case mpSlider:
        m_ptDown.x = x;
        m_ptDown.y = y;
        InvalieByMP( mpSlider );
    	break;
    case mpFirstButton:
    case mpLastButton:
        StartTimer( CtAutoChangedPosTimerID, 100 );
        InvalieByMP( m_mpMousePos );
        break;
    }
}

void CxdBar::CalcMousePos( const int &x, const int &y )
{
    if ( bsVer == m_bsStyle )
    {
        if ( y >= m_nSliderPos && y < m_nSliderPos + m_nSliderSize )
        {
            ChangedNewMousePos( mpSlider );
        }
        else if ( m_bShowCtrlButton )
        {
            y < m_nImgBlockHeight ? 
                ChangedNewMousePos( mpFirstButton ) :  (y > Position()->GetHeight() - m_nImgBlockHeight) ? 
                ChangedNewMousePos( mpLastButton ) : ChangedNewMousePos( mpBK );
        }
        else
        {
            ChangedNewMousePos( mpBK );
        }
    }
    else
    {
        if ( x >= m_nSliderPos && x < m_nSliderPos + m_nSliderSize )
        {
            ChangedNewMousePos( mpSlider );
        }
        else if ( m_bShowCtrlButton )
        {
            x < m_nImgBlockWidth ? 
                ChangedNewMousePos( mpFirstButton ) :  (x > Position()->GetWidth() - m_nImgBlockWidth) ? 
                ChangedNewMousePos( mpLastButton ) : ChangedNewMousePos( mpBK );
        }
        else
        {
            ChangedNewMousePos( mpBK );
        }
    }
}

bool CxdBar::OnTimerMessage( int ATimeID, LPVOID ApData )
{
    if ( CtAutoChangedPosTimerID == ATimeID )
    {
        int n = m_mpMousePos == mpFirstButton ? -m_nTimerChangedCount : ( m_mpMousePos == mpLastButton ? m_nTimerChangedCount : 0 );
        if ( n != 0 )
        {
            int nValue( m_nPos + n );
            if ( nValue >= m_nMax )
            {
                nValue = m_nMax - 1;
            }
            else if ( nValue < m_nMin )
            {
                nValue = m_nMin;
            }
            if ( DoChangedBarPos(nValue, true) )
            {
                return true;
            }
        }
    }
    return false;
}

void CxdBar::OnLButtonUpWidget( UINT AFlags, int x, int y )
{
    CxdGraphicsControl::OnLButtonUpWidget(AFlags, x, y);
    switch ( m_mpMousePos )
    {
    case mpSlider:
        InvalieByMP( mpSlider );
        break;
    case mpFirstButton:
    case mpLastButton:
        EndTimer( CtAutoChangedPosTimerID );
        InvalieByMP( m_mpMousePos );
        break;
    }
    
}

void CxdBar::DoPositionChanged( const LPVOID &ASender )
{
    CalcBarInfo();
    Invalidate();
}

bool CxdBar::OnUIStateChanged( const TxdGpUIState &ANewSate )
{
    return CxdUIComponent::OnUIStateChanged(ANewSate);
}

Rect CxdBar::GetSliderRect( void )
{
    return bsVer == m_bsStyle ?
        Rect( 0, m_nSliderPos, Position()->GetWidth(), m_nSliderSize ):
        Rect( m_nSliderPos, 0, m_nSliderSize, Position()->GetHeight() );
}

void CxdBar::OnParentPositionChanged( CxdPosition* ApPosition )
{
    CxdGraphicsControl::OnParentPositionChanged( ApPosition );
    CalcBarInfo();
}

void CxdBar::SetMin( const int &AMin )
{
    if ( AMin < m_nMax )
    {
        m_nMin = AMin;
    }
}

void CxdBar::SetMax( const int &AMax )
{
    if ( AMax > m_nMin )
    {
        m_nMax = AMax;
        if ( m_nMax <= 1 )
        {
            m_nMax = 2;
        }
        if ( m_nPos >= m_nMax )
        {
            m_nPos = m_nMax - 1;
        }
    }
}

bool CxdBar::DoChangedBarPos( const int &ANewPos, bool AbByMouse )
{
    if ( ANewPos != m_nPos && ANewPos >= m_nMin && ANewPos < m_nMax )
    {
        int nOldPos = m_nSliderPos;
        int nOldBarPos = m_nPos;
        m_nPos = ANewPos;
        CalcSliderInfo();

        if ( bsVer == m_bsStyle )
        {
            CRect R( 0, nOldPos < m_nSliderPos ? nOldPos : m_nSliderPos, Position()->GetWidth(), 0 );
            R.bottom = (nOldPos < m_nSliderPos ? m_nSliderPos : nOldPos) + m_nSliderSize;
            InvalidateRect( &R );
        }
        else
        {
            //Invalidate();
            CRect R( nOldPos < m_nSliderPos ? nOldPos : m_nSliderPos, 0, 0, Position()->GetHeight() );
            R.right = (nOldPos < m_nSliderPos ? m_nSliderPos + 1 : nOldPos) + m_nSliderSize + 1;
            InvalidateRect( &R );
        }
        NotifyInterfaceEx3( BarPosChanged, this, AbByMouse, nOldBarPos );
        return true;
    }
    return false;
}

void CxdBar::OnMidMouseWheel( UINT nFlags, short zDelta, int x, int y )
{
    SetBarPos( GetBarPos() + (zDelta < 0 ? 1 : -1) );
}
