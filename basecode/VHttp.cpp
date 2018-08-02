// VHttp.cpp: implementation of the VHttp class.
//
//////////////////////////////////////////////////////////////////////
#include "basecode/VHttp.h"
#include "basecode/TcpClientSocket.h"
#include "basecode/VFile.h"
#include "basecode/BaseLog.h"
#include "basecode/StrCoding.h"
#include "basecode/basefunc.h"

#define NOTIFY_GAP 1

VHttp::VHttp() : m_iNotifyPos(0),_callback(NULL),_lastTime(0),_iWrited(0),
					_speed(0),m_iStatusCode(0),_bCheckSpeed(false)
{
	_bClose = false;
}

VHttp::~VHttp()
{

}


void
VHttp::clear()
{
	m_iStatusCode = 0;
	m_iNotifyPos = 0;
}

Data VHttp::UrlEncode( const Data& url )
{
	int pos;
	Data dtUrl = url;
	while( ( pos=dtUrl.find( SP ) ) != -1 )
	{
		dtUrl.replace( pos, 1, "%20" );
	}
	return dtUrl;
}


void VHttp::setHttpProxy( const Data& proxy )
{
	LogDebug( MAIN, "VHttp set http proxy: %s", proxy.c_str());
    m_dtHttpProxy = proxy;
}


void VHttp::enableSpeedCheck()
{
	_bCheckSpeed = true;
}

void VHttp::close()
{
	_bClose = true;
	_clientSock.close();
}


int VHttp::GetHttpResponseData( const Data& fullUrl, char* pData, int len, bool bGet, bool bGetResp )
{
	Data dtServer;
	Data dtUrl;
	Data dtFullUrl = fullUrl;
	if ( isEqualNoCase(dtFullUrl.substr(0,7),"http://") )
	{
		dtFullUrl = dtFullUrl.substr( 7 );
	}

	int pos = dtFullUrl.find( "/"  );
	if ( pos == -1 )
	{
		return 0;
	}
	else
	{
		dtServer = dtFullUrl.substr( 0, pos );
		dtUrl = dtFullUrl.substr( pos );
	}

	return GetHttpResponseData( dtServer, dtUrl, pData, len, bGet, bGetResp );
}

int VHttp::GetHttpResponseData( const Data& ip, const Data& url, char* pData, int len, bool bGet, bool bGetResp )
{
	Data dtPost;
	int iLength = 0;
	bool bChunked = false;
	if ( !bGet )
	{
		std::map< Data, Data >::iterator iter;
		for ( iter=_paramMap.begin(); iter!=_paramMap.end(); iter++ )
		{
			dtPost += iter->first;
			dtPost += "=";
			dtPost += iter->second;
			dtPost += "&";
		}
		if ( _paramMap.size() > 0 )
			dtPost = dtPost.substr( 0, dtPost.length()-1 );
	}
	char buff[MTU];
	Data dtHttpRequ;
	if ( bGet )
		dtHttpRequ = "GET ";
	else
		dtHttpRequ = "POST ";
	dtHttpRequ += UrlEncode( url );

	dtHttpRequ += " HTTP/1.1";
	dtHttpRequ += CRLF;
	if ( !bGet )
	{
		dtHttpRequ += "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, */*";
		dtHttpRequ += CRLF;
		dtHttpRequ += "Accept-Language: zh-cn";
		dtHttpRequ += CRLF;
		dtHttpRequ += "Content-Type: application/x-www-form-urlencoded";
		dtHttpRequ += CRLF;
		dtHttpRequ += "Accept-Encoding: gzip, deflate";
		dtHttpRequ += CRLF;
		dtHttpRequ += "Content-Length: ";
		dtHttpRequ += Data::ToString( dtPost.length() );
		dtHttpRequ += CRLF;
	}
	dtHttpRequ += "Host: ";
	dtHttpRequ += ip;
	if ( !m_dtCookie.isEmpty() )
	{
		dtHttpRequ += "Cookie: ";
		dtHttpRequ += m_dtCookie;
	}
	dtHttpRequ += CRLF;
	dtHttpRequ += CRLF;
	
	dtHttpRequ += dtPost;
	TcpClientSocket clientSock( ip, 80 );
	clientSock.connect();
	
	Connection* pConn = clientSock.getConn();
	pConn->writeData( dtHttpRequ );

	if ( !bGetResp )
		return 0;
	
	int iRead;
	int iWrite = 0;

	if ( (iRead=pConn->readLine(buff,MTU)) <= 0 )
		return 0;
	Data dtData;
	Data dtLine( buff, iRead );
	if ( !dtLine.match(SP,&dtData,true) )
		return 0;
	if ( dtData!="HTTP/1.0" && dtData!="HTTP/1.1" )
		return iWrite;
	if ( !dtLine.match(SP,&dtData,true) )
		return 0;
	m_iStatusCode = dtData.toInt32();
	if ( m_iStatusCode != 200 )
		return 0;

	while ( (iRead=pConn->readLine(buff,MTU)) > 0 )
	{
		Data dtLine( buff, iRead );
		Data dtBefVal;
		if ( FOUND == dtLine.match( ":", &dtBefVal, true ) )
		{
			dtBefVal.removeSpaces();
			dtLine.removeSpaces();
			if ( isEqualNoCase( dtBefVal, "Content-Length" ) )
			{	
				iLength = dtLine.toInt32();
			}
			if ( isEqualNoCase(dtBefVal,"Transfer-Encoding:") )
			{
				if ( isEqualNoCase(dtLine,"chunked") )
					bChunked = true;
			}
			_respHeadMap[dtBefVal] = dtLine;
		}
	}
	if ( iRead < 0 )
		return 0;
	
	return iWrite;
}


