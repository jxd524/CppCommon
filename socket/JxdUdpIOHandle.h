/**************************************************************************
Copyright      : pp.cc ����ָ��ʱ�� ɺ����
File           : JxdUdpIOHandle.h & .cpp 
Author         : Terry
Email          : jxd524@163.com
CreateTime     : 2012-8-25 18:19
LastModifyTime : 2012-9-03 18:19
Description    : UDP���ݴ�����

IO��Э�鶨��: 
[������]: �汾(1) - �˰��ܳ���(2) - CRC32��(4) - ����Ϣ(1)[ǰ��λ��0�� ����λ: Э������IoCmd]
IoCmd: 0000 -> ��Ҫ�����ദ��
IoCmd: 0001 -> �����ط���ϰ�����(CtIoCmd_GetCombiPackage), ����: ��ϰ�ID(2) - �������Ϣ(n) - Package1,Package2...PackageN
IoCmd: 0010 -> ������ָ������ϰ�(CtIoCmd_NotFoundCombiPackage), ���ͷ��Ѿ�ɾ��, ����: ��ϰ�ID(2)

[��ϰ�]: �汾(1) - �˰��ܳ���(2) - CRC32��(4) - ����Ϣ(1)[ǰ��λ������CombiCount�� ����λ: ��˳��] - ��־CombiID(2) - ������

����:
1: ��ϰ�������ֻ�� 16 ����, ����: CtCombiPackageSize * 16
2: ������û�н��ж�������
**************************************************************************/

#pragma once

#include "JxdSockBasic.h"
#include "JxdSockDefines.h"
#include "JxdDataStruct.h"
#include "JxdCombiBuffer.h"
#include "JxdMem.h"

//ͳ�ƽ��ռ����͵����ݳ���
#define CountBufferLength

class CxdUdpIOHandle: public CxdSocketBasic
{
public:
    CxdUdpIOHandle();
    ~CxdUdpIOHandle();

    /*����*/
    int  SendBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, int ABufLen);

    /*���Զ���, ʱ�䵥λ: ��*/
    PropertyInterface( UdpRecv, IxdSocketEventManage );       //ʵ�� UDP read �ӿ�, �����Զ�����ӿ�,�ⲿ��Ӧ�����ͷ�
    PropertyValue( RecvQueueMaxCount, UINT, !GetActive() );   //������յ����ݵĶ�����󳤶�, ��С���ܻ�����Զ��������ݰ�
    PropertyValue( HandleIoThreadCount, UINT, !GetActive() ); //IO�����߳�
    PropertyValue( UnUsedMemTimeoutSpace, UINT, true );       //���ڴ�鲻��Ҫʹ��ʱ,�����ʱ��

    /*��ϰ��������*/
    PropertyValue( InitCombiPackageBufferCount, UINT, !GetActive() );  //��ʼ�������С: CtMaxCombiPackageBufferSize, ���ںϲ���ϰ�
    //1.������ϰ��������
    PropertyValue( SendCombiInitBufferCount, UINT, !GetActive() ); //���ͻ���ڵ��ʼ������ �����С CtUdpBufferMaxSize
    PropertyValue( SendCombiHashTableCount, UINT, !GetActive() );  //���ͻ���HASHͰ
    PropertyValue( SendCombiBufferExsitsTime, UINT, true );        //���ͻ�������ʱ��

    //2.������ϰ��������
    PropertyValue( RecvCombiInitBufferCount, UINT, !GetActive() );  //���ջ���ڵ��ʼ������: �����С CtPerCombiPackageBufferSize
    PropertyValue( RecvCombiHashTableCount, UINT, !GetActive() );   //���ջ���HASHͰ
    PropertyValue( RecvCombiBufferExsitsTime, UINT, true );         //���ջ�������ʱ��
    PropertyValue( RecvCombiMaxTimeoutCount, UINT, true );          //���ջ������ʱ����

    /*ֻ������*/
    PropertyGetValue( HandleIORunningThreadCount, long );

    /*ͳ��*/
#ifdef CountBufferLength
    PropertyGetValue( SendBufferLengthMB, UINT );      //��������MB
    PropertyGetValue( SendBufferLengthByte, UINT );    //��������С��MB����
#endif
    PropertyGetValue( SendPackageCount, UINT );        //���Ͷ���������
    PropertyGetValue( SendFailedPackageCount, UINT );  //����ʧ�ܰ�����
    PropertyGetValue( SendSinglePackageCount, UINT );  //���Ͷ���������
    PropertyGetValue( SendCombiPackageCount, UINT );   //������ϰ�����
    PropertyGetValue( LostCombiPackageCount, UINT );   //��ϰ���ȫ��������

