/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdWinControl.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:18:42
	Description: 
**************************************************************************/
#pragma once;
#include "JxduiDefines.h"
#include "JxduiBasic.h"

#include <vector>
using namespace std;
//====================================================================
// WinControl 有句柄基类
//====================================================================
//
template<typename TBaseClass>
class CxdWinControl: public TBaseClass, public CxdUIComponent
{
public:
    CxdWinControl(): CtClearDoubleBmpTimerID( GetUnionID() )
    {
        m_bDrawByParent = false;
        m_nClearDoubleBmpTimeElapse = 5000;
        m_hDoubleBitmap = 0;
        m_nDoubleBmpWidth = 0;
        m_nDoubleBmpHeight = 0;
        m_bSetCapture = false;    
        m_bDrawByParent = false;
        m_DoubleBuffer = false;
        m_bMouseInClient = FALSE;    
        m_bIsWndObject = true;
        m_pCurCaptureWidget = NULL;
        m_bMouseDown = false;
        Position()->AddEventPositionChanged( NewInterfaceByClass(CxdWinControl, CxdPosition*, DoPositionChanged) );
    }
    virtual ~CxdWinControl()
    {
        ClearTimerInfo();
        ClearDoubleBuffer();  
        ClearSubComponents();
        DestroyWindow();
    }

    CtConst xdGetClassName(void) { return CtxdWinControl; }
    bool xdCreate(const int &Ax, const int &Ay, const int &AWidth, const int &AHeight, CxdUIComponent *ApParent = NULL, const TCHAR *ApWindowName = NULL)
    {
        if ( m_bCreated ) return false;

        HWND hParentHandle = 0;
        if ( Assigned(ApParent) )
        {
            CWnd *pWnd = NULL;
            if ( ApParent->IsWndObject() )
            {
                pWnd = dynamic_cast<CWnd*>(ApParent);
            }
            else
            {
                CxdUIComponent *pOwner = ApParent->GetOwner();
                pWnd = dynamic_cast<CWnd*>(pOwner);
            }
            hParentHandle = pWnd->GetSafeHwnd();
        }
        m_pParent = ApParent;

        CString strDefaultClassName( TEXT("#32770") );
        CString strNewClassName( CtxdWinControl );
        DWORD dwStyle = WS_POPUP | WS_MINIMIZEBOX;
        DWORD dwExStyle = 0;
        OnGetCreateInfo( strDefaultClassName, strNewClassName, dwStyle, dwExStyle );
        WNDCLASS wc;
        if ( !GetClassInfo(::GetModuleHandle(0), strNewClassName, &wc) )
        {
            if ( !GetClassInfo( ::GetModuleHandle(0), strDefaultClassName, &wc) )
            {
                return false;
            }
            wc.lpszClassName = strNewClassName;
            RegisterClass( &wc );
        }

        OnInitWindowSize( Ax, Ay, AWidth, AHeight );
        BOOL Result = CreateEx( dwExStyle, strNewClassName, ApWindowName, dwStyle, 
            Position()->GetLeft(), Position()->GetTop(), Position()->GetWidth(), Position()->GetHeight(), hParentHandle, 0 );
        if ( Result )
        {
            m_bCreated = true;
            if ( Assigned(m_pParent) )
            {
                m_pParent->AddSubComponent( this );
            }
            CheckAnchors( NULL );
            OnCreated();
            return true;
        }
        return false;
    }

    CxdUIComponent* GetOwner(void)
    {
        if ( Assigned(m_pParent) )
        {
            return m_pParent->GetOwner();
        }
        return dynamic_cast<CxdUIComponent*>(this);
    }

    bool IsCurCapture(CxdUIComponent *ApObject)
    {
        return Assigned(m_pCurCaptureWidget) && m_pCurCaptureWidget == ApObject;
    }
    bool IsCanMoveObject(void) { return true; }

