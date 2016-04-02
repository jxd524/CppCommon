// CpcHttp.cpp : 实现文件
//

#include "stdafx.h"
#include "JxdHttp.h"
#include "JxdFileSub.h"
#include "JxdMacro.h"
#include "JxdSysSub.h"
#include "JxdDataStream.h"
#include "JxdNetwork.h"
#include <algorithm>

#include "JxdDebugInfo.h"

typedef const TCHAR* const CtConst;
static CtConst CtGet = TEXT("GET");
static CtConst CtPost = TEXT("POST");
static CtConst CtHttpHeadInfo = TEXT("http://");
static CtConst CtHeadSpace = TEXT(": ");
static CtConst CtEqualSign = TEXT("=");
static CtConst CtSemicolon = TEXT(";");
static CtConst CtHttpLineEnd = TEXT("\r\n");
static CtConst CtSetCookie = TEXT("Set-Cookie");
static CtConst CtEncodingGBK = TEXT("gbk");
static CtConst CtEncodingGb2312 = TEXT("gb2312");
static CtConst CtEncodingUtf8 = TEXT("utf-8");
static CtConst CtAccept = TEXT("Accept");
static CtConst CtAcceptLanguage = TEXT("Accept-Language");
static CtConst CtAcceptEncoding = TEXT("Accept-Encoding");
static CtConst CtCookie = TEXT("Cookie");
static CtConst CtConnection = TEXT("Connection");
static CtConst CtContentType  = TEXT("Content-Type");
static CtConst CtContentEncoding = TEXT("Content-Encoding");
static CtConst CtReferer = TEXT("Referer") ;
static CtConst CtLocation = TEXT("Location");
static CtConst CtHttpName = TEXT("Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E)");
static CtConst CtContentTypeTable = 
    TEXT(".001=application/x-001|.301=application/x-301|.323=text/h323|.906=application/x-906|.907=drawing/907|.a11=application/x-a11|.acp=audio/x-mei-aac|.ai=application/postscript|.aif=audio/aiff|.aifc=audio/aiff|.aiff=audio/aiff|\
.anv=application/x-anv|.asa=text/asa|.asf=video/x-ms-asf|.asp=text/asp|.asx=video/x-ms-asf|.au=audio/basic|.avi=video/avi|.awf=application/vnd.adobe.workflow|.biz=text/xml|.bmp=application/x-bmp|.bot=application/x-bot|.c4t=application/x-c4t|.c90=application/x-c90|\
.cal=application/x-cals|.cat=application/vnd.ms-pki.seccat|.cdf=application/x-netcdf|.cdr=application/x-cdr|.cel=application/x-cel|.cer=application/x-x509-ca-cert|.cg4=application/x-g4|.cgm=application/x-cgm|.cit=application/x-cit|.class=java/*|\
.cml=text/xml|.cmp=application/x-cmp|.cmx=application/x-cmx|.cot=application/x-cot|.crl=application/pkix-crl|.crt=application/x-x509-ca-cert|.csi=application/x-csi|.css=text/css|.cut=application/x-cut|.dbf=application/x-dbf|.dbm=application/x-dbm|\
.dbx=application/x-dbx|.dcd=text/xml|.dcx=application/x-dcx|.der=application/x-x509-ca-cert|.dgn=application/x-dgn|.dib=application/x-dib|.dll=application/x-msdownload|.doc=application/msword|.dot=application/msword|.drw=application/x-drw|\
.dtd=text/xml|.dwf=Model/vnd.dwf|.dwf=application/x-dwf|.dwg=application/x-dwg|.dxb=application/x-dxb|.dxf=application/x-dxf|.edn=application/vnd.adobe.edn|.emf=application/x-emf|.eml=message/rfc822|.ent=text/xml|.epi=application/x-epi|.eps=application/x-ps|\
.eps=application/postscript|.etd=application/x-ebx|.exe=application/x-msdownload|.fax=image/fax|.fdf=application/vnd.fdf|.fif=application/fractals|.fo=text/xml|.frm=application/x-frm|.g4=application/x-g4|.gbr=application/x-gbr|.gcd=application/x-gcd|\
.gif=image/gif|.gl2=application/x-gl2|.gp4=application/x-gp4|.hgl=application/x-hgl|.hmr=application/x-hmr|.hpg=application/x-hpgl|.hpl=application/x-hpl|.hqx=application/mac-binhex40|.hrf=application/x-hrf|.hta=application/hta|.htc=text/x-component|\
.htm=text/html|.html=text/html|.htt=text/webviewhtml|.htx=text/html|.icb=application/x-icb|.ico=image/x-icon|.ico=application/x-ico|.iff=application/x-iff|.ig4=application/x-g4|.igs=application/x-igs|.iii=application/x-iphone|.img=application/x-img|\
.ins=application/x-internet-signup|.isp=application/x-internet-signup|.IVF=video/x-ivf|.java=java/*|.jfif=image/jpeg|.jpe=image/jpeg|.jpe=application/x-jpe|.jpeg=image/jpeg|.jpg=image/jpeg|.jpg=application/x-jpg|.js=application/x-javascript|.jsp=text/html|\
.la1=audio/x-liquid-file|.lar=application/x-laplayer-reg|.latex=application/x-latex|.lavs=audio/x-liquid-secure|.lbm=application/x-lbm|.lmsff=audio/x-la-lms|.ls=application/x-javascript|.ltr=application/x-ltr|.m1v=video/x-mpeg|.m2v=video/x-mpeg|.m3u=audio/mpegurl|\
.m4e=video/mpeg4|.mac=application/x-mac|.man=application/x-troff-man|.math=text/xml|.mdb=application/msaccess|.mdb=application/x-mdb|.mfp=application/x-shockwave-flash|.mht=message/rfc822|.mhtml=message/rfc822|.mi=application/x-mi|.mid=audio/mid|.midi=audio/mid|\
.mil=application/x-mil|.mml=text/xml|.mnd=audio/x-musicnet-download|.mns=audio/x-musicnet-stream|.mocha=application/x-javascript|.movie=video/x-sgi-movie|.mp1=audio/mp1|.mp2=audio/mp2|.mp2v=video/mpeg|.mp3=audio/mp3|.mp4=video/mpeg4|.mpa=video/x-mpg|.mpd=application/vnd.ms-project|\
.mpe=video/x-mpeg|.mpeg=video/mpg|.mpg=video/mpg|.mpga=audio/rn-mpeg|.mpp=application/vnd.ms-project|.mps=video/x-mpeg|.mpt=application/vnd.ms-project|.mpv=video/mpg|.mpv2=video/mpeg|.mpw=application/vnd.ms-project|.mpx=application/vnd.ms-project|.mtx=text/xml|.mxp=application/x-mmxp|\
.net=image/pnetvue|.nrf=application/x-nrf|.nws=message/rfc822|.odc=text/x-ms-odc|.out=application/x-out|.p10=application/pkcs10|.p12=application/x-pkcs12|.p7b=application/x-pkcs7-certificates|.p7c=application/pkcs7-mime|.p7m=application/pkcs7-mime|.p7r=application/x-pkcs7-certreqresp|\
.p7s=application/pkcs7-signature|.pc5=application/x-pc5|.pci=application/x-pci|.pcl=application/x-pcl|.pcx=application/x-pcx|.pdf=application/pdf|.pdx=application/vnd.adobe.pdx|.pfx=application/x-pkcs12|.pgl=application/x-pgl|.pic=application/x-pic|\
.pko=application/vnd.ms-pki.pko|.pl=application/x-perl|.plg=text/html|.pls=audio/scpls|.plt=application/x-plt|.png=image/x-png|.pot=application/vnd.ms-powerpoint|.ppa=application/vnd.ms-powerpoint|.ppm=application/x-ppm|.pps=application/vnd.ms-powerpoint|.ppt=application/vnd.ms-powerpoint|\
.ppt=application/x-ppt|.pr=application/x-pr|.prf=application/pics-rules|.prn=application/x-prn|.prt=application/x-prt|.ps=application/x-ps|.ps=application/postscript|.ptn=application/x-ptn|.pwz=application/vnd.ms-powerpoint|.r3t=text/vnd.rn-realtext3d|.ra=audio/vnd.rn-realaudio|\
.ram=audio/x-pn-realaudio|.ras=application/x-ras|.rat=application/rat-file|.rdf=text/xml|.rec=application/vnd.rn-recording|.red=application/x-red|.rgb=application/x-rgb|.rjs=application/vnd.rn-realsystem-rjs|.rjt=application/vnd.rn-realsystem-rjt|.rlc=application/x-rlc|.rle=application/x-rle|\
.rm=application/vnd.rn-realmedia|.rmf=application/vnd.adobe.rmf|.rmi=audio/mid|.rmj=application/vnd.rn-realsystem-rmj|.rmm=audio/x-pn-realaudio|.rmp=application/vnd.rn-rn_music_package|.rms=application/vnd.rn-realmedia-secure|.rmvb=application/vnd.rn-realmedia-vbr|.rmx=application/vnd.rn-realsystem-rmx|\
.rnx=application/vnd.rn-realplayer|.rp=image/vnd.rn-realpix|.rpm=audio/x-pn-realaudio-plugin|.rsml=application/vnd.rn-rsml|.rt=text/vnd.rn-realtext|.rtf=application/msword|.rtf=application/x-rtf|.rv=video/vnd.rn-realvideo|.sam=application/x-sam|.sat=application/x-sat|.sdp=application/sdp|.sdw=application/x-sdw|\
.sit=application/x-stuffit|.slb=application/x-slb|.sld=application/x-sld|.slk=drawing/x-slk|.smi=application/smil|.smil=application/smil|.smk=application/x-smk|.snd=audio/basic|.sol=text/plain|.sor=text/plain|.spc=application/x-pkcs7-certificates|.spl=application/futuresplash|.spp=text/xml|.ssm=application/streamingmedia|\
.sst=application/vnd.ms-pki.certstore|.stl=application/vnd.ms-pki.stl|.stm=text/html|.sty=application/x-sty|.svg=text/xml|.swf=application/x-shockwave-flash|.tdf=application/x-tdf|.tg4=application/x-tg4|.tga=application/x-tga|.tif=image/tiff|.tif=application/x-tif|.tiff=image/tiff|.tld=text/xml|.top=drawing/x-top|\
.torrent=application/x-bittorrent|.tsd=text/xml|.txt=text/plain|.uin=application/x-icq|.uls=text/iuls|.vcf=text/x-vcard|.vda=application/x-vda|.vdx=application/vnd.visio|.vml=text/xml|.vpg=application/x-vpeg005|.vsd=application/vnd.visio|.vsd=application/x-vsd|.vss=application/vnd.visio|.vst=application/vnd.visio|\
.vst=application/x-vst|.vsw=application/vnd.visio|.vsx=application/vnd.visio|.vtx=application/vnd.visio|.vxml=text/xml|.wav=audio/wav|.wax=audio/x-ms-wax|.wb1=application/x-wb1|.wb2=application/x-wb2|.wb3=application/x-wb3|.wbmp=image/vnd.wap.wbmp|.wiz=application/msword|.wk3=application/x-wk3|\
.wk4=application/x-wk4|.wkq=application/x-wkq|.wks=application/x-wks|.wm=video/x-ms-wm|.wma=audio/x-ms-wma|.wmd=application/x-ms-wmd|.wmf=application/x-wmf|.wml=text/vnd.wap.wml|.wmv=video/x-ms-wmv|.wmx=video/x-ms-wmx|.wmz=application/x-ms-wmz|.wp6=application/x-wp6|.wpd=application/x-wpd|\
.wpg=application/x-wpg|.wpl=application/vnd.ms-wpl|.wq1=application/x-wq1|.wr1=application/x-wr1|.wri=application/x-wri|.wrk=application/x-wrk|.ws=application/x-ws|.ws2=application/x-ws|.wsc=text/scriptlet|.wsdl=text/xml|.wvx=video/x-ms-wvx|.xdp=application/vnd.adobe.xdp|.xdr=text/xml|.xfd=application/vnd.adobe.xfd|\
.xfdf=application/vnd.adobe.xfdf|.xhtml=text/html|.xls=application/vnd.ms-excel|.xls=application/x-xls|.xlw=application/x-xlw|.xml=text/xml|.xpl=audio/scpls|.xq=text/xml|.xql=text/xml|.xquery=text/xml|.xsd=text/xml|.xsl=text/xml|.xslt=text/xml|.xwd=application/x-xwd|.x_b=application/x-x_b|\
.x_t=application/x-x_t|");

