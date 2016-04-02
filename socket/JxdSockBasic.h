/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdSockInterface.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-22 16:38:16
	LastModify : 
	Description: 
        TServerBindInfo: Ϊ����󶨱����������õĽṹ��
        TSocketAddr: ����ʹ�õ������ַ
        CxdSocketBasic: Socket������, �ṩ��TCP, UDP����ʹ��
        ��Ҫʵ�ַ���:
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

enum TxdSocketCloseState { scsNone/*����Ҫ����*/, scsServer/*�����������ر�*/, scsServerError/*��������쳣*/,
                           scsClient/*�ͻ��������ر�*/, scsClientError/*�ͻ��˷�������*/};

//Socket ��ַ
struct TxdSocketAddr: public sockaddr_in
{
    TxdSocketAddr(const unsigned long &AIP = 0, const unsigned short int APort = 0, bool bhtons = true)
    {
        sin_family = AF_INET;
        sin_port = bhtons ? htons( APort ) : APort; 
        sin_addr.s_addr = AIP;
    }
};

//����Socket��, ��Ϊ����˴���
class CxdSocketBasic
{
//����Ϣ
    struct TxdServerBindInfo
    {
        TxdServerBindInfo( unsigned long AIP = 0, unsigned short int APort = 0){ IP = AIP; Port = APort; s = 0; }
        unsigned long IP;
        unsigned short int Port;
        SOCKET s;
    };
public:
    //����
    CxdSocketBasic();
    virtual ~CxdSocketBasic();

    //����Ϣ
    bool AddLocal(unsigned short int APort);
    bool AddBindInfo(const char *ApIP, unsigned short int APort);
    bool AddBindInfo(unsigned long AIP, unsigned short int APort);
    bool DeleteBindInfo(unsigned long AIP, unsigned short int APort);
    bool GetBindInfo(const int &AIndex, unsigned long &AIP, unsigned short int &APort);
    void ClearBindInfo(void);
    int  GetBindCount(void);

    PropertyActive( ActiveService(), UnActiveService() );

    //Socket ����
    PropertyValue( BlockSocketOpt, bool, (!m_bActive) );
    PropertyValue( ReUseAddr, bool, (!m_bActive) );
    PropertyValue( AutoIncPort, bool, (!m_bActive) );
    //ϵͳ ���� ���� ����
    PropertyValue( SysSendBufferSize, int, (!m_bActive) );
    PropertyValue( SysRecvBufferSize, int, (!m_bActive) );

    PropertyGetValue( ActiveTime, time_t );
protected:
    //BindInfo
    vector<TxdServerBindInfo> m_ltBindInfo;
    bool IsExistsBinds(unsigned long AIP, unsigned short int APort, bool bDelete);
    void CreateBindSocks();
    void SettingSocketBuffer(int ASocket);

    //����ʵ��
    virtual bool    ActiveService();
    virtual bool    UnActiveService();
    virtual SOCKET  CreateSocket(int AIndex) = 0;
    virtual void    DoSocketClosed(SOCKET s) { };
    virtual void    DoBindSockefFinished(SOCKET ASocket, int AIndex, bool AIsOK);
};

//����ͳ��ͳ��
struct TxdSocketStat
{
    UINT64 OnlineSocketCount;
};


#endif