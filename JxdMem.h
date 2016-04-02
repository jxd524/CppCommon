/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdMem.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 14:56:14
	LastModify : 
	Description: 内存管理类, 以申请大块内存,然后自己管理成,
                 分解为多个小块内存进行使用, 以CxdQueue为核心进行内存地址管理
**************************************************************************/
#pragma once
#ifndef _JxdMem_Lib
#define _JxdMem_Lib

#include <string>
#include <vector>
using std::vector;
using std::string;

#ifdef linux
#include "JxdTypedef.h"
#endif
#include "JxdDataStruct.h"
#include "JxdMacro.h"
#include "JxdSynchro.h"
#include "JxdTime.h"

//固定内存申请与分配, 分块大小最好为2的N次方, 非线程安全
class CxdFixedMemory
{
public:
    CxdFixedMemory(const UINT &ABlockSize, const UINT &ABlockCount);
    ~CxdFixedMemory();

    PropertyGetValue( BlockSize, UINT );
    PropertyGetValue( Capacity, UINT );
    PropertyGetValue( IsMallocOK, bool );
    PropertyGetValue( LastAllMemWidthoutUsed, INT64 );
    inline UINT GetUsedCount(void) { return m_oMemQueuy.GetCapacity() - m_oMemQueuy.GetCount(); }
    inline UINT GetCount(void) { return m_oMemQueuy.GetCount(); }
    inline bool IsAllMemWidthoutUsed(void) { return m_oMemQueuy.IsFull(); }  //当 m_oMemQueuy 全满时, 表明内存都没有使用
    inline bool IsAllMemFullUsed(void) { return m_oMemQueuy.IsEmpty(); }     //当 m_oMemQueuy 全空时, 表明内存全部被用光
    inline bool IsEffective(const LPVOID &Addr) 
    {
        UINT64 nAddr = (UINT64)Addr;
        if ( m_IsMallocOK && nAddr >= m_nAddressMinValue && nAddr < m_nAddressMaxValue )
        {
            INT64 v = nAddr - m_nAddressMinValue;
            if ( m_bBlockSizeN2 ) return 0 == Mod(v, m_BlockSize);

            return (v % m_BlockSize) == 0;
        }
        return false;
    }

    bool GetMem(LPVOID &AMem) { return m_oMemQueuy.Get(AMem); }
    void FreeMem(const LPVOID &AMem) { m_oMemQueuy.Put(AMem); if ( IsAllMemWidthoutUsed() ) m_LastAllMemWidthoutUsed = GetRunningSecond();  }
private:
    typedef CxdQueue<LPVOID> TxdMemQueuy;
    bool m_bBlockSizeN2; //BlockSize是否是2的N次方;
    LPVOID m_pMemoryAddress;
    UINT64 m_nAddressMinValue;
    UINT64 m_nAddressMaxValue;
    TxdMemQueuy m_oMemQueuy;
};

struct TxdGetMemInfo
{
    UINT BlockSize;
    UINT ObjectCount;
    UINT TotalCount;
    UINT UsedCount;
};
string FormatMemInfo(TxdGetMemInfo *ApInfo, int ACount);

//固定内存管理, 基于最大值分配, 非线程安全
class CxdFixedMemoryManage
{
    //结构体: 初始化信息, 指定范围, 初始化内存块
    struct TxdMemSizeInfo
    {
        TxdMemSizeInfo(UINT AMinBlockSize, UINT AMaxBlockSize, UINT ABlockCount)
        {
            MinBlockSize = AMinBlockSize;
            MaxBlockSize = AMaxBlockSize;
            BlockCount = ABlockCount;
            IsFixedMem = MinBlockSize == MaxBlockSize;
        }
        bool IsFixedMem;
        UINT MinBlockSize, MaxBlockSize;
        UINT BlockCount;
    };
public:
    CxdFixedMemoryManage();
    virtual ~CxdFixedMemoryManage();

