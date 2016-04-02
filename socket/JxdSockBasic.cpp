#include "stdafx.h"
#include "JxdSockBasic.h"
#include "JxdDebugInfo.h"
#include "JxdSockSub.h"
#include "JxdTime.h"

#ifdef linux
int xdGetSocketError() { return errno; }
#endif

//CxdSocketBasic
CxdSocketBasic::CxdSocketBasic()
{
    m_bActive = false;
    m_ReUseAddr = false;
    m_AutoIncPort = false;
    m_SysRecvBufferSize = 0;
    m_SysSendBufferSize = 0;
    m_BlockSocketOpt = false;
#ifdef WIN32
    StartWinSock();
#endif
}

CxdSocketBasic::~CxdSocketBasic()
{
#ifdef WIN32
    CleanWinSock();
#endif
}

bool CxdSocketBasic::IsExistsBinds( unsigned long AIP, unsigned short int APort, bool bDelete )
{
    for ( vector<TxdServerBindInfo>::iterator it = m_ltBindInfo.begin(); it != m_ltBindInfo.end(); it++ )
    {
        if ( it->IP == AIP && it->Port == APort )
        {
            if ( bDelete )
            {
                m_ltBindInfo.erase( it );
            }
            return true;
        }
    }
    return false;
}

bool CxdSocketBasic::AddBindInfo( const char *ApIP, unsigned short int APort )
{
    return AddBindInfo( inet_addr(ApIP), APort );
}

bool CxdSocketBasic::AddBindInfo( unsigned long AIP, unsigned short int APort )
{
    if ( !IsExistsBinds(AIP, APort, false) )
    {
        m_ltBindInfo.push_back( TxdServerBindInfo(AIP, APort) );
        return true;
    }
    return false;
}

bool CxdSocketBasic::DeleteBindInfo( unsigned long AIP, unsigned short int APort )
{
    return IsExistsBinds( AIP, APort, true );
}

bool CxdSocketBasic::GetBindInfo( const int &AIndex, unsigned long &AIP, unsigned short int &APort )
{
    vector<TxdServerBindInfo>::iterator it = m_ltBindInfo.begin() + AIndex;
    if ( it != m_ltBindInfo.end() )
    {
        AIP = it->IP;
        APort = it->Port;
        return true;
    }
    return false;
}

void CxdSocketBasic::ClearBindInfo( void )
{
    m_ltBindInfo.clear();
}

int CxdSocketBasic::GetBindCount( void )
{
    return m_ltBindInfo.size();
}

void CxdSocketBasic::CreateBindSocks()
{
    int nIndex = 0;
    for ( vector<TxdServerBindInfo>::iterator it = m_ltBindInfo.begin(); it != m_ltBindInfo.end(); it++ )
    {
        SOCKET nSocket = CreateSocket(nIndex);
        if ( nSocket > 0 )
        {
            SetSocketBlock( nSocket, GetBlockSocketOpt() );
            if ( !SetSocketReUseAddr(nSocket, m_ReUseAddr) ) DebugAO( "SetSocketReUseAddr failed" );

            SettingSocketBuffer( nSocket );
            TxdSocketAddr addr( it->IP, it->Port );
            bool bBindOK = 0 == bind(nSocket, (sockaddr*)&addr, sizeof(TxdSocketAddr));
            int nMaxTryCount( 100 );
            while ( !bBindOK && (nMaxTryCount-- > 0) && GetAutoIncPort() && xdGetSocketError() == EADDRINUSE )
            {
                addr.sin_port = (addr.sin_port + 1) % 65530;
                bBindOK = 0 == bind(nSocket, (sockaddr*)&addr, sizeof(TxdSocketAddr));
            }
            DoBindSockefFinished( nSocket, nIndex++, bBindOK );
            if ( bBindOK )
            {
                it->s = nSocket;
            }
            else
            {
                closesocket( nSocket );
            }
        }
        
    }
}

void CxdSocketBasic::DoBindSockefFinished( SOCKET ASocket, int AIndex, bool AIsOK )
{
    if ( !AIsOK )
    {
        closesocket( ASocket );
    }
}

bool CxdSocketBasic::ActiveService()
{
    if ( m_ltBindInfo.empty() )
    {
        return false;
    }
    CreateBindSocks();
    m_ActiveTime = GetCurrenTime();
    return true;
}

void CxdSocketBasic::SettingSocketBuffer( int ASocket )
{
    if ( m_SysRecvBufferSize >= 0 )
    {
        if ( !SetSocketSysRecvBuffer(ASocket, m_SysRecvBufferSize) )
        {
            DebugA( "SetSocketSysRecvBuffer failed: %d", ASocket );
        }
    }
    if ( m_SysSendBufferSize >= 0 )
    {
        if ( !SetSocketSysSendBuffer(ASocket, m_SysSendBufferSize) )
        {
            DebugA( "SetSocketSysSendBuffer failed: %d", ASocket );
        }
    }
}

bool CxdSocketBasic::AddLocal( unsigned short int APort )
{
    return AddBindInfo( (unsigned long)0, APort );
}

bool CxdSocketBasic::UnActiveService()
{
    for ( vector<TxdServerBindInfo>::iterator it = m_ltBindInfo.begin(); it != m_ltBindInfo.end(); it++ )
    {
        if ( it->s != 0 )
        {
            DoSocketClosed( it->s );
            closesocket( it->s );
            it->s = 0;
        }
    }
    return true;
}


