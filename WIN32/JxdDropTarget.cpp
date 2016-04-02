#include "StdAfx.h"
#include "JxdDropTarget.h"
#include "JxdStringCoder.h"
#include "JxdStringHandle.h"
#include "JxdNetwork.h"

#include <string>
#include <algorithm>
using namespace std; 

static CtConst   CtFileHead = TEXT("file:///");
static const int  CtFileHeadLen = _tcslen(CtFileHead);
static CtConst   CtpStartHTML = TEXT("StartHTML:");
static CtConst   CtpStartFragment = TEXT("StartFragment:");
static CtConst   CtpEndFragment = TEXT("EndFragment:");
static CtConst   CtpSourceURL = TEXT("SourceURL:");
static CtConst   CtpScriptEnd = TEXT("</SCRIPT>");
static CtConst   CtpHtmlBegin = TEXT("<");
static CtConst   CtpHtmlEnd = TEXT(">");
static CtConst   CtHttpHeadInfo = TEXT("http://");
static const int CtHttpHeadInfoLength = _tcslen(CtHttpHeadInfo);
static const int CtStartHTMLLen = _tcslen( CtpStartHTML );
static const int CtStartFragmentLen = _tcslen( CtpStartFragment );
static const int CtEndFragmentLen = _tcslen( CtpEndFragment);
static const int CtSourceURLLen = _tcslen( CtpSourceURL );
static const int CtScriptEndLen= _tcslen( CtpScriptEnd );
static const int CtHtmlBeginLen = _tcslen( CtpHtmlBegin );
static const int CtHtmlEndLen = _tcslen( CtpHtmlEnd );

CxdDropTarget::CxdDropTarget(void)
{
    OleInitialize(NULL);
    UINT formats[1024] = {0};
    UINT nCount = 0;
    InitInterface( DragInfo );
    InitInterface( DragScroll );
    m_nHtmlFormat = RegisterClipboardFormat( TEXT("HTML Format") );
}


CxdDropTarget::~CxdDropTarget(void)
{
    DeleteInterface( DragInfo );
    DeleteInterface( DragScroll );
    ClearClipInfoList();
    Revoke();
    ExternalDisconnect();
    OleUninitialize();
}

DROPEFFECT CxdDropTarget::OnDragEnter( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point )
{
    TRACE( TEXT("CJxdDropTarget::OnDragEnter\r\n") );
    return DROPEFFECT_NONE;
}

DROPEFFECT CxdDropTarget::OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point )
{
    return DROPEFFECT_COPY;
    if ( pDataObject->IsDataAvailable(CF_TEXT) || pDataObject->IsDataAvailable(CF_UNICODETEXT) )
    {
        //EnumOleDataInfo( pDataObject );
        return DROPEFFECT_COPY;
    }
    else
    {
        return DROPEFFECT_NONE;//不接收DropDrag
    }
}

BOOL CxdDropTarget::OnDrop( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point )
{
    TRACE( TEXT("CJxdDropTarget::OnDrop\r\n") );
    //EnumOleDataInfo( pDataObject );

    ParseClipInfo( pDataObject );

    return TRUE;
}

DROPEFFECT CxdDropTarget::OnDropEx( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point )
{
    TRACE( TEXT("CJxdDropTarget::OnDropEx\r\n") );
    return (DROPEFFECT)-1; //不需要处理
}

void CxdDropTarget::OnDragLeave( CWnd* pWnd )
{
    TRACE( TEXT("CJxdDropTarget::OnDragLeave\r\n") ); 
}

DROPEFFECT CxdDropTarget::OnDragScroll( CWnd* pWnd, DWORD dwKeyState, CPoint point )
{
    NotifyInterfaceEx( DragScroll, point.x, point.y );
    return DROPEFFECT_NONE;
}

