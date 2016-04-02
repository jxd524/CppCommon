#include "stdafx.h"
#include "JxdCombiBuffer.h"

//CxdCombiBufferBasic
CxdCombiBasic::CxdCombiBasic(const int &ABufferSize): m_CtBufferSize(ABufferSize)
{
    m_CombiBufferInitCount = 512;
    m_HashTableCount = 999;
    m_pMemManage = NULL;
    m_pHashTable = NULL;
    m_CombiBufferExsitsTime = 3;
    m_MemUnActiveTimeoutSpace = 60 * 60 * 12; //12小时
    m_pThread = NULL;
    InitInterface(FreeCombiBuffer);
}

CxdCombiBasic::~CxdCombiBasic()
{
    SetActive( false );
    DeleteInterface(FreeCombiBuffer);
}

bool CxdCombiBasic::ActiveManage( void )
{
    m_nTestClearMem = 0;
    m_pMemManage = new CxdFixedMemoryManage;
    m_pMemManage->AddAllocInfo( m_CtBufferSize, m_CtBufferSize, GetCombiBufferInitCount() );
    m_pMemManage->InitMem();

    m_pHashTable = CreateHashTable();
    m_pHashTable->SetInitHashNodeMemCount( GetCombiBufferInitCount() );
    m_pHashTable->SetHashTableCount( GetHashTableCount() );
    m_pHashTable->SetActive( true );
    return true;
}

bool CxdCombiBasic::UnActiveManage( void )
{
    AutoMutex( m_lock );
    FreeThread();
    SafeDeleteObject( m_pMemManage );
    SafeDeleteObject( m_pHashTable );
    return true;
}

void CxdCombiBasic::DoThreadCheckMem( int AType )
{
    AutoMutex( m_lock );
    if ( 0 == m_pThread->GetThreadParam() )
    {
        //检查节点
        UINT64 nHashID;
        UINT64 nCurTime = GetRunningSecond();
        TxdCombiBufferInfo *pInfo;

        //检查已经完成的(接收端), 或已经超时的缓存数据
        bool bOK = m_pHashTable->LoopBegin() && m_pHashTable->LoopNext( nHashID, (LPVOID&)pInfo );
        while ( bOK )
        {
            if ( ReleaseCombiBuffer == pInfo->FLastActiveTime )
            {
                DoReleaseCombiBuffer( pInfo );
                m_pHashTable->DeleteLoopItem();
                m_pMemManage->FreeMem( pInfo );
            }
            else if ( nCurTime - pInfo->FLastActiveTime > GetCombiBufferExsitsTime() )
            {
                if ( DoCombiBufferTimeout(pInfo) )
                {
                    DoReleaseCombiBuffer( pInfo );
                    m_pHashTable->DeleteLoopItem();
                    m_pMemManage->FreeMem( pInfo );
                }
            }
            bOK = m_pHashTable->LoopNext( nHashID, (LPVOID&)pInfo );
        }

        //下次循环条件设置
        if ( m_pHashTable->IsEmpty() )
        {
            m_nTestClearMem = 0;
            m_pThread->SetThreadParam( 1 );
            m_pThread->SetLoopExecuteSpaceTime( GetMemUnActiveTimeoutSpace() * 1000 );
        }
        else if ( ++m_nTestClearMem == 100 )
        {
            m_nTestClearMem = 0;
            m_pMemManage->ClearUnUsedMemroy( GetMemUnActiveTimeoutSpace() );
            if ( m_pMemManage->IsEmpty() )
            {
                FreeThread();
            }
        }
    }
    else if ( 1 == m_pThread->GetThreadParam() )
    {
        //只检查大块缓存
        m_pMemManage->ClearUnUsedMemroy( GetMemUnActiveTimeoutSpace() );
        if ( m_pMemManage->IsEmpty() )
        {
            FreeThread();
        }
    }
}

void CxdCombiBasic::ReleaseCombiMem( TxdCombiBufferInfo* ApMem )
{
    if ( ApMem->FLastActiveTime != ReleaseCombiBuffer )
    {
        ApMem->FLastActiveTime = GetRunningSecond();
    }
    CheckThreadForCombiBuffer();
    m_lock.Unlock();
}

void CxdCombiBasic::FreeThread( void )
{
    if ( m_pThread != NULL )
    {
        m_pThread->SetAutoFreeThreadObject( true );
        m_pThread->SetLoopExecuteSpaceTime( 0 );
        m_pThread->PostEvent();
        m_pThread = NULL;
    }
}

void CxdCombiBasic::CheckThreadForCombiBuffer( void )
{
    int nSpaceTime = GetCombiBufferExsitsTime() * 1000;
    if ( NULL == m_pThread )
    {
        m_pThread = CreateLoopExecuteThread( CxdCombiBasic, int, DoThreadCheckMem, 0, nSpaceTime );
    }
    if ( m_pThread->GetLoopExecuteSpaceTime() != nSpaceTime )
    {
        m_pThread->SetThreadParam( 0 );
        m_pThread->SetLoopExecuteSpaceTime( nSpaceTime );
        m_pThread->PostEvent();
    }
}

