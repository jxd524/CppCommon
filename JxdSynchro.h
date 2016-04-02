/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdSynchro.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 14:58:24
	LastModify : //  [10/23/2013 terry]
	Description: 提供对线程及进程同步操作.包含互斥, 读写锁, 信号量
**************************************************************************/
#pragma once

#ifndef _JxdSynchro_Lib
#define _JxdSynchro_Lib

#ifdef linux
#include <pthread.h>
#include <semaphore.h>
#include "JxdTypedef.h"
#include "JxdMacro.h"
#endif

#include <string>
using namespace std;

#ifdef linux
//加
#define InterlockedAdd( v )             __sync_add_and_fetch( &v, 1 )
#define InterlockedAddVar( value, var ) __sync_add_and_fetch( &value, (var) )
#define InterlockedAdd64     InterlockedAdd
#define InterlockedAddVar64  InterlockedAddVar

//减
#define InterlockedDec( v )             __sync_sub_and_fetch( &v, 1 )
#define InterlockedDecVar( value, var ) __sync_sub_and_fetch( &value, var )
#define InterlockedDec64         InterlockedDec
#define InterlockedDecVar64      InterlockedDecVar

//位运算
#define InterlockedOr( value, var )  __sync_or_and_fetch( &value, var )
#define InterlockedAnd( value, var ) __sync_and_and_fetch( &value, var )
#define InterlockedXor( value, var ) __sync_xor_and_fetch( &value, var )

#endif

#ifdef WIN32
#define InterlockedAdd( v )               InterlockedExchangeAdd( &v, 1 )
#define InterlockedAddVar( value, var )   InterlockedExchangeAdd( &value, (var) )
//64位操作,只支持Vista及以上版本, XP会报错
#define InterlockedAdd64( v )             InterlockedExchangeAdd64( &v, 1 )
#define InterlockedAddVar64( value, var ) InterlockedExchangeAdd64( &value, (var) )

//减
#define InterlockedDec( v )               InterlockedExchangeAdd( &v, -1 )
#define InterlockedDecVar( value, var )   InterlockedExchangeAdd( &value, -(var) )
//64位操作,只支持Vista及以上版本, XP会报错
#define InterlockedDec64( v )             InterlockedExchangeAdd64( &v, -1 )
#define InterlockedDecVar64( value, var ) InterlockedExchangeAdd64( &value, -(var) )

//位运算
//#define InterlockedOr( value, var )  InterlockedOr( &value, var )
//#define InterlockedAnd( value, var ) __sync_and_and_fetch( &value, var )
//#define InterlockedXor( value, var ) __sync_xor_and_fetch( &value, var )
#endif

//====================================================================
// CxdMutex
//====================================================================
//
class CxdMutex
{
public:
#ifdef WIN32
    CxdMutex();
#endif

#ifdef linux
    CxdMutex(bool AMultiProcess = false);
#endif
    ~CxdMutex();

    //Lock
    inline bool Lock(void) 
    { 
#ifdef WIN32 
        EnterCriticalSection( &m_csLock ); 
        return true; 
#endif
#ifdef linux
        return 0 == pthread_mutex_lock(m_pMutex); 
#endif
    }

    //Unlock
    inline void Unlock(void) 
    { 
#ifdef WIN32
        LeaveCriticalSection( &m_csLock );
#endif
#ifdef linux
        pthread_mutex_unlock(m_pMutex); 
#endif
    }

    //TryLock
    inline bool TryLock(void) 
    { 
#ifdef linux
        return 0 == pthread_mutex_trylock(m_pMutex);
#endif
#ifdef WIN32
        return TRUE == TryEnterCriticalSection( &m_csLock );
#endif
    }

private:

#ifdef WIN32
    CRITICAL_SECTION m_csLock;
#endif

#ifdef linux
    bool m_bMultiProcess;
    pthread_mutex_t *m_pMutex; 
#endif
};

//====================================================================
// CxdAutoMutex
//====================================================================
//
#define AutoMutex(AMutex) CxdAutoMutex _x_d_Auto_Mutex_##AMutex(&AMutex); 
class CxdAutoMutex
{
public:
    CxdAutoMutex(CxdMutex *ApMutex){ ApMutex->Lock(); m_pMutex = ApMutex; }
    ~CxdAutoMutex() { m_pMutex->Unlock(); }
private:
    CxdMutex *m_pMutex;
};

