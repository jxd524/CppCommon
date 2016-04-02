/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdDateTimeCtrl.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:18:00
	Description: 
**************************************************************************/
#pragma once;
#include <afxdtctl.h>
#include "JxdWinControl.h"

class CxdDateTimeCtrl;

typedef CxdNotifyEventT<CObject, CxdDateTimeCtrl*> CxdDateTimeChangedEvent;
typedef CxdDateTimeChangedEvent::IInterfaceType    IxdDateTimeChangedInterface;

class CxdDateTimeCtrl : public CxdWinControl<CDateTimeCtrl>
{
public:
	CxdDateTimeCtrl(void);
	virtual ~CxdDateTimeCtrl(void);

	CtConst xdGetClassName() { return CtxdDateTimeCtrl; }
	bool xdCreate(const int &Ax, const int &Ay, const int &AWidth, const int &AHeight, CxdUIComponent *ApParent = NULL , const TCHAR *ApWindowName = NULL );
	int GetDateTimeCtrlID(void) { return CtDateTimeCtrlID; }

    PropertyValue( AutoHandleMouseWheel, bool, true );
    PropertyValue( WndStyle, int, true );
    PropertyInterface( DateTimeChanged, IxdDateTimeChangedInterface );
protected:
    BOOL PreTranslateMessage(MSG* pMsg);
	void _OnWndPaint(bool &ACallDefault);
	void OnGetCreateInfo(CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle){}
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CxdDateTimeCtrl)
private:
	const int CtDateTimeCtrlID;
public:
	afx_msg void OnDtnDatetimechange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDropdown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	LRESULT OnSetMCColor(WPARAM wParam, LPARAM lParam);
};