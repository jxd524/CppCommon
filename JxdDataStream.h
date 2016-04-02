/**************************************************************************
Copyright      : pp.cc ����ָ��ʱ�� ɺ����
File           : JxdDataStream.h & .cpp 
Author         : Terry
Email          : jxd524@163.com
CreateTime     : 2012-5-17 13:50
LastModifyTime : 2013-10-23 13:50
Description    : 

���̳з�ʽ: 
CxdStreamBasic 
|->CxdMemoryHandle
|        |->CxdOuterMemory
|        |->CxdDynamicMemory
|        |->CxdShareMemroy
|        |->CxdMemoryFile
|->CxdFileStream
**************************************************************************/
#pragma once

#include <string>
using std::string;
using std::wstring;

#ifdef linux
#include <sys/ipc.h>
#include "JxdTypedef.h"
#define INVALID_HANDLE_VALUE NULL
#endif

//�������쳣�ࣨ���������׳�)
class CxdStreamEexception
{
public:
    CxdStreamEexception(const std::string &AErrorInfo = "xdStreamIO Error" ): m_pErrorInfo( AErrorInfo ) {}
    ~CxdStreamEexception() {}

    const std::string& GetErrorInfo() { return m_pErrorInfo; }
private:
    std::string m_pErrorInfo;
};

//====================================================================
// ���������������࣬����ʵ�������
//====================================================================
//
class CxdStreamBasic
{
public:
    /*��ȡ����*/
    virtual void   ReadLong(LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos = true);
    inline byte    ReadByte(void){ byte b; ReadLong( &b, 1 ); return b; }
    inline char    ReadChar(void) { char c; ReadLong( &c, sizeof(char) ); return c; }
    inline wchar_t ReadWChar(void) { char c; ReadLong( &c, sizeof(wchar_t) ); return c; }
    inline WORD    ReadWord(void){ WORD w; ReadLong( &w, 2 ); return w; }
    inline int     ReadInteger(void){ int i; ReadLong( &i, 4 ); return i; }
    inline DWORD   ReadDword(void){ DWORD dw; ReadLong( &dw, 4); return dw; }
    inline INT64   ReadInt64(void){ INT64 i; ReadLong( &i, sizeof(i)); return i; }
    //��ȡ1λ���ȣ��ٶ���Ӧ���ȵ��ַ���, ���ص� char* ��Ҫ�������ֹ��ͷ� delete []p;
    bool           ReadStringA(char *&ApBuf, byte &nLen);
    bool           ReadStringA(string &AValue);
    bool           ReadStringW(wchar_t *&ApBuf, byte &nLen);
    bool           ReadStringW(wstring &AValue);
    //��ȡ2λ���ȣ��ٶ���Ӧ���ȵ��ַ���, ���ص� char* ��Ҫ�������ֹ��ͷ� delete []p;
    bool           ReadStringAEx(char *&ApBuf, WORD &nLen);
    bool           ReadStringAEx(string &AValue);
    bool           ReadStringWEx(wchar_t *&ApBuf, WORD &nLen);
    bool           ReadStringWEx(wstring &AValue);

    /*����д��*/
    virtual void WriteLong(const LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos = true );
    inline void  WriteByte(const byte &AValue){ WriteLong( (LPVOID)&AValue, 1); }
    inline void  WriteChar(const char &AValue){ WriteLong( (LPVOID)&AValue, 1); }
    inline void  WriteWChar(const wchar_t &AValue){ WriteLong( (LPVOID)&AValue, sizeof(wchar_t) ); }
    inline void  WriteWord(const WORD &AValue){ WriteLong( (LPVOID)&AValue, 2); }
    inline void  WriteInteger(const int &AValue){ WriteLong( (LPVOID)&AValue, 4); }
    inline void  WriteDword(const DWORD &AValue) { WriteLong( (LPVOID)&AValue, 4); }
    inline void  WriteInt64(const INT64 &AValue){ WriteLong( (LPVOID)&AValue, sizeof(AValue) ); }
    //д��1λ���ȣ���д��Ӧ���ȵ��ַ���
    inline void  WriteStringA(const char *ApBuf, byte nLen){ WriteByte(nLen), WriteLong( (LPVOID)ApBuf, nLen); }
    inline void  WriteStringA(const string &AValue) { WriteStringA( AValue.c_str(), (byte)AValue.length() ); }
    inline void  WriteStringW(const wchar_t *ApBuf, byte nLen){ WriteByte(nLen * 2), WriteLong( (LPVOID)ApBuf, nLen * 2); }
    inline void  WriteStringW(const wstring &AValue) { WriteStringW( AValue.c_str(), (byte)AValue.length() ); }

