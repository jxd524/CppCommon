#include "stdafx.h"
#include "JxdUdpIOHandle.h"
#include "JxdCrcSub.h"
#include "JxdDebugInfo.h"
#include "JxdMacro.h"
#include "JxdThread.h"
#include "JxdMem.h"
#include "JxdSockDefines.h"

#define CtMB 1048576
//io cmd version
static const byte CtUdpIOVersion = 1; //协议版本号

//IO CMD
#define CtIoCmd_GetCombiPackage      1
#define CtIoCmd_NotFoundCombiPackage 2

//CxdUdpIOHandle
CxdUdpIOHandle::CxdUdpIOHandle()
{
    InitInterface( UdpRecv );
    m_RecvQueueMaxCount = 1024;
    m_HandleIoThreadCount = 1;
    m_UnUsedMemTimeoutSpace = 600; //10分钟

    m_InitCombiPackageBufferCount = 128;

    m_SendCombiInitBufferCount = 500;
    m_SendCombiHashTableCount = 9999;
    m_SendCombiBufferExsitsTime = 5; //5s

    m_RecvCombiInitBufferCount = 500;
    m_RecvCombiHashTableCount = 9999;
    m_RecvCombiBufferExsitsTime = 1; //1s
    m_RecvCombiMaxTimeoutCount = 3;

    m_HandleIORunningThreadCount = 0;

    m_pCombiBufferManage = NULL;
    m_pCombiSendManage = NULL;
    m_pCombiRecvManage = NULL;
    m_pRecvPackageList = NULL;
    m_pRecvBufManage = NULL;

    InitUdpIOHandleStateValue();
}

CxdUdpIOHandle::~CxdUdpIOHandle()
{
    DeleteInterface( UdpRecv );
}

int CxdUdpIOHandle::SendBuffer( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, int ABufLen )
{
    if ( ABufLen <= CtSinglePackageBufferSize )
    {
        return SendSinglePackage( ASocket, ARemoteAddr, ApSendBuffer, ABufLen, 0 );
    }
    else if ( ABufLen <= CtMaxCombiPackageBufferSize )
    {
        return SendCombiPackage( ASocket, ARemoteAddr, ApSendBuffer, ABufLen );
    }
    DebugAO( "无法发送超过 CtMaxCombiPackageSize 长度的数据包" );
    return -1;
}

void CxdUdpIOHandle::DoHandleUdpPackage( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    if ( ABufLen < CtSinglePackageHeadSize )
    {
        InterlockedAdd( m_RecvErrorIOProtocolCount );
        DebugAO( "接收到的包长度低于最小值" );
        return;
    }
    byte nVer = *(byte*)ApRecvBuffer;
    if ( nVer != CtUdpIOVersion )
    {
        InterlockedAdd( m_RecvErrorIOProtocolCount );
        DebugAO( "协议包值不正确" );
        return;
    }
    WORD nPackageLen = *(WORD*)(ApRecvBuffer + 1);
    if ( nPackageLen != ABufLen )
    {
        InterlockedAdd( m_RecvErrorIOProtocolCount );
        DebugAO( "包的长度有误" );
        return;
    }
    DWORD dwCRC = *(DWORD*)(ApRecvBuffer + 3);
    byte nPageSign = *(byte*)(ApRecvBuffer + 7);
    byte nHi( nPageSign >> 4 );
    byte nLow( nPageSign ^ (nHi << 4) );
    bool bSinglePage( nHi == 0 );
    byte nHeadLen( bSinglePage ? CtSinglePackageHeadSize : CtCombiPackageHeadSize );
    int nLen = ABufLen - nHeadLen;
    char *pBuf = (char*)(ApRecvBuffer + nHeadLen);
    if ( !DecodeBuffer(dwCRC, (byte*)pBuf, nLen) )
    {
        InterlockedAdd( m_RecvErrorIOProtocolCount );
        DebugAO( "CRC解密失败" );
        return;
    }
    if ( bSinglePage )
    {
        InterlockedAdd( m_RecvSinglePackageCount );
        //独立包 nLow: IO命令
        if ( 0 == nLow )
        {
            //提供给子类处理
            OnRecvBuffer( ASocket, ARemoteAddr, pBuf, nLen );
        }
        else
        {
            //网络层IO协议处理
            DoHandlNetProtocolPackage(nLow, ASocket, ARemoteAddr, pBuf, nLen );
        }
    }
    else
    {
        InterlockedAdd( m_RecvCombiPackageCount );
        //组合包 nHi: 组合包数量, nLow: 组合包顺序
        DoHandleCombiPackage(ASocket, ARemoteAddr, pBuf, nLen, *(WORD*)(ApRecvBuffer + 8), nHi, nLow );
    }
}

