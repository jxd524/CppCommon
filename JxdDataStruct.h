/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdDataStruct.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 14:47:01
	LastModify : 
	Description: 
**************************************************************************/
#pragma once
#ifndef _JxdDataStruct_Lib
#define _JxdDataStruct_Lib

#ifdef linux
#include "JxdTypedef.h"
#endif
#include "JxdMacro.h"
#include "JxdSynchro.h"
#include "JxdEvent.h"

class CxdFixedMemoryManage;
struct TxdGetMemInfo;

template<typename T>
class CxdQueue
{
public:
    typedef IxdNotifyEventExT<T&, bool&/*AbBreak*/> IxdQueueLoopInterface;
    typedef IxdNotifyEventEx3T<T&, T&, int&/*Result*/> IxdCompareInterface;

    CxdQueue(const UINT &ACapacity = 1024 * 8)
    {
        m_Capacity = ACapacity;
        m_Count = m_Head = m_Tail = 0;
        m_pAddress = new T[m_Capacity];
    }
    virtual ~CxdQueue()
    {
        delete []m_pAddress;
    }

    PropertyGetValue( Count, UINT );
    PropertyGetValue( Capacity, UINT );
    PropertyGetValue( Head, UINT );
    PropertyGetValue( Tail, UINT );

    inline bool IsEmpty(void) { return  0 == m_Count; }
    inline bool IsFull(void)  { return m_Count == m_Capacity; }

    inline bool Get(T &AValue)
    {
        if ( m_Count != 0 )
        {
            AValue = m_pAddress[m_Head];
            //if ( ++m_Head == m_Capacity ) m_Head = 0; 
            m_Head = (m_Head + 1) % m_Capacity;
            m_Count--;
            return true;
        }
        return false;
    }

    inline bool Put(const T &AValue)
    {
        if ( m_Count < m_Capacity )
        {
            m_pAddress[m_Tail] = AValue;
            //if ( ++m_Tail == m_Capacity ) m_Tail = 0;
            m_Tail = (m_Tail + 1) % m_Capacity;
            m_Count++;
            return true;
        }
        return false;
    }

    //����, Ĭ��ʹ�ý���
    inline void Sort(IxdCompareInterface* ApInterface) 
    {
        int nEnd = m_Head + m_Count;
        for( int i = m_Head; i < nEnd; i++ )
        {
        	for( int j = i + 1; j < nEnd; j++ )
        	{
        		T t1 = m_pAddress[i];
                T t2 = m_pAddress[j];
                int n( 0 );
                ApInterface->Notify( t1, t2, n );

                if ( n < 0 )
                {
                    m_pAddress[i] = t2;
                    m_pAddress[j] = t1;
                }
                
        	}
        }
    }

    //����
    inline void Loop(IxdQueueLoopInterface* ApInterface)
    {
        int nEnd = m_Head + m_Count;
        bool bBreak( false );
        for( int i = m_Head; i < nEnd; i++ )
        {
            bBreak = false;
            ApInterface->Notify( m_pAddress[i], bBreak );
            if ( bBreak ) break;
        }
    }
private:
    T *m_pAddress;
};

//CxdQueueEx: �̰߳�ȫ
//ʹ�ø��ǵķ�ʽʵ���̰߳�ȫ, ��ʹ���麯��
template<typename T>
class CxdQueueEx: public CxdQueue<T>
{
public:
    CxdQueueEx(const UINT &ACapacity = 1024 * 8): CxdQueue<T>(ACapacity) { }
    inline bool Get(T &AValue)
    {
        m_mutex.Lock();
        bool b =  CxdQueue<T>::Get( AValue );
        m_mutex.Unlock();
        return b;
    }
    inline bool Put(const T &AValue)
    {
        m_mutex.Lock();
        bool b =  CxdQueue<T>::Put( AValue );
        m_mutex.Unlock();
        return b;
    }
    inline void Sort(IxdCompareInterface* ApInterface) 
    {
        m_mutex.Lock();
        CxdQueue<T>::Sort( ApInterface );
        m_mutex.Unlock();
    }
    inline void Loop(IxdQueueLoopInterface* ApInterface)
    {
        m_mutex.Lock();
        CxdQueue<T>::Loop( ApInterface );
        m_mutex.Unlock();
    }
private:
    CxdMutex m_mutex;
};

