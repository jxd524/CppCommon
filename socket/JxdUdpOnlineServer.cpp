#include "stdafx.h"
#include "JxdUdpOnlineServer.h"
#include "JxdDataStruct.h"
#include "JxdThread.h"
#include "JxdUdpCmdDefines.h"
#include "JxdTime.h"
#include "JxdDebugInfo.h"

CxdUdpOnlineServer::CxdUdpOnlineServer(): CxdUdpIOHandle()
{
    InitForServer();
    m_OnlineHashTableCount = 99999;
    m_OnlineInitNodeCount = 20000;
    m_UserUnActiveTimeout = 60;  //s
    m_pOnlineManage = NULL;
    m_pOnlineMem = NULL;
    InitStateCmdValue();
}

CxdUdpOnlineServer::~CxdUdpOnlineServer()
{
    SetActive( false );
}

bool CxdUdpOnlineServer::ActiveService( void )
{
    if ( !CxdUdpIOHandle::ActiveService() ) return false;

    InitStateCmdValue();
    m_dwCurRegistID = CtClientMinRegID;

    m_pOnlineManage = new CxdHashArray;
    m_pOnlineManage->SetHashTableCount( m_OnlineHashTableCount );
    m_pOnlineManage->SetInitHashNodeMemCount( m_OnlineInitNodeCount );
    m_pOnlineManage->SetActive( true );

    m_pOnlineMem = new CxdFixedMemoryManage;
    m_pOnlineMem->AddFixedAllocInfo( CtOnlineUserInfoSize, m_OnlineInitNodeCount );
    m_pOnlineMem->InitMem( );

    m_pThreadCheck = CreateLoopExecuteThread( CxdUdpOnlineServer, LPVOID, DoThreadCheckOnlineUser, NULL, 
        (m_UserUnActiveTimeout + 1) * 1000 );

    return true;
}

bool CxdUdpOnlineServer::UnActiveService( void )
{
    FreeCheckThread();
    CxdUdpIOHandle::UnActiveService();
    SafeDeleteObject( m_pOnlineMem );
    SafeDeleteObject( m_pOnlineManage );
    return true;
}

DWORD CxdUdpOnlineServer::GetRegistID( void )
{
    AutoMutex( m_mtRegist );
    m_dwCurRegistID = ( m_dwCurRegistID + 1 ) % 0xFFFFFFFF;
    if ( m_dwCurRegistID <= CtClientMinRegID )
    {
        m_dwCurRegistID = CtClientMinRegID + 1;
    }
    LPVOID vPointer;
    while ( m_pOnlineManage->Find(m_dwCurRegistID, vPointer) )
    {
        m_dwCurRegistID = ( m_dwCurRegistID + 1 ) % 0xFFFFFFFF;
        if ( m_dwCurRegistID <= CtClientMinRegID )
        {
            m_dwCurRegistID = CtClientMinRegID + 1;
        }
    }
    return m_dwCurRegistID;
}