int CxdUdpIOHandle::SendSinglePackage( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, WORD ABufLen, byte AIoCmd /*= 0*/ )
{
    //1.独立包: 版本(1) - 此包总长度(2) - CRC32码(4) - 包信息(1)[前四位：0； 后四位: 协议命令IoCmd，当为0时，表示需要由子类处理]
    AIoCmd = AIoCmd & 0xF; //00001111;
    WORD nPackageLen = CtSinglePackageHeadSize + ABufLen;
    DWORD dwCRC32 = CalcCRC32( (byte*)ApSendBuffer, ABufLen );
    char buf[CtUdpBufferMaxSize] = {0};
    *(byte*)buf        = CtUdpIOVersion;
    *(WORD*)(buf + 1)  = nPackageLen;
    *(DWORD*)(buf + 3) = dwCRC32;
    *(byte*)(buf + 7)  = AIoCmd;
    memcpy( buf + 8, ApSendBuffer, ABufLen );
    EncodeBuffer( (byte*)(buf + 8), ABufLen, dwCRC32 );
    int nSendLen = _SendPackage(ASocket, ARemoteAddr, buf, nPackageLen, true );
    if ( nSendLen == nPackageLen )
    {
        return nSendLen - CtSinglePackageHeadSize;
    }
    return nSendLen;
}

int CxdUdpIOHandle::SendCombiPackage( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, WORD ABufLen )
{
    byte nPackageCount = (ABufLen + CtPerCombiPackageBufferSize - 1) / CtPerCombiPackageBufferSize;
    if ( nPackageCount > CtMaxCombiPackageCount )
    {
        DebugAO( "发送数据量过多，超过最多许可，无法发送" );
        return -1;
    }

    if ( NULL == m_pCombiSendManage ) 
    {
        CreateCombiSendManage();
    }

    TxdSendCombiBufferInfo *pInfo;
    if ( !m_pCombiSendManage->GetCombiMem( -1, pInfo) )
    {
        DebugAO( "申请不到发送缓存节点, 无法发送组合包" );
        return -1;
    }

    pInfo->FCount = nPackageCount;
    LPVOID vPointer;
    char *pBuf;
    WORD nDataLen;
    int nSendLen(0);
    byte nCombiCount( nPackageCount << 4 );
    DWORD dwCRC32;
    byte *pSendBuf;
    for( byte i = 0; i != nPackageCount; i++ )
    {        
        if ( i == nPackageCount - 1 )
        {
            nDataLen = ABufLen - (nPackageCount - 1) * CtPerCombiPackageBufferSize;
            pInfo->FBufLens[i] = nDataLen + CtCombiPackageHeadSize;
        }
        else
        {
            nDataLen = CtPerCombiPackageBufferSize;
            pInfo->FBufLens[i] = CtUdpBufferMaxSize;
        }
        m_pCombiBufferManage->GetFixedMem( vPointer, CtUdpBufferMaxSize );

        pSendBuf = (byte*)(ApSendBuffer + i * CtPerCombiPackageBufferSize);
        dwCRC32 = CalcCRC32( pSendBuf, nDataLen );

        pBuf = (char*)vPointer;
        //2.组合包: 版本(1) - 此包总长度(2) - CRC32码(4) - 包信息(1)[前四位：个数CombiCount； 后四位: 包顺序] - 标志CombiID(2)
        *(byte*)pBuf        = CtUdpIOVersion;
        *(WORD*)(pBuf + 1)  = pInfo->FBufLens[i];
        *(DWORD*)(pBuf + 3) = dwCRC32;
        *(byte*)(pBuf + 7)  = nCombiCount | i;
        *(WORD*)(pBuf + 8)  = pInfo->FCombiID;
        memcpy( pBuf + 10, ApSendBuffer + i * CtPerCombiPackageBufferSize, nDataLen );
        EncodeBuffer( (byte*)(pBuf + 10), nDataLen, dwCRC32 );
        pInfo->FBuffers[i] = (LPVOID)pBuf;
        int nlen = _SendPackage(ASocket, ARemoteAddr, pBuf, pInfo->FBufLens[i], false );
        if ( nlen == pInfo->FBufLens[i] )
        {
            nSendLen += nDataLen;
        }
    }
    m_pCombiSendManage->ReleaseCombiMem( pInfo );
    return nSendLen;
}