    bool GetEnable()
    {
        return TRUE == IsWindowEnabled();
    }
    void SetEnable(bool bEnable)
    {
        BOOL b = bEnable ? TRUE : FALSE;  
        EnableWindow( b ); 
        ClearBufferBitmap();
        Invalidate();
    }

    bool IsDrawCaption(void){ return true; }
    CString GetCaption(void)
    {
        CString strCaption;
        GetWindowText( strCaption );
        return strCaption;
    }
    void SetCaption(const TCHAR* ApCaption)
    {
        SetWindowText( ApCaption );
    }

    void Invalidate()
    {
        if ( m_hWnd == 0 || !CxdUIComponent::GetVisible() )
        {
            return;
        }
        TBaseClass::Invalidate( FALSE );
    }
    void InvalidateRect(CRect *ApRect = NULL)
    {
        if ( m_hWnd == 0 || !CxdUIComponent::GetVisible() )
        {
            return;
        }
        TBaseClass::InvalidateRect( ApRect, FALSE );
    }

    // 二级缓存是否启用 [2012-7-16 22:55 by Terry]
    PropertyValue( DoubleBuffer, bool, true );

    bool IsExsitsStyle(const int &AStyle)
    {
        DWORD dwStyle = GetWindowLong( m_hWnd, GWL_STYLE );
        return (dwStyle & AStyle) == AStyle;
    }
    bool SetWndStyle(const int &ANewStyle, bool bAdd)
    {
        DWORD dwStyle = GetWindowLong( m_hWnd, GWL_STYLE );
        bool bExsits = (dwStyle & ANewStyle) == ANewStyle;
        if ( bAdd )
        {
            if ( !bExsits )
            {
                SetWindowLong( m_hWnd, GWL_STYLE, dwStyle | ANewStyle );
            }
        }
        else
        {
            if ( bExsits )
            {
                SetWindowLong( m_hWnd, GWL_STYLE, dwStyle ^ (dwStyle & ANewStyle) );
            }
        }
        dwStyle = GetWindowLong( m_hWnd, GWL_STYLE );
        bExsits = (dwStyle & ANewStyle) == ANewStyle;
        return bAdd == bExsits;
    }
protected:
    // 实现基类的窗口过程 [2012-7-16 22:57 by Terry]
    virtual LRESULT WindowProc(UINT nMessage, WPARAM wParam, LPARAM lParam)
    {
        switch ( nMessage )
        {
        case WM_PAINT:
            {
                bool bCallDefault = false;
                _OnWndPaint( bCallDefault );
                if ( bCallDefault )
                {
                    return TBaseClass::WindowProc( nMessage, wParam, lParam );
                }
                return TRUE;
            }
        case WM_ERASEBKGND:
            {
                //return TRUE;
                if ( Assigned(m_pImgInfo) && m_pImgInfo->IsExsitsBitmap() )
                {
                    return 1;
                }
                return TBaseClass::WindowProc( nMessage, wParam, lParam );
            }
        case WM_SETCURSOR:
            {
                CxdUIComponent *pWidget = GetTopActiveWidget();
                if ( pWidget->IsExsitsCursor() )
                {
                    ::SetCursor( pWidget->Cursor()->GetCursor() );
                    return TRUE;
                }
                return TBaseClass::WindowProc( nMessage, wParam, lParam );
                break;
            }
            //case WM_SIZE:
            //    {     
            //        if ( SIZE_MINIMIZED == wParam )
            //        {
            //            return TBaseClass::WindowProc( nMessage, wParam, lParam );
            //        }
            //        
            //        LRESULT lResult = TBaseClass::WindowProc( nMessage, wParam, lParam );
            //        Invalidate();
            //        Position()->NotifySizeChanged();     
            //        return lResult;
            //    }
        case CtCustomMessageID:
            {
                try
                {
                    CxdUIComponent *pObj = (CxdUIComponent*)wParam;
                    CxdCustomMessageInfo *pInfo = (CxdCustomMessageInfo*)lParam;
                    pObj->OnCustomMessage( pInfo );
                }
                catch(...)
                {
                }
                return TRUE;
            }
        case WM_MOUSEWHEEL:
            {
                CPoint pt( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
                ::ScreenToClient( Assigned(m_pParent) ? m_pParent->GetOwnerHandle() : GetOwnerHandle(), &pt );
                TxdComponentList *pList = Assigned(m_pParent) ? &m_pParent->m_ltComponents : &m_ltComponents;
                for ( TxdComponentListIT it = pList->begin(); it != pList->end(); it++ )
                {
                    CxdUIComponent *pObj = *it;
                    if ( pObj->GetVisible() && pObj->IsEnable() && pObj->Position()->IsInReect(pt.x, pt.y) )
                    {
                        UINT fwKeys = GET_KEYSTATE_WPARAM(wParam);
                        short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                        pObj->OnMidMouseWheel( fwKeys, zDelta, pt.x, pt.y );
                        if ( pObj->IsWndObject() )
                        {
                            break;
                        }
                        return TRUE;
                    }
                }
            }
        }

        LRESULT lResult = TBaseClass::WindowProc( nMessage, wParam, lParam );
        switch ( nMessage )
        {
        case WM_WINDOWPOSCHANGED:
            {
                WINDOWPOS *pPos = (WINDOWPOS*)lParam;
                CRect Rect;
                if ( IsIconic() )
                {
                    WINDOWPLACEMENT WindowPlacement; 
                    WindowPlacement.length = sizeof(WindowPlacement);
                    GetWindowPlacement( &WindowPlacement );
                    Rect = WindowPlacement.rcNormalPosition;
                }
                else
                {
                    GetWindowRect( &Rect );
                }
                if ( (GetWindowLong(GetSafeHwnd(), GWL_STYLE) & WS_CHILD) == WS_CHILD )
                {
                    CWnd *pParent = GetParent();
                    if ( Assigned(pParent) )
                    {
                        pParent->ScreenToClient( &Rect );
                    }
                }
                Position()->SetLeft( Rect.left );
                Position()->SetTop( Rect.top );

                if ( Position()->GetWidth() != Rect.Width() || Position()->GetHeight() != Rect.Height() )
                {
                    Position()->SetWidth( Rect.Width() );
                    Position()->SetHeight( Rect.Height() );
                    Invalidate();
                    Position()->NotifyPositionChanged();
                }
                //Position()->NotifyPositionChanged();
                break;
            }

        case WM_MOUSEMOVE:
            {
                if ( !m_bMouseInClient )
                {
                    TRACKMOUSEEVENT tme;
                    tme.cbSize = sizeof(tme);
                    tme.hwndTrack = m_hWnd;
                    tme.dwFlags = TME_LEAVE | TME_HOVER;
                    tme.dwHoverTime = 1;
                    m_bMouseInClient = _TrackMouseEvent( &tme );
                }      
                if ( IsMouseActive() )
                {
                    CxdUIComponent *pObj = this;
                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);
                    if ( Assigned(m_pCurCaptureWidget) ) 
                    {
                        if ( m_bMouseDown && m_pCurCaptureWidget->GetAutoMoveForm() )
                        {
                            CxdUIComponent *pMove = m_pCurCaptureWidget->GetMoveFormObject();
                            if ( !Assigned(pMove) )
                            {
                                if ( m_pCurCaptureWidget->IsWndObject() )
                                {
                                    pMove = m_pCurCaptureWidget;
                                }
                                else
                                {
                                    pMove = m_pCurCaptureWidget->GetComponentParent();
                                    if ( !Assigned(pMove) )
                                    {
                                        pMove = m_pCurCaptureWidget->GetOwner();
                                    }
                                }
                            }
                            if ( !Assigned(pMove) ) break;

                            if ( pMove->IsCanMoveObject() )
                            {
                                pMove->Position()->SetLeft( pMove->Position()->GetLeft() + x - m_ptDownPos.x );
                                pMove->Position()->SetTop( pMove->Position()->GetTop() + y - m_ptDownPos.y );
                                pMove->Position()->NotifyPositionChanged();
                            }
                        }

                        CRect uiDest( m_pCurCaptureWidget->GetRealRectOnUI() );
                        x -= uiDest.left;
                        y -= uiDest.top;
                        pObj = m_pCurCaptureWidget;
                    }
                    pObj->OnMouseMoveOnWidget(wParam, x, y );
                }
                break;
            }
        case WM_MOUSEHOVER:
            {
                //TRACE( TEXT("WM_MOUSEHOVER\r\n") );
                //OnUIStateChanged( uiActive );
                OnMouseEnterWidget();
                break;
            }
        case WM_MOUSELEAVE:
            {
                //TRACE( TEXT("WM_MOUSELEAVE\r\n") );
                m_bMouseInClient = FALSE;
                m_bMouseDown = false;
                OnMouseLeaveWidget();
                break;
            }
        case WM_LBUTTONDOWN:
            {
                //TRACE( TEXT("WM_LBUTTONDOWN\r\n") );
                CxdUIComponent *pWidget = GetTopActiveWidget();
                if ( pWidget->IsEnable() )
                {
                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);
                    if ( pWidget->GetAutoMoveForm() )
                    {
                        m_bMouseDown = true;
                        m_pCurCaptureWidget = pWidget;
                        m_ptDownPos.x = x;
                        m_ptDownPos.y = y;
                        m_bSetCapture = true;
                        ReleaseCapture();
                        SetCapture();
                    }
                    else if ( pWidget->GetCaptureOnMouseDown() )
                    {
                        m_pCurCaptureWidget = pWidget;
                        m_bSetCapture = true;
                        ReleaseCapture();
                        SetCapture();
                    }                
                    CRect r = pWidget->GetRealRectOnUI();
                    pWidget->OnLButtonDownWidget( wParam, x - r.left, y - r.top );
                }
                break;
            }
        case WM_RBUTTONDOWN:
            {
                CxdUIComponent *pWidget = GetTopActiveWidget();
                if ( pWidget->IsEnable() )
                {
                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);
                    CRect r = pWidget->GetRealRectOnUI();
                    pWidget->OnRButtonDownWidget( wParam, x - r.left, y - r.top );
                }
                break;
            }
        case WM_RBUTTONUP:
            {
                CxdUIComponent *pObject = NULL;
                if ( Assigned(m_pCurCaptureWidget) )
                {
                    pObject = m_pCurCaptureWidget;
                    m_pCurCaptureWidget = NULL;
                }            
                else
                {
                    pObject = GetTopActiveWidget();
                }
                if ( Assigned(pObject) && pObject->IsEnable() )
                {
                    CRect uiR( pObject->GetRealRectOnUI() );
                    CPoint pt( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
                    TxdGpUIState ui = PtInRect(&uiR, pt) ? uiActive : uiNormal;
                    int x = pt.x - uiR.left;
                    int y = pt.y - uiR.top;
                    if ( ui == uiActive )
                    {
                        pObject->OnRButtonClick( x, y );    
                    }
                    GetCursorPos( &pt );
                    ScreenToClient( &pt );
                    ui = PtInRect(&uiR, pt) ? uiActive : uiNormal;
                    x = pt.x - uiR.left;
                    y = pt.y - uiR.top;
                    pObject->OnRButtonUpWidget( wParam, x, y );
                }
                break;
                break;
            }
        case WM_LBUTTONUP:
            {
                //TRACE( TEXT("WM_LBUTTONUP\r\n") );
                m_bMouseDown = false;
                if ( m_bSetCapture )
                {
                    ReleaseCapture();
                    m_bSetCapture = false;
                }
                CxdUIComponent *pObject = NULL;
                if ( Assigned(m_pCurCaptureWidget) )
                {
                    pObject = m_pCurCaptureWidget;
                    m_pCurCaptureWidget = NULL;
                }            
                else
                {
                    pObject = GetTopActiveWidget();
                }
                if ( Assigned(pObject) && pObject->IsEnable() )
                {
                    CRect uiR( pObject->GetRealRectOnUI() );
                    CPoint pt( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
                    TxdGpUIState ui = PtInRect(&uiR, pt) ? uiActive : uiNormal;
                    int x = pt.x - uiR.left;
                    int y = pt.y - uiR.top;
                    if ( ui == uiActive )
                    {
                        pObject->OnClick( x, y );    
                    }
                    GetCursorPos( &pt );
                    ScreenToClient( &pt );
                    ui = PtInRect(&uiR, pt) ? uiActive : uiNormal;
                    x = pt.x - uiR.left;
                    y = pt.y - uiR.top;
                    pObject->OnLButtonUpWidget( wParam, x, y );
                    if ( uiNormal == ui )
                    {
                        pObject->OnMouseLeaveWidget();
                        OnMouseMoveOnWidget( wParam, x, y );
                    }
                }
                break;
            }
        case WM_LBUTTONDBLCLK:
            {
                //TRACE( TEXT("WM_LBUTTONDBLCLK\r\n") );
                CxdUIComponent *pWidget = GetTopActiveWidget();
                if ( pWidget->IsEnable() )
                {
                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);
                    CRect r = pWidget->GetRealRectOnUI();
                    pWidget->OnLButtonDbclickWidget( wParam, x - r.left, y - r.top );
                }
                break;
            }
        }
        return lResult;
    }

    // 外部改变位置后,调用 Position 的 NotifyChanged 时被触发 [2012-7-16 22:56 by Terry]
    virtual void DoPositionChanged(CxdPosition *ApPosition)
    {
        SetWindowPos( NULL, Position()->GetLeft(), Position()->GetTop(), 
            Position()->GetWidth(), Position()->GetHeight(), SWP_NOZORDER | SWP_NOACTIVATE );
    }

    // 新增虚函数 [2012-7-17 10:47 by Terry]    
    virtual void _OnWndPaint(bool &ACallDefault)
    {
        ACallDefault = false; 
        PAINTSTRUCT ps;
        HDC dc = ::BeginPaint( GetSafeHwnd(), &ps );
        m_rCurReDrawRect = ps.rcPaint;
        //CString str;
        //str.Format( TEXT("WM_PAINT(left: %d, top: %d, right: %d, bottom: %d\r\n"), 
        //    m_rCurReDrawRect.left, m_rCurReDrawRect.top, m_rCurReDrawRect.right, m_rCurReDrawRect.bottom );
        //OutputDebugString( str );
        if ( !Assigned(m_pParent) || !m_bDrawByParent )
        {
            Rect DestR( Rect(0, 0, Position()->GetWidth(), Position()->GetHeight()) );
            if ( m_DoubleBuffer )
            {
                HDC memDC = CreateCompatibleDC(dc);
                if ( m_hDoubleBitmap == 0 || m_nDoubleBmpWidth != DestR.Width || m_nDoubleBmpHeight != DestR.Height )
                {
                    ClearDoubleBuffer();
                    m_nDoubleBmpWidth = DestR.Width;
                    m_nDoubleBmpHeight = DestR.Height;
                    m_hDoubleBitmap = CreateCompatibleBitmap( dc, DestR.Width, DestR.Height );
                    StartTimer( CtClearDoubleBmpTimerID, m_nClearDoubleBmpTimeElapse );
                }
                SelectObject( memDC, m_hDoubleBitmap );

                Graphics G( memDC );
                G.SetClip( Rect(m_rCurReDrawRect.left, m_rCurReDrawRect.top, m_rCurReDrawRect.Width(), m_rCurReDrawRect.Height()) );

                OnPaintComponent( G, DestR );
                BitBlt( dc, m_rCurReDrawRect.left, m_rCurReDrawRect.top, m_rCurReDrawRect.Width(), m_rCurReDrawRect.Height(),
                    memDC, m_rCurReDrawRect.left, m_rCurReDrawRect.top, SRCCOPY );

                DeleteDC( memDC );
            }
            else
            {
                Graphics G( dc );
                G.SetClip( Rect(m_rCurReDrawRect.left, m_rCurReDrawRect.top, m_rCurReDrawRect.Width(), m_rCurReDrawRect.Height()) );

                OnPaintComponent( G, DestR );
            }
        }

        ::EndPaint( GetSafeHwnd(), &ps );
        m_rCurReDrawRect = CRect(0, 0, 0, 0);
    }
    virtual void OnGetCreateInfo(CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle) = 0;
    virtual void OnInitWindowSize(const int &Ax, const int &Ay, const int &AWidth, const int &AHeight)
    {
        Position()->SetLeft( Ax );
        Position()->SetTop( Ay );
        Position()->SetWidth( AWidth );
        Position()->SetHeight( AHeight );
    }

    // 重载父类的相关过程 [2012-7-16 22:58 by Terry]
    const CRect& GetCurReDrawRect(void) { return m_rCurReDrawRect; }
    bool IsNeedDraw(const Gdiplus::Rect ARect)
    {
        RECT r, r1;
        r1.left = ARect.X;
        r1.top = ARect.Y;
        r1.right = ARect.X + ARect.Width;
        r1.bottom = ARect.Y + ARect.Height;
        return IntersectRect( &r, &r1, &m_rCurReDrawRect ) != 0;
    }

    bool OnUIStateChanged(const TxdGpUIState &ANewSate)
    {
        if ( !CxdUIComponent::OnUIStateChanged(ANewSate) )
        {
            return false;
        }

        if ( m_bDrawByParent )
        {
            CWnd *pWnd = GetParent();
            Assigned(pWnd) ? 
                pWnd->InvalidateRect( &Position()->GetCRect(), FALSE ) :
            Invalidate();
        }
        else
        {
            Invalidate();
        }
        ClearBufferBitmap();
        return true;
    }

    CRect GetRealRectByParent(void) { return CRect(Position()->GetLeft(), Position()->GetTop(), Position()->GetWidth(), Position()->GetHeight()); }
    CRect GetRealRectOnUI(void)  { return CRect(0, 0, Position()->GetWidth(), Position()->GetHeight()); }

    void DoSettingVisible(bool bVisible)
    {
        ShowWindow( bVisible ? SW_SHOW : SW_HIDE );
    }
    bool DoGetVisible(void)
    {
        return IsWindowVisible() == TRUE;
    }
    bool OnTimerMessage(int ATimeID, LPVOID ApData)
    {
        if ( CtClearDoubleBmpTimerID == ATimeID )
        {
            ClearDoubleBuffer();
            return false;
        }
        return CxdUIComponent::OnTimerMessage( ATimeID, ApData );
    }
    void PostNcDestroy() {}
protected:
    CRect m_rCurReDrawRect; //当前需要重绘的位置 只读
    bool m_bSetCapture;  
    CxdUIComponent *m_pCurCaptureWidget;
private:
    BOOL m_bMouseInClient;  
    CxdUIComponent *m_pActiveShowHint;
    DWORD m_dwShowHintElapse;

    bool m_bMouseDown;
    CPoint m_ptDownPos;

    //双缓存
    int m_nClearDoubleBmpTimeElapse;
    HBITMAP m_hDoubleBitmap;
    int m_nDoubleBmpWidth;
    int m_nDoubleBmpHeight;
    const int CtClearDoubleBmpTimerID;
    void ClearDoubleBuffer(void)
    {
        if ( m_hDoubleBitmap != 0 )
        {
            DeleteObject( m_hDoubleBitmap );
        }
        m_hDoubleBitmap = 0;
        m_nDoubleBmpWidth = 0;
        m_nDoubleBmpHeight = 0;
    }
};