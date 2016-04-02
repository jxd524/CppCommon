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

    //执行，查询 SQL
    bool         ExecSQL(const CString &ASQL); //执行成功与否
    sqlite_int64 ExecInsSQL(const CString &ASQL); //执行, 返回最后插入数据的ID
    int          OpenSQL(const CString &ASQL, sqlite3_stmt *&ApStmt);
    bool         QuerySQL(const TCHAR* ASQL, TSqliteQueryInfo& AInfo);

    sqlite3_stmt* BeginBlodExec(const CString &ASQL);
    bool          BindBlodData(sqlite3_stmt *&ApStmt, int ABindParamIndex, void * ApParamData, int ADataLen);
    void          EndBlodExec(sqlite3_stmt *&ApStmt);

    static bool NextRecord(sqlite3_stmt *&ApStmt){ return SQLITE_ROW == sqlite3_step(ApStmt); }
    static void CloseStmt(sqlite3_stmt *&ApStmt);

    void MySQLiteTest(void);
    //功能
    int GetTableFields(const wchar_t *ApTableName, TStringWList &Alt);
    int GetTablesName(TStringWList &Alt);

    //数据库相关
    void SetDBFileName(const wchar_t *ApNewFileName);
    inline wstring GetDBFileName(void){ return m_strDBFileName; }

    //方便操作，使用字段名获取信息
    static void InitSqliteQueryInfo(sqlite3_stmt *ApStmt, TSqliteQueryInfo &info);
    static void FreeSqliteQueryInfo(TSqliteQueryInfo &info);
    static bool FindInfo(const TSqliteQueryInfo &info, const CString &AFieldName, int &AIndex);
    static bool GetInfoByFieldName(const TSqliteQueryInfo &info, const CString &AFieldName, CString &AValue);
    static bool GetInfoByFieldName(const TSqliteQueryInfo &info, const CString &AFieldName, int &AValue);
    static bool GetInfoByFieldName(const TSqliteQueryInfo &info, const CString &AFieldName, bool &AValue);
    static bool GetInfoByFieldName(const TSqliteQueryInfo &info, const CString &AFieldName, INT64 &AValue);
protected:
    bool CreateTable(const CString &ATableName, const CString &ACreateSQL);
    void ParseCreateTableFields(const CString &ASQL, TStringWList &Alt); //解析SQL创建表语句，把需要创建的字段提取出来
private:
    wstring m_strDBFileName;
    sqlite3 *m_pSqlConn;
};