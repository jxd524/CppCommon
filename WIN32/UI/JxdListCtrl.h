/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdListCtrl.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2012-11-21 15:09:54
	LastModify : 2013-11-21 15:09:54
	Description: 
        CxdListCtrl : 列表控件, 支持二级子项, 滚动条可定制
**************************************************************************/
#pragma once;
#include "JxduiBasic.h"
#include "JxdGraphicsControl.h"
#include "JxdBar.h"
#include "JxduiCommon.h"

class CxdListCtrl;
struct TSubItemInfo
{
    CxdGraphicsControl *FpParentCtrl;
    bool FIsVisibleSubCtrls;
    TxdGraphicsCtrlList FSubCtrlsList;
};
typedef vector< TSubItemInfo* > TSubItemList;
typedef TSubItemList::iterator TSubItemListIT;

typedef IxdNotifyEventT<CxdListCtrl*> IxdListCtrlInterface;
typedef IxdNotifyEventT<CxdBar**> IxdSettingBarInterface;
typedef IxdNotifyEventExT</*ApCtrl触发对象*/CxdGraphicsControl*, /*ApCtrlParent*/CxdGraphicsControl*> IxdListItemClickInterface;
typedef IxdNotifyEventExT</* ApParentCtrl父对象*/CxdGraphicsControl*, /* bVisible可见状态*/bool> IxdSubItemVisibleInterface;

class CxdListCtrl: public CxdGraphicsControl
{
public:
    CxdListCtrl();
    ~CxdListCtrl();

    //添加一个列表项, ApItemCtrl 在外部不调用 xdCreate
    bool AddListItem(CxdGraphicsControl *ApItemCtrl, int ADefaultHeight = -1);
    //添加一个列表项激活时显示的控件, ApItemCtrl 在外部不调用 xdCreate, 但要设置 position
    bool AddListItemActiveCtrl(CxdGraphicsControl *ApItemCtrl);

    //添加一个子列表项, ApItemCtrl 在外部不调用 xdCreate
    bool AddSubListItem(CxdGraphicsControl *ApItemParent, CxdGraphicsControl *ApItemCtrl, int ADefaultHeight = -1);
    //添加一个子列表项激活时显示的控件, ApItemCtrl 在外部不调用 xdCreate, 但要设置 position
    bool AddListSubItemActiveCtrl(CxdGraphicsControl *ApItemCtrl);

    void LoopItem(CxdGraphicsControl* ApItemParent, IxdLoopCtrlInterface* ApLoop);

    void ClearAllItem(void);
	void ClearAllActiveItem(void);

    void DeleteItem(CxdGraphicsControl *ApCtrl);
    void DeleteBySubCtrl(CxdGraphicsControl *ApSubCtrl); //由子控件获取父类,根据父控件删除项

    void AllExpand(void);
    void AllContract(void);

	//Scroll
	int  GetAllCtrlsHeight();
	int  GetScrollPos(void);
	void SetScrollPos(const int &APos);

	//判断项的状态
	bool IsItemHasSubItem(CxdGraphicsControl *ApCtrl);
    bool IsExsitsItem(CxdGraphicsControl *ApCtrl); 
	void SetItemExpand(CxdGraphicsControl *ApCtrl);

	//项
	CxdGraphicsControl* GetItem(const int &AID);
    int GetItemCount(void) { return m_ltListItems.size(); }
    int GetItemActiveCtrlCount(void) { return m_ltListItemActive.size(); }
    //子项
	CxdGraphicsControl* GetSubItem(const int &AID, CxdGraphicsControl *ApParentCtrl);
    int GetSubItemCount(void) { return m_ltListSubItems.size(); }
    int GetSubItemActiveCtrlCount(void) { return m_ltListSubItemActive.size(); }

    //子项对应的父项
	CxdGraphicsControl* GetSubItemParentCtrl(CxdGraphicsControl *ApSubCtrl);

	//当项的Position改变时重新计算
	void ReCalcAllSize(void);
	int  GetVerScrollBarWidth(void);