void CxdCombiBasic::DoReleaseCombiBuffer( TxdCombiBufferInfo *ApInfo )
{
    for ( byte i = 0; i != CtMaxCombiPackageCount; i++ )
    {
        if ( ApInfo->FBufLens[i] > 0 )
        {
            NotifyInterface( FreeCombiBuffer, ApInfo->FBuffers[i] );
            ApInfo->FBufLens[i] = 0;
        }
    }
}


//CxdCombiSend
bool CxdCombiSend::GetCombiMem( int ACombiID, TxdSendCombiBufferInfo *&ApInfo )
{
    CxdHashArray *pHash = dynamic_cast<CxdHashArray*>( m_pHashTable );
    m_lock.Lock();
    if ( ACombiID < 0 )
    {
        WORD wID;
        if ( !m_pWordTable->GetWordID(wID) )
        {
            m_lock.Unlock();
            return false;
        }
        m_pMemManage->GetFixedMem( (LPVOID&)ApInfo, m_CtBufferSize );
        memset( ApInfo->FBufLens, 0, sizeof(ApInfo->FBufLens) );
        ApInfo->FCombiID = wID;
        ApInfo->FCount = 0;
        ApInfo->FLastActiveTime = 0;
        return pHash->Add( wID, ApInfo );
    }
    else
    {
        if ( pHash->Find( ACombiID, (LPVOID&)ApInfo, false ) )
        {
            return true;
        }
        m_lock.Unlock();
        return false;
    }
}

bool CxdCombiSend::ActiveManage( void )
{
    if ( !CxdCombiBasic::ActiveManage() )
    {
        return false;
    }
    m_pWordTable = new CxdWordTable;
    return true;
}

bool CxdCombiSend::UnActiveManage( void )
{
    CxdCombiBasic::UnActiveManage();
    SafeDeleteObject( m_pWordTable );
    return true;
}

//CxdCombiRecv
CxdCombiRecv::CxdCombiRecv(): CxdCombiBasic(CtRecvCombiBufferInfoSize)
{
    m_MaxTimeoutCount = 3;
    InitInterface( GetLostCombiBuffer );
    InitInterface( RecvCombiBufferError );
}

CxdCombiRecv::~CxdCombiRecv()
{
    DeleteInterface( GetLostCombiBuffer );
    DeleteInterface( RecvCombiBufferError );
}

bool CxdCombiRecv::GetCombiMem( const TxdSocketAddr &AAddr, WORD ACombiID, byte ACombiCount, 
    TxdRecvCombiBufferInfo *&ApInfo, bool AbCreateOnNull )
{
    CxdHashArrayEx *pHash = dynamic_cast<CxdHashArrayEx*>( m_pHashTable );
    m_lock.Lock();
    bool bOK = pHash->FindBegin( ACombiID, (LPVOID&)ApInfo );
    while ( bOK )
    {
        if ( ApInfo->FRemoteIP == AAddr.sin_addr.s_addr && ApInfo->FRemotePort == AAddr.sin_port && 
             ApInfo->FCombiID == ACombiID && ApInfo->FCount == ACombiCount )
        {
            ApInfo->FTimeoutCount = 0;
            return true;
        }
        bOK = pHash->FindNext( (LPVOID&)ApInfo );
    }
    if ( !AbCreateOnNull )
    {
        m_lock.Unlock();
        return false;
    }
    m_pMemManage->GetFixedMem( (LPVOID&)ApInfo, m_CtBufferSize );
    memset( ApInfo->FBufLens, 0, sizeof(ApInfo->FBufLens) );
    ApInfo->FCombiID = ACombiID;
    ApInfo->FCount = ACombiCount;
    ApInfo->FRemoteIP = AAddr.sin_addr.s_addr;
    ApInfo->FRemotePort = AAddr.sin_port;
    ApInfo->FTimeoutCount = 0;
    ApInfo->FCurRecvCount = 0;
    pHash->Add( ACombiID, ApInfo );
    return true;
}

bool CxdCombiRecv::DoCombiBufferTimeout( TxdCombiBufferInfo *ApInfo )
{
    TxdRecvCombiBufferInfo *pInfo = (TxdRecvCombiBufferInfo*)ApInfo;

    //已经完成的组合包
    if ( pInfo->FCount == pInfo->FCurRecvCount ) return true;

    if ( pInfo->FTimeoutCount >= GetMaxTimeoutCount() )
    {
        //失败的组合包
        NotifyInterface( RecvCombiBufferError, pInfo );
        return true;
    }
    else
    {
        //尝试重新找回丢失的数据
        NotifyInterface( GetLostCombiBuffer, pInfo );
        pInfo->FTimeoutCount++;
        pInfo->FLastActiveTime = GetRunningSecond();
        return false;
    }
}