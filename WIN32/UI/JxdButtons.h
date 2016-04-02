#pragma once;

#include "JxdGraphicsControl.h"
#include "JxdLabels.h"

class CxdCheckBox: public CxdLabelEx
{
public:
    CtConst xdGetClassName(void) { return CtxdCheckBox; }

    CxdCheckBox();
    ~CxdCheckBox();

    PropertyValue( ImageOffsetX, int, true );
    PropertyValue( ImageOffsetY, int, true );
    PropertyValue( GroupID, DWORD, true );
    PropertyValue( AutoChangedCheckState, bool, true );

    inline bool GetCheck(void) { return m_bCheck; }
    inline void SetCheck(bool bCheck, bool bReDraw = false);

    inline bool GetIsRadioButton(void) { return m_bRadioButton; }
    void SetRadioButton(bool bRadioButton, bool bReDraw = false);

    inline CxdImageInfo* ImageCheckInfo(void) { return xdCreateObject(m_pImgCheck); }
    inline CxdImageInfo* ImageUnCheckInfo(void) { return xdCreateObject(m_pImgUnCheck); }

	void SetCheckImageInfo(CxdImageInfo *ApNewImg, bool AbResOnly = true);
	void SetUnCheckImageInfo(CxdImageInfo *ApNewImg, bool AbResOnly = true);

    //Group
    inline int GetGroupIndex(void) { return m_nGroupIndex; }
    void AddToGroup(const int &AGroupIndex);
    void DeleteFromGroup(void);
protected:
    void OnClick(const int &x /* = 0 */, const int &y /* = 0 */);
    bool OnUIStateChanged(const TxdGpUIState &ANewSate);
    void DoDrawSubjoinBK(Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt);
private:
    bool m_bRadioButton;
    bool m_bCheck;
    int m_nGroupIndex;
    CxdImageInfo *m_pImgCheck, *m_pImgUnCheck;
    bool GetImgRectOnUI(CxdImageInfo *ApImage, Rect &ARect);

    static TxdCheckButtonList m_ltGroupButtonList;
};

class  CxdButton: public CxdLabelEx
{
public:
    CxdButton();
    ~CxdButton();

    CtConst xdGetClassName(void) { return CtxdButton; }   

    inline bool GetPushDown(void) { return m_bPushDown; }
    void SetPushDown(bool bPushDown, bool bReDraw = false); 

    //Group
    inline int GetGroupIndex(void) { return m_nGroupIndex; }
    void AddToGroup(const int &AGroupIndex);
    void DeleteFromGroup(void);

    PropertyValue( GroupID, DWORD, true );
protected:
    TxdGpUIState GetCurUISate(void);
private:
    bool m_bPushDown;
    int m_nGroupIndex;

    static TxdButtonList G_ltGroupButtonList;
};