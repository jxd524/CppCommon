/*
CNotifyMsgEvent OnDragInfoEvent; 
    WPARAM: TDragStyle ����
        dsString: ��ʱLPARAM������һ�� CString *
        dsDIB����ʱ LPARAM ������ LPBITMAPINFO

*/
#pragma once

#include "afxole.h"
#include <vector>
using namespace std;

#include "JxdMacro.h"
#include "JxdEvent.h"

enum TDragStyle{ dsString, dsDIB, dsHTML, dsFileName };
enum TDropHtmlStyle{ dhsString, dhsImgURL };

class CxdDropTarget;
typedef const TCHAR* const CtConst;
extern CtConst CtFileHead;

struct TDropHtmlInfo 
{
    TDropHtmlStyle FStyle;
    CString FInfo;
};

struct TClipInfo 
{
    UINT   FFormat;
    HANDLE FMemHandle;
};

typedef vector< TDropHtmlInfo* > TDropHtmlInfoList;
typedef IxdNotifyEventExT<int/*λ��X*/, int/*λ��Y*/ > IxdDragScroll;
typedef IxdNotifyEventExT<TDragStyle, LPVOID > IxdDragInfo;

class CxdParseDropHtml
{
public:
    bool ParseDropHtml(const char *ApUtfDropText);
    TDropHtmlInfoList m_lsDropHtmlInfo;
    bool IsHasImageURL(void);
    ~CxdParseDropHtml();
private:
    void ClearDropHtmlInfo(void);
    void ReplaceHtmlInfo(CString &AInfo);
private:
    CString m_strLineEnd;
};

class CxdDropTarget :public COleDropTarget
{
public:
    CxdDropTarget(void);
    ~CxdDropTarget(void);

    void ParseClipInfo(COleDataObject *pDataObject);
    void HanldeClipFormatInfo(COleDataObject *pDataObject);

    PropertyInterface( DragInfo,  IxdDragInfo );
    PropertyInterface( DragScroll, IxdDragScroll );
protected: 
    //ʵ�ָ��෽��
    DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
    DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
    void OnDragLeave(CWnd* pWnd);
    DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point);

    //debug
    void EnumOleDataInfo(COleDataObject *pDataObject);

    //Clipborad handle
    vector< TClipInfo* > m_ClipInfoList;
    void ClearClipInfoList(void);
    void SaveClipInfo(void);
    void RestorClipInfo(void);

    //handle clip data
    BOOL HandleClipData(UINT AFromat, COleDataObject *pDataObject);
    void DoHandleClipData(COleDataObject *pDataObject, UINT AFormat, LPVOID ADataBuf);
    void HandleTextClipData(COleDataObject *pDataObject);
private:
    UINT m_nHtmlFormat;
};

