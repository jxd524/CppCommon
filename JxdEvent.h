/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdEvent.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-8-15 14:54:42
	LastModify : 
	Description: 事件类,辅助跨类处理,以类指针及成员函数指针为方法进行处理
                
                 CxdNotifyEventT: 支持一个参数
                 CxdNotifyEventExT: 支持两个参数
                 CxdNotifyEventExT: 支持三个参数
                 成员方法:
                    ==, SetParam, Clear, IsOK, GetOwner, GetFunc, Notify

                 CxdNotifyEventListT:    保存多个CxdNotifyEventT
                 CxdNotifyEventListExT:  保存多个CxdNotifyEventExT
                 CxdNotifyEventListEx3T: 保存多个CxdNotifyEventExT
                 成员方法:
                    AddEvent, DeleteEvent, IsExsitsEvent, Notify

                以宏的方式进行实现
**************************************************************************/
#pragma once
     
#ifndef _JxdEvent_Lib
#define _JxdEvent_Lib

#ifdef linux
#include <string.h>
#define Inline 
#else
#define Inline inline
#endif
#include "JxdTypedef.h"
#include "JxdMacro.h"

//====================================================================
// EventInterface
//====================================================================
//
#define _EventInterface( className, eventParamTemplate, eventParamDefine )       \
    template<eventParamTemplate>                                                \
    Interface( I##className )                                                   \
        virtual Inline bool  Notify(eventParamDefine) const = 0;                \
        virtual Inline INT64 GetOwnerAddress(void) const = 0;                   \
        virtual Inline INT64 GetFuncAddress(void) const = 0;                    \
                Inline bool  IsEqual( I##className* AOthrer) {                  \
                    return GetOwnerAddress() == AOthrer->GetOwnerAddress() &&   \
                           GetFuncAddress() == AOthrer->GetFuncAddress(); }     \
    InterfaceEnd

//====================================================================
// _EventClassImpl
//====================================================================
//
#define _EventClassImpl(className, owerClassType, templateParamDefine, eventParamType, eventParamDefine, Params )       \
    template<templateParamDefine> class C##className: public I##className< eventParamType >{                            \
    public:                                                                                                             \
        typedef void (owerClassType::*TNotifyFunc)(eventParamDefine);                                                   \
        typedef I##className<eventParamType> IInterfaceType;                                                            \
        C##className(const C##className &AEvent) {                                                                      \
            m_pOwner = AEvent.m_pOwner;                                                                                 \
            m_pFunc = AEvent.m_pFunc;                                                                                   \
        }                                                                                                               \
        C##className(owerClassType *ApOwner = NULL, const TNotifyFunc fun = NULL) {                                     \
            m_pOwner = ApOwner;                                                                                         \
            m_pFunc = fun;                                                                                              \
        }                                                                                                               \
        bool operator==(const C##className &ARight) const {                                                             \
            return (m_pOwner == ARight.m_pOwner) && (m_pFunc == ARight.m_pFunc);                                        \
        }                                                                                                               \
        inline void SetParam(owerClassType *ApOwner, const TNotifyFunc fun){                                            \
            m_pOwner = ApOwner;                                                                                         \
            m_pFunc = fun;                                                                                              \
        }                                                                                                               \
        inline void Clear(void){                                                                                        \
            m_pOwner = NULL;                                                                                            \
            m_pFunc = NULL;                                                                                             \
        }                                                                                                               \
        inline bool Notify(eventParamDefine) const{                                                                     \
            if ( IsOK() ){ (m_pOwner->*m_pFunc)(Params);  return true; }                                                \
            return false;                                                                                               \
        }                                                                                                               \
        inline bool IsOK() const { return m_pOwner != NULL && m_pFunc != NULL; }                                        \
        inline owerClassType* GetOwner(void) const { return m_pOwner; }                                                 \
        inline INT64 GetOwnerAddress(void) const { return (INT64)m_pOwner; }                                            \
        inline TNotifyFunc GetFunc(void) const { return m_pFunc; }                                                      \
        inline INT64 GetFuncAddress(void) const { return (INT64)(*(INT64*)&m_pFunc); }                                  \
    private:                                                                                                            \
        owerClassType *m_pOwner;                                                                                        \
        TNotifyFunc m_pFunc;                                                                                            \
    };

