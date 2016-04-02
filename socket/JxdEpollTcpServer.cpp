#include "JxdEpollTcpServer.h"

#include <errno.h>
#include <sys/epoll.h>

#include "JxdSockSub.h"
#include "JxdDebugInfo.h"
#include "JxdStrSub.h"

//CxdTcpBasicServer
CxdTcpBasicServer::CxdTcpBasicServer()
{
    m_nSockCount = 0;
    m_ListenETMode = true;
    m_ListenThreadCount = 1;
    m_CurListenThreadCount = 0;
    m_MaxWaitListenEpollTimeout = 3000;

    m_ClientThreadCount = 1;
    m_hClientEpoll = 0;
    m_CurClientThreadCount = 0;
    m_MaxWaitClientEpollTimeout = 1000;    
}

CxdTcpBasicServer::~CxdTcpBasicServer()
{
    SetActive( false );
}

bool CxdTcpBasicServer::ActiveService()
{
    if ( !m_OnlineManage.SetActive(true) )
    {
        Debug( "can not set online manage active!" )
        return false;
    }
    //初始化资源
    m_bWaitForUnActive = false;
    m_hListenEpoll = epoll_create( CtMaxBindSocketCount );
    m_hClientEpoll = epoll_create( 1024 );
    if ( !CxdSocketBasic::ActiveService() || m_nSockCount <= 0 )
    {
        close( m_hClientEpoll );
        close( m_hListenEpoll );
        return false;
    }
    //创建监听线程
    CreateListenThread();
    CreateClientThread();
    return true;
}

bool CxdTcpBasicServer::UnActiveService()
{
    m_bWaitForUnActive = true;
    while( m_CurListenThreadCount > 0 || m_CurClientThreadCount > 0 ) Sleep( 50 );

    for ( int i = 0; i != m_nSockCount; i++ )
    {
        close( m_hListenSocks[i] );
    }
    close( m_hClientEpoll );
    close( m_hListenEpoll );

    m_nSockCount = 0;
    m_hClientEpoll = 0;
    m_hListenEpoll = 0;
    m_bWaitForUnActive = false;
    m_OnlineManage.SetActive( false );
    return true;
}

void CxdTcpBasicServer::CreateListenThread( void )
{
    int nThreadCount( m_ListenThreadCount > 1 ? m_ListenThreadCount : 1 );
    CxdThreadAttr att;
    att.SetDetachState( true );
    att.SetPrority( 10 );
    //att.SetStackSize( 1024 * 8 );
    DoSettingListenThreadAttr( &att );
    m_CurListenThreadCount = 0;
    for ( int i = 0; i != nThreadCount; i++ )
    {
        CxdThreadEvent<CxdTcpBasicServer, int> *pThread = new CxdThreadEvent<CxdTcpBasicServer, int>;
        pThread->SetAutoFreeThreadObject( true );
        pThread->GetEventThreadExecute()->SetParam( this, &CxdTcpBasicServer::DoHandleListenSocketThread );
        pThread->GetEventThreadExecuteFinished()->SetParam( this, &CxdTcpBasicServer::DoHandleListenSocketThreadFinished );
        pThread->SetThreadParam( i );
        pThread->BeginThread( &att );
    }
}

void CxdTcpBasicServer::CreateClientThread( void )
{
    int nThreadCount( m_ClientThreadCount > 1 ? m_ClientThreadCount : 1 );
    CxdThreadAttr att;
    att.SetDetachState( true );
    //att.SetPrority( 5 );
    //att.SetStackSize( 1024 * 512 );
    DoSettingClientThreadAttr( &att );
    m_CurClientThreadCount = 0;
    for ( int i = 0; i != nThreadCount; i++ )
    {
        CxdThreadEvent<CxdTcpBasicServer, int> *pThread = new CxdThreadEvent<CxdTcpBasicServer, int>;
        pThread->SetAutoFreeThreadObject( true );
        pThread->GetEventThreadExecute()->SetParam( this, &CxdTcpBasicServer::DoHandleClientThread );
        pThread->GetEventThreadExecuteFinished()->SetParam( this, &CxdTcpBasicServer::DoHandleClientThreadFinished );
        pThread->SetThreadParam( i );
        pThread->BeginThread( &att );
    }
}

