#include <fstream>
#include <assert.h>
#include <errno.h>
#include <iostream>
#include <stdlib.h>

#if defined WINDOWS
#if defined METRO

#else
#endif
#else
#ifdef __ANDROID__
#include <sys/endian.h>
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#endif

#include "basecode/basefunc.h"
#include "basecode/LogOut.h"
#include "basecode/basetype.h"
#include "basecode/UdpStack.h"

UdpStack::UdpStack()
{
	
}

bool UdpStack::init( const Data& server, uint16 clientPort, uint16 servPort, bool block, UdpMode udpMode, bool isMulticast )
{
	setBlock( _block );
	#ifdef METRO
	
	#else
	localPort = -1;
	_block = block;
	mode = udpMode;
	_socketFd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( _socketFd < 0 )
        return false;
	int buf1 = 1;
	socklen_t len1 = sizeof( buf1 );
	#ifndef WINDOWS
/*	struct protoent *protoent;
	protoent = getprotobyname( "icmp" );
    if ( !protoent )
    {
        LogOut( NETWORK, LOG_ERR, "Cannot get icmp protocol" );
	}
	else
    {
		if ( setsockopt( _socketFd, protoent->p_proto, SO_BSDCOMPAT, (char*)&buf1, len1) == -1 )
			LogOut( NETWORK, LOG_ERR, "setsockopt error SO_BSDCOMPAT :%s\n", strerror(errno) );
    }*/
	#endif
	
    if ( isMulticast )
    {
		// set option to get rid of the "Address already in use" error
		if ( setsockopt( _socketFd, SOL_SOCKET, SO_REUSEADDR, (char*)&buf1, len1 ) == -1 )
		{
			fprintf( stderr, "setsockopt error SO_REUSEADDR :%s", strerror(errno) );
		}
	}

    // set up addresses
	memset( (void*)&(_localAddr), 0, sizeof( _localAddr ) );
	( _localAddr ).sin_family = AF_INET;
	( _localAddr ).sin_addr.s_addr = htonl( INADDR_ANY );
	memset( (void*)&(_remoteAddr), 0, sizeof(_remoteAddr) );
	_remoteAddr.sin_family = AF_INET;
	_remoteAddr.sin_addr.s_addr = htonl( INADDR_ANY );
	#endif

	bool bVal = true;
    switch ( mode )
    {
	case inactive :
		if ( !server.isEmpty() )
			bVal = doClient( server,clientPort );
        break;
	case sendonly:			//发送模式
		if( !server.isEmpty() )
			bVal = doClient( server,clientPort );
        break;
	case recvonly:			//接收模式
		if ( servPort == 0 )
			break;
		bVal = doServer( servPort );
		if ( bVal && !server.isEmpty() )
			doClient( server,clientPort );
        break;
	case sendrecv:			//发送接受模式
		if ( bVal && !server.isEmpty() )
			doClient( server,clientPort );
		if ( servPort == 0 )
			break;
		bVal = doServer( servPort );
        break;
	default :
        break;
    }

	if ( !bVal )
	{
		close();
		return false;
	}
	else
	{
		return true;
	}
}

///从开始到结束端口查找可用端口并绑定监听
bool
UdpStack::doServer( uint16 port )
{
	#if !defined METRO
	_localAddr.sin_port = htons( port );
	if ( bind( _socketFd, (struct sockaddr*)&_localAddr, sizeof(_localAddr) ) != 0 )
		return false;
	return true;
	#else
	return false;
	#endif
}


///设置要接受的客户端地址以及端口
bool
UdpStack::doClient( const Data& server, uint16 port )
{
#if defined METRO
	HostName^ hostName;
	try
	{
		hostName = ref new HostName( str2wstr(server.c_str(),server.length()) );
	}
	catch ( Platform::InvalidArgumentException^ e )
	{
		return false;
	}
	DatagramSocket^ socket = ref new DatagramSocket();
	String strServerPort = String::ToString( port );
	task<void>( socket->ConnectAsync(hostName, str2wstr(strServerPort.c_str(),strServerPort.length())) ).then( [this,socket](void)
	{
		this->_dataWriter = ref new DataWriter( socket->OutputStream );
	}
	);
	return true;
#else
	NetworkAddress addr( server, port );
	_remoteAddr.sin_addr.s_addr = addr.getIp4Address();
    _remoteAddr.sin_port = htons( addr.getPort() );
	if ( connect( _socketFd, (struct sockaddr*)&_remoteAddr, sizeof(sockaddr) ) != 0 )
	{
		LogOut( NETWORK, LOG_ERR, "UdpStack during socket connect %s", strerror(errno) );
		return false;
	}
	return true;
#endif
}



NetworkAddress
UdpStack::getDestinationHost() const
{
    NetworkAddress desHost;
	#if defined METRO
	#else
	desHost.setIp4Addr( _remoteAddr.sin_addr.s_addr, ntohs(_remoteAddr.sin_port) );
	#endif
    return desHost;
}