static const int CtReadBufferSize = 1024 * 8;
static const int CtHttpHeadInfoLength = _tcslen(CtHttpHeadInfo);
static const DWORD CtMaxSendBufferSize = 1024 * 64;


CString GetWebContenType( CString AFileExt )
{
    if ( AFileExt.IsEmpty() )
    {
        return TEXT("application/octet-stream");
    }

    AFileExt.MakeLower();
    if ( AFileExt[0] != '.' )
    {
        AFileExt.Insert( 0, '.' );
    }
	wstring strResult;
	GetTheString( (CString)CtContentTypeTable, AFileExt + TEXT("="), (CString)TEXT("|"), strResult );
    return strResult.c_str();
}

CString CreateUpFileHead( const CString &AFileName, const CString &AHeadName, const CString &ABoundary )
{
    static const CString CtUpFileHeadInfo = TEXT("--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: %s\r\n\r\n");
    CString strResult;
    CString strType( ExtractFileExt(AFileName).c_str() );
    strResult.Format( CtUpFileHeadInfo, ABoundary, AHeadName, AFileName, 
        GetWebContenType(strType) );
    return strResult;
}

CString CreateUpFileEnd( const CString& ABoundary )
{
    static const CString CtUpFileHeadEnd = TEXT("\r\n--%s--\r\n");
    CString strResult;
    strResult.Format( CtUpFileHeadEnd, ABoundary );
    return strResult;
}

