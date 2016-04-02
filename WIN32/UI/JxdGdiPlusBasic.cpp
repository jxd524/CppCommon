#include "stdafx.h"
#include "JxdGdiPlusBasic.h"
#include "JxdFileSub.h"
#include "JxdMacro.h"

#include "JxduiCommon.h"
#include "JxduiDefines.h"

static const TCHAR* const ImgFormatBitmap = TEXT("image/bmp");
static const TCHAR* const ImgFormatJpg = TEXT("image/jpeg");
static const TCHAR* const ImgFormatGif = TEXT("image/gif");
static const TCHAR* const ImgFormatTiff = TEXT("image/tiff");
static const TCHAR* const ImgFormatPng = TEXT("image/png");


//此类外部不需要使用，用于初始化GDI+环境
class CxdGdiPlusBasic
{
public: 
    CxdGdiPlusBasic(void);
    virtual~CxdGdiPlusBasic(void);
private:
    ULONG_PTR m_gdiToken;
    static int _gGdiPlusObjCount;
};

int CxdGdiPlusBasic::_gGdiPlusObjCount = 0;

CxdGdiPlusBasic::CxdGdiPlusBasic(void)
{
    if ( 0 == _gGdiPlusObjCount++ )
    {
        GdiplusStartupInput gdiInput;
        GdiplusStartup( &m_gdiToken, &gdiInput, NULL );
    }
}

CxdGdiPlusBasic::~CxdGdiPlusBasic(void)
{
    if ( _gGdiPlusObjCount-- <= 1 )
    {
        GdiplusShutdown( m_gdiToken );
    }
}

void InitGdiplus()
{
    static CxdGdiPlusBasic initObject;
}

