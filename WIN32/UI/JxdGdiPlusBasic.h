/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdGdiPlusBasic.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:17:32
	Description: 
        ʹ��GDI+ʱ, ��Ҫ�Ȱ������ļ�.���ڳ�ʼ��GDI+����
        ����: ���ƹ��ú���

        �ϸ��Ӻ���ʹ������:
        1: CreateCtrlBitmap
        ����ָ������Ϣ, ������Ӧ��ͼƬ
        DWORD fc[] ={ 0xFF00FF00, 0xFFFF0000, 0xFF0000FF };
        DWORD tc[] ={ 0xFF230002, 0x33F0Fa32, 0xFF00FF00 };
        TxdCreateCtrlBitmap info;
        info.Width = 50;
        info.Height = 90;
        info.Mode = LinearGradientModeVertical;
        info.Count = 3;
        info.FormColors = fc;
        info.ToColors = tc;
        info.LineColor = 0xFF123123;
        info.LineStyle = 2 | 4;
        info.CornerRadius = 10;

        Bitmap *pBmp = CreateCtrlBitmap( &info );
**************************************************************************/
#ifndef _GuiPlusBasic_H_By_Terry_At_20091003_
#define _GuiPlusBasic_H_By_Terry_At_20091003_
#pragma once;

#pragma comment(lib, "gdiplus.lib")
#include <GdiPlus.h>
using namespace Gdiplus;

class CxdImageInfo;
class CxdFontInfo;
struct TArgbInfo;
enum TxdGpUIState;
enum TxdGpDrawStyle;
enum TxdDrawResult;
struct TxdCreateCtrlBitmap;

extern const TCHAR* const ImgFormatBitmap;
extern const TCHAR* const ImgFormatJpg;
extern const TCHAR* const ImgFormatGif;
extern const TCHAR* const ImgFormatTiff;
extern const TCHAR* const ImgFormatPng;

//��ʼ��GDI+����
void InitGdiplus(void);

int   GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
CLSID GetEncoderClsid(const WCHAR* format);

//������Ϣ�Զ�����ͼƬ
void  DrawCommonImage(Graphics &ADestGp, const Rect &ADestR, CxdImageInfo *ApSrcImg, TxdGpUIState AuiState, ImageAttributes *ApImgAtt = NULL);
void  DrawCommonImage(Graphics &ADestGp, const Rect &ADestR, CxdImageInfo *ApSrcImg, int AItemIndex, ImageAttributes *ApImgAtt = NULL);
void  DrawCommonImage(Graphics &ADestGp, const Rect &ADestR, Image *ASrcImg, const Rect &ASrcRect, TxdGpDrawStyle ADrawStyle, const int &APosX, const int &APosY, ImageAttributes *ApImgAtt = NULL);
TxdDrawResult DrawStretchImage(Graphics &ADestGp, Image *ASrcImg, const Rect &ADestRect, const Rect &ASrcRect, const int &APosX, const int &APosY, ImageAttributes *ApImgAtt = NULL);
void  DrawCommonText(Graphics &ADestGp, const TCHAR* AText, CxdFontInfo *ApFontInfo, RectF ADestRect, DWORD AFontColor, TxdGpUIState AuiState, REAL ATranslate = 1.0f);
void  DrawCommonText(Graphics &ADestGp, const TCHAR* AText, CxdFontInfo *ApFontInfo, const Rect &ADestRect, DWORD AFontColor, TxdGpUIState AuiState, REAL ATranslate = 1.0f);
CSize CalcStringSize(Graphics &AGh, const Gdiplus::Font *AFont, const TCHAR* ApText, int AMaxWidth = 0, int AMaxHeight = 0);

/*��ɫ����*/
void ColorMatrix_ToIdentity(ColorMatrix &Acm);
// ������ɫ�����͸���� [2012-7-30 17:34 by Terry]
void ColorMatrix_TranslateValue(ColorMatrix &Acm, const REAL &ATraslateValue);
// ���ûҶ���ɫ���� [2012-7-30 17:35 by Terry]
void ColorMatrix_Gray(ColorMatrix &Acm);

void InitTranslateImgAtt(ImageAttributes &Attr, const REAL &ATraslateValue = 0.5f);

//ʹ��ͼƬ��������״
HRGN  CreateStyleByBitmap(Bitmap *ABmp, byte ALessAlpha = 30);
//�Ҷ�ͼƬ
void  ChangedBmpToGray(Bitmap *ApBmp, byte ALessAlpha = 0);
//�ı�ͼƬ������: ABrightValue: ����: -255(��) - 255(��)
void  ChangedBmpBrightness(Bitmap *ApBmp, int ABrightValue);

//����Բ�Ǿ���·��: 2: UpLeft, 4: UpRight, 8: DownLeft, 16: DownRight
GraphicsPath* CreateRoundRectPath(const Rect &r, int ACornerRadius, int ARoundStyle = 2 | 4 | 8 | 16);

//����ָ����Ϣ����Bitmap, ���ڻ��ƿؼ�
Bitmap* CreateCtrlBitmap(TxdCreateCtrlBitmap *ApInfo);

//����ͼƬ
Gdiplus::Bitmap* LoadImageFromRes(HMODULE AResHandle, DWORD AResID, const TCHAR* AResType = TEXT("PNG"));
Gdiplus::Bitmap* DIBToBmp(LPBITMAPINFO lpBl);
#define LoadPngResImage(AID) LoadImageFromRes( AfxGetResourceHandle(), AID, TEXT("PNG") )
#define LoadJpgResImage(AID) LoadImageFromRes( AfxGetResourceHandle(), AID, TEXT("JPG") )
#define LoadBmpResImage(AID) LoadImageFromRes( AfxGetResourceHandle(), AID, TEXT("BMP") )
#define LoadGifResImage(AID) LoadImageFromRes( AfxGetResourceHandle(), AID, TEXT("GIF") )

#endif
