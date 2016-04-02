#include "stdafx.h"
#include "JxdSockSub.h"
#include "JxdMacro.h"

#ifdef linux
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>
#endif


void SetSocketBlock( int ASocket, bool AIsBlock )
{
#ifdef linux
    int nFlags = fcntl( ASocket, F_GETFL, 0 );
    nFlags = AIsBlock ? ( nFlags & ~O_NONBLOCK) : ( nFlags | O_NONBLOCK );
    fcntl( ASocket, F_SETFL, nFlags );
#endif
#ifdef WIN32
    u_long nBlock( AIsBlock ? 0 : 1 );
    ioctlsocket( ASocket, FIONBIO, &nBlock );
#endif
}

bool SetSocketReUseAddr( int ASocket, bool AIsReUse )
{
    int nReUser(1);
    return 0 == setsockopt( ASocket, SOL_SOCKET, AIsReUse ? SO_REUSEADDR : ((int)(~SO_REUSEADDR)), 
        (char*)&nReUser, sizeof(nReUser) );
}

bool SetSocketExclusitveAddr( int ASocket )
{
#ifdef linux
    return false;
#endif
#ifdef WIN32
    int nExclusitve(1);
    return 0 == setsockopt( ASocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&nExclusitve, sizeof(nExclusitve) );
#endif
}

bool SetSocketSysRecvBuffer( int ASocket, int ASize )
{
    return 0 == setsockopt( ASocket, SOL_SOCKET, SO_RCVBUF, (char*)&ASize, sizeof(ASize) );
}

bool SetSocketSysSendBuffer( int ASocket, int ASize )
{
    return 0 == setsockopt( ASocket, SOL_SOCKET, SO_SNDBUF, (char*)&ASize, sizeof(ASize) );
}

int SetSocketForceClose( int ASocket )
{
    struct  linger 
    {
        u_short l_onoff;                /* option on/off */
        u_short l_linger;               /* linger time */
    };
    struct linger lig;
    int iLen;
    lig.l_onoff=1;
    lig.l_linger=0;
    iLen=sizeof(struct linger);
    return setsockopt( ASocket,SOL_SOCKET,SO_LINGER,(char *)&lig,iLen );
}

bool GetLocalIPs( in_addr *ApIPs, const int &AIPsCount, int &ARecvCount )
{
#ifdef linux
    ARecvCount = 0;
    char buf[1024] = {0};
    ifconf cnf;
    cnf.ifc_buf = buf;
    cnf.ifc_len = 1024;

    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        return false;
    }
    ioctl( sockfd, SIOCGIFCONF, &cnf );    //获取所有接口信息
    close( sockfd );

    ifreq *pIq = (struct ifreq*)buf;
    if ( IsNull(pIq) ) return false;

    for( int i = (cnf.ifc_len/sizeof(struct ifreq)); i > 0; i-- )
    {
        if ( ARecvCount < AIPsCount )
        {
            ApIPs[ARecvCount] = ((struct sockaddr_in*)&(pIq->ifr_addr))->sin_addr;
            ARecvCount++;
        }
        pIq++;
    }
    return true;
#endif
#ifdef WIN32
    char HostNameBuf[255] = {0};
    gethostname( HostNameBuf, 255 );
    return GetIPsByName( HostNameBuf, ApIPs, AIPsCount, ARecvCount );
#endif
}

bool GetIPsByName( const char *ApURL, in_addr *ApIPs, const int &AIPsCount, int &ARecvCount )
{
    //res_init();
    ARecvCount = 0;
    hostent *pInfo = gethostbyname( ApURL );
    if ( pInfo == NULL )
    {
        return false;
    }
    while ( pInfo->h_addr_list[ARecvCount] != NULL )
    {
        if ( ARecvCount < AIPsCount )
        {
            ApIPs[ARecvCount].s_addr = *(u_long*)pInfo->h_addr_list[ARecvCount];
        }
        ARecvCount++;
    }
    return ARecvCount <= AIPsCount;
}

#ifdef WIN32
bool StartWinSock( const WORD &AVersion /*= MAKEWORD(2, 2) */ )
{
    WSAData _data;
    return 0 == WSAStartup( MAKEWORD(2, 2), &_data );
}

void CleanWinSock( void )
{
    WSACleanup();
}
#endif

