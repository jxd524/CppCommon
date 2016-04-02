/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdBar.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:18:20
	Description: 
**************************************************************************/
#pragma once
#include "JxdGraphicsControl.h"
#include <vector>
using namespace std;

#include "JxdEvent.h"
#include "JxdMacro.h"

typedef IxdNotifyEventEx3T</*pSender*/CxdBar*, /*bChangedByMouse*/bool, /*nOldBarPos*/int> IxdBarPosChangedInterface;
class CxdBar: public CxdGraphicsControl
{
public:
    CxdBar(const int &AMinSliderSize = 20);
    ~CxdBar();

    CtConst xdGetClassName(void) { return CtxdBar; }

    //信息改变后,可调用重新计算
    void CalcBarInfo(void);

    // 当前滑块位置 [2012-8-6 14:54 by Terry]
    int  GetBarPos(void) { return m_nPos; }
    bool SetBarPos(const int &APos);

    int  GetMin(void) { return m_nMin; }
    void SetMin(const int &AMin);
    int  GetMax(void) { return m_nMax; }
    void SetMax(const int &AMax);

    TxdBarStyle GetBarStyle(void) { return m_bsStyle; }
    void        SetBarStyle(TxdBarStyle AStyle) { m_bsStyle = AStyle; }

    int GetMaxScrllPos(void) { return m_nMax - m_nMin; }

    //按住上下或左右按钮时,每次自动添加ScrollPos
    int  GetTimerChangedCount(void) { return m_nTimerChangedCount; }
    void SetTimerChangedCount(const int &AChangedValue) { m_nTimerChangedCount = AChangedValue; }

    //滑块大小设置, <= 0 表示自动计算
    int  GetSliderSize(void) { return m_nSettingSliderSize; }
    void SetSliderSize(const int &ASize) { m_nSettingSliderSize = ASize; }

    //按钮设置
    bool GetShowButtonVisible(void) { return m_bShowCtrlButton; }
    void SetShowButtonVisible(bool bVisible) { m_bShowCtrlButton = bVisible; }

    //Event
    PropertyInterface( BarPosChanged, IxdBarPosChangedInterface );
protected:
    void DoDrawUI(Graphics &G, const Rect &ADestR);
    void OnCreated(void);

    void DrawVerBar(Graphics &G, ImageAttributes *ApImgAtt);
    void DrawHorBar(Graphics &G, ImageAttributes *ApImgAtt);

    bool OnUIStateChanged(const TxdGpUIState &ANewSate);
    void DoPositionChanged(const LPVOID &ASender);
    void OnParentPositionChanged(CxdPosition* ApPosition);
    bool OnTimerMessage(int ATimeID, LPVOID ApData);
    void OnMouseLeaveWidget();
    void OnMouseMoveOnWidget(UINT AFlags, int x, int y);
    void OnLButtonDownWidget(UINT AFlags, int x, int y);
    void OnMidMouseWheel(UINT nFlags, short zDelta, int x, int y);
    void OnLButtonUpWidget(UINT AFlags, int x, int y);
    void OnClick(const int &x /* = 0 */, const int &y /* = 0 */);
private:
    TxdBarStyle m_bsStyle;
    void CheckImageBar(void); 
    int m_nImgBlockWidth;
    int m_nImgBlockHeight;
    bool m_bShowCtrlButton;
    int m_nMin, m_nMax, m_nPos;
    int m_nSliderSize; // 滑块的高/宽
    int m_nSliderPos;
    double m_dPerPosPixel;
    double m_dSettingPosPixel;
    TxdBarMousePos m_mpMousePos;
    CPoint m_ptDown;
    const int CtAutoChangedPosTimerID;
    const int CtMinSliderSize;  //滑块最小的大小

    int m_nTimerChangedCount;
    bool m_bHandleBkState;
    int m_nSettingSliderSize; //<= 0: 计算滑块大小,否则滑块大小为固定的


    Rect GetSliderRect(void);

    void CalcMousePos(const int &x, const int &y);
    void ChangedNewMousePos(TxdBarMousePos Amp);
    void InvalieByMP(TxdBarMousePos Amp);
    void CalcSliderInfo(void);

    bool DoChangedBarPos(const int &ANewPos, bool AbByMouse);
};