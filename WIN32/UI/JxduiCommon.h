/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxduiCommon.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:19:19
	Description: 
**************************************************************************/
#pragma once

#include <GdiPlus.h>
using namespace Gdiplus;

#include <vector>
using namespace std;

#include "JxdMacro.h"
#include "JxdEvent.h"
#include "JxdTemplateSub.h"
#include "JxduiDefines.h"

inline int GetUnionID(void) { static int _gID = 100; return _gID++; }

//图片信息管理 包含内容：图片，图片数量，图片文件名（全路径)
class CxdImageInfo
{
public:
    CxdImageInfo();
    CxdImageInfo(const CxdImageInfo &ASrcObject);
    //赋值操作, 对Bitmap对象只进行引用
    CxdImageInfo& operator = (const CxdImageInfo &ASrcObject);
    ~CxdImageInfo();

    Gdiplus::Bitmap* GetBitmap(void); //返回指针，外部不要去释放
    bool    SetBitmap(Gdiplus::Bitmap *ApBmp, bool AOnlyRes = true); //注意此函数的使用，AonlyRes说明是否是引用 非引用时，由外部管理BITMAP对象

    CString GetBitmapFileName(void) { return m_strFileName; }
    bool    SetBitmapFileName(const CString &AStrBmpFileName);

    int     GetBitmapItemHeight(void) { return IsExsitsBitmap() ? GetBitmap()->GetHeight() / GetBitmapItemCount() : 0; }
    int     GetBitmapItemWidth(void)  { return IsExsitsBitmap() ? GetBitmap()->GetWidth() : 0; }

    bool    IsExsitsBitmap(void) { return m_pBmp != NULL && m_pBmp->GetType() != Gdiplus::ImageTypeUnknown; }
    void    ForceDelBmpOnFree(void){ m_bFreeBmp = true; }//小心使用此函数，强制在对象释放时释放Bitmap对象

    PropertyValue( BitmapItemCount, int, ANewValue > 0 && ANewValue != GetBitmapItemCount() );
    PropertyValue( BitmapDrawStyle, TxdGpDrawStyle, ANewValue != GetBitmapDrawStyle() );
    PropertyValue( StretchX, int, true );
    PropertyValue( StretchY, int, true );
private:
    bool m_bFreeBmp;
    Gdiplus::Bitmap* m_pBmp;
    CString m_strFileName;
    void CheckClearBmp();
};

//字体信息管理 FontName FontSize FontColor FontTrimming FontAlignment FontLineAlignment
class CxdFontInfo
{
public:
    CxdFontInfo();
    CxdFontInfo& CopyObject(const CxdFontInfo &ASrcObject);
    CxdFontInfo(const CxdFontInfo &ASrcObject);
    CxdFontInfo& operator = (const CxdFontInfo &ASrcObject);
    ~CxdFontInfo();

    void ForceDelFontOnFree(void){ m_bFreeFont = true; }//小心使用此函数，强制在对象释放时释放Font对象

    Gdiplus::Font* GetFont(void);
    void           SetFont(Gdiplus::Font *AFont, bool AOnlyRes = true);

    CString GetFontName(void) const { return m_strFontName; }
    void    SetFontName(const CString &AFontName);

    Gdiplus::REAL GetFontSize(void) const { return m_nFontSize; }
    void          SetFontSize(Gdiplus::REAL ASize);

    PropertyValue( FontColor, Gdiplus::ARGB, true );
    PropertyValue( FontTrimming, Gdiplus::StringTrimming, true );
    PropertyValue( FontAlignment, Gdiplus::StringAlignment, true );
    PropertyValue( FontLineAlignment, Gdiplus::StringAlignment, true );

    int  GetNormalLeftOffset(void) { return GetOffsetValue(m_pNormalOffset, true); }
    int  GetNormalTopOffset(void)  { return GetOffsetValue(m_pNormalOffset, false); }
    void SetNormatOffset(const int &ALeft, const int &ATop) { SetOffsetValue(m_pNormalOffset, ALeft, ATop); }

