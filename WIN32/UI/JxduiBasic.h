/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxduiBasic.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2012-7-21 09:26
	LastModify : 2013-10-28 15:00:58
	Description: CxdUIComponent
        ��������ؼ�ʵ��, ���Ʒ�ʽ: 
            �Ȼ��Ʊ���,�ٻ����ӿؼ�, �ӿؼ��Ļ������ظ��˹���
        ÿ���ؼ�����: 
            Position -> λ��
        �ӿڵ�ʹ��˵��: ���ⲿ new һ���ӿ�, UI�⸺���ͷ�

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
    //�ṩ���ⲿ����ʹ�õ���չ
    INT64 Tag;

    CxdUIComponent();
    virtual ~CxdUIComponent();

    virtual CtConst xdGetClassName(void) { return CtxdUIComponent; }
    virtual bool    xdCreate(const int &Ax, const int &Ay, const int &AWidth, const int &AHeight, CxdUIComponent *ApParent = NULL, const TCHAR *ApWindowName = NULL) = 0;

    inline bool  IsCreated(void) { return m_bCreated; }
    inline bool  IsWndObject(void) { return m_bIsWndObject; }
    virtual bool IsCanMoveObject(void) = 0;

    //���к����Ӷ���
    inline CxdPosition*  Position(void) { return &m_oPosition; } 
    inline CxdImageInfo* ImageInfo(void) { return xdCreateObject(m_pImgInfo); }
    inline CxdFontInfo*  FontInfo(void) { return xdCreateObject(m_pFontInfo); }
    inline CxdCursor*    Cursor(void){ return xdCreateObject(m_pCursor); }        
    CxdAnchors*          Anchors(void);

    //�ж϶������´���
    inline bool          IsExsitsImageInfo() { return m_pImgInfo != NULL; }
    inline bool          IsExsitsFontInfo() { return m_pFontInfo != NULL; }
    inline bool          IsExsitsCursor() { return m_pCursor != NULL; }
    inline bool          IsExsitsAnchor(void) { return m_pAnchors != NULL; }

    //�ṩ��һ�������Ӷ���ķ���
    void SetImageInfo(CxdImageInfo *ApNewImg, bool AbResOnly = true);
    void SetFontInfo(CxdFontInfo *ApNewFont, bool AbResOnly = true);

    // �ɼ��� [2012-7-16 22:48 by Terry]
    bool GetVisible();
    void SetVisible(bool bVisible);

    // ������ [2012-7-16 22:48 by Terry]
    virtual bool GetEnable() = 0;
    virtual void SetEnable(bool bEnable) = 0;

    // ��ʾ��Ϣ, ��Ҫ���� JxduiCommon �еĺ���: SetHintInterface [2012-8-3 14:59 by Terry]
    CString* GetHintText(void) { return m_pStrHintText; }
    void     SetHintText(const TCHAR *ApHintText);

    // Caption��Ϣ [2012-8-1 17:24 by Terry]
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

    // �ػ� [2012-7-16 22:48 by Terry]    
    virtual void Invalidate(void);
    virtual void InvalidateGpRect(const Rect &ARect);
    virtual void InvalidateRect(CRect *ApRect = NULL) = 0;

    // ��ǰ״̬ [2012-7-16 22:48 by Terry]
    virtual TxdGpUIState GetCurUISate(void) { return m_eUIState; }
    inline bool IsMouseActive(void) { return m_bMouseInWidget; }

    // ��ȡ����,��һ����ӵ�о������ [2012-7-21 09:46 by Terry]
    virtual CxdUIComponent* GetComponentParent(void) { return m_pParent; }

    // ��ȡӵ����, һ����ӵ�о������ [2012-7-21 09:48 by Terry]
    virtual CxdUIComponent* GetOwner(void) { return NULL; }
    virtual HWND GetOwnerHandle(void);

    //�Զ�����Ϣ
    void PostCustomMessage(CxdCustomMessageInfo *ApInfo);
    void SendCustomMessage(CxdCustomMessageInfo *ApInfo);

    //Event
    // ���� [2012-7-16 22:49 by Terry]
    void Click(void) { OnClick(); }
    void RButtonClick(void) { OnRButtonClick(); }

    //��ʱ��, ������ OnTimerMessage
    virtual void StartTimer(int ATimeID, int ATimeElapse, LPVOID AData = NULL);
    virtual void EndTimer(int ATimeID);

    // �Ƿ��ƶ�ָ������ [2012-7-16 22:54 by Terry]
    bool  GetAutoMoveForm(void) { return m_bAutoMoveForm; }
    void  SetAutoMoveForm(bool bAutoMove) { m_bAutoMoveForm = bAutoMove; }
    CxdUIComponent* GetMoveFormObject(void) { return m_pAutoMoveForm; }
    void  SetMoveFormObject(CxdUIComponent *ApObject) { m_pAutoMoveForm = ApObject; }

    //��ǰ�Ƿ����ڲ���������
    virtual bool IsCurCapture(CxdUIComponent *ApObject) = 0;

    //��ӽ����Ķ�����������ڽ����ڲ�����,�ⲿ�����ٽ����ͷ�
    bool AddSubComponent(CxdUIComponent *ApObject);
    bool DeleteSubComponent(CxdUIComponent *ApObject);
    void ClearSubComponents(void);
    //TCompoentPList* GetSubComponentList(void){ return &m_ltComponents; }
    //int GetSubComponentCount(void) { return (int)m_ltComponents.size(); }

    // ���ص�ǰ����״̬�Ĳ��� [2012-7-22 16:01 by Terry]
    inline CxdUIComponent* GetCurActiveWidget(void) { return m_pActiveWidget; }
    // �������ļ���״̬���� [2012-7-22 17:32 by Terry]
    CxdUIComponent* GetTopActiveWidget(void) { if (Assigned(m_pActiveWidget)) return m_pActiveWidget->GetTopActiveWidget(); return this; }

    //��ȡ����ͼƬ
    Gdiplus::Bitmap* GetBufferBitmap(void);
    void             DeferClearBufferBitmap(void); //�ӳ��ͷ�
    void             ClearBufferBitmap(void); // �����ͷ� [2012-9-5 21:20 by Terry]
    virtual bool     CopyBufferDC(HDC AhDestDC, const Rect &ACopyRect);
    virtual bool     CopyBufferBitmap(Gdiplus::Bitmap *ApDestBmp, const Gdiplus::Rect &ACopyRect);

    //��Ҫ��ʾ��ʾ��Ϣʱ������, ���� false ʱ �� GetHintText Ϊ��ʱ,����ʾ
    //һ���� JxdHintWindow ���� 
    virtual bool OnHintMessage(void);

    //���Զ��� 
    PropertyValue( BufferBitmapTimeout, int, true ); //����ʱ��
    PropertyValue( VisibleOnParentActive, bool, true ); //�����༤��ʱ�ɼ�
    PropertyValue( AnimateUnVisible, bool, true ); //���ɼ�ʱʹ�ö�̬Ч��
    PropertyValue( DrawGrayOnUnEnable, bool, true ); //������ʱ�Զ���ɫ
    PropertyValue( CaptureOnMouseDown, bool, true ); //��갴��ʱ, �Ƿ񲶻�������

    //�¼����Զ���
    PropertyInterface( Click, IxdComponentInterface );  //���
    PropertyInterface( RButtonClick, IxdComponentInterface ); //R ���
    PropertyInterface( DblClick, IxdComponentInterface ); //˫��
    PropertyInterface( UIStateChanged, IxdComponentInterface ); //UI״̬�ı�ʱ
    PropertyInterface( OnHintMessage, IxdComponentInterface ); //����ʾ��ʾ��Ϣʱ
