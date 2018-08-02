// VHttp.h: interface for the VHttp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined VHTTP_H__
#define VHTTP_H__

#include <map>

#include "basecode/TcpClientSocket.h"
#include "basecode/stringdata.h"
#include "basecode/memstream.h"

class IHttpCallback
{
public:
	virtual void progress( uint32 percent ) = 0;
};


class VHttp  
{
public:
	VHttp();
	virtual ~VHttp();

	static Data UrlEncode( const Data& url );
	int GetHttpResponseData( const Data& fullUrl, char* pData, int len, bool bGet=true, bool bGetResp=true );
	int GetHttpResponseData( const Data& ip, const Data& url, char* pData,
								int len, bool bGet = true, bool bGetResp = true );

    void setHttpProxy( const Data& proxy );
	void setCookie( Data cookie );
	int getHttpFile( const Data& fullUrl, const Data& savefile, int startpos=0 );
	int getHttpFile( const Data& server, const Data& httpfile, const Data& savefile, int startpos=0 );
	ref_ptr<MemStream> getHttpStream( const Data& server, const Data& httpfile, int32 len=-1, uint32 retryNum=1 );
	ref_ptr<MemStream> getHttpFileStream( const Data& fullUrl );
	ref_ptr<MemStream> getHttpFileStream( const Data& server, const Data& httpfile );
	ref_ptr<MemStream> getHttpStream( const Data& file, uint32 retryNum=1 );
	void setNotifyCallback( IHttpCallback* callback );
	void clear();
	void setUserAgent( const Data& agent );
	void debug();
	Data getRespFieldValue( const Data& field );

	void processCallback();
	uint32 getSpeed();

	int getWriteLen();
	int getFileLen();
	void enableSpeedCheck();
	void close();

protected:
	IHttpCallback* _callback;
	TcpClientSocket _clientSock;
	Data	m_dtCookie;
	Data	m_dtUserAgent;
    Data    m_dtHttpProxy;
	uint32	m_iNotifyPos;
	uint32	m_iWriteLen;
	uint32	m_iLength;
	uint32	m_iStatusCode;
	uint32	_lastTime;
	uint32	_iWrited;
	uint32	_speed;
	bool	_bCheckSpeed;
	bool _bClose;

public:
	void clearParam();
	void addParam( const char* name, const char* val );
	std::map<Data,Data> _paramMap;
	std::map<Data,Data> _respHeadMap;
};

#endif
