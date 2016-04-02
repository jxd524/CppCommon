/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdCombiBuffer.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-11-12 18:21:48
	LastModify : 
	Description: 组合包解决方案
        GetCombiMem(...) 将对整个对象上锁, 外部在处理完之后,必须调用:
        ReleaseCombiMem(...)

        数据缓存时间, 删除动作由类自我管理.

        //创建一个Hash表
        virtual CxdHashBasic* CreateHashTable() = 0; 

        //释放存放在结构中的缓存, FBufLens[i] != 0 则释放 FBuffers[i]
        virtual void  DoReleaseCombiBuffer(TxdCombiBufferInfo *ApInfo) = 0;

        //结构体超时, 如果返回 false 则重新等待超时, 否则将触发 DoReleaseCombiBuffer
        virtual bool  DoCombiBufferTimeout(TxdCombiBufferInfo *ApInfo) = 0;  //当项超时,返回true表示删除, 否则不处理
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
    WORD FCombiID;                                 //组合包ID
    byte FCount;                                   //数量
    UINT64 FLastActiveTime;                        //单位: 秒; 最后活动时间,用于释放内存, 为0时表示不对其进行超时检查
    WORD FBufLens[CtMaxCombiPackageCount];         //长度
    LPVOID FBuffers[CtMaxCombiPackageCount];       //具体包,已完成协议编码
};
//发送组合包结构信息
typedef TxdCombiBufferInfo TxdSendCombiBufferInfo;

//接收组合包结构信息
struct TxdRecvCombiBufferInfo: public TxdCombiBufferInfo
{
    byte FCurRecvCount;                            //已经完成的包数
    byte FTimeoutCount;                            //超时次数, 超过指定次数,则说明失败,将释放内存
    SOCKET FScoket;                                //使用的套接字, 找回丢包时使用
    unsigned long FRemoteIP;                       //包IP
    unsigned short int FRemotePort;                //端口
};
#pragma pack()

const int CtSendCombiBufferInfoSize = sizeof(TxdSendCombiBufferInfo);
const int CtRecvCombiBufferInfoSize = sizeof(TxdRecvCombiBufferInfo);

typedef IxdNotifyEventT<LPVOID> IxdCombiBuffer; 
typedef IxdNotifyEventT<TxdRecvCombiBufferInfo*> IxdCombiBufferRecv; 
//====================================================================
// 组合包基本类
//====================================================================
//
class CxdCombiBasic
{
public:
    CxdCombiBasic(const int &ABufferSize);
    virtual ~CxdCombiBasic();

    void ReleaseCombiMem(TxdCombiBufferInfo* ApMem);

    /*内存相关属性*/
    PropertyValue( CombiBufferInitCount, UINT, true );   //组合缓存节点初始值
    PropertyValue( HashTableCount, UINT, !GetActive() ); //Hash表节点数

    //时间相关: 单位(秒)
    PropertyValue( CombiBufferExsitsTime, UINT, true ); //缓存存在时间
    PropertyValue( MemUnActiveTimeoutSpace, UINT, true ); //大块内存不被使用超过指定时间,则被自动释放
    
    //启动
    PropertyActive( ActiveManage(), UnActiveManage() ); 

    //释放保存在类中的节点缓存接口
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
    virtual bool  DoCombiBufferTimeout(TxdCombiBufferInfo *ApInfo) = 0;  //当项超时,返回true表示删除, 否则不处理
private:
    void DoThreadCheckMem(int);
};

//====================================================================
// 组合发送缓存类
//====================================================================
//
class CxdCombiSend: public CxdCombiBasic
{
public:
    CxdCombiSend(): CxdCombiBasic(CtSendCombiBufferInfoSize) {}
    ~CxdCombiSend() {}

    //ACombiID < 0: 直接创建; 否则,只进行查找
    bool GetCombiMem(int ACombiID, TxdSendCombiBufferInfo *&ApInfo);
protected:
    virtual bool ActiveManage(void);
    virtual bool UnActiveManage(void);
    virtual CxdHashBasic* CreateHashTable() { return new CxdHashArray; }
    virtual bool  DoCombiBufferTimeout(TxdCombiBufferInfo *ApInfo) { return true; }
private:
    CxdWordTable *m_pWordTable; //组合ID表
};

//====================================================================
// 组合包接收类
//====================================================================
//
class CxdCombiRecv: public CxdCombiBasic
{
public:
    CxdCombiRecv();
    ~CxdCombiRecv();
    bool GetCombiMem(const TxdSocketAddr &AAddr, WORD ACombiID, byte ACombiCount, 
        TxdRecvCombiBufferInfo *&ApInfo, bool AbCreateOnNull);

    /*接口定义*/
    //找回丢失的数据
    PropertyInterface( GetLostCombiBuffer, IxdCombiBufferRecv );
    //组合包不完整错误
    PropertyInterface( RecvCombiBufferError, IxdCombiBufferRecv );

    //最多超时次数
    PropertyValue( MaxTimeoutCount, int, true );
protected:
    virtual CxdHashBasic* CreateHashTable() { return new CxdHashArrayEx; }
    virtual bool  DoCombiBufferTimeout(TxdCombiBufferInfo *ApInfo);
};

#endif