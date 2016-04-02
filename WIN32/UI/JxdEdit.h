/**************************************************************************
Copyright  : pp.cc 深圳指掌时代 珊瑚组
File       : JxdEdit.h & .cpp 
Author     : Terry
Email      : jxd524@163.com
CreateTime : 2012-7-19 22:50
Description: 编辑框功能封装, 支持透明度, 背景图, 边框状态, 字体颜色等内容
             控件的高度有特殊要求,它的高度由外部指定,但是需要符合指定值 
             CtrlHeight 算法如下:

             HDC dc = ::GetDC( pEdt->GetSafeHwnd() );
             HDC memDC = CreateCompatibleDC( dc );
             SelectObject( memDC, GetGlbCFont()->GetSafeHandle() );
             SIZE size;
             CString str( TEXT("国") );
             ::GetTextExtentPoint32(memDC, str, str.GetLength(), &size);
             DeleteDC(memDC);
             ReleaseDC( pEdt->GetSafeHwnd(), dc );
             pEdt->Position()->SetHeight( size.cy + pEdt->GetBroderWidth() * 2 );
             pEdt->Position()->NotifyPositionChanged();

             不符合此值的高度,会让背景透明时,出现意外的颜色
**************************************************************************/
#pragma once

#include "JxdWinControl.h"


class CxdEdit;

struct TxdEditOnChar
{
    UINT nChar;
    UINT nRepCnt;
    UINT nFlags;
    bool bCallDefault;
};
struct TxdEditMidMouseWheel
{
    UINT nFlags;
    short zDelta;
    int x;
    int y;
};

typedef IxdNotifyEventT<CxdEdit*> IxdEditInterface;
typedef IxdNotifyEventExT<CxdEdit*, TxdEditOnChar*> IxdEditOnCharInterface;
typedef IxdNotifyEventExT<CxdEdit*, TxdEditMidMouseWheel*> IxdEditMidMouseWheelInterface;

class CxdEdit: public CxdWinControl<CEdit>
{
public:
    CxdEdit();
    ~CxdEdit();

    CtConst xdGetClassName(void) { return CtxdEdit; }

    CSize CalcBestSize(const int &ASetWidth, const int &ALineCount = 1);

    //是否显示滚动条,只能在创建前设置
    PropertyValue( ShowVScroll, bool, !IsWindow(m_hWnd) );

    // 背景透明, 在Create之前调用,否则失败 [2012-7-19 21:52 by Terry]
    bool GetTransParent(void) { return m_bTransParent; }
    bool SetTransParent(bool bTransParent) { return SetValueOnBeforCreate(bTransParent, m_bTransParent); }

    // 背景亮度, 当 TransParent 为true 时有用, 其值 -255(黑) ~ 255(白) [2012-7-19 21:56 by Terry]
    int  GetBkBrightness(void) { return m_nBrightness; }
    void SetBkBrightness(const int &AValue) { SetValueOnAnyTime(AValue, m_nBrightness); }

    // 当ImageInfo存在时有用, 用于设置它的透明度 其值: 0.0 ~ 1.0 [2012-7-19 22:00 by Terry]
    REAL GetTranslateValue(void) { return m_fTranslate; }
    void SetTranslateValue(const REAL &AValue) { SetValueOnAnyTime(AValue, m_fTranslate); }

    // 绘制边框的大小, 当设置为 <= 0 时 表示不绘制边框, 边框的大小最好在Create之前设置 [2012-7-19 22:06 by Terry]
    int  GetBroderWidth(void) { return m_nBroderWidth; }
    void SetBroderWidth(const int &AValue) { SetValueOnAnyTime(AValue, m_nBroderWidth); }