#ifdef CountBufferLength
    PropertyGetValue( RecvBufferLengthMB, UINT );      //���������ܳ���
    PropertyGetValue( RecvBufferLengthByte, UINT );    //���������ܳ���
#endif
    PropertyGetValue( RecvPackageCount, UINT );        //���յ������ݳ���
    PropertyGetValue( RecvErrorIOProtocolCount, UINT );//���մ���IOЭ���
    PropertyGetValue( RecvSinglePackageCount, UINT );  //���յ�������
    PropertyGetValue( RecvCombiPackageCount, UINT );   //���յ���ϰ�
    PropertyGetValue( AutoThrowRecvPackage, UINT );    //�Զ��������յ����ݰ�����: ���ն���̫С
protected:
    //�����ʼ��
    void InitForServer(void); 
    void InitForClient(void); 

    //�����յ�һ�����������ݰ��Ǳ�����
    virtual void OnRecvBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen) = 0;

    //��������Ϣ
    bool   ActiveService();
    bool   UnActiveService();
    SOCKET CreateSocket(int AIndex){ return socket(AF_INET, SOCK_DGRAM, 0); }
    void   DoSocketClosed(SOCKET s);
    void   DoBindSockefFinished(SOCKET ASocket, int AIndex, bool AIsOK) { if (AIsOK) GetItUdpRecv()->RelateSocket( ASocket ); }

    //���麯��
    virtual IxdSocketEventManage* GetUdpRecvInterface(void) { return GetItUdpRecv(); }
private:
    //���յ�֪ͨ, ���ýӿڽ���ԭʼ����
    void __DoRecvBuffer(SOCKET ASocket);
    //��������, ����ԭʼ�ӿ�
    int __SendBuffer(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, const char* ApBuffer, int ABufferLen );

    //�������ݰ�
    int _SendPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApPackage, int ALen, bool AIsSingledPackage);
    int SendSinglePackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, WORD ABufLen, byte AIoCmd = 0);
    int SendCombiPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApSendBuffer, WORD ABufLen);

    //��������Э���
    //DoGetLostCombiBuffer: ��ȡ��ʧ�����ݰ�
    void SendNotFoundCombiPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, WORD ACombiID, byte ACombiCount);

    //����ԭʼ��UDP���ݰ�
    void DoHandleUdpPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    //������ϰ�
    void DoHandleCombiPackage(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen, 
        WORD ACombiID, byte ACombiCount, byte ACombiIndex);
    //������ඨ���Э��,�����Э��
    void DoHandlNetProtocolPackage(byte ACmdID, SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoRecvGetCombiPackageCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
    void DoRecvNotFoundCombiPackageCmd(SOCKET ASocket, const TxdSocketAddr &ARemoteAddr, char *ApRecvBuffer, int ABufLen);
private:
    void InitUdpIOHandleStateValue(void);
#ifdef CountBufferLength
    CxdMutex m_mtState; //ͳ�ư�ȫ
#endif
    //�ڴ�������ݽṹ
#pragma pack(1)
    struct TxdRecvPackageInfo 
    {
        SOCKET FSocket;
        unsigned long FRemoteIP;
        unsigned short int FRemotePort;
        WORD FBufferLength;
        char FBuffer[CtUdpBufferMaxSize];
    };
#pragma pack()
    //m_pRecvBufManage: ���� TxdRecvPackageInfo��С���ڴ��;
    CxdFixedMemoryManageEx *m_pRecvBufManage;
    CxdQueueEx<TxdRecvPackageInfo*> *m_pRecvPackageList;
    
    bool m_bWaitToClose;
    void DoThreadHandleIO(LPVOID);

    /*��ϰ�������ݽṹ*/
    //��ϰ��ڴ����, ��������
    //  1: ���͵���ϰ��ڴ� --> CtUdpBufferMaxSize
    //  2: ������ϰ��ڴ�   --> CtPerCombiPackageBufferSize
    //  3: �ϲ����յ���ϰ����ڴ�  --> CtMaxCombiPackageBufferSize
    CxdMutex m_mtInitCombiObject;
    CxdFixedMemoryManageEx *m_pCombiBufferManage; 
    CxdCombiSend *m_pCombiSendManage; //������ϰ����������
    CxdCombiRecv *m_pCombiRecvManage; //������ϰ����������
    void CreateCombiBufferManage(void);
    void CreateCombiSendManage(void);
    void CreateCombiRecvManage(void);

    //�¼�����
    void DoFreeCombiMem(LPVOID AMem); 
    void DoGetLostCombiBuffer(TxdRecvCombiBufferInfo* ApInfo);
    void DoRecvCombiBufferError(TxdRecvCombiBufferInfo* ApInfo);
};