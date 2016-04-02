/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdGifControl.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:17:21
	Description: 
**************************************************************************/
#pragma once;
#include "JxdGraphicsControl.h"

//Gif parse
struct TGifInfo
{
    int FPosIndex;
    int FPauseTime;
};
typedef vector<TGifInfo*> TGifInfoList;
typedef TGifInfoList::iterator TGifInfoListIt;

class CxdGifParse
{
public:
    CxdGifParse();
    ~CxdGifParse();

    bool InitGif(Gdiplus::Bitmap *ApGifBmp);
    int  GetCount(void) { return (int)m_ltGifInfo.size(); }    
    int  NextBitmap(void);

    void ClearGifInfo(void);
    void SetFirstFrame(void) { m_nCurIndex = -1; }
    Gdiplus::Bitmap* GetGifBitmap(void) { return m_pBmpGif; }
private:
    TGifInfoList m_ltGifInfo;
    Gdiplus::Bitmap *m_pBmpGif;
    int m_nCurIndex;
};

/*CxdGifControl: GIF实现类*/
class CxdGifControl: public CxdGraphicsControl
{
public:
    CxdGifControl();
    ~CxdGifControl();
    CtConst xdGetClassName(void) { return CtxdGifControl; }

    bool IsPlaying(void) { return m_bPlaying; }

    void ClearGifInfo(void);
    void BeginShowGif(void);
    void StopShowGif(void);

	void SetGifInfo(CxdGifParse *ApGifInfo);
	CxdGifParse* GetGifInfo(void) { return &m_oGifInfo; }

    bool GetAutoPlayOnActive(void) { return m_bAutoPlayOnActive; }
    void SetAutoPlayOnActive(bool bActive);

    PropertyValue( ShowFirstFrameOnStop, bool, true );
    PropertyValue( DrawNormalBorder, bool, true );
    PropertyValue( DrawActiveBorder, bool, true );
    PropertyValue( BorderNormalColor, DWORD, true );
    PropertyValue( BorderActiveColor, DWORD, true );
protected:
    bool OnUIStateChanged(const TxdGpUIState &ANewSate);
    bool OnTimerMessage(int ATimeID, LPVOID ApData);
    void DoDrawSubjoinBK(Graphics &ADestG, const Rect &ADestRect, ImageAttributes *ApImgAtt);
private:
    CxdGifParse m_oGifInfo;
    bool m_bPlaying;
    bool m_bAutoPlayOnActive;
    const int CtAutShowGifTimerID;
};