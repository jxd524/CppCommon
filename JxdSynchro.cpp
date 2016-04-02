#include "stdafx.h"
#include "JxdSynchro.h"

#ifdef linux
#include <sys/mman.h>
#include <fcntl.h>
#endif

//CxdMutex
#ifdef WIN32
CxdMutex::CxdMutex()
{
    InitializeCriticalSection( &m_csLock );
}
#endif


#ifdef linux
CxdMutex::CxdMutex( bool AMultiProcess /*= false*/ )
{
    m_bMultiProcess = AMultiProcess;
    pthread_mutexattr_t attr_perf;  
    pthread_mutexattr_init( &attr_perf );
    if ( m_bMultiProcess )
    {
        pthread_mutexattr_setpshared( &attr_perf, PTHREAD_PROCESS_SHARED );  
        m_pMutex = (pthread_mutex_t *)mmap( NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0 );
    }
    else
    {
        m_pMutex = new pthread_mutex_t;
    }
    pthread_mutex_init(m_pMutex, &attr_perf);  
}
#endif

CxdMutex::~CxdMutex()
{
#ifdef WIN32
    DeleteCriticalSection( &m_csLock );
#endif

#ifdef linux
    pthread_mutex_destroy( m_pMutex );
    if ( m_bMultiProcess )
    {
        munmap( m_pMutex, sizeof(pthread_mutex_t) );
    }
    else
    {
        delete m_pMutex;
    }
#endif
}

//CxdSemaphore
CxdSemaphore::CxdSemaphore( unsigned int AValue, const char *ApSemName, bool AbAutoDestroyName, unsigned int AInitValue )
{
    m_bAtoDestroySemaphoreName = AbAutoDestroyName;
#ifdef linux
    if ( ApSemName == NULL )
    {
        m_pSem = new sem_t;
        sem_init( m_pSem, 0, AValue );
    }
    else
    {
        m_strSemName = ApSemName;
        m_pSem = sem_open( ApSemName, O_CREAT, 0666, AValue );
    }
    if ( AInitValue > 0 )
    {
        for( unsigned int i = 0; i != AInitValue; i++ )
        {
        	Post();
        }
    }
#endif
#ifdef WIN32
    m_hHandle = NULL;
    if ( ApSemName != NULL )
    {
        m_strSemName = ApSemName;
        m_hHandle = OpenSemaphoreA( SEMAPHORE_ALL_ACCESS, FALSE, ApSemName );
    }
    if ( m_hHandle == NULL )
    {
        if ( ApSemName != NULL ) m_strSemName = ApSemName;
        m_hHandle = CreateSemaphoreA( NULL, AInitValue, AValue, ApSemName );
    }
#endif
}

CxdSemaphore::~CxdSemaphore()
{
#ifdef linux
    if ( m_strSemName.empty() )
    {
        sem_destroy( m_pSem );
        delete m_pSem;
    }
    else
    {
        sem_close( m_pSem );
        if ( m_bAtoDestroySemaphoreName )
        {
            sem_unlink( m_strSemName.c_str() );
        }
    }
#endif
#ifdef WIN32
    if ( m_hHandle != NULL )
    {
        CloseHandle( m_hHandle );
        m_hHandle = NULL;
    }
#endif
}

#ifdef linux
//CxdReadWriteLock
CxdReadWriteLock::CxdReadWriteLock( bool AMultiProcess /*= false */ )
{
    m_bMultiProcess = AMultiProcess;
    pthread_rwlockattr_t attr_pub;   
    pthread_rwlockattr_init( &attr_pub ); 
    if ( m_bMultiProcess )
    {
        pthread_rwlockattr_setpshared( &attr_pub, PTHREAD_PROCESS_SHARED );   
        m_pLock = (pthread_rwlock_t *)mmap(NULL, sizeof(pthread_rwlock_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);    
    }
    else
    {
        m_pLock = new pthread_rwlock_t;
    }
    pthread_rwlock_init( m_pLock, &attr_pub );  
}

CxdReadWriteLock::~CxdReadWriteLock()
{
    pthread_rwlock_destroy( m_pLock );
    if ( m_bMultiProcess )
    {
        munmap( m_pLock, sizeof(pthread_rwlock_t) );
    }
    else
    {
        delete m_pLock;
    }
}
#endif