int CxdTcpBasicServer::CreateSocket( int AIndex )
{
    if ( AIndex >= CtMaxBindSocketCount )
    {
        return -1;
    }
    int nSocket = socket( AF_INET, SOCK_STREAM, 0 );
    SetSocketBlock( nSocket, false );
    return nSocket;
}

void CxdTcpBasicServer::DoBindSockefFinished( int ASocket, int AIndex, bool AIsOK )
{
    if ( !AIsOK )
    {
        close( ASocket );
    }
    else
    {
        int nIndex = m_nSockCount;
        m_hListenSocks[nIndex] = ASocket;
        m_IsCurListenByETMode = m_ListenETMode;

        epoll_event et = {0};
        et.events = EPOLLIN | (m_IsCurListenByETMode ? EPOLLET : 0);
        et.data.fd = m_hListenSocks[nIndex];
        if ( 0 != epoll_ctl(m_hListenEpoll, EPOLL_CTL_ADD, m_hListenSocks[nIndex], &et) )
        {
            Debug( "accept socket(%d) can not add to epoll", ASocket );
            close( ASocket );
        }
        else
        {
            m_nSockCount++;
            listen( ASocket, 1024 );
        }
    }
}

void CxdTcpBasicServer::DoHandleClientThreadFinished( int AThreadIndex )
{
    InterlockedDec( m_CurClientThreadCount );
    Debug( "client handle thread is stop, ThreadIndex=%d, ThreadID=%X", AThreadIndex, pthread_self() );
}

void CxdTcpBasicServer::DoHandleClientThread( int AThreadIndex )
{
    InterlockedAdd( m_CurClientThreadCount );
    Debug( "client handle thread is running, ThreadIndex=%d, ThreadID=%X", AThreadIndex, pthread_self() );

    epoll_event eWaitEvents[CtMaxWaitEpollEvent];
    TxdSockBufBasic *pSockObj;
    int i, nEvents, nError, nRevents;

    while ( !m_bWaitForUnActive )
    {
        nEvents = epoll_wait( m_hClientEpoll, eWaitEvents, CtMaxWaitEpollEvent, m_MaxWaitClientEpollTimeout );
        
        if ( m_bWaitForUnActive ) break;
        nError = (nEvents == -1) ? errno : 0;
        if (nError) { continue; }

        for ( i = 0; i < nEvents ; i++ )
        {
            //(TxdSockBufBasic*)eWaitEvents[i].data.ptr;
            pSockObj = FindSockObject( eWaitEvents[i].data.fd );
            if ( IsNull(pSockObj) ) 
            {
                Debug( "cant not find socket object: %d, it maybe wait close", eWaitEvents[i].data.fd );
                continue;
            }

            nRevents = eWaitEvents[i].events;
            if ( (nRevents & (EPOLLERR|EPOLLHUP)) && (nRevents & (EPOLLIN|EPOLLOUT)) == 0 )
            {
                nRevents |= EPOLLIN | EPOLLOUT;
            }

            if ( nRevents & EPOLLIN ) HandleRecv(pSockObj);
            if ( (pSockObj->CloseState == csNone) && (nRevents & EPOLLOUT) && Assigned(pSockObj) ) DoCanSendWaitBuffer( pSockObj );

            ReleaseSockObject( pSockObj );

            if ( m_bWaitForUnActive ) break;
        }
    }
}

