/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdListCtrl.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2012-11-21 15:09:54
	LastModify : 2013-11-21 15:09:54
	Description: 
        CxdListCtrl : �б�ؼ�, ֧�ֶ�������, �������ɶ���
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
typedef IxdNotifyEventExT</*ApCtrl��������*/CxdGraphicsControl*, /*ApCtrlParent*/CxdGraphicsControl*> IxdListItemClickInterface;
typedef IxdNotifyEventExT</* ApParentCtrl������*/CxdGraphicsControl*, /* bVisible�ɼ�״̬*/bool> IxdSubItemVisibleInterface;

class CxdListCtrl: public CxdGraphicsControl
{
public:
    CxdListCtrl();
    ~CxdListCtrl();

    //���һ���б���, ApItemCtrl ���ⲿ������ xdCreate
    bool AddListItem(CxdGraphicsControl *ApItemCtrl, int ADefaultHeight = -1);
    //���һ���б����ʱ��ʾ�Ŀؼ�, ApItemCtrl ���ⲿ������ xdCreate, ��Ҫ���� position
    bool AddListItemActiveCtrl(CxdGraphicsControl *ApItemCtrl);

    //���һ�����б���, ApItemCtrl ���ⲿ������ xdCreate
    bool AddSubListItem(CxdGraphicsControl *ApItemParent, CxdGraphicsControl *ApItemCtrl, int ADefaultHeight = -1);
    //���һ�����б����ʱ��ʾ�Ŀؼ�, ApItemCtrl ���ⲿ������ xdCreate, ��Ҫ���� position
    bool AddListSubItemActiveCtrl(CxdGraphicsControl *ApItemCtrl);

    void LoopItem(CxdGraphicsControl* ApItemParent, IxdLoopCtrlInterface* ApLoop);

    void ClearAllItem(void);
	void ClearAllActiveItem(void);

    void DeleteItem(CxdGraphicsControl *ApCtrl);
    void DeleteBySubCtrl(CxdGraphicsControl *ApSubCtrl); //���ӿؼ���ȡ����,���ݸ��ؼ�ɾ����

    void AllExpand(void);
    void AllContract(void);

	//Scroll
	int  GetAllCtrlsHeight();
	int  GetScrollPos(void);
	void SetScrollPos(const int &APos);

	//�ж����״̬
	bool IsItemHasSubItem(CxdGraphicsControl *ApCtrl);
    bool IsExsitsItem(CxdGraphicsControl *ApCtrl); 
	void SetItemExpand(CxdGraphicsControl *ApCtrl);

	//��
	CxdGraphicsControl* GetItem(const int &AID);
    int GetItemCount(void) { return m_ltListItems.size(); }
    int GetItemActiveCtrlCount(void) { return m_ltListItemActive.size(); }
    //����
	CxdGraphicsControl* GetSubItem(const int &AID, CxdGraphicsControl *ApParentCtrl);
    int GetSubItemCount(void) { return m_ltListSubItems.size(); }
    int GetSubItemActiveCtrlCount(void) { return m_ltListSubItemActive.size(); }

    //�����Ӧ�ĸ���
	CxdGraphicsControl* GetSubItemParentCtrl(CxdGraphicsControl *ApSubCtrl);

	//�����Position�ı�ʱ���¼���
	void ReCalcAllSize(void);
	int  GetVerScrollBarWidth(void);

    //�������ñ����ڵ��ò���֮ǰ
    PropertyValue( ItemUpDownSpace, int, true );
    PropertyValue( SubItemLeftSpace, int, true );
    PropertyValue( PerPosPiexl, int, true );
    PropertyValue( ClickItemAutoExpand, bool, true );//�������, �����Զ���������

    //�ӿڶ���
    PropertyInterface( CreateVerBar, IxdSettingBarInterface );
    PropertyInterface( ListItemClick, IxdListItemClickInterface );//�����������¼�
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
    //��
    void DoItemClick(CxdUIComponent* ApItem);

    //����
    void SetSubItemCtrlsVisible(TxdGraphicsCtrlListIT &AitParent, TSubItemInfo *ApSubInfo, bool bSetVisible);
    void DoSubItemClick(CxdUIComponent* ApItem);

    //����ڼ���״̬ʱ
    TxdGraphicsCtrlList m_ltListItemActive;
    void DoItemStateChanged(CxdUIComponent* ApItem);

    //������ڼ���״̬ʱ
    TxdGraphicsCtrlList m_ltListSubItemActive;
    void DoSubItemStateChanged(CxdUIComponent* ApItem);

    CxdGraphicsControl *m_pCurActiveCtrl;
    CxdGraphicsControl *m_pCurSubActiveCtrl;
    bool AddActiveCtrl(TxdGraphicsCtrlList &AList, CxdGraphicsControl *ApItemCtrl);
    void ResetActiveParent(TxdGraphicsCtrlList &AList, CxdGraphicsControl *ApParent);

    int MoveItem(CxdGraphicsControl *ApItem, int AItemY, TSubItemInfo *pInfo = NULL);

    void SetAllCtrlWidth(int AOffsetWidth);
    void SetAllCtrlHeight(int AOffsetHeight);

    //������
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