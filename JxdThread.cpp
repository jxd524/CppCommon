#include "stdafx.h"
#include "JxdThread.h"

#ifdef linux
//CxdThreadAttr
bool CxdThreadAttr::IsDetachJoinable()
{
    int nState;
    pthread_attr_getdetachstate( &m_tAttr, &nState );
    return PTHREAD_CREATE_JOINABLE == nState;
}

void CxdThreadAttr::SetDetachState( bool ADetach )
{
    int nState( ADetach ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE );
    pthread_attr_setdetachstate( &m_tAttr, nState );
}

bool CxdThreadAttr::IsInheritSched()
{
    int nSched;
    pthread_attr_getinheritsched( &m_tAttr, &nSched );
    return PTHREAD_INHERIT_SCHED == nSched;
}

void CxdThreadAttr::SetInheritSched( bool AInheritSched )
{
    int nSched( AInheritSched ? PTHREAD_INHERIT_SCHED : PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setinheritsched( &m_tAttr, nSched );
}

int CxdThreadAttr::GetSchedPolicy()
{
    int nPolicy;
    pthread_attr_getschedpolicy( &m_tAttr, &nPolicy );
    return nPolicy;
}

void CxdThreadAttr::SetSchedPolicy( int APolicy )
{
    pthread_attr_setschedpolicy( &m_tAttr, APolicy );
}

int CxdThreadAttr::GetMaxSchedPrority( int APolicy /*= SCHED_FIFO*/ )
{
    return sched_get_priority_max( APolicy );
}

int CxdThreadAttr::GetMinSchedPrority( int APolicy /*= SCHED_FIFO*/ )
{
    return sched_get_priority_min( APolicy );
}

int CxdThreadAttr::GetPrority()
{
    sched_param p;
    pthread_attr_getschedparam( &m_tAttr, &p );
    return p.__sched_priority;
}

void CxdThreadAttr::SetPrority( int APrority )
{
    sched_param p;
    p.__sched_priority = APrority;
    pthread_attr_setschedparam( &m_tAttr, &p );
}

bool CxdThreadAttr::IsScopeProcess()
{
    int nScope;
    pthread_attr_getscope( &m_tAttr, &nScope );
    return PTHREAD_SCOPE_PROCESS == nScope;
}

void CxdThreadAttr::SetScope( bool AProcess )
{
    int nScope( AProcess ? PTHREAD_SCOPE_PROCESS : PTHREAD_SCOPE_SYSTEM );
    pthread_attr_setscope( &m_tAttr, nScope );
}

size_t CxdThreadAttr::GetStackSize()
{
    size_t s;
    pthread_attr_getstacksize( &m_tAttr, &s );
    return s;
}

void CxdThreadAttr::SetStackSize( size_t ASize )
{
    pthread_attr_setstacksize( &m_tAttr, ASize );
}

void* CxdThreadAttr::GetStackAddr()
{
    void *pAddr;
    size_t s;
    pthread_attr_getstack( &m_tAttr, &pAddr, &s );
    return pAddr;
}

void CxdThreadAttr::SetStackAddr( void *ApAddr )
{
    void *pOldAddr;
    size_t s;
    pthread_attr_getstack( &m_tAttr, &pOldAddr, &s );
    pthread_attr_setstack( &m_tAttr, ApAddr, s );
}
#endif

//CxdThread
CxdThread::CxdThread(bool AAutoFree)
{
    m_Running = false;
    m_AutoFreeThreadObject = AAutoFree;
    m_ThreadID = 0;
    m_CanWaitThreadFinished = false;
#ifdef WIN32
    m_ThreadHandle = (HANDLE)-1;
#ifdef _MFC_VER
    m_WinThread = false;
#endif
#endif
}

CxdThread::~CxdThread()
{
#ifdef linux
    if ( m_CanWaitThreadFinished && m_ThreadID > 0 )
    {
        pthread_join( m_ThreadID, 0 );
    }
#endif
}


void CxdThread::_ThreadFinished( void* ApParam )
{
    CxdThread *pObj = (CxdThread*)ApParam;
    pObj->OnTerminate();
    pObj->m_Running = false;
    if ( pObj->GetAutoFreeThreadObject() )
    {
        pObj->m_CanWaitThreadFinished = false;
#ifdef linux
        pthread_detach( pObj->GetThreadID() );
#endif
#ifdef WIN32
        pObj->m_ThreadHandle = (HANDLE)-1;
#endif
        delete pObj;
    }
}

#ifdef WIN32
unsigned __stdcall CxdThread::_Win32ThreadRunning( void *ApParam )
{
    CxdThread *pObj = (CxdThread*)ApParam;
    bool bWin = false;
#ifdef _MFC_VER
    if ( pObj->GetWinThread() ) bWin = true;
#endif
    try
    {
        pObj->OnExecute();
    }
    catch (...)
    {
    	
    }
    _ThreadFinished( pObj );

#ifdef _MFC_VER
    if ( bWin )
    {
        AfxEndThread(0);
    }
    else
    {
        _endthreadex( 0 );
    }
#endif

    return 0;
}

#ifdef _MFC_VER
UINT CxdThread::AfxThreadFunction( LPVOID pthreadParam )
{
    _Win32ThreadRunning( pthreadParam );
    return 0;
}
#endif

#endif

#ifdef linux
void* CxdThread::_LinuxThreadRunning( void* ApParam )
{
    CxdThread *pObj = (CxdThread*)ApParam;
    pthread_cleanup_push( &_ThreadFinished, ApParam ); 
    pObj->OnExecute();
    pthread_cleanup_pop( 1 );
    pthread_exit( 0 );
}
#endif

bool CxdThread::BeginThread( 
#ifdef linux
    CxdThreadAttr *ApAttr/* = NULL*/
#endif
    )
{
#ifdef WIN32
    if ( m_Running )
    {
        return false;
    }
    m_Running = true;
    m_CanWaitThreadFinished = true;
#ifdef _MFC_VER
    if ( GetWinThread() )
    {
        CWinThread *pWndThread = AfxBeginThread( &AfxThreadFunction, (LPVOID)this );
        m_ThreadHandle = pWndThread->m_hThread;
        m_ThreadID = pWndThread->m_nThreadID;
    }
    else
    {
        m_ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, &_Win32ThreadRunning, this, 0, &m_ThreadID);
    }
#else
    m_ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, &_Win32ThreadRunning, this, 0, &m_ThreadID);
#endif

    bool bOK = m_ThreadHandle != (HANDLE)-1;
    if ( !bOK ) m_Running = false;
    return bOK;
#endif

#ifdef linux
    if ( m_Running ) return false;
    m_Running = true;
    m_CanWaitThreadFinished = true;
    if ( ApAttr != NULL && !ApAttr->IsDetachJoinable() )
    {
        m_CanWaitThreadFinished = false;
    }
    if (0 != pthread_create( &m_ThreadID, ApAttr == NULL ? NULL : ApAttr->GetThreadAttr(), 
                                &_LinuxThreadRunning, this ) )
    {
        m_Running = false;
        return false;
    }
    return true;
#endif
}


void CxdThread::WaitFinished()
{
    if ( m_Running && m_CanWaitThreadFinished )
    {
#ifdef linux
        pthread_join( m_ThreadID, NULL );
#endif  
#ifdef WIN32
        while ( m_Running )
        {
            Sleep( 10 );
        }
#endif
    }
}

void CxdThread::CancelThread()
{
    if ( m_Running )
    {
#ifdef WIN32
        TerminateThread( m_ThreadHandle, 0 );
#endif

#ifdef linux
        pthread_cancel( m_ThreadID );
#endif
        
    }
}