/**************************************************************************
Copyright  : pp.cc ����ָ��ʱ�� ɺ����
File       : iecom.h & .cpp 
Author     : Terry
Email      : jxd524@163.com
CreateTime : 2008-1-8
����޸�   : 2012-6-28 09:49
Description: ����ҳ�Ĳ���

CComPtr��CComQIPtr�Ĳ�ͬ�ĵط���
    CComPtrֻ�ܴ����̶����ض��Ľӿ�ָ��ʵ����
    ��CComQIPtr����ʵ����CComPtr�����еĹ��ܣ����ҵ����ǰ�һ����ͬ���͵Ľӿ�ָ�븳ֵ��CComQIPtr��ʱ��
    CComQIPtr���Զ��ĵ��ýӿ�ָ���QueryInterface�ӿڣ�����ö�Ӧ����ȷ�Ľӿ�ָ�롣
**************************************************************************/

#pragma once;
#include <afxhtml.h>

enum TElementCollectionStyle { ecAll, ecBody, ecForms, ecImages, ecScriptes };

class CxdBroswerCom
{
public:
    CxdBroswerCom(void);
    CxdBroswerCom(LPDISPATCH ApDocument);
    CxdBroswerCom(const HWND &AWndHandle);
    ~CxdBroswerCom();

    void InitValue();
    bool InitBrowserCom(LPDISPATCH ApDocument);
    bool InitBrowserCom(const HWND &AWndHandle);

    //title
    bool GetTitle(CString &ATitle);
    bool SetTitle(const TCHAR *ApTitle);

    //htmlԴ��
    bool GetHtml(CString &AHtmlText);

    //Element ����
    bool SetElementClickByID(const TCHAR* ApID);
    bool SetElementValueByName( const TCHAR* ApName, const TCHAR* ApValue );
    bool SetElementValueByID( const TCHAR* ApID, const TCHAR* ApValue );

    //Input Element ����
    CString GetInputValueByID( const TCHAR* ApID );

    //find
    bool GetElementByID(const TCHAR *ApID, CComPtr<IHTMLElement> &ApElement);
    bool GetElementByName(const TCHAR *ApName, CComPtr<IHTMLElementCollection> &ApCollection);
    bool GetElementByTagName(const TCHAR *ApName, CComPtr<IHTMLElementCollection> &ApCollection);

    bool GetDocumentElement(CComPtr<IHTMLElement> &ApRootElement);
    //���ĵ��еõ��������͵ļ��� IHTMLElementCollection
    bool GetElementCollection(const TElementCollectionStyle &tECtype, CComPtr<IHTMLElementCollection> &spElemCollection); 

    //Frames ���
    int  GetFramesCount(void);
    bool GetFramesBroswerObject(int AIndex, CxdBroswerCom& AObject);
    bool GetFramesCollection(CComPtr<IHTMLFramesCollection2> &spFramesCollection);
    static int  GetFramesCount(CComPtr<IHTMLFramesCollection2> &spFramesCollection);
    static bool GetFrameWindow(CComPtr<IHTMLFramesCollection2> &spFramesCollection, int AIndex, CComQIPtr<IHTMLWindow2> &spWin2);
    static bool GetWindowDocument(CComQIPtr<IHTMLWindow2> spWin2, CComPtr<IHTMLDocument2> &ADoc);


    //IHTMLElementCollection��ز���
    static int  GetCollectionElementCount(CComPtr<IHTMLElementCollection> &ApCollection);
    static bool GetCollectionItem(CComPtr<IHTMLElementCollection> &ApCollection, const int &AIndex, LPDISPATCH &ApItem);

