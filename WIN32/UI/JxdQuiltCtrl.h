/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdQuiltCtrl.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-12-23 17:58:00
	LastModify : 
	Description: 
        CxdQuiltCtrl: 瀑布流控件
**************************************************************************/
#pragma once;

#include "JxduiDefines.h"
#include "JxduiBasic.h"
#include "JxdGraphicsControl.h"
#include "JxdBar.h"
#include "JxduiCommon.h"
#include "JxdMacro.h"

class CxdQuiltCtrl;
struct TxdColInfo
{
    int Width;
    int AllHeight;
    TxdGraphicsCtrlList CtrlList;
};
typedef vector< TxdColInfo* >    TxdColInfoList;
typedef TxdColInfoList::iterator TxdColInfoListIT;

typedef IxdNotifyEventT<CxdBar**> IxdSettingBarInterface;
typedef IxdNotifyEventExT<CxdQuiltCtrl*, bool> IxdQuiltInterface;

class CxdQuiltCtrl: public CxdGraphicsControl
{
public:
    CxdQuiltCtrl();
    ~CxdQuiltCtrl();

    //添加控件
    int  AddCol(int AWidth);
    bool AddCtrl(int AIndex, CxdGraphicsControl* ApCtrl, int ACtrlHeight = -1);
    //自动查找适合的列以添加控件,否则, 添加新列
    bool AutoAddCtrl(CxdGraphicsControl* ApCtrl, int AWidth, int AHeight);

    //删除指定控件
    bool DelCtrl(CxdGraphicsControl* ApCtrl);
    void DelCtrl(int AColIndex); //删除一整列
    bool DelCtrl(int AColIndex, int APos);
    void Clear(void);
    void ClearItems(void);

    void LoopItem(IxdLoopCtrlInterface *ApLoop);

    PropertyValue( AnimateOnDelCtrl, bool, true );

    //OnCreated之前设置
    PropertyValue( PerPosPiexl, int, !IsCreated() );
    PropertyValue( LeftRightSpace, int, !IsCreated() );
    PropertyValue( UpDownSpace, int, !IsCreated() );

    //当前最大
    PropertyGetValue( CurMaxHeight, int );

    /*事件*/
    //当需要创建垂直滚动条时
    PropertyInterface( CreateVerBar, IxdSettingBarInterface );

    //当滚动到最上面或者最下面时
    PropertyInterface( BarMoveTopOrDown, IxdQuiltInterface );
protected:
    TxdColInfoList m_ltCols;
    virtual CxdBar* OnCreateVerScrollBar();
    virtual void OnDeleteCtrl(CxdGraphicsControl *ApCtrl) {}
    void OnMidMouseWheel(UINT nFlags, short zDelta, int x, int y);
private:
    void AddToColCtrl(TxdColInfo *ApInfo, CxdGraphicsControl *ApCtrl);
    void DelCtrlFromCol(TxdColInfo *ApInfo, TxdGraphicsCtrlListIT AIt);
    void CalcXY(TxdColInfo *ApCol, int &x, int &y);
    int  CalcCurMaxColHeight(void);
private:
    CxdBar *m_pVerScrollBar;
    void DoVerBarScroll(CxdBar* pSender, bool bChangedByMouse, int nOldBarPos);
    void CheckVerScrollBar(void);
    void SetCtrlsTop(int ATop);
};