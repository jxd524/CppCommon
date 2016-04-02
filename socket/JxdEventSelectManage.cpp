#include "stdafx.h"
#include "JxdEventSelectManage.h"
#include "JxdTime.h"

//CxdEventGroup
CxdEventGroup::CxdEventGroup(): CxdThread(true), m_ltWaitAdd(WSA_MAXIMUM_WAIT_EVENTS), m_ltWaitDelete(WSA_MAXIMUM_WAIT_EVENTS)
{
    SetAutoFreeThreadObject( false );
    m_SocketCount = 0;
    m_RunningThreadCount = 0;
    m_bWaitCloseThread = false;
    m_hEvents[0] = WSACreateEvent();
    m_hSockets[0] = -1;
    m_NetworkEvents = 0;
    m_ThreadStopTime = GetRunningSecond();
}

CxdEventGroup::~CxdEventGroup()
{
    m_bWaitCloseThread = true;
    int nMaxWaitCount = 300;
    while ( (m_RunningThreadCount != 0) && (--nMaxWaitCount > 0) )
    {
        m_bWaitCloseThread = true;
        PostNotify();
        Sleep( 10 );
    }
    for( int i = 0; i <= m_SocketCount; i++ )
    {
    	WSACloseEvent( m_hEvents[i] );
    }
    m_SocketCount = 0;
    for ( TxdNetworkEventListIT it = m_ltNetworkEvent.begin(); it != m_ltNetworkEvent.end(); it++ )
    {
        TxdNetworkEvent *pEvent = *it;
        delete pEvent;
    }
    m_ltNetworkEvent.clear();
}

bool CxdEventGroup::AddNetworkEvent( TxdEventStyle AStyle, IxdSocketEventHandle *ApHandleInterface )
{
    AutoMutex( m_mutex );
    if ( IsExsitsNetworkEvent(AStyle) ) return false;
    TxdNetworkEvent *pEvent = new TxdNetworkEvent;
    pEvent->EventBit = (int)AStyle;
    pEvent->Event = 1 << pEvent->EventBit;
    pEvent->SocketHandle = ApHandleInterface;
    m_ltNetworkEvent.push_back( pEvent );

    m_NetworkEvents |= pEvent->Event;
    return true;
}

bool CxdEventGroup::IsExsitsNetworkEvent( TxdEventStyle AStyle )
{
    for ( TxdNetworkEventListIT it = m_ltNetworkEvent.begin(); it != m_ltNetworkEvent.end(); it++ )
    {
        TxdNetworkEvent *pEvent = *it;
        if ( pEvent->EventBit == (int)AStyle )
        {
            return true;
        }
    }
    return false;
}

void CxdEventGroup::PostNotify( void )
{
    SetEvent( m_hEvents[0] );
}

bool CxdEventGroup::Add( SOCKET ASocket )
{
    AutoMutex( m_mutex );
    int nCount = m_SocketCount + 1 + m_ltWaitAdd.GetCount() ;
    if ( nCount == WSA_MAXIMUM_WAIT_EVENTS ) return false;

    m_ltWaitAdd.Put( ASocket );
    if ( !GetRunning() )
    {
        BeginThread();
    }
    PostNotify();
    return true;
}

bool CxdEventGroup::Delete( SOCKET ASocket )
{
    AutoMutex( m_mutex );
    for( int i = 1; i < m_SocketCount + 1; i++ )
    {
        if ( ASocket == m_hSockets[i] )
        {
            m_ltWaitDelete.Put( ASocket );
            PostNotify();
            return true;
        }
    }
    return false;    
}

void CxdEventGroup::OnExecute()
{
    InterlockedAdd( m_RunningThreadCount );
    int nIndex, nWaitCount;
    DWORD dwCode;
    SOCKET s(0);
    WSANETWORKEVENTS NetEvents;
    while ( !m_bWaitCloseThread )
    {
        nWaitCount = m_SocketCount + 1;
        dwCode = WSAWaitForMultipleEvents( nWaitCount, m_hEvents, FALSE, WSA_INFINITE, FALSE );
        if( m_bWaitCloseThread ) break;

        nIndex = dwCode - WSA_WAIT_EVENT_0;
        for ( int i = nIndex; i < nWaitCount; i++ )
        {
            nIndex = WSAWaitForMultipleEvents( 1, &m_hEvents[i], TRUE, 0, FALSE );
            if ( (nIndex != WSA_WAIT_FAILED) && (nIndex != WSA_WAIT_TIMEOUT) ) 
            {
                if ( SOCKET_ERROR != WSAEnumNetworkEvents(m_hSockets[i], m_hEvents[i], &NetEvents) )
                {
                    //通知接口进行处理
                    AutoMutex( m_mutex );
                    for ( TxdNetworkEventListIT it = m_ltNetworkEvent.begin(); it != m_ltNetworkEvent.end(); it++ )
                    {
                        TxdNetworkEvent *pEvent = *it;
                        if ( (NetEvents.lNetworkEvents & pEvent->Event) && (NetEvents.iErrorCode[int(pEvent->EventBit)] == 0) )
                        {
                            pEvent->SocketHandle->Notify( m_hSockets[i] );
                        }
                        if( m_bWaitCloseThread ) break;
                    }
                }
                //只需重置自己的事件, 其它由API自动完成
                if( 0 == i ) ResetEvent( m_hEvents[i] );
            } 
        }

        //other handle
        if( m_bWaitCloseThread ) break;

        //删除需要删除的SOCKET
        DeleteWaitSocket();

        //为指定Socket添加事件
        AddWaitSocket();
        
        //如果没有需要处理的事件,则停止线程
        AutoMutex( m_mutex );
        if( 0 == m_SocketCount ) break;
        if( m_ltNetworkEvent.empty() ) break;
    }
    //线程准备退出
    for( int i = 1; i <= m_SocketCount; i++ )
    {
    	WSACloseEvent( m_hEvents[i] );
    }
    m_SocketCount = 0;
    InterlockedDec( m_RunningThreadCount );
    m_ThreadStopTime = GetRunningSecond();
}

