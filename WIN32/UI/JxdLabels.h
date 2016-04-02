/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdLabels.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2012-11-21 15:06:47
	LastModify : 2013-11-21 15:06:47
	Description: 
        CxdLabel   : 不处理鼠标状态变化时,界面的重绘
        CxdLabelEx : 支持鼠标激活及按下时,字体颜色
**************************************************************************/
#pragma once;

#include "JxdGraphicsControl.h"

class CxdLabel: public CxdGraphicsControl
{
public:
    CtConst xdGetClassName(void) { return CtxdLable; }
protected:
    bool OnUIStateChanged(const TxdGpUIState &ANewSate) { return CxdUIComponent::OnUIStateChanged(ANewSate); }
};

class CxdLabelEx: public CxdGraphicsControl
{
public:
    CxdLabelEx(): CxdGraphicsControl() { m_FontActiveColor = m_FontDownColor = 0xFF000000;}

    PropertyValue(FontActiveColor, DWORD, true );
    PropertyValue(FontDownColor, DWORD, true );
protected:
    void DoChangedFontColor(DWORD &AFontColor)
    {
        switch ( GetCurUISate() )
        {
        case uiActive:
            AFontColor = m_FontActiveColor;
            break;
        case uiDown:
            AFontColor = m_FontDownColor;
            break;
        }
    }
};