void CxdTcpBasicServer::HandleAccept( int ASocket )
{
    int nNewSocket;
    sockaddr_in actAddr;
    TxdSockBufBasic *pObj;
    epoll_event et;

    //监听连接到来
    while ( !m_bWaitForUnActive )
    {
        socklen_t nlen = sizeof( sockaddr_in );
        nNewSocket = accept( ASocket, (sockaddr*)&actAddr, &nlen );
        if ( nNewSocket > 0 )
        {
            //连接正常状态下
            pObj = NULL;
            if ( DoAcceptNewSocket(nNewSocket, actAddr, pObj) && Assigned(pObj) )
            {
                //接收
                pObj->m_hSocket = nNewSocket;
                pObj->m_nEvents = EPOLLET | EPOLLIN | EPOLLONESHOT;
                pObj->m_nCount = 0;
                pObj->CloseState = csNone;

                SettingSocketBuffer( nNewSocket );
                SetSocketBlock( nNewSocket, false );

                et.events = pObj->m_nEvents;
                et.data.fd = nNewSocket;

                if ( epoll_ctl(m_hClientEpoll, EPOLL_CTL_ADD, nNewSocket, &et) == 0 )
                {
                    m_OnlineLock.Lock();
                    if ( !m_OnlineManage.Add(nNewSocket, pObj) )
                    {
                        TxdSockBufBasic *pOld;
                        if ( m_OnlineManage.Find(nNewSocket, (LPVOID&)pOld, true) )
                        {
                            //多线程的调度造成的原因
                            Debug( "a new socket value exsits in online manage: %d", nNewSocket );
                            FreeSocketBuffer( pOld );
                        }
                        m_OnlineManage.Add( nNewSocket, pObj );
                    }
                    m_OnlineLock.Unlock();
                }
                else
                {
                    Debug( "new socket cant not add to epoll: %d", errno );
                    DoSocketClose( pObj, csServerError );
                    SetSocketForceClose( nNewSocket );
                    close( nNewSocket );
                    FreeSocketBuffer( pObj );
                }
                CheckListenMode( true );
            }
            else
            {
                //拒绝接收连接
                SetSocketForceClose( nNewSocket );
                close( nNewSocket );
            }
        }
        else
        {
            if (errno == EMFILE)
            {
                //当前连接数已经到达系统规定的最大值 (cmd: ulimit -n)
                //如果有需要则改变为LT模式
                CheckListenMode( false );
            } 
            break;
        }

    }
}

void CxdTcpBasicServer::HandleRecv( TxdSockBufBasic *ApSockObj )
{
    int nRecvLen;
    char buf[CtRecvBufferSize - 1];
    //可以接收客户端信息
    while ( !m_bWaitForUnActive && (ApSockObj->CloseState == csNone) )
    {
        nRecvLen = recv( ApSockObj->m_hSocket, buf, CtRecvBufferSize, 0 );
        if ( nRecvLen > 0 )
        {
            DoRecvBuffer(ApSockObj, buf, nRecvLen);
            continue;
        }

        if ( 0 == nRecvLen )
        {
            ApSockObj->CloseState = csClient;
            return;
        }
        else
        {
            //接收出问题
            int nErr = errno;
            if ( EAGAIN == nErr || EWOULDBLOCK == nErr || EINTR == nErr ) break;

            Debug( "recv error, client socket: %d, maybe closed, errno: %d", ApSockObj->m_hSocket, nErr );
            ApSockObj->CloseState = csClientError;
        }
    }
}

int CxdTcpBasicServer::SendBuffer( int ASocket, const char* ApBuffer, int ALen )
{
    return send( ASocket, ApBuffer, ALen, MSG_NOSIGNAL );
}

void CxdTcpBasicServer::CloseSocket( int ASocket )
{
    TxdSockBufBasic *pObj = FindSockObject( ASocket );
    pObj->CloseState = csServer;
    ReleaseSockObject( pObj );
}

bool CxdTcpBasicServer::DoAcceptNewSocket( int ANewSocket, const sockaddr_in &AAddr, TxdSockBufBasic *&ApGetSockBuff )
{
    ApGetSockBuff = new TxdSockBufBasic;
    return true; 
}

void CxdTcpBasicServer::FreeSocketBuffer( TxdSockBufBasic *ApBuf )
{
    delete ApBuf;
}

