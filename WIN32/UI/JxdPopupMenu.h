/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdPopupMenu.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:16:36
	Description: 
**************************************************************************/
#pragma once;

#include "JxdEvent.h"
#include "JxdMacro.h"
#include "JxduiCommon.h"
#include "JxdForm.h"
#include "JxdGraphicsControl.h"
#include "JxdListCtrl.h"

class CxdPopupMenu;
class CxdPopuItemCtrl;
class CxdPopupForm;
class CxdUIComponent;

struct TPopupFormInfo
{
    CxdGraphicsControl *FpItemCtrl;
    CxdPopupForm *FpItemSubForm;
    CxdPopupForm *FpItemParent;
};
typedef vector<TPopupFormInfo*> TPopupFormInfoList;
typedef TPopupFormInfoList::iterator TPopupFormInfoListIT;
typedef IxdNotifyEventEx3T<LPVOID, UINT, CWnd*> IxdWndActiveChangedInterface;

//CxdPopuItemCtrl
class CxdPopuItemCtrl: public CxdGraphicsControl
{
public:
    CxdPopuItemCtrl( CxdPopupMenu *ApOwner ); 
protected:
    CxdPopupMenu *m_pOwnerMenu;
    void DoChangedFontColor(DWORD &AFontColor);
    void DoDrawSubjoinBK(Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt);
};

class CxdPopupForm: public CxdForm
{
public:
    CxdPopupForm();
    ~CxdPopupForm();
    CtConst xdGetClassName(void) { return CtxdPopupForm; }

	//设置listctrl距离窗口的边距
	PropertyValue( CtrlLeftSpace, int, true );
	PropertyValue( CtrlTopSpace, int, true );
	PropertyValue( CtrlRightSpace, int, true );
	PropertyValue( CtrlBottomSpace, int, true );
    void NotifySpaceChanged(void);

    //事件
    PropertyInterface( WndActiveChanged, IxdWndActiveChangedInterface );
    PropertyInterface( ItemClick, IxdGraphicsControlInterface );

	void AddListCtrlItem(CxdGraphicsControl *Item, int nHeight = -1);
	void AddExtendCtrlItem(CxdGraphicsControl *);
public:
    bool FShowOnRight;
    bool FShowOnTop;
protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	void OnCreated();
    TxdGpCloseStyle OnCloseForm(void){ return csDestroy; }
    void OnGetCreateInfo(CString &ADefaultWndClassName, CString &ANewClassName, DWORD &AdwStyle, DWORD &AdwExStyle);
	void DoItemClick( CxdGraphicsControl* ApCtrl, CxdGraphicsControl* ApCtrlParent );
	void DoExtendItemClick(CxdUIComponent *ApParam);
private:
    CxdListCtrl *m_pPopupCtrlList;
};

//CxdPopupMenu
class CxdPopupMenu
{
public:
    CxdPopupMenu();
    ~CxdPopupMenu();

    void InitPopupItems(TPopupMenuItemInfos *ApItems, const int &ACount);
    void AddPopupItem(const TPopupMenuItem *ApItem, TPopupMenuItem *ApParent = NULL);
    void Clear(void);
    TPopupMenuItem* PopuMenu(CxdUIComponent *ApOwner, const int &ALeft, const int &ATop, IxdMenuItemInterface *ApBeforePopupItem = NULL);

	//common listctrlUI setting
	CxdImageInfo* ImageStyle(void) { return xdCreateObject(m_pImgStyle); }
	CxdImageInfo* ImageBK(void) { return xdCreateObject(m_pImgBK); }
	CxdImageInfo* ImageItem(void) { return xdCreateObject(m_pImgNormalItem); }
	CxdFontInfo*  FontInfo(void) { return xdCreateObject(m_pFontInfo); }
	CxdImageInfo* ImageSeparator(void) { return xdCreateObject(m_pImgSeparator); }
	CxdImageInfo* ImageIcons(void) { return xdCreateObject(m_pImgIcons); }
	CxdImageInfo* ImageSubItem(void) { return xdCreateObject(m_pImgSubItems); }

    //Icon位置设置
    PropertyValue( CaptionIconSpace, int, true );
    PropertyValue( DrawIconWidth, int, true );
    PropertyValue( DrawIconLeftSpace, int, true );
    PropertyValue( DrawIconTopSpace, int, true);

    PropertyValue( FixedWidth, int, true ); //设置固定宽度
    PropertyValue( MinHeight, int, true ); //最小高度
    PropertyValue( FormSpace, int, true ); //多级菜单时有效, 父菜单与子菜单之间的间隔
    PropertyValue( CaptionTopSpace, int, true );

    PropertyValue( ItemActiveFontColor, DWORD, true );
protected:
    virtual CxdPopupForm*       CreatePopupForm() { return new CxdPopupForm(); }
    virtual CxdGraphicsControl* CreatePopupItem(){ return new CxdPopuItemCtrl(this); }
	//事件响应
	void DoMainItemActiveChanged(LPVOID ApSender, UINT AState, CWnd* ApWndOther);
	void DoItemClick( CxdGraphicsControl* ApCtrl );
	void DoSubItemUIChanged(CxdUIComponent* ApSender);

	//子菜单相关
	void CreateSubPopupMenu(CxdGraphicsControl *ApCtrl);    
	TPopupFormInfo* FindSubPopupItem(CxdGraphicsControl *ApCtrl, bool bDelete);
	bool IsSubPopupForm(const HWND &AHandle);
	void ClearSubPopupMenu(CxdPopupForm *ApParent);
	void SetSubPopupUnVisible(void);
	TPopupMenuItem* FindPopupItem(const INT64 &ATag);
	void CalcPopupFormPos(CxdPopupForm *ApCurCalcForm, CRect *ApRect, CxdPopupForm *ApParent = NULL );


	void AddItemToListCtrl(CxdPopupForm *AForm, TPopupMenuItem *ApItemInfo);
    TPopupMenuItem* FindPopupItem(const INT64 &ATag, TxdPopupItemList *ApList);
	void ClearPopupItemList(TxdPopupItemList *ApList, bool bFreeList);

private:
    CxdPopupForm *m_pMainPopupForm;
    TxdPopupItemList m_ltPopupItems;
    TPopupMenuItem *m_pClickItem;

	CxdImageInfo	*m_pImgStyle;      //窗口形状
	CxdImageInfo	*m_pImgBK;         //背景
	CxdImageInfo	*m_pImgNormalItem; //普通项, 非分隔符,非次级菜单
	CxdFontInfo		*m_pFontInfo;      //字体信息
	CxdImageInfo	*m_pImgSeparator;  //分隔符    
	CxdImageInfo	*m_pImgSubItems;   //下级子菜单
    CxdImageInfo	*m_pImgIcons;      //分项中的图标, 第一个图片默认为选中状态

    void DoAddItemToForm(CxdPopupForm *ApForm, TxdPopupItemList *ApList);

    //事件
	IxdMenuItemInterface *m_pOnPopupEvent;

    CxdGraphicsControl *m_pCtrlOld;
    TPopupFormInfoList m_ltSubPopup;
     
    friend class CxdPopuItemCtrl;
    friend class CxdPopupForm;
};
