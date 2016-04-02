#include "stdafx.h"
#include "JxduiCommon.h"
#include "JxdTemplateSub.h"
#include "JxdFileSub.h"

CxdImageInfo::CxdImageInfo()
{
    m_bFreeBmp = false;
    m_pBmp = NULL;
    m_BitmapItemCount = 3;    
    m_StretchX = 0;
    m_StretchY = 0;
    m_BitmapDrawStyle = dsStretchByVH;
}

CxdImageInfo::CxdImageInfo(const CxdImageInfo &ASrcObject)
{
    if ( this != &ASrcObject )
    {
        m_bFreeBmp = false;
        m_pBmp = ASrcObject.m_pBmp;
        m_BitmapItemCount = ASrcObject.m_BitmapItemCount;
        m_strFileName = ASrcObject.m_strFileName;
        m_BitmapDrawStyle = ASrcObject.m_BitmapDrawStyle;
        m_StretchX = ASrcObject.m_StretchX;
        m_StretchY = ASrcObject.m_StretchY;
    }
    else
    {
        m_bFreeBmp = false;
        m_pBmp = NULL;
        m_BitmapItemCount = 3;    
        m_StretchX = 0;
        m_StretchY = 0;
        m_BitmapDrawStyle = dsStretchByVH;
    }
}

CxdImageInfo& CxdImageInfo::operator=( const CxdImageInfo &ASrcObject )
{
    if ( this != &ASrcObject )
    {
        if ( m_pBmp != ASrcObject.m_pBmp )
        {
            //非同一图片对象
            CheckClearBmp();
            m_pBmp = ASrcObject.m_pBmp;
            m_BitmapItemCount = ASrcObject.m_BitmapItemCount;
            m_strFileName = ASrcObject.m_strFileName;
            m_BitmapDrawStyle = ASrcObject.m_BitmapDrawStyle;
            m_StretchX = ASrcObject.m_StretchX;
            m_StretchY = ASrcObject.m_StretchY;
            m_bFreeBmp = false;
        }
        else
        {
            //同一图片对象
            m_BitmapItemCount = ASrcObject.m_BitmapItemCount;
            m_BitmapDrawStyle = ASrcObject.m_BitmapDrawStyle;
        }
    }
    return *this;
}

CxdImageInfo::~CxdImageInfo()
{
    CheckClearBmp();
}

Bitmap* CxdImageInfo::GetBitmap( void )
{
    if ( !Assigned(m_pBmp) )
    {
        if ( FileExist(m_strFileName) )
        {
            m_pBmp = Bitmap::FromFile( m_strFileName );
            if ( Assigned(m_pBmp) && Ok == m_pBmp->GetLastStatus() )
            {
                m_bFreeBmp = true;
            }
            else
            {
                SafeDeleteObject( m_pBmp );
            }
        }
    }
    return m_pBmp;
}

bool CxdImageInfo::SetBitmap( Bitmap *ApBmp, bool AOnlyRes )
{
    m_strFileName.Empty();
    CheckClearBmp();
    m_pBmp = ApBmp;
    m_bFreeBmp = !AOnlyRes;
    return true;
}

bool CxdImageInfo::SetBitmapFileName( const CString &AStrBmpFileName )
{
    if ( FileExist(AStrBmpFileName) )
    {
        Bitmap *pBmp = Bitmap::FromFile( AStrBmpFileName );
        if ( Assigned(pBmp) && Ok == pBmp->GetLastStatus() )
        {
            CheckClearBmp();
            m_pBmp = pBmp;
            m_bFreeBmp = true;
        }
        else
        {
            SafeDeleteObject( pBmp );
            return false;
        }
        m_strFileName = AStrBmpFileName;
        return true;
    }
    return false;
}

void CxdImageInfo::CheckClearBmp()
{
    if ( m_bFreeBmp && Assigned(m_pBmp) )
    {
        m_bFreeBmp = false;
        delete m_pBmp;
    }
    m_pBmp = NULL;
}


//CxdFontInfo
CxdFontInfo::CxdFontInfo()
{
    m_strFontName = TEXT("Tahoma");
    m_FontColor = 0xFF000000;
    m_FontTrimming = StringTrimmingEllipsisCharacter;
    m_FontAlignment = StringAlignmentCenter;
    m_FontLineAlignment = StringAlignmentCenter; 
    m_bFreeFont = false;
    m_pFont = NULL;
    m_nFontSize = 11;
    m_pNormalOffset = NULL;
    m_pActiveOffset = NULL;
    m_pDownOffset = NULL;
}

CxdFontInfo::CxdFontInfo( const CxdFontInfo &ASrcObject )
{
    CopyObject(ASrcObject);
}

CxdFontInfo& CxdFontInfo::operator=( const CxdFontInfo &ASrcObject )
{
    return CopyObject(ASrcObject);
}

CxdFontInfo& CxdFontInfo::CopyObject( const CxdFontInfo &ASrcObject )
{
    if ( (INT64)this != (INT64)&ASrcObject )
    {
        if ( m_pFont != ASrcObject.m_pFont )
        {
            //非同一对象
            CheckClearFont();
            m_bFreeFont = false;
            m_pFont = ASrcObject.m_pFont;
        }
        m_strFontName = ASrcObject.m_strFontName;
        m_FontColor = ASrcObject.m_FontColor;
        m_FontAlignment = ASrcObject.m_FontAlignment;
        m_FontLineAlignment = ASrcObject.m_FontLineAlignment;
        m_FontTrimming = ASrcObject.m_FontTrimming;
        m_nFontSize = ASrcObject.m_nFontSize;
        UpdatePointer( m_pNormalOffset, ASrcObject.m_pNormalOffset );
        UpdatePointer( m_pActiveOffset, ASrcObject.m_pActiveOffset );
        UpdatePointer( m_pDownOffset, ASrcObject.m_pDownOffset );
    }
    return *this;
}


