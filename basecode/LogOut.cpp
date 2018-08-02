#include "basecode/TimeVal.h"
#include "basecode/IniFile.h"
#include "basecode/str.h"
#include "basecode/LogOut.h"
#include "basecode/directory.h"

using namespace tq;

ref_ptr< CLogOut > g_logOutItems[MAX_LOG_SYSTEM];
bool g_bFileLogOut = false;
bool g_bUdpLogOut = false;
bool g_bUdpReportOut = false;
VFile g_logFile;
UdpStack g_logUdpStack;
UdpStack g_reportUdpStack;

CLogOut::CLogOut()
{
	for ( uint32 i=0; i<MAX_LOG_TYPE; i++ )
		_outputType[i] = 0;

}

void CLogOut::logOut( LOG_SYSTEM sys, LOG_TYPE type, char* buff )
{
	TimeVal timeVal;
	timeVal.now();
	Data dtTimeStr = timeVal.strftime( "[%H:%M:%S " );
	if ( type < _startType )
		return;
	//char title[32] = {0,};
	Data dtMillsec = Data::ToString( (int)(timeVal.milliseconds()%1000) );
	//dtMillsec.inflate( 3, "0" );
	Data dtLog = dtTimeStr + dtMillsec + "] " + logSysNames[sys] + "(" + logTypeNames[type] + "): " + buff + CRLF;
	if ( _outputType[type] & LOG_OUTPUT_FILE )
	{
		if ( g_bFileLogOut )
			g_logFile.write( dtLog );
	}
	if ( _outputType[type] & LOG_OUTPUT_LOGSERVER )
	{
		if ( g_bUdpLogOut )
		{
			char szBuff[2048];
			LogPack *pLogPack = (LogPack*)szBuff;
			pLogPack->hdr.lLogMsgSize = dtLog.length();
			pLogPack->hdr.sys = sys;
			pLogPack->hdr.type = type;
			strcpy( pLogPack->strLogMsg, dtLog.c_str() );
			g_logUdpStack.transmit( szBuff, sizeof(LOG_HDR)+dtLog.length() );
		}
	}
	if ( _outputType[type] & LOG_OUTPUT_REPORTSERVER )
	{
		if ( g_bUdpReportOut )
		{
			char szBuff[2048];
			LogPack *pLogPack = (LogPack*)szBuff;
			pLogPack->hdr.lLogMsgSize = dtLog.length();
			pLogPack->hdr.sys = sys;
			pLogPack->hdr.type = type;
			strcpy( pLogPack->strLogMsg, dtLog.c_str() );
			g_reportUdpStack.transmit( szBuff, sizeof(LOG_HDR)+dtLog.length() );
		}
	}
}

void CLogOut::initLogOutByField( IniField* pField )
{
	String dtLogType = pField->getItemStr( "LEVEL" );
	dtLogType.removeSpaces();
	LOG_TYPE type = (LOG_TYPE)0;
	for ( int i=0; i<MAX_LOG_TYPE; i++ )
	{
		if ( dtLogType.equal(logTypeNames[i],String::NoCase)  )
		{
			type = (LOG_TYPE)i;
			break;
		}
	}
	String dtData = pField->getItemStr( "output" );
	initLogOutBySting( type, dtData.c_str() );
}

void CLogOut::initLogOutBySting( LOG_TYPE type, const char* str )
{
	_startType = type;
	Data dtStr = str;
	vector<Data> vecData;
	dtStr.removeSpaces();
	dtStr.split( ",", vecData );
	for ( vector<Data>::iterator iter=vecData.begin(); iter!=vecData.end(); iter++ )
	{
		iter->removeSpaces();
		if ( isEqualNoCase(*iter,"FILE") )
		{
			_outputType[type] |= LOG_OUTPUT_FILE;
		}
		if ( isEqualNoCase(*iter,"LOGSERVER") )
		{
			_outputType[type] |= LOG_OUTPUT_LOGSERVER;
		}
		if ( isEqualNoCase(*iter,"REPORTSERVER") )
		{
			_outputType[type] |= LOG_OUTPUT_REPORTSERVER;
		}
	}
}

void CLogOut::setLogType( LOG_TYPE type )
{
	_startType = type;
};

void set_log( LOG_SYSTEM sys, LOG_TYPE type, const Data& str )
{
	ref_ptr<CLogOut> spLogout = new CLogOut();
	spLogout->initLogOutBySting( type, str.c_str() );
	g_logOutItems[sys] = spLogout;
}

void set_sys_log( LOG_TYPE type, const Data& str )
{
	for ( uint32 i=MAIN; i<MAX_LOG_SYSTEM; i++ )
	{
		ref_ptr<CLogOut> spLogout = g_logOutItems[i];
		if ( spLogout == NULL )
			spLogout = new CLogOut();
		spLogout->initLogOutBySting( (LOG_TYPE)type, str.c_str() );
		g_logOutItems[i] = spLogout;
	}	
}

void log_system_out( LOG_SYSTEM sys, LOG_TYPE type, char* buff )
{
	CLogOut* spLogOut = g_logOutItems[sys].get();
	if ( spLogOut == NULL )
		return;
	spLogOut->logOut( sys, type, buff );
}

void log_init( const char* server, const char* path, const char* reportServer )
{
	#if defined WINDOWS
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD( 2, 2 );
	if ( WSAStartup(wVersionRequested,&wsaData) != 0 )
		return;
	#endif

	SetLogOutFunc( log_system_out );
	if ( path != NULL )
	{
		if ( VDirectory::createFileDir(path) )
		{
			if ( VFile::getFileLen(path) > 1000000 )
			{
				if ( g_logFile.openFile(path,VFile::READWRITE) )
					g_bFileLogOut = true;
			}
			else
			{
				if ( g_logFile.openFile(path,VFile::APPEND) )
					g_bFileLogOut = true;
			}
		}
	}
	if ( server != NULL )
	{
		if ( g_logUdpStack.init(server,2323) )
			g_bUdpLogOut = true;
	}
	if ( reportServer != NULL )
	{
		if ( g_reportUdpStack.init(reportServer,2323) )
			g_bUdpReportOut = true;
	}
}

void log_uinit()
{
	g_logFile.closeFile();
}
