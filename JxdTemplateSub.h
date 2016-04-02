/**************************************************************************
	Copyright  : pp.cc 深圳指掌时代 珊瑚组
	File       : JxdTemplateSub.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-10-18 15:47:00
	LastModify : 
	Description: 一些模板函数的实现
**************************************************************************/
#pragma once
#ifndef _JxdTemplateSub_Lib
#define _JxdTemplateSub_Lib

#include "JxdMacro.h"

//检查值, 有需要则更新
template<typename T>
bool UpdateValue(T &ASrcValue, const T &ANewValue)
{
    if ( ASrcValue != ANewValue )
    {
        ASrcValue = ANewValue;
        return true;
    }
    return false;
}

//更新指针, 有需要则创建新指针
template< typename T >
inline T* UpdatePointer( T *&ApSrc, const T *ApNew )
{
    if ( ApNew != NULL )
    {
        if ( NULL == ApSrc ) ApSrc = new T;
        *ApSrc = *ApNew;
    }
    else if ( ApSrc != NULL )
    {
        delete ApSrc;
        ApSrc = NULL;
    }
    return ApSrc;
}

//判断(可能会进行new操作)返回本身
template<typename T>
T* xdCreateObject(T *&ApObject)
{
    if ( !Assigned(ApObject) )
    {
        ApObject = new T;
    }
    return ApObject;
}

//判断对象,返回是否是第一次初始化
template<typename T>
bool CheckObject(T *&ApObject, bool bNewWhenNull, bool bFree)
{
    bool bInit = false;
    if ( bFree )
    {
        SafeDeleteObject( ApObject );
    }
    else if ( bNewWhenNull && !Assigned(ApObject) )
    {
        ApObject = new T;
        bInit = true;
    }
    return bInit;
}

//自动删除对象: 对象必须是 new 出来的
template<typename T>
class CxdAutoDeleteObject
{
public:
    CxdAutoDeleteObject(T *&ApObject){ m_pObject = ApObject; m_pAddress = &ApObject; }
    ~CxdAutoDeleteObject(){ delete m_pObject; memset(m_pAddress, 0, sizeof(T*));  }
private:
    T *m_pObject;
    LPVOID m_pAddress;
};

#endif