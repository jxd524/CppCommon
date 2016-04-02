#include "stdafx.h"
#include "zlib.h"
#include "JxdGzlibSub.h"

TGzipResult GzipDeCompressHttp( Byte *ApGzipBuffer, uLong AGzipBufferLen, Byte *ApOutBuffer, uLong &AInOutLen )
{
    static char dummy_head[2] =  
    { 
        0x8 + 0x7 * 0x10, 
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF, 
    }; 

    int err = 0; 
    z_stream d_stream = {0}; 
    d_stream.next_in  = ApGzipBuffer; 
    d_stream.next_out = ApOutBuffer; 
    if ( inflateInit2(&d_stream, 47) != Z_OK ) 
    {
    	return grDataError;
    } 

    while (d_stream.total_in < AGzipBufferLen && d_stream.total_out < AInOutLen ) 
    { 
        d_stream.avail_in = d_stream.avail_out = AInOutLen;
        if ( (err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END ) break; 

        if (err != Z_OK ) 
        { 
            if ( err == Z_DATA_ERROR ) 
            { 
                d_stream.next_in = (Bytef*) dummy_head; 
                d_stream.avail_in = sizeof(dummy_head); 
                if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)  
                { 
                    return grDataError; 
                } 
            } 
            else return grDataError; 
        } 
    } 
    if ( inflateEnd(&d_stream) != Z_OK ) return grDataError; 

    AInOutLen = d_stream.total_out;
    if ( d_stream.total_in < AGzipBufferLen )
    {
        return grNoEnoughMem;
    }
    return grSuccess; 
}

bool GzipCompressHttp( Byte *ApSrcBuffer, uLong ASrcBufferLen, Byte *ApOutGzipBuffer, uLong &AInOutLen )
{
    if ( !ApSrcBuffer || !ApOutGzipBuffer || ASrcBufferLen <= 0 || AInOutLen <= 0 )
    {
        return false;
    }
    z_stream c_stream = {0}; 
    int err = 0; 

    
    if ( deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,  
                      MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK )
    {
    	return false;
    }
    c_stream.next_in  = ApSrcBuffer; 
    c_stream.avail_in  = ASrcBufferLen; 
    c_stream.next_out = ApOutGzipBuffer; 
    c_stream.avail_out  = AInOutLen; 

    while ( c_stream.avail_in != 0 && c_stream.total_out < AInOutLen )  
    { 
        if ( deflate(&c_stream, Z_NO_FLUSH) != Z_OK ) 
        {
        	return false;
        }
    } 
    if ( c_stream.avail_in != 0 ) 
    {
        return false;
    }
    for (;;) 
    { 
        if ( (err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END ) break; 
        if ( err != Z_OK ) 
        {
        	return false;
        }
    } 
    if ( deflateEnd(&c_stream) != Z_OK ) 
    {
    	return false;
    }
    AInOutLen = c_stream.total_out; 
    return true; 
}
/*
下面是我的一个函数,source指向的是压缩数据,len是压缩数据的大小,dest是解压缩的存放地址,最后一个参数gzip如果不等于0的话表示压缩数据是gzip格式，否则是deflate格式; 

但是现在这个函数只能对deflate进行解压缩，对gzip进行解压缩的话就会报错Z_DATA_ERROR,不知道为什么会这样，用过zlib的人能帮帮忙吗? 

int   inflate_read(char   *source,int   len,char   **dest,int   gzip)   { 
int   ret; 
unsigned   have; 
z_stream   strm; 
unsigned   char   out[CHUNK]; 
int   totalsize   =   0; 

//   allocate   inflate   state 
strm.zalloc   =   Z_NULL; 
strm.zfree   =   Z_NULL; 
strm.opaque   =   Z_NULL; 
strm.avail_in   =   0; 
strm.next_in   =   Z_NULL; 

if(gzip) 
    ret   =   inflateInit2(&strm,   15); 
else 
    ret   =   inflateInit(&strm); 

if   (ret   !=   Z_OK) 
    return   ret; 

strm.avail_in   =   len; 
strm.next_in   =   source; 

//   run   inflate()   on   input   until   output   buffer   not   full 
do   { 
    strm.avail_out   =   CHUNK; 
    strm.next_out   =   out; 
    ret   =   inflate(&strm,   Z_NO_FLUSH); 
    assert(ret   !=   Z_STREAM_ERROR);      //   state   not   clobbered  
    switch   (ret)   { 
    case   Z_NEED_DICT: 
        ret   =   Z_DATA_ERROR;               //   and   fall   through 
    case   Z_DATA_ERROR: 
    case   Z_MEM_ERROR: 
        inflateEnd(&strm); 
        return   ret; 
    } 
    have   =   CHUNK   -   strm.avail_out; 
    totalsize   +=   have; 
    *dest   =   realloc(*dest,totalsize); 
    memcpy(*dest   +   totalsize   -   have,out,have); 
}   while   (strm.avail_out   ==   0); 

// clean   up   and   return  
(void)inflateEnd(&strm); 
return   ret   ==   Z_STREAM_END   ?   Z_OK   :   Z_DATA_ERROR; 
} 
*/