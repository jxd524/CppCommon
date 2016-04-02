#include "stdafx.h"
#include "JxdFileSub.h"
#include "JxdStringHandle.h"

#ifdef linux
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#endif

//====================================================================
// 以下函数为Windows专用
//====================================================================
//

#ifdef WIN32
#include <Shlobj.h>

bool FileExist( const wchar_t* ApFileName )
{
    DWORD code = GetFileAttributesW( ApFileName );
    return ((-1 != code) && !(FILE_ATTRIBUTE_DIRECTORY & code));
}

bool DirectoryExist( const wchar_t* ApFileName )
{
    DWORD code = GetFileAttributesW( ApFileName );
    return ((-1 != code) && (FILE_ATTRIBUTE_DIRECTORY & code));
}

bool ForceDirectories( const wchar_t* ApDir )
{
    if ( NULL == ApDir ) return false;
    wstring strDir = ExcludeTrailingPathDelimiter(ApDir);
    if (DirectoryExist(strDir.c_str()) || ExtractFilePath(strDir.c_str()) == strDir)
    {
        return true;
    }
    return ForceDirectories( ExtractFilePath(strDir.c_str()).c_str() ) && 
        CreateDirectoryW( strDir.c_str(), NULL);
}

//DeleteDiretiories
#define _DeleteDirectioriesImpl(type, FuncType)                             \
    if ( !DirectoryExist(ApDir) ) return false;                            \
    type strDirPath = IncludeTrailingPathDelimiter(ApDir);                  \
    type strTemp;                                                           \
    type strFindPath = strDirPath;                                          \
    strFindPath.append( 1, '*' );                                           \
    WIN32_FIND_DATA##FuncType fd;                                           \
    HANDLE fHandle = FindFirstFile##FuncType( strFindPath.c_str(), &fd );   \
    if (INVALID_HANDLE_VALUE == fHandle) return false;                      \
    do {                                                                    \
        if ( '.' != fd.cFileName[0] && '.' != fd.cFileName[1] ){            \
            strTemp = strDirPath + fd.cFileName;                            \
            if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){          \
                if ( !DeleteDirectiories(strTemp.c_str()) ) break;          \
            } else {                                                        \
                DeleteFile##FuncType( strTemp.c_str());                     \
            }                                                               \
        }                                                                   \
    } while ( FindNextFile##FuncType(fHandle, &fd) );                       \
    FindClose(fHandle);	                                                    \
    return TRUE == RemoveDirectory##FuncType( ApDir );
bool DeleteDirectiories( const wchar_t* ApDir )
{
    _DeleteDirectioriesImpl( wstring, W );
}

//GetSpecialFolderPath
#define _GetSpecialFolderPathImpl(type, apiType, nCsid)                          \
    type info[MAX_PATH] = {0};                                              \
    if ( !::SHGetSpecialFolderPath##apiType(NULL, info, nCsid, FALSE) ) {   \
        return false;                                                       \
    }                                                                       \
    APath = IncludeTrailingPathDelimiter( info );                           \
    return DirectoryExist( APath.c_str() );;
bool GetLocalAppPath( string &APath )
{
    _GetSpecialFolderPathImpl( char, A, CSIDL_LOCAL_APPDATA )
}
bool GetLocalAppPath( wstring &APath )
{
    _GetSpecialFolderPathImpl( wchar_t, W, CSIDL_LOCAL_APPDATA )
}


//GetFileSize
#define _GetFileSizeImpl(apiType)                                   \
    WIN32_FIND_DATA##apiType fd;                                    \
    HANDLE hFind = FindFirstFile##apiType( ApFileName, &fd );       \
    if ( INVALID_HANDLE_VALUE == hFind ) return 0;                  \
    INT64 nSize = ((INT64)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;\
    FindClose( hFind );                                             \
    return nSize;

INT64 GetFileSize( const wchar_t *ApFileName )
{
    _GetFileSizeImpl(W);
}


//GetAppFileName
#define _GetAppFileNameImpl(type, FuncType)                 \
    type strName[256] = {0};                                \
    type strFullName[512] = {0};                            \
    GetModuleFileName##FuncType( hModule, strName, 256 );   \
    GetLongPathName##FuncType( strName, strFullName, 256 ); \
    return strFullName; 
string  GetAppFileNameA( HMODULE hModule /*= NULL*/ ){ _GetAppFileNameImpl(char, A);    }
wstring GetAppFileNameW(HMODULE hModule/* = NULL*/)  { _GetAppFileNameImpl(wchar_t, W); }

//GetSysPath
#define _GetSysPathImpl(type, FuncType)                 \
    type tmpBuf[MAX_PATH] = {0};                        \
    GetSystemDirectory##FuncType( tmpBuf, MAX_PATH );   \
    return tmpBuf;
string  GetSysPathA(void) { _GetSysPathImpl(char, A); }
wstring GetSysPathW(void) { _GetSysPathImpl(wchar_t, W); }

//GetUserPath
#define _GetUserPathImpl(type, FuncType)                                                    \
    type tmpBuf[MAX_PATH] = {0};                                                            \
    if ( !SHGetSpecialFolderPath##FuncType( NULL, tmpBuf, CSIDL_LOCAL_APPDATA, FALSE ))     \
    return GetAppFileName##FuncType();                                                      \
    return tmpBuf;
string  GetUserPathA(void) { _GetUserPathImpl(char, A); }
wstring GetUserPathW(void) { _GetUserPathImpl(wchar_t, W); }

//GetSysTempPath
#define _GetSysTempPathImpl(type, FuncType)                                                 \
    type buf[MAX_PATH] = {0};                                                               \
    GetTempPath##FuncType(MAX_PATH, buf);                                                   \
    return buf;
string GetSysTempPathA( void ) { _GetSysTempPathImpl(char, A); }
wstring GetSysTempPathW( void ){ _GetSysTempPathImpl(wchar_t, W); }


//GetTempFileName
#define _GetTempFileNameImpl(type, FuncType)                                            \
    type strPath = IncludeTrailingPathDelimiter( ApPath == NULL ?                       \
                         GetSysTempPath##FuncType().c_str() : ApPath );                 \
    type strName = GetRandomString##FuncType();                                         \
    type strResult = strPath + strName + ApExt;                                         \
    while ( FileExist(strResult.c_str()) ){                                            \
        strName = GetRandomString##FuncType();                                          \
        strResult = strPath + strName + ApExt;                                          \
    }\
    return strResult;

string  GetTempFileNameA( const char *ApExt, const char *ApPath ){ _GetTempFileNameImpl( string, A ); }
wstring GetTempFileNameW( const wchar_t *ApExt, const wchar_t *ApPath){ _GetTempFileNameImpl( wstring, W ); }

#endif

//====================================================================
// 以下函数为跨平台使用
//====================================================================
//
bool FileExist( const char* ApFileName )
{
#ifdef WIN32
    DWORD code = GetFileAttributesA( ApFileName );
    return ((-1 != code) && !(FILE_ATTRIBUTE_DIRECTORY & code));
#endif
#ifdef linux
    struct stat buf;  
    return (stat( ApFileName, &buf ) == 0) && !S_ISDIR( buf.st_mode );
#endif
}

bool DirectoryExist( const char* ApDirName )
{
#ifdef WIN32
    DWORD code = GetFileAttributesA( ApDirName );
    return ((-1 != code) && (FILE_ATTRIBUTE_DIRECTORY & code));
#endif
#ifdef linux
    struct stat buf;  
    stat( ApDirName, &buf );  
    return S_ISDIR( buf.st_mode );
#endif
}

//ExtractFilePath
#define _ExtractFilePathImpl(type)                          \
    type strResult, strSign( 1, FileSeparator );            \
    int n = PosBack( ApFileFullName, strSign.c_str(), 0 );  \
    if ( n > 0 ) strResult.append( ApFileFullName, n + 1);  \
    return strResult;
string ExtractFilePath( const char* ApFileFullName )
{
    _ExtractFilePathImpl( string );
}
wstring ExtractFilePath( const wchar_t* ApFileFullName )
{
    _ExtractFilePathImpl( wstring );
}

//ExtractFileName
#define _ExtractFileNameImpl(type, lenFunc, Separator)                                          \
    type strResult, strSign( 1, Separator );                                                    \
    int n = PosBack( ApFileFullName, strSign.c_str(), 0 );                                      \
    if ( n > 0 ) strResult.append( ApFileFullName + n + 1, lenFunc(ApFileFullName) - n - 1);    \
    return strResult;
string ExtractFileName( const char* ApFileFullName )
{
    _ExtractFileNameImpl( string, strlen, FileSeparator );
}
wstring ExtractFileName( const wchar_t* ApFileFullName )
{
    _ExtractFileNameImpl( wstring, wcslen, FileSeparator );
}

//ExtractFileExt
#define _ExtractFileExtImpl _ExtractFileNameImpl
string ExtractFileExt( const char* ApFileFullName )
{
    _ExtractFileExtImpl( string, strlen, '.' );
}
wstring ExtractFileExt( const wchar_t* ApFileFullName )
{
    _ExtractFileExtImpl( wstring, wcslen, '.' );
}

//ExcludeTrailingPathDelimiter
#define _ExcludeTrailingPathDelimiterImpl(type)                     \
    type strResult( ApFileFullName );                               \
    int nlen = strResult.length();                                  \
    if ( nlen > 0 && FileSeparator == ApFileFullName[nlen - 1] ) {  \
        strResult.clear();                                          \
        strResult.append( ApFileFullName, nlen - 1 );               \
    }                                                               \
    return strResult;
string ExcludeTrailingPathDelimiter( const char* ApFileFullName )
{
    _ExcludeTrailingPathDelimiterImpl( string );
}
wstring ExcludeTrailingPathDelimiter( const wchar_t* ApFileFullName )
{
    _ExcludeTrailingPathDelimiterImpl( wstring );
}


#define _IncludeTrailingPathDelimiterImpl(type)                     \
    type strResult( ApFileFullName );                               \
    int nlen = strResult.length();                                  \
    if ( nlen > 0 && FileSeparator != ApFileFullName[nlen - 1] ) {  \
        strResult += FileSeparator;                                 \
    }                                                               \
    return strResult;
string IncludeTrailingPathDelimiter( const char* ApFileFullName )
{
    _IncludeTrailingPathDelimiterImpl( string );
}
wstring IncludeTrailingPathDelimiter( const wchar_t* ApFileFullName )
{
    _IncludeTrailingPathDelimiterImpl( wstring );
}

bool ForceDirectories( const char* ApDir )
{
    if ( NULL == ApDir ) return false;

    string strDir = ExcludeTrailingPathDelimiter(ApDir);
    if (DirectoryExist(strDir.c_str()) || ExtractFilePath(strDir.c_str()) == strDir)
    {
        return true;
    }
#ifdef WIN32
    return ForceDirectories( ExtractFilePath(strDir.c_str()).c_str() ) && 
           CreateDirectoryA( strDir.c_str(), NULL);
#endif

#ifdef linux
    return ForceDirectories( ExtractFilePath(strDir.c_str()).c_str() ) && 
        mkdir( ApDir, 777) == 0;
#endif
}

bool DeleteDirectiories( const char* ApDir )
{
#ifdef WIN32
    _DeleteDirectioriesImpl( string, A );
#endif

#ifdef linux
    if ( !DirectoryExist(ApDir) ) return false;

    string strDirPath = IncludeTrailingPathDelimiter(ApDir);
    string strTemp;

    DIR *dir = opendir( ApDir );
    if ( NULL == dir ) return false;

    struct dirent* dirp;
    while ( (dirp = readdir(dir)) != NULL )
    {
        if ( '.' != dirp->d_name[0] && '.' != dirp->d_name[1] )
        {
            strTemp = strDirPath + dirp->d_name;
            if ( FileExist(strTemp.c_str()) )
            {
                remove( strTemp.c_str() );
            }
            else
            {
                DeleteDirectiories( strTemp.c_str() );
            }
        }
    }
    closedir( dir );
    return remove(ApDir) == 0;
#endif
}

INT64 GetFileSize( const char *ApFileName )
{
#ifdef WIN32
    _GetFileSizeImpl( A );
#endif
#ifdef linux
    struct stat buf = {0};
    stat(ApFileName, &buf);
    return buf.st_size;
#endif
}


int LoopDirectiories( const wchar_t* ApDir, IxdLoopFileInterface *ApInterface, bool bFindSub )
{
#ifdef WIN32
    int nCount( 0 ); 
    if ( !DirectoryExist(ApDir) ) return nCount;                            
    wstring strDirPath = IncludeTrailingPathDelimiter(ApDir);                  
    wstring strTemp;                                                           
    wstring strFindPath = strDirPath;                                          
    strFindPath.append( 1, '*' );   

    WIN32_FIND_DATAW fd;                                          
    HANDLE fHandle = FindFirstFileW( strFindPath.c_str(), &fd );   
    if (INVALID_HANDLE_VALUE == fHandle) return nCount;   

    TFileInfo fInfo;
    do {                                                                    
        if ( '.' != fd.cFileName[0] && '.' != fd.cFileName[1] )
        {            
            strTemp = strDirPath + fd.cFileName;                            
            if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {          
                nCount += LoopDirectiories( strTemp.c_str(), ApInterface, bFindSub );
            } 
            else 
            {                                                        
                nCount++;
                fInfo.ftCreationTime = fd.ftCreationTime;
                fInfo.ftLastAccessTime = fd.ftLastAccessTime;
                fInfo.ftLastWriteTime = fd.ftLastWriteTime;
                fInfo.nFileSize = fd.nFileSizeHigh * MAXDWORD + fd.nFileSizeLow;
                fInfo.strFileName = strDirPath + fd.cFileName;
                bool bTerminate = false;
                ApInterface->Notify( &fInfo, bTerminate );
                if ( bTerminate ) 
                {
                    FindClose( fHandle );
                    return nCount;
                }
            }                                                               
        }                                                                   
    } while ( FindNextFileW(fHandle, &fd) );                       
    FindClose(fHandle);	                                                    
    return nCount;
#endif

#ifdef linux
    return 0;
#endif
}

bool IsExistExtFile( const wchar_t *ASrcPath, const wchar_t *ApExtName )
{
	class CMyLoopFile : public CxdLoopFileHandle
	{
	public:
		CMyLoopFile(const wchar_t *ApSrcPath, const wchar_t *ApExtName)
		{
			ParseFormatString( ApExtName, TEXT(","), m_ltExtName );
			m_bFind = false;
			LoopDirectiories( ApSrcPath, this, true );
		}
		bool IsFindZipFile(void) { return m_bFind; }
	private:
		bool m_bFind;
		TStringWList m_ltExtName;
		bool DoLoopFile(TFileInfo* ApParam)
		{
			wstring strExtName = ExtractFileExt( ApParam->strFileName.c_str() );
			for ( TStringWListIT it = m_ltExtName.begin(); it != m_ltExtName.end(); it++ )
			{
				wstring strTemp = *it;
				if ( 0 == strExtName.compare(strTemp) )
				{
					m_bFind = true;
					return false;
				}
			}
			return true;
		}
	};
	CMyLoopFile LoopFile( ASrcPath, ApExtName );
	bool bFind = LoopFile.IsFindZipFile();
	return bFind;
}

/***********************************************************************
    Linux 下相关
************************************************************************/
#ifdef linux 

string GetAppFileName( void )
{
    char buf[1024 * 4] = {0};
    if ( readlink("/proc/self/exe", buf, sizeof(buf)) <= 0 )
    {
        return "";
    }
    return buf;
}


#endif