CString StreamToString( THttpEncode he, CxdMemoryHandle *ApStream )
{
    CString strResult;
    switch (he)
    {
    case heUtf8:
    case heUnknow:
        {
#ifdef UNICODE
            strResult = Utf8ToUnicode( ApStream->Memory() ).c_str();
#else
            strResult = UnicodeToAscii( Utf8ToUnicode( ApStream->Memory() ).c_str() ).c_str();
#endif
            break;
        }

    default: //default 
        {
#ifdef UNICODE
            strResult = Utf8ToUnicode( ApStream->Memory() ).c_str();
#else
            strResult = UnicodeToAscii( Utf8ToUnicode( ApStream->Memory() ).c_str() ).c_str();
#endif
            break;
        }
    }
    return strResult;
}

//CxdHttp
CxdHttp::CxdHttp( const CString &AUserAgent )
{
    m_strUserAgent = AUserAgent;
    if ( m_strUserAgent.IsEmpty() )
    {
        m_strUserAgent = TEXT("Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E)");
    }
    m_dwTimeoutConnect = 10 * 1000;
    m_dwTimeoutRecv = 10 * 1000;
    m_dwTimeoutSend = 10 * 1000;
    m_bAutoRedirect = false;
	m_bIsUseProxy = false;

    SetRawHead( CtAccept, TEXT("application/x-ms-application, image/jpeg, application/xaml+xml, image/gif, image/pjpeg, application/x-ms-xbap, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*") );
    SetRawHead( CtAcceptLanguage, TEXT("zh-CN") );
    SetRawHead( CtConnection, TEXT("Keep-Alive") );
    SetRawHead( CtContentType, TEXT("application/x-www-form-urlencoded") );
    SetRawHead( CtAcceptEncoding, TEXT("utf-8") ); //gzip,deflate,sdch
}

CxdHttp::~CxdHttp()
{
}

