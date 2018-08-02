#if defined WINDOWS
#if defined METRO
#else
#endif
#else
#ifdef __ANDROID__
#include <sys/endian.h>
#endif
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#include "basecode/basefunc.h"
#include "basecode/BaseLog.h"
#include "basecode/TcpClientSocket.h"
#include "basecode/NetworkAddress.h"
#include "basecode/basefunc_metro.h"

TcpClientSocket::TcpClientSocket( bool blocking/*=true*/ )
						: _serverPort( -1 ), _blocking( blocking ), _bAddr(false)
{

}

TcpClientSocket::TcpClientSocket( const Data& hostName, uint16 servPort, bool blocking )
								: _hostName( hostName ), _serverPort( servPort ), _blocking( blocking ),_bAddr(false)
{

}


TcpClientSocket::TcpClientSocket( const NetworkAddress& server, bool blocking )
												: _blocking( blocking )
{
    _hostName = server.getHostName();
    _serverPort = server.getPort();
}


TcpClientSocket::TcpClientSocket( const TcpClientSocket& other )
{
    _conn = other._conn;
	_hostName = other._hostName;
	_serverPort = other._serverPort;
	_blocking = other._blocking;
}


TcpClientSocket&
TcpClientSocket::operator=( TcpClientSocket& other )
{
    if ( this != &other)
    {
        _conn = other._conn;
        _hostName = other._hostName;
        _serverPort = other._serverPort;
        _blocking = other._blocking;
    }
    return *this;
}


TcpClientSocket::~TcpClientSocket()
{
	close();
}


bool
TcpClientSocket::connect( int32 timeout )
{
#ifdef METRO
	HostName^ hostName;
	try
	{
		hostName = ref new HostName( str2wstr(_hostName.c_str(),_hostName.length()) );
	}
	catch ( Platform::InvalidArgumentException^ e )
	{
		return false;
	}
	StreamSocket^ socket = ref new StreamSocket();
	String strServerPort = String::ToString( _serverPort );
	task<void> t( socket->ConnectAsync(hostName, str2wstr(strServerPort.c_str(),strServerPort.length()), SocketProtectionLevel::PlainSocket) );
	if ( _blocking )
	{
		try
		{
			t.get();
			_conn = new Connection();
			_conn->_reader = ref new DataReader( socket->InputStream );
			_conn->_writer = ref new DataWriter( socket->OutputStream );
		}
		catch ( Platform::Exception^ e )
		{
			return false;
		}
	}
	return true;
#else
	_connId = ::socket( AF_INET, SOCK_STREAM, 0 );
	if ( _connId == -1 )
	{
		//LogOut( MAIN, LOG_ERR, "Failed to create socket when connect to fserver %s, reason %s", _hostName.c_str(), strerror(errno) );
#ifdef WINDOWS
		LogOut( MAIN, LOG_ERR, "Failed to create socket when connect to fserver %s, reason %d", _hostName.c_str(), WSAGetLastError() );
#endif
		return false;
	}
	ref_ptr<Connection> conn = new Connection();
	conn->_connId = _connId;
	_conn = conn;

#ifdef WINDOWS
	const char chOpt = 1;
	setsockopt( _connId, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char) );
#endif

	int nRecvBuf = 64*1024;
	setsockopt( _connId, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf,sizeof(int) );
	int nSendBuf = 32*1024;
	setsockopt( _connId, SOL_SOCKET, SO_SNDBUF,(const char*)&nSendBuf,sizeof(int) );

	memset( &conn->_connAddr, 0, sizeof(conn->_connAddr) );
	if ( _bAddr )
	{
		memcpy( &conn->_connAddr, &_addr, sizeof(sockaddr) );
	}
	else
	{
		NetworkAddress na( _hostName, _serverPort );	
		conn->_connAddr.sin_family = AF_INET;
		conn->_connAddr.sin_addr.s_addr = inet_addr( na.getIpName().c_str() );
		conn->_connAddr.sin_port = htons( na.getPort() );
		if ( !na.isValid() )
		{
			log_out( MAIN, LOG_ERR, "addr err" );
			return false;
		}
	}

	if ( timeout < 0 )
	{
		if ( !conn->setBlocking(_blocking) )
			return false;
	}
	else
	{
		conn->setBlocking( false );
	}
	
	if ( ::connect( conn->_connId, (sockaddr*)&conn->_connAddr, sizeof(conn->_connAddr) ) < 0 )
    {
		if ( !_blocking )		//noblocking socket return SOCKET_ERROR when connect
			return true;
		if ( timeout < 0 )
		{
			close();
#ifdef WINDOWS
			LogOut( MAIN, LOG_ERR, "Failed to connect to server %s, reason %d", _hostName.c_str(), WSAGetLastError() );
#endif
			return false;
		}
    }

	if ( timeout >= 0 )
	{
		if ( conn->isWriteReady(timeout) )
		{
			if ( !conn->setBlocking(_blocking) )
				return false;
			return true;
		}
		else
		{
			return false;
		}
	}
#endif
	return true;
}


void
TcpClientSocket::setServer( const Data& server, uint16 serverPort, bool blocking )
{
	close();
	_bAddr = false;
	_hostName = server;
	_serverPort = serverPort;
	_blocking = blocking;
}


#if !defined METRO

void
TcpClientSocket::setServer( struct sockaddr addr, bool blocking )
{
	_bAddr = true;
	_addr = addr;
	_blocking = blocking;
}

#endif

void
TcpClientSocket::close()
{
	if ( _conn )
	{
		_conn->close();
		_conn = NULL;
	}
}
