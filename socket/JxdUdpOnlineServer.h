/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdUdpOnlineServer.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-11-21 14:33:19
	LastModify : 
	Description: UDP���߹�����, ͳ����������, ʵ������:
        CtCmd_Register, CtCmd_Login, CtCmd_Logout, CtCmd_Heartbeat, CtCmd_Hello
**************************************************************************/
#pragma once;
#include "JxdUdpIOHandle.h"
#include "JxdUdpCmdDefines.h"
#include "JxdMacro.h"
#include "JxdMem.h"

//�����û�����
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
    
    //���ö���Ҫ�ڷ�������֮ǰ
    //���߹���HASH������
    PropertyValue( OnlineHashTableCount, UINT, !GetActive() );
    PropertyValue( OnlineInitNodeCount, UINT, !GetActive() );

    //�û��޻ʱ,�Լ���ʱ: ��λ��
    PropertyValue( UserUnActiveTimeout, UINT, true );
    
    //�����ȡ
    PropertyGetValue( RegisterCmd, UINT );
    PropertyGetValue( ClientLoginCmd, UINT );
    PropertyGetValue( ClientLogoutCmd, UINT );
    PropertyGetValue( HeartbeatCmd, UINT );
    PropertyGetValue( ReLoginCmd, UINT );
    PropertyGetValue( HelloCmd, UINT );
    
    inline UINT GetOnlineUserCount(void) { if (m_pOnlineManage != NULL) return m_pOnlineManage->GetCount(); return 0; }
protected:
    //��������Ϣ
    virtual bool ActiveService();
    virtual bool UnActiveService();
    void OnRecvBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);

    //�����
    void DoRegisterCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoClientLoginCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoClientLogoutCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoHeartbeatCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoHelloCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    //
    //�������������ദ��, ����FALSE��ʾ���಻��ʶ������
    virtual bool DoHandleClientCmd(const WORD &ACmdID, SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen) { return true; }

    //���߹������ҵ�������ദ��
    virtual bool IsCanRegister(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, TCmdRegister *ApCmdRecv) { return true; }
    virtual bool IsCanLogin(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, TCmdLogin *ApCmdRecv) { return true; }
    virtual void DoUserLogin(TxdOnlineUserInfo *ApUserInfo) {}
    virtual void DoUserLogout(TxdOnlineUserInfo *ApUserInfo) {}
    virtual bool DoUserTimeout(TxdOnlineUserInfo *ApUserInfo) { return true; } //���� FALSE ��ʾ��ɾ��

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