void CxdDropTarget::EnumOleDataInfo( COleDataObject *pDataObject )
{
    FORMATETC etc;
    UINT      uNumFormats = 0;         //.剪贴板上数据格式种类数
    pDataObject->BeginEnumFormats();

    while ( pDataObject->GetNextFormat ( &etc ))
    {
        TCHAR tName[255] = {0};
        GetClipboardFormatName( etc.cfFormat, tName, 255 );
        CString s;
        s.Format( TEXT("FormatName：%s\r\n"), tName );
        OutputDebugString( s );

        if ( pDataObject->IsDataAvailable ( etc.cfFormat ))
        {
            CString s;
            s.Format( TEXT("格式：%d\r\n"), etc.cfFormat );
            TRACE( s );

            HGLOBAL hMemText = pDataObject->GetGlobalData( etc.cfFormat );
            if ( hMemText != NULL )
            {
                char *pAnsiData = (char*)GlobalLock(hMemText);
                int nSize = GlobalSize(hMemText);
                CString s;
                s.Format( TEXT("  内容大小：%d\r\n"), nSize );
                TRACE( s );

                TRACE( TEXT("以下为内容：\r\n") );
                TRACE( pAnsiData );
                GlobalUnlock(hMemText);
                GlobalFree(hMemText);
                TRACE( TEXT("\r\n内容结束：\r\n") );
            } 
            TRACE( TEXT("==========end========\r\n\r\n") );
            uNumFormats++;
        }
    }
    CString s;
    s.Format( TEXT("当前剪贴板上拥有数据格式数量：%d\r\n"), uNumFormats );
    TRACE( s );
}

void CxdDropTarget::ParseClipInfo( COleDataObject *pDataObject )
{
    //CF_HTML -> CF_TEXT or CF_UNICODETEXT or CF_DIB
    if ( !Assigned(pDataObject) )
    {

        SaveClipInfo();

        //Handle 
        OpenClipboard( NULL );
        EmptyClipboard();    
        CloseClipboard();
        //ctrl + c
        keybd_event( VK_CONTROL, 0, 0, 0 );
        keybd_event( 'C', 0, 0, 0 );    
        keybd_event( 'C', 0, KEYEVENTF_KEYUP, 0 );
        keybd_event( VK_CONTROL, 0, KEYEVENTF_KEYUP, 0 );
        for ( int i = 0; i != 10; i ++ )
        {
            Sleep( 10 );
        }
    }
    HanldeClipFormatInfo( pDataObject );    

    if ( !Assigned(pDataObject) )
    {
        RestorClipInfo();
    }
}

void CxdDropTarget::ClearClipInfoList( void )
{
    for ( vector< TClipInfo* >::iterator it = m_ClipInfoList.begin(); it != m_ClipInfoList.end(); it++ )
    {
        TClipInfo *p = *it;
        if ( Assigned(p->FMemHandle) )
        {
            GlobalFree( p->FMemHandle );
            p->FMemHandle = NULL;
        }
        delete p;
    }
    m_ClipInfoList.clear();
}

void CxdDropTarget::SaveClipInfo( void )
{
    OpenClipboard( NULL );
    UINT nFormat(0);
    while ( (nFormat = EnumClipboardFormats(nFormat)) != 0 )
    {
        HANDLE hMem = GetClipboardData( nFormat );

        LPVOID pData = GlobalLock(hMem);

        SIZE_T nSize = GlobalSize(hMem);
        TClipInfo *p = new TClipInfo;
        p->FFormat = nFormat;
        p->FMemHandle = GlobalAlloc( GMEM_MOVEABLE, nSize );
        LPVOID pSaveData = GlobalLock( p->FMemHandle );
        memcpy( pSaveData, pData, nSize );
        GlobalUnlock( p->FMemHandle );
        m_ClipInfoList.push_back(p);

        GlobalUnlock(hMem);
    }    
    CloseClipboard();
    ClearClipInfoList();
}

void CxdDropTarget::RestorClipInfo( void )
{
    OpenClipboard( NULL );
    EmptyClipboard();    
    for ( vector< TClipInfo* >::iterator it = m_ClipInfoList.begin(); it != m_ClipInfoList.end(); it++ )
    {
        TClipInfo *p = *it;
        LPVOID pSaveData = GlobalLock( p->FMemHandle );
        GlobalUnlock( p->FMemHandle );
        SetClipboardData( p->FFormat, p->FMemHandle );
    }
    CloseClipboard();
}