    //初始化管理信息
    bool AddAllocInfo(UINT AMinBlockSize, UINT AMaxBlockSize, UINT ABlockCount = 0);
    bool AddFixedAllocInfo(UINT ABlockSize, UINT ABlockCount = 0);
    void InitMem(void);
    void InitMem(UINT AMinBlockSize, UINT AMaxBlockSize, UINT ACount = 0);

    //获取使用情况
    inline UINT GetMemObjectCount(void) { return m_ltMem.size(); }
    int         GetMemInfo(TxdGetMemInfo *&ApInfos, int ACount, UINT AFindBlockSize = 0); //返回可使用数量, ACount: 表示最大数量
    static void FreeMemInfo(TxdGetMemInfo *&ApInfos);
    
    //获取固定大小的内存块, 一般获取结构体内存的可以使用
    bool GetFixedMem(LPVOID &AddrValue, const UINT &ASize);
    //获取可变大小的内存块, 可能比ASize大, 范围由 AddAllocInfo 指定, 最大可读写为 AMaxSize
    bool GetValMem(LPVOID &AddrValue, const UINT &ASize, UINT &AMaxSize);
    //获取可变大小的内存块, 外部进行读写时,限制在ASize
    bool GetMem(LPVOID &AddrValue, const UINT &ASize);

    //统一的释放内存方法, 只回收
    bool FreeMem(const LPVOID &AddrValue);

    bool IsEmpty(void);
    int  GetAllMemWidthoutUsedCount(void);

    //释放
    void ClearMemory();

    //单位: 秒; 释放一直没有被使用的内存, 当所有内存都没有被使用时间间隔超过指定数时, 将释放对象
    void ClearUnUsedMemroy(const UINT &AUnUsedSpaceTime);

    //属性
    PropertyValue( DefaultBlockCount, UINT, true );
    PropertyGetValue( UsedMemCount, UINT );
private:
    UINT m_nMaxUsedMemCount;
    void IncUsedMemCount();

    vector<CxdFixedMemory*> m_ltMem;
    vector<TxdMemSizeInfo*> m_ltAllocInfo;

    CxdFixedMemory *m_pLastGetMemObject;
    CxdFixedMemory *m_pLastFreeMemObject;
    UINT GetFixedBlockCount(const UINT &ASize);
    UINT GetValBlockCount(const UINT &ASize, UINT &AMaxSize);
};

//CxdFixedMemoryManageEx: 线程安全
//使用覆盖的方式实现线程安全, 不使用虚函数
class CxdFixedMemoryManageEx: public CxdFixedMemoryManage
{
public:
    //初始化
    //void InitMem(const UINT &ASize, const UINT &ACount);
    bool GetFixedMem(LPVOID &AddrValue, const UINT &ASize);
    bool GetValMem(LPVOID &AddrValue, const UINT &ASize, UINT &AMaxSize);
    bool GetMem(LPVOID &AddrValue, const UINT &ASize);
    bool FreeMem(const LPVOID &AddrValue);
    void ClearMemory();
    void ClearUnUsedMemroy(const UINT &AUnUsedSpaceTime);
private:
    CxdMutex m_mutex;
};

class CxdMemManageHelper
{
public:
    virtual ~CxdMemManageHelper(){}

    bool AddAllocInfo(UINT ABlockSize, UINT ABlockCount) { return m_oMemManage.AddAllocInfo(ABlockSize, ABlockCount); }
    int  GetMemInfo(TxdGetMemInfo *&ApInfos, int &ACount) { return m_oMemManage.GetMemInfo(ApInfos, ACount); }
    void FreeMemInfo(TxdGetMemInfo *&ApInfos) { CxdFixedMemoryManage::FreeMemInfo(ApInfos); }
    void DebugMemInfo(const char *ApMemName);
protected:
    CxdFixedMemoryManage m_oMemManage;
};

#endif