#include "StdAfx.h"
#include "JxdDataStream.h"

#ifdef linux
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>
#endif

#ifdef WIN32
#include "JxdStringCoder.h"
#endif

void CxdStreamBasic::ReadLong( LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos /*= true*/ )
{
    if (ACount <= 0 ) return;

    if ( !CheckReadSize(ACount) )
    {
        //Size 无法满足读取
        RaiseReadError();
    }
    if ( ReadStream(ApBuffer, ACount) > 0 )
    {
        if ( AutoIncPos )
        {
            if ( !SetPos( GetPos() + ACount ) )
            {
                throw CxdStreamEexception( "SetPos error on ReadLong function" );
            }
        }
    }
    else
    {
        RaiseReadError();
    }
}

void CxdStreamBasic::WriteLong( const LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos /*= true */ )
{
    if (ACount <= 0 ) return;

    if ( !CheckWriteSize(ACount) )
    {
        RaiseWriteError();
    }
    if ( WriteStream(ApBuffer, ACount) > 0 )
    {
        SetSize( GetSize() + ACount );
        if ( AutoIncPos )
        {
            SetPos( GetPos() + ACount );
        }
    }
    else
    {
        RaiseWriteError();
    }
}

bool CxdStreamBasic::ReadStringA( char *&ApBuf, byte &nLen )
{
    INT64 nPos = GetPos();
    try
    {        
        nLen = ReadByte(); 
        ApBuf = new char[nLen]; 
        ReadLong( ApBuf, nLen ); 
        return true;
    }
    catch(...)
    {
        SetPos( nPos );
        return false;
    }
}

bool CxdStreamBasic::ReadStringA( string &AValue )
{
    char *pBuf = NULL;
    byte nLen;
    bool b = ReadStringA( pBuf, nLen );
    if ( b )
    {
        AValue.clear();
        AValue.append(pBuf, nLen);
    }
    if ( pBuf )
    {
        delete []pBuf;
        pBuf = NULL;
    }
    return b;
}

bool CxdStreamBasic::ReadStringAEx( char *&ApBuf, WORD &nLen )
{
    INT64 nPos = GetPos();
    try
    {        
        nLen = ReadWord();
        ApBuf = new char[nLen]; 
        ReadLong( ApBuf, nLen ); 
        return true;
    }
    catch(...)
    {
        SetPos( nPos );
        return false;
    }
}

bool CxdStreamBasic::ReadStringAEx( string &AValue )
{
    char *pBuf = NULL;
    WORD nLen;
    bool b = ReadStringAEx( pBuf, nLen );
    if ( b )
    {
        AValue.clear();
        AValue.append(pBuf, nLen);
    }
    if ( pBuf )
    {
        delete []pBuf;
        pBuf = NULL;
    }
    return b;
}

bool CxdStreamBasic::ReadStringW( wchar_t *&ApBuf, byte &nLen )
{
    INT64 nPos = GetPos();
    try
    {        
        nLen = ReadByte(); 
        ApBuf = new wchar_t[nLen]; 
        ReadLong( ApBuf, nLen  ); 
        nLen /= 2;
        return true;
    }
    catch(...)
    {
        SetPos( nPos );
        return false;
    }
}

bool CxdStreamBasic::ReadStringW( wstring &AValue )
{
    wchar_t *pBuf = NULL;
    byte nLen;
    bool b = ReadStringW( pBuf, nLen );
    if ( b )
    {
        AValue.clear();
        AValue.append(pBuf, nLen);
    }
    if ( pBuf )
    {
        delete []pBuf;
        pBuf = NULL;
    }
    return b;
}

bool CxdStreamBasic::ReadStringWEx( wchar_t *&ApBuf, WORD &nLen )
{
    INT64 nPos = GetPos();
    try
    {        
        nLen = ReadWord(); 
        ApBuf = new wchar_t[nLen]; 
        ReadLong( ApBuf, nLen ); 
        nLen /= 2;
        return true;
    }
    catch(...)
    {
        SetPos( nPos );
        return false;
    }
}

