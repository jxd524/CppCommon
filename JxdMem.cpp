#include "stdafx.h"
#include "JxdMem.h"
#ifdef linux
#include <stdlib.h>
#include <string.h>
#endif
#include "JxdStringHandle.h"

//#include "JxdDebugInfo.h"

//CxdFixedMemory
CxdFixedMemory::CxdFixedMemory( const UINT &ABlockSize, const UINT &ABlockCount ): m_oMemQueuy(ABlockCount)
{
    m_BlockSize = ABlockSize;
    m_Capacity = ABlockCount == 0 ? 1024 : ABlockCount;
    m_pMemoryAddress = malloc( m_BlockSize * m_Capacity );
    m_IsMallocOK = m_pMemoryAddress != NULL;
    if ( m_IsMallocOK )
    {
        m_nAddressMinValue = (UINT64)m_pMemoryAddress;
        m_nAddressMaxValue = m_nAddressMinValue + m_Capacity * m_BlockSize;
        for (UINT i = 0; i != m_Capacity; i++)
        {
            m_oMemQueuy.Put( LPVOID(m_nAddressMinValue + m_BlockSize * i) );
        }
    }
    m_bBlockSizeN2 = IsN2( m_BlockSize );
    m_LastAllMemWidthoutUsed = GetRunningSecond();
}

CxdFixedMemory::~CxdFixedMemory()
{
    free( m_pMemoryAddress );
}

//CxdFixedMemoryManage
CxdFixedMemoryManage::CxdFixedMemoryManage()
{
    m_UsedMemCount = 0;
    m_nMaxUsedMemCount = 0;
    m_DefaultBlockCount = 1024;
    m_pLastGetMemObject = NULL;
    m_pLastFreeMemObject = NULL;
}

CxdFixedMemoryManage::~CxdFixedMemoryManage()
{
    ClearMemory();
    TxdMemSizeInfo *pInfo;
    for( vector<TxdMemSizeInfo*>::iterator it = m_ltAllocInfo.begin(); it != m_ltAllocInfo.end(); it++ )
    {
        pInfo = *it;
        delete pInfo;
    }
    m_ltAllocInfo.clear();
}

void CxdFixedMemoryManage::ClearMemory()
{
    m_UsedMemCount = 0;
    m_pLastGetMemObject = NULL;
    m_pLastFreeMemObject = NULL;
    for ( vector<CxdFixedMemory*>::iterator it = m_ltMem.begin(); it != m_ltMem.end(); it++ )
    {
        CxdFixedMemory *pObj = *it;
        delete pObj;
    }
    m_ltMem.clear();
}

void CxdFixedMemoryManage::ClearUnUsedMemroy( const UINT &AUnUsedSpaceTime )
{
    INT64 nCurTime = GetRunningSecond();
    vector<CxdFixedMemory*>::iterator it = m_ltMem.begin();
    while ( it != m_ltMem.end() )
    {
        CxdFixedMemory *pObj = *it;
        if ( pObj->IsAllMemWidthoutUsed() && (nCurTime - pObj->GetLastAllMemWidthoutUsed() > AUnUsedSpaceTime) )
        {
            if ( pObj == m_pLastGetMemObject ) m_pLastGetMemObject = NULL;
            if ( pObj == m_pLastFreeMemObject ) m_pLastFreeMemObject = NULL;
            delete pObj;
            it = m_ltMem.erase( it );
        }
        else
        {
            it++;
        }
    }
}

#define GetMaxBlockMem(AddrValue, ASize, AInitCount)                                                        \
    /*判断最后一个获取对象*/                                                                                \
    if ( m_pLastGetMemObject != NULL ){                                                                     \
        if ( m_pLastGetMemObject->GetBlockSize() == ASize && m_pLastGetMemObject->GetMem(AddrValue) ){      \
            IncUsedMemCount();                                                                              \
            return true;                                                                                    \
        }                                                                                                   \
    }                                                                                                       \
    /*查找一次*/                                                                                            \
    CxdFixedMemory *pObj;                                                                                   \
    for ( vector<CxdFixedMemory*>::iterator it = m_ltMem.begin(); it != m_ltMem.end(); it++ ){    \
        pObj = *it;                                                                                         \
        if ( pObj->GetBlockSize() == ASize && pObj->GetMem(AddrValue) ){                                    \
            m_pLastGetMemObject = pObj;                                                                     \
            IncUsedMemCount();                                                                              \
            return true;                                                                                    \
        }                                                                                                   \
    }                                                                                                       \
    /*重新申请*/                                                                                            \
    pObj = new CxdFixedMemory( ASize, AInitCount );                                                         \
    if ( pObj->GetMem(AddrValue) ){                                                                         \
        m_ltMem.push_back( pObj );                                                                     \
        m_pLastGetMemObject = pObj;                                                                         \
        IncUsedMemCount();                                                                                  \
        return true;                                                                                        \
    }                                                                                                       \
    delete pObj;                                                                                            \
    return false;                                                                                           

