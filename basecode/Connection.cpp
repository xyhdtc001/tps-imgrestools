#ifdef WINDOWS
#else
#ifdef __ANDROID__
#include <sys/endian.h>
#endif
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#endif

#include "basecode/BaseLog.h"
#include "basecode/Connection.h"

int32
Connection::readLine( void* dataRead, int maxlen, int timeout )
{
	memset( dataRead, 0, maxlen );
	char c;
	int rc, i;
	char* ptr = (char*)dataRead;
	for( i = 1; i<maxlen; i++ )
    {
		rc = readn( &c, 1, timeout );
        if ( rc == 1 )
        {
            *ptr++ = c;
            if ( c=='\n' )
			{
				i--;
				ptr--;
				if ( *(ptr-1) == '\r' )
				{ i--;ptr--; }
				break;
			}
        }
        else if ( rc == 0 )
        {
            log_out( MAIN, LOG_ERR, "read line err" );	//can't not go here
			*ptr = 0;
			return -1;
        }
        else
        {
            return -2;
        }
    }
    *ptr = 0;
    return i;
}

int32
Connection::readData( void *dataRead, int nchar, int timeout )
{
    int nleft;
    int nread;
    char *ptr = (char*)dataRead;
    nleft = nchar;
	int bytesRead = 0;
    while ( nleft > 0 )
    {
		nread = readn( ptr,nleft,timeout );
        if ( nread < 0 )
			return -1;
		else if ( nread == 0 )
			return bytesRead;
        nleft -= nread;
        ptr += nread;
        bytesRead += nread;
    }
    return bytesRead;
}

int32
Connection::readn( void *data, int len, int timeout )
{
	if ( _connId == 0 )
		return -1;
	#if defined WINDOWS
	#if defined METRO
	task<unsigned int> t( _reader->LoadAsync(len) );
	try
	{
		unsigned int readLen = t.get();
		Platform::Array<unsigned char>^ readArray = ref new Platform::Array<unsigned char>( (unsigned char*)data, readLen );
		this->_reader->ReadBytes( readArray );
		return readLen;
	}
	catch( Platform::Exception^ e )
	{
		return -1;
	}
	return 0;
	#else
	int iRead = 0;
	int msec = timeout;
	if ( !isReadReady( msec ) )
		return 0;
	iRead = recv( _connId, (char*)data, len, 0 );
	if (  iRead == 0 )
	{
		_error_code = 0;
		return -1;
	}
	else if ( iRead < 0 )
	{
		_error_code = WSAGetLastError();
		if ( _error_code == WSAEWOULDBLOCK )
			return 0;
		LogOut( MAIN, LOG_ERR, "WSAGetLastError:%d", _error_code );
		return -1;
	}
	else
	{
		return iRead;
	}
	#endif
	#else
	int iRead = 0;
	int msec = timeout;
	if ( !isReadReady( msec ) )
		return 0;
	iRead = ::read( _connId, data, len );
	if ( iRead <= 0 )
		return -1;
	else
		return iRead;
	#endif
}


int32
Connection::readIfHasData( void* data, int nchar, int& bytesRead, int timeout )
{
	int nleft;
    int nread;
    char *ptr = (char*)data;
    nleft = nchar;
	bytesRead = 0;
	int tout = 0;
    do
    {
        if ( ( nread = readn( ptr, nleft, tout ) ) <= 0 )
		{
			return nread;
		}
		else
		{
			tout = timeout;
		}
        nleft -= nread;
        ptr += nread;
        bytesRead += nread;
    } while ( nleft > 0 );
	return nread;
}

int32
Connection::writeData( const Data& data, int timeout )
{
	int len = data.length();
	return writeData( data.c_str(), len, timeout );
}


int32 Connection::writen( void* data, uint32 len )
{
	if ( _connId == 0 )
		return -1;
	return ::send( _connId, (const char*)data, len, 0 );
}


