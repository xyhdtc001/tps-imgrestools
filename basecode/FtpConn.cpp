
#include <map>
#include "basecode/basefunc.h"
#include "basecode/FtpConn.h"
#include "basecode/VFile.h"

FtpConn::FtpConn(void)
{
	_timeout = 5000;
	_bConnected = false;
	m_pace = 10;
	m_callBack = NULL;
}

FtpConn::~FtpConn(void)
{
}


void
FtpConn::setTimeout( long millsec )
{
	_timeout = millsec;
}


bool
FtpConn::connect( const Data& server, short port )
{
	close();
	_clientSock.setServer( server, port );
	if ( !_clientSock.connect() )
		return false;
	_conn = _clientSock.getConn();
	if ( _conn->readLine(_cmdBuff,1024,_timeout)<=0 || _cmdBuff[0]!='2' )
	{
		LogErr( NETWORK, "Ftp conn error server: %s, port %d", server.c_str(), port );
		return false;
	}
	else
	{
		return true;
	}
}


bool FtpConn::ftpCmd( const Data& cmd, char ch )
{
	if ( _conn->writeData(cmd,_timeout)!=(int)cmd.length() )
		return false;
	if ( _conn->readLine(_cmdBuff,1024,_timeout)<=0 )
		return false;
	if ( _cmdBuff[0] != ch )
	{
		LogErr( NETWORK, "FTP CMD ERR %s", _cmdBuff );
		return false;
	}
	return true;
}

bool
FtpConn::login( const Data& user, const Data& pasw )
{
	Data dtCmd = "USER ";
	dtCmd += user;
	dtCmd += CRLF;

	if ( !ftpCmd(dtCmd,'2') )
	{
		if ( _cmdBuff[0] != '3' )
		{
			LogErr( NETWORK, "Ftp User error" );
			return false;
		}
	}

	dtCmd = "PASS ";
	dtCmd += pasw;
	dtCmd += CRLF;
	if ( !ftpCmd(dtCmd,'2') )
	{
		LogErr( NETWORK, "Ftp Pasw error" );
		return false;
	}
	_bConnected = true;
	return true;
}

bool
FtpConn::chdir( const Data& dir )
{
	if ( dir.length()>250 )
	{
		LogErr( NETWORK, "ftp dir too long" );
		return false;
	}
	Data dtCmd = "CWD ";
	dtCmd += dir;
	dtCmd += CRLF;
	if ( !ftpCmd(dtCmd,'2') )
	{
		LogErr( NETWORK, "Ftp Chdir error£º %s", dir.c_str() );
		return false;
	}
	return true;
}

bool
FtpConn::mkdir( const Data& dir )
{
	if ( dir.length() > 250 )
	{
		LogErr( NETWORK, "dir too long" );
		return false;
	}
	Data dtCmd = "MKD ";
	dtCmd += dir;
	dtCmd += CRLF;
	if ( !ftpCmd(dtCmd,'2') )
	{
		LogErr( NETWORK, "Ftp mkdir error£º %s", dir.c_str() );
		return false;
	}
	return true;
}


bool FtpConn::dir()
{
	_fileList.clear();
	if ( !ftpCmd("TYPE A\r\n",'2') )
	{
		LogErr( NETWORK, "Ftp Change mode error" );
		return false;
	}
	TcpClientSocket clientSock;
	if ( !openPort(clientSock) )
		return false;
	if ( !ftpCmd("LIST\r\n",'1') )
	{
		LogErr( NETWORK, "Ftp dir coomand error" );
		return false;
	}
	ref_ptr<Connection> conn = clientSock.getConn();
	int iRet;
	while ( (iRet=conn->readLine(_cmdBuff,1024,_timeout))>=0 )
	{
		LogErr( NETWORK, _cmdBuff );
		std::vector<Data> vecData;
		Data(_cmdBuff).split( " ", vecData );
		FtpFileItem item;
		if ( vecData[0].length() == 8 )
		{
			item.name = vecData[3];
			item.bDir = (vecData[2]=="<DIR>");
			if ( !item.bDir )
				item.fileLen = vecData[2].toInt32();
			else
				item.fileLen = 0;
		}
		else
		{
			FtpFileItem item;
			item.name = vecData[8];
			item.bDir = (vecData[0][(unsigned int)0]=='d');
			item.fileLen = vecData[4].toInt32();
		}
		_fileList.push_back( item );
	}
	if ( _conn->readLine(_cmdBuff,1024,_timeout)<=0 || _cmdBuff[0]!='2' )
	{
		LogErr( NETWORK, "get dir return error" );
		return false;
	}
	return true;
}


FtpFileItem* FtpConn::getFtpItem( const Data& name )
{
	for ( std::list< FtpFileItem >::iterator iter=_fileList.begin(); iter!=_fileList.end(); iter++ )
	{
		if ( iter->name == name )
		{
			return &(*iter);
		}
	}
	return NULL;
}


