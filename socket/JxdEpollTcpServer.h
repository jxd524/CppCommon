/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdTcpServer.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-9-1 15:11:58
	LastModify : 
	Description: 
        CxdTcpBasicServer: TCP基本服务器, 使用EPOLL模式, 提供RECV, SEND的事件接收处理
            以多进程方式调用此类, 可以使用 EL 模式,以便连接SOCKET相对平衡,或者使用自己
            的方式进行平衡.

            对子类提供以下虚函数,
                DoAcceptNewSocket
                DoRecvBuffer
                DoCanSendWaitBuffer
                DoSocketClose

            可设置属性:
                ListenSocketMode: 监听套接字使用的模式
                ThreadCount: 启动多少个线程进行处理
                MaxWaitEpollTimeout >0 将可自动超时, 为0 将在无处理时阻塞线程

        CxdTcpServerIOHandler: TCP缓存服务器,支持大数据发送缓存,对发送及接收进行统计
            使用内存方式为: 每个SOCKET需要一个TxdSockIOBuf结构进行数据维护
            一个TxdSockIOBuf里包含一个LOCK和一个TxdTcpBuffer的指针.从自我管理内存缓存中获取内存进行读写
            与内存大小关系密切的函数: CalcMaxBufferSize
**************************************************************************/
#pragma once
#ifndef _JxdTcpServer_Lib
#define _JxdTcpServer_Lib

#include <string.h>

#include "JxdMacro.h"
#include "JxdSockBasic.h"
#include "JxdSynchro.h"
#include "JxdDataStruct.h"
#include "JxdMem.h"
#include "JxdThread.h"
#include "JxdBlockBuffer.h"

#define CtMaxBindSocketCount 16
#define CtMaxWaitEpollEvent 128
#define CtRecvBufferSize 1800

//#pragma pack(1)

//已经连接的Socket 基本结构, 主要只保存Socket套接字, 设计用于子类继承
struct TxdSockBufBasic
{
public:
    inline int  GetSocket(void) { return m_hSocket; }
    inline void AddEvent(int AEvent) { m_nEvents |= AEvent; }
    inline void DeleteEvent(int AEvent) { m_nEvents &= m_nEvents - AEvent;}
protected:
    TSocketCloseState CloseState;
private:
    int m_hSocket;
    int m_nCount;
    uint32_t m_nEvents;
    friend class CxdTcpBasicServer;
};
//#pragma pack()

//CxdTcpBasicServer
class CxdTcpBasicServer: public CxdSocketBasic
{
public:
    CxdTcpBasicServer();
    virtual ~CxdTcpBasicServer();
    void OutputTcpBasicInfo();

    TxdSockBufBasic *pLast;
    void MyTest();
    void MyTestClose();

    /*主动方法*/
    //发送接口
    virtual int SendBuffer(int ASocket,  const char* ApBuffer, int ALen);
    //主动关闭套接字
    void CloseSocket(int ASocket);

    //在线管理类相关
    UINT GetOnlineCount(void) { return m_OnlineManage.GetCount(); }
    UINT GetOnlineHashNodeInitCount(void) { return m_OnlineManage.GetInitHashNodeMemCount(); }
    virtual void SetOnlineHashNodeInitCount(UINT ACount) { m_OnlineManage.SetInitHashNodeMemCount(ACount); }
    UINT GetOnlineHashTableCount(void) { return m_OnlineManage.GetHashTableCount(); }
    bool SetOnlineHashTableCount(UINT AHashTableCount);

    //listen socket相关属性
    PropertyValue( ListenETMode, bool, !GetActive() );
    PropertyValue( MaxWaitListenEpollTimeout, int, true );
    PropertyValue( ListenThreadCount, int, !GetActive() ); //要创建监听线程数量
    PropertyGetValue( CurListenThreadCount, int ); //当前运行的监听线程数量

    //处理客户端相关属性
    PropertyValue( ClientThreadCount, int, !GetActive() );
    PropertyValue( MaxWaitClientEpollTimeout, int, true );
    PropertyGetValue( CurClientThreadCount, int );    
protected:
    /*实现父类相关操作*/
    virtual bool ActiveService();
    virtual bool UnActiveService();
    virtual int  CreateSocket(int AIndex);
    virtual void DoBindSockefFinished(int ASocket, int AIndex, bool AIsOK);

