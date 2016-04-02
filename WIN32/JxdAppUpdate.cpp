#include "stdafx.h"
#include "JxdAppUpdate.h"
#include "JxdNetwork.h"
#include "JxdFileSub.h"
#include "JxdSysSub.h"
#include "JxdTemplateSub.h"
#include "JxdZipSub.h"

/*
节点名称: 软件EXE名
version: 软件版本号
perm: 动作. 对解压的ZIP都进行删除处理
		2  : 对本身做处理: 判断版本号, 进行下载
		4  : 对指定项的版本号进行判断版, 并下载. 如果downItems不为空,则只处理其中的项, 否则处理除本身外的所有项
		8  : 解压ZIP包, 解压指定项unzipItems (), 当解压成功时, 则删除此ZIP包, 如果解压不成功,则由外部进行提示(重启)
		16 : 运行指定项 runItems
url: 文件URL
size: 文件大小 
hash: 文件HASH值
content: 此次更新的内容说明
closeParent: 当项被成功运行之后,是否关闭启动它的进程
downItems: 需要下载的项
unzipItems: 需要解压的项
runItems: 需要运行的项

XML: 
<upConfig>
	<softName1 version="1.1.0.1" perm="2+4+16" url="http://t.pp.cc/wbClient/testUpdate/PPDS.zip" size="87123" hash="xx" content="yy" downItems="softName2"unzipItems="softName2" runItems="softName2,softName3"/>
	<softName2 version="1.0.0.1" perm="0" url="http://t.pp.cc/wbClient/testUpdate/PPDS.zip" size="87123" hash="xx" content="yy" closeParent="true"/>
	<softName3 version="1.0.0.1" perm="0" url="http://t.pp.cc/wbClient/testUpdate/PPDS.zip" size="87123" hash="xx" content="yy"/>
	<softName4 version="1.0.0.1" perm="0" url="http://t.pp.cc/wbClient/testUpdate/PPDS.zip" size="87123" hash="xx" content="yy"/>
</upConfig>
*/

//比较 version 值
int CompareVersion(CString v1, CString v2)
{
    int nResult(0);

    TStringList lt1, lt2;
    ParseFormatString( v1, TEXT("."), lt1 );
    if ( lt1.empty() )
    {
        ParseFormatString( v1, TEXT(","), lt1 );
    }
    ParseFormatString( v2, TEXT("."), lt2 );
    if ( lt2.empty() )
    {
        ParseFormatString( v2, TEXT(","), lt2 );
    }

    TStringWListIT it1 = lt1.begin();
    TStringWListIT it2 = lt2.begin();
    while (it1 != lt1.end() && it2 != lt2.end())
    {
        int n1 = _ttoi( (*it1).c_str() );
        int n2 = _ttoi((*it2).c_str());
        nResult = n1 - n2;
        if ( 0 == nResult )
        {
            it1++;
            it2++;
            continue;
        }
        break;
    }
    return nResult;
}

//"2+4+16" 转成数字
int ConvertPerm(CString v)
{
    int nResult(0);
    TStringList lt;
    ParseFormatString( v, TEXT("+"), lt );
    for ( TStringListIT it = lt.begin(); it != lt.end(); it++ )
    {
        nResult += _ttoi( (*it).c_str() );
    }
    return nResult;
}

bool CovnertItems(CString v, TStringList &ALt)
{
    if ( v.IsEmpty() ) return false;
    ParseFormatString( v, TEXT(","), ALt );
    for ( TStringListIT it = ALt.begin(); it != ALt.end(); it++ )
    {
        CxdString str = *it;
        TrimString( str );
        *it = str;
    }
    return true;
}

void CovnertItemName(CString v, CString &AName, CString &AExt)
{
    TxdParseInfo info;
    if ( ParseFormatString(v, TEXT("."), info) )
    {
        AName = info.First.c_str();
        AExt = info.Second.c_str();
    }
    else
    {
        AName = v;
        AExt = TEXT("exe");
    }
}

class CxdUpConfigXml: public CxdXmlBasic
{
public:
    CxdUpConfigXml(const TCHAR* ApFileName);

    //必须先设置 AInfo.name 的值
    bool GetConfig(TxdUpConfigInfo& AInfo);

    //
    void GetAllItems(TStringList& AList);
private:
    typedef vector<TxdUpConfigInfo> TUpConfigList;
    typedef TUpConfigList::iterator TUpConfigListIT;
    TUpConfigList m_ltUpConfig;
};

