#include "StdAfx.h"
#include "JxdRegeditEx.h"
#include "JxdFileSub.h"
#include "JxdSysSub.h"
#include "JxdStringCoder.h"
#include "JxdStringHandle.h"

bool RegDelete( HKEY ARootKey, const TCHAR* ApSubPath, const TCHAR* ApName )
{
    CRegKey reg;
    if (reg.Open(HKEY_CURRENT_USER, ApSubPath, KEY_ALL_ACCESS) == ERROR_SUCCESS)
    {
        return ERROR_SUCCESS == reg.DeleteValue( ApName );
    }
    return false;

    /*HKEY hOpen;
    if ( ERROR_SUCCESS != RegOpenKeyEx(ARootKey, ApSubPath, 0, KEY_ALL_ACCESS, &hOpen) )
    {
        return false;
    }
    bool Result = true;
    if ( ERROR_SUCCESS !=  RegDeleteKey(hOpen, ApName) )
    {
        Result = false;
    }
    RegCloseKey( hOpen );
    return Result;*/
}

bool RegReadValue( HKEY ARootKey, const TCHAR* ApSubPath, const TCHAR* ApName, TCHAR *ApValue, int &AValueLen )
{
    HKEY hOpen;
    if ( ERROR_SUCCESS != RegOpenKeyEx(ARootKey, ApSubPath, 0, KEY_QUERY_VALUE, &hOpen) )
    {
        return false;
    }
    bool Result = true;
    if ( ERROR_SUCCESS !=  RegQueryValueEx(hOpen, ApName, NULL, NULL, (LPBYTE)ApValue, (LPDWORD)&AValueLen) )
    {
        Result = false;
    }
    RegCloseKey( hOpen );
    return Result;
}

bool RegWriteValue( HKEY ARootKey, const TCHAR* ApSubPath, const TCHAR* ApName, DWORD AType, LPBYTE ApValue, DWORD AValueLen )
{
    bool Result = true;
    HKEY hReg;
    DWORD dispo;
    if ( ERROR_SUCCESS != RegCreateKeyEx(ARootKey, ApSubPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hReg, &dispo) )
    {
        return false;
    }
    if ( ERROR_SUCCESS != RegSetValueEx(hReg, ApName, 0, AType, ApValue, AValueLen) )
    {
        Result = false;
    }
    RegCloseKey( hReg );
    return Result;
}

bool RegGetValueType( HKEY ARootKey, const TCHAR* ApSubPath, const TCHAR* ApName, DWORD &AType )
{
    HKEY hOpen;
    if ( ERROR_SUCCESS != RegOpenKeyEx(ARootKey, ApSubPath, 0, KEY_READ, &hOpen) )
    {
        return false;
    }
    bool Result = true;
    if ( ERROR_SUCCESS != RegQueryValueEx(hOpen, ApName, NULL, &AType, NULL, NULL) )
    {
        Result = false;
    }
    RegCloseKey( hOpen );
    return Result;
}

bool RegWriteString( HKEY ARottKey, const TCHAR* ApSubPath, const TCHAR* ApName, const TCHAR* ApValue, DWORD AStyle /*= REG_SZ*/ )
{
    int len = (_tcsclen(ApValue) + 1) * sizeof(TCHAR);
    return RegWriteValue( ARottKey, ApSubPath, ApName, AStyle, (LPBYTE)ApValue, len );
}

bool IsSoftAutoRunByReg()
{
    CString strApp = GetAppFileName().c_str();
    CString strAppName = ExtractFileName( strApp ).c_str();
    int n = strAppName.Find( '.' );
    if ( n > 0 )
    {
        strAppName = strAppName.Mid( 0, n );
    }
    CString strFileName;
    n = MAX_PATH;
    if ( !RegReadValue( HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), strAppName, strFileName.GetBuffer( MAX_PATH ), n) )
    {
        strFileName.ReleaseBuffer( n );
        return false;
    }
    strFileName.ReleaseBuffer( n );
    return Pos(strFileName, strApp, 0) > 0;
    /*
    if ( strFileName.CompareNoCase(strApp) == 0 )
    {
        return true;
    }
    return false;*/
}

bool SetSoftAutoRunByReg(bool bAutoRun, const TCHAR *ApAutRunParam)
{
    CString strApp = GetAppFileName().c_str();//文件全路径
    CString strAppName = ExtractFileName( strApp ).c_str();//文件名
    int n = strAppName.Find( '.' );
    if ( n > 0 )
    {
        strAppName = strAppName.Mid( 0, n );
    }
    if ( bAutoRun )
    {
        if ( NULL != ApAutRunParam && _tcslen(ApAutRunParam) > 0 )
        {
            strApp = _T("\"") + strApp + _T("\" ");
            strApp += ApAutRunParam;
        }        
        return RegWriteString( HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), strAppName, strApp );
    }
    else
    {
        return RegDelete( HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), strAppName );
    }
}