bool CxdStreamBasic::ReadStringWEx( wstring &AValue )
{
    wchar_t *pBuf = NULL;
    WORD nLen;
    bool b = ReadStringWEx( pBuf, nLen );
    if ( b )
    {
        AValue.clear();
        AValue.append(pBuf, nLen);
    }
    if ( pBuf )
    {
        delete []pBuf;
        pBuf = NULL;
    }
    return b;
}

bool CxdStreamBasic::CopyStream( CxdStreamBasic &ASrcStream )
{
    bool bOK = true;
    const int CtMaxCopySize = 1024 * 64;
    int nSize = (int)(ASrcStream.GetSize() - ASrcStream.GetPos());
    if ( nSize <= CtMaxCopySize )
    {
        char *pBuf = new char[nSize];
        try
        {
        	ASrcStream.ReadLong( pBuf, nSize );
            WriteLong( pBuf, nSize );
        }
        catch(...)
        {
            bOK = false;
        }
        delete []pBuf;
    }
    else
    {
        nSize = CtMaxCopySize;
        char *pBuf = new char[nSize];
        try
        {
            while ( nSize > 0 )
            {
                ASrcStream.ReadLong( pBuf, nSize );
                WriteLong( pBuf, nSize );
                nSize = (int)(ASrcStream.GetSize() - ASrcStream.GetPos());
                if ( nSize > CtMaxCopySize )
                {
                    nSize = CtMaxCopySize;
                }
            }
        }
        catch(...)
        {
            bOK = false;
        }
        delete []pBuf;
    }
    return bOK;
}

CxdMemoryHandle::CxdMemoryHandle()
{
    m_nPos = 0;
    m_nSize = 0;
    m_pMemory = NULL;
}

int CxdMemoryHandle::ReadStream( LPVOID ApBuffer, const UINT AByteCount )
{
    try
    {
        memcpy( ApBuffer, m_pMemory + m_nPos, AByteCount );
        return AByteCount;
    }
    catch (...)
    {
    	return 0;
    }
}

int CxdMemoryHandle::WriteStream( const LPVOID ApBuffer, const UINT AByteCount )
{
    try
    {
        memcpy( m_pMemory + m_nPos, ApBuffer, AByteCount );
        return AByteCount;
    }
    catch (...)
    {
        return 0;
    }
}

void CxdMemoryHandle::Clear( void )
{
    if(m_pMemory)
    { 
        memset(m_pMemory, 0, (size_t)m_nSize); 
        m_nPos = 0;
    }
}


CxdDynamicMemory::CxdDynamicMemory( const UINT &ASize /*= 0*/ )
{
    m_nCapacity = 0;
    m_bAutoIncMem = true;
    if ( ASize > 0 )
    {
        InitMemory( ASize );
    }
}

CxdDynamicMemory::~CxdDynamicMemory()
{
    FreeMemory();
}

bool CxdDynamicMemory::SetCapacity( const INT64 &ACapacity )
{
    if ( m_bAutoIncMem && ACapacity > 0 && m_nCapacity != ACapacity )
    {
        INT64 nNewSize = ACapacity;
        char *pTemp = new char[(unsigned int)nNewSize];
        memset( pTemp, 0, (unsigned int)nNewSize );
        if ( m_nSize > 0 && m_pMemory != NULL )
        {
            memcpy( pTemp, m_pMemory, (int)m_nSize );
        }
        if ( m_pMemory != NULL )
        {
            delete []m_pMemory;
        }        
        m_pMemory = pTemp;
        m_nCapacity = ACapacity;
        if ( m_nSize > ACapacity )
        {
            m_nSize = ACapacity;
        }
        return true;
    }
    return false;
}