bool CxdFixedMemoryManage::GetFixedMem( LPVOID &AddrValue, const UINT &ASize )
{
    GetMaxBlockMem( AddrValue, ASize, GetFixedBlockCount(ASize) );
}

bool CxdFixedMemoryManage::GetValMem( LPVOID &AddrValue, const UINT &ASize, UINT &AMaxSize )
{
    UINT AInitCount = GetValBlockCount( ASize, AMaxSize );
    GetMaxBlockMem( AddrValue, AMaxSize, AInitCount );
}

bool CxdFixedMemoryManage::GetMem( LPVOID &AddrValue, const UINT &ASize )
{
    UINT nMaxSize;
    UINT AInitCount = GetValBlockCount( ASize, nMaxSize );
    GetMaxBlockMem( AddrValue, nMaxSize, AInitCount );
}

bool CxdFixedMemoryManage::FreeMem( const LPVOID &AddrValue )
{
    if ( m_pLastFreeMemObject != NULL && m_pLastFreeMemObject->IsEffective(AddrValue) )
    {
        m_pLastFreeMemObject->FreeMem( AddrValue );
        m_UsedMemCount--;
        return true;
    }
    CxdFixedMemory *pObj;    
    for ( vector<CxdFixedMemory*>::iterator it = m_ltMem.begin(); it != m_ltMem.end(); it++ )
    {
        pObj = *it;
        if ( pObj != m_pLastFreeMemObject && pObj->IsEffective(AddrValue) )
        {            
            pObj->FreeMem(AddrValue);
            m_pLastFreeMemObject = pObj;
            m_UsedMemCount--;
            return true;
        }
    }
    return false;
}

bool CxdFixedMemoryManage::AddAllocInfo( UINT AMinBlockSize, UINT AMaxBlockSize, UINT AInitCount )
{
    if ( AMaxBlockSize == 0 || AMinBlockSize > AMaxBlockSize )
    {
        return false;
    }

    TxdMemSizeInfo *pInfo;
    for( vector<TxdMemSizeInfo*>::iterator it = m_ltAllocInfo.begin(); it != m_ltAllocInfo.end(); it++ )
    {
        pInfo = *it;
        if ( pInfo->MinBlockSize == AMinBlockSize && AMaxBlockSize == pInfo->MaxBlockSize)
        {
            if ( AInitCount > pInfo->BlockCount )
            {
                pInfo->BlockCount = AInitCount;
            }
            return true;
        }
    }

    m_ltAllocInfo.push_back( new TxdMemSizeInfo( AMinBlockSize, AMaxBlockSize, AInitCount == 0 ? m_DefaultBlockCount : AInitCount) );
    return true;
}

void CxdFixedMemoryManage::InitMem( UINT AMinBlockSize, UINT AMaxBlockSize, UINT ACount )
{
    if ( !AddAllocInfo(AMinBlockSize, AMaxBlockSize, ACount) )
    {
        return;
    }
    CxdFixedMemory *pObj = NULL;
    bool bCreateNew = true;
    UINT nAllCount = 0;
    for ( vector<CxdFixedMemory*>::iterator it = m_ltMem.begin(); it != m_ltMem.end(); it++ )
    {
        pObj = *it;
        if ( pObj->GetBlockSize() == AMaxBlockSize )
        {
            nAllCount += pObj->GetCount();
            if ( nAllCount >= ACount )
            {
                bCreateNew = false;
                break;
            }
        }
    }
    if ( bCreateNew )
    {
        pObj = new CxdFixedMemory( AMaxBlockSize, ACount == 0 ? m_DefaultBlockCount : ACount );
        m_ltMem.push_back( pObj );
    }
}

