#include "StdAfx.h"
#include "JxdXmlBasic.h"
#include "JxdMacro.h"
#include "JxdFileSub.h"
#include "JxdStringCoder.h"
#include "JxdStringHandle.h"
#include "Markup.h"

#ifdef WIN32
CxdXmlBasic::CxdXmlBasic( const wstring &AFileName /*= ""*/ )
{
    m_SaveOnFree = true;
    m_RootName = "Config";
    SetConfigFileName( UnicodeToAscii(AFileName.c_str()) );
}
void CxdXmlBasic::LoadFromXml( const wstring &AXmlFileName )
{
    LoadFromXml( UnicodeToAscii(AXmlFileName.c_str()) );
}
void CxdXmlBasic::SaveToXml( wstring AFileName /*= ""*/ )
{
    SaveToXml( UnicodeToAscii(AFileName.c_str()) );
}
#endif

CxdXmlBasic::CxdXmlBasic( const string &AFileName /*= TEXT("")*/ )
{
    m_SaveOnFree = true;
    m_RootName = "Config";
    SetConfigFileName( AFileName );
}

CxdXmlBasic::~CxdXmlBasic(void)
{
    if ( m_SaveOnFree )
    {
        SaveToXml( m_ConfigFileName );
    }
    ClearConfig();
}

void CxdXmlBasic::ClearConfig( void )
{
    for ( TConfigIT it = m_ltConfig.begin(); it != m_ltConfig.end(); it++ )
    {
        TConfigNode *pNode = *it;
        if ( Assigned(pNode) )
        {
            TConfigParam *pParam = pNode->FpInfo;
            while ( Assigned(pParam) )
            {
                TConfigParam *pTemp = pParam->FpNext;
                delete pParam;
                pParam = pTemp;
            }
            delete pNode;
        }
    }
    m_ltConfig.clear();
}

void CxdXmlBasic::LoadFromXml( const string &AXmlFileName )
{
    m_RootName.clear();
    ClearConfig();
    CMarkup xml;
    if (!xml.Load(AXmlFileName.c_str()) ) return;

    if ( xml.FindElem() )
    {
        m_RootName = xml.GetTagName();

        while ( xml.FindChildElem() )
        {
            xml.IntoElem();
            string strNode = xml.GetTagName();
            int n( 0 );
            while ( true )
            {
                string strName = xml.GetAttribName( n++ );
                if ( strName.empty())
                {
                    break;
                }
                string strValue = xml.GetAttrib( strName );
                AddConfigInfo( strNode, strName, strValue );
            }
            xml.OutOfElem();
        }
    }
}

void CxdXmlBasic::SaveToXml( string AXmlFileName )
{
    if ( AXmlFileName.empty() )
    {
        AXmlFileName = m_ConfigFileName;
    }
    if ( m_RootName.empty() ) m_RootName = "Config";
    if ( m_ltConfig.empty() ) return;

    CMarkup xml;
    xml.AddElem( m_RootName.c_str() );
    for ( TConfigIT it = m_ltConfig.begin(); it != m_ltConfig.end(); it++ )
    {
        TConfigNode *pNode = *it;

        if ( !xml.FindChildElem(pNode->FNodeName) )
        {
            xml.AddChildElem( pNode->FNodeName );
        }

        xml.IntoElem();

        TConfigParam *pParam = pNode->FpInfo;
        while ( Assigned(pParam) )
        {
            xml.AddAttrib( pParam->FName.c_str(), pParam->FValue.c_str() );
            pParam = pParam->FpNext;
        }

        xml.OutOfElem();
    }
    xml.Save( AXmlFileName );
}

bool CxdXmlBasic::DoSettingConfigFileName( const string &ANewValue )
{
    if ( FileExist(ANewValue.c_str()) )
    {
        LoadFromXml( ANewValue );
    }
    return true;
}

void CxdXmlBasic::AddConfigInfo( const string &ANodeName, const string &AName, const string &AValue )
{
    TConfigNode *pNode = FindConfigInfo( ANodeName );
    if ( !Assigned(pNode) )
    {
        pNode = new TConfigNode;
        pNode->FNodeName = ANodeName;
        pNode->FpInfo = NULL;
        m_ltConfig.push_back( pNode );
    }
    TConfigParam *pItem = NULL;
    if ( FindConfigInfo(pNode->FpInfo, AName, pItem) )
    {
        pItem->FValue = AValue;
    }
    else
    {
        TConfigParam *pParam  = new TConfigParam;
        pParam->FName = AName;
        pParam->FValue = AValue;
        pParam->FpNext = NULL;
        Assigned(pItem) ? pItem->FpNext = pParam : pNode->FpInfo = pParam;
    }
}

TConfigNode* CxdXmlBasic::FindConfigInfo( const string &ANodeName )
{
    for ( TConfigIT it = m_ltConfig.begin(); it != m_ltConfig.end(); it++ )
    {
        TConfigNode *pNode = *it;
        if ( CompareStringCase(ANodeName.c_str(), pNode->FNodeName.c_str()) )
        {
            return pNode;
        }
    }
    return NULL;
}