void CxdUdpOnlineServer::OnRecvBuffer( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    WORD nCmdID = *(WORD*)ApRecvBuffer;
    switch ( nCmdID )
    {
    case CtCmd_Register:
        InterlockedAdd( m_RegisterCmd );
        DoRegisterCmd( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
    	break;
    case CtCmd_Login:
        InterlockedAdd( m_ClientLoginCmd );
        DoClientLoginCmd( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
        break;
    case CtCmd_Logout:
        InterlockedAdd( m_ClientLogoutCmd );
        DoClientLogoutCmd( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
        break;
    case CtCmd_Heartbeat:
        InterlockedAdd( m_HeartbeatCmd );
        DoHeartbeatCmd( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
        break;
    case CtCmd_Hello:
        InterlockedAdd( m_HelloCmd );
        DoHelloCmd( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
        break;
    default:
        {
            if ( !DoHandleClientCmd(nCmdID, ASocket, ARemoteAddr, ApRecvBuffer, ABufLen) )
            {
                DebugA( "(%s:%d) 无效的命令(%d)", inet_ntoa(ARemoteAddr.sin_addr), ntohs(ARemoteAddr.sin_port), (int)nCmdID );
            }
        }
        break;
    }
}

void CxdUdpOnlineServer::DoRegisterCmd( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    if ( !TCmdRegister::IsSameSize(ABufLen) )
    {
        DebugAO( "Register Cmd length is error" );
        return;
    }
    TCmdRegister *pCmdRecv = (TCmdRegister*)ApRecvBuffer;
    if ( !IsCanRegister(ASocket, ARemoteAddr, pCmdRecv) )
    {
        DebugAO( "IsCanRegister reture false" );
        return;
    }
    TCmdReplyRegister cmd( CtOnlineServerID );
    cmd.FReplySign = 0;
    cmd.FRegID = GetRegistID();
    if ( cmd.FRegID <= CtClientMinRegID )
    {
        cmd.FReplySign = 1;
    }
    SendBuffer( ASocket, ARemoteAddr, (char*)&cmd, sizeof(TCmdReplyRegister) );
}

void CxdUdpOnlineServer::DoClientLoginCmd( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    if ( !TCmdLogin::IsSameSize(ABufLen) )
    {
        DebugAO( "Client login Cmd length is error" );
        return;
    }
    TCmdLogin *pCmdRecv = (TCmdLogin*)ApRecvBuffer;

    TxdOnlineUserInfo *pUserInfo = NULL;
    AutoMutex( m_mtOnline );
    if ( !m_pOnlineManage->Find(pCmdRecv->FUserID, (LPVOID&)pUserInfo) )
    {
        //不存在对象
        if ( !IsCanLogin(ASocket, ARemoteAddr, pCmdRecv) )
        {
            DebugAO( "拒绝登录Client login" );
            TCmdReplyLogin cmd( CtOnlineServerID );
            cmd.FRelaySign = 2; //拒绝登录
            SendBuffer( ASocket, ARemoteAddr, (char*)&cmd, sizeof(TCmdReplyLogin) );
            return;
        }

        m_pOnlineMem->GetFixedMem( (LPVOID&)pUserInfo, CtOnlineUserInfoSize );
        pUserInfo->LocalIP = pCmdRecv->FLocalIP;
        pUserInfo->LocalPort = pCmdRecv->FLocalPort;
        pUserInfo->RemoteAddr = ARemoteAddr;
        pUserInfo->Socket = ASocket;
        pUserInfo->UserID = pCmdRecv->FUserID;
        pUserInfo->SafeCode = (DWORD)GetRunningSecond() + rand();
        pUserInfo->LastActiveTime = GetRunningSecond();
        memcpy( pUserInfo->ClientHash, pCmdRecv->FClientHash, CtHashLength );

        if ( !m_pOnlineManage->Add(pUserInfo->UserID, (LPVOID)pUserInfo) )
        {
            m_pOnlineMem->FreeMem( (LPVOID)pUserInfo );
            DebugAO( "无法添加到 m_pListOnlineUser;" );
            TCmdReplyLogin cmd( CtOnlineServerID );
            cmd.FRelaySign = 2; //拒绝登录
            SendBuffer( ASocket, ARemoteAddr, (char*)&cmd, sizeof(TCmdReplyLogin) );
            return;
        }
        DoUserLogin( pUserInfo );

        TCmdReplyLogin cmd( CtOnlineServerID );
        cmd.FRelaySign = 0; //正常
        cmd.FPublicIP = ARemoteAddr.sin_addr.S_un.S_addr;
        cmd.FPublicPort = ARemoteAddr.sin_port;
        cmd.FSafeCode = pUserInfo->SafeCode;
        SendBuffer( ASocket, ARemoteAddr, (char*)&cmd, sizeof(TCmdReplyLogin) );
    }
    else
    {
        //已经存在对象
        DebugAO( "注册命令 已经存在对象" );
        TCmdReplyLogin cmd( CtOnlineServerID );
        cmd.FRelaySign = 1; //需要重新注册
        SendBuffer( ASocket, ARemoteAddr, (char*)&cmd, sizeof(TCmdReplyLogin) );
    }
}

void CxdUdpOnlineServer::DoClientLogoutCmd( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{    
    if ( !TCmdLogout::IsSameSize(ABufLen) )
    {
        DebugAO( "logout Cmd length is error" );
        return;
    }
    TCmdLogout *pCmdRecv = (TCmdLogout*)ApRecvBuffer;
    
    TxdOnlineUserInfo *pUserInfo;
    AutoMutex( m_mtOnline );
    if ( !m_pOnlineManage->Find(pCmdRecv->FUserID, (LPVOID&)pUserInfo) )
    {
        return;
    }
    if ( pUserInfo->SafeCode == pCmdRecv->FSafeCode )
    {
        m_pOnlineManage->Delete(pCmdRecv->FUserID);
        DoUserLogout( pUserInfo );
        m_pOnlineMem->FreeMem( pUserInfo );
    }
    else
    {
        DebugAO( "logout Cmd: safecode is error. donot delete user" );
    }
}

void CxdUdpOnlineServer::DoHeartbeatCmd( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{    
    if ( !TCmdHeartbeat::IsSameSize(ABufLen) )
    {
        DebugAO( "heartbeat Cmd length is error" );
        return;
    }
    TCmdHeartbeat *pCmdRecv = (TCmdHeartbeat*)ApRecvBuffer;

    bool bOK( true );
    if ( pCmdRecv->FUserID > CtClientMinRegID )
    {
        AutoMutex( m_mtOnline );
        bOK = Assigned( FindOnlineUser(pCmdRecv->FUserID) );
    }    
    if ( !bOK )
    {
        InterlockedAdd( m_ReLoginCmd );
        TCmdClientReLogin cmd(CtOnlineServerID);
        SendBuffer( ASocket, ARemoteAddr, (char*)&cmd, sizeof(TCmdClientReLogin) );
    }
    else if ( pCmdRecv->FNeedReply )
    {
        TCmdHeartbeat cmd( CtOnlineServerID );
        cmd.FNeedReply = false;
        SendBuffer( ASocket, ARemoteAddr, (char*)&cmd, sizeof(TCmdHeartbeat) );
    }
}

void CxdUdpOnlineServer::DoHelloCmd( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    TCmdHello cmd( CtOnlineServerID );
    SendBuffer( ASocket, ARemoteAddr, (char*)&cmd, sizeof(TCmdHead) );
}

TxdOnlineUserInfo* CxdUdpOnlineServer::FindOnlineUser( const DWORD &AUserID, bool bUpdateTime )
{
    TxdOnlineUserInfo* pUser;
    if ( m_pOnlineManage->Find(AUserID, (LPVOID&)pUser) )
    {
        if ( bUpdateTime )
        {
            pUser->LastActiveTime = GetRunningSecond();
        }        
        return pUser;
    }
    return NULL;
}

void CxdUdpOnlineServer::InitStateCmdValue( void )
{
    m_RegisterCmd = 0;
    m_ClientLoginCmd = 0;
    m_ClientLogoutCmd = 0;
    m_HeartbeatCmd = 0;
    m_ReLoginCmd = 0;
    m_HelloCmd = 0;
}

void CxdUdpOnlineServer::DoThreadCheckOnlineUser( LPVOID )
{
    //检查在线用户超时
    AutoMutex( m_mtOnline );
    UINT64 dwCurTime = GetRunningSecond();

    UINT64 nID;
    TxdOnlineUserInfo *pInfo;
    if ( m_pOnlineManage->LoopBegin() )
    {
        while ( m_pOnlineManage->LoopNext(nID, (LPVOID&)pInfo) )
        {
            if ( dwCurTime - pInfo->LastActiveTime >= (UINT64)GetUserUnActiveTimeout() )
            {
                if ( DoUserTimeout(pInfo) )
                {
                    m_pOnlineManage->DeleteLoopItem();
                    m_pOnlineMem->FreeMem( pInfo );
                }
            }
        }
    }
}

void CxdUdpOnlineServer::FreeCheckThread( void )
{
    if ( m_pThreadCheck != NULL )
    {
        m_pThreadCheck->SetLoopExecuteSpaceTime( 0 );
        m_pThreadCheck->SetAutoFreeThreadObject( false );
        m_pThreadCheck->PostEvent();
        while ( m_pThreadCheck->GetRunning() )
        {
            Sleep( 10 );
        }
        delete m_pThreadCheck;
        m_pThreadCheck = NULL;
    }
}
