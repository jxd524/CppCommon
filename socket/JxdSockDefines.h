#pragma once
#ifndef _JxdSockDefines_Lib
#define _JxdSockDefines_Lib

#include "JxdMacro.h"
#include "JxdEvent.h"

//·�����
#define CtMTU        576           /*�й��󲿷�·��MTUֵ; ����IP��Ƭ, �ɸĴ�,�����ܾ���ĳЩ·��ʱ�ᱻ����*/
#define CtRawIpHead  20            /*TCP/IPЭ����ԭʼIPͷ*/
#define CtRawUdpHead 8             /*TCP/IPЭ����ԭʼUDPͷ*/

//UDP���
#define CtUdpBufferMaxSize              (CtMTU - CtRawIpHead - CtRawUdpHead)                    /*ÿ��UDP���ݰ���󳤶�*/
#define CtMaxCombiPackageCount          16                                                      /*�����ϰ�����*/
#define CtSinglePackageHeadSize         8                                                       /*��������ͷ����*/
#define CtSinglePackageBufferSize       (CtUdpBufferMaxSize - CtSinglePackageHeadSize)          /*�������������������*/
#define CtCombiPackageHeadSize          10                                                      /*��ϰ���ͷ���� */
#define CtPerCombiPackageBufferSize     (CtUdpBufferMaxSize - CtCombiPackageHeadSize)           /*ÿ����ϰ��������������*/
#define CtMaxCombiPackageBufferSize     (CtPerCombiPackageBufferSize * CtMaxCombiPackageCount)  /*���ɷ�����ϰ����ݳ���*/

#ifdef linux
#include "JxdTypedef.h"
#define FD_READ_BIT                     0
#define FD_WRITE_BIT                    1
#define FD_OOB_BIT                      2
#define FD_ACCEPT_BIT                   3
#define FD_CONNECT_BIT                  4
#define FD_CLOSE_BIT                    5
#define FD_QOS_BIT                      6
#define FD_GROUP_QOS_BIT                7
#define FD_ROUTING_INTERFACE_CHANGE_BIT 8
#define FD_ADDRESS_LIST_CHANGE_BIT      9
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (SOCKET)(~0)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR            (-1)
#endif

#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK                   10035L
#endif

class CxdSocketBasic;

typedef IxdNotifyEventT<SOCKET> IxdSocketEventHandle;

enum TxdEventStyle{esRead = FD_READ_BIT, esWrite = FD_WRITE_BIT, esOOB = FD_OOB_BIT, esAccept = FD_ACCEPT_BIT,
    esConnect = FD_CONNECT_BIT, esClose = FD_CLOSE_BIT, esQos = FD_QOS_BIT, esGroupQos = FD_GROUP_QOS_BIT,
    esRoutingInterfaceChange = FD_ROUTING_INTERFACE_CHANGE_BIT, esAddressListChange = FD_ADDRESS_LIST_CHANGE_BIT};

//====================================================================
// IxdSocketEvent
//      ��ӵ������ IxdSocketEventHandle �ӿ�, ���ⲿ����,֮���ⲿ����Ҫ�ٶ������.
//          �˽ӿ����ڲ�����
//  �Զ��Socket���й���, ���Socket��ͬһ�ֻ����¼����д���.
//  ��: esRead �¼�, ��ʹ�� AddSocketEvent ��������¼�.
//      ��ʹ�� RelateSocket ���� Socket, ֮��, ���пɶ��¼�( read )
//      �� IxdSocketEventHandle �ӿڱ�����
//====================================================================
//
Interface(IxdSocketEventManage)
    virtual bool AddSocketEvent( TxdEventStyle AEventStyle, IxdSocketEventHandle *ApHandleInterface ) = 0;
    virtual bool RelateSocket( SOCKET ASocket ) = 0;
    virtual bool UnRelateSocket( SOCKET ASocket ) = 0;
    virtual void Clear(void) = 0;
InterfaceEnd

#endif