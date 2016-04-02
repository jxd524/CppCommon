/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdComboBox.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:18:09
	Description: 
**************************************************************************/
#pragma once;

#include "JxdMacro.h"
#include "JxdEdit.h"
#include "JxdEvent.h"
#include "JxdPopupMenu.h"
class CxdGraphicsControl;

class CxdComboBox: public CxdEdit
{
public:
	CxdComboBox();
	~CxdComboBox();
	CtConst xdGetClassName(void) { return CtxdComboBox; }

    CxdImageInfo* ImageDrawDown(void);
	CxdPopupMenu* GetComboList(void) { return &m_oComboMenu; }

    bool GetDrawDownPositionOnRight() { return m_bOnRight; }
	void SetDrawDownPosition(bool AbOnRight);

    PropertyInterface( ListSelected, IxdMenuItemInterface );
protected:
	void  OnCreated();
	CRect CalcEditRect(void);
	void  DrawEditSubComponentBK(Graphics &G);
private:
	bool m_bOnRight;
	int m_nCtrlWidth;
	int m_nCtrlSpace;
	int m_nMaxHeight;
	CxdGraphicsControl *m_pBtnDrawDown;
	CxdPopupMenu m_oComboMenu;
	void DoBtnDrawDownClick(CxdUIComponent* ApBtn);
};