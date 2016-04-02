/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdTrayIconBasic.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2009-10-08
	LastModify : 2013-10-25 17:21:47
	Description: 应用程序托盘类
**************************************************************************/

#ifndef _JxdTrayIconBasic_Lib
#define _JxdTrayIconBasic_Lib

#include <map>
using namespace std;

#include "JxdEvent.h"
#include "JxdMacro.h"

enum TTrayClickType{ tctLBtnClick, tctLBtnDbClick, tctRBtnClick};
typedef IxdNotifyEventT<TTrayClickType> IxdTrayClick;

//使用此类: 一个应用程序只能有一个托盘图标
class CxdTrayIconBasic
{
public:
    enum TBallonToolTipStype
    {
        btsError = NIIF_ERROR,
        btsInfo = NIIF_INFO,
        btsNone = NIIF_NONE,
        btsUser = NIIF_USER,
        btsWarning = NIIF_WARNING,
        btsNoSound = NIIF_NOSOUND
    };

    CxdTrayIconBasic();
    ~CxdTrayIconBasic();
    bool SetTrayIconHandle(HWND hwnd, UINT dwIconID = 0); //首先应该调用此函数进行设置，之后就能随便调用下面的函数了。
    bool SetIcon(UINT dwIconID);
    bool SetHintText(const CString &AHintText);
    bool ShowBalloonToolTip(const wstring &strText, const wstring &strCaption, DWORD btsStyle, UINT uTimeOut = 30);

    //事件
    PropertyInterface(TrayClick, IxdTrayClick);
protected:
    virtual void OnTrayNotifyEvent(WPARAM wID, LPARAM lEvent); //所有托盘事件处理
private:
    const int CtNotifyDataSize;
    NOTIFYICONDATA m_NotifyData;
    void UnTrayIcon(void);

    static map<HWND, CxdTrayIconBasic*> m_hMap;
    static UINT  m_uIDs;
    static DWORD WM_JXDTRAYICONMESSAGE;
    static HHOOK m_hHook;
    static LRESULT CALLBACK CallWindowProc(int code, WPARAM wParam, LPARAM lParam);
};

#endif