void VHttp::setCookie( Data cookie )
{
	m_dtCookie = cookie;
}


void VHttp::setNotifyCallback( IHttpCallback* callback )
{
	_callback = callback;
}


int
VHttp::getHttpFile( const Data& fullUrl, const Data& savefile, int startpos )
{
	Data dtServer;
	Data dtFile;
	Data dtFullUrl = fullUrl;
	if ( isEqualNoCase(dtFullUrl.substr(0,7),"http://") )
	{
		dtFullUrl = dtFullUrl.substr( 7 );
	}

	int pos = dtFullUrl.find( "/"  );
	if ( pos == -1 )
	{
		return 0;
	}
	else
	{
		dtServer = dtFullUrl.substr( 0, pos );
		dtFile = dtFullUrl.substr( pos );
	}
	return getHttpFile( dtServer, dtFile, savefile, startpos );
}


void VHttp::setUserAgent( const Data& agent )
{
	m_dtUserAgent = agent;
}


int
VHttp::getHttpFile( const Data& server, const Data& httpfile, const Data& savefile, int startpos )
{
	bool bChunked = false;
	m_iWriteLen = startpos;
	m_iLength = 0;
	
	_respHeadMap.clear();
	m_iNotifyPos = 0;
	m_iStatusCode = 0;
	bool bGet = ( _paramMap.size() == 0 );

	VFile file;
	Data httpRequest;
	Data dtPost;
	
	if ( bGet )
	{
		httpRequest = "GET ";
	}
	else
	{
		httpRequest = "POST ";
		std::map< Data, Data >::iterator iter;
		for ( iter=_paramMap.begin(); iter!=_paramMap.end(); iter++ )
		{
			if ( iter != _paramMap.begin() )
				dtPost += "&";
			dtPost += iter->first;
			dtPost += "=";
			dtPost += iter->second;
		}
	}

	httpRequest += httpfile;
	httpRequest += " HTTP/1.1";
	httpRequest += CRLF;
	httpRequest += "Host: ";
	httpRequest += server;
	httpRequest += CRLF;
	httpRequest += "Accept: */*";
	httpRequest += CRLF;
	if ( !m_dtUserAgent.isEmpty() )
	{
		httpRequest += "User-Agent: ";
		httpRequest += m_dtUserAgent;
		httpRequest += CRLF;
	}

	if ( startpos > 0 )
	{
		httpRequest += "Range: bytes=";
		httpRequest += startpos;
		httpRequest += "-";
		httpRequest += CRLF;
	}

	httpRequest += "Pragma: no-cache";
	httpRequest += CRLF;
	httpRequest += "Cache-Control: no-cache";
	httpRequest += CRLF;
	httpRequest += "Connection: close";
	httpRequest += CRLF;

	if ( !bGet )
	{
		httpRequest += "Content-Type: application/x-www-form-urlencoded";
		httpRequest += CRLF;
		httpRequest += "Content-Length: ";
		httpRequest += Data::ToString( dtPost.length() );
		httpRequest += CRLF;
	}

	httpRequest += CRLF;

	char buff[MTU] = {0,};
    

    if ( m_dtHttpProxy.isEmpty() )
    {
		LogDebug( MAIN, "Connect to Server:  %s", server.c_str() );
	    _clientSock.setServer( server, 80 );
    }
    else
    {
		LogDebug( MAIN, "Connect to Server:  %s", m_dtHttpProxy.c_str() );
        _clientSock.setServer( m_dtHttpProxy, 80 );
    }
	if ( !_clientSock.connect() )
	{
		LogOut( MAIN, LOG_ERR, "connect err" );
		return -1;
	}

	Connection* pConn = _clientSock.getConn();

	if ( pConn->writeData(httpRequest) != (int)httpRequest.length() )
		return -1;

	if ( !bGet )
	{
		if ( pConn->writeData(dtPost) != (int)dtPost.length() )
			return -1;
	}

	int iRead;

	m_iStatusCode = 0;
	memset( buff, 0, MTU );
	if ( (iRead=pConn->readLine(buff,MTU)) <= 0 )
	{
		LogOut( MAIN, LOG_ERR, "Read command line err" );
		return 0;
	}
	
	Data dtData;
	Data dtLine( buff, iRead );
	if ( !dtLine.match(SP,&dtData,true) )
	{
		LogOut( MAIN, LOG_ERR, "Read command line mactch space err" );
		return 0;
	}
	if ( dtData!="HTTP/1.0" && dtData!="HTTP/1.1" )
	{
		LogOut( MAIN, LOG_ERR, "GET HTTP HEAD ERR" );
		return 0;
	}
	if ( !dtLine.match(SP,&dtData,true) )
	{
		LogOut( MAIN, LOG_ERR, "Read command line mactch space 2 err" );
		return 0;
	}
	m_iStatusCode = dtData.toInt32();

	while ( (iRead=pConn->readLine(buff,MTU)) > 0 )
	{
		Data dtLine( buff, iRead );
		Data dtBefVal;
		if ( FOUND == dtLine.match( ":", &dtBefVal, true ) )
		{
			dtBefVal.removeSpaces();
			dtLine.removeSpaces();
			if ( isEqualNoCase( dtBefVal, "Content-Length" ) )
			{
				m_iLength = dtLine.toInt32() + startpos;
				LogDebug( MAIN, "HTTP FILE LEN IS %d", m_iLength );
			}
			else if ( isEqualNoCase(dtBefVal,"Transfer-Encoding") )
			{
				if ( isEqualNoCase(dtLine,"chunked") )
					bChunked = true;
			}
			_respHeadMap[dtBefVal] = dtLine;
		}
	}

	if ( iRead < 0 )
	{
		LogOut( MAIN, LOG_ERR, "read err" );
		return -1;
	}

	if ( m_iStatusCode!=200 && m_iStatusCode!=206 )
	{
		if ( m_iStatusCode == 302 )
		{
			Data dtRedirectUrl = getRespFieldValue( "Location" );
			if ( dtRedirectUrl.length()>0 && dtRedirectUrl[(unsigned int)0]=='/' )
				dtRedirectUrl = server + dtRedirectUrl;
			return getHttpFile( dtRedirectUrl, savefile, startpos );
		}
		LogOut( MAIN, LOG_ERR, "Err status code %d", m_iStatusCode );
		return 0;
	}


	LogDebug( MAIN, "HTTP RETURN %d CODE", m_iStatusCode );

	if ( startpos <= 0 )
	{
		if ( !file.openFile(savefile,VFile::READWRITE) )
		{
			LogOut( MAIN, LOG_ERR, "open file %s err", savefile.c_str() );
			return 0;
		}
	}
	else
	{
		if ( !file.openFile(savefile,VFile::MODIFY) )
		{
			LogOut( MAIN, LOG_ERR, "open file %s err", savefile.c_str() );
			return 0;
		}
		file.seekTo( startpos, SEEK_SET );
	}

	if ( bChunked )
	{
		LogDebug( MAIN, "CHUNK TYPE" );
		unsigned char* pBuff = new unsigned char[MTU];
		int iBuffLen = MTU;
		while( (iRead=pConn->readLine(buff,MTU)) > 0 )
		{
			if ( iRead>8 )
			{
				LogDebug( MAIN, "read chunk line err %s", buff );
				return -1;
			}
			int len = Data( buff,iRead ).HexToInt();
			if ( len <= 0 )
			{
				delete[] pBuff;
				return m_iWriteLen;
			}
			if ( len > iBuffLen )
			{
				delete[] pBuff;
				iBuffLen = len;
				pBuff = new unsigned char[iBuffLen];
			}
			int iReaded = 0;
			memset( pBuff, 0, len );
			pConn->readData( pBuff, len, iReaded );
			file.write( pBuff, iReaded );
			m_iWriteLen += iReaded;
			if ( m_iLength > 0 )
			{
				if ( m_iWriteLen >= m_iLength )
				{
					file.closeFile();
					break;
				}
			}
			if ( iReaded != len )
			{
				delete[] pBuff;
				LogDebug( MAIN, "chunk len err" );
				return m_iWriteLen;
			}
			if ( pConn->readLine(buff,MTU) != 0 )
				return m_iWriteLen;
		}
		delete[] pBuff;
		return m_iWriteLen;
	}
	else
	{
		LogDebug( MAIN, "DIRECT CONN TYPE" );
		while ( (iRead=pConn->readn(buff,MTU)) > 0 )
		{
			file.write( (unsigned char*)buff, iRead );
			m_iWriteLen += iRead;
			if ( m_iLength > 0 )
			{
				if ( m_iWriteLen >= m_iLength )
				{
					file.closeFile();
					break;
				}
			}
		}
	}
	pConn->close();
	file.closeFile();
	return m_iWriteLen;
}


