#include "stdafx.h"
#include "JxdSQLiteBasic.h"
#include "JxdMacro.h"
#include "JxdStringCoder.h"
#include "JxdStringHandle.h"

CxdSQLiteBasic::CxdSQLiteBasic(const wchar_t *ApDBFileName)
{
    m_pSqlConn = NULL;
    if ( SQLITE_OK == sqlite3_open16(ApDBFileName, &m_pSqlConn) )
    {
        m_strDBFileName = ApDBFileName;
    }
}

CxdSQLiteBasic::~CxdSQLiteBasic()
{
    if ( Assigned(m_pSqlConn) )
    {
        sqlite3_close( m_pSqlConn );
        m_pSqlConn = NULL;
    }    
}

int CxdSQLiteBasic::GetTablesName( TStringWList &Alt )
{
    if ( !Assigned(m_pSqlConn) ) return 0;
    int nResult( 0 );
    sqlite3_stmt *pStmt;
    wstring sSQL = TEXT( "select name from sqlite_master where type='table' order by name" );
    if ( SQLITE_OK == sqlite3_prepare16_v2( m_pSqlConn, sSQL.c_str(), sSQL.length() * 2, &pStmt, 0 ) )
    {
        while( sqlite3_step(pStmt)==SQLITE_ROW ) 
        {
            wchar_t *pName = (wchar_t*)sqlite3_column_text16( pStmt,0 );
            Alt.push_back( pName );
            nResult++;
        }
        sqlite3_finalize( pStmt );
    }
    return nResult;
}

void CxdSQLiteBasic::MySQLiteTest( void )
{
    /*CString s = Format( TEXT("%d %s"), 100, TEXT("aaajjjdddl") );*/

    //CString strMyTable = TEXT( "Create Table myTable123 (ID  Integer primary key,\
    //                            FileHash  varchar(64) default '123', \
    //                            a int,\
    //                            b varchar(23),\
    //                            c byte,\
    //                            MusicName  varchar(120),\
    //                            nv int default 10,\
    //                            mmt varchar(20),\
    //                            ddd datetime )" );
    //CreatTable(TEXT("myTable123"), strMyTable);
    return;

    //CString sSQL = TEXT( "select * from myTable123" );
    //sqlite3_stmt *pStmt;
    //int nStep = OpenSQL( sSQL, pStmt );

    //TSqliteQueryInfo info;
    //InitSqliteQueryInfo( pStmt, info );
    //do 
    //{
    //    CString strMusicName, strHash;
    //    GetInfoByFieldName(info, TEXT("MusicName"), strMusicName );
    //    GetInfoByFieldName(info, TEXT("FileHash"), strHash );
    //} while ( NextRecord(pStmt) );
    //FreeSqliteQueryInfo( info );
}

void CxdSQLiteBasic::CloseStmt( sqlite3_stmt *&ApStmt )
{
    if ( Assigned(ApStmt) )
    {
        sqlite3_finalize( ApStmt );
        ApStmt = NULL;
    }
}

int CxdSQLiteBasic::OpenSQL( const CString &ASQL, sqlite3_stmt *&ApStmt )
{
    if ( !Assigned(m_pSqlConn) ) return SQLITE_EMPTY;

    sqlite3_stmt *pStmt = NULL;
    int nResult = sqlite3_prepare16_v2(m_pSqlConn, ASQL, ASQL.GetLength() * 2, &pStmt, 0);
    if ( Assigned(pStmt) )
    {
        nResult = sqlite3_step( pStmt );
    }
    ApStmt = pStmt;
    return nResult;
}

bool CxdSQLiteBasic::ExecSQL( const CString &ASQL )
{
    bool bOK( false );
    if ( Assigned(m_pSqlConn) )
    {
        sqlite3_stmt *pStmt = NULL;
        int nResult = sqlite3_prepare16_v2(m_pSqlConn, ASQL, ASQL.GetLength() * 2, &pStmt, 0);
        bOK = (SQLITE_OK == nResult) || (SQLITE_DONE == nResult);
        if ( Assigned(pStmt) )
        {
            sqlite3_step( pStmt );
            CloseStmt( pStmt );
        }
    }
    return bOK;
}

