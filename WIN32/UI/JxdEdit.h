/**************************************************************************
Copyright  : pp.cc ����ָ��ʱ�� ɺ����
File       : JxdEdit.h & .cpp 
Author     : Terry
Email      : jxd524@163.com
CreateTime : 2012-7-19 22:50
Description: �༭���ܷ�װ, ֧��͸����, ����ͼ, �߿�״̬, ������ɫ������
             �ؼ��ĸ߶�������Ҫ��,���ĸ߶����ⲿָ��,������Ҫ����ָ��ֵ 
             CtrlHeight �㷨����:

             HDC dc = ::GetDC( pEdt->GetSafeHwnd() );
             HDC memDC = CreateCompatibleDC( dc );
             SelectObject( memDC, GetGlbCFont()->GetSafeHandle() );
             SIZE size;
             CString str( TEXT("��") );
             ::GetTextExtentPoint32(memDC, str, str.GetLength(), &size);
             DeleteDC(memDC);
             ReleaseDC( pEdt->GetSafeHwnd(), dc );
             pEdt->Position()->SetHeight( size.cy + pEdt->GetBroderWidth() * 2 );
             pEdt->Position()->NotifyPositionChanged();

             �����ϴ�ֵ�ĸ߶�,���ñ���͸��ʱ,�����������ɫ
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

    //�Ƿ���ʾ������,ֻ���ڴ���ǰ����
    PropertyValue( ShowVScroll, bool, !IsWindow(m_hWnd) );

    // ����͸��, ��Create֮ǰ����,����ʧ�� [2012-7-19 21:52 by Terry]
    bool GetTransParent(void) { return m_bTransParent; }
    bool SetTransParent(bool bTransParent) { return SetValueOnBeforCreate(bTransParent, m_bTransParent); }

    // ��������, �� TransParent Ϊtrue ʱ����, ��ֵ -255(��) ~ 255(��) [2012-7-19 21:56 by Terry]
    int  GetBkBrightness(void) { return m_nBrightness; }
    void SetBkBrightness(const int &AValue) { SetValueOnAnyTime(AValue, m_nBrightness); }

    // ��ImageInfo����ʱ����, ������������͸���� ��ֵ: 0.0 ~ 1.0 [2012-7-19 22:00 by Terry]
    REAL GetTranslateValue(void) { return m_fTranslate; }
    void SetTranslateValue(const REAL &AValue) { SetValueOnAnyTime(AValue, m_fTranslate); }

    // ���Ʊ߿�Ĵ�С, ������Ϊ <= 0 ʱ ��ʾ�����Ʊ߿�, �߿�Ĵ�С�����Create֮ǰ���� [2012-7-19 22:06 by Terry]
    int  GetBroderWidth(void) { return m_nBroderWidth; }
    void SetBroderWidth(const int &AValue) { SetValueOnAnyTime(AValue, m_nBroderWidth); }

    // ���Ʊ߿������״̬�µ���ɫֵ, ���߿�ֵ > 0ʱ��Ч [2012-7-19 22:11 by Terry]
    DWORD GetBroderNormalColor(void) { return m_dwNormalBroderColor; }
    void  SetBroderNormalColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_dwNormalBroderColor); }

    // ���Ʊ߿�ļ���״̬�µ���ɫֵ, ���߿�ֵ > 0ʱ��Ч [2012-7-19 22:13 by Terry]
    DWORD GetBroderActiveColor(void) { return m_dwActiveBroderColor; }
    void  SetBroderActiveColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_dwActiveBroderColor); }

    // ����״̬��������ɫֵ, ���б߿���ж౳��ͼ���л������� [2012-7-19 22:15 by Terry]
    DWORD GetTextNormalColor(void) { return m_dwNormatTextColor; }
    void  SetTextNormalColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_dwNormatTextColor); }

    //����״̬��������ɫֵ
    DWORD GetTextActiveColor(void) { return m_dwActiveTextColor; }
    void  SetTextActiveColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_dwActiveTextColor); }

    void    SetCaption(const TCHAR* ApCaption);
    CString GetCaption(void);

    // ����ֵʱ����ʾ��Ϣ [2012-7-19 23:20 by Terry]
    CString GetTipText(void) { return m_strTipTextAtNull; }
    void    SetTipTextAtNull(const TCHAR *ApHitText);

    // ��ʾ��Ϣ������ɫ [2012-7-19 23:21 by Terry]
    DWORD GetTipTextColor(void) { return m_TipTextColor; }
    void  SetTipTextColor(const DWORD &AValue) { SetValueOnAnyTime(AValue, m_TipTextColor); }

    // ��ʾ��Ϣ���� [2012-7-19 23:22 by Terry]
    CFont* GetTipTextFont(void) { return m_pHitFont; }
    void   SetTipTextFont(CFont *ApFont, bool AOnlyRes = true);

    // �Ƿ������ʾ [2012-7-19 23:35 by Terry]
    bool GetMultiLine(void) { return m_bMulitLine; }
    bool SetMultiLine(bool bValue){ return SetValueOnBeforCreate(bValue, m_bMulitLine); }

    //�Ƿ���Ϊ�����ʹ��
    bool GetPassword(void) { return m_bPassword; }
    bool SetPassword(bool bValue);

    // �Ƿ�ֻ�������� [2012-7-20 01:23 by Terry]
    bool GetOnlyNumber(void) { return m_bOnlyNumber; }
    bool SetOnlyNumber(bool bValue) { return SettingWndStyle(bValue, m_bOnlyNumber, ES_NUMBER); }

	//ȡ��ѡ��״̬
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
    int m_nBrightness; //��������: -255 ~ 255
    REAL m_fTranslate;
    HBRUSH m_hBkBrush;
    void CreateBkBrush(void);
    void ClearBkBrush(void);
    void OnEdtParentPositionChanged(CxdPosition* ApPosition);

    int m_nSpace;
    int m_nBroderWidth; // <= 0 ʱ�����Ʊ߿�
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