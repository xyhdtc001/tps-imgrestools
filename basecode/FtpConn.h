#if !defined FTP_CONN_H__
#define FTP_CONN_H__

#include <list>
#include "basecode/StringData.h"
#include "basecode/TcpClientSocket.h"

typedef void (*FTP_TRANS_CALLBACK)( unsigned int progress, void* param );

struct FtpFileItem
{
	Data name;
	bool bDir;
	int fileLen;
};

class FtpConn
{
public:
	FtpConn(void);
	~FtpConn(void);
	bool connect( const Data& server, short port );
	bool login( const Data& user, const Data& pasw );
	bool mkdir( const Data& dir );
	bool chdir( const Data& dir );
	bool ftpCmd( const Data& cmd, char ch );
	void setTimeout( long millsec );
	bool getFile( const Data& localFile, const Data& remoteFile );
	bool putFile(const Data &localFile, const Data &remoteFile);
	bool openPort( TcpClientSocket& sock );
	bool modDate( const Data& path, Data& date );
	bool dir();
	void close();
	bool isConnected();
	void setCallbackFunc( FTP_TRANS_CALLBACK func, void* param );
	void setNotifyPercent( unsigned int percent );
	FtpFileItem* getFtpItem( const Data& name );

public:
	std::list< FtpFileItem > _fileList;

private:
	TcpClientSocket _clientSock;
	ref_ptr<Connection> _conn;
	char _cmdBuff[ 1024 ];
	long _timeout;
	bool _bConnected;
	unsigned int m_pace;
	FTP_TRANS_CALLBACK m_callBack;
	void *m_param;
};

#endif
