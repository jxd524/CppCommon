#pragma once

#ifndef _JxdZipSub_Lib
#define _JxdZipSub_Lib

//ѹ��ָ��Ŀ¼, ApExceptItemsָ���������ѹ������. ��ʽ: TEXT("a.txt,dir\\b.bmp")
bool ZipFiles(const TCHAR* ApDir, const TCHAR* ApZipFileName, const TCHAR* ApExceptItems = NULL);

//��ѹZIP����ָ��Ŀ¼, ApExceptItemsָ��������н�ѹ, ��ʽ TEXT("a.txt,dir/b.bmp")
bool UnZipFiles(const TCHAR* ApDir, const TCHAR* ApZipFileName, const TCHAR* ApExceptItems = NULL);

#endif