/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
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

    //排序, 默认使用降序
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

    //遍历
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

//CxdQueueEx: 线程安全
//使用覆盖的方式实现线程安全, 不使用虚函数
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

//WORD 表, 非线程安全
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

//CxdHashBasic, 内存与HASH管理
class CxdHashBasic
{
public:
//节点定义
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

    //当设置内存管理类时,以后对Hash表的操作要在外部处理多线程问题
    bool SetHashNodeMemManage(CxdFixedMemoryManage *ApMemManage);
    int  GetHashNodeMemInfo(TxdGetMemInfo *&ApInfos, int ACount);

    //遍历所有元素, 中途可使用 DeleteLoopItem 来删除
    bool LoopBegin(void);
    bool LoopNext(UINT64 &AHashID, LPVOID &AValue);
    void DeleteLoopItem(void);

    //属性
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

    bool m_bTableCountIsN2; //是否是2的N次方
    int m_nInitNodeCount;
    bool m_bCreateMemManage;
    UINT m_nLoopIndex;
    UINT m_nLoopCount;
    TxdHashNode *m_pParent;
    TxdHashNode *m_pOldParent;
    TxdHashNode *m_pLoopNode;
};

//CxdHashArray表, HashID有唯一性 非线程安全
class CxdHashArray: public CxdHashBasic
{
public:
    //添加信息,如果存在则不做任何处理并返回false,否则添加成功
    bool   Add(const UINT64 &AHashID, const LPVOID &ApData);
    bool   Find(const UINT64 &AHashID, LPVOID &ApFindData, bool bDelete = false);
    bool   Modify(const UINT64 &AHashID, const LPVOID &ApNewData);
    LPVOID Delete(const UINT64 &AHashID);
private:
    bool _FindNode(const UINT64 &AID, LPVOID &AFindData, bool bDelete, bool bCreateOnNull, const LPVOID &ANewValueOnNotFind = 0);
};

//CxdHashArray表, HashID是可重复的
class CxdHashArrayEx: public CxdHashBasic
{
public:
    CxdHashArrayEx();
    //直接添加,不考虑重复
    void Add(const UINT64 &AHashID, const LPVOID &ApData);

    //返回与ID相同的节点
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