bool CxdHttp::DoRequestCmd( THttpMethod AMethod, const CString &AURL, CxdStreamBasic *ApResponseStream, CxdMemoryHandle *ApRequestStream )
{
    CString strMethod;
    emGet == AMethod ? strMethod = CtGet : strMethod = CtPost;

    TURLInfo info;
    if ( !UrlParse(AURL, info, 0) ) 
    {
        TRACE( TEXT("invaild url!\r\n") );
        return false;
    }
    HINTERNET hHttp;
	if ( m_bIsUseProxy && !m_strProxyAddress.IsEmpty() )
	{
		hHttp = InternetOpen( m_strUserAgent, INTERNET_OPEN_TYPE_PROXY, m_strProxyAddress, NULL, 0 );
	}
	else
	{
		hHttp = InternetOpen( m_strUserAgent, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );
	}
    if ( NULL == hHttp )
    {
        TRACE( TEXT("InternetOpen error!!!") );
        return false;
    }

    m_LastRequestMethod = AMethod;
    m_strLastURL = AURL;

    SetOpt( hHttp, INTERNET_OPTION_CONNECT_TIMEOUT, m_dwTimeoutConnect );
    SetOpt( hHttp, INTERNET_OPTION_RECEIVE_TIMEOUT, m_dwTimeoutRecv );
    SetOpt( hHttp, INTERNET_OPTION_SEND_TIMEOUT,  m_dwTimeoutSend );

    //
    HINTERNET hConn = InternetConnect(hHttp, info.FHost, info.FPort, 
        info.FUserName.IsEmpty() ? NULL : info.FUserName,
        info.FPassword.IsEmpty() ? NULL : info.FPassword, INTERNET_SERVICE_HTTP, 0, 0 );
    if ( NULL == hConn )
    {
        TRACE( TEXT("InternetConnect Failed! \r\n") );
        InternetCloseHandle( hHttp );
        return false;
    }

    //open
    DWORD dwFlags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_COOKIES;
    if ( !m_bAutoRedirect )
    {
        dwFlags |= INTERNET_FLAG_NO_AUTO_REDIRECT;
    }
    
    HINTERNET hOpen = HttpOpenRequest(hConn, strMethod, info.FUrlPath + info.FExtraInfo, TEXT("HTTP/1.1"), NULL, NULL, dwFlags, 0 );
    if ( NULL == hOpen )
    {
        TRACE( TEXT("HttpOpenRequest Failed!\r\n") );
        InternetCloseHandle( hHttp );
        return false;
    }

    //build raw head info
    for ( TParseInfoWListIT it = m_ltRequestRawHeader.begin(); it != m_ltRequestRawHeader.end(); it++ )
    {
        TxdParseInfoW &head = *it;
		CString strTemp = head.Second.c_str();
        if ( strTemp.IsEmpty() )
        {
            continue;
        }
        CString strHead = head.First.c_str();
		strHead += CtHeadSpace;
		strHead += head.Second.c_str();
        HttpAddRequestHeaders( hOpen, strHead, strHead.GetLength(), HTTP_ADDREQ_FLAG_ADD );
    }
    CString strCookies = m_CookieManage.GetCookies( AURL );
    if ( !strCookies.IsEmpty() )
    {
        strCookies = CString(CtCookie) + CtHeadSpace + strCookies;
        HttpAddRequestHeaders( hOpen, strCookies, strCookies.GetLength(), HTTP_ADDREQ_FLAG_ADD );
    }

    //begin send
    DWORD dwSize = (ApRequestStream != NULL) ? (DWORD)ApRequestStream->GetSize() : 0, 
          dwSendSize(0), 
          dwPos(0),
          dwCurSend;
    INTERNET_BUFFERS BufferIn = {0};
    BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS );
    BufferIn.dwBufferTotal = dwSize;

    //CString strINFO;
    //ReadHttpRawHeader( hOpen, strINFO, false );
    HttpSendRequestEx( hOpen, &BufferIn, NULL, 0, 0 );
    while ( dwSize > 0 )
    {
        dwCurSend = (DWORD)ApRequestStream->GetSize() - dwPos;
        dwCurSend = dwCurSend > CtMaxSendBufferSize ? CtMaxSendBufferSize : dwCurSend;
        char *pBuf = ApRequestStream->Memory() + dwPos;
        InternetWriteFile( hOpen, pBuf, dwCurSend, &dwSendSize);
        if ( 0 == dwSendSize )
        {
            break;
        }
        dwPos += dwSendSize;
        dwSize -= dwSendSize;
    }
    HttpEndRequest( hOpen, NULL, 0, 0 );
    //Finished send!
    

    //Read response data
    if ( ApResponseStream != NULL )
    {
        DWORD nLen;
        char buf[CtMaxSendBufferSize];
        while( InternetReadFile(hOpen, buf, CtMaxSendBufferSize, &nLen) )
        {
            if ( nLen == 0 )
            {
                break;
            }
            ApResponseStream->WriteLong( buf, nLen );
        }
    }
    //Read raw header!!
    ReadHttpRawHeader( hOpen, m_strLastRecvRawHeaders );
    
    //Close handle
    InternetCloseHandle( hOpen );
    InternetCloseHandle( hConn );
    InternetCloseHandle( hHttp );

    //Parse headers
    HandleRawHeaders();

    return true;
}

void CxdHttp::SetOpt( HINTERNET h, DWORD optLeave, DWORD optValue )
{
    if ( !InternetSetOption(h, optLeave, &optValue, sizeof(DWORD)) )
    {
        TRACE( "InternetSetOption False \r\n" );
    }
}

void CxdHttp::ReadHttpRawHeader( HINTERNET hHttp, CString &ARawText, bool AbResponse )
{    
    LPVOID lpOutBuffer=NULL;
    DWORD dwSize = 0;
retry:
    if( !HttpQueryInfo(hHttp, AbResponse ? HTTP_QUERY_RAW_HEADERS_CRLF : HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS, (LPVOID)lpOutBuffer,&dwSize,NULL) )
    { 
        if (GetLastError()==ERROR_HTTP_HEADER_NOT_FOUND)
        {
            return;
        }        
        else
        {
            if (GetLastError()==ERROR_INSUFFICIENT_BUFFER)
            {
                lpOutBuffer = new char[dwSize];
                goto retry;                
            }        
            else
            {
                if (lpOutBuffer)
                {
                    delete [] lpOutBuffer;
                }
                return;
            }        
        }        
    }    
    if (lpOutBuffer)
    {
#ifdef UNICODE
        ARawText = (wchar_t*)lpOutBuffer;
#else
        ARawText = (char*)lpOutBuffer;
#endif
        delete [] lpOutBuffer;
    }
    return;
}

void CxdHttp::SetHeadAccept( const CString &AText )
{
    SetRawHead( CtAccept, AText );
}

bool CxdHttp::FindRawHead( const CString &AHeadName, TxdParseInfoW &AInfo )
{
    for ( TParseInfoWListIT it = m_ltRequestRawHeader.begin(); it != m_ltRequestRawHeader.end(); it++ )
    {
        TxdParseInfoW &f = *it;
		CString strTemp = f.First.c_str();
        if ( strTemp.CompareNoCase(AHeadName) == 0 )
        {
            AInfo = f;
            return true;
        }
    }
    return false;
}

CString CxdHttp::GetRawHead( const CString &AName )
{
    if ( AName.CompareNoCase(CtCookie) == 0 )
    {
        return m_CookieManage.GetCookies();
    }
    TxdParseInfoW info;
    FindRawHead( AName, info );
    return info.Second.c_str();
}

CString CxdHttp::GetHeadAccept( void )
{
    return GetRawHead( CtAccept );
}