    //д��2λ���ȣ���д����Ӧ���ȵ��ַ���
    inline void  WriteStringAEx(const char *ApBuf, WORD nLen){ WriteWord(nLen); WriteLong( (LPVOID)ApBuf, nLen); }
    inline void  WriteStringAEx(const string &AValue) { WriteStringAEx( AValue.c_str(), (WORD)AValue.length() ); }
    inline void  WriteStringWEx(const wchar_t *ApBuf, WORD nLen){ WriteWord(nLen * 2); WriteLong( (LPVOID)ApBuf, nLen * 2); }
    inline void  WriteStringWEx(const wstring &AValue) { WriteStringWEx( AValue.c_str(), (WORD)AValue.length() ); }

    //MFC֧��
#if defined (WIN32) && (_MFC_VER)
    inline void  WriteString(const CString &AValue) { 
#ifdef UNICODE
        WriteStringW( AValue, (byte)AValue.GetLength() );
#else
        WriteStringA( AValue, (byte)AValue.GetLength()); 
#endif
    }
    inline void  WriteStringEx(const CString &AValue) { 
#ifdef UNICODE
        WriteStringWEx( AValue, (WORD)AValue.GetLength() );
#else
        WriteStringAEx( AValue, (WORD)AValue.GetLength() ); 
#endif
    }
#endif
    /*�������*/
    virtual void Clear(void) = 0;
    /*������*/
    virtual bool CopyStream(CxdStreamBasic &ASrcStream);

    /*������*/
    virtual INT64 GetPos(void) { return 0; }
    virtual bool  SetPos(const INT64 &APos){ return false; }
    virtual INT64 GetSize(void) { return 0; }
    virtual bool  SetSize(const INT64 &ASize){ return false; }
    virtual INT64 GetCapacity(void) { return 0; }
    virtual bool  SetCapacity(const INT64 &ACapacity){ return false; }

    virtual ~CxdStreamBasic() {}
public:
    /*��ģ�嶨��*/
    //���ز����� << д�����
    template<typename T> 
    CxdStreamBasic& operator << (const T &t){ WriteLong( (LPVOID)&t, sizeof(T) ); return *this; }
    CxdStreamBasic& operator << (const std::string &AValue){ WriteStringAEx(AValue); return *this; }
    CxdStreamBasic& operator << (const std::wstring &AValue){ WriteStringWEx(AValue); return *this; }

    //���ز����� >> ��ȡ����
    template<typename T>
    CxdStreamBasic& operator >> (T &t){ ReadLong( &t, sizeof(T) ); return *this; }
    //����  Ĭ������λ��ʾ��С
    CxdStreamBasic& operator >> (std::string &AValue){ ReadStringAEx(AValue); return *this; }
    CxdStreamBasic& operator >> (std::wstring &AValue){ ReadStringWEx(AValue); return *this; }
protected:
    /*���ݶ�д����*/
    virtual int ReadStream(LPVOID ApBuffer, const UINT AByteCount) = 0;
    virtual int WriteStream(const LPVOID ApBuffer, const UINT AByteCount) = 0;

    void RaiseReadError(void){ throw CxdStreamEexception( "read stream error" ); }
    void RaiseWriteError(void){ throw CxdStreamEexception( "write stream error" ); }

    virtual inline bool CheckReadSize(const UINT &AByteCount){ return AByteCount + GetPos() <= GetSize(); }
    virtual inline bool CheckWriteSize(const UINT &AByteCount){ return AByteCount + GetPos() <= GetCapacity(); }
};

//====================================================================
// CxdMemoryHandle �ڴ洦���м���
//====================================================================
//
class CxdMemoryHandle: public CxdStreamBasic
{
public:
    CxdMemoryHandle();
    char* GetCurAddress(void){ return m_pMemory + m_nPos; }
    char* Memory(void){ return m_pMemory; }

    virtual void Clear(void); 

    inline INT64 GetPos(void) { return m_nPos; }
    inline INT64 GetSize(void) { return m_nSize; }
    inline INT64 GetCapacity(void) { return m_nCapacity; }
    inline bool SetPos(const INT64 &APos) { return APos <= m_nSize ? m_nPos = APos, true : false; }
    inline bool SetSize(const INT64 &ASize){ return ASize <= m_nCapacity ? m_nSize = ASize, true : false; }
protected:
    char *m_pMemory;
    INT64 m_nSize;
    INT64 m_nPos;
    INT64 m_nCapacity;
    virtual int ReadStream(LPVOID ApBuffer, const UINT AByteCount);
    virtual int WriteStream(const LPVOID ApBuffer, const UINT AByteCount);
private:
};

//====================================================================
// CxdOuterMemory���������ⲿ������
//====================================================================
//
class CxdOuterMemory: public CxdMemoryHandle
{
public:
    void InitMemory(char* ApBuffer, const INT64 &ASize)
        { m_pMemory = ApBuffer; m_nSize = ASize; m_nPos = 0; m_nCapacity = m_nSize; }
};