void CxdDropTarget::DoHandleClipData( COleDataObject *pDataObject, UINT AFormat, LPVOID ADataBuf )
{
    if ( AFormat == m_nHtmlFormat )
    {
        CxdParseDropHtml obj;
        if ( obj.ParseDropHtml((char*)ADataBuf) )
        {
            if ( obj.IsHasImageURL() )
            {
                NotifyInterfaceEx( DragInfo, dsHTML, &obj );
            }
            else
            {
                HandleTextClipData( pDataObject );
            }
        }
        else
        {
            HandleTextClipData( pDataObject );
        }
    }
    else
    {
        switch ( AFormat )
        {
        case CF_UNICODETEXT:
            {
                NotifyInterfaceEx( DragInfo,  dsString, &CString( (TCHAR*)ADataBuf ) );
                break;
            }
        case CF_TEXT:
            {
                NotifyInterfaceEx( DragInfo, dsString, &CString( AsciiToUnicode( (char*)ADataBuf ).c_str() ) );
                break;
            }
        case CF_DIB:
            {
                NotifyInterfaceEx( DragInfo, dsDIB, ADataBuf );
                break;
            }
        case CF_HDROP:
            {
                HDROP hFiles = (HDROP)ADataBuf;
                int nFileCount = DragQueryFile( hFiles, -1, NULL, 0 );
                for ( int i = 0; i < nFileCount; i++ )
                {
                    int cch = DragQueryFile( hFiles, i, NULL, 0 );
                    cch *= 2;
                    TCHAR* pFileName = new TCHAR[cch + 2]; //(TCHAR*)GlobalAlloc( GPTR, cch + 1 );
                    ZeroMemory( pFileName, cch + 2 );
                    DragQueryFile( hFiles, i, pFileName, cch );

                    NotifyInterfaceEx( DragInfo, dsFileName, &CString( pFileName ) );
                    delete []pFileName;
                }
                break;
            }
        }

    }
}

BOOL CxdDropTarget::HandleClipData( UINT AFromat, COleDataObject *pDataObject )
{
    BOOL bOK;
    if ( Assigned(pDataObject) )
    {
        bOK = pDataObject->IsDataAvailable(AFromat);
        CString s;
        s.Format( TEXT("pDataObject->IsDataAvailable: %d (FORMAT: %d)\r\n"), bOK, AFromat );
        TRACE( s );
    }
    else
    {
        bOK = IsClipboardFormatAvailable(AFromat);
        CString s;
        s.Format( TEXT("IsClipboardFormatAvailable: %d (FORMAT: %d)\r\n"), bOK, AFromat );
        TRACE( s );
    }
    if ( !bOK ) return FALSE;

    TRACE( TEXT("HandleClipData \r\n") );
    HANDLE hClipData = 0;
    bool bCloseClip( false );

    if ( Assigned(pDataObject) )
    {
        hClipData = pDataObject->GetGlobalData( AFromat );
    }
    else
    {
        if ( !OpenClipboard(m_hWnd) ) 
        {
            TRACE( TEXT("can not open clipborad!!! \r\n") );
            return FALSE;
        }
        hClipData = GetClipboardData( AFromat );
        bCloseClip = true;
    }

    if ( Assigned(hClipData) )
    {
        LPVOID pBuf = GlobalLock(hClipData);
        DoHandleClipData( pDataObject, AFromat, pBuf );
        GlobalUnlock( hClipData );

        if ( bCloseClip )
        {
            CloseClipboard();
        }
        return TRUE;
    }

    if ( bCloseClip )
    {
        CloseClipboard();
    }
    return FALSE;
}

void CxdDropTarget::HanldeClipFormatInfo( COleDataObject *pDataObject )
{
    if ( !HandleClipData(m_nHtmlFormat, pDataObject) )
    {
        HandleClipData( CF_DIB, pDataObject );
        HandleTextClipData( pDataObject );
        HandleClipData( CF_HDROP, pDataObject );
    }
}

void CxdDropTarget::HandleTextClipData( COleDataObject *pDataObject )
{
    if ( !HandleClipData(CF_UNICODETEXT, pDataObject) )
    {
        HandleClipData( CF_TEXT, pDataObject );
    }
}

