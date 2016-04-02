#include "stdafx.h"
#include "JxdWebBrowser.h"
#include "mshtmdid.h"
#include "JxdDebugInfo.h"

IMPLEMENT_DYNAMIC( CxdWebBrowser, CHtmlView )
BEGIN_MESSAGE_MAP( CxdWebBrowser, CHtmlView )
    ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CxdWebBrowser::CxdHookEraseBkgnd, CWnd)   
    ON_WM_ERASEBKGND()       
END_MESSAGE_MAP()  

BEGIN_INTERFACE_MAP(CWebInterface, COleControlSite)
    INTERFACE_PART(CWebInterface, IID_IDocHostShowUI, DocHostShowUI)
    INTERFACE_PART(CWebInterface, IID_IDocHostUIHandler, DocHostUIHandler)
	//INTERFACE_PART(CWebInterface, IID_IObjectSafety, ObjectSafety)
    //INTERFACE_PART(CWebInterface, IID_IDispatch, DispatchX)
END_INTERFACE_MAP()


bool DocumentLoaded( LPDISPATCH lpDocument )
{
    if ( Assigned(lpDocument) )
    {
        IHTMLDocument2 *lpDoc;
        return SUCCEEDED( lpDocument->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&lpDoc) );
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////                                          CWebInterface
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CWebInterface::CWebInterface(COleControlContainer *pContainer): COleControlSite(pContainer)
{
}

CWebInterface::~CWebInterface(void)
{
}

inline CxdWebBrowser* CWebInterface::GetView() const
{
    return STATIC_DOWNCAST(CxdWebBrowser, m_pCtrlCont->m_pWnd);
}

//IDocHostUIHandler
STDMETHODIMP_(ULONG) CWebInterface::XDocHostUIHandler::AddRef()
{
    METHOD_PROLOGUE_EX_(CWebInterface, DocHostUIHandler)
    return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CWebInterface::XDocHostUIHandler::Release()
{
    METHOD_PROLOGUE_EX_(CWebInterface, DocHostUIHandler)
    return pThis->ExternalRelease();
}

STDMETHODIMP CWebInterface::XDocHostUIHandler::QueryInterface( REFIID iid, LPVOID far* ppvObj)     
{
    METHOD_PROLOGUE_EX_(CWebInterface, DocHostUIHandler)
    return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CWebInterface::XDocHostUIHandler::GetExternal(LPDISPATCH *lppDispatch)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnGetExternal(lppDispatch);
}

STDMETHODIMP CWebInterface::XDocHostUIHandler::ShowContextMenu(DWORD dwID, LPPOINT ppt, LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
}


STDMETHODIMP CWebInterface::XDocHostUIHandler::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnGetHostInfo(pInfo);
}

STDMETHODIMP CWebInterface::XDocHostUIHandler::ShowUI( DWORD dwID, LPOLEINPLACEACTIVEOBJECT pActiveObject,
    LPOLECOMMANDTARGET pCommandTarget, LPOLEINPLACEFRAME pFrame, LPOLEINPLACEUIWINDOW pDoc)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc);
}

STDMETHODIMP CWebInterface::XDocHostUIHandler::HideUI(void)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnHideUI();
}

STDMETHODIMP CWebInterface::XDocHostUIHandler::UpdateUI(void)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnUpdateUI();
}


STDMETHODIMP CWebInterface::XDocHostUIHandler::EnableModeless(BOOL fEnable)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnEnableModeless(fEnable);
}


STDMETHODIMP CWebInterface::XDocHostUIHandler::OnDocWindowActivate(BOOL fActivate)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnDocWindowActivate(fActivate);
}


STDMETHODIMP CWebInterface::XDocHostUIHandler::OnFrameWindowActivate(
    BOOL fActivate)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnFrameWindowActivate(fActivate);
}

STDMETHODIMP CWebInterface::XDocHostUIHandler::ResizeBorder(
    LPCRECT prcBorder, LPOLEINPLACEUIWINDOW pUIWindow, BOOL fFrameWindow)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnResizeBorder(prcBorder, pUIWindow, fFrameWindow);
}


