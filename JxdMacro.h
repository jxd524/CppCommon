/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdMacro.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 14:55:53
	LastModify : 
	Description: 共同宏定义处
**************************************************************************/
#pragma once
#ifndef _JxdMacro_Lib
#define _JxdMacro_Lib

/***********************************************************************
    Linux 下相关
************************************************************************/
#ifdef linux

#include "JxdTypedef.h"
#define Inline 
//常量
#define MAXUINT8    ((UINT8)~((UINT8)0))
#define MAXINT8     ((INT8)(MAXUINT8 >> 1))
#define MININT8     ((INT8)~MAXINT8)

#define MAXUINT16   ((UINT16)~((UINT16)0))
#define MAXINT16    ((INT16)(MAXUINT16 >> 1))
#define MININT16    ((INT16)~MAXINT16)

#define MAXUINT32   ((UINT32)~((UINT32)0))
#define MAXINT32    ((INT32)(MAXUINT32 >> 1))
#define MININT32    ((INT32)~MAXINT32)

#define MAXUINT64   ((UINT64)~((UINT64)0))
#define MAXINT64    ((INT64)(MAXUINT64 >> 1))
#define MININT64    ((INT64)~MAXINT64)

#define MAXUINT     ((UINT)~((UINT)0))
#define MAXINT      ((int)(MAXUINT >> 1))
#define MININT      ((int)~MAXINT)

#define MAXWORD     0xffff  
#define MAXDWORD    0xffffffff
#define INFINITE    0xFFFFFFFF

#define MakeUINT64(a, b) ( ((UINT64)(a) << 32) | b )
#define HighUINT64(a) UINT32( (UINT64)(a) >> 32 )
#define LowUINT64(a) UINT32( (UINT64)(a) & 0xFFFFFFFF )

//
#define EqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))
#define MoveMemory(Destination,Source,Length)  memmove((Destination),(Source),(Length))
#define CopyMemory(Destination,Source,Length)  memcpy((Destination),(Source),(Length))
#define FillMemory(Destination,Length,Fill)    memset((Destination),(Fill),(Length))
#define ZeroMemory(Destination,Length)         memset((Destination),0,(Length))

//睡眠时间为毫秒
#define Sleep(ms) usleep( ms * 1000 )
#endif

/***********************************************************************
    Window 下相关
************************************************************************/
#ifdef WIN32
#define Inline inline

#ifdef _MFC_VER
#define WndSleep(count, pms) \
    for( int i = 0; i != count; i++ ) { AfxPumpMessage(); Sleep(pms); }
#endif

#endif

/***********************************************************************
    Windows & Linux 共用
************************************************************************/

//使用 free 释放内存
#define SafeFreeBuffer(ABuffer) \
    if ( ABuffer != NULL ) { free( ABuffer ); ABuffer = NULL; } 

//使用 delete 释放内存
#define SafeDeleteNewBuffer(ABuffer) \
    if ( ABuffer != NULL ) { delete []ABuffer; ABuffer = NULL; }

//安全的释放由 new 创建的对象
#define SafeDeleteObject(ApObject) \
    if ( (ApObject) != NULL ) {  delete (ApObject); ApObject = NULL; }

//判断指针
#define Assigned(ApPoint) ((ApPoint) != NULL)
#define IsNull(ApPoint) (NULL == ApPoint)

