/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdWebBrowser.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:18:54
	Description: 
**************************************************************************/
#pragma once;
#include "afxhtml.h"
#include "JxdWinControl.h"
//#include <objsafe.h>

class CxdWebBrowser;

bool DocumentLoaded(LPDISPATCH lpDocument);

#define GetWebBrowserObject(xName) \
    METHOD_PROLOGUE_EX_(CWebInterface, xName); \
    CxdWebBrowser* pView = pThis->GetView(); \
    ASSERT_VALID(pView); \
    if (pView == NULL) return E_FAIL;

class CWebInterface:public COleControlSite
{
public:
    CWebInterface(COleControlContainer *pContainer);
    virtual ~CWebInterface(void);
    CxdWebBrowser* GetView() const;

    //IDocHostUIHandler
    BEGIN_INTERFACE_PART(DocHostUIHandler, IDocHostUIHandler)
        STDMETHOD(ShowContextMenu)(DWORD, LPPOINT, LPUNKNOWN, LPDISPATCH);
        STDMETHOD(GetHostInfo)(DOCHOSTUIINFO*);
        STDMETHOD(ShowUI)(DWORD, LPOLEINPLACEACTIVEOBJECT,
            LPOLECOMMANDTARGET, LPOLEINPLACEFRAME, LPOLEINPLACEUIWINDOW);
        STDMETHOD(HideUI)(void);
        STDMETHOD(UpdateUI)(void);
        STDMETHOD(EnableModeless)(BOOL);
        STDMETHOD(OnDocWindowActivate)(BOOL);
        STDMETHOD(OnFrameWindowActivate)(BOOL);
        STDMETHOD(ResizeBorder)(LPCRECT, LPOLEINPLACEUIWINDOW, BOOL);
        STDMETHOD(TranslateAccelerator)(LPMSG, const GUID*, DWORD);
        STDMETHOD(GetOptionKeyPath)(OLECHAR **, DWORD);
        STDMETHOD(GetDropTarget)(LPDROPTARGET, LPDROPTARGET*);
        STDMETHOD(GetExternal)(LPDISPATCH*);
        STDMETHOD(TranslateUrl)(DWORD, OLECHAR*, OLECHAR **);
        STDMETHOD(FilterDataObject)(LPDATAOBJECT , LPDATAOBJECT*);
    END_INTERFACE_PART(DocHostUIHandler)

    //IDocHostShowUI
    BEGIN_INTERFACE_PART(DocHostShowUI, IDocHostShowUI)
        STDMETHOD(ShowMessage)(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult);
        STDMETHOD(ShowHelp)(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit);
    END_INTERFACE_PART(DocHostShowUI)

    //IDispatch 
    //BEGIN_INTERFACE_PART(DispatchX, IDispatch)
    //    STDMETHOD(GetTypeInfoCount)(unsigned int FAR*  pctinfo);
    //    STDMETHOD(GetTypeInfo)(unsigned int  iTInfo, LCID  lcid, ITypeInfo FAR* FAR*  ppTInfo);
    //    STDMETHOD(GetIDsOfNames)(REFIID  riid, OLECHAR FAR* FAR*  rgszNames, unsigned int cNames, LCID   lcid, DISPID FAR*  rgDispId);
    //    STDMETHOD(Invoke)(DISPID  dispIdMember, REFIID  riid, LCID  lcid, WORD  wFlags, DISPPARAMS FAR*  pDispParams, VARIANT FAR*  pVarResult, EXCEPINFO FAR*  pExcepInfo,unsigned int FAR*  puArgErr);
    //END_INTERFACE_PART(DispatchX)

// 	BEGIN_INTERFACE_PART(ObjectSafety, IObjectSafety)
// 		STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD __RPC_FAR *pdwSupportedOptions, DWORD __RPC_FAR *pdwEnabledOptions);
// 		STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);
// 	END_INTERFACE_PART(ObjectSafety)

    DECLARE_INTERFACE_MAP();
};

class CxdWebBrowser: public CxdWinControl<CHtmlView>
{
public:
    CxdWebBrowser();
    ~CxdWebBrowser();

    BOOL CreateControlSite(COleControlContainer* pContainer, COleControlSite** ppSite, UINT nID, REFCLSID clsid);

    CtConst xdGetClassName(void) { return CtxdWebBrowser; }
    bool xdCreate(const int &Ax, const int &Ay, const int &AWidth, const int &AHeight, CxdUIComponent *ApParent = NULL, const TCHAR *ApWindowName = NULL);
    int GetWebBrowserID(void) { return CtWebBrowserID; }
	CString GetCookies(void);
	bool    SetCookies(const TCHAR *ApCookies);

    BOOL GetIsCanForward(void) { return m_bIsCanForward; }
    BOOL GetIsCanGoBack(void) { return m_bIsCanGoBack; }

    bool GetWeb3DBorder(void) { return m_bWeb3DBorder; }
    void SetWeb3DBorder(bool bValue) { m_bWeb3DBorder = bValue; }

    bool GetWebScroll(void) { return m_bWebScroll; }
    void SetWebScroll(bool bValue) { m_bWebScroll = bValue; }

    bool GetWebTextSelection(void) { return m_bWebTextSelection; }
    void SetWebTextSelection(bool bValue) { m_bWebTextSelection = bValue; }

    bool GetDisplayTheme(void) { return m_bDisplayTheme; }
    bool SetDisplayTheme(bool bValue) { m_bDisplayTheme = bValue; }
protected:
    virtual LRESULT WindowProc(UINT nMessage, WPARAM wParam, LPARAM lParam);
    void OnGetCreateInfo(CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle) {}
    bool OnUIStateChanged(const TxdGpUIState &ANewSate){ return CxdUIComponent::OnUIStateChanged(ANewSate); }

    DECLARE_DYNAMIC(CxdWebBrowser)
    DECLARE_MESSAGE_MAP()

    afx_msg void OnSize(UINT nType, int ANewWidth, int ANewHeight);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest,UINT message) { return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message); }
    void OnDestroy() { m_pBrowserApp = NULL; CWnd::OnDestroy(); }

    void OnCommandStateChange(long nCommand,BOOL bEnable);
    void OnNavigateComplete2(LPCTSTR strURL);

    //IDocHostUIHandler
    HRESULT OnGetHostInfo(DOCHOSTUIINFO *pInfo);

    //IDocHostShowUI
    virtual STDMETHODIMP OnShowMessage(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult);
    STDMETHODIMP OnShowHelp(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit);

private:
    const int CtWebBrowserID;
    BOOL m_bIsCanGoBack;
    BOOL m_bIsCanForward;
    bool m_bWeb3DBorder;
    bool m_bWebScroll;
    bool m_bWebTextSelection;
    bool m_bDisplayTheme;

protected:
    //HOOK WM_ERASEBKGND
    class CxdHookEraseBkgnd: public CWnd
    {
    protected:
        DECLARE_MESSAGE_MAP() 
        afx_msg BOOL OnEraseBkgnd(CDC* pDC) { return TRUE; }
    };
    bool m_bHookEraseBK;
    CxdHookEraseBkgnd m_HookEraseBK;
    void HookWebEraseBK(void);

    friend class CWebInterface;
};

struct TOnUrlParam
{
	CString FpURL;
	CxdWebBrowser *FpObject;
};