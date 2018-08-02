#if defined WINDOWS
#if defined METRO
#else
#endif
#else
#ifdef __ANDROID__
#include <sys/endian.h>
#endif
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string.h>
#include "basecode/BaseType.h"
#include "basecode/BaseLog.h"
#include "basecode/TcpServerSocket.h"

TcpServerSocket::TcpServerSocket()
{
	
}


bool TcpServerSocket::init( int servPort, int lisQ )
{
	m_srvPort = -1;
	m_listenQ = lisQ;

#if !defined METRO
	_serverConn = new Connection();
	_serverConn->_connId = ::socket( AF_INET, SOCK_STREAM, 0 );
	if ( _serverConn->_connId < 0 )
	{
        LogOut( MAIN, LOG_ERR, "socket failed, reason: %s", strerror(errno) );
        return false;
    }
	_serverConn->setBlocking( true );
  
	#ifndef WINDOWS
	int on = 1;
    if ( setsockopt ( _serverConn->_connId, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ) ) )
    {
        log_out(MAIN, LOG_ERR, "setsockopt failed, reason: %s", strerror(errno) );
        return false;
    }
	#endif
    
    memset( &_serverConn->_connAddr, 0, sizeof(_serverConn->_connAddr) );
	
	unsigned short minport = 1024, maxport = 65535;
	if ( servPort != -1 )
		minport = maxport = servPort;

	for ( short port=minport; port<=maxport; port++ )
	{
		_serverConn->_connAddr.sin_family = AF_INET;
		_serverConn->_connAddr.sin_addr.s_addr = htonl( INADDR_ANY );
		_serverConn->_connAddr.sin_port = htons( port );
		if ( bind( _serverConn->_connId, (sockaddr*)&_serverConn->_connAddr, sizeof(_serverConn->_connAddr)) != 0 ) 
		{
			if (  port == maxport )
			{
				LogOut( MAIN, LOG_ERR, "Port %d, bind failed, reason:%s", servPort, strerror(errno) );
				return false;	
			}
		}
		else
		{
			m_srvPort = port;
			break;
		}
	}
	if ( ::listen(_serverConn->_connId, m_listenQ) )					//¿ªÊ¼¼àÌý
    {
		LogOut( MAIN, LOG_ERR, "listen failed, reason:%s", strerror(errno) );
		return false;
    }
#endif
	return true;
}

short TcpServerSocket::getServerPort()
{
	return m_srvPort;
}

TcpServerSocket::TcpServerSocket( const TcpServerSocket& other )
{
	_serverConn = other._serverConn;
	m_listenQ = other.m_listenQ;
	m_srvPort = other.m_srvPort;
}


TcpServerSocket&
TcpServerSocket::operator=( TcpServerSocket& other )
{
    if ( this != &other )
    {
		_serverConn = other._serverConn;
		m_listenQ = other.m_listenQ;
		m_srvPort = other.m_srvPort;
    }
	return *this;
}

bool
TcpServerSocket::accept( Connection* conn, int timeout )
{
	if ( !_serverConn->isReadReady( timeout ) )
		return false;
	conn->close();
	#if !defined METRO
	socklen_t connAddrLen = sizeof( conn->_connAddr );
    if ( ( conn->_connId=::accept(_serverConn->_connId,(sockaddr*)&conn->_connAddr,&connAddrLen) ) < 0 )
    {
        LogOut( MAIN, LOG_ERR, "Failed to except the connection, reason:%s", strerror(errno) ); 
		return false;
    }
	LogDebug( MAIN, "Accept connection from %s", conn->getDescription().c_str() );
    conn->setBlocking( true );
    return true;
	#else
	return false;
	#endif
}


TcpServerSocket::~TcpServerSocket()
{

}
