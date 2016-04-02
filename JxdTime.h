/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdTime.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 15:01:31
	LastModify : 
	Description: �й�ʱ��, ��ʱ�Ȳ������
**************************************************************************/
#pragma once

#ifndef _JxdTime_Lib
#define _JxdTime_Lib

#ifdef linux
#include "JxdTypedef.h"
#endif
#include <string>
using std::string;
using std::wstring;

//��ȡ�������е����ڵ�������( Linux: ϵͳ���е������õ�ʱ��, Window: �������е������õ�ʱ�� )
UINT64 GetRunningSecond(void);
UINT64 GetRunningMSecond(void);

//string����תTime
__time64_t GetTimeFromString(const char *ApTime);
__time64_t GetTimeFromString(const wchar_t *ApTime);

//��ǰϵͳʱ��
time_t GetCurrenTime(void); 

//��ʱ���ʽ��Ϊ: YYYY-MM-DD HH:MM::SS
void   FormatTime(const time_t &ATime, string &AResult); 
void   FormatTime(const time_t &ATime, wstring &AResult);

//������ʱ��Ĳ��ʽ��Ϊ: %d��%dʱ%d��%d��
void   FormatTime(const time_t &ABigTime, const time_t &ASmallTime, string &AResult); 
void   FormatTime(const time_t &ABigTime, const time_t &ASmallTime, wstring &AResult);

#ifdef linux
//��Ҫ g++ -lrt
#include <time.h>
class CxdRunSpaceTime;
#define BILLION  1000000000.0;

#define BeginTickCount(s) \
    CxdRunSpaceTime _st_obj_tick; cout<<s<<endl;
#define RecoveryTickCount(s) \
    _st_obj_tick.RecoveryTime(); cout<<s<<endl;
#define EndTickCount_cout \
    {timespec _tend_cout;\
     _st_obj_tick.GetCurSpaceTime(_tend_cout);\
     double d = _tend_cout.tv_sec + _tend_cout.tv_nsec / BILLION; \
     cout<<"  Run space time("<<__FUNCTION__<<"-"<<__LINE__<<"): "<< d << " s"<<endl; }


class CxdRunSpaceTime
{
public:
    CxdRunSpaceTime() { clock_gettime(CLOCK_MONOTONIC, &m_lBeginTime); }
    inline void GetCurSpaceTime(timespec &ASpaceTime)
    {
        clock_gettime(CLOCK_MONOTONIC, &ASpaceTime);
        ASpaceTime.tv_nsec = ASpaceTime.tv_nsec - m_lBeginTime.tv_nsec;
        ASpaceTime.tv_sec = ASpaceTime.tv_sec - m_lBeginTime.tv_sec;
    }
    inline void RecoveryTime(void) { clock_gettime(CLOCK_MONOTONIC, &m_lBeginTime); }
private:
    timespec m_lBeginTime;
};
#endif

#endif