    //CComPtr<IHTMLElement> ��ز���
    static CString GetElementInnerHTML(CComPtr<IHTMLElement> &ApElement);
	static bool    SetElementInnerHTML(CComPtr<IHTMLElement> &ApElement, const TCHAR *ApHtml);
    static CString GetElementInnerText(CComPtr<IHTMLElement> &ApElement);
    static bool    SetElementInnerText(CComPtr<IHTMLElement> &ApElement, const TCHAR *ApText);
	static CString GetElementOuterHTML(CComPtr<IHTMLElement> &ApElement);
	static bool    SetElementOuterHTML(CComPtr<IHTMLElement> &ApElement, const  TCHAR *ApHtml);
	static CString GetElementOuterText(CComPtr<IHTMLElement> &ApElement);
	static bool    SetElementOuterText(CComPtr<IHTMLElement> &ApElement, const TCHAR *ApText);
	static CString GetImgElementSrc(CComPtr<IHTMLImgElement> &ApImgElement);
	static CString GetElementStyleDisplay(CComPtr<IHTMLElement> &ApElement);
	static CString GetScriptElementText(CComPtr<IHTMLScriptElement> &ApScriptElement);
	static bool    SetScriptElementText(CComPtr<IHTMLScriptElement> &ApScriptElement, const TCHAR *ApText);

    //��ѯ�ӿ�, ��ȡ��Ӧ�ӿ�ָ��
    static bool CheckFormInterface(LPDISPATCH ApDispatch, CComPtr<IHTMLFormElement> &ApForm);
    static bool CheckElementInterface(LPDISPATCH ApDispatch, CComPtr<IHTMLElement> &ApElement);
    static bool CheckElement2Interface(LPDISPATCH ApDispatch, CComPtr<IHTMLElement2> &ApElement);
    static bool CheckElement3Interface(LPDISPATCH ApDispatch, CComPtr<IHTMLElement3> &ApElement);    
    static bool CheckInputElementInterface(LPDISPATCH ApDispatch, CComPtr<IHTMLInputElement> &ApElement);
    static bool CheckImgElementInterface(LPDISPATCH ApDispatch, CComPtr<IHTMLImgElement> &ApElement);
    static bool CheckLinkElementInterface(LPDISPATCH ApDispatch, CComPtr<IHTMLLinkElement> &ApElement);
    static bool CheckScriptElementInterface(LPDISPATCH ApDispatch, CComPtr<IHTMLScriptElement> &ApElement);
    static bool CheckAnchorElementInterface(LPDISPATCH ApDispatch, CComPtr<IHTMLAnchorElement> &ApElement);

    //����Ԫ��
    static bool FindElementByTagName(CComPtr<IHTMLElementCollection> &ApCollection, CComPtr<IHTMLElement> &ApElement,
        const TCHAR* ApFind1, const TCHAR* ApFind2 = NULL, const TCHAR* ApFind3 = NULL);
protected:
    CComPtr<IHTMLDocument2> m_lpDoc2;
    CComPtr<IHTMLDocument3> m_lpDoc3;
    bool CheckDoc3(void);
    inline bool IsInited(void) { return m_lpDoc2 != NULL; }
private:
    bool m_bInitByHandle;
};

