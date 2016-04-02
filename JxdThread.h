/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdThread.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 15:00:48
	LastModify : 
	Description: �̲߳�����װ��
                 CxdThread: ע���߳����û��join��pthread�ǵ�detach( �����Ѵ��� )
                   ���� BeginThread �������߳�
                 CxdThreadEvent����ʹ�÷���:
                 class CMyTest
                 {
                 public:
                    void DoTest(int AParam)
                    {
                        Debug( Format("DoTest: %d", AParam).c_str() );
                    }
                 };
                 //
                 CMyTest o;
                 CxdThreadEvent<CMyTest, int> obj;
                 obj.SetAutoFreeThreadObject( false );
                 obj.SetThreadParam( 100 );
                 obj.GetEventThreadExecute()->SetParam( &o, &CMyTest::DoTest );
                 obj.BeginThread();
                 obj.WaitFinished();
**************************************************************************/
#pragma once

#ifndef _JxdThread_Lib
#define _JxdThread_Lib

#ifdef WIN32
#include <process.h>
#endif

#ifdef linux
#include <pthread.h>
#endif


#include "JxdMacro.h"
#include "JxdEvent.h"
#include "JxdTypedef.h"
#include "JxdSynchro.h"

//���������е��߳�
#define CreateSimpleThread(TClassType, TParamType, AExecuteFunc, AParam)                                    \
    CxdThreadEvent<TParamType>::CreateThread( NewInterfaceByClass(TClassType, TParamType, AExecuteFunc),    \
    NULL, NULL, AParam, 0 )

//�߳�ѭ������ָ������, ʱ�䵥λ: ����
#define CreateLoopExecuteThread(TClassType, TParamType, AExecuteFunc, AParam, ALoopSpaceTime)               \
    CxdThreadEvent<TParamType>::CreateThread( NewInterfaceByClass(TClassType, TParamType, AExecuteFunc),    \
    NULL, NULL, AParam, ALoopSpaceTime )

#if defined (WIN32) && (_MFC_VER)
//֧��MFC
#define CreateSimpleWinThread(TClassType, TParamType, AExecuteFunc, AParam)                                 \
    CxdThreadEvent<TParamType>::CreateThread( NewInterfaceByClass(TClassType, TParamType, AExecuteFunc),    \
    NULL, NULL, AParam, 0, true )
#define CreateLoopExecuteWinThread(TClassType, TParamType, AExecuteFunc, AParam, ALoopSpaceTime)            \
    CxdThreadEvent<TParamType>::CreateThread( NewInterfaceByClass(TClassType, TParamType, AExecuteFunc),    \
    NULL, NULL, AParam, ALoopSpaceTime, true )
#endif


#ifdef _MFC_VER
#endif


#ifdef linux
//====================================================================
// �߳�������, Ŀǰֻ֧��Linux
//====================================================================
//
class CxdThreadAttr
{
public:
    CxdThreadAttr(){ pthread_attr_init( &m_tAttr); }
    ~CxdThreadAttr() { pthread_attr_destroy( &m_tAttr); }
    pthread_attr_t* GetThreadAttr(void) { return &m_tAttr; }

    //����״̬: 
    /* PTHREAD_CREATE_DETACHED: ����״̬����
       PTHREAD_CREATE_JOINABLE: ���������߳�
    */
    bool IsDetachJoinable();
    void SetDetachState(bool ADetach);
     
    //�̳���: 
    /* PTHREAD_INHERIT_SCHED�� �µ��̼̳߳д����̵߳Ĳ��ԺͲ�����
       PTHREAD_EXPLICIT_SCHED���µ��̼̳߳в��ԺͲ���������schedpolicy��schedparam��������ʽ
                               ���õĵ�����Ϣ��
    */
    bool IsInheritSched();
    void SetInheritSched(bool AInheritSched);

    //���Ȳ���:
    //SCHED_FIFO SCHED_RR SCHED_OTHER
    int  GetSchedPolicy();
    void SetSchedPolicy(int APolicy);
    static int GetMaxSchedPrority(int APolicy = SCHED_FIFO);
    static int GetMinSchedPrority(int APolicy = SCHED_FIFO);

    //���Ȳ���
    int  GetPrority();
    void SetPrority(int APrority);

    //������
    /* PTHREAD_SCOPE_PROCESS: �����ھ�����Դ
       PTHREAD_SCOPE_SYSTEM:  ϵͳ��������Դ
    */
    bool IsScopeProcess();
    void SetScope(bool AProcess);

    //�̶߳�ջ�Ĵ�С
    size_t GetStackSize();
    void   SetStackSize(size_t ASize);
      
    //�̶߳�ջ�ĵ�ַ 
    void* GetStackAddr();
    void  SetStackAddr(void *ApAddr); 
private:
    pthread_attr_t m_tAttr;
};

