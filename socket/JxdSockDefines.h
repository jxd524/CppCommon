#pragma once
#ifndef _JxdSockDefines_Lib
#define _JxdSockDefines_Lib

#include "JxdMacro.h"
#include "JxdEvent.h"

//路由相关
#define CtMTU        576           /*中国大部分路由MTU值; 避免IP分片, 可改大,但可能经过某些路由时会被丢弃*/
#define CtRawIpHead  20            /*TCP/IP协议中原始IP头*/
#define CtRawUdpHead 8             /*TCP/IP协议中原始UDP头*/

//UDP相关
#define CtUdpBufferMaxSize              (CtMTU - CtRawIpHead - CtRawUdpHead)                    /*每个UDP数据包最大长度*/
#define CtMaxCombiPackageCount          16                                                      /*最大组合包数量*/
#define CtSinglePackageHeadSize         8                                                       /*单独包包头长度*/
#define CtSinglePackageBufferSize       (CtUdpBufferMaxSize - CtSinglePackageHeadSize)          /*独立包最大数据区长度*/
#define CtCombiPackageHeadSize          10                                                      /*组合包包头长度 */
#define CtPerCombiPackageBufferSize     (CtUdpBufferMaxSize - CtCombiPackageHeadSize)           /*每个组合包最大数据区长度*/
#define CtMaxCombiPackageBufferSize     (CtPerCombiPackageBufferSize * CtMaxCombiPackageCount)  /*最多可发送组合包数据长度*/

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
//      添加到此类的 IxdSocketEventHandle 接口, 由外部创建,之后外部将不要再对其操作.
//          此接口由内部管理
//  对多个Socket进行管理, 多个Socket对同一种或几种事件进行处理.
//  如: esRead 事件, 先使用 AddSocketEvent 进行添加事件.
//      再使用 RelateSocket 关联 Socket, 之后, 当有可读事件( read )
//      则 IxdSocketEventHandle 接口被触发
//====================================================================
//
Interface(IxdSocketEventManage)
    virtual bool AddSocketEvent( TxdEventStyle AEventStyle, IxdSocketEventHandle *ApHandleInterface ) = 0;
    virtual bool RelateSocket( SOCKET ASocket ) = 0;
    virtual bool UnRelateSocket( SOCKET ASocket ) = 0;
    virtual void Clear(void) = 0;
InterfaceEnd

#endif