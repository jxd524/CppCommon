/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdGraphicsControl.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:17:09
	Description: 
**************************************************************************/
#pragma once;
#include "JxduiBasic.h"

struct TImgIconInfo 
{
    TImgIconInfo(int AIconIndex = 0, int AOffsetX = 5, int AOffsetY = 5, 
        int AOffsetXOnActive = 0, int AOffsetYOnActive = 0, int AOffsetXOnDown = 1, int AOffsetYOnDown = 1)
    {
        FIconIndex = AIconIndex;
        FOffsetX = AOffsetX;
        FOffsetY = AOffsetY;
        FOffsetXOnActive = AOffsetXOnActive;
        FOffsetYOnActive = AOffsetYOnActive;
        FOffsetXOnDown = AOffsetXOnDown;
        FOffsetYOnDown = AOffsetYOnDown;
    }
    int FIconIndex;
    int FOffsetX;
    int FOffsetY;
    int FOffsetXOnActive, FOffsetYOnActive;
    int FOffsetXOnDown, FOffsetYOnDown;
};
typedef vector< TImgIconInfo* > TImgIconInfoList;
typedef TImgIconInfoList::iterator TImgIconInfoListIT;
typedef IxdNotifyEventT<CxdGraphicsControl*> IxdGraphicsControlInterface;
//无句柄类
class CxdGraphicsControl: public CxdUIComponent
{
public:
    CxdGraphicsControl();
    ~CxdGraphicsControl();

    CtConst xdGetClassName(void) { return CtxdGraphicsControl; }
    bool xdCreate(const int &Ax, const int &Ay, const int &AWidth, const int &AHeight, CxdUIComponent *ApParent = NULL, const TCHAR *ApWindowName = NULL);

    //Icon image setting
    inline CxdImageInfo* ImageIconInfo(void) { return xdCreateObject(m_pImgIconInfo); }
    void SetImageIconInfo(CxdImageInfo *ApNewImg, bool AbResOnly = true);
    int  AddIconInfo(const TImgIconInfo &AInfo);
    bool DeleteIconInfo(const UINT &AIconIndex);
    TImgIconInfo* GetIconInfo(const UINT &AIconIndex);
    void ClearIconInfos(void);

    void ReSetParent(CxdUIComponent *ApParent);

    bool GetEnable(){ return m_bEnable; }
    void SetEnable(bool bEnable){ m_bEnable = bEnable;}

    bool IsDrawCaption(void){ return Assigned(m_pstrCaption) && (!m_pstrCaption->IsEmpty()); }
    CString GetCaption(void); 
    void SetCaption(const TCHAR* ApCaption); 

    void InvalidateRect(CRect *ApRect = NULL);
    bool IsCurCapture(CxdUIComponent *ApObject) { return m_pOwner->IsCurCapture(ApObject); }
    bool IsCanMoveObject(void) { return true; }

    // 获取拥有者, 一定是拥有句柄的类 [2012-7-21 14:47 by Terry]
    CxdUIComponent* GetOwner(void) { return m_pOwner; }

    // 获取实际位置 [2012-7-21 14:46 by Terry]
    CRect GetRealRectByParent(void);
    CRect GetRealRectOnUI(void);     
protected:
    CxdUIComponent *m_pOwner;
    // 外部改变位置后,调用 Position 的 NotifyChanged 时被触发 [2012-7-21 14:43 by Terry]
    virtual void DoPositionChanged(CxdPosition* ApSender); 

    // 重载父类信息 [2012-7-22 16:12 by Terry]
    bool IsNeedDraw(const Gdiplus::Rect ASrcRect);
    bool OnUIStateChanged(const TxdGpUIState &ANewSate);
    void DoSettingVisible(bool bVisible){ m_bVisible = bVisible; }
    bool DoGetVisible(void) { return m_bVisible; }
    const CRect& GetCurReDrawRect(void) { return m_pOwner->GetCurReDrawRect(); }
    void DoDrawSubjoinBK(Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt);
private:
    bool m_bVisible;
    bool m_bEnable;
    CString *m_pstrCaption;

    CxdImageInfo *m_pImgIconInfo;
    TImgIconInfoList m_ltImgIconInfoList;
};