bool CxdParseDropHtml::ParseDropHtml( const char *ApUtfDropText )
{
    //可能包含多个选择内容
    ClearDropHtmlInfo();

    CStringA strSrcHtml( ApUtfDropText );
    CString strText = Utf8ToUnicode( ApUtfDropText ).c_str();
    OutputDebugString( strText );
    m_strLineEnd = GetLineEndSign( strText ).c_str();
    if ( m_strLineEnd.IsEmpty() ) return false;

    int nPos1(0), nPos2(0);
    wstring strTemp = GetTheString( strText, CtpStartHTML, m_strLineEnd );
    nPos1 = (int)StrToInt64( strTemp.c_str() );
    if ( nPos1 <= -1 ) return false;

    CString strHtmlHead = strText.Mid( 0, nPos1 );
    wstring strSourceURL = GetTheString( strHtmlHead, CtpSourceURL, m_strLineEnd );
    TURLInfo urlInfo;
    if ( !strSourceURL.empty() && UrlParse(strSourceURL.c_str(), urlInfo, ICU_ESCAPE) )
    {
        strSourceURL = urlInfo.FUrlHead + TEXT("://") + urlInfo.FHost;
    }
    int nBeginPos(0);
    CStringA strTempSelHtml;
    CStringW strSelHtml;
    while ( true )
    {
        nBeginPos = GetTheString( strHtmlHead, CtpStartFragment, m_strLineEnd, strTemp, tpNULL, nBeginPos );
        if ( -1 == nBeginPos )
        {
            break;
        }
        nPos1 = _ttoi( strTemp.c_str() );

        nBeginPos = GetTheString( strHtmlHead, CtpEndFragment, m_strLineEnd, strTemp, tpNULL, nBeginPos );
        if ( -1 == nBeginPos )
        {
            break;
        }
        nPos2 = _ttoi( strTemp.c_str() );
        if ( nPos2 <= nPos1 ) break;
        CString strTmp = strTemp.c_str();
        strTempSelHtml = strSrcHtml.Mid( nPos1, nPos2 - nPos1 );
        strSelHtml = Utf8ToUnicode( strTempSelHtml ).c_str();
        int nHtmlPos(0);
        while ( true )
        {
            int n1 = strSelHtml.Find( CtpHtmlBegin, nHtmlPos );
            if ( -1 == n1 )
            {
                strTmp = strSelHtml.Mid( nHtmlPos );
                strTmp.Trim();
                if ( !strTmp.IsEmpty() )
                {

                    ReplaceHtmlInfo( strTmp );
                    TDropHtmlInfo *p = new TDropHtmlInfo;
                    p->FInfo = strTmp;
                    p->FStyle = dhsString;
                    m_lsDropHtmlInfo.push_back( p );
                }
                break;
            }
            if ( n1 - nHtmlPos > 0 )
            {
                strTmp = strSelHtml.Mid( nHtmlPos, n1 - nHtmlPos );
                if ( !strTemp.empty() )
                {
                    ReplaceHtmlInfo( strTmp );
                    TDropHtmlInfo *p = new TDropHtmlInfo;
                    p->FInfo = strTmp;
                    p->FStyle = dhsString;
                    m_lsDropHtmlInfo.push_back( p );
                }
                nHtmlPos = n1 + CtHtmlBeginLen;
            }
            else
            {
                nHtmlPos += CtHtmlBeginLen;
            }
            int n2 = strSelHtml.Find( CtpHtmlEnd, nHtmlPos );
            if ( n2 > 0 )
            {                                
                strTmp = strSelHtml.Mid( nHtmlPos, n2 - nHtmlPos );
                nHtmlPos = n2 + CtHtmlEndLen;
                int nContentLen = strTmp.GetLength();
                //分析HTML内容中所需要处理的信息：IMG
                if ( (nContentLen > 3) && (0 == _tcsnicmp(TEXT("img"), strTmp, 3)) ) //IMG
                {
                    CString strImgSign( TEXT("src=\"") );
                    int nImgPos = Pos( strTmp, strImgSign, 0 );
                    if ( -1 == nImgPos )
                    {
                        continue;
                    }
                    nImgPos += strImgSign.GetLength();
                    int nImgEndPos = strTmp.Find( '"', nImgPos );
                    if ( nImgPos < nImgEndPos )
                    {
                        strTmp = strTmp.Mid( nImgPos, nImgEndPos - nImgPos );
                        if ( !strTmp.IsEmpty() )
                        {
                            bool bAddUrlHead( true );
                            if ( (strTmp.GetLength() > CtHttpHeadInfoLength) &&
                                (0 == _tcsnicmp(CtHttpHeadInfo, strTmp, CtHttpHeadInfoLength)) )
                            {
                                bAddUrlHead = false;
                            }
                            if ( bAddUrlHead )
                            {
                                if ( 0 != _tcsnicmp(CtFileHead, strTmp, CtFileHeadLen) )
                                {
                                    ( '/' == strTemp[0] ) ?
                                        strTmp = strSourceURL.c_str() + strTmp :
                                    strTmp = strSourceURL.c_str() + '/' + strTmp;
                                }
                            }
                            TDropHtmlInfo *p = new TDropHtmlInfo;
                            p->FInfo = strTmp;
                            p->FStyle = dhsImgURL;
                            m_lsDropHtmlInfo.push_back( p );
                        }
                    }
                } //End Img
                else if ( (nContentLen > 6) && (0 == _tcsnicmp(TEXT("SCRIPT"), strTmp, 6)) ) //SCRIPT
                {
                    int nEndSrciptPos = Pos( strSelHtml, CtpScriptEnd, nHtmlPos );
                    if ( nEndSrciptPos > nHtmlPos )
                    {
                        nHtmlPos = nEndSrciptPos + CtScriptEndLen;
                    }
                } // End SCRIPT
                else if ( (nContentLen >= 2) && (0 == _tcsnicmp(TEXT("br"), strTmp, 2)) ) //br
                {
                    if ( strTmp.GetLength() > 2 && strTemp[2] != ' ' ) continue;
                    for ( TDropHtmlInfoList::reverse_iterator it = m_lsDropHtmlInfo.rbegin(); it != m_lsDropHtmlInfo.rend(); it++ )
                    {
                        TDropHtmlInfo *p = *it;
                        if ( p->FStyle == dhsString )
                        {
                            p->FInfo += TEXT("\r\n");
                            break;
                        }
                    }
                } //End br
            }
            else
            {
                break;
            }
        }
    }
    return !m_lsDropHtmlInfo.empty();
}