void CxdFixedMemoryManage::InitMem( void )
{
    TxdMemSizeInfo *pInfo;
    for( vector<TxdMemSizeInfo*>::iterator it = m_ltAllocInfo.begin(); it != m_ltAllocInfo.end(); it++ )
    {
        pInfo = *it;
        InitMem( pInfo->MinBlockSize, pInfo->MaxBlockSize, pInfo->BlockCount );
    }
}

int CxdFixedMemoryManage::GetMemInfo( TxdGetMemInfo *&ApInfos, int ACount, UINT AFindBlockSize )
{
    int nCount = GetMemObjectCount();
    if ( AFindBlockSize > 0 )
    {
        nCount = 0;
        CxdFixedMemory *pObj = NULL;
        for ( vector<CxdFixedMemory*>::iterator it = m_ltMem.begin(); it != m_ltMem.end(); it++ )
        {
            pObj = *it;
            if ( pObj->GetBlockSize() == AFindBlockSize )
            {
                nCount = 1;
                break;
            }
        }
    }

    if ( nCount == 0 ) return 0;

    if ( ACount < nCount )
    {
        FreeMemInfo(ApInfos);
        ApInfos = new TxdGetMemInfo[nCount];
    }

    memset( ApInfos, 0, sizeof(TxdGetMemInfo) * nCount );
    int nIndex(0);
    bool bAdd;
    CxdFixedMemory *pObj = NULL;
    for ( vector<CxdFixedMemory*>::iterator it = m_ltMem.begin(); it != m_ltMem.end(); it++ )
    {
        pObj = *it;
        bAdd = true;
        if ( AFindBlockSize > 0 && AFindBlockSize != pObj->GetBlockSize() )
        {
            continue;
        }
        for ( int j = 0; j < nIndex; j++ )
        {
            if ( ApInfos[j].BlockSize == pObj->GetBlockSize() )
            {
                ApInfos[j].TotalCount += pObj->GetCapacity();
                ApInfos[j].UsedCount += pObj->GetUsedCount();
                ApInfos[j].ObjectCount++;
                bAdd = false;
                break;
            }
        }
        if ( bAdd )
        {
            ApInfos[nIndex].BlockSize = pObj->GetBlockSize();
            ApInfos[nIndex].UsedCount = pObj->GetUsedCount();
            ApInfos[nIndex].TotalCount = pObj->GetCapacity();
            ApInfos[nIndex].ObjectCount = 1;
            nIndex++;
        }
    }
    return nIndex;
}

void CxdFixedMemoryManage::FreeMemInfo( TxdGetMemInfo *&ApInfos )
{
    delete []ApInfos;
}

void CxdFixedMemoryManage::IncUsedMemCount()
{
    m_UsedMemCount++;
    if ( m_UsedMemCount > m_nMaxUsedMemCount )
    {
        m_nMaxUsedMemCount = m_UsedMemCount;
    }
}

UINT CxdFixedMemoryManage::GetFixedBlockCount( const UINT &ASize )
{
    UINT nResult(0);
    TxdMemSizeInfo *pInfo;
    for( vector<TxdMemSizeInfo*>::iterator it = m_ltAllocInfo.begin(); it != m_ltAllocInfo.end(); it++ )
    {
        pInfo = *it;
        if ( pInfo->IsFixedMem && (pInfo->MaxBlockSize == ASize) )
        {
            nResult = pInfo->BlockCount;
            break;
        }
    }
    if ( 0 == nResult ) nResult = m_DefaultBlockCount;

    return nResult;
}

UINT CxdFixedMemoryManage::GetValBlockCount( const UINT &ASize, UINT &AMaxSize )
{
    AMaxSize = 0;
    UINT nResult(0);
    TxdMemSizeInfo *pInfo;
    for( vector<TxdMemSizeInfo*>::iterator it = m_ltAllocInfo.begin(); it != m_ltAllocInfo.end(); it++ )
    {
        pInfo = *it;
        if ( !pInfo->IsFixedMem && (ASize >= pInfo->MinBlockSize) && (ASize <= pInfo->MaxBlockSize) )
        {
            nResult = pInfo->BlockCount;
            AMaxSize = pInfo->MaxBlockSize;
            break;
        }
    }
    if ( 0 == nResult ) nResult = m_DefaultBlockCount;
    if ( 0 == AMaxSize ) AMaxSize = ASize;

    return nResult;
}

