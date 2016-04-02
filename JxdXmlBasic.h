#pragma once

/*
xml ≈‰÷√ ∂®“Â

<Config>
    <MyItem1 att1="v1" att2="v2" />
    <MyItem2 att22="v12" att22="v22" />
</Config>

*/

#ifdef linux
#include "JxdTypedef.h"
#endif
#include <vector>
#include <string>
using namespace std;

#include "JxdMacro.h"

struct TConfigParam
{
    string FName, FValue;
    TConfigParam *FpNext;
};

struct TConfigNode
{
    string FNodeName;
    TConfigParam *FpInfo;
};

typedef vector<TConfigNode*> TConfigList;
typedef TConfigList::iterator TConfigIT;

#define _PropertyXml(fType, paramType, seccion, nodeName, defaultV, runWhenSetOK)\
    public:\
        paramType  Get##nodeName() { return Get##fType(seccion, #nodeName, defaultV); }\
        bool Set##nodeName(paramType ANewValue) { if( Get##nodeName() != ANewValue ) { Set##fType(seccion, #nodeName, ANewValue); runWhenSetOK; } return false; }

#define _PropertyXmlW(fType, paramType, seccion, nodeName, defaultV, runWhenSetOK)\
    public:\
        paramType  Get##nodeName() { return Get##fType(seccion, L###nodeName, defaultV); }\
        bool Set##nodeName(paramType ANewValue) { if( Get##nodeName() != ANewValue ) { Set##fType(seccion, L###nodeName, ANewValue); runWhenSetOK; } return false; }

#define PropertyXmlInt_Run(seccion, nodeName, defaultV, runWhenSetOK)\
    _PropertyXml( Integer, int, seccion, nodeName, defaultV, runWhenSetOK )
#define PropertyXmlInt(seccion, nodeName, defaultV) PropertyXmlInt_Run(seccion, nodeName, defaultV, ;)

#define PropertyXmlInt64_Run(seccion, nodeName, defaultV, runWhenSetOK)\
    _PropertyXml( Int64, INT64, seccion, nodeName, defaultV, runWhenSetOK )
#define PropertyXmlInt64(seccion, nodeName, defaultV) PropertyXmlInt64_Run(seccion, nodeName, defaultV, ;)

#define PropertyXmlBoolean_Run(seccion, nodeName, defaultV, runWhenSetOK)\
    _PropertyXml( Boolean, bool, seccion, nodeName, defaultV, runWhenSetOK )
#define PropertyXmlBoolean(seccion, nodeName, defaultV) PropertyXmlBoolean_Run(seccion, nodeName, defaultV, ;)

#define PropertyXmlString_Run(seccion, nodeName, defaultV, runWhenSetOK)\
    _PropertyXml( String, string, seccion, nodeName, defaultV, runWhenSetOK )
#define PropertyXmlString(seccion, nodeName, defaultV) PropertyXmlString_Run(seccion, nodeName, defaultV, ;)

#define PropertyXmlWString_Run(seccion, nodeName, defaultV, runWhenSetOK)\
    _PropertyXmlW( String, wstring, seccion, nodeName, defaultV, runWhenSetOK )
#define PropertyXmlWString(seccion, nodeName, defaultV) PropertyXmlWString_Run(seccion, nodeName, defaultV, ;)

#define PropertyXmlCString_Run(seccion, nodeName, defaultV, runWhenSetOK)\
    _PropertyXmlW( CString, CString, seccion, nodeName, defaultV, runWhenSetOK )
#define PropertyXmlCString(seccion, nodeName, defaultV) PropertyXmlCString_Run(seccion, nodeName, defaultV, ;)

class CxdXmlBasic
{
public:
    CxdXmlBasic(const string &AFileName);
#ifdef WIN32
    CxdXmlBasic(const wstring &AFileName);
#endif
    virtual ~CxdXmlBasic(void);

    void Flush(void) { SaveToXml(m_ConfigFileName); }

    PropertyValue( SaveOnFree, bool, true );
    PropertyValueProtected( ConfigFileName, string, DoSettingConfigFileName(ANewValue) );
    PropertyValueProtected( RootName, string, true );
protected:
    TConfigList m_ltConfig;

    void SaveToXml(string AFileName = "");
    void LoadFromXml(const string &AXmlFileName);

#ifdef WIN32
    void LoadFromXml(const wstring &AXmlFileName);
    void SaveToXml(wstring AFileName);
#endif

    string   GetValue(const string &ANode, const string &AAttName);
    wstring  GetValue(const wstring &ANode, const wstring &AAttName);
    void     SetValue(const string &ANode, const string &AAttName, const string &AValue);
    void     SetValue(const wstring &ANode, const wstring &AAttName, const wstring &AValue);

#define _ClassFuncDefine(FuncName, returnType, defaultValue )\
    returnType Get##FuncName(const string &ASeccion, const string &ANodeName, returnType ADefault = defaultValue);   \
    returnType Get##FuncName(const wstring &ASeccion, const wstring &ANodeName, returnType ADefault = defaultValue); \
    void       Set##FuncName(const string &ASeccion, const string &ANodeName, returnType b);                         \
    void       Set##FuncName(const wstring &ASeccion, const wstring &ANodeName, returnType b);

    //GetBoolean, SetBoolean
    _ClassFuncDefine(Boolean, bool, false );

    //GetInteger, SetInteger
    _ClassFuncDefine(Integer, int, -1 );

    //GetInt64, SetInt64
    _ClassFuncDefine(Int64, INT64, -1 );

    string   GetString(const string &ANode, const string &AAttName, const string& ADefaultValue);
    wstring  GetString(const wstring &ANode, const wstring &AAttName, const wstring& ADefaultValue);
    void     SetString(const string &ANode, const string &AAttName, const string &AValue) { SetValue(ANode, AAttName, AValue); }
    void     SetString(const wstring &ANode, const wstring &AAttName, const wstring &AValue){ SetValue(ANode, AAttName, AValue); }

#ifdef WIN32
    CString GetCString(const TCHAR* ANode, const TCHAR* AAttName, const TCHAR* ADefaultValue);
    void    SetCString(const TCHAR* ANode, const TCHAR* AAttName, const TCHAR* AValue){ SetValue(ANode, AAttName, AValue); }
#endif

    void ClearConfig(void);

    void         AddConfigInfo(const string &ANodeName, const string &AName, const string &AValue);
    string       GetConfigValue(const string &ANodeName, const string &AName);
    TConfigNode* FindConfigInfo(const string &ANodeName);
    bool         FindConfigInfo(TConfigParam *ApParam, const string &AName, TConfigParam *&ApFind, bool bByComparentText = true);
private:
    bool DoSettingConfigFileName(const string &ANewValue);
};