/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdTabControl.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:16:26
	Description: 
**************************************************************************/
#pragma once;

#include "JxdGraphicsControl.h"
#include "JxdButtons.h"

class CxdTabControl: public CxdGraphicsControl
{
public:
    CxdTabControl();
    ~CxdTabControl();

    PropertyValue( LeftSpace, int, true );
    PropertyValue( RightSpace, int, true );
    PropertyValue( TopSpace, int, true );
    PropertyValue( DownSpace, int, true );
    PropertyValue( SpaceSize, int, true );
    PropertyValue( ItemMaxWidth, int, true );


    CxdImageInfo* ImageTabItem(void) { return xdCreateObject(m_pImgTabItem); }
    CxdImageInfo* ImageTabItemClose(void) { return xdCreateObject(m_pImgTabItemClose); }

    CxdImageInfo* ImageAddTab(void) { return m_pBtnAddTab->ImageInfo(); }
    void SetAddTabButtonPos(const int &ATop, const int& AWidth, const int &AHeight);

    int  AddTab(const TCHAR *ApCaption, const UINT64 &ATag);
    void SelTab(const int &AIndex);
    void DelTab(const int &AIndex);
protected:
    void OnCreated();
    CxdImageInfo *m_pImgTabItem;
    CxdImageInfo *m_pImgTabItemClose;
    virtual CxdButton* DoNewTabItem(void) { return new CxdButton; }
    virtual CxdGraphicsControl* DoNewTabItemClose(void) { return new CxdGraphicsControl; }
    virtual bool DoSelecteTab(CxdButton *ApBtn);
    virtual bool DoCloseTab(CxdButton *ApBtn);

    void _DeleteTab(CxdButton *ApBtn);
    bool OnTimerMessage(int ATimeID, LPVOID ApData);
private:
    CxdGraphicsControl *m_pBtnAddTab;
    TxdButtonList m_ltItems;
    TxdButtonList m_ltWaitDeleteItem;
    const DWORD m_nCtDeleteItem;

    void DoItemClick(CxdUIComponent* ApBtn);
    void DoItemCloseClick(CxdUIComponent* ApBtn);
};