    /*由子类具体实现*/
    virtual void FreeSocketBuffer(TxdSockBufBasic *ApBuf);
    /*将会被线程触发的, 由此类发出*/
    //当新连接到来, 返回 TRUE: 接收此连接, 否则强制性主动关闭连接
    virtual bool DoAcceptNewSocket(int ANewSocket, const sockaddr_in &AAddr, TxdSockBufBasic *&ApGetSockBuff);
    //当接收到数据时
    virtual void DoRecvBuffer(TxdSockBufBasic *ApSockObj, char* ApBuf, int ALen){}
    //当可以发送数据时
    virtual void DoCanSendWaitBuffer(TxdSockBufBasic *ApSockObj){}
    //当客户端关闭时触发: 同一套接字,有可能被调用多次,这与系统的进程,线程调度算法有关系
    virtual void DoSocketClose(TxdSockBufBasic *ApSockObj, TSocketCloseState ACloseState){}

    //设置监听线程属性: 默认以比较高的级别运行
    virtual void DoSettingListenThreadAttr(CxdThreadAttr *ApAttr){}
    //设置处理客户线程属性: 默认以中等的级别运行
    virtual void DoSettingClientThreadAttr(CxdThreadAttr *ApAttr){}
protected:
    //在线管理表
    CxdHashArray m_OnlineManage; //以SOCKET值, 保存 TxdSockBufBasic 指针
    CxdMutex m_OnlineLock;

    TxdSockBufBasic* FindSockObject(int ASocket); 
    void             ReleaseSockObject(TxdSockBufBasic *ApObj);
private:
    //总控制
    bool m_bWaitForUnActive;

    //监听套接字
    int m_hListenEpoll; //只负责监听套接字
    int m_nSockCount;
    bool m_IsCurListenByETMode;
    int  m_hListenSocks[CtMaxBindSocketCount];
    CxdMutex m_LockSockMode;
    void CreateListenThread(void);
    void CheckListenMode(bool AForET);
    void DoHandleListenSocketThread(int AThreadIndex);
    void DoHandleListenSocketThreadFinished(int AThreadIndex);
    void HandleAccept(int ASocket);

    //客户端处理
    int m_hClientEpoll;
    void CreateClientThread(void);
    void DoHandleClientThread(int AThreadIndex);
    void DoHandleClientThreadFinished(int AThreadIndex);
    void HandleRecv(TxdSockBufBasic *ApSockObj);
};

//TxdSockIOBuf
class CxdTcpIOBufferServer;
struct TxdSockIOBuf: public TxdSockBufBasic
{
private:
    pthread_mutex_t m_SendBufferLock;
    TxdBlockBuffer *m_pWaitSendBuffer;
    friend class CxdTcpIOBufferServer;
};

//CxdTcpServerIOHandler
class CxdTcpIOBufferServer: public CxdTcpBasicServer
{
public:
    CxdTcpIOBufferServer();
    void OutputTcpIOBufferServerInfo();
    virtual int  SendBuffer(int ASocket, const char* ApBuffer, int ALen);
    virtual void SetOnlineHashNodeInitCount(UINT ACount);
    //发送缓存设置
    UINT GetWaitSendDefaultBlockCount() { return m_mmWaitSend.GetDefaultBlockCount(); }
    bool SetWaitSendDefaultBlockCount(UINT ADefaultValue) { return m_mmWaitSend.SetDefaultBlockCount(ADefaultValue); }

    PropertyGetValue( SendBufferLength, UINT64 );
    PropertyGetValue( RecvBufferLength, UINT64 );
protected:
    int SendBuffer(TxdSockIOBuf *ApSockObj, const char* ApBuffer, int ALen);
    //新的虚拟函数
    virtual void InitDefaultInfo();
    virtual void DoSendFinsiehd(TxdSockIOBuf *ApSockObj, const char* ApBuffer, int ALen);

    //父类虚函数实现
    virtual bool ActiveService(); 
    virtual bool DoAcceptNewSocket(int ANewSocket, const sockaddr_in &AAddr, TxdSockBufBasic *&ApGetSockBuff);
    virtual void DoRecvBuffer(TxdSockBufBasic *ApSockObj, char* ApBuf, int ALen);
    virtual void DoCanSendWaitBuffer(TxdSockBufBasic *ApSockObj);
    virtual void DoSocketClose(TxdSockBufBasic *ApSockObj, TSocketCloseState ACloseState);
    virtual void FreeSocketBuffer(TxdSockBufBasic *ApBuf);
protected:
    //发送缓存
    CxdMutex m_mxWaitSendLock;
    CxdFixedMemoryManage m_mmWaitSend;
    TxdBlockBuffer* GetTcpBufferForWaitSend(int ALen);
    void            AddToTcpBuffer(TxdBlockBuffer *ApBuf, const char* ApAddBuff, int ALen);
    int             FreeTcpBuffer(TxdBlockBuffer *&ApBuf, bool AFreeAll, bool ALock);
};

#endif