    int  GetActiveLeftOffset(void) { return GetOffsetValue(m_pActiveOffset, true); }
    int  GetActiveTopOffset(void)  { return GetOffsetValue(m_pActiveOffset, false); }
    void SetActiveOffset(const int &ALeft, const int &ATop) { SetOffsetValue(m_pActiveOffset, ALeft, ATop); }

    int  GetDownLeftOffset(void) { return GetOffsetValue(m_pDownOffset, true); }
    int  GetDownTopOffset(void)  { return GetOffsetValue(m_pDownOffset, false); }
    void SetDownOffset(const int &ALeft, const int &ATop) { SetOffsetValue(m_pDownOffset, ALeft, ATop); }
private:
    class CxdOffsetInfo
    {
    public:
        CxdOffsetInfo(): FLeftOffset(0), FTopOffset(0) {}
        int FLeftOffset;
        int FTopOffset;
    };

    bool m_bFreeFont;
    Gdiplus::Font *m_pFont;
    CString m_strFontName;
    Gdiplus::REAL m_nFontSize;
    void CheckClearFont(void);

    inline int GetOffsetValue(CxdOffsetInfo *ApInfo, bool AbGetLeft)
    {
        if ( Assigned(ApInfo) )
        {
            if ( AbGetLeft )
            {
                return ApInfo->FLeftOffset;
            }
            return ApInfo->FTopOffset;
        }
        return 0;
    }
    inline void SetOffsetValue(CxdOffsetInfo *&ApInfo, const int &ALeftOffset, const int &ATopOffset)
    {
        ApInfo = xdCreateObject(ApInfo);
        ApInfo->FLeftOffset = ALeftOffset;
        ApInfo->FTopOffset = ATopOffset;
    }
    CxdOffsetInfo *m_pNormalOffset;
    CxdOffsetInfo *m_pActiveOffset;
    CxdOffsetInfo *m_pDownOffset;    
};

class CxdPosition;
typedef CxdNotifyEventListT<CxdPosition*> CxdPositionEventList;
typedef CxdPositionEventList::IEventType  IxdPositionInterface;
class CxdPosition
{
#define  CheckReSetValue(origValue, newValue, oldValue) \
    if ( origValue != newValue) { oldValue = origValue; origValue = newValue; };

public:
    CxdPosition(): m_Position(0, 0, 0, 0), m_OldPosition(0, 0, 0, 0){}

    inline Gdiplus::Rect& GetRect() { return m_Position; }
    inline Gdiplus::Rect& GetOldRect() { return m_OldPosition; }
    inline CRect GetCRect() { return CRect(m_Position.X, m_Position.Y, m_Position.X + m_Position.Width, m_Position.Y + m_Position.Height); }
    inline CRect GetOldCRect() { return CRect(m_OldPosition.X, m_OldPosition.Y, m_OldPosition.X + m_OldPosition.Width, m_OldPosition.Y + m_OldPosition.Height); }

    inline int GetLeft(void) const { return m_Position.X; }
    inline int GetRight(void) const { return m_Position.X + m_Position.Width; }
    inline int GetTop(void) const { return m_Position.Y; }
    inline int GetBottom(void) const { return m_Position.Y + m_Position.Height; }
    inline int GetWidth(void) const { return m_Position.Width; }
    inline int GetHeight(void) const { return m_Position.Height; }

    inline bool IsInReect(const int& x, const int& y) { return x >= m_Position.X && x < GetRight() && y >= m_Position.Y && y < GetBottom(); }

    virtual void SetLeft(const int &ALeft)     { CheckReSetValue( m_Position.X, ALeft, m_OldPosition.X ); }
    virtual void SetTop(const int &ATop)       { CheckReSetValue( m_Position.Y, ATop, m_OldPosition.Y); }
    virtual void SetWidth(const int &AWidth)   { CheckReSetValue( m_Position.Width, AWidth, m_OldPosition.Width); }
    virtual void SetHeight(const int &AHeight) { CheckReSetValue( m_Position.Height, AHeight, m_OldPosition.Height ); }