/*

void EnumForm(IHTMLDocument2* pIHTMLDocument2);
void EnumFrame(IHTMLDocument2* pIHTMLDocument2)   
{   
    if ( !pIHTMLDocument2 )	return;
    HRESULT hr;
    CComPtr<IHTMLFramesCollection2> spFramesCollection2;
    pIHTMLDocument2->get_frames( &spFramesCollection2 );	//ȡ�ÿ��frame�ļ���   

    long nFrameCount=0;	 //ȡ���ӿ�ܸ���   
    hr = spFramesCollection2->get_length( &nFrameCount );   
    if ( FAILED(hr) || 0 == nFrameCount ) return;   

    for(long i=0; i<nFrameCount; i++)   
    {   
        CComVariant   vDispWin2;	 //ȡ���ӿ�ܵ��Զ����ӿ�   
        hr = spFramesCollection2->item(   &CComVariant(i),   &vDispWin2   );   
        if (FAILED(hr))	continue;

        CComQIPtr<IHTMLWindow2> spWin2 = vDispWin2.pdispVal;   
        CComQIPtr<IHTMLWindow4> spWin4 = vDispWin2.pdispVal;   

        if( !spWin2 ) continue;	//ȡ���ӿ�ܵ�   IHTMLWindow2   �ӿ�   
        if( !spWin4 ) continue;   
        CComQIPtr<IHTMLFrameBase> fbase;   
        hr=spWin4->get_frameElement(&fbase);//fbase��Զ��NULL   

        CComPtr<IHTMLDocument2> spDoc2;   
        spWin2->get_document(&spDoc2);	//ȡ���ֿ�ܵ�   IHTMLDocument2   �ӿ�   

        EnumForm( spDoc2 );	 //�ݹ�ö�ٵ�ǰ�ӿ��   IHTMLDocument2   �ϵı�form   
    }   
}   

void EnumForm(IHTMLDocument2* pIHTMLDocument2)   
{   
    if( !pIHTMLDocument2) return;   
    EnumFrame(pIHTMLDocument2);	//�ݹ�ö�ٵ�ǰ   IHTMLDocument2   �ϵ��ӿ��fram   
    HRESULT   hr;
    CComBSTR   bstrTitle;
    pIHTMLDocument2->get_title(&bstrTitle);	//ȡ���ĵ�����   

    CComQIPtr<IHTMLElementCollection> spElementCollection; 
    hr = pIHTMLDocument2->get_forms(&spElementCollection);	//ȡ�ñ�����   
    if ( FAILED(hr) ) return;

    long nFormCount=0;	 //ȡ�ñ���Ŀ   
    hr = spElementCollection->get_length(&nFormCount);   
    if(FAILED(hr)) return;
    
    for(long i=0; i<nFormCount; i++)   
    {   
        IDispatch *pDisp   =   NULL;	//ȡ�õ�   i   ���   
        hr = spElementCollection->item(CComVariant(i), CComVariant(), &pDisp);   
        if( FAILED(hr) ) continue;   

        CComQIPtr<IHTMLFormElement> spFormElement = pDisp;   
        pDisp->Release();   

        long  nElemCount=0;	 //ȡ�ñ���   ��   ����Ŀ   
        hr = spFormElement->get_length( &nElemCount );   
        if( FAILED(hr) ) continue;   

        for( long j=0; j < nElemCount; j++ )   
        {
            CComDispatchDriver spInputElement;	//ȡ�õ�   j   �����   
            hr = spFormElement->item( CComVariant(j), CComVariant(), &spInputElement );   
            if( FAILED(hr) ) continue;   

            CComVariant  vName, vVal, vType;	 //ȡ�ñ����   ����ֵ������   
            hr = spInputElement.GetPropertyByName(L"name", &vName);   
            if( FAILED(hr) )	continue;  

            hr = spInputElement.GetPropertyByName(L"value", &vVal );   
            if(FAILED(hr))	continue;  

            hr = spInputElement.GetPropertyByName(L"type", &vType );   
            if(FAILED(hr)) 	continue;   

        }   
        //���ύ�������ɾ����������ע�Ͱ�   
        //pForm->submit();   
    }   
}

IHTMLChangeSink: �ӿ�, ����ҳ�б仯ʱ, Notify������������

IHTMLChangeLog* ChangeLog;
CComQIPtr<IMarkupContainer2> spMarkup;
DWORD dwCookie;
spMarkup = m_pWebBrowser->GetHtmlDocument();
CMyTest *pTest = new CMyTest(this);
if ( SUCCEEDED(spMarkup->CreateChangeLog( pTest, &ChangeLog, 1, 1 )) )
{
    if ( SUCCEEDED(spMarkup->RegisterForDirtyRange( pTest, &dwCookie ) ) )
    {
        OutputDebugString( TEXT("Success") );
    }
}


class CMyTest: public IHTMLChangeSink
{
public:
    CMyTest(CfrmLogin* ApOwner) { m_cRef= 0; p = ApOwner; }
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
    {

        if (riid == IID_IUnknown)
            *ppvObject = (IUnknown*)this;
        else if ( riid == IID_IHTMLChangeSink )
            *ppvObject = (IHTMLChangeSink*)this;
        else
            return E_NOINTERFACE;

        ((IUnknown *) *ppvObject)->AddRef();

        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return ++m_cRef;
    }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG cRef = --m_cRef;

        if (cRef == 0)
            delete this;

        return cRef;
    }

    virtual HRESULT STDMETHODCALLTYPE Notify( void)
    {
        if ( ���ٽ���֪ͨ )
        {
            if( SUCCEEDED(spMarkup->UnRegisterForDirtyRange( dwCookie )) )
            {
                ChangeLog->Release();
                OutputDebugString( TEXT("SJDFlk") );
             }
        }
        return S_OK;
    }
private:
    ULONG m_cRef;                 // COM reference count
    CfrmLogin *p;
};

*/