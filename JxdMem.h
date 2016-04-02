/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdMem.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 14:56:14
	LastModify : 
	Description: �ڴ������, ���������ڴ�,Ȼ���Լ������,
                 �ֽ�Ϊ���С���ڴ����ʹ��, ��CxdQueueΪ���Ľ����ڴ��ַ����
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

//�̶��ڴ����������, �ֿ��С���Ϊ2��N�η�, ���̰߳�ȫ
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
    inline bool IsAllMemWidthoutUsed(void) { return m_oMemQueuy.IsFull(); }  //�� m_oMemQueuy ȫ��ʱ, �����ڴ涼û��ʹ��
    inline bool IsAllMemFullUsed(void) { return m_oMemQueuy.IsEmpty(); }     //�� m_oMemQueuy ȫ��ʱ, �����ڴ�ȫ�����ù�
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
    bool m_bBlockSizeN2; //BlockSize�Ƿ���2��N�η�;
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

//�̶��ڴ����, �������ֵ����, ���̰߳�ȫ
class CxdFixedMemoryManage
{
    //�ṹ��: ��ʼ����Ϣ, ָ����Χ, ��ʼ���ڴ��
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

    //��ʼ��������Ϣ
    bool AddAllocInfo(UINT AMinBlockSize, UINT AMaxBlockSize, UINT ABlockCount = 0);
    bool AddFixedAllocInfo(UINT ABlockSize, UINT ABlockCount = 0);
    void InitMem(void);
    void InitMem(UINT AMinBlockSize, UINT AMaxBlockSize, UINT ACount = 0);

    //��ȡʹ�����
    inline UINT GetMemObjectCount(void) { return m_ltMem.size(); }
    int         GetMemInfo(TxdGetMemInfo *&ApInfos, int ACount, UINT AFindBlockSize = 0); //���ؿ�ʹ������, ACount: ��ʾ�������
    static void FreeMemInfo(TxdGetMemInfo *&ApInfos);
    
    //��ȡ�̶���С���ڴ��, һ���ȡ�ṹ���ڴ�Ŀ���ʹ��
    bool GetFixedMem(LPVOID &AddrValue, const UINT &ASize);
    //��ȡ�ɱ��С���ڴ��, ���ܱ�ASize��, ��Χ�� AddAllocInfo ָ��, ���ɶ�дΪ AMaxSize
    bool GetValMem(LPVOID &AddrValue, const UINT &ASize, UINT &AMaxSize);
    //��ȡ�ɱ��С���ڴ��, �ⲿ���ж�дʱ,������ASize
    bool GetMem(LPVOID &AddrValue, const UINT &ASize);

    //ͳһ���ͷ��ڴ淽��, ֻ����
    bool FreeMem(const LPVOID &AddrValue);

    bool IsEmpty(void);
    int  GetAllMemWidthoutUsedCount(void);

    //�ͷ�
    void ClearMemory();

    //��λ: ��; �ͷ�һֱû�б�ʹ�õ��ڴ�, �������ڴ涼û�б�ʹ��ʱ��������ָ����ʱ, ���ͷŶ���
    void ClearUnUsedMemroy(const UINT &AUnUsedSpaceTime);

    //����
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

//CxdFixedMemoryManageEx: �̰߳�ȫ
//ʹ�ø��ǵķ�ʽʵ���̰߳�ȫ, ��ʹ���麯��
class CxdFixedMemoryManageEx: public CxdFixedMemoryManage
{
public:
    //��ʼ��
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