bool CxdSQLiteBasic::CreateTable( const CString &ATableName, const CString &ACreateSQL )
{
    if ( !ExecSQL(ACreateSQL) )
    {
        vector<int> Items;
        TStringWList ltOld, ltNew;
        GetTableFields( ATableName, ltOld );
        ParseCreateTableFields( ACreateSQL, ltNew );
        int nIndex(0);
        for ( TStringWListIT it = ltNew.begin(); it != ltNew.end(); it++ )
        {
            TxdParseInfoW info;
            ParseFormatString( (*it).c_str(), TEXT(" "), info );

            bool bAdd = true;
            for ( TStringWListIT itCheck = ltOld.begin(); itCheck != ltOld.end(); itCheck++ )
            {
				CString strTemp = info.First.c_str();
                if ( 0 == strTemp.CompareNoCase((*itCheck).c_str()) )
                {
                    bAdd = false;
                    break;
                }
            }
            if ( bAdd )
            {
                Items.push_back( nIndex );
            }
            nIndex++;
        }
        //
        for ( vector<int>::iterator it = Items.begin(); it != Items.end(); it++ )
        {
            int nIndex = *it;
            CString strNewColumn = ltNew[nIndex].c_str();
            CString strAlterTable = TEXT( "alter table ") + ATableName + TEXT(" add column " ) + strNewColumn;
            ExecSQL( strAlterTable );
        }
    }
    return true;
}

int CxdSQLiteBasic::GetTableFields( const wchar_t *ApTableName, TStringWList &Alt )
{
    const wchar_t* const CtpQueryTableInfo = TEXT( "PRAGMA table_info('%s')" );

    int nResult( 0 );
    wchar_t buf[512] = {0};
    swprintf_s( buf, 510, CtpQueryTableInfo, ApTableName );

    sqlite3_stmt *pStmt = NULL;
    int nState = OpenSQL( buf, pStmt );
    while ( SQLITE_ROW == nState )
    {
        const wchar_t *pFieldName = (const wchar_t*)sqlite3_column_text16( pStmt, 1 );
        if ( Assigned(pFieldName) )
        {
            nResult++;
            Alt.push_back( pFieldName );
        }
        nState = NextRecord( pStmt );
    }
    CloseStmt( pStmt );
    return nResult;
}

void CxdSQLiteBasic::ParseCreateTableFields( const CString &ASQL, TStringWList &Alt )
{
    /*
    Create Table myTable123 (ID  Integer primary key,
                             FileHash  varchar(64) default '123',
                             MusicName  varchar(120) )
    */
    Alt.clear();
    int nPos1 = ASQL.Find( '(' );
    if ( -1 == nPos1 ) return;

    int nPos2 = ASQL.ReverseFind( ')' );
    if ( -1 == nPos2 ) return;

    CString strFields = ASQL.Mid( nPos1 + 1, nPos2 - nPos1 - 1 );
    ParseFormatString( strFields, TEXT(","), Alt );
}

void CxdSQLiteBasic::SetDBFileName( const wchar_t *ApNewFileName )
{
    if ( CopyFile(m_strDBFileName.c_str(), ApNewFileName, FALSE) )
    {
        if ( Assigned(m_pSqlConn) )
        {
            sqlite3_close( m_pSqlConn );
            m_pSqlConn = NULL;
        }
        if ( SQLITE_OK == sqlite3_open16(ApNewFileName, &m_pSqlConn) )
        {
            m_strDBFileName = ApNewFileName;
        }
        else
        {
            sqlite3_open16( m_strDBFileName.c_str(), &m_pSqlConn );
        }
    }
}

void CxdSQLiteBasic::InitSqliteQueryInfo( sqlite3_stmt *ApStmt, TSqliteQueryInfo &info )
{
    info.FpStmt = ApStmt;
    info.FFieldsName.clear();
    int nCount = sqlite3_column_count( ApStmt );
    for ( int i = 0; i != nCount; i++ )
    {
        info.FFieldsName.push_back( (TCHAR*)sqlite3_column_name16(ApStmt, i) );
    }
}

void CxdSQLiteBasic::FreeSqliteQueryInfo( TSqliteQueryInfo &info )
{
    CloseStmt( info.FpStmt );
    info.FFieldsName.clear();
}