    //属性设置必须在调用操作之前
    PropertyValue( ItemUpDownSpace, int, true );
    PropertyValue( SubItemLeftSpace, int, true );
    PropertyValue( PerPosPiexl, int, true );
    PropertyValue( ClickItemAutoExpand, bool, true );//点击项是, 否是自动伸缩子项

    //接口定义
    PropertyInterface( CreateVerBar, IxdSettingBarInterface );
    PropertyInterface( ListItemClick, IxdListItemClickInterface );//鼠标点击单项的事件
    PropertyInterface( MoveToTop, IxdListCtrlInterface );
    PropertyInterface( MoveToBottom, IxdListCtrlInterface );
    PropertyInterface( SubItemVisible, IxdSubItemVisibleInterface );

    PropertyValue( BorderLineColor, DWORD, true );
protected:
    TxdGraphicsCtrlList m_ltListItems;
    TSubItemList m_ltListSubItems;

    void OnPaintComponent(Graphics &G, const Rect &ADestR);
    void OnMidMouseWheel(UINT nFlags, short zDelta, int x, int y);

    virtual CxdBar* DoCreateVerScrollBar(void);
    virtual void DoDeleteItem(CxdGraphicsControl *ApItem){}
    virtual void DoDeleteSubItem(CxdGraphicsControl *ApItem){}
	virtual void DoSetActiveCtrlPos(CxdGraphicsControl *ApActiveCtrl, const int &AHeight){}

    TSubItemInfo* FindSubItemInfo(CxdGraphicsControl *ApItemParent, bool bDel);
    TSubItemInfo* FindSubItemInfoBySub(CxdGraphicsControl *ApSubItem);

    void SetScrollBarPos(CxdGraphicsControl *ApCtrl);
private:
    IxdComponentInterface *m_pItItemClick;
    IxdComponentInterface *m_pItItemStateChanged;
    IxdComponentInterface *m_pItSubItemClick;
    IxdComponentInterface *m_pItSubItemStateChanged;
    //项
    void DoItemClick(CxdUIComponent* ApItem);

    //子类
    void SetSubItemCtrlsVisible(TxdGraphicsCtrlListIT &AitParent, TSubItemInfo *ApSubInfo, bool bSetVisible);
    void DoSubItemClick(CxdUIComponent* ApItem);

    //当项处于激活状态时
    TxdGraphicsCtrlList m_ltListItemActive;
    void DoItemStateChanged(CxdUIComponent* ApItem);

    //当子项处于激活状态时
    TxdGraphicsCtrlList m_ltListSubItemActive;
    void DoSubItemStateChanged(CxdUIComponent* ApItem);

    CxdGraphicsControl *m_pCurActiveCtrl;
    CxdGraphicsControl *m_pCurSubActiveCtrl;
    bool AddActiveCtrl(TxdGraphicsCtrlList &AList, CxdGraphicsControl *ApItemCtrl);
    void ResetActiveParent(TxdGraphicsCtrlList &AList, CxdGraphicsControl *ApParent);

    int MoveItem(CxdGraphicsControl *ApItem, int AItemY, TSubItemInfo *pInfo = NULL);

    void SetAllCtrlWidth(int AOffsetWidth);
    void SetAllCtrlHeight(int AOffsetHeight);

    //滚动条
    typedef CxdNotifyEventEx3T<CxdListCtrl, /*pSender*/CxdBar*, /*bChangedByMouse*/bool, /*nOldBarPos*/int> CxdListCtrlBarPosChangedEvent;
    int m_nAllCtrlsHeight;
    CxdBar *m_pVerScrollBar;
    void CalcAllCtrlsHeight(void);    
    void DoVerBarScroll(CxdBar* pSender, bool bChangedByMouse, int nOldBarPos);
    void CheckVerScrollBar(void);
    void ReSettingScrollInfo(int ATop);

    const int CtDeleteCtrlID;
    bool OnCustomMessage(CxdCustomMessageInfo *pInfo);

    void DoPositionChanged( CxdPosition *ApPosition );
};