void CxdEventGroup::AddWaitSocket( void )
{
    SOCKET s;
    while ( m_ltWaitAdd.Get(s) )
    {
        AutoMutex( m_mutex );
        m_SocketCount++;
        m_hSockets[m_SocketCount] = s;
        m_hEvents[m_SocketCount] = WSACreateEvent();
        WSAEventSelect( m_hSockets[m_SocketCount], m_hEvents[m_SocketCount], m_NetworkEvents );
    }
}

void CxdEventGroup::DeleteWaitSocket( void )
{
    SOCKET s;
    while ( m_ltWaitDelete.Get(s) )
    {
        AutoMutex( m_mutex );
        int nCount = m_SocketCount + 1;
        for( int i = 1; i < nCount; i++ )
        {
            if ( s == m_hSockets[i] )
            {
                WSACloseEvent( m_hEvents[i] );
                if ( i + 1 != nCount )
                {
                    memcpy( &m_hSockets[i], &m_hSockets[i + 1], (nCount - i - 1) * sizeof(SOCKET) );
                    memcpy( &m_hEvents[i], &m_hEvents[i + 1], (nCount - i - 1) * sizeof(HANDLE) );
                }
                m_SocketCount--;
                break;
            }
        }
    }
}

void CxdEventGroup::WaitFreeObject()
{
    SetAutoFreeThreadObject( true );
    m_bWaitCloseThread = true;
    PostNotify();
}

//CxdEventSelectIO
CxdEventSelectManage::CxdEventSelectManage()
{
}

CxdEventSelectManage::~CxdEventSelectManage()
{
    Clear();
}

bool CxdEventSelectManage::AddSocketEvent( TxdEventStyle AEventStyle, IxdSocketEventHandle *ApHandleInterface )
{
    if ( ApHandleInterface == NULL || !m_ltGroup.empty() ) return false;

    AutoMutex( m_mutext );
    for ( TxdStyleListIT it = m_ltStyle.begin(); it != m_ltStyle.end(); it++ )
    {
        TxdStyleInfo *pStyle = *it;
        if ( pStyle->EventStyle == AEventStyle )
        {
            return false;
        }
    }
    TxdStyleInfo *pStyle = new TxdStyleInfo;
    pStyle->EventStyle = AEventStyle;
    pStyle->EventHandle = ApHandleInterface;
    m_ltStyle.push_back( pStyle );
    return true;
}

bool CxdEventSelectManage::RelateSocket( SOCKET ASocket )
{
    AutoMutex( m_mutext );
    for ( TxdGroupListIT it = m_ltGroup.begin(); it != m_ltGroup.end(); it++ )
    {
        CxdEventGroup *pGroup = *it;
        if ( pGroup->Add(ASocket) )
        {
            return true;
        }
    }
    CxdEventGroup *pGroup = new CxdEventGroup;
    for ( TxdStyleListIT it = m_ltStyle.begin(); it != m_ltStyle.end(); it++ )
    {
        TxdStyleInfo *pStyle = *it;
        pGroup->AddNetworkEvent( pStyle->EventStyle, pStyle->EventHandle );
    }
    m_ltGroup.push_back( pGroup );
    return pGroup->Add( ASocket );
}

bool CxdEventSelectManage::UnRelateSocket( SOCKET ASocket )
{
    AutoMutex( m_mutext );
    for ( TxdGroupListIT it = m_ltGroup.begin(); it != m_ltGroup.end(); it++ )
    {
        CxdEventGroup *pGroup = *it;
        if ( pGroup->Delete(ASocket) )
        {
            return true;
        }
    }
    return false;
}

void CxdEventSelectManage::Clear( void )
{
    AutoMutex( m_mutext );
    for ( TxdGroupListIT it = m_ltGroup.begin(); it != m_ltGroup.end(); it++ )
    {
        CxdEventGroup *pGroup = *it;
        pGroup->WaitFreeObject();
    }
    m_ltGroup.clear();

    for ( TxdStyleListIT it = m_ltStyle.begin(); it != m_ltStyle.end(); it++ )
    {
        TxdStyleInfo *pStyle = *it;
        delete pStyle->EventHandle;
        delete pStyle;
    }
    m_ltStyle.clear();
}
