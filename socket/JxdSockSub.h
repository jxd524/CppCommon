/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdSockSub.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-9-5 16:38:20
	LastModify : 
	Description: 网络相关函数
**************************************************************************/
#pragma once
#ifndef _JxdSockSub_Lib
#define _JxdSockSub_Lib

#ifdef linux
#include <arpa/inet.h>
#endif

#ifdef WIN32
bool StartWinSock(const WORD &AVersion = MAKEWORD(2, 2) );
void CleanWinSock(void);
#endif

void SetSocketBlock(int ASocket, bool AIsBlock = false); //阻塞与否
bool SetSocketReUseAddr(int ASocket, bool AIsReUse = true); //地址(端口)复用或独占端口
bool SetSocketExclusitveAddr(int ASocket);//独占地址
bool SetSocketSysRecvBuffer(int ASocket, int ASize); //接收缓存
bool SetSocketSysSendBuffer(int ASocket, int ASize); //发送缓存
int  SetSocketForceClose(int ASocket); //设置当 closeSocket时 强制关闭SOCKET
bool GetLocalIPs(in_addr *ApIPs, const int &AIPsCount, int &ARecvCount);
bool GetIPsByName(const char *ApURL, in_addr *ApIPs, const int &AIPsCount, int &ARecvCount);

#endif