void CxdTcpBasicServer::CheckListenMode( bool AForET )
{
    if ( m_ListenETMode )
    {
        epoll_event et;
        bool bHandle = false;
        bool bOldMode = m_IsCurListenByETMode;
        if ( AForET )
        {
            if ( !m_IsCurListenByETMode )
            {
                et.events = EPOLLIN | EPOLLET;
                bHandle = true;
            }
        }
        else
        {
            if ( m_IsCurListenByETMode )
            {
                et.events = EPOLLIN;
                bHandle = true;
            }
        }
        if ( bHandle )
        {
            bHandle = false;

            m_LockSockMode.Lock();
            if ( bOldMode == m_IsCurListenByETMode )
            {
                m_IsCurListenByETMode = !m_IsCurListenByETMode;
                bHandle = true;
            }
            m_LockSockMode.Unlock();

            if ( bHandle )
            {
                Debug( m_IsCurListenByETMode ? "changed mod: EEEEET" : "changed mod: LLLLLLT" );
                for ( int i = 0; i != m_nSockCount; i++ )
                {
                    et.data.fd = m_hListenSocks[i];
                    epoll_ctl( m_hListenEpoll, EPOLL_CTL_MOD, m_hListenSocks[i], &et );
                }
            }
        }
    }
}

bool CxdTcpBasicServer::SetOnlineHashTableCount( UINT AHashTableCount )
{
    return m_OnlineManage.SetHashTableCount( AHashTableCount );
}

void CxdTcpBasicServer::MyTest()
{
    int nRecvLen;
    char buf[CtRecvBufferSize - 1];
    //可以接收客户端信息
    nRecvLen = recv( pLast->m_hSocket, buf, CtRecvBufferSize, 0 );
    if ( nRecvLen > 0 )
    {
        DoRecvBuffer( pLast, buf, nRecvLen );
    }
        
}

void CxdTcpBasicServer::DoHandleListenSocketThread( int AThreadIndex )
{
    InterlockedAdd( m_CurListenThreadCount );
    Debug( "listen thread is running. ThreadIndex=%d, ThreadID=%X", AThreadIndex, pthread_self() );
    
    epoll_event eWaitEvents[CtMaxBindSocketCount];
    int i, nSocket, nEvents, nError;

    while ( !m_bWaitForUnActive )
    {
        nEvents = epoll_wait( m_hListenEpoll, eWaitEvents, CtMaxBindSocketCount, m_MaxWaitListenEpollTimeout );
        
        if ( m_bWaitForUnActive ) break;
        nError = (nEvents == -1) ? errno : 0;
        if (nError) { continue; }

        for ( i = 0; i < nEvents ; i++ )
        {
            nSocket = eWaitEvents[i].data.fd;
            HandleAccept( nSocket );
        }
    }
}

void CxdTcpBasicServer::DoHandleListenSocketThreadFinished( int AThreadIndex )
{
    InterlockedDec( m_CurListenThreadCount );
    Debug( "listen thread is stop. ThreadIndex=%d, ThreadID=%X", AThreadIndex, pthread_self() );
}

void CxdTcpBasicServer::OutputTcpBasicInfo()
{
    TxdGetMemInfo *pInfos = NULL;
    int nCount = m_OnlineManage.GetHashNodeMemInfo(pInfos, 0);
    Debug( "TcpBasicServerInfo:\r\n    ListenETMode: %s\r\n    CurListenETMode: %s\r\n    MaxWaitListenEpollTimeout: %d\r\n\
    ListenThreadCount: %d\r\n    CurListenThreadCount: %d\r\n\r\n    ClientThreadCount: %d\r\n    \
    MaxWaitClientEpollTimeout: %d\r\nCurClientThreadCount: %d\r\n\r\n\
    OnlineCount: %d\r\n    OnlineHashTableCount: %d\r\n    OnlineHashMemInfo: %s\r\n\r\n    ",
        GetListenETMode() ? "ET" : "LT", m_IsCurListenByETMode ? "ET" : "LT", GetMaxWaitListenEpollTimeout(),
        GetListenThreadCount(), GetCurListenThreadCount(), GetClientThreadCount(), 
        GetMaxWaitClientEpollTimeout(), GetCurClientThreadCount(), GetOnlineCount(), GetOnlineHashTableCount(),
        FormatMemInfo(pInfos, nCount).c_str()  );
    CxdFixedMemoryManage::FreeMemInfo( pInfos );
}

TxdSockBufBasic* CxdTcpBasicServer::FindSockObject( int ASocket )
{
    TxdSockBufBasic *pObj = NULL;
    m_OnlineLock.Lock();
    if ( m_OnlineManage.Find(ASocket, (LPVOID&)pObj, false) ) 
    {
        if ( pObj->CloseState == csNone )
        {
            pObj->m_nCount++;
        }
        else
        {
            pObj = NULL;
        }
    }
    m_OnlineLock.Unlock();
    return pObj;
}

