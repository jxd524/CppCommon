/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxduiDefines.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:19:05
	Description: 
**************************************************************************/
#pragma once;

#include <GdiPlus.h>
using namespace Gdiplus;
/*stl header*/
#include <vector>
using namespace std;

#include "JxdTypedef.h"
#include "JxdEvent.h"

/*class forword*/
class CxdPosition;
class CxdAnchors;
class CxdUIComponent;
class CxdGraphicsControl;
class CxdCheckBox;
class CxdButton;
class CxdGifControl;
class CxdEdit;
class CxdBar;
struct TPopupMenuItem;

//���Ͷ���
typedef const TCHAR* const CtConst;

typedef vector< CxdUIComponent* > TxdComponentList;
typedef TxdComponentList::iterator TxdComponentListIT;

typedef vector< TPopupMenuItem* > TxdPopupItemList;
typedef TxdPopupItemList::iterator TxdPopupItemListIT;
//
typedef vector< CxdGraphicsControl* > TxdGraphicsCtrlList;
typedef TxdGraphicsCtrlList::iterator TxdGraphicsCtrlListIT;

typedef vector< CxdCheckBox* > TxdCheckButtonList;
typedef TxdCheckButtonList::iterator TxdCheckButtonListIT;

typedef vector< CxdButton* > TxdButtonList;
typedef TxdButtonList::iterator TxdButtonListIT;

typedef vector<CxdGifControl *> TxdGifCtrlList;
typedef TxdGifCtrlList::iterator TxdGifCtrlListIT;

/*�¼����Ͷ���*/
typedef IxdNotifyEventT<LPVOID>                             IxdDataInterface;
typedef IxdNotifyEventT<CxdUIComponent*>                    IxdComponentInterface;
typedef IxdNotifyEventT<CxdGraphicsControl*>                IxdGraphicsInterface;
typedef IxdNotifyEventT<TPopupMenuItem*>                    IxdMenuItemInterface;
typedef IxdNotifyEventExT<CxdGraphicsControl*, bool&>       IxdLoopCtrlInterface;

/*terry ui lib enum*/
enum TxdGpUIState{ uiNormal, uiActive, uiDown};
enum TxdGpDrawStyle{ dsPaste, dsPasteOnCenter, dsStretchByVH, dsStretchAll, dsStretchByPos };
enum TxdGpCloseStyle{ csDestroy, csMini, csHide, csNone};
enum TWindowState{ wsNormal, wsMinimized, wsMaximized };
enum TCursorStyle{ crDefault, crArrow, crIbeam, crWait, crCross, crUparrow, crSize, crIcon, 
    crSizeNWSE, crSizeNESW, crSizeWE, crSizeNS, crSizeALL, crHand, crAppStarting, crHelp };
enum TxdDrawResult{ drPaste, drStretchWidth, drStretchHeight, drStretchWH };
enum TxdBarStyle{ bsHor, bsVer };
enum TxdBarMousePos{ mpNone, mpBK, mpFirstButton, mpLastButton, mpSlider};

/*terry ui lib struct*/
#pragma pack(1)
struct TArgbInfo
{
    byte FBlue, 
        FGreen, 
        FRed,
        FAlpha;
};
#pragma pack()

//��ʱ���ṹ
struct TxdTimerInfo 
{
    CxdUIComponent*    pOwner;
    int                TimerID;
    LPVOID             Data;
};

/*terry ui list struct*/
struct TPopupMenuItem
{
    bool    FIsCheck;
    bool    FIsEnable;
    bool    FIsVisible;
    int     FImgIndex;
    INT64   FTag;
    CString FCaption;
    TxdPopupItemList *FpSubList;
	
    TPopupMenuItem(const TCHAR *ApCaption = NULL, INT64 Tag=0, bool bCheck=false) 
    { 
        FCaption = ApCaption; FpSubList = NULL; FTag = Tag; 
        FIsVisible = true;  FIsEnable = true; FIsCheck = bCheck;
    }
    inline bool IsSeparator(void) const { return FCaption.IsEmpty() || FCaption == TEXT("-"); }
    inline bool IsHashSubItem(void) const { return FpSubList != NULL && !FpSubList->empty(); }
};

/*terry ui Create controls Bitmap by info*/
struct TxdCreateCtrlBitmap
{
    //���
    int Width;
    int Height;
    //������ɫ��Ϣ
    LinearGradientMode Mode;
    DWORD *FormColors;
    DWORD *ToColors;
    int Count;
    //����Ϣ
    DWORD LineColor;
    int LineStyle;//-1: ��������, 0: ֱ��, ����: 2,4,8,16
    int CornerRadius;
};

struct TPopupMenuItemInfos
{
    CString FCaption;
    INT64   FTag;
    int     FImgIndex;
    bool    FIsCheck;
    bool    FIsEnable;
    bool    FIsVisible;
    INT64   FParentTag;
    TPopupMenuItemInfos(const TCHAR* ApCaption, const INT64 &ATag = -1, int AImgIndex = -1, const INT64 &AParentTag = -1, 
                        bool AIsCheck = false, bool AIsEnable = true, bool AIsVisible = true)
    {
        FCaption = ApCaption;
        FTag = ATag;
        FImgIndex = AImgIndex;
        FIsCheck = AIsCheck;
        FIsEnable = AIsEnable;
        FIsVisible = AIsVisible;
        FParentTag = AParentTag;
    }
};

//�Զ�����Ϣ WPARM: this; LPVOID: CxdCustomMessageInfo
const DWORD CtCustomMessageID = 65511;
struct CxdCustomMessageInfo
{
    DWORD  MessageID;
    LPVOID MessageData;
};

/*terry ui lib const string*/
static CtConst CtxdUIComponent = TEXT("xdUIComponent");
static CtConst CtxdWinControl = TEXT("xdWinControl");
static CtConst CtxdGraphicsControl = TEXT("xdGraphicsControl");
static CtConst CtxdLable = TEXT("xdLable");
static CtConst CtxdForm = TEXT("xdForm");
static CtConst CtxdPopupForm = TEXT("xdPopupForm");
static CtConst CtxdEdit = TEXT("xdEdit");
static CtConst CtxdHitWindow = TEXT("xdHitWindow");
static CtConst CtxdGifControl = TEXT("xdGifControl");
static CtConst CtxdBar = TEXT("xdBar");
static CtConst CtxdWebBrowser = TEXT("xdWebBrowser");
static CtConst CtxdCheckBox = TEXT("xdCheckBox");
static CtConst CtxdButton = TEXT("xdButton");
static CtConst CtxdComboBox = TEXT("xdComboBox");
static CtConst CtxdDateTimeCtrl = TEXT("xdDateTimeCtrl");