#endif

//====================================================================
// �̻߳���, ֧��Window��Linux
//====================================================================
//
class CxdThread
{
public:
    CxdThread(bool AAutoFree = true);
    virtual ~CxdThread();

    bool BeginThread(
#ifdef linux
        CxdThreadAttr *ApAttr = NULL
#endif
        );

    void CancelThread();
    void WaitFinished(); 
    PropertyValue( AutoFreeThreadObject, bool, true );    
    PropertyGetValue( CanWaitThreadFinished, bool );
    PropertyGetValue( Running, bool );
    
#ifdef WIN32
    PropertyGetValue( ThreadID, unsigned );
    PropertyGetValue( ThreadHandle, HANDLE );

    #ifdef _MFC_VER
        PropertyValue( WinThread, bool, !m_Running );
    #endif

#endif

#ifdef linux 
    PropertyGetValue( ThreadID, pthread_t );
#endif

protected:
    //�߳�ִ�к���
    virtual void OnExecute(){}
    virtual void OnTerminate(){}
private:

#ifdef WIN32
    static unsigned __stdcall _Win32ThreadRunning(void *ApParam);

    #ifdef _MFC_VER
        static UINT AfxThreadFunction(LPVOID pthreadParam);
    #endif

#endif

#ifdef linux
    static void* _LinuxThreadRunning(void* ApParam);
#endif

    static void _ThreadFinished(void* ApParam);
};


//====================================================================
// �߳��¼�
//====================================================================
//
template<typename TParamType>
class CxdThreadEvent: public CxdThread
{
public:
    typedef IxdNotifyEventT<TParamType> IxdThreadInterface;

    //��̬����, ���ڴ��� CxdThreadEvent ����
    static CxdThreadEvent<TParamType>* CreateThread(IxdThreadInterface *ApExecute, IxdThreadInterface *ApExecuteFinished,
        IxdThreadInterface *ApTerminate, const TParamType &AParam, int ALoopExecuteSpaceTime
#if defined (WIN32) && (_MFC_VER)
        ,bool AbCreateWinThread = false
#endif
        )
    {
        CxdThreadEvent<TParamType> *pThread = new CxdThreadEvent<TParamType>;
        pThread->SetAutoFreeThreadObject( true );
        pThread->SetThreadParam( AParam );
        pThread->SetItThreadExecute( ApExecute );
        pThread->SetItThreadExecuteFinished( ApExecuteFinished );
        pThread->SetItThreadTerminate( ApTerminate );
        pThread->SetLoopExecuteSpaceTime( ALoopExecuteSpaceTime );
#if defined (WIN32) && (_MFC_VER)
        pThread->SetWinThread( AbCreateWinThread );
#endif
        if ( !pThread->BeginThread() )
        {
            delete pThread;
            pThread = NULL;
        }
        return pThread;
    }

    //class
    CxdThreadEvent()
    {
        SetLoopExecuteSpaceTime( 0 );
        InitInterface( ThreadExecute );
        InitInterface( ThreadExecuteFinished );
        InitInterface( ThreadTerminate );
        m_pSem = NULL;
    }
    ~CxdThreadEvent()
    {
        DeleteInterface( ThreadExecute );
        DeleteInterface( ThreadExecuteFinished );
        DeleteInterface( ThreadTerminate );
        SafeDeleteObject( m_pSem );
    }
    void PostEvent(void) { if (m_pSem != NULL) m_pSem->Post(); }
    PropertyInterface( ThreadExecute, IxdThreadInterface );
    PropertyInterface( ThreadExecuteFinished, IxdThreadInterface );
    PropertyInterface( ThreadTerminate, IxdThreadInterface );
    PropertyValue( ThreadParam, TParamType, true );
    PropertyValue( LoopExecuteSpaceTime, int, true ); //ѭ��ִ�� ThreadExecute �ӿڵļ��ʱ��, ���<=0 ���ʾ����ѭ��
protected:
    virtual void OnExecute()
    {
        if ( m_LoopExecuteSpaceTime > 0 )
        {
            if ( m_pSem == NULL ) m_pSem = new CxdSemaphore;
            while ( m_LoopExecuteSpaceTime > 0 )
            {
                m_pSem->Wait(m_LoopExecuteSpaceTime);
                if ( m_LoopExecuteSpaceTime > 0 )
                {
                    NotifyInterface( ThreadExecute, GetThreadParam() );
                }
            }
        }
        else
        {
            NotifyInterface( ThreadExecute, GetThreadParam() );
        }
        NotifyInterface( ThreadExecuteFinished, GetThreadParam() );
    }
    virtual void OnTerminate()
    {
        NotifyInterface( ThreadTerminate, GetThreadParam() );
    }
private:
    CxdSemaphore *m_pSem;
};

#endif