/**************************************************************************
Copyright      : pp.cc 深圳指掌时代 珊瑚组
File           : JxdUdpClient.h & .cpp 
Author         : Terry
Email          : jxd524@163.com
CreateTime     : 2012-9-19 13:33
LastModifyTime : 2012-9-19 13:33
Description    : UDP客户端实现,支持同步命令

限制:
接收到的数据前两个字节为命令ID
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

//同步包处理
struct TxdSyncBufferInfo 
{
    WORD FWaitCmdID;
    SOCKET FRecvSocket;
    TxdSocketAddr FRemoteAddr;
    bool FWaitting;
    int  FRecvBufferLength;
    char FRecvBuffer[CtMaxCombiPackageBufferSize]; //保存原始的数据,包含前面两个字节
    inline bool IsSuccessRecv(void) { return FRecvBufferLength > 0; }
};

class CxdUdpClient: public CxdUdpIOHandle
{
public:
    CxdUdpClient();
    ~CxdUdpClient();

    //在线服务器地址
    PropertyValue( OnlineServerIP, DWORD, !GetActive() );
    PropertyValue( OnlineServerPort, WORD, !GetActive() );
    PropertyValue( OnlineServerName, string, !GetActive() );

    PropertyValue( BindLocalPort, WORD, !GetActive() );
    PropertyValue( ClientID, DWORD, !GetActive() );
    PropertyValue( ClientHash, string, !GetActive() );

    PropertyValue( LoginByBlock, bool, !GetActive() ); //是否以阻塞的方式登陆服务器
    PropertyGetValue( IsLogin, bool );

    //服务器返回信息(Local为本地设置)
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

    //客户检查线程
    CxdThreadEvent<LPVOID>* m_pCheckThread;
    virtual void DoThreadCheckClient(LPVOID);
private:
    //同步列表
    CxdHashArray* m_pSyncPackageList;
    CxdMutex m_csLockSync;
    void CheckSyncObject(void);

    //当前绑定在本地的信息
    struct TSocketInfo 
    {
        SOCKET s;
        DWORD  ip;
        WORD   port;
    };
    typedef vector<TSocketInfo>   TSocketList;
    typedef TSocketList::iterator TSocketListIT;
    TSocketList m_ltSockets;

    //登陆在线服务器
    bool m_bLoginingToServer;
    void ActiveLogin(bool bBlock);
    void DoLoginToOnlineServer(LPVOID);

    DWORD m_dwLastOnlineActive;
    int m_nTryKeepLiveTimeCount;
};