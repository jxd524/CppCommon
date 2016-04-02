#include "stdafx.h"
#include "JxdGifControl.h"
#include "JxdGdiPlusBasic.h"

//CxdGifParse
CxdGifParse::CxdGifParse()
{
    m_pBmpGif = NULL;
    m_nCurIndex = -1;
}

CxdGifParse::~CxdGifParse()
{
    ClearGifInfo();
}

void CxdGifParse::ClearGifInfo( void )
{
    for ( TGifInfoListIt it = m_ltGifInfo.begin(); it != m_ltGifInfo.end(); it++ )
    {
        TGifInfo *p = *it;
        delete p;
    }
    m_ltGifInfo.clear();
    m_pBmpGif = NULL;
    m_nCurIndex = -1;
}

bool CxdGifParse::InitGif( Gdiplus::Bitmap *ApGifBmp )
{
    ClearGifInfo();
    m_pBmpGif = ApGifBmp;
    if ( !Assigned(m_pBmpGif) )
    {
        return false;
    }
    int nCount = m_pBmpGif->GetFrameDimensionsCount();
    if ( nCount <= 0 )
    {
        return false;
    }
    m_nCurIndex = -1;
    GUID GD;
    m_pBmpGif->GetFrameDimensionsList( &GD, 1 );
    nCount = m_pBmpGif->GetFrameCount( &GD );
    UINT nSize = m_pBmpGif->GetPropertyItemSize( PropertyTagFrameDelay );
    if ( nSize == 0 )
    {
        nSize = sizeof(PropertyItem);
    }
    PropertyItem *pItem = (PropertyItem*)malloc(nSize);
    try
    {
        if (Ok != m_pBmpGif->GetPropertyItem( PropertyTagFrameDelay, nSize, pItem ) )
        {
            TGifInfo *p = new TGifInfo;
            p->FPauseTime = 0xFFFFFFFF;
            p->FPosIndex = 0;
            m_ltGifInfo.push_back( p );
        }
        else
        {
            UINT *pValue = (UINT*)pItem->value;
            for ( int i = 0; i != nCount; i++ )
            {
                TGifInfo *p = new TGifInfo;
                // modify [2012-8-3 23:28 by Terry]
                p->FPauseTime = pValue[i] * 10 + 10;
                p->FPosIndex = i;
                m_ltGifInfo.push_back( p );
            }
        }

    }
    catch (...)
    {
    }
    free(pItem);
    return !m_ltGifInfo.empty();
}

int CxdGifParse::NextBitmap( void )
{
    if ( !m_ltGifInfo.empty() && Assigned(m_pBmpGif) )
    {
        m_nCurIndex = (m_nCurIndex + 1) % m_ltGifInfo.size();
        TGifInfo *p = m_ltGifInfo[m_nCurIndex];    
        m_pBmpGif->SelectActiveFrame( &FrameDimensionTime, p->FPosIndex );
        return p->FPauseTime;
    }
    return -1;
}


/*CxdGifControl*/
CxdGifControl::CxdGifControl(): CxdGraphicsControl(), CtAutShowGifTimerID(GetUnionID())
{
    SetBufferBitmapTimeout( 0 );
    m_bPlaying = false;
    m_ShowFirstFrameOnStop = true;
    m_bAutoPlayOnActive = false;
    m_DrawActiveBorder = false;
    m_DrawNormalBorder = false;
    m_BorderNormalColor = 0xFF9900aa;
    m_BorderNormalColor = 0xFFF0FF00;
}

CxdGifControl::~CxdGifControl()
{ 
    EndTimer( CtAutShowGifTimerID );
    ClearGifInfo();
}

void CxdGifControl::BeginShowGif( void )
{
    if ( Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() )
    {
        if ( m_oGifInfo.GetGifBitmap() != m_pImgInfo->GetBitmap() )
        {
            ClearGifInfo();
            m_oGifInfo.InitGif( m_pImgInfo->GetBitmap() );
        }
        if ( m_oGifInfo.GetCount() > 0 )
        {
            m_bPlaying = true;
            StartTimer( CtAutShowGifTimerID, m_oGifInfo.NextBitmap() );
        }
    }
}

void CxdGifControl::ClearGifInfo( void )
{
    m_oGifInfo.ClearGifInfo();
}

void CxdGifControl::StopShowGif( )
{
    if ( m_bPlaying )
    {
        m_bPlaying = false;
        EndTimer( CtAutShowGifTimerID );

        if ( m_ShowFirstFrameOnStop && m_oGifInfo.GetCount() > 1 )
        {
            m_oGifInfo.SetFirstFrame();
            m_oGifInfo.NextBitmap();
            Invalidate();
        }
    }    
}

bool CxdGifControl::OnTimerMessage( int ATimeID, LPVOID ApData )
{
    if ( ATimeID == CtAutShowGifTimerID )
    {
        if ( !m_bPlaying )
        {
            m_oGifInfo.SetFirstFrame();
            m_oGifInfo.NextBitmap();
            Invalidate();
            return false;
        }
        else
        {
            StartTimer( CtAutShowGifTimerID, m_oGifInfo.NextBitmap() );
            Invalidate();
            return true;
        }
    }
    return CxdGraphicsControl::OnTimerMessage(ATimeID, ApData);
}

bool CxdGifControl::OnUIStateChanged( const TxdGpUIState &ANewSate )
{
    if ( m_bAutoPlayOnActive )
    {
        ANewSate != uiNormal ? BeginShowGif() : StopShowGif();
    }
    if ( m_DrawNormalBorder || m_DrawNormalBorder )
    {
        Invalidate();
    }
    return CxdGraphicsControl::OnUIStateChanged(ANewSate);
}

void CxdGifControl::SetAutoPlayOnActive( bool bActive )
{
    if ( m_bAutoPlayOnActive != bActive )
    {
        m_bAutoPlayOnActive = bActive;
        GetCurUISate() != uiNormal ? BeginShowGif() : ( m_bPlaying ? NULL : StopShowGif() );
    }
}

void CxdGifControl::DoDrawSubjoinBK(Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt)
{
    if ( m_DrawNormalBorder && GetCurUISate() == uiNormal )
    {
        Rect R( ADestRect.X, ADestRect.Y, ADestRect.Width - 1, ADestRect.Height - 1 );
        ADestG.DrawRectangle( &Pen(m_BorderNormalColor), R );
    }
    else if ( m_DrawNormalBorder && GetCurUISate() == uiActive )
    {
        Rect R( ADestRect.X, ADestRect.Y, ADestRect.Width - 1, ADestRect.Height - 1 );
        ADestG.DrawRectangle( &Pen(m_BorderNormalColor), R );
    }
}

void CxdGifControl::SetGifInfo( CxdGifParse *ApGifInfo )
{
	if ( !Assigned( ApGifInfo) ) return;

	m_oGifInfo = *ApGifInfo;
}
