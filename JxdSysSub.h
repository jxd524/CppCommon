/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdSysSub.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 15:00:05
	LastModify : 
	Description: 相关系统API进一步封闭文件
**************************************************************************/
#pragma once

#ifndef _JxdSysSub_Lib
#define _JxdSysSub_Lib

#include <string>
using std::string;
using std::wstring;

/***********************************************************************
    Window 下相关
************************************************************************/
#ifdef WIN32
/*头文件*/
#include <Windows.h>

/*函数*/
void OpenIE(const TCHAR* ApURL, bool ATryOpenIE = true);
void BringWindowToShow(HWND hWnd);

CString GetAppVersion(const TCHAR* AppName);
CString GetVersionMark();
CString GetSystemName();

CString GetIEVersion();
bool    IsIE6OrIE7Version(void);

inline BOOL IsCtrlPressed() { return ((GetKeyState(VK_CONTROL) & (1<<(sizeof(SHORT)*8-1))) != 0); }
inline BOOL IsAltPressed()  { return ((GetKeyState(VK_MENU) & (1<<(sizeof(SHORT)*8-1))) != 0);}
inline BOOL IsShiftPressed(){ return ((GetKeyState(VK_SHIFT) & (1<<(sizeof(SHORT)*8-1))) != 0); }

void CreateStartShortCut(const TCHAR *ApAppFileName, const char *ApShortCut);

//将指定资源释放到指定文件中(ApFileName); 
bool ExtenResource(HMODULE AResHandle, DWORD AResID, const TCHAR* AResType, const TCHAR* ApFileName);

#define ExtenZipResource(ID, fileName) ExtenResource( AfxGetResourceHandle(), ID, TEXT("ZIP"), fileName );

#endif




#endif