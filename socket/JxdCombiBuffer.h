/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdCombiBuffer.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-11-12 18:21:48
	LastModify : 
	Description: ��ϰ��������
        GetCombiMem(...) ����������������, �ⲿ�ڴ�����֮��,�������:
        ReleaseCombiMem(...)

        ���ݻ���ʱ��, ɾ�������������ҹ���.

        //����һ��Hash��
        virtual CxdHashBasic* CreateHashTable() = 0; 

        //�ͷŴ���ڽṹ�еĻ���, FBufLens[i] != 0 ���ͷ� FBuffers[i]
        virtual void  DoReleaseCombiBuffer(TxdCombiBufferInfo *ApInfo) = 0;

        //�ṹ�峬ʱ, ������� false �����µȴ���ʱ, ���򽫴��� DoReleaseCombiBuffer
        virtual bool  DoCombiBufferTimeout(TxdCombiBufferInfo *ApInfo) = 0;  //���ʱ,����true��ʾɾ��, ���򲻴���
**************************************************************************/

#pragma once

#ifndef _JxdCombiBuffer_Lib
#define _JxdCombiBuffer_Lib

#ifdef linux
#include <string.h>
#endif

#include "JxdTypedef.h"
#include "JxdMacro.h"
#include "JxdEvent.h"
#include "JxdSockDefines.h"
#include "JxdSynchro.h"
#include "JxdMem.h"
#include "JxdThread.h"
#include "JxdSockBasic.h"

#define ReleaseCombiBuffer 1

#pragma pack(1)
struct TxdCombiBufferInfo
{
    WORD FCombiID;                                 //��ϰ�ID
    byte FCount;                                   //����
    UINT64 FLastActiveTime;                        //��λ: ��; ���ʱ��,�����ͷ��ڴ�, Ϊ0ʱ��ʾ��������г�ʱ���
    WORD FBufLens[CtMaxCombiPackageCount];         //����
    LPVOID FBuffers[CtMaxCombiPackageCount];       //�����,�����Э�����
};
//������ϰ��ṹ��Ϣ
typedef TxdCombiBufferInfo TxdSendCombiBufferInfo;

//������ϰ��ṹ��Ϣ
struct TxdRecvCombiBufferInfo: public TxdCombiBufferInfo
{
    byte FCurRecvCount;                            //�Ѿ���ɵİ���
    byte FTimeoutCount;                            //��ʱ����, ����ָ������,��˵��ʧ��,���ͷ��ڴ�
    SOCKET FScoket;                                //ʹ�õ��׽���, �һض���ʱʹ��
    unsigned long FRemoteIP;                       //��IP
    unsigned short int FRemotePort;                //�˿�
};
#pragma pack()

const int CtSendCombiBufferInfoSize = sizeof(TxdSendCombiBufferInfo);
const int CtRecvCombiBufferInfoSize = sizeof(TxdRecvCombiBufferInfo);

typedef IxdNotifyEventT<LPVOID> IxdCombiBuffer; 
typedef IxdNotifyEventT<TxdRecvCombiBufferInfo*> IxdCombiBufferRecv; 
//====================================================================
// ��ϰ�������
//====================================================================
//
class CxdCombiBasic
{
public:
    CxdCombiBasic(const int &ABufferSize);
    virtual ~CxdCombiBasic();

    void ReleaseCombiMem(TxdCombiBufferInfo* ApMem);

    /*�ڴ��������*/
    PropertyValue( CombiBufferInitCount, UINT, true );   //��ϻ���ڵ��ʼֵ
    PropertyValue( HashTableCount, UINT, !GetActive() ); //Hash��ڵ���

    //ʱ�����: ��λ(��)
    PropertyValue( CombiBufferExsitsTime, UINT, true ); //�������ʱ��
    PropertyValue( MemUnActiveTimeoutSpace, UINT, true ); //����ڴ治��ʹ�ó���ָ��ʱ��,���Զ��ͷ�
    
    //����
    PropertyActive( ActiveManage(), UnActiveManage() ); 

    //�ͷű��������еĽڵ㻺��ӿ�
    PropertyInterface( FreeCombiBuffer, IxdCombiBuffer );
protected:
    const int m_CtBufferSize;
    
    CxdMutex m_lock;
    int m_nTestClearMem;
    CxdFixedMemoryManage *m_pMemManage;
    CxdHashBasic *m_pHashTable;
    CxdThreadEvent<int> *m_pThread;
    void CheckThreadForCombiBuffer(void);
    void FreeThread(void);
    void RelaseCombiBuffer(TxdCombiBufferInfo *ApInfo);

    virtual bool ActiveManage(void);
    virtual bool UnActiveManage(void);
    virtual CxdHashBasic* CreateHashTable() = 0;
    virtual void  DoReleaseCombiBuffer(TxdCombiBufferInfo *ApInfo);
    virtual bool  DoCombiBufferTimeout(TxdCombiBufferInfo *ApInfo) = 0;  //���ʱ,����true��ʾɾ��, ���򲻴���
private:
    void DoThreadCheckMem(int);
};

//====================================================================
// ��Ϸ��ͻ�����
//====================================================================
//
class CxdCombiSend: public CxdCombiBasic
{
public:
    CxdCombiSend(): CxdCombiBasic(CtSendCombiBufferInfoSize) {}
    ~CxdCombiSend() {}

    //ACombiID < 0: ֱ�Ӵ���; ����,ֻ���в���
    bool GetCombiMem(int ACombiID, TxdSendCombiBufferInfo *&ApInfo);
protected:
    virtual bool ActiveManage(void);
    virtual bool UnActiveManage(void);
    virtual CxdHashBasic* CreateHashTable() { return new CxdHashArray; }
    virtual bool  DoCombiBufferTimeout(TxdCombiBufferInfo *ApInfo) { return true; }
private:
    CxdWordTable *m_pWordTable; //���ID��
};

//====================================================================
// ��ϰ�������
//====================================================================
//
class CxdCombiRecv: public CxdCombiBasic
{
public:
    CxdCombiRecv();
    ~CxdCombiRecv();
    bool GetCombiMem(const TxdSocketAddr &AAddr, WORD ACombiID, byte ACombiCount, 
        TxdRecvCombiBufferInfo *&ApInfo, bool AbCreateOnNull);

    /*�ӿڶ���*/
    //�һض�ʧ������
    PropertyInterface( GetLostCombiBuffer, IxdCombiBufferRecv );
    //��ϰ�����������
    PropertyInterface( RecvCombiBufferError, IxdCombiBufferRecv );

    //��೬ʱ����
    PropertyValue( MaxTimeoutCount, int, true );
protected:
    virtual CxdHashBasic* CreateHashTable() { return new CxdHashArrayEx; }
    virtual bool  DoCombiBufferTimeout(TxdCombiBufferInfo *ApInfo);
};

#endif