void CxdTcpBasicServer::ReleaseSockObject( TxdSockBufBasic *ApObj )
{
    epoll_event et = { 0, {0} };

    m_OnlineLock.Lock();
    bool bZore = 0 == --ApObj->m_nCount;
    if ( bZore )
    {
        et.events = ApObj->m_nEvents;
        et.data.fd = ApObj->m_hSocket;
    }
    bool bDel = bZore && (ApObj->CloseState != csNone);
    if ( bDel )
    {
        m_OnlineManage.Delete( ApObj->m_hSocket );
    }
    m_OnlineLock.Unlock();

    
    if ( bDel )
    {
        epoll_ctl( m_hClientEpoll, EPOLL_CTL_DEL, ApObj->m_hSocket, &et );
        DoSocketClose( ApObj, ApObj->CloseState );
        ApObj->CloseState == csClient ? 
            shutdown( ApObj->m_hSocket, SHUT_RDWR ) : SetSocketForceClose( ApObj->m_hSocket );
        close( ApObj->m_hSocket );
        FreeSocketBuffer( ApObj );
    }
    else if ( bZore )
    {
        epoll_ctl( m_hClientEpoll, EPOLL_CTL_MOD, ApObj->m_hSocket, &et );
    }
}

void CxdTcpBasicServer::MyTestClose()
{
    CloseSocket( pLast->GetSocket() );
}


//CxdTcpServerIOHandler
CxdTcpIOBufferServer::CxdTcpIOBufferServer()
{
    InitDefaultInfo();    
}

void CxdTcpIOBufferServer::InitDefaultInfo()
{
    //固定长度
    m_mmWaitSend.AddAllocInfo( sizeof(TxdSockIOBuf), sizeof(TxdSockIOBuf), 5000 );
    m_mmWaitSend.AddAllocInfo( sizeof(TxdBlockBuffer), sizeof(TxdBlockBuffer), 3000 );
    //范围长度
    m_mmWaitSend.AddAllocInfo(          0, 1024,        1024 * 4 );
    m_mmWaitSend.AddAllocInfo(       1024, 1024 * 4,    1024     ); 
    m_mmWaitSend.AddAllocInfo(   1024 * 4, 1024 * 8,    512      );
    m_mmWaitSend.AddAllocInfo(   1024 * 8, 1024 * 16,   512      );
    m_mmWaitSend.AddAllocInfo(  1024 * 16, 1024 * 32,   128      );
    m_mmWaitSend.AddAllocInfo(  1024 * 32, 1024 * 64,   128      );
    m_mmWaitSend.AddAllocInfo(  1024 * 64, 1024 * 128,  128      );
    m_mmWaitSend.AddAllocInfo( 1024 * 128, 1024 * 256,  128      );
    m_mmWaitSend.AddAllocInfo( 1024 * 256, 1024 * 1024, 64       );
}

void CxdTcpIOBufferServer::SetOnlineHashNodeInitCount( UINT ACount )
{
    CxdTcpBasicServer::SetOnlineHashNodeInitCount( ACount );
    m_mmWaitSend.AddAllocInfo( sizeof(TxdSockIOBuf), ACount );
}

void CxdTcpIOBufferServer::FreeSocketBuffer( TxdSockBufBasic *ApBuf )
{
    TxdSockIOBuf *pIOBuf = (TxdSockIOBuf*)ApBuf;
    m_mxWaitSendLock.Lock();
    FreeTcpBuffer( pIOBuf->m_pWaitSendBuffer, true, false );
    m_mmWaitSend.FreeMem( pIOBuf );
    m_mxWaitSendLock.Unlock();
}

int CxdTcpIOBufferServer::SendBuffer( int ASocket, const char* ApBuffer, int ALen )
{
    int nReuslt = -1;
    TxdSockBufBasic *pObj = FindSockObject( ASocket );
    if ( Assigned(pObj) )
    {
        nReuslt = SendBuffer( (TxdSockIOBuf*)pObj, ApBuffer, ALen );
        ReleaseSockObject( pObj );
    }
    return nReuslt;
}