int32
Connection::writeData( const void* data, int n, int timeout )
{
	if ( _connId == 0 )
		return -1;
	int32 msec = timeout;
    int32 nleft;
    int32 nwritten = 0;
    char *ptr = (char*)data;
    nleft = n;
    while ( nleft > 0 )
    {
	#if defined WINDOWS
	#if defined METRO
		Platform::Array<unsigned char>^ writeArray = ref new Platform::Array<unsigned char>( (unsigned char*)ptr, nleft );
		_writer->WriteBytes( writeArray );
		task<unsigned int> t( _writer->StoreAsync() );
		try
		{
			nwritten = (int32)t.get();
			if ( nwritten == 0 )
				nwritten = -1;
		}
		catch( Platform::Exception^ e )
		{
			nwritten = -1;
		}
		return 0;
	#else
		if ( !isWriteReady( msec ) )
			return (int32)(ptr-(char*)data);
		nwritten = ::send( _connId, ptr, nleft, 0 );
	#endif
	#else
		if ( !isWriteReady( msec ) )
			return (int32)(ptr-(char*)data);
        try {
            nwritten = ::write( _connId, ptr, nleft );
        } catch (...) {
            printf("write eroor");
            return -1;
        }
		//nwritten = ::write( _connId, ptr, nleft );
	#endif
		if ( nwritten < 0 )
        {
			if ( ptr != data )
			{
				return (int32)(ptr-(char*)data);
			}
			else
			{
				return -1;
			}
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
	return (int32)(ptr-(char*)data);
}



#ifdef WINDOWS
#define snprintf _snprintf
#endif

Data
Connection::getDescription() const
{
#if defined METRO
	return "";
#else
    Data retStr;
    sockaddr* sa = (sockaddr*) & _connAddr;
    const u_char* p = (const u_char*)&_connAddr.sin_addr;
    switch ( sa->sa_family )
    {
    case AF_INET:
        char temp[256];
        snprintf( temp, sizeof(temp), "%d.%d.%d.%d", p[0], p[1], p[2], p[3] );
        char pStr[56];
        sprintf( pStr, ":%d", ntohs(_connAddr.sin_port) );
        retStr = temp;
        retStr += pStr;
		return ( retStr );
        break;
	default:
        return "Unknown";
        break;
    }
    return ("Unknown");
#endif
}

Data
Connection::getIp() const
{
#if defined METRO
	return "";
#else
    Data retStr;
    sockaddr* sa = (sockaddr*)&_connAddr;
    const u_char* p = (const u_char*)&_connAddr.sin_addr;
    switch ( sa->sa_family )
    {
	case AF_INET:
        char temp[256];
        snprintf( temp, sizeof(temp), "%d.%d.%d.%d",
					p[0], p[1], p[2], p[3] );
        retStr = temp;
        return retStr;
        break;
	default:
        return "Unknown";
        break;
    }
    return "Unknown";
#endif
}

unsigned short
Connection::getPort() const
{
#if defined METRO
	return 0;
#else
	return ntohs( _connAddr.sin_port );
#endif
}

Connection::~Connection()
{
	close();
}

void
Connection::close()
{
	if ( _connId != 0 )
#if defined WINDOWS
#if defined METRO
	
#else
	closesocket( _connId );
#endif
#else
	::close( _connId );
#endif
	_connId = 0;
}


bool
Connection::setBlocking( bool block )
{	
	_blocking = block;
#if defined WINDOWS
#if defined METRO
	return false;
#else
	unsigned long non_blocking = _blocking ? 0 : 1;
	if ( ioctlsocket( _connId, FIONBIO, &non_blocking ) == SOCKET_ERROR )
	{
		log_out( MAIN, LOG_ERR, "Error setting Connection FIONBIO: %s", strerror(errno) );
		return false;
	}
#endif
#else
	int non_blocking = _blocking ? 0 : 1;
	if ( ioctl( _connId, FIONBIO, &non_blocking ) < 0 )
	{
		LogOut( MAIN, LOG_ERR, "Error setting Connection FIONBIO: %s", strerror(errno) );	
	}
#endif
	return true;
}


bool
Connection::isReadReady( int mSeconds ) const
{
#if defined METRO
	return false;
#else
    fd_set rfds;
	fd_set efds;
    struct timeval tv;
	struct timeval *ptv;
    int retval;
	FD_ZERO( &rfds );
	FD_ZERO( &efds );
	FD_SET( _connId, &rfds );
	FD_SET( _connId, &efds );
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
    retval = select( (int)(_connId+1), &rfds, NULL, &efds, ptv );
    if ( retval>0 && FD_ISSET(_connId,&rfds) )
    {
        return true;
    }
    return false;
#endif
}

bool
Connection::isWriteReady( int mSeconds ) const
{
#if defined METRO
	return false;
#else
    fd_set wfds;
	fd_set efds;
    struct timeval tv;
	struct timeval *ptv;
    int retval;
    FD_ZERO( &wfds );
	FD_ZERO( &efds );
    FD_SET( _connId, &wfds );
	FD_SET( _connId, &efds );
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
    retval = select( (int)(_connId+1), NULL, &wfds, &efds, ptv );
    if ( retval>0 && FD_ISSET(_connId,&wfds) )
        return true;
    return false;
#endif
}

int32 Connection::read( void *dataRead, int nchar )
{
	if  ( _connId == 0 )
		return -1;
#if defined WINDOWS
#if defined METRO
	return 0;
#else
	return recv( _connId, (char*)dataRead, nchar, 0 );
#endif
#else
	return ::read( _connId, dataRead, nchar );
#endif
}