int CxdUdpIOHandle::_SendPackage( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApPackage, int ALen, bool AIsSingledPackage )
{
    int nlen = __SendBuffer( ASocket, ARemoteAddr, ApPackage, ALen );
    if ( nlen == ALen )
    {
        AIsSingledPackage ? InterlockedAdd( m_SendSinglePackageCount ) : InterlockedAdd( m_SendCombiPackageCount );
    }
    return nlen;
}


void CxdUdpIOHandle::InitUdpIOHandleStateValue( void )
{
    m_RecvPackageCount = 0;
    m_RecvErrorIOProtocolCount = 0;
    m_RecvSinglePackageCount = 0;
    m_RecvCombiPackageCount = 0;
    m_AutoThrowRecvPackage = 0;
    m_LostCombiPackageCount = 0;

    m_SendPackageCount = 0;
    m_SendFailedPackageCount = 0;
    m_SendSinglePackageCount = 0;
    m_SendCombiPackageCount = 0;

#ifdef CountBufferLength
    m_SendBufferLengthMB = 0;
    m_SendBufferLengthByte = 0;
    m_RecvBufferLengthMB = 0;
    m_RecvBufferLengthByte = 0;
#endif
}

bool CxdUdpIOHandle::ActiveService( void )
{
    //接口判断
    if ( !IsExsitsItUdpRecv() )
    {
        SetItUdpRecv( GetUdpRecvInterface() );
        if ( !IsExsitsItUdpRecv() )
        {
            throw "must set udp recv Interface";
            return false;
        }
    }
    if ( !GetItUdpRecv()->AddSocketEvent( esRead, NewInterfaceByClass(CxdUdpIOHandle, SOCKET, __DoRecvBuffer)) )
    {
        throw "cant not call AddSocketEvent.";
    }
    if ( !CxdSocketBasic::ActiveService() ) return false;

    //用于接收原始UDP数据
    m_pRecvBufManage = new CxdFixedMemoryManageEx;
    m_pRecvBufManage->AddAllocInfo( sizeof(TxdRecvPackageInfo), sizeof(TxdRecvPackageInfo), GetRecvQueueMaxCount() );
    m_pRecvBufManage->InitMem();
    m_pRecvPackageList = new CxdQueueEx<TxdRecvPackageInfo*>( GetRecvQueueMaxCount() );

    InitUdpIOHandleStateValue();

    m_bWaitToClose = false;
    m_HandleIORunningThreadCount = 0;
    for (unsigned int i = 0; i != m_HandleIoThreadCount; i++)
    {
        CreateSimpleThread( CxdUdpIOHandle, LPVOID, DoThreadHandleIO, NULL );
    }
    return true;
}

bool CxdUdpIOHandle::UnActiveService( void )
{
    m_bWaitToClose = true;
    
    GetItUdpRecv()->Clear();

    while ( m_HandleIORunningThreadCount != 0 )
    {
        Sleep( 10 );
    }

    SafeDeleteObject( m_pCombiBufferManage );
    SafeDeleteObject( m_pCombiSendManage );
    SafeDeleteObject( m_pCombiRecvManage );
    SafeDeleteObject( m_pRecvPackageList );
    SafeDeleteObject( m_pRecvBufManage );
    return CxdSocketBasic::UnActiveService();
}