void CxdHttp::SetRawHead( const CString &AName, const CString &AValue )
{
    if ( AName.CompareNoCase(CtCookie) == 0 )
    {
        return;
    }
    for ( TParseInfoWListIT it = m_ltRequestRawHeader.begin(); it != m_ltRequestRawHeader.end(); it++ )
    {
        TxdParseInfoW &f = *it;
		CString strT = f.First.c_str();
        if ( strT.CompareNoCase(AName) == 0 )
        {
            f.Second = AValue;
            return;
        }
    }
    //No find
    TxdParseInfoW info;
    info.First = AName;
    info.Second = AValue;
    m_ltRequestRawHeader.push_back(info);
}

void CxdHttp::HandleRawHeaders( void )
{
    TStringWList lt;
    TxdParseInfoW info;
    m_strContentEncoding.Empty();
    m_strContentType.Empty();
    m_strLocation.Empty();

    m_CookieManage.AddHttpHeader( m_strLastRecvRawHeaders, m_strLastURL );
	wstring strRID;
	GetTheString( m_strLastRecvRawHeaders, TEXT(" "), TEXT(" "), strRID );
    m_nResponseID = _ttoi( strRID.c_str() );
    wstring strEncoding, strType, strLocation;
	GetTheString( m_strLastRecvRawHeaders, CString(CtContentEncoding) + CtHeadSpace, CtHttpLineEnd, strEncoding );
	GetTheString( m_strLastRecvRawHeaders, CString(CtContentType) + CtHeadSpace, CtHttpLineEnd, strType );
    GetTheString( m_strLastRecvRawHeaders, CString(CtLocation) + CtHeadSpace, CtHttpLineEnd, strLocation );
	m_strContentEncoding = strEncoding.c_str();
	m_strContentType = strType.c_str();
	m_strLocation = strLocation.c_str();
}

THttpEncode CxdHttp::CheckContentType( const CString &AContentType )
{
    int nPos = AContentType.Find( CtEqualSign );
    if ( -1 == nPos )
    {
        return heUnknow;
    }
    CString str = AContentType.Mid( nPos + 1 );
    str.Trim();
    if ( 0 == str.CompareNoCase(CtEncodingUtf8) )
    {
        return heUtf8;
    }
    if ( 0 == str.CompareNoCase(CtEncodingGBK) )
    {
        return heGBK;
    }
    if ( 0 == str.CompareNoCase(CtEncodingGb2312) )
    {
        return heGb2313;
    }
    return heUnknow;
}

CString CxdHttp::StreamToStr( CxdMemoryHandle *ApStream )
{
    CString strResult;
    THttpEncode he = ResponseType();
    switch (he)
    {
    case heUtf8:
    case heUnknow:
        {
#ifdef UNICODE
            strResult = Utf8ToUnicode( ApStream->Memory() ).c_str();
#else
            strResult = UnicodeToAscii( Utf8ToUnicode( ApStream->Memory() ).c_str() ).c_str();
#endif
            break;
        }

    default: //default 
        {
#ifdef UNICODE
            strResult = AsciiToUnicode( ApStream->Memory() ).c_str();
#else
            strResult = ApStream->Memory();
#endif
            break;
        }
    }
    return strResult;
}

CString CxdHttp::GetHeadReferer( void )
{
    return GetRawHead( CtReferer );
}

void CxdHttp::SetHeadReferer( const CString &AReferer )
{
    SetRawHead( CtReferer, AReferer );
}

CStringA CxdHttp::Get( const CString &AURL, CxdStreamBasic *ApRawRecvStream )
{
    CxdStreamBasic *pStream = (NULL == ApRawRecvStream) ? new CxdDynamicMemory(CtMaxSendBufferSize) : ApRawRecvStream;
    CStringA strResult;
    if ( DoRequestCmd(emGet, AURL, pStream, NULL) )
    {
        if ( ApRawRecvStream == NULL )
        {
            strResult = ((CxdMemoryHandle*)pStream)->Memory();//StreamToString( ResponseType(), (CxdMemoryHandle*)pStream );
        }        
    }
    (NULL == ApRawRecvStream) ? delete pStream : pStream = NULL;
    return strResult;
}

CStringA CxdHttp::Post( const CString &AURL, CxdMemoryHandle *ApPostData, CxdStreamBasic *ApRawRecvStream )
{
    CxdStreamBasic *pStream = (NULL == ApRawRecvStream) ? new CxdDynamicMemory(CtMaxSendBufferSize) : ApRawRecvStream;
    CStringA strResult;
    if ( DoRequestCmd(emPost, AURL, pStream, ApPostData) )
    {
        if ( ApRawRecvStream == NULL )
        {
            strResult = ((CxdMemoryHandle*)pStream)->Memory();//StreamToString( ResponseType(), (CxdMemoryHandle*)pStream );
        }
    }
    (NULL == ApRawRecvStream) ? delete pStream : pStream = NULL;
    return strResult;
}

CStringA CxdHttp::Post( const CString &AURL, const char *ApPostData, const int &Alen )
{
    CxdOuterMemory stream;
    stream.InitMemory( (char*)ApPostData, Alen );
    return Post( AURL, &stream );
}

CStringA CxdHttp::Post( const CString &AURL, const CString &APostData )
{
    string buf;
#ifdef UNICODE
    buf = UnicodeToAscii(APostData);
#else
    buf = APostData;
#endif
    return Post( AURL, buf.c_str(), buf.length() );
}

