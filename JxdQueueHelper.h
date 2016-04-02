/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdThreadQueue.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-12-13 10:53:44
	LastModify : 
	Description: 使用线程来处理队列
        一个队列, 使用设置的N个线程(当不需要时, 线程自动退出), 对其进行处理, 默认加入队列的对象是唯一的

        使用:  
            CxdQueueHelper<int> obj;
            obj.SetThreadCount( 2 );
            obj.SetWinThread( true ); //只要MFC下有效
            obj.AddForUnionObject( false ); //默认为 true
            obj.SetItHandleItem( NewInterfaceByClass(CMyClass, int, DoThreadHandle) ); //此设置为必须, 用于处理对象
            obj.Add(100); //开始启动线程处理

            obj.StopAndWait(); //

**************************************************************************/
#pragma once

#ifndef _JxdThreadManage_Lib
#define _JxdThreadManage_Lib

#include "JxdEvent.h"
#include "JxdSynchro.h"
#include "JxdThread.h"
#include "JxdDataStruct.h"

#include <list>
using std::list;

template<typename T>
class CxdQueueHelper
{
public:
    typedef list< T > TxdHandlingList;

    typedef IxdNotifyEventT<T> IxdHandleThreadQueue;
    typedef IxdNotifyEventEx3T<T&, T&, int&> IxdCompareInterface;
    CxdQueueHelper()
    {
        m_ThreadCount = 1;
        m_CurThreadCount = 0;
        m_StopThread = false;
        m_AddForUnionObject = true;
        m_DelaySleepTime = 10;
        m_DelaySleepCount = 100;
        InitInterface( HandleItem );
        InitInterface( CompareInterface );
    }
    ~CxdQueueHelper()
    {
        StopAndWait();
        DeleteInterface( HandleItem );
        DeleteInterface( CompareInterface );        
    }
    //处理队列对象最大线程数
    PropertyValue( ThreadCount, UINT, true );

    //当前运行线程数
    PropertyGetValue( CurThreadCount, UINT );

    //线程是否停止
    PropertyGetValue( StopThread, bool );

    //是否使用WIN线程
#if defined (WIN32) && (_MFC_VER)
    PropertyValue( WinThread, bool, true );
#endif 

    //添加时,是否判断已经存在对象
    PropertyValue( AddForUnionObject, bool, true );

    //线程中被执行的接口
    PropertyInterface( HandleItem, IxdHandleThreadQueue );

    //用于比较对象
    PropertyInterface( CompareInterface, IxdCompareInterface );

    //单位: 毫秒; 当线程获取不到数据处理时,延迟多长时间再退出(延迟为了多做处理)
    //每次Sleep时长, 默认为 10
    PropertyValue( DelaySleepTime, UINT, true ); 
    //最多Sleep次数, 0则表示不启用
    PropertyValue( DelaySleepCount, UINT, true );

    bool Add(T t)
    {
        bool bOK;
        AutoMutex( m_lock );
        if ( m_AddForUnionObject )
        {
            bOK = !IsExsit(t) && m_queue.Put( t );
        }
        else
        {
            bOK = m_queue.Put( t );
        }
        CheckThread();
        CxdQueue<T>::IxdCompareInterface *p = NULL;
        return bOK;
    }

    //是存存在对象
    bool HasObject(T t)
    {
        AutoMutex( m_lock );
        return IsExsit(t);
    }
    
    void Stop(void)
    {
        m_StopThread = true;
    }
    void StopAndWait(void)
    {
        Stop();
        while ( m_CurThreadCount > 0 )
        {
            Sleep( 10 );
        }
    }
protected:
    void DoThreadExec(LPVOID)
    {
        T t;
        bool bSleep = false;
        UINT nCount = 0;
lblDelay: 
        while ( !m_StopThread )
        {
            if ( bSleep ) { Sleep( m_DelaySleepTime );  }

            //获取可用对象
            {
                AutoMutex( m_lock );
                if ( !m_queue.Get(t) )
                {
                    if ( !bSleep )
                    {
                        bSleep = true;
                        nCount = m_DelaySleepCount;
                        if ( nCount > 0 ) goto lblDelay;
                    }
                    else
                    {
                        if ( nCount-- > 0 ) continue;
                    }
                    //退出线程
                    m_CurThreadCount--;
                    return;
                }
                if ( m_AddForUnionObject )
                {
                    m_ltHandlingObject.push_back( t );
                }
            }
            if ( bSleep ) bSleep = false;

            //通知外部, 处于非锁定状态
            NotifyInterface( HandleItem, t );

            //处理完成
            if ( m_AddForUnionObject )
            {
                AutoMutex( m_lock );
                for ( TxdHandlingList::iterator it = m_ltHandlingObject.begin(); 
                    it != m_ltHandlingObject.end(); it++ )
                {
                    T t2 = *it;
                    if ( IsSameObject(t, t2) )
                    {
                        m_ltHandlingObject.erase( it );
                        break;
                    }
                }
            }
        }

        AutoMutex( m_lock );
        m_CurThreadCount--;
    }

    bool IsExsit(T t)
    {
        for ( TxdHandlingList::iterator it = m_ltHandlingObject.begin(); 
                it != m_ltHandlingObject.end(); it++ )
        {
            T t2 = *it;
            if ( IsSameObject(t, t2) )
            {
                return true;
            }
        }
        m_FindValue = t;
        m_bFind = false;
        m_queue.Loop( &InterfaceObjectExByClass(CxdQueueHelper, T&, bool&, DoLoopQueue) );
        return m_bFind;
    }
private:
    CxdMutex m_lock;
    CxdQueue<T> m_queue;
    TxdHandlingList m_ltHandlingObject;


    void CheckThread(void)
    {
        m_StopThread = false;
        UINT nCount = m_ThreadCount - m_CurThreadCount;
        if ( nCount > m_queue.GetCount() )
        {
            nCount = m_queue.GetCount();
            if ( nCount > m_CurThreadCount )
            {
                nCount -= m_CurThreadCount;
            }
            else
            {
                //不需要再启动线程了
                return;
            }
        }
        for ( UINT i = 0; i < nCount; i++ )
        {
            m_CurThreadCount++;

#if defined (WIN32) && (_MFC_VER)
            if ( GetWinThread() )
            {
                CreateSimpleWinThread( CxdQueueHelper, LPVOID, DoThreadExec, NULL );
                continue;
            }
#endif
            CreateSimpleThread( CxdQueueHelper, LPVOID, DoThreadExec, NULL );
        }
    }

    bool IsSameObject(T& t1, T& t2)
    {
        if ( IsExsitsItCompareInterface() )
        {
            int nRet = 1;
            NotifyInterfaceEx3( CompareInterface, t1, t2, nRet );
            if ( nRet == 0 ) return true;
        }
        else if ( t1 == t2 )
        {
            return true;
        }
        return false;
    }

    T m_FindValue;
    bool m_bFind;
    void DoLoopQueue(T& t, bool& AbBreak)
    {
        m_bFind = IsSameObject( t, m_FindValue );
        if ( m_bFind ) AbBreak = true;
    }
};

#endif