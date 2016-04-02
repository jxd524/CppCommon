#include "stdafx.h"
#include "JxdQuiltCtrl.h"
#include "JxdDebugInfo.h"


CxdQuiltCtrl::CxdQuiltCtrl()
{
    m_LeftRightSpace = 0;
    m_UpDownSpace = 0;
    m_CurMaxHeight = 0;
    m_PerPosPiexl = 10;
    m_pVerScrollBar = NULL;
    m_AnimateOnDelCtrl = false;

    InitInterface( CreateVerBar );
    InitInterface( BarMoveTopOrDown );
}

CxdQuiltCtrl::~CxdQuiltCtrl()
{
    for( TxdColInfoListIT it = m_ltCols.begin(); it != m_ltCols.end(); it++ )
    {
        TxdColInfo *pInfo = *it;
        delete pInfo;
    }
    m_ltCols.clear();
    DeleteInterface( CreateVerBar );
    DeleteInterface( BarMoveTopOrDown );
}

int CxdQuiltCtrl::AddCol( int AWidth )
{
    TxdColInfo *pInfo = new TxdColInfo;
    pInfo->AllHeight = 0;
    pInfo->Width = AWidth;
    m_ltCols.push_back( pInfo );
    return (int)m_ltCols.size() - 1;
}

bool CxdQuiltCtrl::AddCtrl( int AIndex, CxdGraphicsControl* ApCtrl, int ACtrlHeight )
{
    TxdColInfoListIT it = m_ltCols.begin() + AIndex;
    if ( it == m_ltCols.end() ) return false;

    TxdColInfo *pInfo = *it;
    int x, y;
    CalcXY( pInfo, x, y );
    if ( ACtrlHeight <= 0 ) ACtrlHeight = ApCtrl->Position()->GetHeight();    

    if ( ApCtrl->IsCreated() )
    {
        ApCtrl->ReSetParent( this );
        ApCtrl->Position()->SetLeft( x );
        ApCtrl->Position()->SetTop( y );
        ApCtrl->Position()->SetWidth( pInfo->Width );
        ApCtrl->Position()->SetHeight( ACtrlHeight );
        ApCtrl->Invalidate();
    }
    else
    {       
        ApCtrl->xdCreate( x, y, pInfo->Width, ACtrlHeight, this );
    }
    AddToColCtrl( pInfo, ApCtrl );
    return true;
}

bool CxdQuiltCtrl::AutoAddCtrl( CxdGraphicsControl* ApCtrl, int AWidth, int AHeight )
{
    int i = 0;
    int nIndex = -1;
    int nMinH = MAXINT;
    for ( TxdColInfoListIT it = m_ltCols.begin(); it != m_ltCols.end(); it++ )
    {
        TxdColInfo *pInfo = *it;
        if ( pInfo->Width == AWidth )
        {
            if ( nMinH > pInfo->AllHeight )
            {
                nMinH = pInfo->AllHeight;
                nIndex = i;
            }
        }
        i++;
    }
    if ( nIndex == -1 )
    {
        nIndex = AddCol( AWidth );
    }
    return AddCtrl( nIndex, ApCtrl, AHeight );
}

void CxdQuiltCtrl::CalcXY( TxdColInfo *ApCol, int &x, int &y )
{
    if ( !ApCol->CtrlList.empty() )
    {
        CxdGraphicsControl *pCtrl = ApCol->CtrlList.back();
        x = pCtrl->Position()->GetLeft();
        y = pCtrl->Position()->GetBottom() + m_UpDownSpace;
    }
    else
    {
        x = m_LeftRightSpace;
        y = m_UpDownSpace;
        for ( TxdColInfoListIT it = m_ltCols.begin(); it != m_ltCols.end(); it++ )
        {
            TxdColInfo *pInfo = *it;
            if ( (INT64)pInfo == (INT64)ApCol ) break;
            
            x += pInfo->Width + m_LeftRightSpace;
        }
    }    
}

void CxdQuiltCtrl::AddToColCtrl( TxdColInfo *ApInfo, CxdGraphicsControl *ApCtrl )
{
    ApInfo->CtrlList.push_back( ApCtrl );
    ApInfo->AllHeight += ApCtrl->Position()->GetHeight() + m_UpDownSpace;
    if ( ApInfo->AllHeight <= m_CurMaxHeight ) return;

    m_CurMaxHeight = ApInfo->AllHeight;
    CheckVerScrollBar();
}

