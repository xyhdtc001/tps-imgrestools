#ifndef Connection_hxx
#define Connection_hxx

#include "basecode/basehdr.h"
#include "basecode/basefunc_metro.h"

#ifdef WINDOWS
#ifdef METRO
using namespace Windows::Networking::Sockets;
using namespace Windows::Networking;
using namespace Windows::Storage::Streams;
#else
#include "windows.h"
#include "winsock.h"
#endif
#else
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/types.h>
#endif

#include "basecode/BaseType.h"
#include "basecode/Referenced.h"
#include "basecode/StringData.h"

class TcpServerSocket;
class TcpClientSocket;

class Connection : public CReferenced
{
public:
	Connection()
	{
		_blocking = true;
		_connId = 0;
		_error_code = 0;
	}
	
	void setConnId( int connid )
	{
		_connId = connid;
	}

	int32 getErrorCode()
	{
		return _error_code;
	}

	#ifndef METRO
	inline SOCKET getConnId() const
	{
		return _connId;
	}
	#endif

	#if !defined METRO
	inline struct sockaddr_in getConnAddr() const
	{
		return _connAddr;
	}
	#endif

	virtual ~Connection();
	
	int32 readLine( void* data, int maxlen, int timeout=-1 );

	int32 read( void *dataRead, int nchar );
	
	/**Reads nchar from the connection.
	Returns 0 if timeout
	Returns -1 if connection is closed */
	int32 readData( void* data, int nchar, int timeout = -1 );

	int32 readn( void *data, int len, int timeout=-1 );

	int32 readIfHasData( void* data, int nchar, int& bytesRead, int timeout );
	
	/** Writes data to the connection
	Returns 0 if timeout
	Returns 1 if connection is closed **/
	int32 writeData( const void* data, int n, int timeout=-1 );
	
	int32 writeData( const std::string& data, int timeout=-1 );

	int32 writeData( const Data& data, int timeout=-1 );

	int32 writen( void* data, uint32 len );
	
	//Gets the connection description IP_ADDRESSS:Port
	Data getDescription() const;
	
	///Gets the IP of the destination 
	Data getIp() const;

	unsigned short getPort() const;
	
	void close();
	
	///Check if data is ready to be read within given seconds and microseconds
	bool isReadReady( int mSeconds=-1 ) const;
	bool isWriteReady( int mSeconds=500 ) const;
	bool setBlocking( bool block=true );
	
private:
	int _timeout;
	friend class TcpServerSocket;
	friend class TcpClientSocket;

	#ifdef METRO
	uint32 _connId;
	DataReader^ _reader;
	DataWriter^ _writer;
	#else
	volatile SOCKET _connId;
	struct sockaddr_in _connAddr;
	#endif

	bool _blocking;

	int32 _error_code;
};

#endif