ref_ptr<MemStream>
VHttp::getHttpFileStream( const Data& fullUrl )
{
	Data dtServer;
	Data dtFile;
	Data dtFullUrl = fullUrl;
	if ( isEqualNoCase(dtFullUrl.substr(0,7),"http://") )
	{
		dtFullUrl = dtFullUrl.substr( 7 );
	}

	int pos = dtFullUrl.find( "/"  );
	if ( pos == -1 )
	{
		return 0;
	}
	else
	{
		dtServer = dtFullUrl.substr( 0, pos );
		dtFile = dtFullUrl.substr( pos );
	}
	return getHttpFileStream( dtServer, dtFile );
}


ref_ptr<MemStream>
VHttp::getHttpFileStream( const Data& server, const Data& httpfile )
{
	bool bChunked = false;
	m_iWriteLen = 0;
	m_iLength = 0;
	
	_respHeadMap.clear();
	m_iNotifyPos = 0;
	m_iStatusCode = 0;
	bool bGet = ( _paramMap.size() == 0 );

	VFile file;
	Data httpRequest;
	Data dtPost;
	
	if ( bGet )
	{
		httpRequest = "GET ";
	}
	else
	{
		httpRequest = "POST ";
		std::map< Data, Data >::iterator iter;
		for ( iter=_paramMap.begin(); iter!=_paramMap.end(); iter++ )
		{
			if ( iter != _paramMap.begin() )
				dtPost += "&";
			dtPost += iter->first;
			dtPost += "=";
			dtPost += iter->second;
		}
	}

	httpRequest += httpfile;
	httpRequest += " HTTP/1.1";
	httpRequest += CRLF;
	httpRequest += "Host: ";
	httpRequest += server;
	httpRequest += CRLF;
	httpRequest += "Accept: */*";
	httpRequest += CRLF;
	if ( !m_dtUserAgent.isEmpty() )
	{
		httpRequest += "User-Agent: ";
		httpRequest += m_dtUserAgent;
		httpRequest += CRLF;
	}


	httpRequest += "Pragma: no-cache";
	httpRequest += CRLF;
	httpRequest += "Cache-Control: no-cache";
	httpRequest += CRLF;
	httpRequest += "Connection: close";
	httpRequest += CRLF;

	if ( !bGet )
	{
		httpRequest += "Content-Type: application/x-www-form-urlencoded";
		httpRequest += CRLF;
		httpRequest += "Content-Length: ";
		httpRequest += Data::ToString( dtPost.length() );
		httpRequest += CRLF;
	}

	httpRequest += CRLF;

	char buff[MTU] = {0,};
    

    if ( m_dtHttpProxy.isEmpty() )
    {
		LogDebug( MAIN, "Connect to Server:  %s", server.c_str() );
	    _clientSock.setServer( server, 80 );
    }
    else
    {
		LogDebug( MAIN, "Connect to Server:  %s", m_dtHttpProxy.c_str() );
        _clientSock.setServer( m_dtHttpProxy, 80 );
    }
	if ( !_clientSock.connect() )
	{
		LogOut( MAIN, LOG_ERR, "connect err" );
		return NULL;
	}

	Connection* pConn = _clientSock.getConn();

	if ( pConn->writeData(httpRequest) != (int)httpRequest.length() )
		return NULL;

	if ( !bGet )
	{
		if ( pConn->writeData(dtPost) != (int)dtPost.length() )
			return NULL;
	}

	int iRead;

	m_iStatusCode = 0;
	memset( buff, 0, MTU );
	if ( (iRead=pConn->readLine(buff,MTU)) <= 0 )
	{
		LogOut( MAIN, LOG_ERR, "Read command line err" );
		return 0;
	}
	
	Data dtData;
	Data dtLine( buff, iRead );
	if ( !dtLine.match(SP,&dtData,true) )
	{
		LogOut( MAIN, LOG_ERR, "Read command line mactch space err" );
		return 0;
	}
	if ( dtData!="HTTP/1.0" && dtData!="HTTP/1.1" )
	{
		LogOut( MAIN, LOG_ERR, "GET HTTP HEAD ERR" );
		return 0;
	}
	if ( !dtLine.match(SP,&dtData,true) )
	{
		LogOut( MAIN, LOG_ERR, "Read command line mactch space 2 err" );
		return 0;
	}
	m_iStatusCode = dtData.toInt32();

	while ( (iRead=pConn->readLine(buff,MTU)) > 0 )
	{
		Data dtLine( buff, iRead );
		Data dtBefVal;
		if ( dtLine.match( ":", &dtBefVal, true ) )
		{
			dtBefVal.removeSpaces();
			dtLine.removeSpaces();
			if ( isEqualNoCase( dtBefVal, "Content-Length" ) )
			{
				m_iLength = dtLine.toInt32();
				LogDebug( MAIN, "HTTP FILE LEN IS %d", m_iLength );
			}
			else if ( isEqualNoCase(dtBefVal,"Transfer-Encoding") )
			{
				if ( isEqualNoCase(dtLine,"chunked") )
					bChunked = true;
			}
			_respHeadMap[dtBefVal] = dtLine;
		}
	}

	if ( iRead < 0 )
	{
		LogOut( MAIN, LOG_ERR, "read err" );
		return NULL;
	}

	if ( m_iStatusCode!=200 && m_iStatusCode!=206 )
	{
		if ( m_iStatusCode == 302 )
		{
			Data dtRedirectUrl = getRespFieldValue( "Location" );
			if ( dtRedirectUrl.length()>0 && dtRedirectUrl[(unsigned int)0]=='/' )
				dtRedirectUrl = server + dtRedirectUrl;
			return getHttpFileStream( dtRedirectUrl );
		}
		LogOut( MAIN, LOG_ERR, "Err status code %d", m_iStatusCode );
		return 0;
	}


	LogDebug( MAIN, "HTTP RETURN %d CODE", m_iStatusCode );
	vector<ref_ptr<MemStream>> vecMem;
	if ( bChunked )
	{
		LogDebug( MAIN, "CHUNK TYPE" );
		unsigned char buff[MTU];
		ref_ptr<MemStream> memStream;
		while( (iRead=pConn->readLine(buff,MTU)) > 0 )
		{
			if ( iRead>8 )
			{
				LogDebug( MAIN, "read chunk line err %s", buff );
				return NULL;
			}
			int len = Data( (char*)buff,iRead ).HexToInt();
			if ( len <= 0 )
				break;
			memStream = new ( len ) MemStream( len );
			int iReaded = pConn->readData( memStream->getPoint(), len );
			vecMem.push_back( memStream );
			m_iWriteLen += iReaded;
			if ( iReaded != len )
			{
				LogDebug( MAIN, "chunk len err" );
				return NULL;
			}
			if ( pConn->readLine(buff,MTU) != 0 )
				return NULL;
		}
		pConn->close();
		uint32 maxLen = 0;
		for ( vector<ref_ptr<MemStream>>::iterator iter=vecMem.begin(); iter!=vecMem.end(); iter++ )
		{
			maxLen += (*iter)->getStreamSize();
		}
		memStream = new ( maxLen ) MemStream( maxLen );
		for ( vector<ref_ptr<MemStream>>::iterator iter=vecMem.begin(); iter!=vecMem.end(); iter++ )
		{
			memStream->write( (*iter)->getStreamSize(), (*iter)->getPoint() );
		}
		return memStream;
	}
	else
	{
		if ( m_iLength <= 0 )
			return NULL;
		LogDebug( MAIN, "DIRECT CONN TYPE" );
		ref_ptr<MemStream> memStream = new ( m_iLength ) MemStream( m_iLength );
		
		if ( pConn->readData(memStream->getPoint(),m_iLength) != m_iLength )
			return NULL;
		return memStream;
	}
	
}