STDMETHODIMP CWebInterface::XDocHostUIHandler::TranslateAccelerator(
    LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnTranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
}

STDMETHODIMP CWebInterface::XDocHostUIHandler::GetOptionKeyPath(
    LPOLESTR* pchKey, DWORD dwReserved)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnGetOptionKeyPath(pchKey, dwReserved);
}


STDMETHODIMP CWebInterface::XDocHostUIHandler::GetDropTarget(
    LPDROPTARGET pDropTarget, LPDROPTARGET* ppDropTarget)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnGetDropTarget(pDropTarget, ppDropTarget);
}


STDMETHODIMP CWebInterface::XDocHostUIHandler::TranslateUrl(
    DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnTranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
}


STDMETHODIMP CWebInterface::XDocHostUIHandler::FilterDataObject(
    LPDATAOBJECT pDataObject, LPDATAOBJECT* ppDataObject)
{
    GetWebBrowserObject( DocHostUIHandler );
    return pView->OnFilterDataObject(pDataObject, ppDataObject);
}

//IDocHostShowUI
unsigned long __stdcall CWebInterface::XDocHostShowUI::AddRef(void)
{
    METHOD_PROLOGUE_EX_(CWebInterface, DocHostShowUI);
    return pThis->ExternalAddRef();
}

unsigned long __stdcall CWebInterface::XDocHostShowUI::Release(void)
{
    METHOD_PROLOGUE_EX_(CWebInterface, DocHostShowUI);
    return pThis->ExternalRelease();
}