//====================================================================
// CxdAutoMutexVal
//====================================================================
//
#define AutoMutexVal(AMutex) CxdAutoMutexVal _x_d_Auto_Mutex_val( &AMutex );
class CxdAutoMutexVal
{
#ifdef WIN32
public:
    CxdAutoMutexVal(CRITICAL_SECTION *ApMutex){ EnterCriticalSection(ApMutex); m_pMutex = ApMutex; }
    ~CxdAutoMutexVal() { LeaveCriticalSection(m_pMutex); }
private:
    CRITICAL_SECTION *m_pMutex;
#endif

#ifdef linux
public:
    CxdAutoMutexVal(pthread_mutex_t *ApMutex){ pthread_mutex_lock(ApMutex); m_pMutex = ApMutex; }
    ~CxdAutoMutexVal() { pthread_mutex_unlock(m_pMutex); }
private:
    pthread_mutex_t *m_pMutex;
#endif
};

//====================================================================
// 信号量
//====================================================================
//
class CxdSemaphore
{
public:
    CxdSemaphore(unsigned int AValue = 1, const char *ApSemName = NULL, bool AbAutoDestroyName = false, unsigned int AInitValue = 0);
    ~CxdSemaphore();

    //Wait: 一直等待
    inline bool Wait( DWORD AdwMilliseconds = INFINITE ) 
    {
#ifdef linux
        if ( INFINITE == AdwMilliseconds )
        {
            return 0 == sem_wait( m_pSem ); 
        }
        else
        {
            struct timespec ts;  
            clock_gettime(CLOCK_REALTIME, &ts);  
            long secs = AdwMilliseconds / 1000;  
            AdwMilliseconds = AdwMilliseconds % 1000;  

            long add = 0;  
            AdwMilliseconds = AdwMilliseconds * 1000 * 1000 + ts.tv_nsec;
            add = AdwMilliseconds / ( 1000 * 1000 * 1000 );
            ts.tv_sec += (add + secs);
            ts.tv_nsec = AdwMilliseconds % ( 1000 * 1000 * 1000);

            return 0 == sem_timedwait( m_pSem, &ts ); 
        }
#endif
#ifdef WIN32
        return WAIT_OBJECT_0 == WaitForSingleObject( m_hHandle, AdwMilliseconds );
#endif
    }

    //Post: 对信号量加 1 
    inline bool Post() 
    { 
#ifdef linux
        return 0 == sem_post( m_pSem );
#endif
#ifdef WIN32
        LONG v;
        return TRUE == ReleaseSemaphore( m_hHandle, 1, &v );
#endif
     }

    //Linux 下
#ifdef linux
    inline int  GetValue() { int n(0); sem_getvalue( m_pSem, &n ); return n; }
    inline bool WaitTry() { return 0 == sem_trywait( m_pSem ); }
    inline void DestroySemaphoreName(void) { if( !m_strSemName.empty() ) sem_unlink( m_strSemName.c_str() ); }
#endif 
private:
    bool m_bAtoDestroySemaphoreName;
    string m_strSemName;
#ifdef linux
    sem_t* m_pSem;
#endif
#ifdef WIN32
    HANDLE m_hHandle;
#endif
    
};

#ifdef linux
//读写锁
class CxdReadWriteLock
{
public:
    CxdReadWriteLock(bool AMultiProcess = false );
    ~CxdReadWriteLock();

    inline bool LockRead(void) { return 0 == pthread_rwlock_rdlock(m_pLock); }
    inline bool LockWrite(void) { return 0 == pthread_rwlock_wrlock(m_pLock); }
    inline bool TryLockRead(void) { return 0 ==  pthread_rwlock_tryrdlock(m_pLock); }
    inline bool TryLockWrite(void) { return 0 == pthread_rwlock_trywrlock(m_pLock); }
    inline void UnLock(void) { pthread_rwlock_unlock(m_pLock); }
private:
    bool m_bMultiProcess;
    pthread_rwlock_t *m_pLock;
};

#endif

#endif