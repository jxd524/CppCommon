#include "stdafx.h"
#include "JxdBrowserCom.h"
#include "JxdMacro.h"
#include "JxdStringHandle.h"

//ObjectFromLresult
#include <OleAcc.h>
#pragma comment( lib, "Oleacc.lib" )

#define CheckInited(ReturnValue) {\
    if ( !IsInited() ) \
        { return ReturnValue; }\
    }




CxdBroswerCom::CxdBroswerCom()
{
    InitValue();
}

CxdBroswerCom::CxdBroswerCom( const HWND &AWndHandle )
{
    InitValue();
    InitBrowserCom( AWndHandle );
}

CxdBroswerCom::CxdBroswerCom( LPDISPATCH ApDocument )
{
    InitValue();
    InitBrowserCom( ApDocument );
}


CxdBroswerCom::~CxdBroswerCom()
{
    if ( m_bInitByHandle )
    {
        m_lpDoc2 = NULL;
        m_lpDoc3 = NULL;
        ::CoUninitialize();
    }
}

bool CxdBroswerCom::InitBrowserCom( LPDISPATCH ApDocument )
{
    if ( !Assigned(ApDocument) )
    {
        return false;
    }
    if ( m_bInitByHandle )
    {
        ::CoUninitialize();
    }
    m_lpDoc3 = NULL;
    m_lpDoc2 = NULL;
    HRESULT hr = ApDocument->QueryInterface( IID_IHTMLDocument2, (LPVOID*)&m_lpDoc2 );
    if ( FAILED(hr) )
    {
        m_lpDoc2 = NULL;
        return false;
    }
    return true;
}

BOOL CALLBACK EnumIEChildProc(HWND hwnd,LPARAM lParam)
{
    TCHAR szClassName[128] = {0};
    ::GetClassName( hwnd,  szClassName, 128 );
    const CString strIESign( TEXT("Internet Explorer_Server") );
    if ( 0 == strIESign.CompareNoCase(szClassName) )
    {
        *(HWND*)lParam = hwnd;
        return FALSE;		// 找到第一个 IE 控件的子窗口就停止
    }
    else	
        return TRUE;		// 继续枚举子窗口
};

bool CxdBroswerCom::InitBrowserCom( const HWND &AWndHandle )
{
    if ( !IsWindow(AWndHandle) )
    {
        return false;
    }

    m_bInitByHandle = true;
    m_lpDoc2 = NULL;
    m_lpDoc3 = NULL;
    ::CoInitialize(NULL);

    HWND hWndChild=NULL;
    ::EnumChildWindows( AWndHandle, EnumIEChildProc, (LPARAM)&hWndChild );
    if( hWndChild )	
    {
        UINT nMsg = ::RegisterWindowMessage( _T("WM_HTML_GETOBJECT") );
        LRESULT lRes;
        ::SendMessageTimeout( hWndChild, nMsg, 0L, 0L, SMTO_ABORTIFHUNG, 1000, (DWORD*)&lRes );
        HRESULT hr = ::ObjectFromLresult ( lRes, IID_IHTMLDocument2, 0 , (LPVOID*)&m_lpDoc2 );
        if ( SUCCEEDED(hr) )
        {
            return true;
        }
    }
    m_lpDoc2 = NULL;
    m_bInitByHandle = false;
    ::CoUninitialize();
    return false;
}

bool CxdBroswerCom::GetTitle( CString &ATitle )
{
    if ( !IsInited() )
    {
        return false;
    }
    BSTR bsTitle;
    HRESULT hr = m_lpDoc2->get_title( &bsTitle );
    if ( FAILED(hr) )
    {
        return false;
    }
    ATitle = bsTitle;
    return true;
}

bool CxdBroswerCom::SetTitle( const TCHAR *ApTitle )
{
    CheckInited( false );
    BSTR bsTitle = _bstr_t(ApTitle);
    return SUCCEEDED( m_lpDoc2->put_title(bsTitle) );
}