void CxdParseDropHtml::ClearDropHtmlInfo( void )
{
    for ( vector< TDropHtmlInfo* >::iterator it = m_lsDropHtmlInfo.begin(); it != m_lsDropHtmlInfo.end(); it++ )
    {
        TDropHtmlInfo *p = *it;
        delete p;
    }
    m_lsDropHtmlInfo.clear();
}

CxdParseDropHtml::~CxdParseDropHtml()
{
    ClearDropHtmlInfo();
}

void CxdParseDropHtml::ReplaceHtmlInfo( CString &AInfo )
{
    AInfo.Replace( TEXT("&lt;"), TEXT("<") );
    AInfo.Replace( TEXT("&gt;"), TEXT(">") );
    AInfo.Replace( TEXT("&rt;"), TEXT(">") );
    AInfo.Replace( TEXT("&quot;"), TEXT("\\") );    
    AInfo.Replace( TEXT("&#039;"), TEXT("@#") );
    AInfo.Replace( TEXT("&nbsp;"), TEXT(" ") );
    AInfo.Replace( TEXT("&amp;"), TEXT("&") );
}

bool CxdParseDropHtml::IsHasImageURL( void )
{
    for ( TDropHtmlInfoList::iterator it = m_lsDropHtmlInfo.begin(); it != m_lsDropHtmlInfo.end(); it++ )
    {
        TDropHtmlInfo *pInfo = *it;
        if ( dhsImgURL == pInfo->FStyle )
        {
            return true;
        }
    }
    return false;
}