protected:
    /*�ɸ���������������: ����ʽ*/
    virtual void OnCreated(void); // ���ⲿ����xdCreate�ɹ�֮�󱻵��� [2012-7-17 14:02 by Terry]

    // ���Ʊ��� �ɴ˸������ [2012-7-16 22:51 by Terry]
    virtual void OnPaintComponent(Graphics &G, const Rect &ADestR);

    //���Ʒַ����� OnPaintWidget -> DoDrawUI 
    virtual void DoDrawUI(Graphics &G, const Rect &ADestR);    
    virtual void DoDrawSubjoinBK(Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt) {};

    //��������
    virtual void DoDrawText(Graphics &G, const Rect &AuiDestR);    
    //���Ʒַ����� OnPaintWidget -> DoDrawSubComponent
    virtual void DoDrawSubComponent(Graphics &G, const Rect &AuiDestR);

    //��ȡ��������
    virtual ImageAttributes* GetDrawBKImageAttributes(void);

    // UI״̬�ı�ʱ������ [2012-7-16 22:51 by Terry]
    virtual bool OnUIStateChanged(const TxdGpUIState &ANewSate);
    virtual void DoSettingVisible(bool bVisible) = 0;
    virtual bool DoGetVisible(void) = 0;
    virtual void DoChangedFontColor(DWORD &AFontColor){}
    virtual void DoChangedFontRect(RectF &AFontRect) {}

    // �������麯�� [2012-7-16 22:51 by Terry]
    virtual void OnMouseEnterWidget(void);
    virtual void OnMouseLeaveWidget(void);
    virtual void OnMidMouseWheel(UINT nFlags, short zDelta, int x, int y);
    virtual void OnMouseMoveOnWidget(WPARAM AFlags, int x, int y);
    virtual void OnLButtonDownWidget(WPARAM AFlags, int x, int y);
    virtual void OnLButtonUpWidget(WPARAM AFlags, int x, int y);
    virtual void OnLButtonDbclickWidget(WPARAM AFlags, int x, int y);
    virtual void OnRButtonDownWidget(WPARAM AFlags, int x, int y);
    virtual void OnRButtonUpWidget(WPARAM AFlags, int x, int y);

    // ���ʱ���� [2012-7-16 22:52 by Terry]
    virtual void OnClick(const int &x = 0, const int &y = 0);
    virtual void OnRButtonClick(const int &x = 0, const int &y = 0);

    //Timer
    //true: ����Щ����Щ��Ϣ, ����,ɾ���˶�ʱ��
    virtual bool OnTimerMessage(int ATimeID, LPVOID ApData);

    //Anchors���Ա仯ʱ,���ⲿ���ö�������
    void CheckAnchors(CxdAnchors* ApAnchors);
    virtual void OnParentPositionChanged(CxdPosition *ApPosition);

    //���Զ�����Ϣ������ʱ
    virtual bool OnCustomMessage(CxdCustomMessageInfo *ApInfo){ return false; }
    // ��Ը����λ�� [2012-7-28 10:13 by Terry]
    virtual CRect GetRealRectByParent(void) = 0;
    // ���ʵ��ӵ���ߵ�λ�� [2012-7-28 10:14 by Terry]
    virtual CRect GetRealRectOnUI(void) = 0;

    // �ṩ���ӿؼ�ʹ��,�ж��������Ƿ���Ҫ�ػ� [2012-7-16 22:53 by Terry]
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

    //�ӿؼ���Ϣ 
    TxdComponentList m_ltComponents;
    bool m_bDrawByParent;

    bool m_bIsWndObject;
    bool m_bForceDrawText;
    bool m_bCreated;
private:
    CxdPosition m_oPosition;  

    //��ʱ��
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

    //������Ϣ
    Gdiplus::Bitmap *m_pBufBitmap;
    bool m_bHasSubUINotDraw;
    bool IsHasSubUINotDraw(void);
    
    void DoHandleParentSizeChanged(const int &ANewWidth, const int &ANewHeight);   
    void DoHandleHintMessage(bool bActive, CxdUIComponent *ApObject);
public:
    static void CalcScreenPos(Rect &R); //�޸�X,Y ����Ӧ����Ļ����ʾ
    static bool IsValideCRect(const CRect &R) { return (R.left >= 0) && (R.top >= 0) && (R.right > R.left) && (R.bottom > R.top); }

    friend VOID CALLBACK _TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
    template<typename TBaseClass> friend class CxdWinControl;
    friend class CxdGraphicsControl;
};