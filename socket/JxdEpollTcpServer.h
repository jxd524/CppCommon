/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdTcpServer.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-9-1 15:11:58
	LastModify : 
	Description: 
        CxdTcpBasicServer: TCP����������, ʹ��EPOLLģʽ, �ṩRECV, SEND���¼����մ���
            �Զ���̷�ʽ���ô���, ����ʹ�� EL ģʽ,�Ա�����SOCKET���ƽ��,����ʹ���Լ�
            �ķ�ʽ����ƽ��.

            �������ṩ�����麯��,
                DoAcceptNewSocket
                DoRecvBuffer
                DoCanSendWaitBuffer
                DoSocketClose

            ����������:
                ListenSocketMode: �����׽���ʹ�õ�ģʽ
                ThreadCount: �������ٸ��߳̽��д���
                MaxWaitEpollTimeout >0 �����Զ���ʱ, Ϊ0 �����޴���ʱ�����߳�

        CxdTcpServerIOHandler: TCP���������,֧�ִ����ݷ��ͻ���,�Է��ͼ����ս���ͳ��
            ʹ���ڴ淽ʽΪ: ÿ��SOCKET��Ҫһ��TxdSockIOBuf�ṹ��������ά��
            һ��TxdSockIOBuf�����һ��LOCK��һ��TxdTcpBuffer��ָ��.�����ҹ����ڴ滺���л�ȡ�ڴ���ж�д
            ���ڴ��С��ϵ���еĺ���: CalcMaxBufferSize
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

//�Ѿ����ӵ�Socket �����ṹ, ��Ҫֻ����Socket�׽���, �����������̳�
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

    /*��������*/
    //���ͽӿ�
    virtual int SendBuffer(int ASocket,  const char* ApBuffer, int ALen);
    //�����ر��׽���
    void CloseSocket(int ASocket);

    //���߹��������
    UINT GetOnlineCount(void) { return m_OnlineManage.GetCount(); }
    UINT GetOnlineHashNodeInitCount(void) { return m_OnlineManage.GetInitHashNodeMemCount(); }
    virtual void SetOnlineHashNodeInitCount(UINT ACount) { m_OnlineManage.SetInitHashNodeMemCount(ACount); }
    UINT GetOnlineHashTableCount(void) { return m_OnlineManage.GetHashTableCount(); }
    bool SetOnlineHashTableCount(UINT AHashTableCount);

    //listen socket�������
    PropertyValue( ListenETMode, bool, !GetActive() );
    PropertyValue( MaxWaitListenEpollTimeout, int, true );
    PropertyValue( ListenThreadCount, int, !GetActive() ); //Ҫ���������߳�����
    PropertyGetValue( CurListenThreadCount, int ); //��ǰ���еļ����߳�����

    //����ͻ����������
    PropertyValue( ClientThreadCount, int, !GetActive() );
    PropertyValue( MaxWaitClientEpollTimeout, int, true );
    PropertyGetValue( CurClientThreadCount, int );    
protected:
    /*ʵ�ָ�����ز���*/
    virtual bool ActiveService();
    virtual bool UnActiveService();
    virtual int  CreateSocket(int AIndex);
    virtual void DoBindSockefFinished(int ASocket, int AIndex, bool AIsOK);

    /*���������ʵ��*/
    virtual void FreeSocketBuffer(TxdSockBufBasic *ApBuf);
    /*���ᱻ�̴߳�����, �ɴ��෢��*/
    //�������ӵ���, ���� TRUE: ���մ�����, ����ǿ���������ر�����
    virtual bool DoAcceptNewSocket(int ANewSocket, const sockaddr_in &AAddr, TxdSockBufBasic *&ApGetSockBuff);
    //�����յ�����ʱ
    virtual void DoRecvBuffer(TxdSockBufBasic *ApSockObj, char* ApBuf, int ALen){}
    //�����Է�������ʱ
    virtual void DoCanSendWaitBuffer(TxdSockBufBasic *ApSockObj){}
    //���ͻ��˹ر�ʱ����: ͬһ�׽���,�п��ܱ����ö��,����ϵͳ�Ľ���,�̵߳����㷨�й�ϵ
    virtual void DoSocketClose(TxdSockBufBasic *ApSockObj, TSocketCloseState ACloseState){}

    //���ü����߳�����: Ĭ���ԱȽϸߵļ�������
    virtual void DoSettingListenThreadAttr(CxdThreadAttr *ApAttr){}
    //���ô���ͻ��߳�����: Ĭ�����еȵļ�������
    virtual void DoSettingClientThreadAttr(CxdThreadAttr *ApAttr){}
protected:
    //���߹����
    CxdHashArray m_OnlineManage; //��SOCKETֵ, ���� TxdSockBufBasic ָ��
    CxdMutex m_OnlineLock;

    TxdSockBufBasic* FindSockObject(int ASocket); 
    void             ReleaseSockObject(TxdSockBufBasic *ApObj);
private:
    //�ܿ���
    bool m_bWaitForUnActive;

    //�����׽���
    int m_hListenEpoll; //ֻ��������׽���
    int m_nSockCount;
    bool m_IsCurListenByETMode;
    int  m_hListenSocks[CtMaxBindSocketCount];
    CxdMutex m_LockSockMode;
    void CreateListenThread(void);
    void CheckListenMode(bool AForET);
    void DoHandleListenSocketThread(int AThreadIndex);
    void DoHandleListenSocketThreadFinished(int AThreadIndex);
    void HandleAccept(int ASocket);

    //�ͻ��˴���
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
    //���ͻ�������
    UINT GetWaitSendDefaultBlockCount() { return m_mmWaitSend.GetDefaultBlockCount(); }
    bool SetWaitSendDefaultBlockCount(UINT ADefaultValue) { return m_mmWaitSend.SetDefaultBlockCount(ADefaultValue); }

    PropertyGetValue( SendBufferLength, UINT64 );
    PropertyGetValue( RecvBufferLength, UINT64 );
protected:
    int SendBuffer(TxdSockIOBuf *ApSockObj, const char* ApBuffer, int ALen);
    //�µ����⺯��
    virtual void InitDefaultInfo();
    virtual void DoSendFinsiehd(TxdSockIOBuf *ApSockObj, const char* ApBuffer, int ALen);

    //�����麯��ʵ��
    virtual bool ActiveService(); 
    virtual bool DoAcceptNewSocket(int ANewSocket, const sockaddr_in &AAddr, TxdSockBufBasic *&ApGetSockBuff);
    virtual void DoRecvBuffer(TxdSockBufBasic *ApSockObj, char* ApBuf, int ALen);
    virtual void DoCanSendWaitBuffer(TxdSockBufBasic *ApSockObj);
    virtual void DoSocketClose(TxdSockBufBasic *ApSockObj, TSocketCloseState ACloseState);
    virtual void FreeSocketBuffer(TxdSockBufBasic *ApBuf);
protected:
    //���ͻ���
    CxdMutex m_mxWaitSendLock;
    CxdFixedMemoryManage m_mmWaitSend;
    TxdBlockBuffer* GetTcpBufferForWaitSend(int ALen);
    void            AddToTcpBuffer(TxdBlockBuffer *ApBuf, const char* ApAddBuff, int ALen);
    int             FreeTcpBuffer(TxdBlockBuffer *&ApBuf, bool AFreeAll, bool ALock);
};

#endif