//void CxdUdpIOHandle::OnRecvBuffer( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
//{
//    //SendBuffer( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
//    //ApRecvBuffer[ABufLen] = 0;
//    //OutputDebugStringA( ApRecvBuffer );
//    //OutputDebugStringA( "\r\n" );
//}

void CxdUdpIOHandle::DoHandlNetProtocolPackage( byte ACmdID, SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    switch (ACmdID)
    {
    case CtIoCmd_GetCombiPackage:
        DoRecvGetCombiPackageCmd( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
    	break;
    case CtIoCmd_NotFoundCombiPackage:
        DoRecvNotFoundCombiPackageCmd( ASocket, ARemoteAddr, ApRecvBuffer, ABufLen );
        break;
    }
}

void CxdUdpIOHandle::DoRecvGetCombiPackageCmd( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    //组合包ID(2) - 丢包数量(n) - Package1,Package2...PackageN
    WORD nCombiID = *(WORD*)ApRecvBuffer;
    byte nCount = *(byte*)(ApRecvBuffer + 2);
    int nCalcLen = nCount + 3;
    if ( nCalcLen != ABufLen || NULL == m_pCombiSendManage )
    {
        DebugAO( "获取丢包命令出错或者不存在组合对象,将丢弃" );
        SendNotFoundCombiPackage( ASocket, ARemoteAddr, nCombiID, nCount );
        return;
    }

    TxdSendCombiBufferInfo *pInfo;
    if ( !m_pCombiSendManage->GetCombiMem(nCombiID, pInfo) )
    {
        SendNotFoundCombiPackage( ASocket, ARemoteAddr, nCombiID, nCount );
        DebugAO( "需要查找的组合包已经丢弃");
        return;
    }

    byte nIndex;
    for ( byte i = 0; i != nCount; i++ )
    {
        nIndex = *(byte*)(ApRecvBuffer + 3 + i );
        if ( nIndex < pInfo->FCount )
        {
            _SendPackage( ASocket, ARemoteAddr, (char*)pInfo->FBuffers[nIndex], pInfo->FBufLens[nIndex], false );
        }
    }
    m_pCombiSendManage->ReleaseCombiMem( pInfo );
}

void CxdUdpIOHandle::DoRecvNotFoundCombiPackageCmd( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen )
{
    if ( NULL == m_pCombiRecvManage )
    {
        DebugAO( "数据接收对象出错" );
        return;
    }
    WORD nCombiID = *(WORD*)ApRecvBuffer;
    byte nCombiCount = *(byte*)(ApRecvBuffer + 2);

    TxdRecvCombiBufferInfo *pInfo;
    if ( m_pCombiRecvManage->GetCombiMem(ARemoteAddr, nCombiID, nCombiCount, pInfo, false) )
    {
        pInfo->FLastActiveTime = ReleaseCombiBuffer;
        m_pCombiRecvManage->ReleaseCombiMem( pInfo );
    }
}

void CxdUdpIOHandle::SendNotFoundCombiPackage( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, WORD ACombiID, byte ACombiCount )
{
    char buf[3];
    *(WORD*)buf = ACombiID;
    *(byte*)(buf + 2) = ACombiCount;
    SendSinglePackage( ASocket, ARemoteAddr, buf, 3, CtIoCmd_NotFoundCombiPackage );
}


void CxdUdpIOHandle::DoHandleCombiPackage( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, 
    int ABufLen, WORD ACombiID, byte ACombiCount, byte ACombiIndex )
{
    if ( ACombiIndex >= ACombiCount )
    {
        DebugAO( "接收到组合包信息出错" );
        return;
    }
    CreateCombiRecvManage();

    TxdRecvCombiBufferInfo *pInfo;
    m_pCombiRecvManage->GetCombiMem( ARemoteAddr, ACombiID, ACombiCount, pInfo, true );
    if ( pInfo->FBufLens[ACombiIndex] != 0 ) 
    {
        m_pCombiRecvManage->ReleaseCombiMem( pInfo );
        return;
    }

    pInfo->FScoket = ASocket;
    pInfo->FCurRecvCount += 1;

    if ( pInfo->FCount == pInfo->FCurRecvCount )
    {
        //完成组合包
        char *pCombiBuffer;
        int nCombiBufferLenght = 0;
        m_pCombiBufferManage->GetFixedMem( (LPVOID&)pCombiBuffer, CtMaxCombiPackageBufferSize);

        for ( byte i = 0; i != pInfo->FCount; i++ )
        {
            if ( i == ACombiIndex )
            {
                memcpy( pCombiBuffer + nCombiBufferLenght, ApRecvBuffer, ABufLen );
                nCombiBufferLenght += ABufLen;
            }
            else
            {
                memcpy( pCombiBuffer + nCombiBufferLenght, (char*)pInfo->FBuffers[i], pInfo->FBufLens[i] );
                nCombiBufferLenght += pInfo->FBufLens[i];
            }
        }

        //完成
        pInfo->FLastActiveTime = ReleaseCombiBuffer;
        m_pCombiRecvManage->ReleaseCombiMem( pInfo );

        OnRecvBuffer(ASocket, ARemoteAddr, pCombiBuffer, nCombiBufferLenght );
        m_pCombiBufferManage->FreeMem( (LPVOID)pCombiBuffer );
    }
    else
    {
        //缓存数据
        LPVOID vPointer;
        m_pCombiBufferManage->GetFixedMem(vPointer, CtPerCombiPackageBufferSize );
        char *pRecvBuffer = (char*)vPointer;
        memcpy( pRecvBuffer, ApRecvBuffer, ABufLen );
        pInfo->FBuffers[ACombiIndex] = (LPVOID)pRecvBuffer;
        pInfo->FBufLens[ACombiIndex] = ABufLen;
        m_pCombiRecvManage->ReleaseCombiMem( pInfo );
    }
}

void CxdUdpIOHandle::DoThreadHandleIO( LPVOID )
{
    InterlockedAdd( m_HandleIORunningThreadCount );
    TxdRecvPackageInfo *pInfo;
    while ( !m_bWaitToClose )
    {
        while ( m_pRecvPackageList->Get( pInfo ) )
        {
            DoHandleUdpPackage( pInfo->FSocket, TxdSocketAddr(pInfo->FRemoteIP, pInfo->FRemotePort, false), pInfo->FBuffer, pInfo->FBufferLength );
            m_pRecvBufManage->FreeMem( pInfo );
        }
        Sleep(5);
    }
    InterlockedDec( m_HandleIORunningThreadCount );
}

void CxdUdpIOHandle::__DoRecvBuffer( SOCKET ASocket )
{
    TxdRecvPackageInfo *pInfo;
    TxdSocketAddr addr;
    socklen_t nAddrlen;
    ssize_t nRecvLen;
    while ( true )
    {
        nAddrlen = sizeof(sockaddr_in);
        m_pRecvBufManage->GetFixedMem( (LPVOID&)pInfo, sizeof(TxdRecvPackageInfo) );
        nRecvLen = recvfrom( ASocket, pInfo->FBuffer, CtUdpBufferMaxSize, 0, (sockaddr*)&addr, &nAddrlen );
        if ( (SOCKET_ERROR == nRecvLen) || (xdGetSocketError() == EWOULDBLOCK) || 0 == nRecvLen )
        {
            m_pRecvBufManage->FreeMem( pInfo );
            return;
        }
        pInfo->FBufferLength = nRecvLen;
        pInfo->FRemoteIP = addr.sin_addr.s_addr;
        pInfo->FRemotePort = addr.sin_port;
        pInfo->FSocket = ASocket;
        InterlockedAdd( m_RecvPackageCount );

#ifdef CountBufferLength
        InterlockedAddVar( m_RecvBufferLengthByte, nRecvLen );
        if ( m_RecvBufferLengthByte >= CtMB )
        {
            AutoMutex( m_mtState );
            if ( m_RecvBufferLengthByte >= CtMB )
            {
                InterlockedAdd( m_RecvBufferLengthMB );
                InterlockedDecVar( m_RecvBufferLengthByte, CtMB );
            }
        }
#endif

        if ( !m_pRecvPackageList->Put(pInfo) )
        {
            m_pRecvBufManage->FreeMem( pInfo );
            InterlockedAdd( m_AutoThrowRecvPackage );
        }
    }  
}

int CxdUdpIOHandle::__SendBuffer( SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, const char* ApBuffer, int ABufferLen )
{
    int nSendResult = sendto( ASocket, ApBuffer, ABufferLen, 0, (sockaddr*)&ARemoteAddr, sizeof(sockaddr_in) );
    if ( ABufferLen == nSendResult )
    {
        InterlockedAdd( m_SendPackageCount );

#ifdef CountBufferLength
        InterlockedAddVar( m_SendBufferLengthByte, ABufferLen );
        if ( m_SendBufferLengthByte >= CtMB )
        {
            AutoMutex( m_mtState );
            if ( m_SendBufferLengthByte >= CtMB )
            {
                InterlockedAdd( m_SendBufferLengthMB );
                InterlockedDecVar( m_SendBufferLengthByte, CtMB );
            }
        }
#endif
    }
    else
    {
        InterlockedAdd( m_SendFailedPackageCount );
    }   
    return nSendResult;
}

void CxdUdpIOHandle::CreateCombiBufferManage( void )
{
    if ( NULL == m_pCombiBufferManage )
    {
        AutoMutex( m_mtInitCombiObject );
        if ( NULL == m_pCombiBufferManage )
        {
            m_pCombiBufferManage = new CxdFixedMemoryManageEx;
            m_pCombiBufferManage->AddFixedAllocInfo( CtMaxCombiPackageBufferSize, GetInitCombiPackageBufferCount() );
            m_pCombiBufferManage->AddFixedAllocInfo( CtUdpBufferMaxSize, GetSendCombiInitBufferCount() );
            m_pCombiBufferManage->AddFixedAllocInfo( CtPerCombiPackageBufferSize, GetRecvCombiInitBufferCount() );
        }
    }
}

void CxdUdpIOHandle::CreateCombiSendManage( void )
{
    CreateCombiBufferManage();
    if ( NULL == m_pCombiSendManage )
    {
        AutoMutex( m_mtInitCombiObject );
        if ( NULL == m_pCombiSendManage )
        {
            m_pCombiSendManage = new CxdCombiSend;
            m_pCombiSendManage->SetCombiBufferInitCount( GetSendCombiInitBufferCount() );
            m_pCombiSendManage->SetHashTableCount( GetSendCombiHashTableCount() );
            m_pCombiSendManage->SetCombiBufferExsitsTime( GetSendCombiBufferExsitsTime() );
            m_pCombiSendManage->SetMemUnActiveTimeoutSpace( GetUnUsedMemTimeoutSpace() );
            m_pCombiSendManage->SetItFreeCombiBuffer( NewInterfaceByClass(CxdUdpIOHandle, LPVOID, DoFreeCombiMem) );
            m_pCombiSendManage->SetActive( true );
        }
    }
}

void CxdUdpIOHandle::CreateCombiRecvManage( void )
{
    CreateCombiBufferManage();
    if ( NULL == m_pCombiRecvManage )
    {
        AutoMutex( m_mtInitCombiObject );
        if ( NULL == m_pCombiRecvManage )
        {
            m_pCombiRecvManage = new CxdCombiRecv;
            m_pCombiRecvManage->SetCombiBufferInitCount( GetRecvCombiInitBufferCount() );
            m_pCombiRecvManage->SetHashTableCount( GetRecvCombiHashTableCount() );
            m_pCombiRecvManage->SetCombiBufferExsitsTime( GetRecvCombiBufferExsitsTime() );
            m_pCombiRecvManage->SetMemUnActiveTimeoutSpace( GetUnUsedMemTimeoutSpace() );
            m_pCombiRecvManage->SetMaxTimeoutCount( GetRecvCombiMaxTimeoutCount() );
            m_pCombiRecvManage->SetItFreeCombiBuffer( NewInterfaceByClass(CxdUdpIOHandle, LPVOID, DoFreeCombiMem) );
            m_pCombiRecvManage->SetItGetLostCombiBuffer( NewInterfaceByClass(CxdUdpIOHandle, TxdRecvCombiBufferInfo*, DoGetLostCombiBuffer) );
            m_pCombiRecvManage->SetItRecvCombiBufferError( NewInterfaceByClass(CxdUdpIOHandle, TxdRecvCombiBufferInfo*, DoRecvCombiBufferError) );
            m_pCombiRecvManage->SetActive( true );
        }
    }
}

void CxdUdpIOHandle::DoFreeCombiMem( LPVOID AMem )
{
    m_pCombiBufferManage->FreeMem( AMem );
}

void CxdUdpIOHandle::DoGetLostCombiBuffer( TxdRecvCombiBufferInfo* ApInfo )
{
    //组合包ID(2) - 丢包数量(n) - Package1,Package2...PackageN
    char buf[CtUdpBufferMaxSize] = {0};
    *(WORD*)buf  = ApInfo->FCombiID;
    byte nCount(0);
    WORD nLen(3);
    for ( byte i = 0; i != ApInfo->FCount; i++ )
    {
        if ( ApInfo->FBufLens[i] == 0 )
        {
            *(byte*)(buf + nLen) = i;
            nCount++;
            nLen++;
        }
    }
    if ( nCount == 0 )
    {
        return;
    }
    *(byte*)(buf + 2) = nCount;
    SendSinglePackage( ApInfo->FScoket, TxdSocketAddr(ApInfo->FRemoteIP, ApInfo->FRemotePort, false), buf, nLen, CtIoCmd_GetCombiPackage );
}

void CxdUdpIOHandle::DoRecvCombiBufferError( TxdRecvCombiBufferInfo* ApInfo )
{
    InterlockedAdd( m_LostCombiPackageCount );
}

void CxdUdpIOHandle::InitForServer( void )
{
    m_RecvQueueMaxCount = 2048;
    m_HandleIoThreadCount = 3;
    m_UnUsedMemTimeoutSpace = 3600; //一个小时

    m_InitCombiPackageBufferCount = 512;

    m_SendCombiInitBufferCount = 300;
    m_SendCombiHashTableCount = 9999;
    m_SendCombiBufferExsitsTime = 3; //5s

    m_RecvCombiInitBufferCount = 3000;
    m_RecvCombiHashTableCount = 99999;
    m_RecvCombiBufferExsitsTime = 1; //1s
    m_RecvCombiMaxTimeoutCount = 2;

    SetBlockSocketOpt( false );
    SetReUseAddr( false );
    SetAutoIncPort( false );
    SetSysSendBufferSize( 1024 * 1024 * 8 );
    SetSysRecvBufferSize( 1024 * 1024 * 8 );
}

void CxdUdpIOHandle::InitForClient( void )
{
    m_RecvQueueMaxCount = 128;
    m_HandleIoThreadCount = 1;
    m_UnUsedMemTimeoutSpace = 600; //一个小时

    m_InitCombiPackageBufferCount = 100;

    m_SendCombiInitBufferCount = 100;
    m_SendCombiHashTableCount = 99;
    m_SendCombiBufferExsitsTime = 3; //5s

    m_RecvCombiInitBufferCount = 100;
    m_RecvCombiHashTableCount = 99;
    m_RecvCombiBufferExsitsTime = 1; //1s
    m_RecvCombiMaxTimeoutCount = 3;

    SetBlockSocketOpt( false );
    SetReUseAddr( true );
    SetAutoIncPort( true );
    SetSysSendBufferSize( 1024 * 1024 );
    SetSysRecvBufferSize( 1024 * 1024 );
}

void CxdUdpIOHandle::DoSocketClosed( SOCKET s )
{
    GetItUdpRecv()->UnRelateSocket( s );
}