CxdBar* CxdQuiltCtrl::OnCreateVerScrollBar()
{
    CxdBar *pBar = NULL;
    NotifyInterface( CreateVerBar, &pBar );
    //pBar = new CxdBar;
    //pBar->ImageInfo()->SetBitmapFileName( TEXT("E:\\work\\ScrollBar.png") );
    //pBar->ImageInfo()->SetBitmapDrawStyle( dsStretchByVH );
    //pBar->ImageInfo()->SetBitmapItemCount( 3 );
    //pBar->SetBarStyle( bsVer );
    return pBar;
}

void CxdQuiltCtrl::CheckVerScrollBar( void )
{
    int nUnVisibleH = m_CurMaxHeight - Position()->GetHeight();
    if ( nUnVisibleH <= 0 )
    {
        //取消滚动条
        if ( Assigned(m_pVerScrollBar) && m_pVerScrollBar->GetVisible() )
        {
            m_pVerScrollBar->SetVisible( false );
            SetCtrlsTop( m_UpDownSpace );
        }
    }
    else
    {
        if ( IsNull(m_pVerScrollBar) )
        {
            m_pVerScrollBar = OnCreateVerScrollBar();
            if( IsNull(m_pVerScrollBar) ) return;

            m_pVerScrollBar->SetBarStyle( bsVer );
            m_pVerScrollBar->SetBarPos( 0 );
            m_pVerScrollBar->SetMin( 0 );
            m_pVerScrollBar->SetItBarPosChanged( NewInterfaceEx3ByClass(CxdQuiltCtrl, CxdBar*, bool, int, DoVerBarScroll) );
            m_pVerScrollBar->CalcBarInfo();
            int nW = m_pVerScrollBar->ImageInfo()->GetBitmap()->GetWidth();
            m_pVerScrollBar->xdCreate( Position()->GetWidth() - nW, 0, nW, Position()->GetHeight(), this );            
            m_pVerScrollBar->SetVisible( true );
        }

        m_pVerScrollBar->SetMax( ( nUnVisibleH + m_PerPosPiexl - 1) / m_PerPosPiexl );
        m_pVerScrollBar->CalcBarInfo();
        m_pVerScrollBar->Invalidate();
    }
}

void CxdQuiltCtrl::SetCtrlsTop( int ATop )
{
    for ( TxdColInfoListIT it = m_ltCols.begin(); it != m_ltCols.end(); it++ )
    {
        TxdColInfo *pInfo = *it;
        int Y = ATop;
        for ( TxdGraphicsCtrlListIT itCtrl = pInfo->CtrlList.begin(); itCtrl != pInfo->CtrlList.end(); itCtrl++ )
        {
            CxdGraphicsControl* pCtrl = *itCtrl;
            pCtrl->Position()->SetTop( Y );
            Y += pCtrl->Position()->GetHeight() + m_UpDownSpace;
        }
    }
}

void CxdQuiltCtrl::DoVerBarScroll( CxdBar* pSender, bool bChangedByMouse, int nOldBarPos )
{
    ClearBufferBitmap();

    int nType = m_pVerScrollBar->GetBarPos() == 0 ? 1 : 
        ((m_pVerScrollBar->GetBarPos() == m_pVerScrollBar->GetMax() - 1) ? 2 : -1);
    int nY = -m_pVerScrollBar->GetBarPos() * m_PerPosPiexl;
    if ( m_pVerScrollBar->GetBarPos() == m_pVerScrollBar->GetMax() - 1 )
    {
        nY = -((m_pVerScrollBar->GetBarPos() + 1) * m_PerPosPiexl);
    }
    SetCtrlsTop( nY );

    if ( nType > 0 && Assigned(m_pItBarMoveTopOrDown) )
    {
        m_pItBarMoveTopOrDown->Notify( this, nType == 1 );
    }

    Invalidate();
}

void CxdQuiltCtrl::OnMidMouseWheel( UINT nFlags, short zDelta, int x, int y )
{
    if ( Assigned(m_pVerScrollBar) && m_pVerScrollBar->GetVisible() )
    {
        m_pVerScrollBar->SetBarPos( m_pVerScrollBar->GetBarPos() + (zDelta < 0 ? 1 : -1) );
    }
}

int CxdQuiltCtrl::CalcCurMaxColHeight( void )
{
    int nH = 0;
    for ( TxdColInfoListIT it = m_ltCols.begin(); it != m_ltCols.end(); it++ )
    {
        TxdColInfo *pInfo = *it;
        if ( pInfo->AllHeight > nH ) nH = pInfo->AllHeight;
    }
    return nH;
}

bool CxdQuiltCtrl::DelCtrl( CxdGraphicsControl* ApCtrl )
{
    for ( TxdColInfoListIT it = m_ltCols.begin(); it != m_ltCols.end(); it++ )
    {
        TxdColInfo *pInfo = *it;
        for ( TxdGraphicsCtrlListIT itCtrl = pInfo->CtrlList.begin(); itCtrl != pInfo->CtrlList.end(); itCtrl++ )
        {
            CxdGraphicsControl* pCtrl = *itCtrl;
            if ( (INT64)pCtrl == (INT64)ApCtrl )
            {
                DelCtrlFromCol( pInfo, itCtrl );
                Invalidate();
                return true;
            }
        }
    }
    return false;
}