long __stdcall CWebInterface::XDocHostShowUI::QueryInterface(struct _GUID const &iid,LPVOID* ppvObj)
{
    METHOD_PROLOGUE_EX_(CWebInterface, DocHostShowUI);
    return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CWebInterface::XDocHostShowUI::ShowMessage( HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult )
{
    GetWebBrowserObject( DocHostShowUI );
    return pView->OnShowMessage(hwnd, lpstrText, lpstrCaption, dwType, lpstrHelpFile, dwHelpContext, plResult);
}

STDMETHODIMP CWebInterface::XDocHostShowUI::ShowHelp( HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit )
{
    GetWebBrowserObject( DocHostShowUI );
    return pView->OnShowHelp( hwnd, pszHelpFile, uCommand, dwData, ptMouse, pDispatchObjectHit );
}

//IDispatch
//unsigned long __stdcall CWebInterface::XDispatchX::AddRef(void)
//{
//    METHOD_PROLOGUE_EX_(CWebInterface, DispatchX);
//    return pThis->ExternalAddRef();
//}
//
//unsigned long __stdcall CWebInterface::XDispatchX::Release(void)
//{
//    METHOD_PROLOGUE_EX_(CWebInterface, DispatchX);
//    return pThis->ExternalRelease();
//}
//
//long __stdcall CWebInterface::XDispatchX::QueryInterface(struct _GUID const &iid,LPVOID* ppvObj)
//{
//    METHOD_PROLOGUE_EX_(CWebInterface, DispatchX);
//    return pThis->ExternalQueryInterface(&iid, ppvObj);
//}
//
//STDMETHODIMP CWebInterface::XDispatchX::GetTypeInfoCount( unsigned int FAR* pctinfo )
//{
//    return S_OK;
//}
//
//STDMETHODIMP CWebInterface::XDispatchX::GetTypeInfo( unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo )
//{
//    return S_OK;
//}
//
//STDMETHODIMP CWebInterface::XDispatchX::GetIDsOfNames( REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, 
//    LCID lcid, DISPID FAR* rgDispId )
//{
//    return S_OK;
//}
//
//STDMETHODIMP CWebInterface::XDispatchX::Invoke( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, 
//    DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo,unsigned int FAR* puArgErr )
//{
//    switch (dispIdMember)
//
//    {
//
//    case DISPID_AMBIENT_DLCONTROL:
//
//        pVarResult->vt = VT_I4;
//
//        pVarResult->lVal = DLCTL_DLIMAGES | DLCTL_VIDEOS | DLCTL_NO_SCRIPTS;
//
//        break;
//
//    default:
//
//        return DISP_E_MEMBERNOTFOUND;
//
//    }
//
//    return S_OK;
//
//
//    ////GetWebBrowserObject( DispatchX );
//    //if ( (wFlags & DISPATCH_PROPERTYGET) != 0 && pVarResult != NULL )
//    //{
//    //    if ( dispIdMember == DISPID_AMBIENT_DLCONTROL )
//    //    {
//    //        _variant_t l(624);
//    //        *pVarResult = l;
//    //    }
//    //}
//    ////return CCmdTarget::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr );
//    //return S_OK;
//}

//ObjectSafety
// STDMETHODIMP CWebInterface::XObjectSafety::GetInterfaceSafetyOptions(
// 	REFIID riid, 
// 	DWORD __RPC_FAR *pdwSupportedOptions, 
// 	DWORD __RPC_FAR *pdwEnabledOptions)
// {
// 	METHOD_PROLOGUE_EX(CWebInterface, ObjectSafety)
// 
// 		if (!pdwSupportedOptions || !pdwEnabledOptions)
// 		{
// 			return E_POINTER;
// 		}
// 
// 		*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
// 		*pdwEnabledOptions = 0;
// 
// 		if (NULL == pThis->GetInterface(&riid))
// 		{
// 			TRACE("Requested interface is not supported.\n");
// 			return E_NOINTERFACE;
// 		}
// 
// 		// What interface is being checked out anyhow?
// 		OLECHAR szGUID[39];
// 		int i = StringFromGUID2(riid, szGUID, 39);
// 
// 		if (riid == IID_IDispatch)
// 		{
// 			// Client wants to know if object is safe for scripting
// 			*pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
// 			return S_OK;
// 		}
// 		else if (riid == IID_IPersistPropertyBag 
// 			|| riid == IID_IPersistStreamInit
// 			|| riid == IID_IPersistStorage
// 			|| riid == IID_IPersistMemory)
// 		{
// 			// Those are the persistence interfaces COleControl derived controls support
// 			// as indicated in AFXCTL.H
// 			// Client wants to know if object is safe for initializing from persistent data
// 			*pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
// 			return S_OK;
// 		}
// 		else
// 		{
// 			// Find out what interface this is, and decide what options to enable
// 			TRACE("We didn't account for the safety of this interface, and it's one we support...\n");
// 			return E_NOINTERFACE;
// 		} 
// }
// 
// STDMETHODIMP CWebInterface::XObjectSafety::SetInterfaceSafetyOptions(
// 	REFIID riid, 
// 	DWORD dwOptionSetMask, 
// 	DWORD dwEnabledOptions)
// {
// 	METHOD_PROLOGUE_EX(CWebInterface, ObjectSafety)
// 
// 		OLECHAR szGUID[39];
// 	// What is this interface anyway?
// 	// We can do a quick lookup in the registry under HKEY_CLASSES_ROOT\Interface
// 	int i = StringFromGUID2(riid, szGUID, 39);
// 
// 	if (0 == dwOptionSetMask && 0 == dwEnabledOptions)
// 	{
// 		// the control certainly supports NO requests through the specified interface
// 		// so it's safe to return S_OK even if the interface isn't supported.
// 		return S_OK;
// 	}
// 
// 	// Do we support the specified interface?
// 	if (NULL == pThis->GetInterface(&riid))
// 	{
// 		TRACE1("%s is not support.\n", szGUID);
// 		return E_FAIL;
// 	}
// 
// 
// 	if (riid == IID_IDispatch)
// 	{
// 		TRACE("Client asking if it's safe to call through IDispatch.\n");
// 		TRACE("In other words, is the control safe for scripting?\n");
// 		if (INTERFACESAFE_FOR_UNTRUSTED_CALLER == dwOptionSetMask && INTERFACESAFE_FOR_UNTRUSTED_CALLER == dwEnabledOptions)
// 		{
// 			return S_OK;
// 		}
// 		else
// 		{
// 			return E_FAIL;
// 		}
// 	}
// 	else if (riid == IID_IPersistPropertyBag 
// 		|| riid == IID_IPersistStreamInit
// 		|| riid == IID_IPersistStorage
// 		|| riid == IID_IPersistMemory)
// 	{
// 		TRACE("Client asking if it's safe to call through IPersist*.\n");
// 		TRACE("In other words, is the control safe for initializing from persistent data?\n");
// 
// 		if (INTERFACESAFE_FOR_UNTRUSTED_DATA == dwOptionSetMask && INTERFACESAFE_FOR_UNTRUSTED_DATA == dwEnabledOptions)
// 		{
// 			return NOERROR;
// 		}
// 		else
// 		{
// 			return E_FAIL;
// 		}
// 	}
// 	else
// 	{
// 		TRACE1("We didn't account for the safety of %s, and it's one we support...\n", szGUID);
// 		return E_FAIL;
// 	}
// }
// 
// STDMETHODIMP_(ULONG) CWebInterface::XObjectSafety::AddRef()
// {
// 	METHOD_PROLOGUE_EX_(CWebInterface, ObjectSafety)
// 		return (ULONG)pThis->ExternalAddRef();
// }
// 
// STDMETHODIMP_(ULONG) CWebInterface::XObjectSafety::Release()
// {
// 	METHOD_PROLOGUE_EX_(CWebInterface, ObjectSafety)
// 		return (ULONG)pThis->ExternalRelease();
// }
// 
// STDMETHODIMP CWebInterface::XObjectSafety::QueryInterface(
// 	REFIID iid, LPVOID* ppvObj)
// {
// 	METHOD_PROLOGUE_EX_(CWebInterface, ObjectSafety)
// 		return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
// }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////                                          CxdWebBrowser
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CxdWebBrowser::CxdWebBrowser(): CxdWinControl<CHtmlView>(), CtWebBrowserID( GetUnionID() )
{
    m_bIsCanGoBack = FALSE;
    m_bIsCanForward = FALSE;
    m_bHookEraseBK = false;
    m_bWeb3DBorder = false;
    m_bWebScroll = true;
    m_bWebTextSelection = true;
    m_bDisplayTheme = true;
}

CxdWebBrowser::~CxdWebBrowser()
{
}

bool CxdWebBrowser::xdCreate( const int &Ax, const int &Ay, const int &AWidth, const int &AHeight, CxdUIComponent *ApParent /*= NULL*/, const TCHAR *ApWindowName /*= NULL*/ )
{
    if ( !Assigned(ApParent) || !ApParent->IsWndObject() )
    {
        return false;
    }
    m_pParent = ApParent;
    CWnd *pObj = dynamic_cast<CWnd*>(ApParent);

    Position()->SetLeft( Ax );
    Position()->SetTop( Ay );
    if ( AWidth > 0 ) Position()->SetWidth( AWidth );
    if ( AHeight > 0 ) Position()->SetHeight( AHeight );
    bool bOK = TRUE == Create(NULL, NULL, WS_CHILD | WS_VISIBLE, Position()->GetCRect(), pObj, CtWebBrowserID);
    if ( bOK )
    {
        m_bCreated = true;
        if ( Assigned(m_pParent) )
        {
            m_pParent->AddSubComponent( this );
        }
        CheckAnchors( NULL );
        OnCreated();
    }
    return bOK;
}

CString CxdWebBrowser::GetCookies( void )
{
	LPDISPATCH lpDoc = GetHtmlDocument();
	if ( DocumentLoaded(lpDoc) )
	{
		CComPtr<IHTMLDocument2> spHtml;
		if (SUCCEEDED(lpDoc->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&spHtml)) )
		{
			CComBSTR v;
			spHtml->get_cookie( &v );
			return CString( v );
		}
	}
	return TEXT("");
}

