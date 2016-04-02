/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdUdpOnlineServer.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-11-21 14:33:19
	LastModify : 
	Description: UDP在线管理类, 统计在线人数, 实现命令:
        CtCmd_Register, CtCmd_Login, CtCmd_Logout, CtCmd_Heartbeat, CtCmd_Hello
**************************************************************************/
#pragma once;
#include "JxdUdpIOHandle.h"
#include "JxdUdpCmdDefines.h"
#include "JxdMacro.h"
#include "JxdMem.h"

//在线用户管理
#pragma pack(1)
struct TxdOnlineUserInfo
{
    DWORD  UserID;
    SOCKET Socket;
    DWORD  LocalIP;
    DWORD  SafeCode;
    WORD   LocalPort;
    UINT64 LastActiveTime;
    TxdSocketAddr RemoteAddr;
    char ClientHash[CtHashLength];
};
#pragma pack()
static const int CtOnlineUserInfoSize = sizeof(TxdOnlineUserInfo);

class CxdUdpOnlineServer: public CxdUdpIOHandle
{
public:
    CxdUdpOnlineServer();
    ~CxdUdpOnlineServer();
    
    //设置都需要在服务启动之前
    //在线管理HASH表设置
    PropertyValue( OnlineHashTableCount, UINT, !GetActive() );
    PropertyValue( OnlineInitNodeCount, UINT, !GetActive() );

    //用户无活动时,自己超时: 单位秒
    PropertyValue( UserUnActiveTimeout, UINT, true );
    
    //命令获取
    PropertyGetValue( RegisterCmd, UINT );
    PropertyGetValue( ClientLoginCmd, UINT );
    PropertyGetValue( ClientLogoutCmd, UINT );
    PropertyGetValue( HeartbeatCmd, UINT );
    PropertyGetValue( ReLoginCmd, UINT );
    PropertyGetValue( HelloCmd, UINT );
    
    inline UINT GetOnlineUserCount(void) { if (m_pOnlineManage != NULL) return m_pOnlineManage->GetCount(); return 0; }
protected:
    //处理父类信息
    virtual bool ActiveService();
    virtual bool UnActiveService();
    void OnRecvBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);

    //命令处理
    void DoRegisterCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoClientLoginCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoClientLogoutCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoHeartbeatCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoHelloCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    //
    //其它命令由子类处理, 返回FALSE表示子类不认识此命令
    virtual bool DoHandleClientCmd(const WORD &ACmdID, SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen) { return true; }

    //在线管理具体业务由子类处理
    virtual bool IsCanRegister(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, TCmdRegister *ApCmdRecv) { return true; }
    virtual bool IsCanLogin(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, TCmdLogin *ApCmdRecv) { return true; }
    virtual void DoUserLogin(TxdOnlineUserInfo *ApUserInfo) {}
    virtual void DoUserLogout(TxdOnlineUserInfo *ApUserInfo) {}
    virtual bool DoUserTimeout(TxdOnlineUserInfo *ApUserInfo) { return true; } //返回 FALSE 表示不删除

    //var
    //online manage
    CxdMutex m_mtOnline;
    CxdHashArray *m_pOnlineManage;
    TxdOnlineUserInfo* FindOnlineUser(const DWORD &AUserID, bool bUpdateTime = true);
private:
    //regist
    DWORD m_dwCurRegistID;
    CxdMutex m_mtRegist;
    DWORD GetRegistID(void);

    CxdFixedMemoryManage *m_pOnlineMem;

    //check thread
    CxdThreadEvent<LPVOID>* m_pThreadCheck;
    void DoThreadCheckOnlineUser(LPVOID);
    void FreeCheckThread(void);

    //cmd state
    void InitStateCmdValue(void);
};