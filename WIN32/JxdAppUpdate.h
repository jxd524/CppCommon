/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdAppUpdate.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-12-19 14:16:14
	LastModify : 
	Description: Ӧ�ó����Զ��������ù�����
**************************************************************************/
#pragma once

#include "JxdXmlBasic.h"
#include "JxdEvent.h"
#include "JxdStringCoder.h"
#include "JxdStringHandle.h"
#include "JxdThread.h"
#include "JxdSynchro.h"
#include <vector>

class CxdUpConfigXml;
class CxdAppUpdate;
enum TxdAppUpdate{ auNULL, auChecking, auDownFile, auFinsihed, auRecentVersion, auFailed};

struct TxdUpConfigInfo
{
    CString name, version, perm, url, size, hash, closeParent; 
    CString content, downItems, unzipItems, runItems;
};

typedef IxdNotifyEventT<const CxdAppUpdate*> IxdAppUpdateInterface;
typedef IxdNotifyEventExT<const TxdUpConfigInfo&, bool&/*AbContinuDo*/> IxdUpdataHandleInterface;
typedef IxdNotifyEventEx3T<const TxdUpConfigInfo&, bool&/*AbContinuDo*/, CString&/*ARunParam*/> IxdRunAppInterface;

class CxdAppUpdate
{
public:
    CxdAppUpdate(const TCHAR* ApConfigURL, int ADelaySecond = 60 * 5, const TCHAR* ApUpdateDirName = TEXT("update"));
    ~CxdAppUpdate();

    void CheckNow(void);

    //״̬
    PropertyGetValue( UpdateState, TxdAppUpdate );

    //����ZIP�ļ�ʱ
    PropertyInterface( DownZip, IxdUpdataHandleInterface );

    //��ѹZIP�ļ�ʱ
    PropertyInterface( UnZipFile, IxdUpdataHandleInterface );

    //����EXEʱ
    PropertyInterface( RunApp, IxdRunAppInterface );

    //��Ҫ�رձ���ʱ
    PropertyInterface( CloseSelf, IxdAppUpdateInterface );

    //���߳�ִ�����ʱ
    PropertyInterface( Finished, IxdAppUpdateInterface );
private:
    CxdMutex m_lock;
    CString m_ConfigFileName;
    CString m_ConfigURL;
    CString m_AppName;
    CString m_CurPath;
    CString m_ZipTempDir;
    
    CxdUpConfigXml* m_pConfig;

    //�߳�
    bool m_bWaitFree;
    CxdThreadEvent<LPVOID>* m_pThread;
    void DoThreadCheckApp(LPVOID);
    void DoThreadFinished(LPVOID);

    CString GetTempZipFileName(const TCHAR* ApName) { return m_ZipTempDir + ApName + TEXT(".zip_temp"); }
    CString GetZipFileName(const TCHAR* ApName) { return m_ZipTempDir + ApName + TEXT(".zip"); }

    //����
    bool m_bCurDownNewVersion;
    int  DoDownByList(TStringList& AList);
    void DoUnzipByList(TStringList& AList);
    void DoRunAppByList(TStringList& AList);
};