bool CxdWebBrowser::SetCookies( const TCHAR *ApCookies )
{
	LPDISPATCH lpDoc = GetHtmlDocument();
	if ( DocumentLoaded(lpDoc) )
	{
		CComPtr<IHTMLDocument2> spHtml;
		if ( SUCCEEDED(lpDoc->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&spHtml)) )
		{
			BSTR v = _bstr_t( ApCookies );
			spHtml->put_cookie( v );
			return true;
		}
	}
	return false;
}

void CxdWebBrowser::HookWebEraseBK( void )
{
    if( m_HookEraseBK.GetSafeHwnd() == NULL )
    {   
        CWnd *pWnd=GetWindow( GW_CHILD );   
        while(pWnd!=NULL)   
        {
            TCHAR szClass[MAX_PATH]={0};   
            GetClassName( pWnd->GetSafeHwnd(), szClass, MAX_PATH - 1 );     
            if( _tcscmp(szClass,TEXT("Internet Explorer_Server") ) == 0 )   
            {   
                m_HookEraseBK.SubclassWindow( pWnd->GetSafeHwnd() );   
                break;    
            }   
            pWnd=pWnd->GetWindow( GW_CHILD );   
        }   
    }
    m_bHookEraseBK = m_HookEraseBK.GetSafeHwnd() != NULL;
}

