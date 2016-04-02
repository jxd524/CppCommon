/**************************************************************************
Copyright  : pp.cc 深圳指掌时代 珊瑚组
File       : JxduiControls.h & .cpp 
Author     : Terry
Email      : jxd524@163.com
CreateTime : 2012-8-4 09:35
Description: UI库具体控件实现
**************************************************************************/
#pragma once

#include "JxdGraphicsControl.h"
#include "JxdForm.h"
#include "JxduiCommon.h"

/*CxdHintWindow: 提示窗口*/
class CxdHintWindow: public CxdForm
{
public:
    CxdHintWindow();
    ~CxdHintWindow();

    void ShowHintMessage(const TCHAR *ApHintText, const int &ALeft, const int &ATop);
    void HideHintMessage(void);

	void SetHintBKColor(DWORD startColor, DWORD endColor, LinearGradientMode lgMode );

    PropertyValue(AnimateTime, DWORD, true );
    PropertyValue(MouseTransParent, bool, !IsCreated() );
protected:
    bool IsDrawCaption(void){ return true; }
    void OnGetCreateInfo(CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle);
    virtual LRESULT WindowProc(UINT nMessage, WPARAM wParam, LPARAM lParam);

    virtual LRESULT OnEraseBkgnd(HDC dc);
	void OnCreated(void);

    DECLARE_DYNAMIC(CxdHintWindow)
    DECLARE_MESSAGE_MAP()
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg virtual void OnSize(UINT nType, int ANewWidth, int ANewHeight);
private:
	DWORD m_dwStartColor;
	DWORD m_dwEndColor;
	LinearGradientMode m_lgMode;
	Bitmap* CreateGradientBmp(void);
};

/*CxdHintManage*/
typedef IxdNotifyEventT<CxdHintWindow**> IxdCreateHintWndInterface;
class CxdHintManage: public IHintMessage
{
    enum TTimerMode{tmShow, tmHide};
public:
    CxdHintManage();
    ~CxdHintManage();

    void ActivateHint(CxdUIComponent *ApObject);
    void UnActiveHint(CxdUIComponent *ApObject);

    PropertyValue( DeferShowTimeElapse, DWORD, true );
    PropertyValue( PauseTimeElapse, DWORD, true );
    PropertyInterface( CreateHintWindow, IxdCreateHintWndInterface );
protected:
    void BeginHandleHintWindow(void);
    virtual void DoCreateHintWindow(void);
private:
    UINT_PTR m_hTimerHandle;
    TTimerMode m_tmMode;

    CxdUIComponent *m_pCurHintComponent;
    CxdHintWindow *m_pHintWnd;
    void StartHintTimer(const DWORD &AValue);
    void StopHintTimer(void);

    static VOID CALLBACK TimerProc(__in  HWND hwnd, __in  UINT uMsg, __in  UINT_PTR idEvent, __in  DWORD dwTime);
};