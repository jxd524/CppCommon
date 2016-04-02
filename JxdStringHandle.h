/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdStringHandle.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-10-8 15:54:50
	LastModify : 
	Description: �ַ�������Ԫ
**************************************************************************/
#pragma once
#ifndef _JxdStringHandle_Lib
#define _JxdStringHandle_Lib

#include <string>
#include <vector>
using namespace std;

#include <stdio.h>

#ifdef linux
#include "JxdTypedef.h"
int _strnicmp(const char * first, const char * last, size_t count);
int _wcsnicmp(const wchar_t * first, const wchar_t * last, size_t count);
#else
#pragma warning(disable:4996)
#endif

enum TxdThePos{ tpNULL, tpForward, tpBack };

struct TxdMatchInfo 
{
    int  CurCount;
    int  MaxCount;
    int* MatchInfo; 
};

struct TxdParseInfoA 
{
    string First;
    string Second;
};
struct TxdParseInfoW 
{
    wstring First;
    wstring Second;
};

typedef vector<string> TStringAList;
typedef TStringAList::iterator TStringAListIT;
typedef TStringAList::reverse_iterator TRStringAListIT;

typedef vector<wstring> TStringWList;
typedef TStringWList::iterator TStringWListIT;
typedef TStringWList::reverse_iterator TRStringWListIT;

typedef vector<TxdParseInfoA> TParseInfoAList;
typedef TParseInfoAList::iterator TParseInfoAListIT;
typedef TParseInfoAList::reverse_iterator TRParseInfoAListIT;

typedef vector<TxdParseInfoW> TParseInfoWList;
typedef TParseInfoWList::iterator TParseInfoWListIT;
typedef TParseInfoWList::reverse_iterator TRParseInfoWListIT;

//====================================================================
// Pos �������: Pos, PosCase, PosNotOf, PosCaseNotOf
//               PosBack, PosBackCase, PosBackNotOf, PosBackCaseNotOf
//====================================================================
//
/*��ָ��λ���������ָ���ַ���λ��*/
//�����ִ�Сд
int Pos( const char* ApText, const char *ApSubText, int AFindPos);
int Pos( const wchar_t* ApText, const wchar_t *ApSubText, int AFindPos);
//���ִ�Сд
int PosCase( const char* ApText, const char *ApSubText, int AFindPos);
int PosCase( const wchar_t* ApText, const wchar_t *ApSubText, int AFindPos);

/*��ָ��λ�����������ָ���ַ�����һ�β�ƥ���λ��*/
//�����ִ�Сд
int PosNotOf( const char* ApText, const char *ApSubText, int AFindPos);
int PosNotOf( const wchar_t* ApText, const wchar_t *ApSubText, int AFindPos);
//���ִ�Сд
int PosCaseNotOf( const char* ApText, const char *ApSubText, int AFindPos);
int PosCaseNotOf( const wchar_t* ApText, const wchar_t *ApSubText, int AFindPos);

/*��ָ��λ���������ָ���ַ���λ��*/
//�����ִ�Сд, ABackPos: �������ǰ����
int PosBack( const char* ApText, const char *ApSubText, int AFindPos);
int PosBack( const wchar_t* ApText, const wchar_t *ApSubText, int AFindPos);
//���ִ�Сд
int PosBackCase( const char* ApText, const char *ApSubText, int AFindPos);
int PosBackCase( const wchar_t* ApText, const wchar_t *ApSubText, int AFindPos);

/*��ָ��λ����ǰ������ָ���ַ�����һ�β�ƥ���λ��*/
//�����ִ�Сд
int PosBackNotOf( const char* ApText, const char *ApSubText, int AFindPos);
int PosBackNotOf( const wchar_t* ApText, const wchar_t *ApSubText, int AFindPos);
//���ִ�Сд
int PosBackCaseNotOf( const char* ApText, const char *ApSubText, int AFindPos);
int PosBackCaseNotOf( const wchar_t* ApText, const wchar_t *ApSubText, int AFindPos);