NetworkAddress
UdpStack::getLocalHost() const
{
	NetworkAddress tarAddr;
#if defined METRO

#else
	struct sockaddr_in addr;
	socklen_t len = sizeof( sockaddr_in );
	getsockname( _socketFd, (sockaddr*)&addr, &len );
	tarAddr.setIp4Addr( addr.sin_addr.s_addr, ntohs(addr.sin_port) );
#endif
	return tarAddr;
}


#ifndef METRO

SOCKET
UdpStack::getSocketFD ()
{
    return _socketFd;
}


void UdpStack::addToFdSet( fd_set* set )
{
    assert(set);
    FD_SET( _socketFd, set );
}

bool UdpStack::checkIfSet( fd_set* set )
{
	assert(set);
	return FD_ISSET( _socketFd, set ) != 0;
}


int UdpStack::receiveFrom( const void* buffer, const int bufSize, sockaddr_in* sender, long millsecond )
{
	if ( (mode==sendonly) || (mode==inactive) )
		return -1;
	socklen_t lenSrc = sizeof( sockaddr_in );
	if ( !isReadReady( millsecond ) )
		return 0;
	int len = recvfrom( _socketFd, (char *)buffer, bufSize, 0, (sockaddr*)sender, &lenSrc );
	if ( len <= 0 )
	{
		return -1;
	}
	else
	{
		return len;
	}
}


int
UdpStack::transmitTo( const void* buffer, const int length, const sockaddr_in dest, long millsecond )
{
	if ( (mode==recvonly) || (mode==inactive) )
    {
        return 0;
    }
    assert( buffer );
    assert( length > 0 );

	if ( !isWriteReady( millsecond ) )
		return 0;

    int count = sendto( _socketFd, (char*)buffer, length, 0, (struct sockaddr*)&dest, sizeof(sockaddr_in) );
    if ( count < 0 )
    {
		/*
        int err = errno;
        Data errMsg = "UdpStack transmit err :";
        switch ( err )
        {
		case ECONNREFUSED:
	        //This is the most common error - you get it if the host
			//does not exist or is nor running a program to recevice
		    //the packets. This is what you get with the other side
			//crashes.
			errMsg  += "UdConnection refused by destination host";
            break;
		case EHOSTDOWN:
			errMsg += "destination host is down";
            break;
		case EHOSTUNREACH:
			errMsg += "no route to to destination host";
			return 3;
			break;
		default:
			errMsg += strerror( err );
			break;
        }
		*/
		return -1;
    }
	else
	{
		if ( count != length )
		{
			Data errMsg = "UdpStack transmit err :";
		}
		return count;
    }
}

#endif

/*
int
UdpStack::getMaxFD ( int prevMax )
{
    return ( getSocketFD() > prevMax ) ? getSocketFD() : prevMax;
}
*/

void
UdpStack::close()
{
	#if defined WINDOWS
	#if defined METRO
	#else
	closesocket( _socketFd );
	#endif
	#else
	::close( _socketFd );
	_socketFd = -1;
	#endif
	localPort = -1;
}


UdpMode
UdpStack::getMode ()
{
	return mode;
};

void
UdpStack::setMode ( const UdpMode theMode )
{
	mode = theMode;
};

unsigned short
UdpStack::getLocalPort()
{
	return localPort;
};

unsigned short
UdpStack::getDestinationPort()
{
#if defined METRO
	return 0;
#else
	return ntohs( _remoteAddr.sin_port );
#endif
};

int
UdpStack::receive( const void* buf, const int bufSize, long millsecond )
{
    if ( (mode==sendonly) || (mode==inactive) )
    {
        return -1;
    }
	if ( !isReadReady( millsecond ) )
		return 0;
    int len = 0;
	#if defined METRO
	#else
	len = recv( _socketFd, (char *)buf, bufSize, 0 );
	#endif
    if ( len <= 0 )
    {
		return -1;
    }
    else
    {
		return len;
    }
}


int
UdpStack::receiveFrom( const void* buffer, const int bufSize, NetworkAddress* sender, long millsecond )
{
    if ( (mode==sendonly) || (mode==inactive) )
        return -1;
	int len = 0;
    Data hostname;
	if ( !isReadReady( millsecond ) )
		return 0;
	#if defined METRO
	#else
	struct sockaddr_in addr;
	socklen_t addrLen = sizeof( addr );
    len = recvfrom( _socketFd, (char *)buffer, bufSize, 0, (struct sockaddr*)&addr, &addrLen );
	#endif
    if ( len <= 0 )
    {
		return -1;
    }
    else
    {
		if ( NULL != sender )
		{
			#if defined METRO
			#else
			sender->setIp4Addr( addr.sin_addr.s_addr, ntohs( addr.sin_port ) );
			#endif
		}
		return len;
    }
}



/** uses send() which is better to get ICMP msg back
 function returns a 0  normally **/
