#include "stdafx.h"
#include "zip.h"
#include "unzip.h"
#include "JxdZipSub.h"
#include "JxdFileSub.h"
#include "JxdStringHandle.h"

class CxdZipFiles : public CxdLoopFileHandle
{
public:
    CxdZipFiles(const TCHAR *ApSrcPath, const TCHAR *ApZipFileName, const TCHAR* ApExceptItems )
    {
        m_strSrcPath = IncludeTrailingPathDelimiter( ApSrcPath );
        m_strDestZipFileName = ApZipFileName;
        m_strDestPath = ExtractFilePath( ApZipFileName );
        m_pList = NULL;
        if ( Assigned(ApExceptItems) )
        {
            m_pList = new TStringList;
            ParseFormatString( ApExceptItems, TEXT(","), *m_pList );
        }

        m_hZip = CreateZip( m_strDestZipFileName.c_str(), NULL );
        LoopDirectiories( ApSrcPath, this, true );
        CloseZip( m_hZip );
        SafeDeleteObject( m_pList );
    }
private:
    bool DoLoopFile(TFileInfo* ApParam)
    {
        CxdString strTemp = GetTheString( ApParam->strFileName.c_str(), m_strSrcPath.c_str(), NULL, tpBack );
        bool bAdd( true );
        if ( Assigned(m_pList) )
        {
            for ( TStringListIT it = m_pList->begin(); it != m_pList->end(); it++ )
            {
                if ( CompareString(strTemp.c_str(), (*it).c_str()) )
                {
                    bAdd = false;
                    break;
                }
            }
        }

        if ( bAdd )
        {
            ZipAdd( m_hZip, strTemp.c_str(), ApParam->strFileName.c_str() );
        }
        return true;
    }
private:
    CxdString m_strSrcPath;
    CxdString m_strDestZipFileName;
    CxdString m_strDestPath;
    TStringList* m_pList;
    HZIP m_hZip;
};

bool ZipFiles( const TCHAR* ApDir, const TCHAR* ApZipFileName, const TCHAR* ApExceptItems )
{
    if ( !DirectoryExist(ApDir) ||
         !ForceDirectories(ExtractFilePath(ApZipFileName).c_str()) ) return false;

    CxdZipFiles zips( ApDir, ApZipFileName, ApExceptItems );
    return FileExist( ApZipFileName );
}

bool UnZipFiles( const TCHAR* ApDir, const TCHAR* ApZipFileName, const TCHAR* ApExceptItems /*= NULL*/ )
{
    if ( !FileExist(ApZipFileName) ) return false;

    HZIP hZip = OpenZip( ApZipFileName, 0 );
    if ( 0 == hZip ) return false;

    CString strDestPath( IncludeTrailingPathDelimiter(ApDir).c_str() );
    ForceDirectories( strDestPath );

    TStringList lt;
    if ( Assigned(ApExceptItems) )
    {
        ParseFormatString( ApExceptItems, TEXT(","), lt );
    }
    
    ZIPENTRY ze; 
    GetZipItem( hZip,-1,&ze ); 
    int nItemCount = ze.index;
    for ( int i = 0; i < nItemCount; i++ )
    {
        GetZipItem( hZip, i, &ze );
        if ( !(ze.attr & FILE_ATTRIBUTE_ARCHIVE) )
        {
            continue;
        }
        bool bUnzip( true );
        for ( TStringListIT it = lt.begin(); it != lt.end(); it++ )
        {
            if ( CompareString(ze.name, (*it).c_str()) )
            {
                bUnzip = false;
                break;
            }
        }
        if ( bUnzip )
        {
            UnzipItem( hZip, i, strDestPath + ze.name );
        }
    }
    CloseZip( hZip );
    return true;
}