CxdUpConfigXml::CxdUpConfigXml( const TCHAR* ApFileName ): CxdXmlBasic(ApFileName)
{
#define ReadInfo(name) \
    if ( FindConfigInfo( pNode->FpInfo, #name, param, true ) ) info.name = AsciiToUnicode( param->FValue.c_str() ).c_str();

    SetSaveOnFree( false );
    TConfigNode* pNode;
    TConfigParam *param;
    for ( TConfigIT it = m_ltConfig.begin(); it != m_ltConfig.end(); it++ )
    {
        pNode = *it;
        TxdUpConfigInfo info;
        info.name = AsciiToUnicode( pNode->FNodeName.c_str() ).c_str();
        ReadInfo( version );
        ReadInfo( perm );
        ReadInfo( url );
        ReadInfo( size );
        ReadInfo( hash );
        ReadInfo( closeParent );
        ReadInfo( content );
        ReadInfo( downItems );
        ReadInfo( unzipItems );
        ReadInfo( runItems );
        m_ltUpConfig.push_back( info );
    }
}

bool CxdUpConfigXml::GetConfig( TxdUpConfigInfo& AInfo )
{
    for (TUpConfigListIT it = m_ltUpConfig.begin(); it != m_ltUpConfig.end(); it++ )
    {
        TxdUpConfigInfo info = *it;
        if ( info.name.CompareNoCase(AInfo.name) == 0 )
        {
            AInfo = info;
            return true;
        }
    }
    return false;
}

void CxdUpConfigXml::GetAllItems( TStringList& AList )
{
    for (TUpConfigListIT it = m_ltUpConfig.begin(); it != m_ltUpConfig.end(); it++ )
    {
        TxdUpConfigInfo info = *it;
        CxdString str = info.name;
        AList.push_back( str );
    }
}


//CxdAppUpdate
CxdAppUpdate::CxdAppUpdate( const TCHAR* ApConfigURL, int ADelaySecond, const TCHAR* ApUpdateDirName )
{
    InitInterface( DownZip );
    InitInterface( UnZipFile );
    InitInterface( RunApp );
    InitInterface( CloseSelf );
    InitInterface( Finished );
    m_UpdateState = auChecking;
    m_AppName = GetCurProcessName();
    m_CurPath = GetCurProcessPath();
    m_ConfigURL = ApConfigURL;
    m_ConfigFileName =  m_CurPath + m_AppName + TEXT("_UpConfig.xml");
    m_ZipTempDir = m_CurPath;
    if ( Assigned(ApUpdateDirName) )
    {
        m_ZipTempDir += ApUpdateDirName;
        m_ZipTempDir = IncludeTrailingPathDelimiter( m_ZipTempDir ).c_str();
    }
    m_bWaitFree = false;
    int n = ADelaySecond > 0 ? ADelaySecond * 1000 : 1000;
    m_pThread = CreateLoopExecuteWinThread( CxdAppUpdate, LPVOID, DoThreadCheckApp, NULL, n );
    m_pThread->SetAutoFreeThreadObject( false );
    m_pThread->SetItThreadExecuteFinished( NewInterfaceByClass(CxdAppUpdate, LPVOID, DoThreadFinished) );
}

CxdAppUpdate::~CxdAppUpdate()
{
    m_bWaitFree = true;
    if ( Assigned(m_pThread) )
    {
        AutoMutex( m_lock );
        if ( Assigned(m_pThread) )
        {
            m_pThread->SetItThreadExecuteFinished( NULL );
            m_pThread->SetAutoFreeThreadObject( true );
            m_pThread->SetLoopExecuteSpaceTime( 0 );
            m_pThread->PostEvent();
            m_pThread = NULL;
        }
    }
    DeleteInterface( DownZip );
    DeleteInterface( UnZipFile );
    DeleteInterface( RunApp );
    DeleteInterface( CloseSelf );
    DeleteInterface( Finished );
    DeleteFile( m_ConfigFileName );
}

void CxdAppUpdate::CheckNow()
{
    if ( Assigned(m_pThread) )
    {
        m_pThread->PostEvent();
    }
}


void CxdAppUpdate::DoThreadFinished( LPVOID )
{
    NotifyInterface( Finished, this );
    if ( Assigned(m_pThread) )
    {
        AutoMutex( m_lock );
        if ( Assigned(m_pThread) )
        {
            m_pThread->SetAutoFreeThreadObject( true );
            m_pThread->SetLoopExecuteSpaceTime( 0 );
            m_pThread->PostEvent();
            m_pThread = NULL;
        }
    }
    DeleteFile( m_ConfigFileName );
}

void CxdAppUpdate::DoThreadCheckApp( LPVOID )
{
#define ReturnError { m_UpdateState = auFailed; return; }

    AutoMutex( m_lock );

    m_pThread->SetLoopExecuteSpaceTime( 0 );
    m_pThread->PostEvent();

    m_UpdateState = auChecking;
    if( !DownFile(m_ConfigURL, m_ConfigFileName) ) ReturnError;

    m_pConfig = new CxdUpConfigXml( m_ConfigFileName );
    CxdAutoDeleteObject<CxdUpConfigXml> ao(m_pConfig);

    TxdUpConfigInfo info;
    info.name = m_AppName;
    if ( !m_pConfig->GetConfig(info) ) ReturnError;

    int nPerm = ConvertPerm( info.perm );
    if ( !m_bWaitFree && nPerm & 2 )
    {
        //对指定项的版本号进行判断版, 并下载. 如果downItems不为空,则只处理其中的项, 否则处理所有项
        m_UpdateState = auDownFile;
        TStringList lt;
        if ( !CovnertItems(info.downItems, lt) ) m_pConfig->GetAllItems( lt );
        if ( DoDownByList(lt) == 0 )
        {
            m_UpdateState = auRecentVersion;
            return;
        }
    }
    if ( !m_bWaitFree && nPerm & 4 )
    {
        //解压ZIP包, 解压指定项unzipItems (), 当解压成功时, 则删除此ZIP包
        TStringList lt;
        if ( CovnertItems(info.unzipItems, lt) )
        {
            DoUnzipByList( lt );
        }
    }
    if ( !m_bWaitFree && m_bCurDownNewVersion && nPerm & 8 )
    {
        //运行指定项 runItems
        TStringList lt;
        if ( CovnertItems(info.runItems, lt) )
        {
            DoRunAppByList( lt );
        }
    }
    m_UpdateState = auFinsihed;
}

int CxdAppUpdate::DoDownByList( TStringList& AList )
{
    int nResult = 0;
    m_bCurDownNewVersion = false;
    for ( TStringListIT it = AList.begin(); it != AList.end(); it++ )
    {
        if ( m_bWaitFree ) return nResult;

        CxdString str = *it;
        CString strName, strExt;
        CovnertItemName( str.c_str(), strName, strExt );

        TxdUpConfigInfo info;
        info.name = strName; 
        if ( m_pConfig->GetConfig(info) )
        {
            CString strLocatAppFileName = m_CurPath + strName + TEXT(".") + strExt;
            CString strVersion = GetAppVersion( strLocatAppFileName ); 
            
            if ( strVersion.IsEmpty() || CompareVersion(info.version, strVersion) > 0 )
            {
                DeleteFile( GetZipFileName(strName) ); //直接重新下载
                ForceDirectories( m_ZipTempDir );
                bool bDown( true );
                NotifyInterfaceEx( DownZip, info, bDown );
                if ( bDown )
                {
                    CString strDownName = GetTempZipFileName( strName );
                    if ( DownFile( info.url, strDownName ) )
                    {
                        MoveFile( strDownName, GetZipFileName(strName) );
                        if ( !m_bCurDownNewVersion && CompareString(m_AppName, info.name) )
                        {
                            m_bCurDownNewVersion = true;
                        }
                        nResult++;
                    }
                    else
                    {
                        DeleteFile( strDownName );
                    }
                }
            }
        }
    }
    return nResult;
}

void CxdAppUpdate::DoUnzipByList( TStringList& AList )
{
    for ( TStringListIT it = AList.begin(); it != AList.end(); it++ )
    {
        if ( m_bWaitFree ) return;
        TxdUpConfigInfo info;
        info.name = (*it).c_str();
        if ( m_pConfig->GetConfig(info) )
        {
            bool bUnZip( true );
            NotifyInterfaceEx( UnZipFile, info, bUnZip );
            if ( bUnZip )
            {
                CString strZipFileName = GetZipFileName( info.name );
                if ( UnZipFiles(m_CurPath, strZipFileName) )
                {
                    DeleteFile( strZipFileName );
                }
            }
        }
    }
}

void CxdAppUpdate::DoRunAppByList( TStringList& AList )
{
    bool bCloseSelf(false);
    for ( TStringListIT it = AList.begin(); it != AList.end(); it++ )
    {
        if ( m_bWaitFree ) return;

        TxdUpConfigInfo info;
        info.name = (*it).c_str();
        if ( !m_pConfig->GetConfig(info) ) continue;

        CString strApp = m_CurPath + info.name + TEXT(".exe");
        if( !FileExist(strApp) ) continue;

        bool bRun( true );
        CString strParam;
        NotifyInterfaceEx3( RunApp, info, bRun, strParam );
        if ( bRun )
        {
            HINSTANCE hInstance = ShellExecute( NULL, TEXT("open"), strApp, strParam, NULL, SW_NORMAL );
            if ( hInstance > (HINSTANCE)32 )
            {
                if ( !bCloseSelf && CompareString(TEXT("true"), info.closeParent) )
                {
                    bCloseSelf = true;
                }
            }
        }
    }
    if ( bCloseSelf )
    {
        NotifyInterface( CloseSelf, this );
    }
}