int CxdFixedMemoryManage::GetAllMemWidthoutUsedCount( void )
{
    int nCount = 0;
    for ( vector<CxdFixedMemory*>::iterator it = m_ltMem.begin(); it != m_ltMem.end(); it++ )
    {
        CxdFixedMemory *pObj = *it;
        if ( pObj->IsAllMemWidthoutUsed() )
        {
            nCount++;
        }
    }
    return nCount;
}

bool CxdFixedMemoryManage::IsEmpty( void )
{
    return m_ltMem.empty();
}

bool CxdFixedMemoryManage::AddFixedAllocInfo( UINT ABlockSize, UINT ABlockCount /*= 0*/ )
{
    return AddAllocInfo( ABlockSize, ABlockSize, ABlockCount );
}

void CxdMemManageHelper::DebugMemInfo( const char *ApMemName )
{
    int nCount = 0;
    TxdGetMemInfo *pInfo = NULL;
    m_oMemManage.GetMemInfo( pInfo, nCount );
    for ( int i = 0; i != nCount; i++ )
    {
        //Debug( "%s mem info: BlockSize=%d, ObjectCount=%d, TotalCount=%d, UsedCount=%d", 
        //    ApMemName, pInfo[i].BlockSize, pInfo[i].ObjectCount, pInfo[i].TotalCount, pInfo[i].UsedCount );
    }
    m_oMemManage.FreeMemInfo( pInfo );
}

string FormatMemInfo( TxdGetMemInfo *ApInfo, int ACount )
{
    string strInfo = Format( "all mem info count: %d", ACount );
    for ( int i = 0; i != ACount; i++ )
    {
        strInfo += Format("\r\n        %02d: BlockSize=%d\t ObjectCount=%d\t TotalCount=%d\t UsedCount=%d\t", 
            i + 1, ApInfo[i].BlockSize, ApInfo[i].ObjectCount, ApInfo[i].TotalCount, ApInfo[i].UsedCount );
    }
    return strInfo;
}

//void CxdFixedMemoryManageEx::InitMem( const UINT &ASize, const UINT &ACount )
//{
//    m_mutex.Lock();
//    CxdFixedMemoryManage::InitMem( ASize, ACount );
//    m_mutex.Unlock();
//}


bool CxdFixedMemoryManageEx::FreeMem( const LPVOID &APointer )
{
    m_mutex.Lock();
    bool b = CxdFixedMemoryManage::FreeMem( APointer );
    m_mutex.Unlock();
    return b;
}

bool CxdFixedMemoryManageEx::GetFixedMem( LPVOID &AddrValue, const UINT &ASize )
{
    m_mutex.Lock();
    bool b = CxdFixedMemoryManage::GetFixedMem( AddrValue, ASize );
    m_mutex.Unlock();
    return b;
}

bool CxdFixedMemoryManageEx::GetValMem( LPVOID &AddrValue, const UINT &ASize, UINT &AMaxSize )
{
    m_mutex.Lock();
    bool b = CxdFixedMemoryManage::GetValMem( AddrValue, ASize, AMaxSize );
    m_mutex.Unlock();
    return b;
}

bool CxdFixedMemoryManageEx::GetMem( LPVOID &AddrValue, const UINT &ASize )
{
    m_mutex.Lock();
    bool b = CxdFixedMemoryManage::GetMem( AddrValue, ASize );
    m_mutex.Unlock();
    return b;
}

void CxdFixedMemoryManageEx::ClearMemory()
{
    m_mutex.Lock();
    CxdFixedMemoryManage::ClearMemory();
    m_mutex.Unlock();
}

void CxdFixedMemoryManageEx::ClearUnUsedMemroy( const UINT &AUnUsedSpaceTime )
{
    m_mutex.Lock();
    CxdFixedMemoryManage::ClearUnUsedMemroy( AUnUsedSpaceTime );
    m_mutex.Unlock();
}
