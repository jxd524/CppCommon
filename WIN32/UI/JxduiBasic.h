/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxduiBasic.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2012-7-21 09:26
	LastModify : 2013-10-28 15:00:58
	Description: CxdUIComponent
        基本界面控件实现, 绘制方式: 
            先绘制本身,再绘制子控件, 子控件的绘制再重复此过程
        每个控件包含: 
            Position -> 位置
        接口的使用说明: 由外部 new 一个接口, UI库负责释放

**************************************************************************/

#pragma once;

#include <map>
#include <vector>
#include <list>
using namespace std;

#include "JxdMacro.h"
#include "JxdEvent.h"
#include "JxduiCommon.h"
#include "JxduiDefines.h"

class Gdiplus::Bitmap;
class CxdUIComponent;
typedef IxdNotifyEventT<CxdUIComponent*> IxdComponentInterface;

class CxdUIComponent
{
public:
    //提供给外部自由使用的扩展
    INT64 Tag;

    CxdUIComponent();
    virtual ~CxdUIComponent();

    virtual CtConst xdGetClassName(void) { return CtxdUIComponent; }
    virtual bool    xdCreate(const int &Ax, const int &Ay, const int &AWidth, const int &AHeight, CxdUIComponent *ApParent = NULL, const TCHAR *ApWindowName = NULL) = 0;

    inline bool  IsCreated(void) { return m_bCreated; }
    inline bool  IsWndObject(void) { return m_bIsWndObject; }
    virtual bool IsCanMoveObject(void) = 0;

    //类中核心子对象
    inline CxdPosition*  Position(void) { return &m_oPosition; } 
    inline CxdImageInfo* ImageInfo(void) { return xdCreateObject(m_pImgInfo); }
    inline CxdFontInfo*  FontInfo(void) { return xdCreateObject(m_pFontInfo); }
    inline CxdCursor*    Cursor(void){ return xdCreateObject(m_pCursor); }        
    CxdAnchors*          Anchors(void);

    //判断对象是事存在
    inline bool          IsExsitsImageInfo() { return m_pImgInfo != NULL; }
    inline bool          IsExsitsFontInfo() { return m_pFontInfo != NULL; }
    inline bool          IsExsitsCursor() { return m_pCursor != NULL; }
    inline bool          IsExsitsAnchor(void) { return m_pAnchors != NULL; }

    //提供另一种设置子对象的方法
    void SetImageInfo(CxdImageInfo *ApNewImg, bool AbResOnly = true);
    void SetFontInfo(CxdFontInfo *ApNewFont, bool AbResOnly = true);

    // 可见性 [2012-7-16 22:48 by Terry]
    bool GetVisible();
    void SetVisible(bool bVisible);

    // 可用性 [2012-7-16 22:48 by Terry]
    virtual bool GetEnable() = 0;
    virtual void SetEnable(bool bEnable) = 0;

    // 提示信息, 需要设置 JxduiCommon 中的函数: SetHintInterface [2012-8-3 14:59 by Terry]
    CString* GetHintText(void) { return m_pStrHintText; }
    void     SetHintText(const TCHAR *ApHintText);

    // Caption信息 [2012-8-1 17:24 by Terry]
    virtual bool    IsDrawCaption(void) = 0;
    virtual CString GetCaption(void) = 0; 
    virtual void    SetCaption(const TCHAR* ApCaption) = 0; 
    int  GetCaptionLeftSpace(void) { return m_nCaptionLeft; }
    void SetCaptionLeftSpace(const int &ALefSpace) { m_nCaptionLeft = ALefSpace; }
    int  GetCaptionTopSpace(void) { return m_nCaptionTop; }
    void SetCaptionTopSpace(const int &ATopSpace) { m_nCaptionTop = ATopSpace; }
    void SetAutoSizeByCaption(const int &ALeftRightSpace = 2, const int &ATopDownSpace = 2, const int &AMaxWidth = 0, const int &AMaxHeight = 0);

    CSize CalcStringSize(const TCHAR *ApString, const int &AMaxWidth = 0, const int &AMaxHeight = 0);
    void  SetAutoSizeByImage(void);

    // 重绘 [2012-7-16 22:48 by Terry]    
    virtual void Invalidate(void);
    virtual void InvalidateGpRect(const Rect &ARect);
    virtual void InvalidateRect(CRect *ApRect = NULL) = 0;

    // 当前状态 [2012-7-16 22:48 by Terry]
    virtual TxdGpUIState GetCurUISate(void) { return m_eUIState; }
    inline bool IsMouseActive(void) { return m_bMouseInWidget; }

    // 获取父类,不一定是拥有句柄的类 [2012-7-21 09:46 by Terry]
    virtual CxdUIComponent* GetComponentParent(void) { return m_pParent; }

