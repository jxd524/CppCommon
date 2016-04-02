/**********************************************************
Copyright  : pp.cc ����ָ��ʱ�� ɺ����
File       : JxdFileSub.h & .cpp
Author     : Terry
Email      : jxd524@163.com
CreateTime : 2013-12-7 16:56:53
LastModify : 
Description: �ļ���ز�����װ
***********************************************************/
#pragma once
#ifndef _JxdFileSub_Lib
#define _JxdFileSub_Lib

#include <string>
using std::string;
using std::wstring;

#ifdef WIN32
//Windown �¶���
#define FileSeparator '\\'
struct TFileInfo 
{
    FILETIME ftCreationTime;  
    FILETIME ftLastAccessTime;  
    FILETIME ftLastWriteTime;
    INT64    nFileSize;
    wstring  strFileName;
};
#endif

#ifdef linux
//linux �¶���
#include "JxdTypedef.h"
#define FileSeparator '/'
struct TFileInfo 
{
    wstring  strFileName;
};
#endif

#include "JxdEvent.h"
typedef IxdNotifyEventExT<TFileInfo*, bool&> IxdLoopFileInterface;

//�ṩ�ⲿ���ڱ����ļ���
class CxdLoopFileHandle: public IxdLoopFileInterface
{
protected:
    //�����ļ�, ���� FALSE ��ʾ���ٱ���
    virtual bool DoLoopFile(TFileInfo* ApInfo) { return true; };

    virtual inline bool  Notify(TFileInfo* ApInfo, bool& AbTerminate) const 
    { 
        CxdLoopFileHandle* pThis = const_cast<CxdLoopFileHandle*>(this);
        AbTerminate = !pThis->DoLoopFile(ApInfo); 
        return true; 
    }
    virtual inline INT64 GetOwnerAddress(void) const { return 0; }
    virtual inline INT64 GetFuncAddress(void) const { return 0; }
};

//�ж��ļ��Ƿ����
bool FileExist(const char* ApFileName);

//�ж�Ŀ¼�Ƿ����
bool DirectoryExist(const char *ApDirName);

//��ȡ���һ�� FileSeparator ֮ǰ�������ַ���
string  ExtractFilePath(const char* ApFileFullName);
wstring ExtractFilePath(const wchar_t* ApFileFullName);

//��ȡ���һ�� FileSeparator ֮����ַ���
string  ExtractFileName(const char* ApFileFullName);
wstring ExtractFileName(const wchar_t* ApFileFullName);

//��ȡ���� "." ֮����ַ���, ������ ".", �� exe txt��
string  ExtractFileExt(const char* ApFileFullName);
wstring ExtractFileExt(const wchar_t* ApFileFullName);

//ȷ�����һ���ַ�����Ϊ: FileSeparator
string  ExcludeTrailingPathDelimiter(const char* ApFileFullName);
wstring ExcludeTrailingPathDelimiter(const wchar_t* ApFileFullName);

//ȷ�����һ���ַ�Ϊ: FileSeparator
string  IncludeTrailingPathDelimiter(const char* ApFileFullName);
wstring IncludeTrailingPathDelimiter(const wchar_t* ApFileFullName);

//֧�ִ������Ŀ¼
bool ForceDirectories(const char* ApDir);

//֧�ֶ��ɾ��, ������Ŀ¼�����ļ�
bool DeleteDirectiories(const char* ApDir);

//����Ŀ¼�е��ļ�, �¼����ݵĲ������ͣ�TFileInfo
int LoopDirectiories(const wchar_t* ApDir, IxdLoopFileInterface *ApInterface, bool bFindSub);

INT64 GetFileSize(const char *ApFileName);

//�����Ƿ����ָ�����͵��ļ�, ApExtName: exe,doc,..
bool IsExistExtFile(const wchar_t *ASrcPath, const wchar_t *ApExtName);

//====================================================================
// ������Windowsϵͳ 
//====================================================================
//
#ifdef WIN32
    bool  FileExist(const wchar_t* ApFileName);
    bool  DirectoryExist(const wchar_t* ApDirName);
    bool  ForceDirectories(const wchar_t* ApDir);
    bool  DeleteDirectiories(const wchar_t* ApDir);
    
    INT64 GetFileSize(const wchar_t *ApFileName);

    //LocalApp·��
    bool  GetLocalAppPath(string &APath);
    bool  GetLocalAppPath(wstring &APath);

    //�����еĳ���ȫ·��
    string  GetAppFileNameA(HMODULE hModule = NULL);
    wstring GetAppFileNameW(HMODULE hModule = NULL);

    //�õ�ϵͳĿ¼
    string  GetSysPathA(void);
    wstring GetSysPathW(void);

    //ϵͳ�û�·��
    string  GetUserPathA(void);
    wstring GetUserPathW(void);

    //ϵͳ��ʱ·��
    string  GetSysTempPathA(void);
    wstring GetSysTempPathW(void);

    //��ȡָ��Ŀ¼��һ���޴��ڵ��ļ���������·������ ApPathΪ��ʱ����ʾϵͳ��ʱ·��
    string  GetTempFileNameA( const char *ApExt = "._t_tmp", const char *ApPath = NULL );
    wstring GetTempFileNameW( const wchar_t *ApExt = L"._t_tmp", const wchar_t *ApPath = NULL );

#ifdef UNICODE 
    #define GetAppFileName GetAppFileNameW
    #define GetSysPath GetSysPathW
    #define GetUserPath GetUserPathW
    #define GetSysTempPath GetSysTempPathW
    #define GetTempFileName GetTempFileNameW
#else
    #define GetAppFileName GetAppFileNameA
    #define GetSysPath GetSysPathA
    #define GetUserPath GetUserPathA
    #define GetSysTempPath GetSysTempPathA
    #define GetTempFileName GetTempFileNameA
#endif

//��ȡ��ǰ�������� ��: wbClient
#define GetCurProcessName() GetTheStringFromBack( ExtractFileName( GetAppFileName().c_str() ).c_str(), TEXT("."), NULL, tpForward, 0 ).c_str()

//��ȡ��ǰ����·�� 
#define GetCurProcessPath() ExtractFilePath( GetAppFileName().c_str() ).c_str()

#endif


/***********************************************************************
    Linux �����
************************************************************************/
#ifdef linux
    string GetAppFileName( void );
#endif

#endif