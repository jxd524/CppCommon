/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdSockInterface.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-22 16:38:16
	LastModify : 
	Description: 
        TServerBindInfo: 为方便绑定本地网卡设置的结构体
        TSocketAddr: 方便使用的网络地址
        CxdSocketBasic: Socket基本类, 提供给TCP, UDP基类使用
        主要实现方法:
        virtual SOCKET  CreateSocket(int AIndex){ return -1; }
        virtual void DoBindSockefFinished(SOCKET ASocket, int AIndex, bool AIsOK);
        virtual bool ActiveService();
        virtual bool UnActiveService(){}
**************************************************************************/
#pragma once

#ifndef _JxdSockBasic_Lib
#define _JxdSockBasic_Lib

#ifdef linux
#include <arpa/inet.h>
#include <errno.h>
#define closesocket close
int xdGetSocketError();
#endif

#ifdef WIN32
#define xdGetSocketError WSAGetLastError
#endif

#include "JxdTypedef.h"
#include "JxdMacro.h"

#include <vector>
using std::vector;

enum TxdSocketCloseState { scsNone/*无需要处理*/, scsServer/*服务器主动关闭*/, scsServerError/*服务操作异常*/,
                           scsClient/*客户端主动关闭*/, scsClientError/*客户端发生错误*/};

//Socket 地址
struct TxdSocketAddr: public sockaddr_in
{
    TxdSocketAddr(const unsigned long &AIP = 0, const unsigned short int APort = 0, bool bhtons = true)
    {
        sin_family = AF_INET;
        sin_port = bhtons ? htons( APort ) : APort; 
        sin_addr.s_addr = AIP;
    }
};

//基本Socket类, 作为服务端处理
class CxdSocketBasic
{
//绑定信息
    struct TxdServerBindInfo
    {
        TxdServerBindInfo( unsigned long AIP = 0, unsigned short int APort = 0){ IP = AIP; Port = APort; s = 0; }
        unsigned long IP;
        unsigned short int Port;
        SOCKET s;
    };
public:
    //构造
    CxdSocketBasic();
    virtual ~CxdSocketBasic();

    //绑定信息
    bool AddLocal(unsigned short int APort);
    bool AddBindInfo(const char *ApIP, unsigned short int APort);
    bool AddBindInfo(unsigned long AIP, unsigned short int APort);
    bool DeleteBindInfo(unsigned long AIP, unsigned short int APort);
    bool GetBindInfo(const int &AIndex, unsigned long &AIP, unsigned short int &APort);
    void ClearBindInfo(void);
    int  GetBindCount(void);

    PropertyActive( ActiveService(), UnActiveService() );

    //Socket 属性
    PropertyValue( BlockSocketOpt, bool, (!m_bActive) );
    PropertyValue( ReUseAddr, bool, (!m_bActive) );
    PropertyValue( AutoIncPort, bool, (!m_bActive) );
    //系统 发送 接收 缓存
    PropertyValue( SysSendBufferSize, int, (!m_bActive) );
    PropertyValue( SysRecvBufferSize, int, (!m_bActive) );

    PropertyGetValue( ActiveTime, time_t );
protected:
    //BindInfo
    vector<TxdServerBindInfo> m_ltBindInfo;
    bool IsExistsBinds(unsigned long AIP, unsigned short int APort, bool bDelete);
    void CreateBindSocks();
    void SettingSocketBuffer(int ASocket);

    //子类实现
    virtual bool    ActiveService();
    virtual bool    UnActiveService();
    virtual SOCKET  CreateSocket(int AIndex) = 0;
    virtual void    DoSocketClosed(SOCKET s) { };
    virtual void    DoBindSockefFinished(SOCKET ASocket, int AIndex, bool AIsOK);
};

//数据统计统计
struct TxdSocketStat
{
    UINT64 OnlineSocketCount;
};


#endif