Data VHttp::getRespFieldValue( const Data& field )
{
	for( std::map< Data, Data >::iterator iter=_respHeadMap.begin(); iter!=_respHeadMap.end(); iter++ )
	{
		if ( isEqualNoCase(iter->first, field) )
		{
			return iter->second;
		}
	}
	return "";
}


ref_ptr<MemStream>
VHttp::getHttpStream( const Data& file, uint32 retryNum )
{
	int32 pos;
	if ( (pos=file.find("/")) == -1 )
		return NULL;
	Data server = file.substr( 0, pos );
	Data httpfile = file.substr( pos );
	return getHttpStream( server, httpfile, -1, retryNum );
}


ref_ptr<MemStream>
VHttp::getHttpStream( const Data& server, const Data& httpfile, int32 len, uint32 retryNum )
{
	LogDebug( RES, "get http server %s file %s", server.c_str(), httpfile.c_str() );
	uint16 port = 80;
	vector<Data> vecData;
	server.split( ":", vecData );
	Data dtServer;
	if ( vecData.size() == 2 )
	{
		dtServer = vecData[0];
		port = (uint16)vecData[1].toInt32();
	}
	else
	{
		dtServer = server;
	}

	Data file = httpfile;
	if ( httpfile[0] != '/' )
		file = Data("/") + httpfile;

	if ( _bCheckSpeed )
	{
		_lastTime = getSysMillsec();
		_speed = 0;
		_iWrited = 0;
	}

	m_iLength = 0;
	m_iWriteLen = 0;
	m_iNotifyPos = 0;

	bool bVal = true;
	
	file = strCoding::UrlUTF8( file.c_str() );
	ref_ptr<MemStream> spStream;

	for ( uint32 retryIndex=0; retryIndex<retryNum; retryIndex++ )
	{
		if ( _bClose )
			return NULL;
		Data httpRequest = "GET ";
		httpRequest += file;
		httpRequest += " HTTP/1.1";
		httpRequest += CRLF;
		httpRequest += "Host: ";
		httpRequest += server;
		httpRequest += CRLF;

		if ( m_iWriteLen > 0 )
		{
			httpRequest += "Range: bytes=";
			httpRequest += m_iWriteLen;
			httpRequest += "-";
			httpRequest += CRLF;
		}
		httpRequest += CRLF;
		unsigned char buff[MTU];
		_clientSock.setServer( dtServer, port );
		
		if ( !_clientSock.connect() )
		{
			LogOut( RES, LOG_ERR, "http conn err" );
			vsleep( 100 );
			continue;
		}
		
		Connection* pConn = _clientSock.getConn();
		if ( pConn == NULL )
			return NULL;
		pConn->writeData( httpRequest );
		int iRead;
		bool bFound = false;
		Data dtHead;

		while ( (iRead=pConn->readn(buff,MTU,10000))>0 )
		{
			if ( _bCheckSpeed )
			{
				_iWrited += iRead;
				uint32 time = getSysMillsec();
				uint32 timeGap = time-_lastTime;
				if ( timeGap > 1000 )
				{
					_speed = (uint32)( ((float)_iWrited / ((float)timeGap/1000.0f)) / 1000.0f );
					//_speed = _iWrited/1000;
					_lastTime = time;
					_iWrited = 0;
				}
			}
			if ( !bFound )
			{
				for ( int i=0; i<= iRead-4; i++ )
				{
					if ( buff[i] == 0x0d && buff[i+1] == 0x0a && buff[i+2] == 0x0d && buff[i+3] == 0x0a )
					{
						Data dtLine;
						dtHead = Data( (char*)buff, i+4 );
						Data dtRequestLine,dtCode;
						if ( !dtHead.match( CRLF, &dtRequestLine, true ) )
						{
							LogOut( MAIN, LOG_ERR, "match request line error" );
							return NULL;
						}
						if ( !dtRequestLine.match( "HTTP/1.1 ", NULL, true ) 
							&& !dtRequestLine.match( "HTTP/1.0 ", NULL, true ) )
						{
							LogOut( MAIN, LOG_ERR, "match http head error" );
							return NULL;
						}

						if ( !dtRequestLine.match( SP, &dtCode, true ) )
						{
							LogOut( MAIN, LOG_ERR, "match SP2 error" );
							return NULL;
						}

						m_iStatusCode=dtCode.toInt32();
						if ( m_iStatusCode!=200 && m_iStatusCode!=206 && m_iStatusCode!=302 &&  m_iStatusCode!=301 )
						{
							LogOut( MAIN, LOG_ERR, "match status code error %d file %s", m_iStatusCode, httpfile.c_str() );
							return NULL;
						}

						while ( dtHead.match( CRLF, &dtLine, true ) )
						{
							Data dtBefVal;
							if ( dtLine.match( ":", &dtBefVal, true ) )
							{
								dtBefVal.removeSpaces();
								if ( isEqualNoCase(dtBefVal, "Content-Length") )
								{
									dtLine.removeSpaces();
									m_iLength = dtLine.toInt32();
									if ( len>0 && m_iLength==len )
									{
										return NULL;
									}
								}
								else if ( (m_iStatusCode==302 || m_iStatusCode==301) && isEqualNoCase(dtBefVal,"Location") )
								{
									dtLine.removeSpaces();
									if ( dtLine.length()>7 && dtLine.substr(0,7).isEqualNoCase("http://") )
									{
										dtLine = dtLine.substr( 7 );
										LogDebug( RES, "http redirect 111 %s", dtLine.c_str() );
										return getHttpStream( dtLine, retryNum );
									}
									else
									{
										LogDebug( RES, "http redirect %s", dtLine.c_str() );
										return getHttpStream( dtLine, retryNum );
									}
								}

							}
							/*
							else if ( dtLine.match( "HTTP/1.1", NULL, true ) )
							{
							dtLine.removeSpaces();
							dtLine.match( " ", &dtBefVal, true );
							m_iStatusCode = dtBefVal.toInt32();
							}
							*/
						}

						if ( m_iLength <= 0 )
						{
							LogOut( MAIN, LOG_ERR, "file len is 0 : %s", httpfile.c_str() );
							return spStream;
						}
						else if ( m_iStatusCode!=200 && m_iStatusCode!=206 )
						{
							LogOut( MAIN, LOG_ERR, "Get status err: %d", m_iStatusCode );
							return spStream;
						}
						else
						{
							if ( m_iStatusCode == 200 )	
							{
								spStream = new ( m_iLength ) MemStream( m_iLength );
								m_iWriteLen = 0;
							}
							else
							{
								m_iLength = m_iWriteLen + m_iLength;
							}
						}
						spStream->write( iRead-i-4, &buff[i+4] );
						m_iWriteLen += (iRead-i-4);
						processCallback();
						bFound = true;
						if ( m_iLength>0 && m_iWriteLen>=m_iLength )
						{
							LogDebug( MAIN, "LEN ENOUGH" );
							return spStream;
						}
						break;
					}
				}
			}
			else
			{
				spStream->write( iRead, buff );
				m_iWriteLen += iRead;
				processCallback();
				if ( m_iWriteLen >= m_iLength )
				{
					return spStream;
				}
			}
		}			
	}
	
	LogOut( RES, LOG_ERR, "read err %d %d", m_iWriteLen, m_iLength );
	return NULL;
}

void VHttp::addParam( const char* name, const char* val )
{
	_paramMap[ name ] = val;
}

void VHttp::clearParam()
{
	_paramMap.clear();
}

void VHttp::processCallback()
{
	if ( _callback == NULL )
		return;
	uint32 pos = (uint32)( (float32)m_iWriteLen / (float32)m_iLength * 100 );
	if ( pos == 100 )
	{
		_callback->progress( 100 );
	}
	else if ( pos > m_iNotifyPos )
	{
		_callback->progress( m_iNotifyPos );
		m_iNotifyPos = (pos/NOTIFY_GAP+1)*NOTIFY_GAP;
	}
}


uint32 VHttp::getSpeed()
{
	return _speed;
}


int VHttp::getWriteLen()
{
	return m_iWriteLen;
}

int VHttp::getFileLen()
{
	return m_iLength;
}
