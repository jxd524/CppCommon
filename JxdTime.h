/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdTime.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 15:01:31
	LastModify : 
	Description: 有关时间, 计时等操作封闭
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

//获取程序运行到现在的总秒数( Linux: 系统运行到被调用的时间, Window: 程序运行到被调用的时间 )
UINT64 GetRunningSecond(void);
UINT64 GetRunningMSecond(void);

//string类型转Time
__time64_t GetTimeFromString(const char *ApTime);
__time64_t GetTimeFromString(const wchar_t *ApTime);

//当前系统时间
time_t GetCurrenTime(void); 

//将时间格式化为: YYYY-MM-DD HH:MM::SS
void   FormatTime(const time_t &ATime, string &AResult); 
void   FormatTime(const time_t &ATime, wstring &AResult);

//将两个时间的差格式化为: %d天%d时%d分%d秒
void   FormatTime(const time_t &ABigTime, const time_t &ASmallTime, string &AResult); 
void   FormatTime(const time_t &ABigTime, const time_t &ASmallTime, wstring &AResult);

#ifdef linux
//需要 g++ -lrt
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