/*��������ƥ���ַ�����λ��, ע�����: TxdMatchInfo, ������Ҫʱ,һ����Ҫ���� SafeDeleteNewBuffer*/
//�����ִ�Сд
int SearchMatchInfo(const char *ApText, const char *ApSubText, int ABeginPos, TxdMatchInfo &AInfo, int AMaxMatchCount = -1);
int SearchMatchInfo(const wchar_t *ApText, const wchar_t *ApSubText, int ABeginPos, TxdMatchInfo &AInfo, int AMaxMatchCount = -1);
//���ִ�Сд
int SearchMatchInfoCase(const char *ApText, const char *ApSubText, int ABeginPos, TxdMatchInfo &AInfo, int AMaxMatchCount = -1);
int SearchMatchInfoCase(const wchar_t *ApText, const wchar_t *ApSubText, int ABeginPos, TxdMatchInfo &AInfo, int AMaxMatchCount = -1);

/*�ַ����滻*/
//�����ִ�Сд, ���سɹ��滻����, ANewString�����µ��ַ���
int ReplaceString(const char *ApText, const char *ApOldSubText, const char *ApNewSubText, string &ANewString, 
    int ABeginPos = 0, int AReplaceCount = -1);
int ReplaceString(const wchar_t *ApText, const wchar_t *ApOldSubText, const wchar_t *ApNewSubText, wstring &ANewString, 
    int ABeginPos = 0, int AReplaceCount = -1);

string  ReplaceString(const char *ApText, const char *ApOldSubText, const char *ApNewSubText, int ABeginPos = 0, int AReplaceCount = -1);
wstring ReplaceString(const wchar_t *ApText, const wchar_t *ApOldSubText, const wchar_t *ApNewSubText, int ABeginPos = 0, int AReplaceCount = -1);

//���ִ�Сд
int ReplaceStringCase(const char *ApText, const char *ApOldSubText, const char *ApNewSubText, string &ANewString, 
    int ABeginPos = 0, int AReplaceCount = -1);
int ReplaceStringCase(const wchar_t *ApText, const wchar_t *ApOldSubText, const wchar_t *ApNewSubText, wstring &ANewString, 
    int ABeginPos = 0, int AReplaceCount = -1);

string  ReplaceStringCase(const char *ApText, const char *ApOldSubText, const char *ApNewSubText, int ABeginPos = 0, int AReplaceCount = -1);
wstring ReplaceStringCase(const wchar_t *ApText, const wchar_t *ApOldSubText, const wchar_t *ApNewSubText, int ABeginPos = 0, int AReplaceCount = -1);