    // 获取拥有者, 一定是拥有句柄的类 [2012-7-21 09:48 by Terry]
    virtual CxdUIComponent* GetOwner(void) { return NULL; }
    virtual HWND GetOwnerHandle(void);

    //自定义消息
    void PostCustomMessage(CxdCustomMessageInfo *ApInfo);
    void SendCustomMessage(CxdCustomMessageInfo *ApInfo);

    //Event
    // 单击 [2012-7-16 22:49 by Terry]
    void Click(void) { OnClick(); }
    void RButtonClick(void) { OnRButtonClick(); }

    //定时器, 将触发 OnTimerMessage
    virtual void StartTimer(int ATimeID, int ATimeElapse, LPVOID AData = NULL);
    virtual void EndTimer(int ATimeID);

    // 是否移动指定窗口 [2012-7-16 22:54 by Terry]
    bool  GetAutoMoveForm(void) { return m_bAutoMoveForm; }
    void  SetAutoMoveForm(bool bAutoMove) { m_bAutoMoveForm = bAutoMove; }
    CxdUIComponent* GetMoveFormObject(void) { return m_pAutoMoveForm; }
    void  SetMoveFormObject(CxdUIComponent *ApObject) { m_pAutoMoveForm = ApObject; }

    //当前是否正在捕获鼠标操作
    virtual bool IsCurCapture(CxdUIComponent *ApObject) = 0;

    //添加进来的对象的生命周期将由内部管理,外部不可再进行释放
    bool AddSubComponent(CxdUIComponent *ApObject);
    bool DeleteSubComponent(CxdUIComponent *ApObject);
    void ClearSubComponents(void);
    //TCompoentPList* GetSubComponentList(void){ return &m_ltComponents; }
    //int GetSubComponentCount(void) { return (int)m_ltComponents.size(); }

    // 返回当前激活状态的部件 [2012-7-22 16:01 by Terry]
    inline CxdUIComponent* GetCurActiveWidget(void) { return m_pActiveWidget; }
    // 返回最顶层的激活状态部件 [2012-7-22 17:32 by Terry]
    CxdUIComponent* GetTopActiveWidget(void) { if (Assigned(m_pActiveWidget)) return m_pActiveWidget->GetTopActiveWidget(); return this; }

    //获取背景图片
    Gdiplus::Bitmap* GetBufferBitmap(void);
    void             DeferClearBufferBitmap(void); //延迟释放
    void             ClearBufferBitmap(void); // 立即释放 [2012-9-5 21:20 by Terry]
    virtual bool     CopyBufferDC(HDC AhDestDC, const Rect &ACopyRect);
    virtual bool     CopyBufferBitmap(Gdiplus::Bitmap *ApDestBmp, const Gdiplus::Rect &ACopyRect);

    //当要显示提示信息时被触发, 返回 false 时 或 GetHintText 为空时,不提示
    //一般由 JxdHintWindow 调用 
    virtual bool OnHintMessage(void);

    //属性定义 
    PropertyValue( BufferBitmapTimeout, int, true ); //缓存时间
    PropertyValue( VisibleOnParentActive, bool, true ); //当父类激活时可见
    PropertyValue( AnimateUnVisible, bool, true ); //不可见时使用动态效果
    PropertyValue( DrawGrayOnUnEnable, bool, true ); //不可用时自动灰色
    PropertyValue( CaptureOnMouseDown, bool, true ); //鼠标按下时, 是否捕获鼠标操作

    //事件属性定义
    PropertyInterface( Click, IxdComponentInterface );  //点击
    PropertyInterface( RButtonClick, IxdComponentInterface ); //R 点击
    PropertyInterface( DblClick, IxdComponentInterface ); //双击
    PropertyInterface( UIStateChanged, IxdComponentInterface ); //UI状态改变时
    PropertyInterface( OnHintMessage, IxdComponentInterface ); //当显示提示信息时
protected:
    /*由父类或其派生类调用: 触发式*/
    virtual void OnCreated(void); // 当外部调用xdCreate成功之后被调用 [2012-7-17 14:02 by Terry]

    // 绘制本身 由此父类调用 [2012-7-16 22:51 by Terry]
    virtual void OnPaintComponent(Graphics &G, const Rect &ADestR);

    //绘制分发函数 OnPaintWidget -> DoDrawUI 
    virtual void DoDrawUI(Graphics &G, const Rect &ADestR);    
    virtual void DoDrawSubjoinBK(Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt) {};

    //绘制文字
    virtual void DoDrawText(Graphics &G, const Rect &AuiDestR);    
    //绘制分发函数 OnPaintWidget -> DoDrawSubComponent
    virtual void DoDrawSubComponent(Graphics &G, const Rect &AuiDestR);

    //获取绘制属性
    virtual ImageAttributes* GetDrawBKImageAttributes(void);

