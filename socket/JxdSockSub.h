/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdSockSub.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-9-5 16:38:20
	LastModify : 
	Description: ������غ���
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

void SetSocketBlock(int ASocket, bool AIsBlock = false); //�������
bool SetSocketReUseAddr(int ASocket, bool AIsReUse = true); //��ַ(�˿�)���û��ռ�˿�
bool SetSocketExclusitveAddr(int ASocket);//��ռ��ַ
bool SetSocketSysRecvBuffer(int ASocket, int ASize); //���ջ���
bool SetSocketSysSendBuffer(int ASocket, int ASize); //���ͻ���
int  SetSocketForceClose(int ASocket); //���õ� closeSocketʱ ǿ�ƹر�SOCKET
bool GetLocalIPs(in_addr *ApIPs, const int &AIPsCount, int &ARecvCount);
bool GetIPsByName(const char *ApURL, in_addr *ApIPs, const int &AIPsCount, int &ARecvCount);

#endif