#include "stdafx.h"
#include "JxdDataStruct.h"
#include <string.h>
#include "JxdMem.h"

//CxdWordManage
CxdWordTable::CxdWordTable()
{
    for (int i = 0; i != MAXWORD + 1; i++)
    {
        m_aWordIDTable[i] = i;
    }
    m_nHead = m_nTail = 0;
    m_Count = MAXWORD + 1;
    m_nCapacity = MAXWORD + 1;
    m_CheckReclaimID = true;
}

CxdWordTable::~CxdWordTable()
{

}

bool CxdWordTable::GetWordID( WORD &AID )
{
    if ( m_Count != 0 )
    {
        AID = m_aWordIDTable[m_nHead];
        m_nHead = (m_nHead + 1) % m_nCapacity;
        m_Count--;
        return true;
    }
    return false;
}

bool CxdWordTable::ReclaimWordID( const WORD &AID )
{
    if ( m_Count < m_nCapacity )
    {
        if ( m_CheckReclaimID )
        {
            //判断是否是回收ID, 无存在于有效队列中
            UINT nCount = m_Count;
            UINT nIndex( m_nHead );
            while ( nCount != 0 )
            {
                --m_Count;
                if ( m_aWordIDTable[nIndex] == AID )
                {
                    return false;
                }
                nIndex = (nIndex + 1) % m_nCapacity;
            }
        }
        //回收
        m_aWordIDTable[m_nTail] = AID;
        m_nTail = (m_nTail + 1) % m_nCapacity;
        m_Count++;
        return true;
    }
    return false;
}

//CxdHashArray
CxdHashBasic::CxdHashBasic()
{
    m_bActive = false;
    m_HashTableCount = 999;
    m_pHashTable = NULL;
    m_pHashNodeManage = NULL;
    m_nInitNodeCount = 10000;
    m_bCreateMemManage = true;
    m_Count = 0;
}

CxdHashBasic::~CxdHashBasic()
{
    SetActive( false );
}

bool CxdHashBasic::ActiveHashArray()
{
    m_Count = 0;
    m_bTableCountIsN2 = IsN2( m_HashTableCount );
    m_pHashTable = new LPVOID[m_HashTableCount];
    ZeroMemory( m_pHashTable, m_HashTableCount * sizeof(LPVOID) );
    if ( m_bCreateMemManage )
    {
        m_pHashNodeManage = new CxdFixedMemoryManage;
    }
    m_pHashNodeManage->AddAllocInfo( sizeof(TxdHashNode), m_nInitNodeCount );

    return true;
}

bool CxdHashBasic::UnActiveHashArray()
{
    m_bActive = false;
    if ( m_bCreateMemManage )
    {
        delete m_pHashNodeManage;
        m_pHashNodeManage = NULL;
    }
    
    delete []m_pHashTable;
    m_pHashTable = NULL;
    return true;
}

void CxdHashBasic::SetInitHashNodeMemCount( UINT ACount )
{
    m_nInitNodeCount = ACount;
    if( m_pHashNodeManage != NULL ) 
    {
        m_pHashNodeManage->AddAllocInfo( sizeof(TxdHashNode), m_nInitNodeCount );
    }
}

bool CxdHashBasic::LoopBegin()
{
    if ( m_Count == 0 )
    {
        return false;
    }
    m_nLoopIndex = 0;
    m_pLoopNode = (TxdHashNode*)m_pHashTable[m_nLoopIndex];
    while ( m_pLoopNode == NULL )
    {
        m_nLoopIndex++;
        if ( m_nLoopIndex == m_HashTableCount )
        {
            return false;
        }
        m_pLoopNode = (TxdHashNode*)m_pHashTable[m_nLoopIndex];
    }
    m_pParent = NULL;
    m_pOldParent = NULL;
    m_nLoopCount = m_Count;
    return true;
}

bool CxdHashBasic::LoopNext( UINT64 &AHashID, LPVOID &AValue )
{
    if ( m_nLoopCount == 0 )
    {
        return false;
    }

    if ( m_pLoopNode == NULL )
    {
        m_pLoopNode = (TxdHashNode*)m_pHashTable[++m_nLoopIndex];
        while ( m_pLoopNode == NULL )
        {
            m_pLoopNode = (TxdHashNode*)m_pHashTable[++m_nLoopIndex];
        }
        m_pParent = NULL;
        m_pOldParent = NULL;
        return LoopNext( AHashID, AValue );
    }

    m_pOldParent = m_pParent;
    m_pParent = m_pLoopNode;
    AValue = m_pLoopNode->HashData;
    AHashID = m_pLoopNode->HashID;
    m_pLoopNode = m_pLoopNode->NextNode;
    m_nLoopCount--;
    return true;
}

void CxdHashBasic::DeleteLoopItem( void )
{
    if ( m_pParent != NULL )
    {
        if ( m_pOldParent == NULL )
        {
            m_pHashTable[m_nLoopIndex] = (LPVOID)m_pLoopNode;
            m_pHashNodeManage->FreeMem( (LPVOID)m_pParent );
            m_Count--;
        }
        else
        {
            m_pOldParent->NextNode = m_pLoopNode;
            m_pHashNodeManage->FreeMem( (LPVOID)m_pParent );
            m_Count--;
        }
        m_pParent = m_pOldParent;
    }
}

bool CxdHashBasic::SetHashNodeMemManage( CxdFixedMemoryManage *ApMemManage )
{
    if ( m_bActive )
    {
        return false;
    }
    if ( Assigned(m_pHashNodeManage) && m_bCreateMemManage )
    {
        delete m_pHashNodeManage;
    }
    m_pHashNodeManage = ApMemManage;
    m_bCreateMemManage = IsNull( m_pHashNodeManage );
    return true;
}