    // UI状态改变时被触发 [2012-7-16 22:51 by Terry]
    virtual bool OnUIStateChanged(const TxdGpUIState &ANewSate);
    virtual void DoSettingVisible(bool bVisible) = 0;
    virtual bool DoGetVisible(void) = 0;
    virtual void DoChangedFontColor(DWORD &AFontColor){}
    virtual void DoChangedFontRect(RectF &AFontRect) {}

    // 鼠标相关虚函数 [2012-7-16 22:51 by Terry]
    virtual void OnMouseEnterWidget(void);
    virtual void OnMouseLeaveWidget(void);
    virtual void OnMidMouseWheel(UINT nFlags, short zDelta, int x, int y);
    virtual void OnMouseMoveOnWidget(WPARAM AFlags, int x, int y);
    virtual void OnLButtonDownWidget(WPARAM AFlags, int x, int y);
    virtual void OnLButtonUpWidget(WPARAM AFlags, int x, int y);
    virtual void OnLButtonDbclickWidget(WPARAM AFlags, int x, int y);
    virtual void OnRButtonDownWidget(WPARAM AFlags, int x, int y);
    virtual void OnRButtonUpWidget(WPARAM AFlags, int x, int y);

    // 点击时触发 [2012-7-16 22:52 by Terry]
    virtual void OnClick(const int &x = 0, const int &y = 0);
    virtual void OnRButtonClick(const int &x = 0, const int &y = 0);

    //Timer
    //true: 将再些触发些消息, 否则,删除此定时器
    virtual bool OnTimerMessage(int ATimeID, LPVOID ApData);

    //Anchors属性变化时,由外部调用而被触发
    void CheckAnchors(CxdAnchors* ApAnchors);
    virtual void OnParentPositionChanged(CxdPosition *ApPosition);

    //当自定义消息被接收时
    virtual bool OnCustomMessage(CxdCustomMessageInfo *ApInfo){ return false; }
    // 相对父类的位置 [2012-7-28 10:13 by Terry]
    virtual CRect GetRealRectByParent(void) = 0;
    // 相对实际拥有者的位置 [2012-7-28 10:14 by Terry]
    virtual CRect GetRealRectOnUI(void) = 0;

    // 提供给子控件使用,判断其自身是否需要重绘 [2012-7-16 22:53 by Terry]
    virtual const CRect& GetCurReDrawRect(void) = 0;
    virtual bool IsNeedDraw(const Gdiplus::Rect ASrcRect);
    bool IsNeedDraw(const CRect &ARect) { return IsNeedDraw(Rect(ARect.left, ARect.top, ARect.Width(), ARect.Height())); }

    bool IsEnable(void);

    void ClearTimerInfo(void);
protected:
    CxdCursor    *m_pCursor;
    CxdImageInfo *m_pImgInfo;
    CxdAnchors   *m_pAnchors;
    CxdFontInfo  *m_pFontInfo;
    CxdUIComponent *m_pParent;

    //子控件信息 
    TxdComponentList m_ltComponents;
    bool m_bDrawByParent;

    bool m_bIsWndObject;
    bool m_bForceDrawText;
    bool m_bCreated;
private:
    CxdPosition m_oPosition;  

    //定时器
    typedef list<TxdTimerInfo*> TTimerInfoList;
    typedef TTimerInfoList::iterator TTimerInfoListIT;
    TTimerInfoList m_ltTimerInfo;
    TxdTimerInfo* FindTimerInfo(int ATimeID, bool ADelete);
    

    int m_nAnimateunVisibleElapse;
    REAL m_fUnVisibleTranslate;
    bool m_bCurAnimateUnVisible;
    bool IsCurAnimateUnVisible(void);
    REAL GetCurAnimateTranslate();
    bool m_bMouseInWidget;
    bool m_bAutoMoveForm;
    CxdUIComponent *m_pAutoMoveForm;
      
    TxdGpUIState m_eUIState;
    CxdUIComponent *m_pActiveWidget;
    int m_nCaptionLeft, m_nCaptionTop;

    int m_nOldParentWidth, m_nOldParentHeight;

    CString *m_pStrHintText;

    const DWORD CtAnimateUnVisibleTimerID;
    const DWORD CtClearBufferResTimerID;

    //缓存信息
    Gdiplus::Bitmap *m_pBufBitmap;
    bool m_bHasSubUINotDraw;
    bool IsHasSubUINotDraw(void);
    
    void DoHandleParentSizeChanged(const int &ANewWidth, const int &ANewHeight);   
    void DoHandleHintMessage(bool bActive, CxdUIComponent *ApObject);
public:
    static void CalcScreenPos(Rect &R); //修改X,Y 以适应在屏幕上显示
    static bool IsValideCRect(const CRect &R) { return (R.left >= 0) && (R.top >= 0) && (R.right > R.left) && (R.bottom > R.top); }

    friend VOID CALLBACK _TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
    template<typename TBaseClass> friend class CxdWinControl;
    friend class CxdGraphicsControl;
};