bool CxdSQLiteBasic::GetInfoByFieldName( const TSqliteQueryInfo &info, const CString &AFieldName, CString &AValue )
{
    int nIndex(0);
    bool bOK = FindInfo(info, AFieldName, nIndex);
    if ( bOK )
    {
        AValue = (TCHAR*)sqlite3_column_text16( info.FpStmt, nIndex );
    }
    return bOK;
}

bool CxdSQLiteBasic::GetInfoByFieldName( const TSqliteQueryInfo &info, const CString &AFieldName, int &AValue )
{
    int nIndex(0);
    bool bOK = FindInfo(info, AFieldName, nIndex);
    if ( bOK )
    {
        AValue = sqlite3_column_int( info.FpStmt, nIndex );
    }
    return bOK;
}

bool CxdSQLiteBasic::GetInfoByFieldName( const TSqliteQueryInfo &info, const CString &AFieldName, INT64 &AValue )
{
    int nIndex(0);
    bool bOK = FindInfo(info, AFieldName, nIndex);
    if ( bOK )
    {
        AValue = sqlite3_column_int64( info.FpStmt, nIndex );
    }
    return bOK;
}

bool CxdSQLiteBasic::GetInfoByFieldName( const TSqliteQueryInfo &info, const CString &AFieldName, bool &AValue )
{
    int nValue;
    bool bOK = GetInfoByFieldName( info, AFieldName, nValue );
    if ( bOK )
    {
        AValue = nValue != 0;
    }
    return bOK;
}

bool CxdSQLiteBasic::FindInfo( const TSqliteQueryInfo &info, const CString &AFieldName, int &AIndex )
{
    int n(0);
    bool bFind(false);
	TStringWList ltTemp = info.FFieldsName;
    for ( TStringWListIT it = ltTemp.begin(); it != ltTemp.end(); it++ )
    {
        if ( 0 == AFieldName.CompareNoCase((*it).c_str()) )
        {
            bFind = true;
            break;
        }
        n++;
    }
    if ( bFind )
    {
        AIndex = n;
    }
    return bFind;
}

sqlite_int64 CxdSQLiteBasic::ExecInsSQL( const CString &ASQL )
{
    sqlite_int64 v = -1;
    if ( Assigned(m_pSqlConn) )
    {
        sqlite3_stmt *pStmt = NULL;
        int nResult = sqlite3_prepare16_v2(m_pSqlConn, ASQL, ASQL.GetLength() * 2, &pStmt, 0);
        if ( ((SQLITE_OK == nResult) || (SQLITE_DONE == nResult)) && Assigned(pStmt) )
        {
            sqlite3_step( pStmt );
            v = sqlite3_last_insert_rowid( m_pSqlConn );
            CloseStmt( pStmt );
        }
    }
    return v;
}

sqlite3_stmt* CxdSQLiteBasic::BeginBlodExec( const CString &ASQL )
{
    sqlite3_stmt *pStmt = NULL;
    if ( Assigned(m_pSqlConn) )
    {
        sqlite3_prepare16_v2(m_pSqlConn, ASQL, ASQL.GetLength() * 2, &pStmt, 0);
    }
    return pStmt;
}

void CxdSQLiteBasic::EndBlodExec( sqlite3_stmt *&ApStmt )
{
    if ( Assigned(ApStmt) )
    {
        sqlite3_step( ApStmt );
        CloseStmt( ApStmt );
    }
}

bool CxdSQLiteBasic::BindBlodData( sqlite3_stmt *&ApStmt, int ABindParamIndex, void * ApParamData, int ADataLen )
{
    return SQLITE_OK == sqlite3_bind_blob(ApStmt, ABindParamIndex, ApParamData, ADataLen, NULL);  
}

bool CxdSQLiteBasic::QuerySQL( const TCHAR* ASQL, TSqliteQueryInfo& AInfo )
{
    sqlite3_stmt *pStmt = NULL;
    int nResult = OpenSQL( ASQL, pStmt );
    if ( SQLITE_ROW == nResult )
    {
        InitSqliteQueryInfo( pStmt, AInfo );
        return true;
    }
    CloseStmt(pStmt);
    return false;
}
