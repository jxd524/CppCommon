/**************************************************************************
Copyright      : pp.cc ����ָ��ʱ�� ɺ����
File           : JxdUdpClient.h & .cpp 
Author         : Terry
Email          : jxd524@163.com
CreateTime     : 2012-9-19 13:33
LastModifyTime : 2012-9-19 13:33
Description    : UDP�ͻ���ʵ��,֧��ͬ������

����:
���յ�������ǰ�����ֽ�Ϊ����ID
**************************************************************************/
#pragma once;
#include "JxdUdpIOHandle.h"
#include "JxdUdpCmdDefines.h"
#include "JxdDataStruct.h"
#include "JxdMacro.h"
#include "JxdSynchro.h"
#include "JxdThread.h"

const WORD CtClientCmdVersion = 1;

#include <vector>
using std::vector;

//ͬ��������
struct TxdSyncBufferInfo 
{
    WORD FWaitCmdID;
    SOCKET FRecvSocket;
    TxdSocketAddr FRemoteAddr;
    bool FWaitting;
    int  FRecvBufferLength;
    char FRecvBuffer[CtMaxCombiPackageBufferSize]; //����ԭʼ������,����ǰ�������ֽ�
    inline bool IsSuccessRecv(void) { return FRecvBufferLength > 0; }
};

class CxdUdpClient: public CxdUdpIOHandle
{
public:
    CxdUdpClient();
    ~CxdUdpClient();

    //���߷�������ַ
    PropertyValue( OnlineServerIP, DWORD, !GetActive() );
    PropertyValue( OnlineServerPort, WORD, !GetActive() );
    PropertyValue( OnlineServerName, string, !GetActive() );

    PropertyValue( BindLocalPort, WORD, !GetActive() );
    PropertyValue( ClientID, DWORD, !GetActive() );
    PropertyValue( ClientHash, string, !GetActive() );

    PropertyValue( LoginByBlock, bool, !GetActive() ); //�Ƿ��������ķ�ʽ��½������
    PropertyGetValue( IsLogin, bool );

    //������������Ϣ(LocalΪ��������)
    PropertyGetValue( LocalIP, DWORD );
    PropertyGetValue( LocalPort, WORD );
    PropertyGetValue( PublicIP, DWORD );
    PropertyGetValue( PublicPort, DWORD );
    PropertyGetValue( SafeCode, DWORD );

    int SendToOnlineServer(char *ApSendBuffer, int ABufLen);
    int SendSyncBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, int ABufLen, 
                       TxdSyncBufferInfo &ARecvInfo, UINT AMaxWaitTime = 3000);
protected:
    virtual bool ActiveService();
    virtual bool UnActiveService();
    virtual void DoBindSockefFinished(SOCKET ASocket, int AIndex, bool AIsOK);

    void OnRecvBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    //void OnCheckByThread(void);
    //
    void DoCmdHeartbeat(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoCmdReLogin(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    //
    virtual void OnLoginOnlineServer(bool AbSuccess){}
    virtual void OnRecvCmd(TCmdHead *ApCmdHead, SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen) {}
    virtual void OnUnConnetOnlineServerByTimeout(void){}
    //var
    PropertyGetValue( BindSocket, SOCKET );

    //�ͻ�����߳�
    CxdThreadEvent<LPVOID>* m_pCheckThread;
    virtual void DoThreadCheckClient(LPVOID);
private:
    //ͬ���б�
    CxdHashArray* m_pSyncPackageList;
    CxdMutex m_csLockSync;
    void CheckSyncObject(void);

    //��ǰ���ڱ��ص���Ϣ
    struct TSocketInfo 
    {
        SOCKET s;
        DWORD  ip;
        WORD   port;
    };
    typedef vector<TSocketInfo>   TSocketList;
    typedef TSocketList::iterator TSocketListIT;
    TSocketList m_ltSockets;

    //��½���߷�����
    bool m_bLoginingToServer;
    void ActiveLogin(bool bBlock);
    void DoLoginToOnlineServer(LPVOID);

    DWORD m_dwLastOnlineActive;
    int m_nTryKeepLiveTimeCount;
};