void CxdQuiltCtrl::DelCtrl( int AColIndex )
{
    TxdColInfoListIT it = m_ltCols.begin() + AColIndex;
    if ( it == m_ltCols.end() ) return ;

    TxdColInfo *pInfo = *it;
    bool bMaxColHeight = pInfo->AllHeight == m_CurMaxHeight;
    for ( TxdGraphicsCtrlListIT itCtrl = pInfo->CtrlList.begin(); itCtrl != pInfo->CtrlList.end(); itCtrl++ )
    {
        CxdGraphicsControl *pCtrl = *itCtrl;
        OnDeleteCtrl( pCtrl );
        delete pCtrl;
    }
    pInfo->CtrlList.clear();
    pInfo->AllHeight = 0;

    if ( bMaxColHeight )
    {
        m_CurMaxHeight = CalcCurMaxColHeight();
        CheckVerScrollBar();
    }
    Invalidate();
}

bool CxdQuiltCtrl::DelCtrl( int AColIndex, int APos )
{
    TxdColInfoListIT it = m_ltCols.begin() + AColIndex;
    if ( it == m_ltCols.end() ) return false;

    TxdColInfo *pInfo = *it;
    TxdGraphicsCtrlListIT itCtrl = pInfo->CtrlList.begin() + APos;
    if ( itCtrl == pInfo->CtrlList.end() ) return false;

    DelCtrlFromCol( pInfo, itCtrl );
    Invalidate();
    return true;
}

void CxdQuiltCtrl::DelCtrlFromCol( TxdColInfo *ApInfo, TxdGraphicsCtrlListIT AIt )
{
    CxdGraphicsControl *pCtrl = *AIt;
    bool bMaxHeightCol = ApInfo->AllHeight == m_CurMaxHeight;
    int nH = pCtrl->Position()->GetHeight() + m_UpDownSpace;
    ApInfo->AllHeight -= nH;

    if ( m_AnimateOnDelCtrl )
    {
        //pCtrl->SetAnimateUnVisible( true );
        //pCtrl->SetVisible( false );
        int n = nH / 10;
        for ( int i = 0; i != 10; i++ )
        {
            if ( i == 9 )
            {
                n = nH - n * 9;
            }
            for ( TxdGraphicsCtrlListIT it = AIt + 1; it != ApInfo->CtrlList.end(); it++ )
            {
                CxdGraphicsControl *pItem = *it;
                pItem->Position()->SetTop( pItem->Position()->GetTop() - n );
                pItem->Invalidate();
                AfxPumpMessage();
                Sleep( 1 );
            }
        }
    }
    else
    {
        for ( TxdGraphicsCtrlListIT it = AIt + 1; it != ApInfo->CtrlList.end(); it++ )
        {
            CxdGraphicsControl *pItem = *it;
            pItem->Position()->SetTop( pItem->Position()->GetTop() - nH );
        }
    }

    if ( bMaxHeightCol )
    {
        m_CurMaxHeight = CalcCurMaxColHeight();
        CheckVerScrollBar();
    }
    ApInfo->CtrlList.erase( AIt );
    OnDeleteCtrl( pCtrl );
    delete pCtrl;
}

void CxdQuiltCtrl::Clear( void )
{
    for( int i = 0; i != (int)m_ltCols.size(); i++ )
    {
    	DelCtrl( i );
        TxdColInfoListIT it = m_ltCols.begin() + i;
        TxdColInfo *pInfo = *it;
        delete pInfo;
    }
    m_ltCols.clear();
}

void CxdQuiltCtrl::ClearItems( void )
{
    for( int i = 0; i != (int)m_ltCols.size(); i++ )
    {
        DelCtrl( i );
    }
}

void CxdQuiltCtrl::LoopItem( IxdLoopCtrlInterface *ApLoop )
{
    for( TxdColInfoListIT it = m_ltCols.begin(); it != m_ltCols.end(); it++ )
    {
        TxdColInfo *pInfo = *it;
        for ( TxdGraphicsCtrlListIT itCtrl = pInfo->CtrlList.begin(); itCtrl != pInfo->CtrlList.end(); itCtrl++ )
        {
            CxdGraphicsControl *pCtrl = *itCtrl;
            bool bBreak = false;
            ApLoop->Notify( pCtrl, bBreak );
            if ( bBreak ) return;
        }
    }
}