bool CxdBroswerCom::GetElementCollection( const TElementCollectionStyle &tECtype, CComPtr<IHTMLElementCollection> &spElemCollection )
{
    CheckInited( false );
    HRESULT hr;
    switch(tECtype)
    {
    case ecAll:
        {
            hr = m_lpDoc2->get_all(&spElemCollection);
            break;
        }
    case ecBody:
        {
            CComPtr<IHTMLElement> spBodyElem;
            hr = m_lpDoc2->get_body(&spBodyElem);
            if ( SUCCEEDED(hr) )
            {
                CComPtr<IDispatch> spChildElem;
                hr = spBodyElem->get_children(&spChildElem);
                if ( SUCCEEDED(hr) )
                {
                    hr = spChildElem->QueryInterface(IID_IHTMLElementCollection, (LPVOID*)&spElemCollection);
                }
            }
            break;
        }
    case ecForms:
        {
            hr = m_lpDoc2->get_forms(&spElemCollection);
            break;
        }
    case ecScriptes:
        {
            hr = m_lpDoc2->get_scripts(&spElemCollection);
            break;
        }
    case ecImages:
        {
            hr = m_lpDoc2->get_images(&spElemCollection);
            break;
        }
    default:
        return false;
    }
    return SUCCEEDED(hr) && spElemCollection != NULL;
}

int CxdBroswerCom::GetCollectionElementCount( CComPtr<IHTMLElementCollection> &ApCollection )
{
    long nCount(0);
    ApCollection->get_length( &nCount );
    return nCount;
}

bool CxdBroswerCom::GetCollectionItem( CComPtr<IHTMLElementCollection> &ApCollection, const int &AIndex, LPDISPATCH &ApItem )
{
    return SUCCEEDED(ApCollection->item(CComVariant(AIndex), CComVariant(0), &ApItem)) && Assigned(ApItem);
}

bool CxdBroswerCom::CheckFormInterface( LPDISPATCH ApDispatch, CComPtr<IHTMLFormElement> &ApForm )
{
    return SUCCEEDED( ApDispatch->QueryInterface(IID_IHTMLFormElement, (LPVOID*)&ApForm) ) && Assigned(ApForm);
}

bool CxdBroswerCom::CheckElementInterface( LPDISPATCH ApDispatch, CComPtr<IHTMLElement> &ApElement )
{
    return SUCCEEDED( ApDispatch->QueryInterface(IID_IHTMLElement, (LPVOID*)&ApElement) ) && Assigned(ApElement);
}

bool CxdBroswerCom::CheckInputElementInterface( LPDISPATCH ApDispatch, CComPtr<IHTMLInputElement> &ApElement )
{
    return SUCCEEDED( ApDispatch->QueryInterface(IID_IHTMLInputElement, (LPVOID*)&ApElement) ) && Assigned(ApElement);
}

bool CxdBroswerCom::CheckImgElementInterface( LPDISPATCH ApDispatch, CComPtr<IHTMLImgElement> &ApElement )
{
    return SUCCEEDED( ApDispatch->QueryInterface(IID_IHTMLImgElement, (LPVOID*)&ApElement) ) && Assigned(ApElement);
}

bool CxdBroswerCom::CheckLinkElementInterface( LPDISPATCH ApDispatch, CComPtr<IHTMLLinkElement> &ApElement )
{
    return SUCCEEDED( ApDispatch->QueryInterface(IID_IHTMLLinkElement, (LPVOID*)&ApElement) ) && Assigned(ApElement);
}

bool CxdBroswerCom::CheckScriptElementInterface( LPDISPATCH ApDispatch, CComPtr<IHTMLScriptElement> &ApElement )
{
    return SUCCEEDED( ApDispatch->QueryInterface(IID_IHTMLScriptElement, (LPVOID*)&ApElement) ) && Assigned(ApElement);
}

bool CxdBroswerCom::CheckElement2Interface( LPDISPATCH ApDispatch, CComPtr<IHTMLElement2> &ApElement )
{
    return SUCCEEDED( ApDispatch->QueryInterface(IID_IHTMLElement2, (LPVOID*)&ApElement) ) && Assigned(ApElement);
}