int CxdHashBasic::GetHashNodeMemInfo( TxdGetMemInfo *&ApInfos, int ACount )
{
    return m_pHashNodeManage->GetMemInfo(ApInfos, ACount, sizeof(TxdHashNode) );
}

//CxdHashArray
bool CxdHashArray::_FindNode( const UINT64 &AID, LPVOID &AFindData, bool bDelete, bool bCreateOnNull, const LPVOID &ANewValueOnNotFind )
{
    UINT nIndex = GetIndexByID(AID);
    TxdHashNode *pNode = (TxdHashNode*)m_pHashTable[nIndex];
    TxdHashNode *pParent = NULL;
    while ( pNode != NULL )
    {
        if ( pNode->HashID == AID )
        {
            AFindData = pNode->HashData;
            if ( bDelete )
            {
                if (NULL == pParent) 
                {
                    m_pHashTable[nIndex] = (LPVOID)pNode->NextNode;	
                }
                else
                {
                    pParent->NextNode = pNode->NextNode;
                }
                if ( m_pHashNodeManage->FreeMem((LPVOID)pNode) )
                {
                    m_Count--;
                }
            }
            return !bCreateOnNull;
        }
        else
        {
            pParent = pNode;
            pNode = pNode->NextNode;
        }
    }
    //没有找到
    if ( bCreateOnNull )
    {
        TxdHashNode *pNewNode;
        m_pHashNodeManage->GetFixedMem( (LPVOID&)pNewNode, sizeof(TxdHashNode) );
        pNewNode->HashData = ANewValueOnNotFind;
        pNewNode->HashID = AID;
        pNewNode->NextNode = NULL;
        if ( NULL == pParent )
        { 
            m_pHashTable[nIndex] = (LPVOID)pNewNode;
        }
        else
        {
            pParent->NextNode = pNewNode;	
        }
        m_Count++;
        return true;
    }
    return false;
}

bool CxdHashArray::Add( const UINT64 &AHashID, const LPVOID &ApData )
{
    LPVOID vLPVOID;
    return _FindNode( AHashID, vLPVOID, false, true, ApData );
}

bool CxdHashArray::Find( const UINT64 &AHashID, LPVOID &ApFindData, bool bDelete )
{
    return _FindNode( AHashID, ApFindData, bDelete, false, 0 );
}

bool CxdHashArray::Modify( const UINT64 &AHashID, const LPVOID &ApNewData )
{
    TxdHashNode *pNode = (TxdHashNode*)m_pHashTable[ GetIndexByID(AHashID) ];
    while ( Assigned(pNode) )
    {
        if ( pNode->HashID == AHashID )
        {
            pNode->HashData = ApNewData;
            return true;
        }
        pNode = pNode->NextNode;
    }
    return false;
}

LPVOID CxdHashArray::Delete( const UINT64 &AHashID )
{
    LPVOID vLPVOID = NULL;
    _FindNode( AHashID, vLPVOID, true, false, 0 );
    return vLPVOID;
}

//CxdHashArrayEx

CxdHashArrayEx::CxdHashArrayEx(): CxdHashBasic()
{
    m_pFindNode = NULL;
    m_pParent = NULL;
    m_HashID = 0;
}

void CxdHashArrayEx::Add( const UINT64 &AHashID, const LPVOID &ApData )
{
    LPVOID vLPVOID;
    m_pHashNodeManage->GetFixedMem( vLPVOID, sizeof(TxdHashNode) );
    m_Count++;
    TxdHashNode *pNewNode = (TxdHashNode*)vLPVOID;
    pNewNode->HashData = ApData;
    pNewNode->HashID = AHashID;
    pNewNode->NextNode = NULL;

    UINT nIndex = GetIndexByID( AHashID );
    TxdHashNode *pNode = (TxdHashNode*)m_pHashTable[nIndex];
    if ( pNode == NULL )
    {
        m_pHashTable[nIndex] = (LPVOID)pNewNode;
    }
    else
    {
        while ( pNode->NextNode != NULL )
        {
            pNode = pNode->NextNode;
        }
        pNode->NextNode = pNewNode;
    }
}

bool CxdHashArrayEx::FindBegin( const UINT64 &AHashID, LPVOID &AFindValue )
{
    m_HashID = AHashID;
    m_nFindIndex = GetIndexByID( m_HashID );
    m_pFindNode = (TxdHashNode*)m_pHashTable[m_nFindIndex];
    m_pParent = NULL;
    m_pOldParent = NULL;
    return FindNext( AFindValue );
}

bool CxdHashArrayEx::FindNext( LPVOID &AFindValue )
{
    while ( m_pFindNode != NULL )
    {        
        m_pOldParent = m_pParent;
        m_pParent = m_pFindNode;
        if ( m_pFindNode->HashID == m_HashID )
        {
            AFindValue = m_pFindNode->HashData;
            m_pFindNode = m_pFindNode->NextNode;
            return true;
        }
        m_pFindNode = m_pFindNode->NextNode;
    }
    return false;
}

void CxdHashArrayEx::DeleteFindItem( void )
{
    if ( m_pParent != NULL && m_pParent->HashID == m_HashID )
    {
        if ( m_pOldParent == NULL )
        {
            m_pHashTable[m_nFindIndex] = (LPVOID)m_pFindNode;
            m_pHashNodeManage->FreeMem( (LPVOID)m_pParent );
            m_Count--;
        }
        else
        {
            m_pOldParent->NextNode = m_pFindNode;
            m_pHashNodeManage->FreeMem( (LPVOID)m_pParent );
            m_Count--;
        }
        m_pParent = m_pOldParent;
    }
}