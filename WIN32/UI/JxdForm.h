/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdForm.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:17:47
	Description: 
**************************************************************************/
#pragma once;

#include "JxdMacro.h"
#include "JxdWinControl.h"
enum TxdGpCloseStyle;
enum TWindowState;
enum TCursorStyle;
class CxdImageInfo;

class CxdForm: public CxdWinControl<CDialog>
{
public:
    CxdForm( const TCHAR *ApClassName = CtxdForm );
    virtual ~CxdForm();

    CtConst xdGetClassName(void) { return CtxdForm; }
    //window state
    TWindowState GetWindowState(void) const { return m_WindowState; }
    void         SetWindowState(TWindowState AState);
    bool IsCanMoveObject(void) { return m_WindowState == wsNormal; }
    //auto changed ui size
    bool GetAutoChangedSize() const { return m_bAutoChangedSize; }
    void SetAutoChangedSize(bool bAuto) { m_bAutoChangedSize = bAuto; }
    //Image
    CxdImageInfo* ImageFormStyle(void) { return xdCreateObject(m_pImgFormStyle); }
    void SetImageFormStyle(const CxdImageInfo *ApNew) { UpdatePointer(m_pImgFormStyle, ApNew); }
    //ICON
    void SetMainIcon(DWORD AIconID);

    void BringWindowToShow(void);

    //Broder
    COLORREF GetBroderColor(void) { return m_dwBorderColor; }
    void  SetBroderColor(const COLORREF &AColor);

    //size contanis
    int  GetMinWidth(void) { return m_nMinWidth; }
    int  GetMinHeight(void) { return m_nMinHeight; }
    void SetMinSize(const int &AMinWidth, const int &AMinHeight){ m_nMinWidth = AMinWidth; m_nMinHeight = AMinHeight; }
    int  GetMaxWidth(void) { return m_nMaxWidth; }
    int  GetMaxHeight(void) { return m_nMaxHeight; }
    void SetMaxSize(const int &AMaxWidth, const int &AMaxHeight){ m_nMaxWidth = AMaxWidth; m_nMaxHeight = AMaxHeight; }

    //TopMost
    void SetTopMost(bool bTopMost);
    bool GetTopMost(void);

    void Close(const DWORD &ADeferTime = 0);
    void ShowModel(bool AbSetParentDisable = true, bool AbSetVisible = true);

    PropertyValue( RoundValue, int, true );
protected:
    CxdImageInfo *m_pImgFormStyle;

    void OnInitWindowSize(const int &Ax, const int &Ay, const int &AWidth, const int &AHeight);
    virtual TxdGpCloseStyle OnCloseForm(void);
    virtual bool CheckNewSize(int &ANewWidth, int &ANewHeight);

    //父类接口实现   
    bool IsDrawCaption(void){ return false; }
    void DoDrawSubjoinBK(Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt);
    bool OnUIStateChanged(const TxdGpUIState &ANewSate) { return false; } //do not need to changed ui state
    void OnGetCreateInfo(CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle); //must
    bool OnTimerMessage(int ATimeID, LPVOID ApData);
private:
    TWindowState m_WindowState;
    void UpdateWindowState(void);

    bool m_bNeedSaveOldCursorStyle;
    TCursorStyle m_csOldStyle;
    bool m_bAutoChangedSize;
    void ChangedCursorStyle(const TCursorStyle &ANewStyle);

    //处理事件
    void DoCursorChanged(void*);

    //form style
    int m_nCurStyleWidth, m_nCurStyleHeight;

    //边框
    HBRUSH m_hBroderBrush;
    COLORREF  m_dwBorderColor;   

    bool m_bTopMost;
    bool m_bLoopModel;

    int m_nMinWidth, m_nMinHeight;
    int m_nMaxWidth, m_nMaxHeight;

    const CString m_CtClassName;
    const int CtCloseTimerID;
protected:
    DECLARE_DYNAMIC(CxdForm)
    DECLARE_MESSAGE_MAP()

    void OnOK(){};
    void OnCancel();
    afx_msg void OnClose();
    afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg virtual void OnSize(UINT nType, int ANewWidth, int ANewHeight);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnLButtonDblClk(UINT, CPoint);

	BOOL _Animate_close_window(HWND hWnd);
};