bool CxdBroswerCom::CheckElement3Interface( LPDISPATCH ApDispatch, CComPtr<IHTMLElement3> &ApElement )
{
    return SUCCEEDED(ApDispatch->QueryInterface(IID_IHTMLImgElement3, (LPVOID*)&ApElement)) && Assigned(ApElement);
}

bool CxdBroswerCom::GetHtml( CString &AHtmlText )
{
    CComPtr<IHTMLElement> spRoot;
    if ( !GetDocumentElement(spRoot) )
    {
        return false;
    }
    BSTR bsText;
    spRoot->get_innerHTML( &bsText );
    AHtmlText = bsText;
    return true;
}

bool CxdBroswerCom::GetDocumentElement( CComPtr<IHTMLElement> &ApRootElement )
{
    CheckInited( false );
    if ( !CheckDoc3() )
    {
        return false;
    }
    return SUCCEEDED( m_lpDoc3->get_documentElement(&ApRootElement) ) && Assigned(ApRootElement);
}

bool CxdBroswerCom::CheckDoc3( void )
{
    if ( !Assigned(m_lpDoc3) )
    {
        if ( !Assigned(m_lpDoc2) )
        {
            return false;
        }
        if ( FAILED(m_lpDoc2->QueryInterface(IID_IHTMLDocument3, (LPVOID*)&m_lpDoc3)) || !Assigned(m_lpDoc3) )
        {
            m_lpDoc3 = NULL;
            return false;
        }
    }
    return true;
}

bool CxdBroswerCom::GetElementByID( const TCHAR *ApID, CComPtr<IHTMLElement> &ApElement )
{
    CheckInited( false );
    if ( CheckDoc3() )
    {
        BSTR bsID = _bstr_t(ApID);
        return SUCCEEDED(m_lpDoc3->getElementById(bsID, &ApElement)) && Assigned(ApElement);
    }
    return false;
}

bool CxdBroswerCom::GetElementByName( const TCHAR *ApName, CComPtr<IHTMLElementCollection> &ApCollection )
{
    CheckInited( false );
    if ( CheckDoc3() )
    {
        BSTR bsName = _bstr_t(ApName);
        return SUCCEEDED( m_lpDoc3->getElementsByName(bsName, &ApCollection) );
    }
    return false;
}

bool CxdBroswerCom::GetElementByTagName( const TCHAR *ApName, CComPtr<IHTMLElementCollection> &ApCollection )
{
    CheckInited( false );
    if ( CheckDoc3() )
    {
        BSTR bsName = _bstr_t(ApName);
        return SUCCEEDED( m_lpDoc3->getElementsByTagName(bsName, &ApCollection) );
    }
    return false;
}

bool CxdBroswerCom::CheckAnchorElementInterface( LPDISPATCH ApDispatch, CComPtr<IHTMLAnchorElement> &ApElement )
{
    return SUCCEEDED( ApDispatch->QueryInterface(IID_IHTMLAnchorElement, (LPVOID*)&ApElement) );
}

bool CxdBroswerCom::GetFramesCollection( CComPtr<IHTMLFramesCollection2> &spFramesCollection )
{
    CheckInited( false );
    return SUCCEEDED( m_lpDoc2->get_frames(&spFramesCollection) ) && Assigned(spFramesCollection);
}

void CxdBroswerCom::InitValue()
{
    m_bInitByHandle = false;
    m_lpDoc2 = NULL;
    m_lpDoc3 = NULL;
}

CString CxdBroswerCom::GetElementInnerHTML( CComPtr<IHTMLElement> &ApElement )
{
    BSTR bStr;
    if ( SUCCEEDED(ApElement->get_innerHTML(&bStr)) )
    {
        return CString(bStr);
    }
    return TEXT("");
}

CString CxdBroswerCom::GetElementInnerText( CComPtr<IHTMLElement> &ApElement )
{
    BSTR bStr;
    if ( SUCCEEDED(ApElement->get_innerText(&bStr)) )
    {
        return CString(bStr);
    }
    return TEXT("");
}

bool CxdBroswerCom::SetElementInnerText( CComPtr<IHTMLElement> &ApElement, const TCHAR *ApText )
{
    BSTR bStr = _bstr_t( ApText );
    if ( SUCCEEDED(ApElement->put_innerText(bStr)) )
    {
        return true;
    }
    return false;
}

