/**********************************************************
Copyright  : pp.cc 深圳指掌时代 珊瑚组
File       : JxdFileSub.h & .cpp
Author     : Terry
Email      : jxd524@163.com
CreateTime : 2013-12-7 16:56:53
LastModify : 
Description: 文件相关操作封装
***********************************************************/
#pragma once
#ifndef _JxdFileSub_Lib
#define _JxdFileSub_Lib

#include <string>
using std::string;
using std::wstring;

#ifdef WIN32
//Windown 下定义
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
//linux 下定义
#include "JxdTypedef.h"
#define FileSeparator '/'
struct TFileInfo 
{
    wstring  strFileName;
};
#endif

#include "JxdEvent.h"
typedef IxdNotifyEventExT<TFileInfo*, bool&> IxdLoopFileInterface;

//提供外部用于遍历文件夹
class CxdLoopFileHandle: public IxdLoopFileInterface
{
protected:
    //遍历文件, 返回 FALSE 表示不再遍历
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

//判断文件是否存在
bool FileExist(const char* ApFileName);

//判断目录是否存在
bool DirectoryExist(const char *ApDirName);

//获取最后一个 FileSeparator 之前的所有字符串
string  ExtractFilePath(const char* ApFileFullName);
wstring ExtractFilePath(const wchar_t* ApFileFullName);

//获取最后一个 FileSeparator 之后的字符串
string  ExtractFileName(const char* ApFileFullName);
wstring ExtractFileName(const wchar_t* ApFileFullName);

//获取符号 "." 之后的字符串, 不包含 ".", 如 exe txt等
string  ExtractFileExt(const char* ApFileFullName);
wstring ExtractFileExt(const wchar_t* ApFileFullName);

//确保最后一个字符不是为: FileSeparator
string  ExcludeTrailingPathDelimiter(const char* ApFileFullName);
wstring ExcludeTrailingPathDelimiter(const wchar_t* ApFileFullName);

//确保最后一个字符为: FileSeparator
string  IncludeTrailingPathDelimiter(const char* ApFileFullName);
wstring IncludeTrailingPathDelimiter(const wchar_t* ApFileFullName);

//支持创建多层目录
bool ForceDirectories(const char* ApDir);

//支持多层删除, 包含子目录及其文件
bool DeleteDirectiories(const char* ApDir);

//遍历目录中的文件, 事件传递的参数类型：TFileInfo
int LoopDirectiories(const wchar_t* ApDir, IxdLoopFileInterface *ApInterface, bool bFindSub);

INT64 GetFileSize(const char *ApFileName);

//查找是否存在指定类型的文件, ApExtName: exe,doc,..
bool IsExistExtFile(const wchar_t *ASrcPath, const wchar_t *ApExtName);

//====================================================================
// 仅用于Windows系统 
//====================================================================
//
#ifdef WIN32
    bool  FileExist(const wchar_t* ApFileName);
    bool  DirectoryExist(const wchar_t* ApDirName);
    bool  ForceDirectories(const wchar_t* ApDir);
    bool  DeleteDirectiories(const wchar_t* ApDir);
    
    INT64 GetFileSize(const wchar_t *ApFileName);

    //LocalApp路径
    bool  GetLocalAppPath(string &APath);
    bool  GetLocalAppPath(wstring &APath);

    //运行中的程序全路径
    string  GetAppFileNameA(HMODULE hModule = NULL);
    wstring GetAppFileNameW(HMODULE hModule = NULL);

    //得到系统目录
    string  GetSysPathA(void);
    wstring GetSysPathW(void);

    //系统用户路径
    string  GetUserPathA(void);
    wstring GetUserPathW(void);

    //系统临时路径
    string  GetSysTempPathA(void);
    wstring GetSysTempPathW(void);

    //获取指定目录下一个无存在的文件名（包含路径）。 ApPath为空时，表示系统临时路径
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

//获取当前进程名称 如: wbClient
#define GetCurProcessName() GetTheStringFromBack( ExtractFileName( GetAppFileName().c_str() ).c_str(), TEXT("."), NULL, tpForward, 0 ).c_str()

//获取当前进程路径 
#define GetCurProcessPath() ExtractFilePath( GetAppFileName().c_str() ).c_str()

#endif


/***********************************************************************
    Linux 下相关
************************************************************************/
#ifdef linux
    string GetAppFileName( void );
#endif

#endif