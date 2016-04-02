#include "stdafx.h"
#include "JxdTime.h"
#include "JxdStringCoder.h"
#include "JxdStringHandle.h"


#pragma warning(disable:4996)

UINT64 GetRunningSecond( void )
{
#ifdef linux
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts );
    return (UINT64)ts.tv_sec; 
#endif
#ifdef WIN32
    return clock() / CLOCKS_PER_SEC; //GetTickCount() / 1000;
#endif
}

UINT64 GetRunningMSecond( void )
{
#ifdef linux
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts );
    return (UINT64)(ts.tv_nsec / (1000 * 1000) + ts.tv_sec * 1000); 
#endif
#ifdef WIN32
    return clock();
#endif
}

__time64_t GetTimeFromString( const char *ApTime )
{
	if ( ApTime == NULL ) return 0;
	
	int nYear, nMonth, nDate, nHour, nMin, nSec;   
	sscanf_s(ApTime, "%d-%d-%d %d:%d:%d",&nYear,&nMonth,&nDate,&nHour,&nMin,&nSec);  
	CTime t( nYear, nMonth, nDate, nHour, nMin, nSec );
	return t.GetTime();
}

__time64_t GetTimeFromString( const wchar_t *ApTime )
{
	return GetTimeFromString( UnicodeToAscii(ApTime).c_str() );
}

time_t GetCurrenTime( void )
{
    time_t rawtime;
    time( &rawtime );
    return (UINT64)rawtime;
}

#define _FormatTimeImpl_1(fmt)\
    struct tm* t= localtime ( &ATime ); \
    AResult = Format( fmt, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,\
                        t->tm_hour, t->tm_min, t->tm_sec);

void FormatTime( const time_t &ATime, string &AResult )
{
    _FormatTimeImpl_1( "%d-%02d-%02d %02d:%02d:%02d" );
}

void FormatTime( const time_t &ATime, wstring &AResult )
{
    _FormatTimeImpl_1( L"%d-%02d-%02d %02d:%02d:%02d" );
}

#define _FormatTimeImpl_2(fmt)\
    time_t spanTime = ABigTime - ASmallTime;\
    static const int CtMinteSecond = 60;\
    static const int CtHourSecond = 3600;\
    static const int CtDaySecond = CtHourSecond * 24; \
    int nDay =  (int)spanTime / CtDaySecond;\
    spanTime -= nDay * CtDaySecond;\
    int nHour = (int)spanTime / CtHourSecond;\
    spanTime -= nHour * CtHourSecond;\
    int nMinte = (int)spanTime / CtMinteSecond;\
    spanTime -= nMinte * CtMinteSecond;\
    AResult = Format( fmt, nDay, nHour, nMinte, spanTime );

void FormatTime( const time_t &ABigTime, const time_t &ASmallTime, string &AResult )
{
    _FormatTimeImpl_2( "%d天%d时%d分%d秒" );
}

void FormatTime( const time_t &ABigTime, const time_t &ASmallTime, wstring &AResult )
{
    _FormatTimeImpl_2( L"%d天%d时%d分%d秒" );
}

