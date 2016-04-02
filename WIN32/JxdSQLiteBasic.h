#pragma once
extern "C"
{
    #include "sqlite3.h"
};

#include <string>
#include <vector>
#include "JxdStringCoder.h"
#include "JxdStringHandle.h"
using namespace std;

struct TSqliteQueryInfo
{
    TSqliteQueryInfo() { FpStmt = NULL; }
    sqlite3_stmt *FpStmt;
    TStringWList FFieldsName;
};

class CxdSQLiteBasic
{
public:
    CxdSQLiteBasic(const wchar_t *ApDBFileName);
    ~CxdSQLiteBasic();

    //ִ�У���ѯ SQL
    bool         ExecSQL(const CString &ASQL); //ִ�гɹ����
    sqlite_int64 ExecInsSQL(const CString &ASQL); //ִ��, �������������ݵ�ID
    int          OpenSQL(const CString &ASQL, sqlite3_stmt *&ApStmt);
    bool         QuerySQL(const TCHAR* ASQL, TSqliteQueryInfo& AInfo);

    sqlite3_stmt* BeginBlodExec(const CString &ASQL);
    bool          BindBlodData(sqlite3_stmt *&ApStmt, int ABindParamIndex, void * ApParamData, int ADataLen);
    void          EndBlodExec(sqlite3_stmt *&ApStmt);

    static bool NextRecord(sqlite3_stmt *&ApStmt){ return SQLITE_ROW == sqlite3_step(ApStmt); }
    static void CloseStmt(sqlite3_stmt *&ApStmt);

    void MySQLiteTest(void);
    //����
    int GetTableFields(const wchar_t *ApTableName, TStringWList &Alt);
    int GetTablesName(TStringWList &Alt);

    //���ݿ����
    void SetDBFileName(const wchar_t *ApNewFileName);
    inline wstring GetDBFileName(void){ return m_strDBFileName; }

    //���������ʹ���ֶ�����ȡ��Ϣ
    static void InitSqliteQueryInfo(sqlite3_stmt *ApStmt, TSqliteQueryInfo &info);
    static void FreeSqliteQueryInfo(TSqliteQueryInfo &info);
    static bool FindInfo(const TSqliteQueryInfo &info, const CString &AFieldName, int &AIndex);
    static bool GetInfoByFieldName(const TSqliteQueryInfo &info, const CString &AFieldName, CString &AValue);
    static bool GetInfoByFieldName(const TSqliteQueryInfo &info, const CString &AFieldName, int &AValue);
    static bool GetInfoByFieldName(const TSqliteQueryInfo &info, const CString &AFieldName, bool &AValue);
    static bool GetInfoByFieldName(const TSqliteQueryInfo &info, const CString &AFieldName, INT64 &AValue);
protected:
    bool CreateTable(const CString &ATableName, const CString &ACreateSQL);
    void ParseCreateTableFields(const CString &ASQL, TStringWList &Alt); //����SQL��������䣬����Ҫ�������ֶ���ȡ����
private:
    wstring m_strDBFileName;
    sqlite3 *m_pSqlConn;
};