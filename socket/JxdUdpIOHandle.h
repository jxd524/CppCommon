/**************************************************************************
Copyright      : pp.cc 深圳指掌时代 珊瑚组
File           : JxdUdpIOHandle.h & .cpp 
Author         : Terry
Email          : jxd524@163.com
CreateTime     : 2012-8-25 18:19
LastModifyTime : 2012-9-03 18:19
Description    : UDP数据处理类

IO层协议定义: 
[独立包]: 版本(1) - 此包总长度(2) - CRC32码(4) - 包信息(1)[前四位：0； 后四位: 协议命令IoCmd]
IoCmd: 0000 -> 需要由子类处理
IoCmd: 0001 -> 请求重发组合包命令(CtIoCmd_GetCombiPackage), 内容: 组合包ID(2) - 请求包信息(n) - Package1,Package2...PackageN
IoCmd: 0010 -> 不存在指定的组合包(CtIoCmd_NotFoundCombiPackage), 发送方已经删除, 内容: 组合包ID(2)

[组合包]: 版本(1) - 此包总长度(2) - CRC32码(4) - 包信息(1)[前四位：个数CombiCount； 后四位: 包顺序] - 标志CombiID(2) - 包内容

限制:
1: 组合包最大组合只能 16 个包, 长度: CtCombiPackageSize * 16
2: 独立包没有进行丢包处理
**************************************************************************/

#pragma once

#include "JxdSockBasic.h"
#include "JxdSockDefines.h"
#include "JxdDataStruct.h"
#include "JxdCombiBuffer.h"
#include "JxdMem.h"

//统计接收及发送的数据长度
#define CountBufferLength

class CxdUdpIOHandle: public CxdSocketBasic
{
public:
    CxdUdpIOHandle();
    ~CxdUdpIOHandle();

    /*功能*/
    int  SendBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, int ABufLen);

    /*属性定义, 时间单位: 秒*/
    PropertyInterface( UdpRecv, IxdSocketEventManage );       //实现 UDP read 接口, 由类自动管理接口,外部不应该再释放
    PropertyValue( RecvQueueMaxCount, UINT, !GetActive() );   //缓存接收到数据的队列最大长度, 可小可能会出现自动丢弃数据包
    PropertyValue( HandleIoThreadCount, UINT, !GetActive() ); //IO处理线程
    PropertyValue( UnUsedMemTimeoutSpace, UINT, true );       //当内存块不需要使用时,最长保持时间

    /*组合包相关设置*/
    PropertyValue( InitCombiPackageBufferCount, UINT, !GetActive() );  //初始化缓存大小: CtMaxCombiPackageBufferSize, 用于合并组合包
    //1.发送组合包相关属性
    PropertyValue( SendCombiInitBufferCount, UINT, !GetActive() ); //发送缓存节点初始化数量 缓存大小 CtUdpBufferMaxSize
    PropertyValue( SendCombiHashTableCount, UINT, !GetActive() );  //发送缓存HASH桶
    PropertyValue( SendCombiBufferExsitsTime, UINT, true );        //发送缓存存在最长时间

    //2.接收组合包相关属性
    PropertyValue( RecvCombiInitBufferCount, UINT, !GetActive() );  //接收缓存节点初始化数量: 缓存大小 CtPerCombiPackageBufferSize
    PropertyValue( RecvCombiHashTableCount, UINT, !GetActive() );   //接收缓存HASH桶
    PropertyValue( RecvCombiBufferExsitsTime, UINT, true );         //接收缓存存在最长时间
    PropertyValue( RecvCombiMaxTimeoutCount, UINT, true );          //接收缓存最大超时次数

    /*只读属性*/
    PropertyGetValue( HandleIORunningThreadCount, long );

    /*统计*/
#ifdef CountBufferLength
    PropertyGetValue( SendBufferLengthMB, UINT );      //发送数据MB
    PropertyGetValue( SendBufferLengthByte, UINT );    //发送数据小于MB部分
#endif
    PropertyGetValue( SendPackageCount, UINT );        //发送独立包数量
    PropertyGetValue( SendFailedPackageCount, UINT );  //发送失败包数量
    PropertyGetValue( SendSinglePackageCount, UINT );  //发送独立包数量
    PropertyGetValue( SendCombiPackageCount, UINT );   //发送组合包数量
    PropertyGetValue( LostCombiPackageCount, UINT );   //组合包不全丢弃数量

