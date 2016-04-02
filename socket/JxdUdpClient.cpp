#include "stdafx.h"
#include "JxdUdpClient.h"
#include "JxdDebugInfo.h"
#include "JxdSockSub.h"
#include "JxdEventSelectManage.h"

CxdUdpClient::CxdUdpClient()
{
    InitForClient();
    m_pSyncPackageList = NULL;
    
    m_BindLocalPort = 14235;
    m_LocalIP = 0;
    m_LocalPort = 0;
    m_BindSocket = 0;
    m_OnlineServerIP = 0;
    m_OnlineServerPort = 0;
    m_ClientID = 0;
    m_dwLastOnlineActive = 0;
    m_nTryKeepLiveTimeCount = 0;
    m_IsLogin = false;
    m_bLoginingToServer = false;
    m_LoginByBlock = true;
}

CxdUdpClient::~CxdUdpClient()
{
    SetActive( false );
}

int CxdUdpClient::SendSyncBuffer( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, int ABufLen, 
        TxdSyncBufferInfo &ARecvInfo, UINT AMaxWaitTime )
{
//添加
#define AddToSyncList                                                                   \
    {                                                                                   \
        AutoMutex( m_csLockSync );                                                      \
        if ( !m_pSyncPackageList->Add(ARecvInfo.FWaitCmdID, &ARecvInfo) ) return 2;     \
    }
    
//删除
#define DeleteFromSyncList                                                              \
    {                                                                                   \
        AutoMutex( m_csLockSync );                                                      \
        m_pSyncPackageList->Delete( ARecvInfo.FWaitCmdID );                             \
    }

    CheckSyncObject();
    ARecvInfo.FWaitting = true;
    ARecvInfo.FRecvBufferLength = 0;
    
    AddToSyncList;

    int nResult = SendBuffer( ASocket, ARemoteAddr, ApSendBuffer, ABufLen );
    if ( nResult != ABufLen )
    {
        DeleteFromSyncList;
        return nResult;
    }
    
    int nSleepTime(5);
    int nMod( 200 ); //大概1秒之后重新发送数据
    int nMaxSleepCount = AMaxWaitTime / (nSleepTime + 3);
    int nSleepCount(0);

    if ( nMaxSleepCount < 200 )
    {
        nMaxSleepCount = 200;
    }
    Sleep( 10 );
    while ( ARecvInfo.FWaitting && nMaxSleepCount-- > 0 )
    {
        Sleep( nSleepTime );
        if ( !ARecvInfo.FWaitting ) break;

        if ( 0 == (++nSleepCount % nMod) )
        {
            nResult = SendBuffer( ASocket, ARemoteAddr, ApSendBuffer, ABufLen );
            if ( nResult != ABufLen )
            {
                DeleteFromSyncList;
                return nResult;
            }
        }
    }
    DeleteFromSyncList;    
    return nResult;
}