//定义简单的属性: 生成 变量m_##FVarName, Get, Set 函数(setState为真为能设置值)
#define _PropertyValue(tlimit, FVarName, type, setState)                        \
    tlimit:                                                                     \
        type m_##FVarName;                                                      \
    public:                                                                     \
        inline type Get##FVarName(void) { return m_##FVarName; }                \
        bool Set##FVarName(const type &ANewValue) {                             \
            if ( setState ) { m_##FVarName = ANewValue; return true; } return false;}

#define PropertyValue(FVarName, type, setState)          _PropertyValue(private,   FVarName, type, setState)
#define PropertyValueProtected(FVarName, type, setState) _PropertyValue(protected, FVarName, type, setState)

//定义简单的属性: 生成 变量m_##FVarName, Get函数
#define _PropertyGetValue(tlimit, FVarName, type)                               \
    tlimit:                                                                     \
        type m_##FVarName;                                                      \
    public:                                                                     \
        Inline type Get##FVarName(void) { return m_##FVarName; }

#define PropertyGetValue(FVarName, type)          _PropertyGetValue(private,   FVarName, type)
#define PropertyGetValueProtected(FVarName, type) _PropertyGetValue(protected, FVarName, type)


//定义指针类型属性: 生成 (new)变量指针m_p##FVarName, Get, Set 函数(setState为真为能设置值)
#define _PropertyPoint(tlimit, FVarName, type, setState)                        \
    tlimit:                                                                     \
        type* m_p##FVarName;                                                    \
    public:                                                                     \
        Inline type* Get##FVarName(void) { return m_p##FVarName; }              \
        bool Set##FVarName(const type* ApNew) {                                 \
            if ( setState ) {xdCheckCreatePointer(m_p##FVarName, ApNew); return true; } return false; }

#define PropertyPoint(FVarName, type, setState)          _PropertyPoint(private,   FVarName, type, setState)
#define PropertyPointProtected(FVarName, type, setState) _PropertyPoint(protected, FVarName, type, setState)

//定义指针类型属性: 指针m_p##FVarName仅引用外部变量, Get, Set 函数(setState为真为能设置值), 
//                  并生成是否自动释放此引用
#define _PropertyResPoint(tlimit, FVarName, type, setState)                                                 \
    tlimit:                                                                                                 \
        type* m_p##FVarName;                                                                                \
        bool  m_bAutoFree##FVarName;                                                                        \
    public:                                                                                                 \
        Inline type* Get##FVarName(void) { return m_p##FVarName; }                                          \
        bool Set##FVarName(type* ApNew) {                                                                   \
            if ( setState ) { CheckFree##FVarName(); m_p##FVarName = ApNew; return true; } return false; }  \
        Inline bool GetAutoFree##FVarName(void) { return m_bAutoFree##FVarName; }                           \
        Inline void SetAutoFree##FVarName(bool AbAuto) { m_bAutoFree##FVarName = AbAuto; }                  \
    protected:                                                                                              \
        Inline void CheckFree##FVarName(void) {                                                             \
            if(Assigned(m_p##FVarName)){ if(m_bAutoFree##FVarName) delete m_p##FVarName; m_p##FVarName = NULL;} }


#define PropertyResPoint(FVarName, type, setState)          _PropertyResPoint(private,   FVarName, type, setState)
#define PropertyResPointProtected(FVarName, type, setState) _PropertyResPoint(protected, FVarName, type, setState)

//定义类接口
#define _PropertyInterface(tlimit, FVarName, type)                                  \
    tlimit:                                                                         \
        type* m_pIt##FVarName;                                                      \
        bool  m_bResOnlyIt##FVarName;                                               \
    public:                                                                         \
        Inline bool  IsExsitsIt##FVarName(void) { return m_pIt##FVarName != NULL; } \
        Inline bool  IsOnlyResIt##FVarName(void) { return m_bResOnlyIt##FVarName; } \
        Inline type* GetIt##FVarName(void) { return m_pIt##FVarName; }              \
        Inline void  SetIt##FVarName(type* ApIt, bool AbResOnly = false) {          \
            DeleteInterface( FVarName );                                            \
            m_pIt##FVarName = ApIt;                                                 \
            m_bResOnlyIt##FVarName = AbResOnly;                                     \
        }

#define PropertyInterface(FVarName, type)          _PropertyInterface(private,   FVarName, type)
#define PropertyInterfaceProtected(FVarName, type) _PropertyInterface(protected, FVarName, type)

#define InitInterface(FVarName) m_pIt##FVarName = NULL; m_bResOnlyIt##FVarName = true;
#define DeleteInterface(FVarName) \
    if ( !m_bResOnlyIt##FVarName) SafeDeleteObject( m_pIt##FVarName ); m_pIt##FVarName = NULL;
#define NotifyInterface(FVarName, v) if ( IsExsitsIt##FVarName() ) m_pIt##FVarName->Notify(v);
#define NotifyInterfaceEx(FVarName, v1, v2) if ( IsExsitsIt##FVarName() ) m_pIt##FVarName->Notify(v1, v2);
#define NotifyInterfaceEx3(FVarName, v1, v2, v3) if ( IsExsitsIt##FVarName() ) m_pIt##FVarName->Notify(v1, v2, v3);


//定义对象列表: 使用vector, 生成m_##FVarName, Find, Add, Delete
#define _PropertyResObjectList(tlimit, FVarName, TElemtClass)                                       \
    tlimit:                                                                                         \
        typedef vector< TElemtClass* >::iterator T##FVarName##IT;                                   \
        vector< TElemtClass* > m_##FVarName;                                                        \
    public:                                                                                         \
        bool Find##FVarName( TElemtClass* ApItem, bool AbRemove ) {                                 \
            for(T##FVarName##IT it = m_##FVarName.begin(); it != m_##FVarName.end(); it++) {        \
                TElemtClass *p = *it; if ( p == ApItem ) {                                          \
                    if( AbRemove) m_##FVarName.erase(it);                                           \
                    return true;} }                                                                 \
            return false; }                                                                         \
        bool Add##FVarName( TElemtClass* ApItem ) {                                                 \
            if( !Find##FVarName(ApItem, false) ) {m_##FVarName.push_back(ApItem); return true;}     \
            return false; }                                                                         \
        void Delete##FVarName( TElemtClass* ApItem ){ Find##FVarName(ApItem, true); }               \
        void ClearAll##FVarName(void) { m_##FVarName.clear(); }

#define PropertyResObjectList(FVarName, TElemtClass) _PropertyResObjectList(private, FVarName, TElemtClass)
#define PropertyResObjectListProtected(FVarName, TElemtClass) _PropertyResObjectList(protected, FVarName, TElemtClass)

#define LoopResObjectList(FVarName, Func)                                                               \
    for(T##FVarName##IT it_a_b_z = m_##FVarName.begin(); it_a_b_z != m_##FVarName.end(); it_a_b_z++ )   \
        { (*it_a_b_z)->Func;}

//定义Active属性
#define PropertyActive(ActiveFunc, UnActiveFunc)                                         \
    private:                                                                             \
        bool m_bActive;                                                                  \
    public:                                                                              \
        Inline bool GetActive(void) { return m_bActive; }                                \
        bool SetActive(bool bActive) {                                                   \
            if (m_bActive != bActive) { m_bActive = bActive ? ActiveFunc : !UnActiveFunc;\
                                        return m_bActive == bActive;}  return false; }

//定义接口 必须成对出现
#define Interface(IName) struct IName { virtual ~IName(){}
#define InterfaceEnd };
    
//定义常量  const int CtFVarNameSize = sizeof(?FVarName);
#define DefineSize(FVarName)  const int Ct##FVarName##Size = sizeof(##FVarName);
#define DefineTSize(FVarName) const int Ct##FVarName##Size = sizeof(T##FVarName);
#define DefineCSize(FVarName) const int Ct##FVarName##Size = sizeof(C##FVarName);

//判断是否是2的N次方
#define IsN2(N) (0 == ((N) & ((N) -1) ) )

//MOD: B必须是2的N次方
#define Mod(A, B) ((A) & ((B) - 1) )

//方便测试
#define LoopFun(Count, funName) for(int i = 0; i != Count; i++ ) funName;



//自动释放全局对象, 在全局对象构造函数调用
#define AutoDeleteGloableObject(className) \
    static CxdAutoDeleteObject<className> _auto_delete_glob_obj_( const_cast<className*>(this) );

#endif