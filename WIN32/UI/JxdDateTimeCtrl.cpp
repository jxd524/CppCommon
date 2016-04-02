#include "stdafx.h"
#include "JxdDateTimeCtrl.h"

IMPLEMENT_DYNAMIC(CxdDateTimeCtrl, CDateTimeCtrl)
BEGIN_MESSAGE_MAP(CxdDateTimeCtrl, CDateTimeCtrl)
	ON_NOTIFY_REFLECT(DTN_DATETIMECHANGE, &CxdDateTimeCtrl::OnDtnDatetimechange)
	//ON_NOTIFY_REFLECT(DTN_DROPDOWN, &CxdDateTimeCtrl::OnDtnDropdown)
	ON_MESSAGE( DTM_SETMCCOLOR, OnSetMCColor )
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CxdDateTimeCtrl::CxdDateTimeCtrl(void):CxdWinControl<CDateTimeCtrl>(), CtDateTimeCtrlID( GetUnionID() )
{
	m_AutoHandleMouseWheel = true;
    InitInterface( DateTimeChanged );
}

CxdDateTimeCtrl::~CxdDateTimeCtrl(void)
{
    DeleteInterface( DateTimeChanged );
}

bool CxdDateTimeCtrl::xdCreate(const int &Ax, const int &Ay, const int &AWidth, const int &AHeight, CxdUIComponent *ApParent , const TCHAR *ApWindowName )
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
	bool bOK = TRUE == Create( GetWndStyle(), Position()->GetCRect(), pObj, CtDateTimeCtrlID );
	if ( bOK )
	{
        if ( Assigned(m_pParent) )
        {
            m_pParent->AddSubComponent( this );
        }
		if ( Assigned(m_pAnchors) )
		{
			m_pAnchors->NotifyAnchorChanged( );
		}
		OnCreated();
	}
	return bOK;
}

void CxdDateTimeCtrl::_OnWndPaint(bool &ACallDefault)
{
	ACallDefault = true;
}

void CxdDateTimeCtrl::OnDtnDatetimechange(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
    NotifyInterface( DateTimeChanged, this );
	*pResult = 0;
}


void CxdDateTimeCtrl::OnDtnDropdown(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
    CMonthCalCtrl* pMoCalCtrl = GetMonthCalCtrl(); 
	*pResult = 0;
    OutputDebugString( TEXT("OnDtnDropdown123") );
}


BOOL CxdDateTimeCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ( GetAutoHandleMouseWheel() )
	{
		WPARAM w = zDelta > 0 ? VK_DOWN : VK_UP;
		PostMessage( WM_KEYDOWN, w, 0x1480001 );
		PostMessage( WM_KEYUP, w, 0x1480001 );
	}
	return CxdWinControl<CDateTimeCtrl>::OnMouseWheel(nFlags, zDelta, pt);
}

LRESULT CxdDateTimeCtrl::OnSetMCColor(WPARAM wParam, LPARAM lParam)
{
	return -1;
}

BOOL CxdDateTimeCtrl::PreTranslateMessage( MSG* pMsg )
{
    if ( /*SPI_SETACTIVEWNDTRKZORDER*/4109 == pMsg->message )
    {
        CloseMonthCal();
    }
    return CxdWinControl<CDateTimeCtrl>::PreTranslateMessage(pMsg);
}