    // 绘制边框的正常状态下的颜色值, 当边框值 > 0时有效 [2012-7-19 22:11 by Terry]
    DWORD GetBroderNormalColor(void) { return m_dwNormalBroderColor; }
    void  SetBroderNormalColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_dwNormalBroderColor); }

    // 绘制边框的激活状态下的颜色值, 当边框值 > 0时有效 [2012-7-19 22:13 by Terry]
    DWORD GetBroderActiveColor(void) { return m_dwActiveBroderColor; }
    void  SetBroderActiveColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_dwActiveBroderColor); }

    // 正常状态下字体颜色值, 在有边框或有多背景图下有积极作用 [2012-7-19 22:15 by Terry]
    DWORD GetTextNormalColor(void) { return m_dwNormatTextColor; }
    void  SetTextNormalColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_dwNormatTextColor); }

    //激活状态下字体颜色值
    DWORD GetTextActiveColor(void) { return m_dwActiveTextColor; }
    void  SetTextActiveColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_dwActiveTextColor); }

    void    SetCaption(const TCHAR* ApCaption);
    CString GetCaption(void);

    // 当空值时的提示信息 [2012-7-19 23:20 by Terry]
    CString GetTipText(void) { return m_strTipTextAtNull; }
    void    SetTipTextAtNull(const TCHAR *ApHitText);

    // 提示信息字体颜色 [2012-7-19 23:21 by Terry]
    DWORD GetTipTextColor(void) { return m_TipTextColor; }
    void  SetTipTextColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_TipTextColor); }

    // 提示信息字体 [2012-7-19 23:22 by Terry]
    CFont* GetTipTextFont(void) { return m_pHitFont; }
    void   SetTipTextFont(CFont *ApFont, bool AOnlyRes = true);

    // 是否多行显示 [2012-7-19 23:35 by Terry]
    bool GetMultiLine(void) { return m_bMulitLine; }
    bool SetMultiLine(bool bValue){ return SetValueOnBeforCreate(bValue, m_bMulitLine); }

    //是否作为密码框使用
    bool GetPassword(void) { return m_bPassword; }
    bool SetPassword(bool bValue);

    // 是否只接收数字 [2012-7-20 01:23 by Terry]
    bool GetOnlyNumber(void) { return m_bOnlyNumber; }
    bool SetOnlyNumber(bool bValue) { return SettingWndStyle(bValue, m_bOnlyNumber, ES_NUMBER); }

	//取消选中状态
	bool GetNoChooseState(void) { return m_bNoChooseState; }
	void SetNoChooseState(bool bNoChoose) { m_bNoChooseState = bNoChoose; }

    bool GetTextInLeft(void) { return m_bTextInLeft; }
    bool SetTextInLeft(bool bValue);

    //Event
    PropertyInterface( OnPaste, IxdEditInterface );
    PropertyInterface( OnChar, IxdEditOnCharInterface );
    PropertyInterface( OnMidMouseWheel, IxdEditMidMouseWheelInterface );
	PropertyInterface( OnEnEditChange, IxdEditInterface );
protected:
    void OnCreated(void);   
    void OnGetCreateInfo(CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle);
    void _OnWndPaint(bool &ACallDefault);
    bool OnUIStateChanged(const TxdGpUIState &ANewSate);
    void OnMidMouseWheel(UINT nFlags, short zDelta, int x, int y);

    LRESULT WindowProc(UINT nMessage, WPARAM wParam, LPARAM lParam);
    DECLARE_DYNAMIC(CxdEdit)
    DECLARE_MESSAGE_MAP()

    afx_msg UINT OnGetDlgCode();
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg BOOL OnEraseBkgnd(CDC*) { return TRUE; }
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnEditChanged();

    DWORD m_dwSpaceColor;
    virtual CRect CalcEditRect(void);
    virtual void DrawEditSubComponentBK(Graphics &G) {};
private:
    bool m_bMulitLine;
    bool m_bPassword;
    bool m_bOnlyNumber;
    bool m_bTextInLeft;
	bool m_bEnterDown;
	bool m_bNoChooseState;
	int m_nStart, m_nEnd;

    bool IsCurNormal(void);

    bool m_bTransParent;
    int m_nBrightness; //背景亮度: -255 ~ 255
    REAL m_fTranslate;
    HBRUSH m_hBkBrush;
    void CreateBkBrush(void);
    void ClearBkBrush(void);
    void OnEdtParentPositionChanged(CxdPosition* ApPosition);

    int m_nSpace;
    int m_nBroderWidth; // <= 0 时不绘制边框
    DWORD m_dwNormalBroderColor, m_dwActiveBroderColor;
    DWORD m_dwNormatTextColor, m_dwActiveTextColor, m_TipTextColor;
    void SetEditRect(void);

    bool m_bResHitFont;
    bool m_bResetFont;
    bool m_bCurIsHitText;
    CFont *m_pOldFont;
    CFont *m_pHitFont;
    CString m_strTipTextAtNull;
    void ClearHitFont(void);
private:
    template<typename T>
    bool SetValueOnBeforCreate(const T &ANewValue, T &ABeSetValue)
    {
        if ( IsWindow(m_hWnd) )
        {
            return false;
        }
        ABeSetValue = ANewValue;
        return true;
    }

    template<typename T>
    void SetValueOnAnyTime(const T &ANewValue, T&ABeSetValue)
    {
        if ( ANewValue != ABeSetValue )
        {
            ABeSetValue = ANewValue;
            if ( IsWindow(m_hWnd) )
            {
                ClearBkBrush();
                Invalidate();
            }
        }
    }

    bool SettingWndStyle(const bool &ANewValue, bool &ABeSetValue, int AStyleValue);
};