    inline void ClearOldPosition(void) { m_OldPosition = Gdiplus::Rect(0, 0, 0, 0); }

    bool AddEventPositionChanged(IxdPositionInterface *ApEvent) { return m_etPositionChangedObjectList.AddEvent( ApEvent, true ); }
    void NotifyPositionChanged(void) { m_etPositionChangedObjectList.Notify(this); }
    void ClearEventByOwner(const INT64& AOwnerAddr) { m_etPositionChangedObjectList.ClearEventByOwner( AOwnerAddr); }
private:
    Gdiplus::Rect m_Position;
    Gdiplus::Rect m_OldPosition;

    CxdPositionEventList    m_etPositionChangedObjectList;
};

//CxdCursor
class CxdCursor;
typedef CxdNotifyEventListT<CxdAnchors*> CxdAnchorEventList;
typedef CxdAnchorEventList::IEventType   IxdAnchorInterface;
class CxdCursor
{
public:
    CxdCursor() { SetCursorStyle(crDefault); }
    TCursorStyle SetCursorStyle(TCursorStyle AStyle);
    TCursorStyle GetCursorStyle() const { return m_crCursorStyle; }
    HCURSOR      GetCursor(){ return m_hCursor; }
private:
    TCursorStyle m_crCursorStyle;
    TCHAR* m_pCursorName;
    HCURSOR m_hCursor;
};

//CxdAnchors
enum TAnchorStyle{ akLeft = 2, akTop = 4, akRight = 8, akBottom = 16 };
class CxdAnchors
{
public:
    CxdAnchors(): m_nAnchors(0){ }
    ~CxdAnchors(){}
    void Add(TAnchorStyle AStyle);
    void SetValue(const DWORD &AStyle) { m_nAnchors = AStyle; NotifyAnchorChanged(); }
    void AddAll() { m_nAnchors = akLeft | akTop | akRight | akBottom; NotifyAnchorChanged(); }
    void SetLeftTop(void) { m_nAnchors = akLeft | akTop; NotifyAnchorChanged(); }
    void SetRightBottom(void) { m_nAnchors = akRight | akBottom; NotifyAnchorChanged(); }
    void SetRightTop(void) { m_nAnchors = akRight | akTop; NotifyAnchorChanged(); }
    void Del(TAnchorStyle AStyle);
    void Clear(void) { m_nAnchors = 0; }
    bool IsExsits(TAnchorStyle AStyle);

    bool AddEventAnchorChanged(IxdAnchorInterface* ApEvent) { return m_etAnchorChangedObjectList.AddEvent( ApEvent ); }
    void NotifyAnchorChanged(void) { m_etAnchorChangedObjectList.Notify(this); }
    void ClearEventByOwner(const INT64& AOwnerAddr) { m_etAnchorChangedObjectList.ClearEventByOwner( AOwnerAddr); }
private:
    int m_nAnchors;
    CxdAnchorEventList m_etAnchorChangedObjectList;
};

//CxdScreen
class CxdScreen
{
public:
    CxdScreen();
    virtual ~CxdScreen();
    inline int GetScreenWidth(void)  { return m_nScreenWidth; }
    inline int GetScreenHeight(void) { return m_nScreenHeight; }
private:
    int m_nScreenWidth;
    int m_nScreenHeight;
};

//====================================================================
// HintMessage 接口
//====================================================================
//
Interface(IHintMessage)
    virtual void ActivateHint(CxdUIComponent *ApObject) = 0;
    virtual void UnActiveHint(CxdUIComponent *ApObject) = 0;
InterfaceEnd;

//当不存在 IHintMessage 接口时, 通知指定接口
void SetNotifyHintInterface(IxdDataInterface *ApInterface);

//提供给CxdComponent使用
IHintMessage* GetHintInterface(void);
bool          IsExsitsHintInterface(void);
void          SetHintInterface(IHintMessage *ApInterface);