int GetEncoderClsid( const WCHAR* format, CLSID* pClsid )
{
    UINT num= 0;
    UINT size= 0;

    ImageCodecInfo* pImageCodecInfo= NULL;

    GetImageEncodersSize(&num, &size);
    if(size== 0)
    {
        return -1;
    }
    pImageCodecInfo= (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo== NULL)
    {
        return -1;
    }

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j=0; j< num; ++j)
    {
        if(wcscmp(pImageCodecInfo[j].MimeType, format)== 0)
        {
            *pClsid= pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}

CLSID GetEncoderClsid( const WCHAR* format )
{
    CLSID c = {0};
    GetEncoderClsid(format, &c);
    return c;
}

TxdDrawResult DrawStretchImage( Graphics &ADestGp, Image *ASrcImg, const Rect &ADestRect, const Rect &ASrcRect, const int &APosX, const int &APosY, ImageAttributes *ApImgAtt )
{
#define StretchByHor \
    ADestGp.DrawImage( ASrcImg, DestR, SrcR.X, SrcR.Y, SrcR.Width, SrcR.Height, UnitPixel, ApImgAtt );\
    SrcR.X += SrcR.Width;\
    SrcR.Width = 1;\
    DestR.X += DestR.Width; \
    DestR.Width = nSrcStretchWidth;\
    ADestGp.DrawImage( ASrcImg, DestR, SrcR.X, SrcR.Y, SrcR.Width, SrcR.Height, UnitPixel, ApImgAtt );\
    SrcR.X += SrcR.Width;\
    SrcR.Width = nSrcRightWidth;\
    DestR.X += DestR.Width; \
    DestR.Width = nSrcRightWidth;\
    ADestGp.DrawImage( ASrcImg, DestR, SrcR.X, SrcR.Y, SrcR.Width, SrcR.Height, UnitPixel, ApImgAtt );

    if (ADestRect.Width <= ASrcRect.Width)
    {
        int nMaxW = ADestRect.Width;
        //目标宽度比源宽度小或相等
        if (ADestRect.Height <= ASrcRect.Height)
        {
            ADestGp.DrawImage( ASrcImg, ADestRect, ASrcRect.X, ASrcRect.Y, nMaxW, ADestRect.Height, UnitPixel, ApImgAtt );
            return drPaste;
        }
        else
        {
            int nDestUpHeight = APosY;
            int nDestDownHeight = ASrcRect.Height - nDestUpHeight;
            int nDestStretchHeight = ADestRect.Height - nDestUpHeight - nDestDownHeight + 1;

            //上: 直接复制
            Rect SrcR( ASrcRect.X, ASrcRect.Y, nMaxW, nDestUpHeight );
            Rect DestR( ADestRect.X, ADestRect.Y, nMaxW, nDestUpHeight );
            ADestGp.DrawImage( ASrcImg, DestR, SrcR.X, SrcR.Y, nMaxW, SrcR.Height, UnitPixel, ApImgAtt );

            //中: 拉伸
            SrcR.Y += nDestUpHeight;
            SrcR.Height = 1;
            DestR.Y += nDestUpHeight;
            DestR.Height = nDestStretchHeight;
            ADestGp.DrawImage( ASrcImg, DestR, SrcR.X, SrcR.Y, nMaxW, 1, UnitPixel, ApImgAtt );

            ////下: 直接复制
            SrcR.Y += SrcR.Height;
            SrcR.Height = nDestDownHeight;
            DestR.Y += DestR.Height;
            DestR.Height = nDestDownHeight - 1;
            ADestGp.DrawImage( ASrcImg, DestR, SrcR.X, SrcR.Y, nMaxW, SrcR.Height, UnitPixel, ApImgAtt );

            return drStretchHeight;
        }
    }
    else //end if (ADestRect.Width <= ASrcRect.Width)
    {
        //目标宽度比源宽度大
        int nSrcLeftWidth = APosX;
        int nSrcRightWidth = ASrcRect.Width - APosX;
        int nSrcStretchWidth = ADestRect.Width - nSrcLeftWidth - nSrcRightWidth + 1;

        if (ADestRect.Height <= ASrcRect.Height)
        {
            int nMaxHeight = ADestRect.Height;
            
            Rect SrcR( ASrcRect.X, ASrcRect.Y, nSrcLeftWidth, nMaxHeight );
            Rect DestR( ADestRect.X, ADestRect.Y, nSrcLeftWidth, nMaxHeight );
            StretchByHor;
            return drStretchWidth;
        }
        else
        {
            int nDestUpHeight = APosY;
            int nDestDownHeight = ASrcRect.Height - nDestUpHeight;
            int nDestStretchHeight = ADestRect.Height - nDestUpHeight - nDestDownHeight;

            Rect SrcR( ASrcRect.X, ASrcRect.Y, nSrcLeftWidth, nDestUpHeight );
            Rect DestR( ADestRect.X, ADestRect.Y, nSrcLeftWidth, nDestUpHeight );
            StretchByHor;

            SrcR = Rect( ASrcRect.X, ASrcRect.Y + nDestUpHeight, nSrcLeftWidth, nDestDownHeight );
            DestR = Rect( ADestRect.X, ADestRect.Y + nDestUpHeight + nDestStretchHeight, nSrcLeftWidth, nDestDownHeight );
            StretchByHor;

            //左边 - 中间绘制
            SrcR = Rect( ASrcRect.X, ASrcRect.Y + nDestUpHeight, nSrcLeftWidth, 1 );
            DestR = Rect( ADestRect.X, ADestRect.Y + nDestUpHeight, nSrcLeftWidth, nDestStretchHeight );
            ADestGp.DrawImage( ASrcImg, DestR, SrcR.X, SrcR.Y, SrcR.Width, SrcR.Height, UnitPixel, ApImgAtt );
            //中间 - 中间绘制
            SrcR.X += nSrcLeftWidth;
            SrcR.Width = 1;
            SrcR.Height = 1;
            DestR.X += nSrcLeftWidth;
            DestR.Width = nSrcStretchWidth;
            ADestGp.DrawImage( ASrcImg, DestR, SrcR.X, SrcR.Y, SrcR.Width, SrcR.Height, UnitPixel, ApImgAtt );
            //右边 - 中间绘制
            SrcR.X += SrcR.Width;
            SrcR.Width = nSrcRightWidth;
            SrcR.Height = 1;
            DestR.X += DestR.Width;
            DestR.Width = nSrcRightWidth;
            ADestGp.DrawImage( ASrcImg, DestR, SrcR.X, SrcR.Y, SrcR.Width, SrcR.Height, UnitPixel, ApImgAtt );

            return drStretchWH;
        }
    }
}

HRGN CreateStyleByBitmap( Bitmap *ABmp, byte ALessAlpha /*= 30*/ )
{
    BitmapData bmpData;
    HRGN tempH, Result;
    int nW( ABmp->GetWidth() ), nH( ABmp->GetHeight() );

    Result = CreateRectRgn(0, 0, nW, nH );
    ABmp->LockBits( &Rect(0, 0, nW, nH), ImageLockModeRead, PixelFormat32bppARGB, &bmpData);

    TArgbInfo* p = (TArgbInfo*)bmpData.Scan0;
    for ( int y = 0; y != bmpData.Height; y++ )
    {
        for ( int x = 0; x != bmpData.Width; x++ )
        {
            if ( p->FAlpha < ALessAlpha )
            {
                tempH = CreateRectRgn(x, y, x+1, y+1);
                CombineRgn(Result, Result, tempH,  RGN_XOR);
                DeleteObject( tempH );
            }
            p++;
        }
    }
    ABmp->UnlockBits( &bmpData );

    return Result;
}

void DrawCommonImage( Graphics &ADestGp, const Rect &ADestR, CxdImageInfo *ApSrcImg, TxdGpUIState AuiState, ImageAttributes *ApImgAtt )
{
    int nIndex;
    switch( AuiState )
    {
    case uiDown:
        nIndex = 2;
        break;
    case uiActive:
        nIndex = 1;
        break;
    default:
        nIndex = 0;
    }
    DrawCommonImage( ADestGp, ADestR, ApSrcImg, nIndex, ApImgAtt );
}

void DrawCommonImage( Graphics &ADestGp, const Rect &ADestR, CxdImageInfo *ApSrcImg, int AItemIndex, ImageAttributes *ApImgAtt )
{
    if ( !Assigned(ApSrcImg) || !ApSrcImg->IsExsitsBitmap() )
    {
        return;
    }
    int nTemp = ApSrcImg->GetBitmap()->GetHeight() / ApSrcImg->GetBitmapItemCount();
    Rect BmpR(0, 0, ApSrcImg->GetBitmap()->GetWidth(), nTemp );
    if ( AItemIndex >= ApSrcImg->GetBitmapItemCount() )
    {
        AItemIndex = ApSrcImg->GetBitmapItemCount() - 1;
    }
    BmpR.Y += nTemp * AItemIndex;
    Rect DestR( ADestR );

    DrawCommonImage( ADestGp, ADestR, ApSrcImg->GetBitmap(), BmpR, ApSrcImg->GetBitmapDrawStyle(),
        ApSrcImg->GetStretchX(), ApSrcImg->GetStretchY(), ApImgAtt );

  /*  switch ( ApSrcImg->GetImageDrawStyle() )
    {
    case dsPaste:
    case dsPasteOnCenter:
        {
            int nW, nH;
            (BmpR.Width > DestR.Width) ? nW = DestR.Width : (nW = BmpR.Width, DestR.Width = nW);
            (BmpR.Height > DestR.Height) ? nH = DestR.Height : (nH = BmpR.Height, DestR.Height = nH);
            if ( dsPasteOnCenter == ApSrcImg->GetImageDrawStyle() )
            {
                DestR.X = ADestR.X + (ADestR.Width - BmpR.Width) / 2;
                DestR.Y = ADestR.Y + (ADestR.Height - BmpR.Height) / 2;
            }
            ADestGp.DrawImage( ApSrcImg->GetBitmap(), DestR, BmpR.X, BmpR.Y, nW, nH, UnitPixel, ApImgAtt );
        }
        break;
    case dsStretchByVH:
        DrawStretchImage( ADestGp, ApSrcImg->GetBitmap(), DestR, BmpR, BmpR.Width / 2, BmpR.Height / 2, ApImgAtt );
        break;
    case dsStretchAll:
        ADestGp.DrawImage( ApSrcImg->GetBitmap(), DestR, BmpR.X, BmpR.Y, BmpR.Width, BmpR.Height, UnitPixel, ApImgAtt );
        break;
    case dsStretchByPos:
        DrawStretchImage( ADestGp, ApSrcImg->GetBitmap(), DestR, BmpR, ApSrcImg->GetStretchX(), ApSrcImg->GetStretchY(), ApImgAtt );
        break;
    default:
        TRACE( TEXT("error draw style!\r\n") );
    }*/
}

void DrawCommonImage( Graphics &ADestGp, const Rect &ADestR, Image *ASrcImg, const Rect &ASrcRect, TxdGpDrawStyle ADrawStyle, 
    const int &APosX, const int &APosY, ImageAttributes *ApImgAtt /*= NULL*/ )
{
    if ( !Assigned(ASrcImg) ) return;

    Rect BmpR( ASrcRect );
    Rect DestR( ADestR );
    switch ( ADrawStyle )
    {
    case dsPaste:
    case dsPasteOnCenter:
        {
            int nW, nH;
            (BmpR.Width > DestR.Width) ? nW = DestR.Width : (nW = BmpR.Width, DestR.Width = nW);
            (BmpR.Height > DestR.Height) ? nH = DestR.Height : (nH = BmpR.Height, DestR.Height = nH);
            if ( dsPasteOnCenter == ADrawStyle )
            {
                DestR.X = ADestR.X + (ADestR.Width - BmpR.Width) / 2;
                DestR.Y = ADestR.Y + (ADestR.Height - BmpR.Height) / 2;
            }
            ADestGp.DrawImage( ASrcImg, DestR, BmpR.X, BmpR.Y, nW, nH, UnitPixel, ApImgAtt );
        }
        break;
    case dsStretchByVH:
        DrawStretchImage( ADestGp, ASrcImg, DestR, BmpR, BmpR.Width / 2, BmpR.Height / 2, ApImgAtt );
        break;
    case dsStretchAll:
        ADestGp.DrawImage( ASrcImg, DestR, BmpR.X, BmpR.Y, BmpR.Width - 1, BmpR.Height - 1, UnitPixel, ApImgAtt );
        break;
    case dsStretchByPos:
        DrawStretchImage( ADestGp, ASrcImg, DestR, BmpR, APosX, APosY, ApImgAtt );
        break;
    default:
        TRACE( TEXT("error draw style!\r\n") );
    }
}

Bitmap* LoadImageFromRes( HMODULE AResHandle, DWORD AResID, const TCHAR* AResType )
{
    Bitmap *pRetImg = NULL;
    HGLOBAL hGlobal = NULL;
    HRSRC hSource = NULL;
    LPVOID lpVoid  = NULL;
    int nSize = 0;

    hSource = FindResource( AResHandle, MAKEINTRESOURCE(AResID), AResType );
    if( NULL == hSource )
        return pRetImg;

    hGlobal = LoadResource( AResHandle, hSource );
    if( NULL == hGlobal )
    {    
        FreeResource(hGlobal);         
        return pRetImg;        
    }
    lpVoid = LockResource(hGlobal);
    if( NULL == lpVoid )
    {
        goto Exit;
    }

    nSize = (UINT)SizeofResource( AResHandle, hSource );
    HGLOBAL hGlobal2 = GlobalAlloc( GMEM_MOVEABLE, nSize );
    if( hGlobal2 == NULL )
    {
        goto Exit;
    }

    void* pData = GlobalLock( hGlobal2 );
    memcpy( pData, (void *)hGlobal, nSize );
    GlobalUnlock(hGlobal2);

    IStream* pStream = NULL;
    if( S_OK == CreateStreamOnHGlobal(hGlobal2, TRUE, &pStream) )
    {
        pRetImg = Bitmap::FromStream( pStream, FALSE );
        pStream->Release();
    }
    FreeResource(hGlobal2); 

Exit:

    UnlockResource(hGlobal);    
    FreeResource(hGlobal); 
    return pRetImg;
}

void DrawCommonText( Graphics &ADestGp, const TCHAR* AText, CxdFontInfo *ApFontInfo, RectF ADestRect, DWORD AFontColor, TxdGpUIState AuiState, REAL ATranslate )
{
    if ( !Assigned(ApFontInfo) )
    {
        return;
    }
    BYTE a = BYTE(ATranslate * 255);
    AFontColor &= (a << 24) + 0xFFFFFF;
    SolidBrush fontColor( AFontColor );       
    StringFormat strFrm;
    strFrm.SetAlignment( ApFontInfo->GetFontAlignment() );
    strFrm.SetLineAlignment( ApFontInfo->GetFontLineAlignment() );
    strFrm.SetTrimming( ApFontInfo->GetFontTrimming() );

    switch ( AuiState )
    {
    case uiNormal:
        ADestRect.X += ApFontInfo->GetNormalLeftOffset();
        ADestRect.Y += ApFontInfo->GetNormalTopOffset();
    	break;
    case uiActive:
        ADestRect.X += ApFontInfo->GetActiveLeftOffset();
        ADestRect.Y += ApFontInfo->GetActiveTopOffset();
        break;
    case uiDown:
        ADestRect.X += ApFontInfo->GetDownLeftOffset();
        ADestRect.Y += ApFontInfo->GetDownTopOffset();
        break;
    }

    ADestGp.DrawString( AText, -1, ApFontInfo->GetFont(), ADestRect, &strFrm, &fontColor );
}

void DrawCommonText( Gdiplus::Graphics &ADestGp, const TCHAR* AText, CxdFontInfo *ApFontInfo, const Gdiplus::Rect &ADestRect, DWORD AFontColor, TxdGpUIState AuiState, REAL ATranslate )
{
    RectF R( (REAL)ADestRect.X, (REAL)ADestRect.Y, (REAL)ADestRect.Width, (REAL)ADestRect.Height );
    DrawCommonText( ADestGp, AText, ApFontInfo, R, AFontColor, AuiState, ATranslate );
}

CSize CalcStringSize( Graphics &AGh, const Gdiplus::Font *AFont, const TCHAR* ApText, int AMaxWidth, int AMaxHeight )
{
    RectF R;
    AGh.MeasureString( ApText, -1, AFont, RectF(0.0, 0.0, (REAL)AMaxWidth, (REAL)AMaxHeight), &R );
    CSize Result;
    Result.cx = (long)R.Width;
    Result.cy = (long)R.Height;
    return Result;
}

void ChangedBmpToGray( Bitmap *ApBmp, byte ALessAlpha )
{
    BitmapData bmpData;
    TArgbInfo *p;
    int x, y, nW, nH, Gray(0);

    nW = ApBmp->GetWidth();
    nH = ApBmp->GetHeight();
    if ( (0 == nW) || (0 == nH) ) return;

    ApBmp->LockBits( &Rect(0, 0, nW, nH), ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bmpData );

    p = bmpData.Stride > 0 ? (TArgbInfo*)bmpData.Scan0 : (TArgbInfo*)bmpData.Scan0 + bmpData.Stride * (bmpData.Height - 1);
    for (y = 0; y != bmpData.Height; y++)
    {
        for (x = 0; x != bmpData.Width; x++)
        {
            if (p->FAlpha > ALessAlpha)
            {
                Gray = ( p->FRed + p->FGreen + p->FBlue ) / 3; 
                p->FBlue = Gray;
                p->FGreen = Gray;
                p->FRed = Gray;
            }
            p++;
        }
    }
    ApBmp->UnlockBits( &bmpData );
}

Bitmap* DIBToBmp( LPBITMAPINFO lpBl )
{
    LPVOID pDIBBits;
    int nColors = lpBl->bmiHeader.biClrUsed ? lpBl->bmiHeader.biClrUsed : 1 << lpBl->bmiHeader.biBitCount;
    if( lpBl->bmiHeader.biBitCount > 8 )
    {
        pDIBBits = (LPVOID*)((LPDWORD)(lpBl->bmiColors + lpBl->bmiHeader.biClrUsed) +
            ((lpBl->bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
    }
    else
    {
        pDIBBits = (LPVOID*)(lpBl->bmiColors + nColors);
    }
    return Bitmap::FromBITMAPINFO( lpBl, pDIBBits );
}

int __BrightValue(int AValue, int ABrightValue)
{
    int nResult = AValue + ABrightValue;
    return nResult < 0 ? 0 : ( nResult > 255 ? 255 : nResult );
}

void ChangedBmpBrightness( Bitmap *ApBmp, int ABrightValue )
{
    BitmapData bmpData = {0};
    TArgbInfo *p;
    ApBmp->LockBits( &Rect(0, 0, ApBmp->GetWidth(), ApBmp->GetHeight()), ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bmpData );
    p = bmpData.Stride > 0 ? (TArgbInfo*)bmpData.Scan0 : (TArgbInfo*)bmpData.Scan0 + bmpData.Stride * (bmpData.Height - 1);
    for( int y = 0; y != bmpData.Height; y++ )
    {
        for( int x = 0; x != bmpData.Width; x++ )
        {
            p->FBlue  = __BrightValue( p->FBlue, ABrightValue );
            p->FGreen = __BrightValue( p->FGreen, ABrightValue );
            p->FRed   = __BrightValue( p->FRed, ABrightValue );    
            p++;
        }
    }
    ApBmp->UnlockBits( &bmpData );
}

void InitTranslateImgAtt( ImageAttributes &Attr, const REAL &ATraslateValue )
{
    ColorMatrix cm = 
    {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, ATraslateValue, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    Attr.SetColorMatrix( &cm, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap );
}

void ColorMatrix_TranslateValue( ColorMatrix &Acm, const REAL &ATraslateValue )
{
    Acm.m[3][3] = ATraslateValue;
}

void ColorMatrix_Gray( ColorMatrix &Acm )
{
    Acm.m[0][0] = Acm.m[0][1] = Acm.m[0][2] = 0.3f;
    Acm.m[1][0] = Acm.m[1][1] = Acm.m[1][2] = 0.59f;
    Acm.m[2][0] = Acm.m[2][1] = Acm.m[2][2] = 0.11f;
}

void ColorMatrix_ToIdentity( ColorMatrix &Acm )
{
    Acm.m[0][0] = 1.0f; Acm.m[0][1] = 0.0f; Acm.m[0][2] = 0.0f; Acm.m[0][3] = 0.0f; Acm.m[0][4] = 0.0f;
    Acm.m[1][0] = 0.0f; Acm.m[1][1] = 1.0f; Acm.m[1][2] = 0.0f; Acm.m[1][3] = 0.0f; Acm.m[1][4] = 0.0f;
    Acm.m[2][0] = 0.0f; Acm.m[2][1] = 0.0f; Acm.m[2][2] = 1.0f; Acm.m[2][3] = 0.0f; Acm.m[2][4] = 0.0f;
    Acm.m[3][0] = 0.0f; Acm.m[3][1] = 0.0f; Acm.m[3][2] = 0.0f; Acm.m[3][3] = 1.0f; Acm.m[3][4] = 0.0f;
    Acm.m[4][0] = 0.0f; Acm.m[4][1] = 0.0f; Acm.m[4][2] = 0.0f; Acm.m[4][3] = 0.0f; Acm.m[4][4] = 1.0f;
}

GraphicsPath* CreateRoundRectPath( const Rect &r, int ACornerRadius, int ARoundStyle )
{
    int CR2 = ACornerRadius * 2;
    GraphicsPath *pPath = new GraphicsPath();

    if ( ARoundStyle & 2 )
    {
        pPath->AddArc( r.X, r.Y, CR2, CR2, 180, 90 );
        pPath->AddLine( r.X+ ACornerRadius, r.Y, r.GetRight() - CR2, r.Y );
    }
    else
    {
        pPath->AddLine(r.X, r.Y, r.GetRight() - CR2, r.Y);
    }

    if ( ARoundStyle & 4 )
    {
        pPath->AddArc(r.X + r.Width - CR2, r.Y, CR2, CR2, 270, 90);
        pPath->AddLine(r.GetRight(), r.Y + CR2, r.GetRight(), r.Y + r.Height - CR2);
    }
    else
    {
        pPath->AddLine(r.GetRight(), r.Y, r.GetRight(), r.Y + r.Height - CR2);
    }
    
    if ( ARoundStyle & 16 )
    {
        pPath->AddArc(r.X + r.Width - CR2, r.Y + r.Height - CR2, CR2, CR2, 0, 90);
        pPath->AddLine(r.GetRight() - CR2, r.GetBottom(), r.X + CR2, r.GetBottom());
    }
    else
    {
        pPath->AddLine(r.GetRight(), r.GetBottom(), r.X + CR2, r.GetBottom());
    }
    
    if ( ARoundStyle & 8 )
    {
        pPath->AddArc(r.X, r.GetBottom() - CR2, CR2, CR2, 90, 90);
        pPath->AddLine(r.X, r.GetBottom() - CR2, r.X, r.Y + CR2);
    }
    else
    {
        pPath->AddLine(r.X, r.GetBottom(), r.X, r.Y + CR2);
    }

    pPath->CloseFigure();
    return pPath;
}

Bitmap* CreateCtrlBitmap( TxdCreateCtrlBitmap *ApInfo )
{
    Bitmap *pBmp = new Bitmap( ApInfo->Width, ApInfo->Height );
    Graphics G( pBmp );
    G.SetSmoothingMode( SmoothingModeAntiAlias );

    int nH = ApInfo->Height / ApInfo->Count;
    for (int i = 0; i < ApInfo->Count; i++ )
    {
        Rect R(0, i * nH, ApInfo->Width - 1, nH - 1);
        Brush *pBrush;
        if ( ApInfo->FormColors[i] == ApInfo->ToColors[i] )
        {
            pBrush = new SolidBrush( ApInfo->FormColors[i] );
        }
        else
        {
            pBrush = new LinearGradientBrush( R, ApInfo->FormColors[i], ApInfo->ToColors[i], ApInfo->Mode );
        }

        if ( ApInfo->LineStyle >= 0 )
        {
            GraphicsPath *pPath = CreateRoundRectPath( R, ApInfo->CornerRadius, ApInfo->LineStyle );
            G.FillPath( pBrush, pPath );
            G.DrawPath( &Pen(ApInfo->LineColor), pPath);
            delete pPath;
        }
        else
        {
            R = Rect( 0, i * nH, ApInfo->Width, nH );
            G.FillRectangle( pBrush, R );
        }
        delete pBrush;
    }
    return pBmp;
}