int CxdTcpIOBufferServer::SendBuffer( TxdSockIOBuf *ApSockObj, const char* ApBuffer, int ALen )
{
    AutoMutexVal( ApSockObj->m_SendBufferLock );
    if ( IsNull(ApSockObj->m_pWaitSendBuffer) )
    {
        int nResult = CxdTcpBasicServer::SendBuffer( ApSockObj->GetSocket(), ApBuffer, ALen );
        if ( ALen == nResult )
        {
            DoSendFinsiehd( ApSockObj, ApBuffer, ALen );
            return nResult;
        }

        if ( -1 == nResult )
        {
            int nErrNo = errno;
            if ( nErrNo != EAGAIN || nErrNo != EWOULDBLOCK )
            {
                Debug( "Send Buffer Error, client maybe close: errno = %d, will close this socket", nErrNo );
                ApSockObj->CloseState = csServerError;
                return -1;
            }
            nResult = 0;
        }
        else if ( nResult > 0 )
        {
            ApBuffer += nResult;
            DoSendFinsiehd( ApSockObj, ApBuffer, nResult );
        }
        UINT nSpaceSize = ALen - nResult;
        ApSockObj->m_pWaitSendBuffer = GetTcpBufferForWaitSend( nSpaceSize );
        AddToTcpBuffer( ApSockObj->m_pWaitSendBuffer, ApBuffer, nSpaceSize );
        ApSockObj->AddEvent( EPOLLOUT );
        return nResult;
    }
    else
    {
        AddToTcpBuffer( ApSockObj->m_pWaitSendBuffer, ApBuffer, ALen );
        return 0;
    }
    return -2;
}


bool CxdTcpIOBufferServer::DoAcceptNewSocket( int ANewSocket, const sockaddr_in &AAddr, TxdSockBufBasic *&ApGetSockBuff )
{
    TxdSockIOBuf *pBuf = NULL;
    m_mxWaitSendLock.Lock();
    m_mmWaitSend.GetFixedMem( (LPVOID&)pBuf, sizeof(TxdSockIOBuf) );
    m_mxWaitSendLock.Unlock();

    memset( &pBuf->m_SendBufferLock, 0, sizeof(pthread_mutex_t) );
    pBuf->m_pWaitSendBuffer = NULL;
    ApGetSockBuff = (TxdSockBufBasic*)pBuf;
    return true;
}

void CxdTcpIOBufferServer::DoCanSendWaitBuffer( TxdSockBufBasic *ApSockObj )
{
    TxdSockIOBuf *pSockIOBuf = (TxdSockIOBuf*)ApSockObj;
    AutoMutexVal( pSockIOBuf->m_SendBufferLock );
    TxdBlockBuffer *pWaitBuffer = pSockIOBuf->m_pWaitSendBuffer;
    while ( Assigned(pWaitBuffer) )
    {
        int nLen = CxdTcpBasicServer::SendBuffer( pSockIOBuf->GetSocket(), pWaitBuffer->GetCurBuffer(), pWaitBuffer->GetLength() );
        if ( nLen == (int)pWaitBuffer->GetLength() )
        {
            //全部发送成功
            DoSendFinsiehd( pSockIOBuf, pWaitBuffer->GetCurBuffer(), pWaitBuffer->GetLength() );
            FreeTcpBuffer( pWaitBuffer, false, true );
        }
        else if ( nLen > 0 )
        {
            //发送了部分
            DoSendFinsiehd(pSockIOBuf, pWaitBuffer->GetCurBuffer(), nLen );
            pWaitBuffer->IncPosition( nLen );
            break;
        }
        else if ( nLen == -1 )
        {
            int nErrNo = errno;
            if ( nErrNo != EAGAIN || nErrNo != EWOULDBLOCK )
            {
                Debug( "Send Buffer Error, will free all send buffer: errno = %d", nErrNo );
                FreeTcpBuffer( pWaitBuffer, true, true );
            }
            break;
        }
    }
    pSockIOBuf->m_pWaitSendBuffer = pWaitBuffer;
    if ( IsNull(pSockIOBuf->m_pWaitSendBuffer) )
    {
        //Debug( "all wait send buffer has finished: %d", pSockIOBuf->GetSocket() );
        pSockIOBuf->DeleteEvent( EPOLLOUT );
    }
}

