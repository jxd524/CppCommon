/**************************************************************************
	Copyright  : pp.cc ����ָ��ʱ�� ɺ����
	File       : JxdTemplateSub.h & .cpp 
	Author     : Terry
	Email      : jxd524@163.com
	CreateTime : 2013-10-18 15:47:00
	LastModify : 
	Description: һЩģ�庯����ʵ��
**************************************************************************/
#pragma once
#ifndef _JxdTemplateSub_Lib
#define _JxdTemplateSub_Lib

#include "JxdMacro.h"

//���ֵ, ����Ҫ�����
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

//����ָ��, ����Ҫ�򴴽���ָ��
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

//�ж�(���ܻ����new����)���ر���
template<typename T>
T* xdCreateObject(T *&ApObject)
{
    if ( !Assigned(ApObject) )
    {
        ApObject = new T;
    }
    return ApObject;
}

//�ж϶���,�����Ƿ��ǵ�һ�γ�ʼ��
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

//�Զ�ɾ������: ��������� new ������
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