#ifdef CountBufferLength
    PropertyGetValue( RecvBufferLengthMB, UINT );      //接收数据总长度
    PropertyGetValue( RecvBufferLengthByte, UINT );    //接收数据总长度
#endif
    PropertyGetValue( RecvPackageCount, UINT );        //接收到总数据长度
    PropertyGetValue( RecvErrorIOProtocolCount, UINT );//接收错误IO协议包
    PropertyGetValue( RecvSinglePackageCount, UINT );  //接收到独立包
    PropertyGetValue( RecvCombiPackageCount, UINT );   //接收到组合包
    PropertyGetValue( AutoThrowRecvPackage, UINT );    //自动丢弃接收到数据包总量: 接收队列太小
protected:
    //方便初始化
    void InitForServer(void); 
    void InitForClient(void); 

    //当接收到一个完整的数据包是被触发
    virtual void OnRecvBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen) = 0;

    //处理父类信息
    bool   ActiveService();
    bool   UnActiveService();
    SOCKET CreateSocket(int AIndex){ return socket(AF_INET, SOCK_DGRAM, 0); }
    void   DoSocketClosed(SOCKET s);
    void   DoBindSockefFinished(SOCKET ASocket, int AIndex, bool AIsOK) { if (AIsOK) GetItUdpRecv()->RelateSocket( ASocket ); }

    //新虚函数
    virtual IxdSocketEventManage* GetUdpRecvInterface(void) { return GetItUdpRecv(); }
private:
    //接收到通知, 调用接口接收原始数据
    void __DoRecvBuffer(SOCKET ASocket);
    //发送数据, 调用原始接口
    int __SendBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, const char* ApBuffer, int ABufferLen );

    //发送数据包
    int _SendPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApPackage, int ALen, bool AIsSingledPackage);
    int SendSinglePackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, WORD ABufLen, byte AIoCmd = 0);
    int SendCombiPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, WORD ABufLen);

    //发送网络协议包
    //DoGetLostCombiBuffer: 获取丢失的数据包
    void SendNotFoundCombiPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, WORD ACombiID, byte ACombiCount);

    //处理原始的UDP数据包
    void DoHandleUdpPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    //处理组合包
    void DoHandleCombiPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen, 
        WORD ACombiID, byte ACombiCount, byte ACombiIndex);
    //处理此类定义的协议,网络层协议
    void DoHandlNetProtocolPackage(byte ACmdID, SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoRecvGetCombiPackageCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoRecvNotFoundCombiPackageCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
private:
    void InitUdpIOHandleStateValue(void);
#ifdef CountBufferLength
    CxdMutex m_mtState; //统计安全
#endif
    //内存相关数据结构
#pragma pack(1)
    struct TxdRecvPackageInfo 
    {
        SOCKET FSocket;
        unsigned long FRemoteIP;
        unsigned short int FRemotePort;
        WORD FBufferLength;
        char FBuffer[CtUdpBufferMaxSize];
    };
#pragma pack()
    //m_pRecvBufManage: 包含 TxdRecvPackageInfo大小的内存块;
    CxdFixedMemoryManageEx *m_pRecvBufManage;
    CxdQueueEx<TxdRecvPackageInfo*> *m_pRecvPackageList;
    
    bool m_bWaitToClose;
    void DoThreadHandleIO(LPVOID);

    /*组合包相关数据结构*/
    //组合包内存管理, 包含用于
    //  1: 发送的组合包内存 --> CtUdpBufferMaxSize
    //  2: 接收组合包内存   --> CtPerCombiPackageBufferSize
    //  3: 合并接收到组合包的内存  --> CtMaxCombiPackageBufferSize
    CxdMutex m_mtInitCombiObject;
    CxdFixedMemoryManageEx *m_pCombiBufferManage; 
    CxdCombiSend *m_pCombiSendManage; //发送组合包缓存管理器
    CxdCombiRecv *m_pCombiRecvManage; //接收组合包缓存管理器
    void CreateCombiBufferManage(void);
    void CreateCombiSendManage(void);
    void CreateCombiRecvManage(void);

    //事件处理
    void DoFreeCombiMem(LPVOID AMem); 
    void DoGetLostCombiBuffer(TxdRecvCombiBufferInfo* ApInfo);
    void DoRecvCombiBufferError(TxdRecvCombiBufferInfo* ApInfo);
};