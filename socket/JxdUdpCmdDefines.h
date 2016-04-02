#pragma once;

/*                      命令定义           */

//客户端注册用户ID, 同步命令
const WORD CtCmd_Register         = 10;
const WORD CtCmdReply_Register    = 11;

//客户端登录服务器, 同步命令
const WORD CtCmd_Login            = 100;
const WORD CtCmdReply_Login       = 101;
const WORD CtCmd_ClientReLogin    = 102;

//客户端退出服务器
const WORD CtCmd_Logout           = 65500;

//心跳包
const WORD CtCmd_Heartbeat        = 58732;

//Hello包(简单用于确认)
const WORD CtCmd_Hello            = 55555;

/*   其它  */
const WORD CtOnlineServerID       = 889;
const UINT CtClientMinRegID       = 19999;
const int  CtHashLength           = 16;
const WORD CtClientDefaultPort    = 11524;

#pragma pack(1)

struct TCmdHead 
{
    WORD  FCmdID;
    DWORD FUserID;
};

//注册
struct TCmdRegister: public TCmdHead
{
    TCmdRegister(const DWORD &AUserID){ FCmdID = CtCmd_Register; FUserID = AUserID; }
    char FClientHash[CtHashLength];
    static inline bool IsSameSize(const int &ASize) { return ASize == sizeof(TCmdRegister); }
};
struct TCmdReplyRegister: public TCmdHead
{
    TCmdReplyRegister(const DWORD &AUserID) { FCmdID = CtCmdReply_Register; FUserID = AUserID; }
    byte FReplySign;
    DWORD FRegID;
    static inline bool IsSameSize(const int &ASize) { return ASize == sizeof(TCmdReplyRegister); }
};

//登录
struct TCmdLogin: public TCmdHead
{
    TCmdLogin(const DWORD &AUserID) { FCmdID = CtCmd_Login; FUserID = AUserID; }
    WORD FClientVersion;
    DWORD FLocalIP;
    WORD  FLocalPort;
    char  FClientHash[CtHashLength];
    static inline bool IsSameSize(const int &ASize) { return ASize == sizeof(TCmdLogin); }
};
struct TCmdReplyLogin: public TCmdHead
{
    TCmdReplyLogin(const DWORD &AUserID) { FCmdID = CtCmdReply_Login; FUserID = AUserID; }
    byte FRelaySign;
    DWORD FPublicIP;
    WORD  FPublicPort;
    DWORD FSafeCode;
    static inline bool IsSameSize(const int &ASize) { return ASize == sizeof(TCmdReplyLogin); }
};
struct TCmdClientReLogin: public TCmdHead 
{
    TCmdClientReLogin(const DWORD &AUserID) { FCmdID = CtCmd_ClientReLogin; FUserID = AUserID; }
    static inline bool IsSameSize(const int &ASize) { return ASize == sizeof(TCmdClientReLogin); }
};

//退出
struct TCmdLogout: public TCmdHead
{
    TCmdLogout(const DWORD &AUserID) { FCmdID = CtCmd_Logout; FUserID = AUserID; }
    DWORD FSafeCode;
    static inline bool IsSameSize(const int &ASize) { return ASize == sizeof(TCmdLogout); }
};

//心跳
struct TCmdHeartbeat: public TCmdHead
{
    TCmdHeartbeat(const DWORD &AUserID) { FCmdID = CtCmd_Heartbeat; FUserID = AUserID; }
    bool FNeedReply;
    static inline bool IsSameSize(const int &ASize) { return ASize == sizeof(TCmdHeartbeat); }
};

//Hello
struct TCmdHello: public TCmdHead
{
    TCmdHello(const DWORD &AUserID) { FCmdID = CtCmd_Hello; FUserID = AUserID; }
    static inline bool IsSameSize(const int &ASize) { return ASize == sizeof(TCmdHello); }
};
#pragma pack()