CStringA CxdHttp::UploadFile( const CString &AURL, const CString &AFileName, const CString &AHeadName /*= TEXT("picture") */ )
{
    if ( !FileExist(AFileName) )
    {
        return "";
    }
    CFile upFile;
    if ( !upFile.Open(AFileName, CFile::modeRead | CFile::shareDenyWrite) ) 
    {
        return "";
    }

    CString strOnlyFileName = ExtractFileName( AFileName ).c_str();
    CString strBoundary = TEXT("-----------------------------");
	strBoundary += GetRandomStringW(12, 13).c_str();
    CString strFileHead = CreateUpFileHead( strOnlyFileName, AHeadName, strBoundary );
    CString strFileEnd = CreateUpFileEnd( strBoundary );
    CString strOldContentType = GetRawHead( CtContentType );
    SetRawHead( CtContentType, TEXT("multipart/form-data; boundary=") + strBoundary );
    
    CStringA strHead = UnicodeToAscii( strFileHead ).c_str();
    CStringA strEnd = UnicodeToAscii( strFileEnd ).c_str();

    CxdDynamicMemory SendStream( (int)upFile.GetLength() + strHead.GetLength() + strEnd.GetLength() );

    SendStream.WriteLong( strHead.GetBuffer(), strHead.GetLength() );

    char buf[CtMaxSendBufferSize];
    int nlen;
    upFile.SeekToBegin( );
    while( nlen = upFile.Read(buf , CtMaxSendBufferSize) )
    {
        SendStream.WriteLong( buf, nlen );
    }

    SendStream.WriteLong( strEnd.GetBuffer(), strEnd.GetLength() );

    CStringA str = Post( AURL, &SendStream );
    SetRawHead( CtContentType, strOldContentType );
    return str;
}

void CxdHttp::SetContentType( const CString &AContentType )
{
	m_strContentType = AContentType;
	SetRawHead( CtContentType, AContentType );
}

//CxdCookieManage
CxdCookieManage::CxdCookieManage()
{

}

CxdCookieManage::~CxdCookieManage()
{
    Clear();
}

bool CxdCookieManage::AddCookie( CString ACookie, const TCHAR *ApDomainName )
{
    ACookie.Trim();
    ACookie += TEXT(";");
    int nPos1 = Pos(ACookie, TEXT("="), 0 );
    if ( -1 == nPos1 )
    {
        return false;
    }
    int nPos2 = Pos(ACookie, TEXT(";"), nPos1 );
    if ( -1 == nPos2 )
    {
        return false;
    }
    CString strName = ACookie.Mid( 0, nPos1++ );
    CString strValue = ACookie.Mid( nPos1, nPos2 - nPos1 );
    wstring strDomain;
    GetTheString( ACookie, TEXT("domain="), TEXT(";"), strDomain );
    if ( strDomain.empty() && Assigned(ApDomainName) )
    {
        strDomain = ApDomainName;
    }
    strDomain = CheckDomainName( strDomain.c_str() );

    if ( 0 == strValue.CompareNoCase(TEXT("deleted")) )
    {
        DeleteCookie( strName, ApDomainName );
        return true;
    }
    TCookieInfo *pInfo = FindCookie( strName, strDomain.c_str(), true, false );
    pInfo->FCookieValue = strValue;
	wstring strExpires, strPath;
    GetTheString( ACookie, TEXT("expires="), TEXT(";"), strExpires );
    GetTheString( ACookie, TEXT("path="), TEXT(";"), strPath );
	pInfo->FExpires = strExpires.c_str();
	pInfo->FPath = strPath.c_str();

    return true;
}

TCookieInfo* CxdCookieManage::FindCookie( const CString &ACookieName, const CString &ADomainName, bool bCreateOnNull, bool bDelete )
{
    CString strDomain = CheckDomainName( ADomainName );

    //精确搜索
    for ( TCookieInfoListIT it = m_ltCookies.begin(); it != m_ltCookies.end(); it++ )
    {
        TCookieInfo *pInfo = *it;
        if ( 0 == pInfo->FDomain.CompareNoCase(strDomain) )
        {
            if ( 0 == pInfo->FCookieName.CompareNoCase(ACookieName) )
            {
                if ( bDelete )
                {
                    m_ltCookies.erase( it );
                }
                return pInfo;
            }
        }
    }

    //模糊搜索
    for ( TCookieInfoListIT it = m_ltCookies.begin(); it != m_ltCookies.end(); it++ )
    {
        TCookieInfo *pInfo = *it;
        if ( Pos(ADomainName, pInfo->FDomain, 0) > 0 )
        {
            if ( 0 == pInfo->FCookieName.CompareNoCase(ACookieName) )
            {
                if ( bDelete )
                {
                    m_ltCookies.erase( it );
                }
                return pInfo;
            }
        }
    }

    //创建新项
    if ( bCreateOnNull )
    {
        TCookieInfo *pInfo = new TCookieInfo;
        pInfo->FCookieName = ACookieName;
        pInfo->FDomain = ADomainName;
        m_ltCookies.push_back( pInfo );
        return pInfo;
    }
    return NULL;
}

void CxdCookieManage::Clear( void )
{
    for ( TCookieInfoListIT it = m_ltCookies.begin(); it != m_ltCookies.end(); it++ )
    {
        TCookieInfo* pInfo = *it;
        delete pInfo;
    }
    m_ltCookies.clear();
}

void CxdCookieManage::DeleteCookie( const TCHAR *ApCookieName, const TCHAR *ApDomainName )
{
    TCookieInfo *pInfo = FindCookie( ApCookieName, ApDomainName, false, true );
    SafeDeleteObject( pInfo );
}

void CxdCookieManage::AddHttpHeader( CString AHttpHead, const CString &AURL )
{
    AHttpHead += TEXT("\r\n");
    int nPos1(0), nPo2(0), nBegin(0);
	wstring strCookie;
    while( true )
    {
		
        nBegin = GetTheString( AHttpHead, TEXT("Set-Cookie:"), TEXT("\r\n"), strCookie, tpBack, nBegin );
        if ( strCookie.empty() || nBegin == -1 ) break;

		AddCookie( strCookie.c_str(), AURL );
    }
}