CString CxdBroswerCom::GetElementOuterHTML( CComPtr<IHTMLElement> &ApElement )
{
	BSTR bStr;
	if ( SUCCEEDED(ApElement->get_outerHTML(&bStr)) )
	{
		return CString(bStr);
	}
	return TEXT("");
}

CString CxdBroswerCom::GetElementOuterText( CComPtr<IHTMLElement> &ApElement )
{
	BSTR bStr;
	if ( SUCCEEDED(ApElement->get_outerText(&bStr)) )
	{
		return CString(bStr);
	}
	return TEXT("");
}

CString CxdBroswerCom::GetImgElementSrc( CComPtr<IHTMLImgElement> &ApImgElement )
{
	BSTR bStr;
	if ( SUCCEEDED(ApImgElement->get_src(&bStr)) )
	{
		return CString(bStr);
	}
	return TEXT("");
}

bool CxdBroswerCom::SetElementInnerHTML( CComPtr<IHTMLElement> &ApElement, const TCHAR *ApHtml )
{
	BSTR bStr = _bstr_t( ApHtml );
	if ( SUCCEEDED(ApElement->put_innerHTML(bStr)) )
	{
		return true;
	}
	return false;
}

CString CxdBroswerCom::GetElementStyleDisplay( CComPtr<IHTMLElement> &ApElement )
{
	CComPtr<IHTMLStyle> spStyle;
	if ( SUCCEEDED(ApElement->get_style(&spStyle)) )
	{
		BSTR bStr;
		if ( SUCCEEDED(spStyle->get_display(&bStr)) )
		{
			return CString(bStr);
		}
	}
	return TEXT("");
}

bool CxdBroswerCom::SetElementOuterHTML( CComPtr<IHTMLElement> &ApElement, const TCHAR *ApHtml )
{
	BSTR bStr = _bstr_t( ApHtml );
	if ( SUCCEEDED(ApElement->put_outerHTML(bStr)) )
	{
		return true;
	}
	return false;
}

bool CxdBroswerCom::SetElementOuterText( CComPtr<IHTMLElement> &ApElement, const TCHAR *ApText )
{
	BSTR bStr = _bstr_t( ApText );
	if ( SUCCEEDED(ApElement->put_outerText(bStr)) )
	{
		return true;
	}
	return false;
}

CString CxdBroswerCom::GetScriptElementText( CComPtr<IHTMLScriptElement> &ApScriptElement )
{
	BSTR bstr;
	if ( SUCCEEDED(ApScriptElement->get_text(&bstr)) )
	{
		return CString( bstr );
	}
	return TEXT("");
}

bool CxdBroswerCom::SetScriptElementText( CComPtr<IHTMLScriptElement> &ApScriptElement, const TCHAR *ApText )
{
	BSTR bstr = _bstr_t( ApText );
	if ( SUCCEEDED(ApScriptElement->put_text(bstr)) )
	{
		return true;
	}
	return false;
}

int CxdBroswerCom::GetFramesCount( CComPtr<IHTMLFramesCollection2> &spFramesCollection )
{
    long nCount(0);
    spFramesCollection->get_length( &nCount );
    return nCount;
}

int CxdBroswerCom::GetFramesCount( void )
{
    CComPtr<IHTMLFramesCollection2> spC;
    if ( GetFramesCollection(spC) )
    {
        return GetFramesCount( spC );
    }
    return 0;
}

bool CxdBroswerCom::GetFrameWindow( CComPtr<IHTMLFramesCollection2> &spFramesCollection, int AIndex, CComQIPtr<IHTMLWindow2> &spWin2 )
{
    CComVariant vDispWin2;
    if ( SUCCEEDED(spFramesCollection->item(&CComVariant(AIndex),   &vDispWin2)) )
    {
        spWin2 = vDispWin2.pdispVal;
        return true;
    }
    return false;
}