//====================================================================
// CxdDynamicMemory��̬�����ڴ�,����ָ���Ƿ��Զ�����
//====================================================================
//
class CxdDynamicMemory: public CxdMemoryHandle
{
public:
    CxdDynamicMemory(const UINT &ASize = 0);
    ~CxdDynamicMemory();

    void InitMemory(const UINT &ASize);
    void Clear();
    void FreeMemory(void);

    virtual bool SetCapacity(const INT64 &ACapacity);
    void WriteLong(const LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos = true );

    bool GetAutoIncMem(void){ return m_bAutoIncMem; }
    void SetAutoIncMem(bool bAutoInc){ m_bAutoIncMem = bAutoInc; }
protected:
    bool m_bAutoIncMem;
private:
    void CheckDynMem(const int AWriteByteCount);
};

//====================================================================
// �ڴ�ӳ���ļ����������ļ������
//====================================================================
//
class CxdMemoryFile: public CxdMemoryHandle
{
public:
    CxdMemoryFile(const char *ApFileName, const char *ApShareName = NULL, bool ACreateAlways = false, const bool AOnlyRead = false );
    ~CxdMemoryFile();
    //ӳ��ָ��λ�õ��ڴ�, ABeginPos: Window����Ҫ�Ƿ������ӣ�65536���ı���, Linux����Ҫ��ҳ(һ��Ϊ4K)�ı���
    bool MapFileToMemory(const INT64 ABeginPos, const DWORD AMapSize);
    //д���ļ�
    bool Flush(DWORD ASize=0);
    //�ļ�����
    INT64 GetCurMapBeginPos(void) const { return m_nCurMapPos; } //��ǰӳ���ļ���λ��
    INT64 GetFileSize(void);
    bool  GetFileSize(DWORD &AdwHigh, DWORD &AdwLow);
    bool  SetFileSize(const INT64 &ANewFileSize);
private:
    void CloseMap(void);

#ifdef WIN32
    //Windows
    HANDLE m_hFileMap;
    HANDLE m_hFile;
    bool CreateMap(void);
#endif
#ifdef linux
    //Linux
    int m_hFile;
#endif
    
    string m_strFileName, m_strShareName;
    INT64 m_nCurMapPos;
    bool m_bIsReadOnly;
};

//====================================================================
// �����ڴ���, ���������ļ������
//====================================================================
//
class CxdShareMemroy: public CxdMemoryHandle
{
public:
    CxdShareMemroy();
    ~CxdShareMemroy();

    void Clear();
    void FreeMemory(void);

    //InitMemory
#ifdef WIN32
    //Windows
    bool InitMemory(const DWORD &ASize, LPCTSTR lpName);
private:
    HANDLE m_hFile;
    string m_strShareName;
#endif

#ifdef linux
    //Linux
    bool InitMemory(const size_t &ASize, key_t AExistKey = IPC_PRIVATE);
private:
    int m_nShmID;
    key_t m_nExistKey;
#endif
};

//====================================================================
// ��ͨ�ļ�����
//====================================================================
//
enum TFileSeekType{ stBegin = 0, stCurrent = 1, stEnd = 2};
class CxdFileStream: public CxdStreamBasic
{ 
public:
    CxdFileStream(const char *ApFileName, bool ACreateAlways = false);
    ~CxdFileStream();
#ifdef WIN32
    CxdFileStream(const wchar_t *ApFileName, bool ACreateAlways = false);
#endif

    inline bool IsOpenSuccess(void) { return m_hFile != INVALID_HANDLE_VALUE; }
    inline void Clear(void){  };
    void        Flush(void);

    INT64 GetFileSize(void);
    bool  SetFileSize(const INT64 &ANewFileSize);

    size_t FileSeek(INT64 AOffset, TFileSeekType ASeekType = stCurrent);
    size_t FileWrite(const void *pBuf, DWORD AWriteBytesNum);
    size_t FileRead(void *pBuf, DWORD AReadBytesNum);

    //����ʵ��
    virtual INT64 GetSize(void) { return GetFileSize(); }
    virtual bool  SetSize(const INT64 &ANewFileSize) { return SetFileSize(ANewFileSize); }
    virtual inline INT64 GetPos(void){ return FileSeek(0); }
    virtual inline bool  SetPos(const INT64 &APos){ FileSeek(APos, stBegin); return true; }

    virtual void  ReadLong(LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos = true);
    virtual void  WriteLong(const LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos = true );
protected: 
    virtual int ReadStream(LPVOID ApBuffer, const UINT AByteCount){ return 0; };
    virtual int WriteStream(const LPVOID ApBuffer, const UINT AByteCount){ return 0; };
private:
    string m_strFileName;

#ifdef WIN32
    HANDLE m_hFile;
#endif
#ifdef linux
    FILE *m_hFile;
#endif
};