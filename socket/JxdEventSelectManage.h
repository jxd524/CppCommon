/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdEventSelectIO.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-11-7 11:05:29
	LastModify : 
	Description: ��IOģʽֻ֧��Windowϵͳ
        CxdEventGroup: ʹ�� WSAEventSelect ģʽ��SOCKET���д���, 
            ÿ��Groupʹ��һ���߳�
            �� IxdSocketEventHandle �ӿ�, ֻ��������,����������ͷŲ���

        CxdEventSelectIO ʵ�� ISocketEvent �ӿ�, �������󴴽��㹻�� CxdEventGroup ����
**************************************************************************/
#pragma once

#ifndef _JxdEventSelectIO_Lib
#define _JxdEventSelectIO_Lib

#include <vector>
using std::vector;

#include "JxdSockDefines.h"
#include "JxdSynchro.h"
#include "JxdDataStruct.h"
#include "JxdThread.h"

class CxdEventGroup: protected CxdThread
{
public:
    CxdEventGroup();
    ~CxdEventGroup();

    bool AddNetworkEvent( TxdEventStyle AStyle, IxdSocketEventHandle *ApHandleInterface ); 
    bool IsExsitsNetworkEvent( TxdEventStyle AStyle );

    //��Socket���¼�������һ��
    bool Add(SOCKET ASocket);
    bool Delete(SOCKET ASocket);

    void WaitFreeObject();

    PropertyGetValue( RunningThreadCount, long );
    PropertyGetValue( SocketCount, int );
    PropertyGetValue( NetworkEvents, int );
    PropertyGetValue( ThreadStopTime, INT64 );
private:
    struct TxdNetworkEvent
    {
        int Event;
        int EventBit;
        IxdSocketEventHandle* SocketHandle;
    };
    typedef vector< TxdNetworkEvent* > TxdNetworkEventList;
    typedef TxdNetworkEventList::iterator TxdNetworkEventListIT;
    TxdNetworkEventList m_ltNetworkEvent;
private:
    CxdMutex m_mutex;
    bool m_bWaitCloseThread;

    void PostNotify(void);
    //��0������֪ͨ
    WSAEVENT m_hEvents[WSA_MAXIMUM_WAIT_EVENTS];
    SOCKET   m_hSockets[WSA_MAXIMUM_WAIT_EVENTS];

    //�ȴ���ӵ�SOCKET
    CxdQueueEx<SOCKET> m_ltWaitAdd;
    void AddWaitSocket(void);

    //�ȴ�ɾ����SOCKET
    CxdQueueEx<SOCKET> m_ltWaitDelete; 
    void DeleteWaitSocket(void);
    
    void OnExecute();
};

class CxdEventSelectManage: public IxdSocketEventManage
{
public:
    CxdEventSelectManage();
    ~CxdEventSelectManage();

    //�ӿ�ʵ��
    bool AddSocketEvent( TxdEventStyle AEventStyle, IxdSocketEventHandle *ApHandleInterface );

    bool RelateSocket( SOCKET ASocket );
    bool UnRelateSocket( SOCKET ASocket );

    void Clear(void);
private:
    CxdMutex m_mutext;
    struct TxdStyleInfo
    {
        TxdEventStyle EventStyle;
        IxdSocketEventHandle *EventHandle;
    };
    typedef vector<TxdStyleInfo*>    TxdStyleList;
    typedef TxdStyleList::iterator  TxdStyleListIT;
    TxdStyleList m_ltStyle;
private:
    typedef vector<CxdEventGroup*> TxdGroupList;
    typedef TxdGroupList::iterator TxdGroupListIT;
    TxdGroupList m_ltGroup;
};

#endif