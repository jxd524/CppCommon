/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdQuiltCtrl.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-12-23 17:58:00
	LastModify : 
	Description: 
        CxdQuiltCtrl: �ٲ����ؼ�
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

    //��ӿؼ�
    int  AddCol(int AWidth);
    bool AddCtrl(int AIndex, CxdGraphicsControl* ApCtrl, int ACtrlHeight = -1);
    //�Զ������ʺϵ�������ӿؼ�,����, �������
    bool AutoAddCtrl(CxdGraphicsControl* ApCtrl, int AWidth, int AHeight);

    //ɾ��ָ���ؼ�
    bool DelCtrl(CxdGraphicsControl* ApCtrl);
    void DelCtrl(int AColIndex); //ɾ��һ����
    bool DelCtrl(int AColIndex, int APos);
    void Clear(void);
    void ClearItems(void);

    void LoopItem(IxdLoopCtrlInterface *ApLoop);

    PropertyValue( AnimateOnDelCtrl, bool, true );

    //OnCreated֮ǰ����
    PropertyValue( PerPosPiexl, int, !IsCreated() );
    PropertyValue( LeftRightSpace, int, !IsCreated() );
    PropertyValue( UpDownSpace, int, !IsCreated() );

    //��ǰ���
    PropertyGetValue( CurMaxHeight, int );

    /*�¼�*/
    //����Ҫ������ֱ������ʱ
    PropertyInterface( CreateVerBar, IxdSettingBarInterface );

    //�����������������������ʱ
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