void CxdWebBrowser::OnCommandStateChange( long nCommand, BOOL bEnable )
{
    switch ( nCommand )
    {
    case CSC_NAVIGATEBACK:
        m_bIsCanGoBack = bEnable;
    	break;
    case CSC_NAVIGATEFORWARD:
        m_bIsCanForward = bEnable;
        break;
    }
}

void CxdWebBrowser::OnSize( UINT nType, int ANewWidth, int ANewHeight )
{
    //解决拖动时警告信息
    CFormView::OnSize(nType, ANewWidth, ANewHeight);
    if (::IsWindow(m_wndBrowser.m_hWnd)) 
    { 
        CRect rect; 
        GetClientRect(rect); 
        //::AdjustWindowRectEx( rect, GetStyle(), FALSE, WS_EX_CLIENTEDGE );
        m_wndBrowser.SetWindowPos( NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER );
    } 
}

LRESULT CxdWebBrowser::WindowProc( UINT nMessage, WPARAM wParam, LPARAM lParam )
{
    switch ( nMessage )
    {
    case WM_PAINT:
    case WM_SIZE:
        return CFormView::WindowProc(nMessage, wParam, lParam );
    	break;
    case WM_ERASEBKGND:
        return TRUE;
        break;
    }
    return CxdWinControl<CHtmlView>::WindowProc(nMessage, wParam, lParam );
}

HRESULT CxdWebBrowser::OnGetHostInfo( DOCHOSTUIINFO *pInfo )
{
    return S_OK;
#define CheckStyle(b, style)\
    b ? pInfo->dwFlags ^= pInfo->dwFlags & style : pInfo->dwFlags |= style

    CheckStyle( m_bWeb3DBorder, DOCHOSTUIFLAG_NO3DBORDER );
    CheckStyle( m_bWebScroll, DOCHOSTUIFLAG_SCROLL_NO );
    CheckStyle( m_bWebTextSelection, DOCHOSTUIFLAG_DIALOG );
    
    if ( m_bDisplayTheme )
    {
        pInfo->dwFlags &= ~DOCHOSTUIFLAG_NOTHEME;
        pInfo->dwFlags |=  DOCHOSTUIFLAG_THEME;
    }

    return S_OK;
}

BOOL CxdWebBrowser::CreateControlSite(COleControlContainer* pContainer, COleControlSite** ppSite, UINT nID, REFCLSID clsid)
{
    *ppSite = new CWebInterface(pContainer);
    return TRUE;
}

STDMETHODIMP CxdWebBrowser::OnShowMessage( HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult )
{
    return S_FALSE;
}

STDMETHODIMP CxdWebBrowser::OnShowHelp( HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit )
{
    return S_FALSE;
}

void CxdWebBrowser::OnNavigateComplete2( LPCTSTR strURL )
{
    if ( !m_bHookEraseBK) HookWebEraseBK(); 
}