int
UdpStack::transmit( const void* data, uint32 len, long millsecond )
{
    if ( (mode==recvonly) || (mode==inactive) )
        return 0;
    assert( data );
    assert( len > 0 );
	int count = 0;
	#if defined METRO
	Platform::Array<unsigned char>^ writeArray = ref new Platform::Array<unsigned char>( (unsigned char*)data, len );
	_dataWriter->WriteBytes( writeArray );
	#else
	if ( !isWriteReady( millsecond ) )
		return 0;
	count = send( _socketFd,(char *)data, len, 0 );
	#endif
    if ( count < 0 )
    {
		/*
        int err = errno;
        switch ( err )
        {
            case ECONNREFUSED:
				break;
            case EHOSTDOWN:
				break;
            case EHOSTUNREACH:
				break;
            default:
				break;
        }
		*/
		return -1;
    }
    else
    {
		return count;
    }
}

int
UdpStack::transmitTo( const void* buffer, const int length, const NetworkAddress dest, long millsecond )
{
    if ( (mode==recvonly) || (mode==inactive) )
    {
        return 0;
    }
    assert( buffer );
    assert( length > 0 );

	if ( !isWriteReady( millsecond ) )
		return 0;  
    int count = 0;
	#if defined METRO
	#else
	struct sockaddr_in addrDest;
	addrDest.sin_family = AF_INET;
	addrDest.sin_port = htons( dest.getPort() );
	addrDest.sin_addr.s_addr = dest.getIp4Address();
	sendto( _socketFd, (char*)buffer, length, 0, (struct sockaddr*)&addrDest, sizeof(sockaddr_in) );
	#endif
    if ( count < 0 )
    {
		/*
        int err = errno;
        Data errMsg = "UdpStack transmit err :";
        switch ( err )
        {
		case ECONNREFUSED:
	        // This is the most common error - you get it if the host
		    // does not exist or is nor running a program to recevice
			// the packets. This is what you get with the other side
			// crashes.
			errMsg  += "UdConnection refused by destination host";
			break;
		case EHOSTDOWN:
			errMsg += "destination host is down";
			break;
		case EHOSTUNREACH:
			errMsg += "no route to to destination host";
			break;
		default:
			errMsg += strerror( err );
			break;
        }
		*/
		return -1;
    }
	else
	{
		return count;
    }
}


void
UdpStack::setBlock( bool bBlock )
{
	if ( bBlock )
	{
		#if defined WINDOWS
		#ifdef METRO
		#else
		unsigned long non_blocking = 0;
		ioctlsocket( _socketFd, FIONBIO, &non_blocking );
		#endif
		#else
		int flags;
		if( ( flags = fcntl( _socketFd, F_GETFL, 0 ) ) < 0 )
			return;
		flags &= ~O_NONBLOCK;
		fcntl( _socketFd, F_SETFL, flags );
		#endif
	}
	else
	{
		#if defined WINDOWS
		#ifdef METRO
		#else
		unsigned long non_blocking = 1;
		ioctlsocket( _socketFd, FIONBIO, &non_blocking );
		#endif
		#else
		int flags;
		if( ( flags = fcntl( _socketFd, F_GETFL, 0 ) ) < 0 )
			return;
		flags |= O_NONBLOCK;
		fcntl( _socketFd, F_SETFL, flags );
		#endif
	}
}


bool
UdpStack::isReadReady( int mSeconds ) const
{
#if defined METRO
	return false;
#else
    fd_set rfds;
    struct timeval tv;
	struct timeval *ptv;
    int retval;
	FD_ZERO( &rfds );
	FD_SET( _socketFd, &rfds );
	if ( mSeconds < 0 )
	{
		ptv = NULL;
	}
	else
	{
		ptv = &tv;
		tv.tv_sec = mSeconds/1000;
		tv.tv_usec = ( mSeconds%1000 ) * 1000;
	}
    retval = select( (int)(_socketFd+1), &rfds, NULL, NULL, ptv );
    if ( retval > 0 && FD_ISSET(_socketFd,&rfds) )
        return true;
	else
		return false;
#endif
}

bool
UdpStack::isWriteReady( int mSeconds ) const
{
#if defined METRO
	return false;
#else
    fd_set wfds;
    struct timeval tv;
	struct timeval *ptv;
    int retval;
    FD_ZERO( &wfds );
    FD_SET( _socketFd, &wfds );
	if ( mSeconds < 0 )
	{
		ptv = NULL;
	}
	else
	{
		ptv = &tv;
		tv.tv_sec = mSeconds/1000;
		tv.tv_usec = (mSeconds%1000)*1000;
	}
    retval = select( (int)(_socketFd+1), NULL, &wfds, NULL, ptv );
    if ( retval > 0 && FD_ISSET( _socketFd, &wfds ) )
    {
        return true;
    }
    return false;
#endif
}


UdpStack::~UdpStack()
{
	close();
}
