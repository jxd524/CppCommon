#include "stdafx.h"
#include "JxdDebugInfo.h"
#include <time.h>
#include <string.h>
#include "JxdMacro.h"
#include "JxdFileSub.h"
#include "JxdDataStream.h"
#include "JxdSynchro.h"
#include "JxdTemplateSub.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

string GetDebugFileName(const char *ApFileName)
{
    static string strName;
    if ( Assigned(ApFileName) ) 
    {
        strName = ApFileName;
    }
    return strName;
}
void SetDebugFileName(const char *ApFileName)
{
	GetDebugFileName( ApFileName );
}

void __LogToFile(
#ifdef WriteDebugToFile
    CxdFileStream* ApFile, 
#endif
    const char *ApFileName, const char *ApFunctionName, const int &ALineName, const char *ApText)
{
    time_t aclock;
    time(&aclock);
    struct tm *pct = localtime( &aclock );
    char strTemp[256] = {0};
    sprintf(strTemp, "(%d-%02d-%02d %02d:%02d:%02d)[%s--%d: %s] ", 
        pct->tm_year + 1990, pct->tm_mon + 1, pct->tm_mday, 
        pct->tm_hour, pct->tm_min, pct->tm_sec,
        ApFileName, ALineName, ApFunctionName );
    
    string strInfo = strTemp;
    strInfo += ApText;
    strInfo += "\r\n";

#ifdef WIN32
    OutputDebugStringA( strInfo.c_str() );
#endif

#ifdef linux
    printf( strInfo.c_str() );
#endif

#ifdef WriteDebugToFile
    ApFile->WriteLong( (const LPVOID)strInfo.c_str(), strInfo.length() );
    ApFile->Flush();
#endif
}

class CxdMutexEx: public CxdMutex
{
public:
    ~CxdMutexEx(){ LogToFile( NULL, "", 0, "" ); }
};

void LogToFile(const char *ApFileName, const char *ApFunctionName, const int &ALineName, const char *ApText)
{
    static CxdMutexEx lock;
    AutoMutex( lock );

#ifdef WriteDebugToFile
    static CxdFileStream* pFile = NULL;
    if ( IsNull(pFile) )
    {
        string strName = GetDebugFileName(NULL);
        if ( strName.empty() )
        {
            strName = GetAppFileNameA().c_str();
            strName += ".debug";
        }
        pFile = new CxdFileStream( strName.c_str(), false );
        pFile->SetPos( pFile->GetFileSize() );
        __LogToFile( pFile, "==============================Begin Log File=====================", "==", 0, "==" );
    }
    else
    {
        if ( IsNull(ApFileName) && ALineName == 0 )
        {
            SafeDeleteObject( pFile );
            return;
        }
    }
#endif

    int nLen = strlen( ApFileName );
    int nPos = 0;
    for ( int i = nLen; i >= 0; i-- )
    {
        if ( ApFileName[i] == '\\' )
        {
            nPos = i + 1; 
            break;
        }
    }

    __LogToFile( 
#ifdef WriteDebugToFile
        pFile, 
#endif
        &ApFileName[nPos], ApFunctionName, ALineName, ApText );
}

void LogToFile( const char *ApFileName, const char *ApFunctionName, const int &ALineName, const wchar_t *ApText )
{
    LogToFile( ApFileName, ApFunctionName, ALineName, UnicodeToAscii(ApText).c_str() );
}