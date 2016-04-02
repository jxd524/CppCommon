/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdApplication.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 
	LastModify : 2013-11-21 15:18:29
	Description: 
**************************************************************************/

#pragma once

class CxdHintWindow;
class CxdHintManage;
struct _EXCEPTION_POINTERS;
#include "JxduiDefines.h"
#include "JxdAppUpdate.h"
#include "JxdStringHandle.h"

class CxdApplication : public CWinApp
{
public:
	CxdApplication();
    ~CxdApplication();
public:
	virtual BOOL InitInstance();
    virtual int  ExitInstance();
    virtual bool OnUnHandleException(_EXCEPTION_POINTERS* ExceptionInfo){ AfxMessageBox(TEXT("抱歉, 发生不可恢复的错误，软件将自动终止!")); return true; }

    //命令参数, 以=作为键值对
    bool    IsExistCommond(const TCHAR* const ApName);
    CString GetCommondValue(const TCHAR* const ApName);

    //自动更新
    TxdAppUpdate GetAppUpdateState(void);
    void         CheckAppUpdate(const TCHAR* ApConfigURL, int ADelaySecond = 300, const TCHAR* ApUpdateDirName = TEXT("update"));

	DECLARE_MESSAGE_MAP()
protected:
    //提示窗口
    virtual void DoCreateHintWindow(CxdHintWindow** AWnd);

    //判断指定目录是否存在ZIP文件,有则启动更新程序
    //bool CheckZipUpdate(const TCHAR* ApDir)

    //使用应用程序只运行一个实例, ApMainClassName必须为唯一值(GUID), 并为主窗体类名
    BOOL CheckRunOnes(const TCHAR* const ApMainClassName, bool bShow = true);

    //程序异常退出时, 记录到文件
    void SetUnHandleExceptionFileName(const TCHAR* const ApFileName = NULL);

    //自动更新实现函数
    virtual void DoUpdateDownZip(const TxdUpConfigInfo&, bool& AbContinuDo){}
    virtual void DoUpdateUnZipFile(const TxdUpConfigInfo&, bool& AbContinuDo){}
    virtual void DoUpdateRunApp(const TxdUpConfigInfo&, bool& AbContinuDo, CString& ARunParam){}
    virtual void DoUpdateCloseSelf(const CxdAppUpdate* ApUpdate){}
    virtual void DoUpdateFinished(const CxdAppUpdate* ApUpdate){ m_CurUpState = m_pUpdate->GetUpdateState(); SafeDeleteObject(m_pUpdate); } 
private:
    //命令行
    TStringList* m_pCommond;
    void CheckCommonList(void);

    //自动升级
    TxdAppUpdate m_CurUpState;
    CxdAppUpdate* m_pUpdate;

    HANDLE m_hMutex;

    //提示窗口
    IxdDataInterface *m_pNotifyCreateHintManage;
    void DoCreateHintManage(LPVOID);
    CxdHintManage *m_pfrmHintManage;

    //异常
    static CString m_strDumpFileName;
    static LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo);
};