bool CxdXmlBasic::FindConfigInfo( TConfigParam *ApParam, const string &AName, TConfigParam *&ApFind, bool bByComparentText )
{
    TConfigParam *pPre = NULL;
    while ( Assigned(ApParam) )
    {
        pPre = ApParam;
        if ( bByComparentText )
        {
            if ( CompareString(ApParam->FName.c_str(), AName.c_str()) )
            {
                ApFind = ApParam;
                return true;
            }
        }
        else
        {
            if ( -1 != Pos(ApParam->FName.c_str(), AName.c_str(), 0) )
            {
                ApFind = ApParam;
                return true;
            }
        }
        ApParam = ApParam->FpNext;
    }
    ApFind = pPre;
    return false; 
}

string CxdXmlBasic::GetValue( const string &ANode, const string &AAttName )
{
    TConfigNode *pNode = FindConfigInfo( ANode );
    if ( Assigned(pNode) )
    {
        TConfigParam *pItem = NULL;
        if ( FindConfigInfo(pNode->FpInfo, AAttName, pItem) )
        {
            return pItem->FValue;
        }
    }    
    return "";
}

wstring CxdXmlBasic::GetValue( const wstring &ANode, const wstring &AAttName )
{
#ifdef WIN32
    string strNode = UnicodeToAscii( ANode.c_str() );
    string strName = UnicodeToAscii( AAttName.c_str() );
    return AsciiToUnicode( GetValue(strNode, strName).c_str() );
#endif
    throw "do not complete at linux";
}

void CxdXmlBasic::SetValue( const string &ANode, const string &AAttName, const string &AValue )
{
    AddConfigInfo( ANode, AAttName, AValue );
}

void CxdXmlBasic::SetValue( const wstring &ANode, const wstring &AAttName, const wstring &AValue )
{
#ifdef WIN32
    string strNode = UnicodeToAscii( ANode.c_str() );
    string strAttName = UnicodeToAscii( AAttName.c_str() );
    string strValue = UnicodeToAscii( AValue.c_str() );
    SetValue( strNode, strAttName, strValue );
#endif
    throw "do not complete at linux";
}

string CxdXmlBasic::GetString( const string &ANode, const string &AAttName, const string& ADefaultValue )
{
    string s = GetValue( ANode, AAttName );
    if ( s.empty() )
    {
        return ADefaultValue;
    }
    return s;
}

wstring CxdXmlBasic::GetString( const wstring &ANode, const wstring &AAttName, const wstring& ADefaultValue )
{
    wstring s = GetValue( ANode, AAttName );
    if ( s.empty() )
    {
        return ADefaultValue;
    }
    return s;
}

#ifdef WIN32

CString CxdXmlBasic::GetCString( const TCHAR* ANode, const TCHAR* AAttName, const TCHAR* ADefaultValue )
{
    CString s = GetValue( ANode, AAttName ).c_str();
    if ( s.IsEmpty() )
    {
        s = ADefaultValue;
    }
    return s;
}


#endif

bool    StringToBooleanA(const string& AValue) { return CompareString( AValue.c_str(), "true" );  }
bool    StringToBooleanW(const wstring& AValue){ return CompareString( AValue.c_str(), L"true" ); }
string  BooleanToStringA(boolean b) { return b ? "true" : "false"; }
wstring BooleanToStringW(boolean b) { return b ? L"true" : L"false"; }

INT64   StringToInt64A(const string& AValue) { return StrToInt64( AValue.c_str() ); }
INT64   StringToInt64W(const wstring& AValue){ return StrToInt64( AValue.c_str() ); }

#define _ClassFuncImpl(FuncName, returnType, defaultValue, StrToType, TypeToStr)                                    \
    returnType CxdXmlBasic::Get##FuncName( const string &ASeccion, const string &ANodeName, returnType ADefault ){  \
        string strValue( GetValue(ASeccion, ANodeName) );                                                           \
        if( strValue.empty() ) return ADefault;                                                                     \
        return (returnType)StrToType##A( strValue );                                                                \
    }                                                                                                               \
    returnType CxdXmlBasic::Get##FuncName( const wstring &ASeccion, const wstring &ANodeName, returnType ADefault ){\
        wstring strValue( GetValue(ASeccion, ANodeName) );                                                          \
        if( strValue.empty() ) return ADefault;                                                                     \
        return (returnType)StrToType##W( strValue );                                                                \
    }                                                                                                               \
    void CxdXmlBasic::Set##FuncName(const string &ASeccion, const string &ANodeName, returnType b){                 \
        SetValue( ASeccion, ANodeName, TypeToStr##A(b) );                                                           \
    }                                                                                                               \
    void CxdXmlBasic::Set##FuncName(const wstring &ASeccion, const wstring &ANodeName, returnType b){               \
        SetValue( ASeccion, ANodeName, TypeToStr##W(b) );                                                           \
    }

_ClassFuncImpl(Boolean, bool, false, StringToBoolean, BooleanToString);
_ClassFuncImpl(Integer, int, -1, StringToInt64, Int64ToStr);
_ClassFuncImpl(Int64, INT64, -1, StringToInt64, Int64ToStr);