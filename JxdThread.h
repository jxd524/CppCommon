/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdThread.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 15:00:48
	LastModify : 
	Description: 线程操作封装类
                 CxdThread: 注意线程如果没有join的pthread记得detach( 类中已处理 )
                   调用 BeginThread 将启动线程
                 CxdThreadEvent基本使用方法:
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

//创建简单运行的线程
#define CreateSimpleThread(TClassType, TParamType, AExecuteFunc, AParam)                                    \
    CxdThreadEvent<TParamType>::CreateThread( NewInterfaceByClass(TClassType, TParamType, AExecuteFunc),    \
    NULL, NULL, AParam, 0 )

//线程循环触发指定函数, 时间单位: 毫秒
#define CreateLoopExecuteThread(TClassType, TParamType, AExecuteFunc, AParam, ALoopSpaceTime)               \
    CxdThreadEvent<TParamType>::CreateThread( NewInterfaceByClass(TClassType, TParamType, AExecuteFunc),    \
    NULL, NULL, AParam, ALoopSpaceTime )

#if defined (WIN32) && (_MFC_VER)
//支持MFC
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
// 线程属性类, 目前只支持Linux
//====================================================================
//
class CxdThreadAttr
{
public:
    CxdThreadAttr(){ pthread_attr_init( &m_tAttr); }
    ~CxdThreadAttr() { pthread_attr_destroy( &m_tAttr); }
    pthread_attr_t* GetThreadAttr(void) { return &m_tAttr; }

    //分离状态: 
    /* PTHREAD_CREATE_DETACHED: 分离状态启动
       PTHREAD_CREATE_JOINABLE: 正常启动线程
    */
    bool IsDetachJoinable();
    void SetDetachState(bool ADetach);
     
    //继承性: 
    /* PTHREAD_INHERIT_SCHED： 新的线程继承创建线程的策略和参数！
       PTHREAD_EXPLICIT_SCHED：新的线程继承策略和参数来自于schedpolicy和schedparam属性中显式
                               设置的调度信息！
    */
    bool IsInheritSched();
    void SetInheritSched(bool AInheritSched);

    //调度策略:
    //SCHED_FIFO SCHED_RR SCHED_OTHER
    int  GetSchedPolicy();
    void SetSchedPolicy(int APolicy);
    static int GetMaxSchedPrority(int APolicy = SCHED_FIFO);
    static int GetMinSchedPrority(int APolicy = SCHED_FIFO);

    //调度参数
    int  GetPrority();
    void SetPrority(int APrority);

    //作用域
    /* PTHREAD_SCOPE_PROCESS: 进程内竞争资源
       PTHREAD_SCOPE_SYSTEM:  系统级竞争资源
    */
    bool IsScopeProcess();
    void SetScope(bool AProcess);

    //线程堆栈的大小
    size_t GetStackSize();
    void   SetStackSize(size_t ASize);
      
    //线程堆栈的地址 
    void* GetStackAddr();
    void  SetStackAddr(void *ApAddr); 
private:
    pthread_attr_t m_tAttr;
};

#endif

//====================================================================
// 线程基类, 支持Window及Linux
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
    //线程执行函数
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
// 线程事件
//====================================================================
//
template<typename TParamType>
class CxdThreadEvent: public CxdThread
{
public:
    typedef IxdNotifyEventT<TParamType> IxdThreadInterface;

    //静态函数, 用于创建 CxdThreadEvent 对象
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
    PropertyValue( LoopExecuteSpaceTime, int, true ); //循环执行 ThreadExecute 接口的间隔时间, 如果<=0 则表示不做循环
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