/*��ȡָ��ƥ���ַ�: ��ǰ���*/
//�����ִ�Сд, AWhenBackNotFind: ��ApBack�ַ���û���ҵ�ʱ,��δ���, ����-1��ʾ�Ҳ���, ���򷵻ز鵽�ַ������λ��
int     GetTheString(const char *ApText, const char *ApForward, const char *ApBack, string &ANewString, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
int     GetTheString(const wchar_t *ApText, const wchar_t *ApForward, const wchar_t *ApBack, wstring &ANewString, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);

string  GetTheString(const char *ApText, const char *ApForward, const char *ApBack, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
wstring GetTheString(const wchar_t *ApText, const wchar_t *ApForward, const wchar_t *ApBack, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
//���ִ�Сд
int     GetTheStringCase(const char *ApText, const char *ApForward, const char *ApBack, string &ANewString, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
int     GetTheStringCase(const wchar_t *ApText, const wchar_t *ApForward, const wchar_t *ApBack, wstring &ANewString, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);

string  GetTheStringCase(const char *ApText, const char *ApForward, const char *ApBack, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
wstring GetTheStringCase(const wchar_t *ApText, const wchar_t *ApForward, const wchar_t *ApBack, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);

/*��ȡָ��ƥ���ַ�: �Ӻ���ǰ*/
int     GetTheStringFromBack(const char *ApText, const char *ApForward, const char *ApBack, string &ANewString, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
int     GetTheStringFromBack(const wchar_t *ApText, const wchar_t *ApForward, const wchar_t *ApBack, wstring &ANewString, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);

string  GetTheStringFromBack(const char *ApText, const char *ApForward, const char *ApBack, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
wstring GetTheStringFromBack(const wchar_t *ApText, const wchar_t *ApForward, const wchar_t *ApBack, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
//���ִ�Сд
int     GetTheStringFromBackCase(const char *ApText, const char *ApForward, const char *ApBack, string &ANewString, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
int     GetTheStringFromBackCase(const wchar_t *ApText, const wchar_t *ApForward, const wchar_t *ApBack, wstring &ANewString, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);

string  GetTheStringFromBackCase(const char *ApText, const char *ApForward, const char *ApBack, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);
wstring GetTheStringFromBackCase(const wchar_t *ApText, const wchar_t *ApForward, const wchar_t *ApBack, TxdThePos AWhenBackNotFind = tpNULL, int ABeginPos = 0);


/*��ʽ��*/
string  Format(const char* AFrmText, ...);
wstring Format(const wchar_t* AFrmText, ...);

/*��ȡ����ַ���*/
string  GetRandomStringA(const int AMinCount = 10, const int AMaxCount = 30, const bool AOnlyNumber = false); 
wstring GetRandomStringW(const int AMinCount = 10, const int AMaxCount = 30, const bool AOnlyNumber = false); 

/**/
string   LowerCase(const string &s); 
wstring  LowerCase(const wstring &s); 

string   UpperCase(const string &s);
wstring  UpperCase(const wstring &s);

void    TrimString(string &AText);
void    TrimString(wstring &AText);
#define TrimStringList(l) for ( TStringListIT it = l.begin(); it != l.end(); it++ ) { TrimString(*it); }

//���� ApSing ��ApText�ָ��� ����ֶ�
int     ParseFormatString(const char *ApText, const char *ApSign, TStringAList &Alt, int AParseCount = -1);
int     ParseFormatString(const wchar_t *ApText, const wchar_t *ApSign, TStringWList &Alt, int AParseCount = -1);
bool    ParseFormatString(const char *ApText, const char *ApSign, TxdParseInfoA &AInfo);
bool    ParseFormatString(const wchar_t *ApText, const wchar_t *ApSign, TxdParseInfoW &AInfo);
int     ParseFormatStringCase(const char *ApText, const char *ApSign, TStringAList &Alt, int AParseCount = -1);
int     ParseFormatStringCase(const wchar_t *ApText, const wchar_t *ApSign, TStringWList &Alt, int AParseCount = -1);
bool    ParseFormatStringCase(const char *ApText, const char *ApSign, TxdParseInfoA &AInfo);
bool    ParseFormatStringCase(const wchar_t *ApText, const wchar_t *ApSign, TxdParseInfoW &AInfo);

string  GetUrlDomain(const char *ApURL);
wstring GetUrlDomain(const wchar_t *ApURL);

string  GetLineEndSign(const char *ApText);
wstring GetLineEndSign(const wchar_t *ApText);

/*�Ƚ��ַ���*/
bool CompareString( const char* AParam1, const char* AParam2 );
bool CompareString( const wchar_t* AParam1, const wchar_t* AParam2 );
bool CompareStringCase( const char* AParam1, const char* AParam2 );
bool CompareStringCase( const wchar_t* AParam1, const wchar_t* AParam2 );

INT64 StrToInt64( const char* AStr);
INT64 StrToInt64( const wchar_t* AStr);
string  Int64ToStrA(const INT64 &AValue);
wstring Int64ToStrW(const INT64 &AValue);

//�������ַ���Сд�Ա�
#define CompareTextA(ApString1, ApString2, ACount) _strnicmp(ApString1, ApString2, ACount)
#define CompareTextW(ApString1, ApString2, ACount) _wcsnicmp(ApString1, ApString2, ACount)

#ifdef UNICODE
    #define Int64ToStr Int64ToStrW
    #define CompareText CompareTextW
    #define GetRandomString GetRandomStringW
    #define TxdParseInfo TxdParseInfoW
    #define CxdString wstring
    #define TStringList  TStringWList
    #define TStringListIT TStringWListIT
    #define TRStringListIT TRStringWListIT
#else
    #define Int64ToStr Int64ToStrA
    #define CompareText CompareTextA
    #define GetRandomString GetRandomStringA
    #define TxdParseInfo TxdParseInfoA
    #define CxdString string
    #define TStringList  TStringAList
    #define TStringListIT TStringAListIT
    #define TRStringListIT TRStringAListIT
#endif



#endif