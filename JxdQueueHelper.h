/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdThreadQueue.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-12-13 10:53:44
	LastModify : 
	Description: ʹ���߳����������
        һ������, ʹ�����õ�N���߳�(������Ҫʱ, �߳��Զ��˳�), ������д���, Ĭ�ϼ�����еĶ�����Ψһ��

        ʹ��:  
            CxdQueueHelper<int> obj;
            obj.SetThreadCount( 2 );
            obj.SetWinThread( true ); //ֻҪMFC����Ч
            obj.AddForUnionObject( false ); //Ĭ��Ϊ true
            obj.SetItHandleItem( NewInterfaceByClass(CMyClass, int, DoThreadHandle) ); //������Ϊ����, ���ڴ������
            obj.Add(100); //��ʼ�����̴߳���

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
    //������ж�������߳���
    PropertyValue( ThreadCount, UINT, true );

    //��ǰ�����߳���
    PropertyGetValue( CurThreadCount, UINT );

    //�߳��Ƿ�ֹͣ
    PropertyGetValue( StopThread, bool );

    //�Ƿ�ʹ��WIN�߳�
#if defined (WIN32) && (_MFC_VER)
    PropertyValue( WinThread, bool, true );
#endif 

    //���ʱ,�Ƿ��ж��Ѿ����ڶ���
    PropertyValue( AddForUnionObject, bool, true );

    //�߳��б�ִ�еĽӿ�
    PropertyInterface( HandleItem, IxdHandleThreadQueue );

    //���ڱȽ϶���
    PropertyInterface( CompareInterface, IxdCompareInterface );

    //��λ: ����; ���̻߳�ȡ�������ݴ���ʱ,�ӳٶ೤ʱ�����˳�(�ӳ�Ϊ�˶�������)
    //ÿ��Sleepʱ��, Ĭ��Ϊ 10
    PropertyValue( DelaySleepTime, UINT, true ); 
    //���Sleep����, 0���ʾ������
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

    //�Ǵ���ڶ���
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

            //��ȡ���ö���
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
                    //�˳��߳�
                    m_CurThreadCount--;
                    return;
                }
                if ( m_AddForUnionObject )
                {
                    m_ltHandlingObject.push_back( t );
                }
            }
            if ( bSleep ) bSleep = false;

            //֪ͨ�ⲿ, ���ڷ�����״̬
            NotifyInterface( HandleItem, t );

            //�������
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
                //����Ҫ�������߳���
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