//====================================================================
// _EventListClassImpl
//====================================================================
//
#define _EventListClassImpl(className, interfaceName, eventParamTemplate, eventParamType, eventParamDefine,             \
                            Params, ContainType)                                                                        \
    template<eventParamTemplate> class className{                                                                       \
    public:                                                                                                             \
        typedef interfaceName< eventParamType > IEventType;                                                             \
    private:                                                                                                            \
        int m_nCount, m_nCapacity;                                                                                      \
        IEventType** m_pEventList;                                                                                      \
    public:                                                                                                             \
        className(){m_nCount = 0; m_nCapacity = 1; m_pEventList = NULL; }                                               \
        ~className() { this->Clear(); }                                                                                 \
                                                                                                                        \
        bool AddEvent(IEventType* ApEvent, bool ACheckExsists = true){                                                  \
            if ( !ACheckExsists || !IsExsitsEvent(ApEvent) ){                                                           \
                if( (m_pEventList == NULL) || (m_nCount + 1 >= m_nCapacity) ) {                                         \
                    m_nCapacity *= 2;                                                                                   \
                    IEventType** pTemp = (IEventType**)malloc( sizeof(int*) * m_nCapacity );                            \
                    if( m_nCount > 0 ) {                                                                                \
                        memcpy( pTemp, m_pEventList, sizeof(IEventType*) * m_nCount );                                  \
                        free( m_pEventList );                                                                           \
                    }                                                                                                   \
                    m_pEventList = pTemp;                                                                               \
                }                                                                                                       \
                m_pEventList[m_nCount++] = ApEvent;                                                                     \
                return true;                                                                                            \
            }                                                                                                           \
            return false;                                                                                               \
        }                                                                                                               \
        void DeleteEvent(IEventType* ApEvent){                                                                          \
            for ( int i = m_nCount - 1; i >= 0; i-- ) {                                                                 \
                if ( m_pEventList[i]->IsEqual(ApEvent) ){                                                               \
                    delete m_pEventList[i];                                                                             \
                    if( i + 1 < m_nCount ){                                                                             \
                        memcpy( m_pEventList + i, m_pEventList + i + 1, (m_nCount - i) * sizeof(IEventType*) );         \
                    }                                                                                                   \
                    m_nCount--;                                                                                         \
                    break;                                                                                              \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
        void ClearEventByOwner(const INT64& AOwnerAddress) {                                                            \
            for ( int i = m_nCount - 1; i >= 0; i-- ) {                                                                 \
                if ( AOwnerAddress == m_pEventList[i]->GetOwnerAddress() ){                                             \
                    delete m_pEventList[i];                                                                             \
                    if( i + 1 < m_nCount ){                                                                             \
                        memcpy( m_pEventList + i, m_pEventList + i + 1, (m_nCount - i) * sizeof(IEventType*) );         \
                    }                                                                                                   \
                    m_nCount--;                                                                                         \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
        bool IsExsitsEvent(IEventType* ApEventAEvent){                                                                  \
            for ( int i = 0; i < m_nCount; i++ ) {                                                                      \
                if ( m_pEventList[i]->IsEqual(ApEventAEvent) ){                                                         \
                    return true;                                                                                        \
                }                                                                                                       \
            }                                                                                                           \
            return false;                                                                                               \
        }                                                                                                               \
        void Notify(eventParamDefine) {                                                                                 \
            for ( int i = 0; i < m_nCount; i++ ) {                                                                      \
                m_pEventList[i]->Notify( Params );                                                                      \
            }                                                                                                           \
        }                                                                                                               \
        void Clear() {                                                                                                  \
            for ( int i = 0; i < m_nCount; i++ ) {                                                                      \
                delete m_pEventList[i];                                                                                 \
            }                                                                                                           \
            free( m_pEventList );                                                                                       \
            m_pEventList = NULL;                                                                                        \
            m_nCount = 0; m_nCapacity = 1;                                                                              \
        }                                                                                                               \
    };

#define _OwerClassType          TClassType

#define _TemplateParamDefine1   typename _OwerClassType, typename TParamType1
#define _EventParamTemplate1    typename TParamType1
#define _EventParamDefine1      TParamType1 AParam1
#define _EventParamType1        TParamType1
#define _Param1                 AParam1

#define _TemplateParamDefine2   _TemplateParamDefine1, typename TParamType2
#define _EventParamTemplate2    _EventParamTemplate1, typename TParamType2
#define _EventParamDefine2      _EventParamDefine1, TParamType2 AParam2
#define _EventParamType2        _EventParamType1, TParamType2
#define _Param2                 _Param1, AParam2

#define _TemplateParamDefine3   _TemplateParamDefine2, typename TParamType3
#define _EventParamTemplate3    _EventParamTemplate2, typename TParamType3
#define _EventParamDefine3      _EventParamDefine2, TParamType3 AParam3
#define _EventParamType3        _EventParamType2, TParamType3
#define _Param3                 _Param2, AParam3

//接口
_EventInterface( xdNotifyEventT, _EventParamTemplate1, _EventParamDefine1 );
_EventInterface( xdNotifyEventExT, _EventParamTemplate2, _EventParamDefine2 );
_EventInterface( xdNotifyEventEx3T, _EventParamTemplate3, _EventParamDefine3 );

//事件
_EventClassImpl( xdNotifyEventT,    _OwerClassType, _TemplateParamDefine1, _EventParamType1, _EventParamDefine1, _Param1 );
_EventClassImpl( xdNotifyEventExT,  _OwerClassType, _TemplateParamDefine2, _EventParamType2, _EventParamDefine2, _Param2 );
_EventClassImpl( xdNotifyEventEx3T, _OwerClassType, _TemplateParamDefine3, _EventParamType3, _EventParamDefine3, _Param3 );

//具体类型事件
typedef CxdNotifyEventT<CxdObject, LPVOID> CxdNotifyEvent;
typedef IxdNotifyEventT<LPVOID>            IxdNotifyEvent;
typedef CxdNotifyEventExT<CxdObject, LPVOID, LPVOID> CxdNotifyEventEx;
typedef IxdNotifyEventExT<LPVOID, LPVOID>            IxdNotifyEventEx;
typedef CxdNotifyEventEx3T<CxdObject, LPVOID, LPVOID, LPVOID> CxdNotifyEventEx3;
typedef IxdNotifyEventEx3T<LPVOID, LPVOID, LPVOID>            IxdNotifyEventEx3;

#ifdef WIN32

//事件列表
_EventListClassImpl( CxdNotifyEventListT, IxdNotifyEventT, _EventParamTemplate1, _EventParamType1, _EventParamDefine1, _Param1, vector ); 
_EventListClassImpl( CxdNotifyEventListExT, IxdNotifyEventExT, _EventParamTemplate2, _EventParamType2, _EventParamDefine2, _Param2, vector ); 
_EventListClassImpl( CxdNotifyEventListEx3T, IxdNotifyEventEx3T, _EventParamTemplate3, _EventParamType3, _EventParamDefine3, _Param3, vector ); 

//具体事件列表
typedef CxdNotifyEventListT<LPVOID> CxdNotifyEventList;
typedef CxdNotifyEventListExT<LPVOID, LPVOID> CxdNotifyEventListEx;
typedef CxdNotifyEventListEx3T<LPVOID, LPVOID, LPVOID> CxdNotifyEventListEx3;
#endif
//new一个事件
#define NewInterface(pOwner, ClassType, ParamType, func)                \
    new CxdNotifyEventT<ClassType, ParamType> ( pOwner, static_cast<    \
        CxdNotifyEventT<ClassType, ParamType>::TNotifyFunc>( &ClassType::func) )
#define NewInterfaceEx(pOwner, ClassType, ParamType1, ParamType2, func)                 \
    new CxdNotifyEventExT<ClassType, ParamType1, ParamType2> ( pOwner, static_cast<     \
        CxdNotifyEventExT<ClassType, ParamType1, ParamType2>::TNotifyFunc>( &ClassType::func) )
#define NewInterfaceEx3(pOwner, ClassType, ParamType1, ParamType2, ParamType3, func)                \
    new CxdNotifyEventEx3T<ClassType, ParamType1, ParamType2, ParamType3> ( pOwner, static_cast<    \
        CxdNotifyEventEx3T<ClassType, ParamType1, ParamType2, ParamType3>::TNotifyFunc>( &ClassType::func) )

//构造一个事件
#define InterfaceObject(pOwner, ClassType, ParamType, func)         \
    CxdNotifyEventT<ClassType, ParamType> ( pOwner, static_cast<    \
        CxdNotifyEventT<ClassType, ParamType>::TNotifyFunc>( &ClassType::func) )
#define InterfaceObjectEx(pOwner, ClassType, ParamType1, ParamType2, func)          \
    CxdNotifyEventExT<ClassType, ParamType1, ParamType2> ( pOwner, static_cast<     \
        CxdNotifyEventExT<ClassType, ParamType1, ParamType2>::TNotifyFunc>( &ClassType::func) )
#define InterfaceObjectEx3(pOwner, ClassType, ParamType1, ParamType2, ParamType3, func)         \
    CxdNotifyEventEx3T<ClassType, ParamType1, ParamType2, ParamType3> ( pOwner, static_cast<    \
        CxdNotifyEventEx3T<ClassType, ParamType1, ParamType2, ParamType3>::TNotifyFunc>( &ClassType::func) )

#define NewInterfaceByClass(ClassType, ParamType, func) \
    NewInterface( this, ClassType, ParamType, func )
#define NewInterfaceExByClass(ClassType, ParamType1, ParamType2, func) \
    NewInterfaceEx( this, ClassType, ParamType1, ParamType2, func )
#define NewInterfaceEx3ByClass(ClassType, ParamType1, ParamType2, ParamType3, func) \
    NewInterfaceEx3( this, ClassType, ParamType1, ParamType2, ParamType3, func )

#define InterfaceObjectByClass(ClassType, ParamType, func) \
    InterfaceObject( this, ClassType, ParamType, func )
#define InterfaceObjectExByClass(ClassType, ParamType1, ParamType2, func) \
    InterfaceObjectEx( this, ClassType, ParamType1, ParamType2, func )
#define InterfaceObjectEx3ByClass(ClassType, ParamType1, ParamType2, ParamType3, func) \
    InterfaceObjectEx3( this, ClassType, ParamType1, ParamType2, ParamType3, func )

#endif