void CxdTcpIOBufferServer::DoSocketClose( TxdSockBufBasic *ApSockObj, TSocketCloseState ACloseState )
{

}

TxdBlockBuffer* CxdTcpIOBufferServer::GetTcpBufferForWaitSend( int ALen )
{
    UINT nMaxLen;
    LPVOID pBuf = NULL;
    TxdBlockBuffer *pObj;

    m_mxWaitSendLock.Lock();
    m_mmWaitSend.GetFixedMem( (LPVOID&)pObj, sizeof(TxdBlockBuffer) );
    m_mmWaitSend.GetValMem( pBuf, ALen, nMaxLen );
    m_mxWaitSendLock.Unlock();

    memset( pObj, 0, sizeof(TxdBlockBuffer) );
    pObj->InitBuffer( pBuf, nMaxLen );
    return pObj;
}

void CxdTcpIOBufferServer::AddToTcpBuffer( TxdBlockBuffer *ApBuf, const char* ApAddBuff, int ALen )
{
    while ( Assigned(ApBuf->GetNextBuffer()) )
    {
        ApBuf = ApBuf->GetNextBuffer();
    }
    UINT nCopySize = ApBuf->AddBuffer( ApAddBuff, ALen );
    UINT nSpaceSize = ALen - nCopySize;
    if ( nSpaceSize > 0 )
    {
        ApAddBuff += nCopySize;
        TxdBlockBuffer *pNewBuffer = GetTcpBufferForWaitSend( nSpaceSize );
        ApBuf->SetNextBuffer( pNewBuffer );
        pNewBuffer->AddBuffer( ApAddBuff, nSpaceSize );
    }
}

int CxdTcpIOBufferServer::FreeTcpBuffer( TxdBlockBuffer *&ApBuf, bool AFreeAll, bool ALock )
{
    int nResult = 0;
    TxdBlockBuffer *pTemp;
    if ( ALock ) m_mxWaitSendLock.Lock();
    while ( Assigned(ApBuf) )
    {
        nResult += ApBuf->GetLength();
        pTemp = ApBuf->GetNextBuffer();
        if ( Assigned(ApBuf->GetBuffer()) )
        {
            m_mmWaitSend.FreeMem( ApBuf->GetBuffer() );
        }
        m_mmWaitSend.FreeMem( ApBuf );
        ApBuf = pTemp;
    }
    if ( ALock ) m_mxWaitSendLock.Unlock();
    return nResult;
}

void CxdTcpIOBufferServer::OutputTcpIOBufferServerInfo()
{
    TxdGetMemInfo *pWaitMemInfos = NULL;
    m_mxWaitSendLock.Lock();
    int nCount = m_mmWaitSend.GetMemInfo(pWaitMemInfos, 0);
    m_mxWaitSendLock.Unlock();
    Debug( "TcpIOBufferServerInfo:\r\n    WaitSendDefaultBlockCount: %d\r\n    SendBufferLegth: %ld\r\n\
    RecvBufferLength: %ld\r\n    WaitSendMemInfo: %.2048s",
        GetWaitSendDefaultBlockCount(), m_SendBufferLength, m_RecvBufferLength, FormatMemInfo(pWaitMemInfos, nCount).c_str() );
    CxdFixedMemoryManage::FreeMemInfo( pWaitMemInfos );
}

bool CxdTcpIOBufferServer::ActiveService()
{
    m_SendBufferLength = 0;
    m_RecvBufferLength = 0;
    return CxdTcpBasicServer::ActiveService();
}

void CxdTcpIOBufferServer::DoSendFinsiehd( TxdSockIOBuf *ApSockObj, const char* ApBuffer, int ALen )
{
    UINT64 n = ALen;
    InterlockedAddVar( m_SendBufferLength, n );
}

void CxdTcpIOBufferServer::DoRecvBuffer( TxdSockBufBasic *ApSockObj, char* ApBuf, int ALen )
{
    UINT64 n = ALen;
    InterlockedAddVar( m_RecvBufferLength, n );
}