CxdFontInfo::~CxdFontInfo()
{
    CheckClearFont();
    SafeDeleteObject( m_pNormalOffset );
    SafeDeleteObject( m_pActiveOffset );
    SafeDeleteObject( m_pDownOffset );
}

void CxdFontInfo::SetFontName( const CString &AFontName )
{
    if ( AFontName.CompareNoCase(m_strFontName) != 0 )
    {
        CheckClearFont();
        m_strFontName = AFontName;
    }    
}

void CxdFontInfo::CheckClearFont( void )
{
    if ( m_bFreeFont )
    {
        SafeDeleteObject( m_pFont );
    }
    m_pFont = NULL;
    m_bFreeFont = false;
}

void CxdFontInfo::SetFont( Gdiplus::Font *AFont, bool AOnlyRes )
{
    CheckClearFont();
    m_bFreeFont = !AOnlyRes;
    m_pFont = AFont;
    if ( Assigned(m_pFont) )
    {
        FontFamily ff;
        m_pFont->GetFamily( &ff );
        TCHAR buf[1024] = {0};
        ff.GetFamilyName(buf);
        m_strFontName = buf;
        m_nFontSize = m_pFont->GetSize();
    }
    
}

Gdiplus::Font* CxdFontInfo::GetFont( void )
{
    if ( !Assigned(m_pFont) )
    {
        m_pFont = new Gdiplus::Font( m_strFontName, m_nFontSize );
        m_bFreeFont = true;
    }
    return m_pFont;
}

void CxdFontInfo::SetFontSize( Gdiplus::REAL ASize )
{
    if ( ASize > 0 && m_nFontSize != ASize )
    {
        m_nFontSize = ASize;
        CheckClearFont();
    }    
}

//CxdCursor
TCursorStyle CxdCursor::SetCursorStyle( TCursorStyle AStyle )
{
    if ( AStyle != m_crCursorStyle )
    {
        TCursorStyle csResult = m_crCursorStyle;
        m_crCursorStyle = AStyle;
        switch ( m_crCursorStyle )
        {
        case crDefault:
            m_pCursorName = IDC_ARROW;
            break;
        case crArrow:
            m_pCursorName = IDC_ARROW;
            break;
        case crIbeam:
            m_pCursorName = IDC_IBEAM;
            break;
        case crWait:
            m_pCursorName = IDC_WAIT;
            break;
        case crCross:
            m_pCursorName = IDC_CROSS;
            break;
        case crUparrow:
            m_pCursorName = IDC_UPARROW;
            break;
        case crSize:
            m_pCursorName = IDC_SIZE;
            break;
        case crIcon:
            m_pCursorName = IDC_ICON;
            break;
        case crSizeNWSE:
            m_pCursorName = IDC_SIZENWSE;
            break;
        case crSizeNESW:
            m_pCursorName = IDC_SIZENESW;
            break;
        case crSizeWE:
            m_pCursorName = IDC_SIZEWE;
            break;
        case crSizeNS:
            m_pCursorName = IDC_SIZENS;
            break;
        case crSizeALL:
            m_pCursorName = IDC_SIZEALL;
            break;
        case crHand:
            m_pCursorName = IDC_HAND;
            break;
        case crAppStarting:
            m_pCursorName = IDC_APPSTARTING;
            break;
        case crHelp:
            m_pCursorName = IDC_HELP;
            break;
        default:
            m_pCursorName = IDC_ARROW;
        }
        m_hCursor = LoadCursor( NULL, m_pCursorName );
        return csResult;
    }
    return AStyle;
}

void CxdAnchors::Add( TAnchorStyle AStyle )
{
    if ( AStyle != (m_nAnchors & AStyle) )
    {
        m_nAnchors |= AStyle;
    }
}

void CxdAnchors::Del( TAnchorStyle AStyle )
{
    if ( AStyle == (m_nAnchors & AStyle) )
    {
        m_nAnchors &= ~AStyle;
    }
}

bool CxdAnchors::IsExsits( TAnchorStyle AStyle )
{
    return AStyle == (m_nAnchors & AStyle);
}

CxdScreen::CxdScreen()
{
    m_nScreenWidth = GetSystemMetrics( SM_CXFULLSCREEN ) + GetSystemMetrics(SM_CXDLGFRAME);
    m_nScreenHeight = GetSystemMetrics( SM_CYFULLSCREEN ) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME);
}

CxdScreen::~CxdScreen()
{

}

//====================================================================
// HitMessage 接口
//====================================================================
//

static IxdDataInterface *HintNotifyInterface = NULL;
static IHintMessage *_pInterfaceHitMessage = NULL;

IHintMessage* GetHintInterface( void )
{
    if ( _pInterfaceHitMessage == NULL )
    {
        if ( HintNotifyInterface != NULL )
        {
            HintNotifyInterface->Notify( NULL );
        }
    }
    return _pInterfaceHitMessage;
}

void SetHintInterface( IHintMessage *ApInterface )
{
    _pInterfaceHitMessage = ApInterface;
}

void SetNotifyHintInterface( IxdDataInterface *ApInterface )
{
    HintNotifyInterface = ApInterface;
}

bool IsExsitsHintInterface( void )
{
    return _pInterfaceHitMessage != NULL;
}