void CxdDynamicMemory::InitMemory( const UINT &ASize )
{
    if ( m_pMemory )
    {
        delete []m_pMemory;
        m_pMemory = NULL;
    }
    m_pMemory = new char[ASize];
    memset( m_pMemory, 0, ASize );
    m_nSize = 0;
    m_nPos = 0;
    m_nCapacity = ASize;
}

void CxdDynamicMemory::Clear()
{
    m_nPos = 0;
    m_nSize = 0;
}

void CxdDynamicMemory::FreeMemory( void )
{
    if ( m_pMemory )
    {
        delete []m_pMemory;
        m_pMemory = NULL;
        m_nPos = 0;
        m_nSize = 0;
    }
}

void CxdDynamicMemory::WriteLong( const LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos /*= true */ )
{
    CheckDynMem( ACount );
    CxdMemoryHandle::WriteLong(ApBuffer, ACount, AutoIncPos );
}

void CxdDynamicMemory::CheckDynMem( const int AWriteByteCount )
{
    if ( AWriteByteCount + m_nPos > m_nCapacity )
    {      
        SetCapacity( m_nCapacity * 2 + AWriteByteCount );
    }
}

//CxdMemoryFile
CxdMemoryFile::CxdMemoryFile( const char *ApFileName, const char *ApShareName /*= NULL*/, bool ACreateAlways /*= false*/,
    const bool AOnlyRead /*= false */ )
{
    m_nCurMapPos = 0;
    m_bIsReadOnly = AOnlyRead;
    m_strFileName = ApFileName;
    if ( ApShareName != NULL )
    {
        m_strShareName = ApShareName;
    }

#ifdef WIN32
    DWORD dwStyle( ACreateAlways ? CREATE_ALWAYS : OPEN_ALWAYS );
    m_hFile = CreateFileA( ApFileName, m_bIsReadOnly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE, 
        m_bIsReadOnly ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, dwStyle, 0, 0 );
    if ( INVALID_HANDLE_VALUE == m_hFile )
        throw CxdStreamEexception( string("can not create file: ") + ApFileName );
    m_hFileMap = 0;
    CreateMap();
#endif

#ifdef linux
    struct stat t;
    if ( ACreateAlways || 0 != stat(ApFileName, &t) )
    {
        m_hFile = open( ApFileName, (AOnlyRead ? O_RDONLY : O_RDWR) | O_LARGEFILE | O_CREAT | O_APPEND, 
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
    }
    else
    {
        m_hFile = open( ApFileName, (AOnlyRead ? O_RDONLY : O_RDWR) | O_LARGEFILE );
    }
    
    if ( m_hFile < 0 )
        throw CxdStreamEexception( "can not open file for CxdMemoryFile" );
#endif

}

CxdMemoryFile::~CxdMemoryFile()
{
    Flush();
    CloseMap();
#ifdef WIN32
    if ( m_hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
    }
#endif
#ifdef linux
    if ( m_hFile )
    {
        close( m_hFile );
        m_hFile = 0;
    }
#endif

}

bool CxdMemoryFile::GetFileSize( DWORD &AdwHigh, DWORD &AdwLow )
{
#ifdef WIN32
    if ( INVALID_HANDLE_VALUE == m_hFile )
        return false;
    AdwLow = ::GetFileSize(m_hFile, &AdwHigh );
    return true;
#endif

#ifdef linux
    struct stat t;
    if ( 0 == stat(m_strFileName.c_str(), &t) )
    {
        size_t nFileSize = t.st_size;
        AdwLow = nFileSize;
        AdwHigh = nFileSize >> 32;
        return true;
    }
    return false;
#endif
}

INT64 CxdMemoryFile::GetFileSize( void )
{
    DWORD dwHigh, dwLow;
    return GetFileSize(dwHigh, dwLow) ? ( (INT64)dwHigh << 32 ) | dwLow : 0;
}

bool CxdMemoryFile::SetFileSize( const INT64 &ANewFileSize )
{
#ifdef WIN32
    if ( (m_hFile != INVALID_HANDLE_VALUE) && (ANewFileSize > 0) && (ANewFileSize != GetFileSize()) )
    {
        DWORD dwHigh(ANewFileSize >> 32), dwLow( (DWORD)ANewFileSize );
        LONG l( dwHigh );
        SetFilePointer(m_hFile, dwLow, dwHigh > 0 ? &l : NULL, FILE_BEGIN);
        SetEndOfFile( m_hFile );
        m_nSize = ANewFileSize;
        m_nCapacity = m_nSize;
        CloseMap();
        return true;
    }
#endif

#ifdef linux
    if ( m_hFile )
    {
        return ftruncate( m_hFile, ANewFileSize ) == 0;
    }
#endif
    return false;
}

#ifdef WIN32
bool CxdMemoryFile::CreateMap( void )
{
    CloseMap();
    DWORD dwHigh, dwLow;
    if ( GetFileSize(dwHigh, dwLow) )
    {
        DWORD dwProtect;
        m_bIsReadOnly ? dwProtect = PAGE_READONLY : dwProtect = PAGE_READWRITE;
        m_hFileMap = CreateFileMappingA(m_hFile, NULL, dwProtect, dwHigh, dwLow, 
            m_strShareName.empty() ? NULL : m_strShareName.c_str() );
        return m_hFileMap != 0;
    }
    return false;
}
#endif

void CxdMemoryFile::CloseMap( void )
{
#ifdef WIN32
    if ( m_hFileMap != 0 )
    {
        if (m_pMemory != NULL)
        {
            UnmapViewOfFile( m_pMemory );
            m_pMemory = NULL;
        }
        CloseHandle( m_hFileMap );
        m_hFileMap = 0;
    }
#endif

#ifdef linux
    if ( m_pMemory != NULL )
    {
        munmap( m_pMemory, m_nCapacity );
        m_pMemory = NULL;
    }
#endif
}

bool CxdMemoryFile::MapFileToMemory( const INT64 ABeginPos, const DWORD AMapSize )
{
    if ( GetFileSize() < ABeginPos + (INT64)AMapSize )
    {
        SetFileSize( ABeginPos + AMapSize );
    }
    CloseMap();
    INT64 nPos = ABeginPos;
    DWORD nMapSize = AMapSize;
#ifdef WIN32
    if (0 == m_hFileMap)
    {
        if ( !CreateMap() ) 
            return false;
    }
    //由外部自行计算, 根据分配粒子进行计算
    //SYSTEM_INFO SysInfo;
    //GetSystemInfo( &SysInfo );
    //DWORD dwSysGran = SysInfo.dwAllocationGranularity;
    //nPos = (ABeginPos / dwSysGran) * dwSysGran;
    DWORD nHighOffset = nPos >> 32;
    DWORD nLowOffset = (DWORD)nPos;
    //nMapSize = (ABeginPos % dwSysGran) + AMapSize;

    

    m_bIsReadOnly ?
        m_pMemory = (char*)MapViewOfFile( m_hFileMap, FILE_MAP_READ, 0 == nHighOffset ? NULL : nHighOffset, nLowOffset, nMapSize ) :
        m_pMemory = (char*)MapViewOfFile( m_hFileMap, FILE_MAP_ALL_ACCESS, 0 == nHighOffset ? NULL : nHighOffset, nLowOffset, nMapSize );
#endif

#ifdef linux
    int nProt = PROT_READ;
    if ( !m_bIsReadOnly ) nProt |= PROT_WRITE;

    int nFlags = m_strShareName.empty() ? MAP_PRIVATE : MAP_SHARED;
    if ( -1 == m_hFile )
    {
        nFlags |= MAP_ANON;
    }
    m_pMemory = (char*)mmap( NULL, nMapSize, nProt, nFlags, m_hFile, nPos );
    if ( MAP_FAILED == m_pMemory )
    {
        m_pMemory = NULL;
    }
#endif

    m_nCapacity = m_nSize = nMapSize > 0 ? nMapSize : GetFileSize();
    m_nCurMapPos = nPos;
    m_nPos = 0;
    return m_pMemory != NULL;
}

bool CxdMemoryFile::Flush( DWORD ASize/*=0*/ )
{
#ifdef WIN32
    if ( m_pMemory != NULL )
    {
        FlushViewOfFile( m_pMemory, ASize );
        return true;
    }
#endif

#ifdef linux
    if ( m_pMemory != NULL )
    {
        if ( ASize <= 0 )
        {
            ASize = m_nCapacity;
        }
        msync( m_pMemory, ASize, MS_SYNC );
        return true;
    }
#endif

    return false;
}


//CxdShareMemroy
CxdShareMemroy::CxdShareMemroy()
{
#ifdef linux
    m_nShmID = -1;
    m_nExistKey = IPC_PRIVATE;
#endif
#ifdef WIN32
    m_hFile = INVALID_HANDLE_VALUE;
#endif
    m_nCapacity = 0;
}

CxdShareMemroy::~CxdShareMemroy()
{
    FreeMemory();
}

void CxdShareMemroy::FreeMemory( void )
{
#ifdef WIN32
    if ( m_pMemory != NULL )
    {
        FlushViewOfFile( m_pMemory, (size_t)m_nSize );
    }
    if ( m_hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
    }
#endif

#ifdef linux
    if ( m_pMemory )
    {
        shmdt( m_pMemory );
        shmctl( m_nShmID, IPC_RMID, NULL );
    }
    m_nShmID = -1;
#endif
    m_pMemory = NULL;
    m_nPos = 0;
    m_nSize = 0;
}

void CxdShareMemroy::Clear()
{
    m_nPos = 0;
    m_nSize = 0;
}

#ifdef linux
bool CxdShareMemroy::InitMemory( const size_t &ASize, key_t AExistKey /*= IPC_PRIVATE*/ )
{
    FreeMemory();
    m_nShmID = -1;
    if ( AExistKey != IPC_PRIVATE )
    {
        m_nShmID = shmget( AExistKey, 0, 0 );
    }
    if ( -1 == m_nShmID )
    {
        m_nShmID = shmget( AExistKey, (size_t)ASize, IPC_CREAT );
    }
    if ( -1 == m_nShmID )
    {
        return false;
    }
    shmid_ds ds;
    shmctl( m_nShmID, IPC_STAT, &ds );
    m_nSize = m_nCapacity = ds.shm_segsz;
    m_pMemory = (char *)shmat( m_nShmID, NULL, 0 );
    return true;
}
#endif

#ifdef WIN32
bool CxdShareMemroy::InitMemory( const DWORD &ASize, LPCTSTR lpName )
{
    FreeMemory();
    m_hFile = CreateFileMapping( INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, ASize, lpName );
    m_pMemory = (char*)MapViewOfFile( m_hFile, FILE_MAP_ALL_ACCESS, 0, 0, ASize );
    m_nCapacity = m_nSize = ASize;
    return m_pMemory != NULL;
}
#endif

//CxdFileStream
CxdFileStream::CxdFileStream( const char *ApFileName, bool ACreateAlways /*= false*/ )
{
    m_strFileName = ApFileName;

#ifdef WIN32
    m_hFile = CreateFileA( ApFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
        NULL, ACreateAlways ? CREATE_ALWAYS : OPEN_ALWAYS, 0, 0 );
#endif

#ifdef linux
    m_hFile = fopen( ApFileName, ACreateAlways ? "w+b" : "a+b" );
#endif
}

#ifdef WIN32
CxdFileStream::CxdFileStream( const wchar_t *ApFileName, bool ACreateAlways /*= false*/ )
{
    m_hFile = CreateFileW( ApFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
        NULL, ACreateAlways ? CREATE_ALWAYS : OPEN_ALWAYS, 0, 0 );
    m_strFileName = UnicodeToAscii( ApFileName );
}
#endif


CxdFileStream::~CxdFileStream()
{
    if ( IsOpenSuccess() )
    {
        Flush();

#ifdef WIN32
        CloseHandle( m_hFile );
#endif

#ifdef linux
        fclose( m_hFile );
#endif
        m_hFile = INVALID_HANDLE_VALUE;
    }
}


void CxdFileStream::Flush( void )
{
    if ( IsOpenSuccess() )
    {

#ifdef WIN32
        FlushFileBuffers(m_hFile);
#endif

#ifdef linux
        fflush(m_hFile);
#endif

    }
}

INT64 CxdFileStream::GetFileSize( void )
{
    if ( !IsOpenSuccess() )
    {
        return 0;
    }

#ifdef WIN32
    DWORD dwHigh;
    DWORD dwLow = ::GetFileSize( m_hFile, &dwHigh );
    return ( (INT64)dwHigh << 32 ) | dwLow;
#endif

#ifdef linux
    size_t nPos = GetPos();
    size_t nEndPos = FileSeek(0, stEnd);
    SetPos( nPos );
    return nEndPos;
#endif

}

bool CxdFileStream::SetFileSize( const INT64 &ANewFileSize )
{
    if ( (m_hFile != INVALID_HANDLE_VALUE) && (ANewFileSize > 0) && (ANewFileSize != GetSize()) )
    {
        Flush();
        
#ifdef WIN32
        DWORD dwHigh(ANewFileSize >> 32), dwLow( (DWORD)ANewFileSize );
        LONG l( dwHigh );
        SetFilePointer(m_hFile, dwLow, dwHigh > 0 ? &l : NULL, FILE_BEGIN);
        SetEndOfFile( m_hFile );
        return true;
#endif

#ifdef linux
        if ( 0 == truncate( m_strFileName.c_str(), ANewFileSize ) )
        {
            return true;
        }
        return false;
#endif
    }
    return false;
}

size_t CxdFileStream::FileSeek( INT64 AOffset, TFileSeekType ASeekType )
{
#ifdef WIN32
    long nHighOffset = AOffset >> 32;
    long nLowOffset = (long)AOffset;
    return SetFilePointer( m_hFile, nLowOffset, &nHighOffset, (DWORD)ASeekType );
#endif

#ifdef linux
    return fseek( m_hFile, AOffset, (int)ASeekType );
#endif
}

size_t CxdFileStream::FileWrite( const void *pBuf, DWORD AWriteBytesNum )
{
#ifdef WIN32
    DWORD dwBW(0);
    if ( WriteFile(m_hFile, pBuf, AWriteBytesNum, &dwBW, NULL) )
    {
        return dwBW;
    }
    return -1;
#endif

#ifdef linux
    return fwrite(pBuf, AWriteBytesNum, 1, m_hFile);
#endif
}

size_t CxdFileStream::FileRead( void *pBuf, DWORD AReadBytesNum )
{
#ifdef WIN32
    DWORD dwBW(0);
    if (ReadFile(m_hFile, pBuf, AReadBytesNum, &dwBW, NULL))
    {
        if (AReadBytesNum != 0 && dwBW != 0)
        {
            return dwBW;
        }
    }
    return -1;
#endif

#ifdef linux
    return fread( pBuf, AReadBytesNum, 1, m_hFile );
#endif
}

void CxdFileStream::ReadLong( LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos /*= true*/ )
{
    if (ACount <= 0 ) return;
    FileRead(ApBuffer, ACount);
}

void CxdFileStream::WriteLong( const LPVOID ApBuffer, const UINT &ACount, const bool AutoIncPos /*= true */ )
{
    if (ACount <= 0 ) return;
    FileWrite(ApBuffer, ACount);
}
