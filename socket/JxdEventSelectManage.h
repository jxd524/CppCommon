/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdEventSelectIO.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-11-7 11:05:29
	LastModify : 
	Description: 此IO模式只支持Window系统
        CxdEventGroup: 使用 WSAEventSelect 模式对SOCKET进行处理, 
            每个Group使用一个线程
            对 IxdSocketEventHandle 接口, 只进行引用,不对其进行释放操作

        CxdEventSelectIO 实现 ISocketEvent 接口, 根据请求创建足够的 CxdEventGroup 对象
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

    //将Socket与事件关联在一起
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
    //第0个用于通知
    WSAEVENT m_hEvents[WSA_MAXIMUM_WAIT_EVENTS];
    SOCKET   m_hSockets[WSA_MAXIMUM_WAIT_EVENTS];

    //等待添加的SOCKET
    CxdQueueEx<SOCKET> m_ltWaitAdd;
    void AddWaitSocket(void);

    //等待删除的SOCKET
    CxdQueueEx<SOCKET> m_ltWaitDelete; 
    void DeleteWaitSocket(void);
    
    void OnExecute();
};

class CxdEventSelectManage: public IxdSocketEventManage
{
public:
    CxdEventSelectManage();
    ~CxdEventSelectManage();

    //接口实现
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