CString CxdCookieManage::GetSysCookies( const TCHAR *ApURL, bool IsHttponly/* = false*/ )
{
	static bool bIsIE6= IsIE6OrIE7Version();

	CString strURL( ApURL );
	CheckURL( strURL );
	TURLInfo urlInfo;
	if ( !UrlParse(strURL, urlInfo, 0) )
	{
		return TEXT("");
	}
	DWORD dwLen = 0;
	strURL = urlInfo.FUrlHead + TEXT("://") + urlInfo.FHost;

	CString strResult;
	if ( bIsIE6 && IsHttponly )
	{
		class CMyLoopFile: public CxdLoopFileHandle
		{
		public:
			CMyLoopFile(const TCHAR *ApDomain)
			{
				m_strDomain = ApDomain;
				LoopDirectiories( GetIE6SysCookiePath(), this, true );
			}
			CString GetIE6Cookies(void) { return m_strCookies; }
		private:
			CString m_strCookies;
			CString m_strDomain;
            bool DoLoopFile(TFileInfo* ApParam)
			{
				CString strFileExt = ExtractFileExt( ApParam->strFileName.c_str() ).c_str();
				if ( 0 != strFileExt.CompareNoCase(TEXT("txt")) ) return true;

                CxdFileStream file( ApParam->strFileName.c_str() );
                INT64 nLen = file.GetFileSize();
                char *pBuf = new char[(unsigned int)nLen + 1];
                file.FileRead( (void *)pBuf, (DWORD)nLen );
                pBuf[nLen] = '\0';
                CString strFileInfo = AsciiToUnicode(pBuf).c_str();
                delete []pBuf;

                if ( -1 == Pos(strFileInfo, m_strDomain, 0) ) return true;

                TStringWList lt;
                ParseFormatString( strFileInfo, TEXT("*\n"), lt );
                for ( TStringWListIT it = lt.begin(); it != lt.end(); it++ )
                {
                    CString strTemp = (*it).c_str();
                    strTemp.Trim();
                    if ( strTemp.IsEmpty() ) continue;

                    TStringWList ltSub;
                    ParseFormatString( strTemp, TEXT("\n"), ltSub );
                    TStringWListIT itSub = ltSub.begin();
                    if ( itSub != ltSub.end() )
                    {
                        m_strCookies += (*itSub).c_str();
                        m_strCookies += TEXT("=");
                    }
                    itSub = ltSub.begin() + 1;
                    if ( itSub != ltSub.end() )
                    {
                        m_strCookies += (*itSub).c_str();
                        m_strCookies += TEXT("; ");
                    }
                }
                return true;
			}
		};
		CMyLoopFile LoopFile(urlInfo.FHost);
		strResult += LoopFile.GetIE6Cookies();
	}
	else
	{
		if ( !InternetGetCookieEx( strURL, NULL, NULL, &dwLen, INTERNET_COOKIE_HTTPONLY, 0) )
		{
			return TEXT("");
		}
		TCHAR *pBuf = new TCHAR[dwLen + 1];
		ZeroMemory( pBuf, dwLen + 1 );
		InternetGetCookieEx( strURL, NULL, pBuf, &dwLen, INTERNET_COOKIE_HTTPONLY, 0 );
		strResult += pBuf;
		delete []pBuf;
	}
    return strResult;
}

void CxdCookieManage::AddSysCookies( const TCHAR *ApURL )
{
    CString strURL( ApURL );
    CheckURL( strURL );
    TURLInfo urlInfo;
    if ( !UrlParse(strURL, urlInfo, 0) )
    {
        return;
    }
    DWORD dwLen = 0;
    strURL = urlInfo.FUrlHead + TEXT("://") + urlInfo.FHost;
    if ( !InternetGetCookieEx( strURL, NULL, NULL, &dwLen, INTERNET_COOKIE_HTTPONLY, 0) )
    {
        return;
    }
    TCHAR *pBuf = new TCHAR[dwLen + 1];
    ZeroMemory( pBuf, dwLen + 1 );
    InternetGetCookieEx( strURL, NULL, pBuf, &dwLen, INTERNET_COOKIE_HTTPONLY, 0 );

    TStringWList lt;
    CString strTemp;
    TxdParseInfoW tInfo;
    ParseFormatString( pBuf, TEXT(";"), lt );
    delete []pBuf;

    for ( TStringWListIT it = lt.begin(); it != lt.end(); it++ )
    {
        strTemp = (*it).c_str();
        if ( ParseFormatString(strTemp, TEXT("="), tInfo) )
        {
            TCookieInfo *pInfo = FindCookie( tInfo.First.c_str(), urlInfo.FHost, true, false );
            pInfo->FCookieValue = tInfo.Second.c_str();
        }
    }
}

void CxdCookieManage::SynToSystem( const TCHAR *ApDefaultDomain )
{
    for ( TCookieInfoListIT it = m_ltCookies.begin(); it != m_ltCookies.end(); it++ )
    {
        TCookieInfo *pInfo = *it;
        if ( !pInfo->FExpires.IsEmpty())
        {
            CString strValue( pInfo->FCookieName + TEXT("=") + pInfo->FCookieValue + TEXT(";expires=") + pInfo->FExpires );
            CString strURL = pInfo->FDomain;
            if ( strURL.IsEmpty() )
            {
                strURL = ApDefaultDomain;
            }
            CheckURL( strURL );
            InternetSetCookie( strURL, NULL, strValue );
        }
    }
}

CString CxdCookieManage::GetCookies( const TCHAR *ApURL /*= NULL*/ )
{
    CString strResult;
    IxdLoopCookieInfo *pIt = NewInterfaceExByClass(CxdCookieManage, TCookieInfo*, LPVOID, DoBuildCookies );
    LoopFindCookieInfo( ApURL, pIt, (LPVOID)&strResult );
    delete pIt;
    return strResult;
}

void CxdCookieManage::ClearSysCookies( const TCHAR *ApURL /*= NULL*/ )
{
    IxdLoopCookieInfo *pIt = NewInterfaceExByClass(CxdCookieManage, TCookieInfo*, LPVOID, DoClearCookies );
    LoopFindCookieInfo( ApURL, pIt, NULL );
    delete pIt;
}