void CxdUdpClient::OnRecvBuffer( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    if ( ABufLen < sizeof(TCmdHead) )
    {
        DebugA( "接收到无效的数据" );
        return;
    }
    TCmdHead *pCmdHead = (TCmdHead*)ApRecvBuffer;
    if ( pCmdHead->FUserID == CtOnlineServerID )
    {
        m_nTryKeepLiveTimeCount = 0;
        m_dwLastOnlineActive = GetTickCount();
    }
    if ( m_pSyncPackageList != NULL && m_pSyncPackageList->IsHasElement() )
    {
        AutoMutex( m_csLockSync );
        TxdSyncBufferInfo *pInfo;
        if ( m_pSyncPackageList->Find(pCmdHead->FCmdID, (LPVOID&)pInfo, false) )
        {
            pInfo->FRecvSocket = ASocket;
            pInfo->FRemoteAddr = ARemoteAddr;
            pInfo->FRecvBufferLength = ABufLen;
            memcpy( pInfo->FRecvBuffer, ApRecvBuffer, pInfo->FRecvBufferLength );
            pInfo->FWaitting = false;
            return;
        }
    }
    switch ( pCmdHead->FCmdID )
    {
    case CtCmd_Heartbeat:
        DoCmdHeartbeat( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
    	break;
    case CtCmd_ClientReLogin:
        DoCmdReLogin( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
        break;
    default:
        OnRecvCmd( pCmdHead, ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
    }
}

void CxdUdpClient::CheckSyncObject( void )
{
    if ( m_pSyncPackageList == NULL)
    {
        m_pSyncPackageList = new CxdHashArray;
        m_pSyncPackageList->SetHashTableCount( 9 );
        m_pSyncPackageList->SetInitHashNodeMemCount( 128 );
        m_pSyncPackageList->SetActive( true );
    }
}

bool CxdUdpClient::ActiveService( void )
{
    if ( m_OnlineServerPort == 0 ) throw "must set online server port!";
    if ( m_OnlineServerIP == 0 && m_OnlineServerName.empty() ) throw "must set online server IP!";

    m_IsLogin = false;
    m_dwLastOnlineActive = 0;
    m_nTryKeepLiveTimeCount = 0;
    m_bLoginingToServer = true;
    m_ltSockets.clear();

    //当本地网卡地址都绑上.
    ClearBindInfo();
    in_addr IPs[128];
    int nCount;
    if ( GetLocalIPs(IPs, 128, nCount) )
    {
        for( int i = 0; i != nCount; i++ )
        {
        	AddBindInfo( IPs[i].s_addr, m_BindLocalPort );
        }
    }
    if ( !CxdUdpIOHandle::ActiveService() ) return false;

    ActiveLogin( GetLoginByBlock() );
    m_pCheckThread = CreateLoopExecuteThread( CxdUdpClient, LPVOID, DoThreadCheckClient, NULL, 1000 * 19 );
    return true;
}

bool CxdUdpClient::UnActiveService( void )
{
    if ( m_pCheckThread != NULL )
    {
        m_pCheckThread->SetAutoFreeThreadObject( true );
        m_pCheckThread->SetLoopExecuteSpaceTime( 0 );
        m_pCheckThread->PostEvent();
        m_pCheckThread = NULL;
    }
    if ( m_IsLogin )
    {
        TCmdLogout cmd(m_ClientID);
        cmd.FSafeCode = m_SafeCode;
        SendToOnlineServer( (char*)&cmd, sizeof(TCmdLogout) );
        Sleep( 5 );
    }
    SafeDeleteObject( m_pSyncPackageList );
    return CxdUdpIOHandle::UnActiveService();
}


void CxdUdpClient::DoBindSockefFinished( SOCKET ASocket, int AIndex, bool AIsOK )
{
    CxdUdpIOHandle::DoBindSockefFinished( ASocket, AIndex, AIsOK );
    if ( AIsOK )
    {
        TSocketInfo info = {0};
        GetBindInfo( AIndex, info.ip, info.port );
        info.s = ASocket;
        m_ltSockets.push_back( info );
    }
}

int CxdUdpClient::SendToOnlineServer( char *ApSendBuffer, int ABufLen )
{
    return SendBuffer( m_BindSocket, TxdSocketAddr(m_OnlineServerIP, m_OnlineServerPort), ApSendBuffer, ABufLen );
}

void CxdUdpClient::DoCmdHeartbeat( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    if ( !TCmdHeartbeat::IsSameSize(ABufLen) )
    {
        DebugA( "Heartbeat length is error" );
        return;
    }
    TCmdHeartbeat *pCmdRecv = (TCmdHeartbeat*)ApRecvBuffer;
    if ( pCmdRecv->FNeedReply )
    {
        pCmdRecv->FNeedReply = false;
        SendBuffer( ASocket, ARemoteAddr, (char*)pCmdRecv, sizeof(TCmdHeartbeat) );
    }
}

void CxdUdpClient::DoCmdReLogin( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    if ( !TCmdClientReLogin::IsSameSize(ABufLen) )
    {
        DebugA( "ReLogin cmd length is error" );
        return;
    }
    ActiveLogin( false );
}

void CxdUdpClient::ActiveLogin( bool bBlock )
{
    if ( bBlock )
    {
        DoLoginToOnlineServer(NULL);
    }
    else
    {
        CreateSimpleThread( CxdUdpClient, LPVOID, DoLoginToOnlineServer, NULL );
    }
}

void CxdUdpClient::DoLoginToOnlineServer( LPVOID )
{
    //自动处理
    class CxdAuto
    {
    public:
        CxdAuto(CxdUdpClient *pOwner) { pThis = pOwner; pThis->m_bLoginingToServer = true; pThis->m_IsLogin = false; }
        ~CxdAuto(){ pThis->m_bLoginingToServer = false;  pThis->OnLoginOnlineServer( pThis->m_IsLogin ); };
    private:
        CxdUdpClient *pThis;
    };
    //自动化处理
    CxdAuto ao(this);

    //
    DWORD dwTryIPs[256];
    int nTryIPCount = 0;
    if ( m_OnlineServerIP > 0 )
    {
        dwTryIPs[0] = m_OnlineServerIP;
        nTryIPCount++;
    }
    if ( !m_OnlineServerName.empty() )
    {
        in_addr IPs[1024];
        int nCount(0);
        if ( GetIPsByName(m_OnlineServerName.c_str(), IPs, 1024, nCount) )
        {
            for( int i = 0; i != nCount; i++ )
            {
            	dwTryIPs[nTryIPCount++] = IPs[i].s_addr;
            }
        }
    }
    
    //判断有效的本地SOCKET及服务器地址
    TCmdHello hello(0);
    int nSendLen;
    m_BindSocket = 0;
    for( int i = 0; i != nTryIPCount; i++ )
    {
        if ( m_BindSocket != 0 ) break;
    	for ( TSocketListIT it = m_ltSockets.begin(); it != m_ltSockets.end(); it++ )
    	{
            TSocketInfo info = *it;
            TxdSyncBufferInfo recvInfo = {0};
            recvInfo.FWaitCmdID = CtCmd_Hello;
            //in_addr a;
            //a.s_addr = dwTryIPs[i];
            //OutputDebugStringA( inet_ntoa( a ) );
            nSendLen = SendSyncBuffer( info.s, TxdSocketAddr(dwTryIPs[i], m_OnlineServerPort), 
                                      (char*)&hello, sizeof(TCmdHeartbeat), recvInfo );
            if ( recvInfo.IsSuccessRecv() )
            {
                m_BindSocket = info.s;
                m_LocalIP = info.ip;
                m_LocalPort = info.port;
                m_OnlineServerIP = dwTryIPs[i];
                break;
            }
    	}
    }
    if ( m_BindSocket == 0 ) return;

lblRegisterAgain:
    //register cmd
    if ( m_ClientID <= CtClientMinRegID )
    {
        TCmdRegister cmd(0);
        memcpy( cmd.FClientHash, m_ClientHash.c_str(), CtHashLength );

        TxdSyncBufferInfo recvInfo = {0};
        recvInfo.FWaitCmdID = CtCmdReply_Register;
        SendSyncBuffer( m_BindSocket, TxdSocketAddr(m_OnlineServerIP, m_OnlineServerPort),
                        (char*)&cmd, sizeof(TCmdRegister), recvInfo );

        if ( recvInfo.IsSuccessRecv() && TCmdReplyRegister::IsSameSize(recvInfo.FRecvBufferLength) )
        {
            TCmdReplyRegister *pRecvCmd = (TCmdReplyRegister*)recvInfo.FRecvBuffer;
            m_ClientID = pRecvCmd->FRegID;
        }
    }
    if ( m_ClientID <= CtClientMinRegID ) return;

    //Login cmd
    TCmdLogin cmd( m_ClientID );
    cmd.FClientVersion = CtClientCmdVersion;
    cmd.FLocalIP = m_LocalIP;
    cmd.FLocalPort = m_LocalPort;
    memcpy( cmd.FClientHash, m_ClientHash.c_str(), CtHashLength );

    TxdSyncBufferInfo recvInfo = {0};
    recvInfo.FWaitCmdID = CtCmdReply_Login;
    SendSyncBuffer( m_BindSocket, TxdSocketAddr(m_OnlineServerIP, m_OnlineServerPort),
                    (char*)&cmd, sizeof(TCmdLogin), recvInfo );

    if ( TCmdReplyLogin::IsSameSize(recvInfo.FRecvBufferLength) )
    {
        TCmdReplyLogin *pRecvCmd = (TCmdReplyLogin*)recvInfo.FRecvBuffer;
        if ( 0 == pRecvCmd->FRelaySign )
        {
            m_SafeCode = pRecvCmd->FSafeCode;
            m_PublicIP = pRecvCmd->FPublicIP;
            m_PublicPort = pRecvCmd->FPublicPort;
            m_IsLogin = true;
        }
        else if ( 1 == pRecvCmd->FRelaySign )
        {
            //ClientID 无效, 重新注册
            m_ClientID = 0;
            goto lblRegisterAgain;
        }
    }
}

void CxdUdpClient::DoThreadCheckClient( LPVOID )
{
    //每20秒调用一次
    if ( !m_IsLogin ) 
    {
        if ( m_bLoginingToServer ) return;
        Sleep( 1000 * 60 * 2 );
        DoLoginToOnlineServer( NULL );
    }
    else
    {
        if ( m_nTryKeepLiveTimeCount >= 4 )
        {
            OnUnConnetOnlineServerByTimeout();
            m_IsLogin = false;
        }
        else
        {
            m_nTryKeepLiveTimeCount++;
            TCmdHeartbeat cmd(m_ClientID);
            cmd.FNeedReply = true;
            SendToOnlineServer( (char*)&cmd, sizeof(TCmdHeartbeat) );
        }
        
    }
}
