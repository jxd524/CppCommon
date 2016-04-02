/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
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
    virtual bool OnUnHandleException(_EXCEPTION_POINTERS* ExceptionInfo){ AfxMessageBox(TEXT("��Ǹ, �������ɻָ��Ĵ���������Զ���ֹ!")); return true; }

    //�������, ��=��Ϊ��ֵ��
    bool    IsExistCommond(const TCHAR* const ApName);
    CString GetCommondValue(const TCHAR* const ApName);

    //�Զ�����
    TxdAppUpdate GetAppUpdateState(void);
    void         CheckAppUpdate(const TCHAR* ApConfigURL, int ADelaySecond = 300, const TCHAR* ApUpdateDirName = TEXT("update"));

	DECLARE_MESSAGE_MAP()
protected:
    //��ʾ����
    virtual void DoCreateHintWindow(CxdHintWindow** AWnd);

    //�ж�ָ��Ŀ¼�Ƿ����ZIP�ļ�,�����������³���
    //bool CheckZipUpdate(const TCHAR* ApDir)

    //ʹ��Ӧ�ó���ֻ����һ��ʵ��, ApMainClassName����ΪΨһֵ(GUID), ��Ϊ����������
    BOOL CheckRunOnes(const TCHAR* const ApMainClassName, bool bShow = true);

    //�����쳣�˳�ʱ, ��¼���ļ�
    void SetUnHandleExceptionFileName(const TCHAR* const ApFileName = NULL);

    //�Զ�����ʵ�ֺ���
    virtual void DoUpdateDownZip(const TxdUpConfigInfo&, bool& AbContinuDo){}
    virtual void DoUpdateUnZipFile(const TxdUpConfigInfo&, bool& AbContinuDo){}
    virtual void DoUpdateRunApp(const TxdUpConfigInfo&, bool& AbContinuDo, CString& ARunParam){}
    virtual void DoUpdateCloseSelf(const CxdAppUpdate* ApUpdate){}
    virtual void DoUpdateFinished(const CxdAppUpdate* ApUpdate){ m_CurUpState = m_pUpdate->GetUpdateState(); SafeDeleteObject(m_pUpdate); } 
private:
    //������
    TStringList* m_pCommond;
    void CheckCommonList(void);

    //�Զ�����
    TxdAppUpdate m_CurUpState;
    CxdAppUpdate* m_pUpdate;

    HANDLE m_hMutex;

    //��ʾ����
    IxdDataInterface *m_pNotifyCreateHintManage;
    void DoCreateHintManage(LPVOID);
    CxdHintManage *m_pfrmHintManage;

    //�쳣
    static CString m_strDumpFileName;
    static LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo);
};