void CxdCookieManage::LoopFindCookieInfo( const TCHAR *ApURL, IxdLoopCookieInfo* ApIt, LPVOID ApParam )
{
    CString strDomain = CheckDomainName( ApURL );
    bool bNotify;
    CString strResult;
    for ( TCookieInfoListIT it = m_ltCookies.begin(); it != m_ltCookies.end(); it++ )
    {
        TCookieInfo *pInfo = *it;
        bNotify = true;
        
        if ( !pInfo->FDomain.IsEmpty() && !strDomain.IsEmpty() )
        {
            if ( Pos(strDomain, pInfo->FDomain, 0) == -1 )
            {
                bNotify = false;
            }
        }
        if ( bNotify )
        {
            ApIt->Notify( pInfo, ApParam );
        }
    }
}

void CxdCookieManage::DoBuildCookies( TCookieInfo* ApCookieInfo, LPVOID ApParam )
{
    CString *pCookies = (CString*)ApParam;
	bool bFind = false;
    if ( Assigned(ApCookieInfo) && Assigned(pCookies) )
    {
		TStringWList lt;
		ParseFormatString( *pCookies, TEXT("; "), lt );
		for ( TStringWListIT it = lt.begin(); it != lt.end(); it++ )
		{
			CString strCookie = (*it).c_str();
			TxdParseInfoW info;
			ParseFormatString( strCookie, TEXT("="), info );
			CString strTemp = info.First.c_str();
			if ( 0 != strTemp.CompareNoCase(ApCookieInfo->FCookieName) )
			{
				bFind = false;
			}
			else
			{
				bFind = true;
				break;
			}
		}

		if ( !bFind )
		{
			if ( !(*pCookies).IsEmpty() )
			{
				(*pCookies) += TEXT("; ");
			}
			(*pCookies) += ApCookieInfo->FCookieName + TEXT("=") + ApCookieInfo->FCookieValue;
		}
    }
}

void CxdCookieManage::DoClearCookies( TCookieInfo* ApCookieInfo, LPVOID ApParam )
{
    if ( Assigned(ApCookieInfo) )
    {
        CString strURL( ApCookieInfo->FDomain );
        CheckURL( strURL );
        InternetSetCookie( strURL, ApCookieInfo->FCookieName, ApCookieInfo->FCookieName + TEXT(";expires=Thu, 01-Jan-1970 00:00:01 GMT") );
    }
}

void CxdCookieManage::CheckURL( CString &AURL )
{
    if ( AURL.IsEmpty() )
    {
        return;
    }
    if ( '.' == AURL[0] )
    {
        AURL = AURL.Mid(1);
    }
    if ( AURL.GetLength() <= CtHttpHeadInfoLength )
    {
        AURL = CtHttpHeadInfo + AURL;
    }
    else
    {
        CString strTemp( AURL.Mid(0, CtHttpHeadInfoLength) );
        if ( strTemp.CompareNoCase(CtHttpHeadInfo) != 0 )
        {
            AURL = CtHttpHeadInfo + AURL;
        }
    }
}

void CxdCookieManage::AddCookies( CString ACookies, const CString &AURL )
{
    TStringWList lt;
    ParseFormatString( ACookies, CtSemicolon, lt );
    for ( TStringWListIT it = lt.begin(); it != lt.end(); it++ )
    {
        AddCookie( (*it).c_str(), AURL );
    }
}

void CxdCookieManage::GetDomainList( TStringWList &AList )
{
    bool bExists;
    for ( TCookieInfoListIT it = m_ltCookies.begin(); it != m_ltCookies.end(); it++ )
    {
    	TCookieInfo *pInfo = *it;
        bExists = false;
        for ( TStringWListIT itDomain = AList.begin(); itDomain != AList.end(); itDomain++ )
        {
        	if ( 0 == pInfo->FDomain.CompareNoCase((*itDomain).c_str()) )
        	{
                bExists = true;
                break;
        	}
        }
        if ( !bExists )
        {
            AList.push_back( (wstring)pInfo->FDomain );
        }
    }
}

CString CxdCookieManage::CheckDomainName( const TCHAR *ApURL )
{
    if ( !Assigned(ApURL) )
    {
        return TEXT("");
    }
    CString strReuslt( ApURL );
    CheckURL( strReuslt );
    TURLInfo urlInfo;
    UrlParse(strReuslt, urlInfo);
    strReuslt = urlInfo.FHost;
    return strReuslt;
}

CString CxdCookieManage::GetIE6SysCookiePath()
{
	TCHAR szPath[1024 * 8];	
	CString strPath;
	if ( SHGetSpecialFolderPath(NULL, szPath, CSIDL_COOKIES, FALSE) )
	{
		strPath = szPath;
	}
	return strPath;
}

void CxdCookieManage::AddExpiresToCookie( const TCHAR *ApExpires )
{
	if ( !Assigned(ApExpires) || _tcslen(ApExpires) <= 0 ) return;

	for ( TCookieInfoListIT it = m_ltCookies.begin(); it != m_ltCookies.end(); it++ )
	{
		TCookieInfo *pInfo = *it;
		if ( pInfo->FExpires.IsEmpty() )
		{
			pInfo->FExpires = ApExpires;
		}
	}
}

CString CxdCookieManage::GetAllCookieInfo( void )
{
	CString strInfo;
	for ( TCookieInfoListIT it = m_ltCookies.begin(); it != m_ltCookies.end(); it++ )
	{
		TCookieInfo *pInfo = *it;
		strInfo += pInfo->FCookieName + TEXT("=");
		strInfo += pInfo->FCookieValue + TEXT("; ");
		strInfo += TEXT("domain=") + pInfo->FDomain + TEXT("; ");
		if ( !pInfo->FPath.IsEmpty() )
		{
			strInfo += TEXT("path=") + pInfo->FPath + TEXT("; ");
		}
		if ( !pInfo->FExpires.IsEmpty() )
		{
			strInfo += TEXT("expires=") + pInfo->FExpires;
		}
		strInfo += TEXT("\r\n");
	}
	return strInfo;
}