bool CxdBroswerCom::GetWindowDocument( CComQIPtr<IHTMLWindow2> spWin2, CComPtr<IHTMLDocument2> &ADoc )
{
    return SUCCEEDED( spWin2->get_document( &ADoc ) );
}

bool CxdBroswerCom::FindElementByTagName( CComPtr<IHTMLElementCollection> &ApCollection, CComPtr<IHTMLElement> &ApElement, const TCHAR* ApFind1, const TCHAR* ApFind2 /*= NULL*/, const TCHAR* ApFind3 /*= NULL*/ )
{
    int nCount = CxdBroswerCom::GetCollectionElementCount( ApCollection );
    for ( int i = 0; i < nCount; i++ )
    {
        LPDISPATCH lpDis;
        if ( CxdBroswerCom::GetCollectionItem(ApCollection, i, lpDis) )
        {
            CComPtr<IHTMLElement> spTemp;
            if ( CxdBroswerCom::CheckElementInterface(lpDis, spTemp) )
            {
                CString strHtml = CxdBroswerCom::GetElementOuterHTML( spTemp );
                if ( (Pos(strHtml, ApFind1, 0) >= 0) || 
                     ( (ApFind2 != NULL) && (Pos(strHtml, ApFind2, 0) >= 0)) ||
                     ( (ApFind3 != NULL) && (Pos(strHtml, ApFind3, 0) >= 0)) )
                {
                    ApElement = spTemp;
                    return true;
                }
            }
        }
    }
    return false;
}

bool CxdBroswerCom::GetFramesBroswerObject( int AIndex, CxdBroswerCom& AObject )
{
    CComPtr<IHTMLFramesCollection2> spC;
    if ( GetFramesCollection(spC) )
    {
        CComQIPtr<IHTMLWindow2> spWin2;
        CComPtr<IHTMLDocument2> spDoc;
        if ( CxdBroswerCom::GetFrameWindow(spC, AIndex, spWin2) &&
             CxdBroswerCom::GetWindowDocument(spWin2, spDoc) )
        {
            return AObject.InitBrowserCom( spDoc );
        }
    }
    return false;
}

bool CxdBroswerCom::SetElementValueByName( const TCHAR* ApName, const TCHAR* ApValue )
{
    CComPtr<IHTMLElementCollection> sp;
    if ( GetElementByName( ApName, sp ) )
    {
        CComQIPtr<IHTMLElement> it;
        CComQIPtr<IHTMLElement2> it2;
        LPDISPATCH pdis;
        int nCount = CxdBroswerCom::GetCollectionElementCount(sp);
        for ( int i = 0; i < nCount; i++ )
        {
            if ( CxdBroswerCom::GetCollectionItem(sp, i, pdis) )
            {
                it = pdis;
                it2 = pdis;
                it2->focus();
                Sleep( 10 );
                CxdBroswerCom::SetElementInnerText( it, ApValue );
            }
        }
        return true;
    }
    return false;
}

bool CxdBroswerCom::SetElementValueByID( const TCHAR* ApID, const TCHAR* ApValue )
{
    CComPtr<IHTMLElement> spID;
    if ( GetElementByID(ApID, spID) )
    {
        CComQIPtr<IHTMLElement2> spTemp = spID;
        spTemp->focus();
        Sleep(10);
        CxdBroswerCom::SetElementInnerText( spID, ApValue );
        spTemp->blur();
        return true;
    }
    return false;
}

CString CxdBroswerCom::GetInputValueByID( const TCHAR* ApID )
{
    CComPtr<IHTMLElement> sp;
    if ( GetElementByID( ApID, sp ) )
    {
        CComPtr<IHTMLInputElement> sp2;
        if ( CxdBroswerCom::CheckInputElementInterface( sp, sp2 ) )
        {
            BSTR bstr;
            if ( SUCCEEDED(sp2->get_value(&bstr)) )
            {
                CString s( bstr );
                return s;
            }
        }
    }
    return TEXT("");
}

bool CxdBroswerCom::SetElementClickByID( const TCHAR* ApID )
{
    CComPtr<IHTMLElement> it;
    if ( GetElementByID( ApID, it ) )
    {
        return SUCCEEDED( it->click() );
    }
    return false;
}