//WORD ��, ���̰߳�ȫ
class CxdWordTable
{
public:
    CxdWordTable();
    ~CxdWordTable();

    bool GetWordID(WORD &AID);
    bool ReclaimWordID(const WORD &AID);

    PropertyGetValue( Count, UINT );
    PropertyValue( CheckReclaimID, bool, true );
private:
    WORD m_aWordIDTable[MAXWORD + 1];
    UINT m_nCapacity;
    UINT m_nHead, m_nTail;
};

//CxdHashBasic, �ڴ���HASH����
class CxdHashBasic
{
public:
//�ڵ㶨��
#pragma pack(1)
    struct TxdHashNode 
    {
        UINT64 HashID;
        LPVOID HashData;
        TxdHashNode *NextNode;
    };
#pragma pack()

public:
    CxdHashBasic();
    virtual ~CxdHashBasic();

    //�������ڴ������ʱ,�Ժ��Hash��Ĳ���Ҫ���ⲿ������߳�����
    bool SetHashNodeMemManage(CxdFixedMemoryManage *ApMemManage);
    int  GetHashNodeMemInfo(TxdGetMemInfo *&ApInfos, int ACount);

    //��������Ԫ��, ��;��ʹ�� DeleteLoopItem ��ɾ��
    bool LoopBegin(void);
    bool LoopNext(UINT64 &AHashID, LPVOID &AValue);
    void DeleteLoopItem(void);

    //����
    inline bool IsEmpty(void) { return 0 == m_Count; }
    inline bool IsHasElement(void) { return 0 != m_Count; }
    inline UINT GetInitHashNodeMemCount(void) { return m_nInitNodeCount; }
    void        SetInitHashNodeMemCount(UINT ACount);
    PropertyValue( HashTableCount, UINT, !GetActive() );
    PropertyActive( ActiveHashArray(), UnActiveHashArray() );
    PropertyGetValueProtected( Count, UINT );
protected:
    LPVOID *m_pHashTable;
    CxdFixedMemoryManage *m_pHashNodeManage;
    inline UINT GetIndexByID(const UINT64 &AID) { return m_bTableCountIsN2 ? Mod(AID, m_HashTableCount) : AID % m_HashTableCount; }
private:
    bool ActiveHashArray();
    bool UnActiveHashArray();

    bool m_bTableCountIsN2; //�Ƿ���2��N�η�
    int m_nInitNodeCount;
    bool m_bCreateMemManage;
    UINT m_nLoopIndex;
    UINT m_nLoopCount;
    TxdHashNode *m_pParent;
    TxdHashNode *m_pOldParent;
    TxdHashNode *m_pLoopNode;
};

//CxdHashArray��, HashID��Ψһ�� ���̰߳�ȫ
class CxdHashArray: public CxdHashBasic
{
public:
    //�����Ϣ,������������κδ�������false,������ӳɹ�
    bool   Add(const UINT64 &AHashID, const LPVOID &ApData);
    bool   Find(const UINT64 &AHashID, LPVOID &ApFindData, bool bDelete = false);
    bool   Modify(const UINT64 &AHashID, const LPVOID &ApNewData);
    LPVOID Delete(const UINT64 &AHashID);
private:
    bool _FindNode(const UINT64 &AID, LPVOID &AFindData, bool bDelete, bool bCreateOnNull, const LPVOID &ANewValueOnNotFind = 0);
};

//CxdHashArray��, HashID�ǿ��ظ���
class CxdHashArrayEx: public CxdHashBasic
{
public:
    CxdHashArrayEx();
    //ֱ�����,�������ظ�
    void Add(const UINT64 &AHashID, const LPVOID &ApData);

    //������ID��ͬ�Ľڵ�
    bool FindBegin(const UINT64 &AHashID, LPVOID &AFindValue);
    bool FindNext(LPVOID &AFindValue);
    void DeleteFindItem(void); 
private:
    UINT64 m_HashID;
    UINT m_nFindIndex;
    TxdHashNode *m_pParent;
    TxdHashNode *m_pOldParent;
    TxdHashNode *m_pFindNode;
};


#endif