bool FtpConn::modDate( const Data& path, Data& date )
{
	Data dtCmd = "MDTM ";
	dtCmd += path;
	dtCmd += CRLF;
	if ( !ftpCmd(dtCmd,'2') )
	{
		LogErr( NETWORK, "ModeDate %s err", path.c_str() );
		return false;
	}
	else
	{
		date = (char*)(_cmdBuff+4);
	}
	return true;
}


bool FtpConn::openPort( TcpClientSocket& sock )
{
	if ( !ftpCmd("PASV\r\n",'2') )
	{
		LogErr( NETWORK, "Set Pasv mode error" );
		return false;
	}

	struct sockaddr sa;
	memset( &sa, 0, sizeof(sa) );
	sa.sa_family = AF_INET;

	char* cp = NULL;
	cp = strchr( _cmdBuff, '(' );
	if ( cp == NULL )
	{
		LogErr( NETWORK, "Set Pasv res error" );
	    return false;
	}
	cp++;
	unsigned int v[6];
	sscanf( cp,"%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1] );
	sa.sa_data[2] = v[2];
	sa.sa_data[3] = v[3];
	sa.sa_data[4] = v[4];
	sa.sa_data[5] = v[5];
	sa.sa_data[0] = v[0];
	sa.sa_data[1] = v[1];

	sock.setServer( sa );
	if ( !sock.connect() )
		return false;
	return true;
}


bool FtpConn::getFile( const Data& localFile, const Data& remoteFile )
{
	if ( !ftpCmd("TYPE I\r\n",'2') )
	{
		LogErr( NETWORK, "Ftp Change mode error" );
		return false;
	}
	
	TcpClientSocket clientSock;
	if ( !openPort(clientSock) )
		return false;

	Data dtCmd = "RETR ";
	dtCmd += remoteFile;
	dtCmd += CRLF;
	
	if ( !ftpCmd(dtCmd,'1') )
		return false;

	ref_ptr<Connection> conn = clientSock.getConn();
	unsigned char buff[MTU];
	VFile file;
	if ( !file.openFile(localFile,VFile::READWRITE) )
		return false;

	int iReaded = 0;
	int iRet;
	while ( (iRet=conn->readn(buff,MTU,_timeout)) > 0 )
	{
		if ( file.write(buff,iReaded) != iReaded )
		{
			LogErr( NETWORK, "write file err" );
			return false;
		}
	}
	if ( file.write(buff,iReaded) != iReaded )
	{
		LogErr( NETWORK, "write file err" );
		return false;
	}
	if ( _conn->readLine(_cmdBuff,1024,_timeout)<=0 || _cmdBuff[0]!='2' )
	{
		LogErr( NETWORK, "Get File End error£º %s", remoteFile.c_str() );
		return false;
	}
	return true;
}

void FtpConn::setNotifyPercent( unsigned int percent )
{
	if ( percent > 100 )
		return;
	m_pace = percent;
}


bool FtpConn::putFile(const Data &localFile, const Data &remoteFile)
{
	if ( !ftpCmd("TYPE I\r\n",'2') )
	{
		LogErr( NETWORK, "Ftp Change mode error" );
		return false;
	}
	TcpClientSocket clientSock;
	if ( !openPort( clientSock ) )
		return false;

	Data dtCmd = "STOR ";
	dtCmd += remoteFile;
	dtCmd += CRLF;
	
	if ( !ftpCmd(dtCmd,'1') )
	{
		LogErr( NETWORK, "Get File error£º %s", remoteFile.c_str() );
		return false;
	}

	ref_ptr<Connection> conn = clientSock.getConn();
	unsigned char buff[MTU];
	VFile file;
	if ( !file.openFile(localFile,VFile::READ) )
		return false;

	int fileLen = file.getFileLen();
	int iRead;
	int iReaded = 0;
	int gap = fileLen / m_pace;
	int iNotifyPos = gap;
	int pos = 1;
	while ( (iRead=file.read(buff,MTU)) > 0  )
	{
		if ( conn->writeData(buff,iRead,_timeout) != iRead )
		{
			LogErr( NETWORK, "WRITE DATA ERR" );
			return false;
		}
		if ( m_callBack != NULL )
		{
			iReaded += iRead;
			if ( iReaded > iNotifyPos )
			{
				iNotifyPos = gap * ++pos;
				m_callBack( (int)((float)iReaded/(float)fileLen*100), m_param );
			}
		}
	}
	if ( m_callBack != NULL )
		m_callBack( 100, m_param );
	clientSock.close();
	if ( _conn->readLine(_cmdBuff,1024,_timeout)<=0 || _cmdBuff[0]!='2' )
	{
		LogErr( NETWORK, "Get File End error£º %s", remoteFile.c_str() );
		return false;
	}
	return true;
}

void FtpConn::close()
{
	_clientSock.close();
	_bConnected = false;
}

bool FtpConn::isConnected()
{
	return _bConnected;
}

void FtpConn::setCallbackFunc( FTP_TRANS_CALLBACK func, void* param )
{
	m_callBack = func;
	m_param = param;
}
