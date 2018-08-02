#include "basecode/BaseLog.h"

#include "stdio.h"
#include "stdarg.h"
#include "Windows.h"
#include <string.h>

LogOutFunc g_logFunc;

struct  LOG_SYSTEM_ITEM
{
	LOG_SYSTEM sys;
	char* name;
};

const char* logSysNames[] = {		"MAIN",			\
							"GRAPHIC",		\
							"ROLE",			\
							"ANIMATION",	\
							"SCRIPT",		\
							"NETWORK",		\
							"PHYX",			\
							"PLUGIN",		\
							"RES",			\
							"GUI",			\
							"SOUND",		\
};


const char* logTypeNames[] = {	"FATAL",		\
							"ERR",			\
							"WARN",			\
							"DEBUG",		\
							"INFO",			\
};

void log_out( LOG_SYSTEM sys, LOG_TYPE pri, const char* fmt, ... )
{
	if(g_logFunc == NULL || fmt == NULL)
		return;

	char buff[4096];
	memset( buff, 0, 4096 );

	va_list va_alist;
	va_start( va_alist, fmt );
	vsprintf( buff, fmt, va_alist );
	va_end( va_alist );

	g_logFunc( sys, pri, buff );
}

void log_debug( LOG_SYSTEM sys, const char* fmt, ... )
{
	if ( g_logFunc == NULL || fmt == NULL )
		return;

	char buff[2048];
	memset( buff, 0, 2048 );

	va_list va_alist;
	va_start( va_alist, fmt );
	vsprintf( buff, fmt, va_alist );
	va_end( va_alist );

	g_logFunc( sys, LOG_DEBUG, buff );
}


void log_err( LOG_SYSTEM sys, const char* fmt, ... )
{
	if ( g_logFunc == NULL || fmt == NULL )
		return;

	char buff[2048];
	memset( buff, 0, 2048 );

	va_list va_alist;
	va_start( va_alist, fmt );
	vsprintf( buff, fmt, va_alist );
	va_end( va_alist );

	g_logFunc( sys, LOG_ERR, buff );
}


void log_fatal( LOG_SYSTEM sys, const char* fmt, ... )
{
	if ( g_logFunc == NULL || fmt == NULL )
		return;

	char buff[2048];
	memset( buff, 0, 2048 );

	va_list va_alist;
	va_start( va_alist, fmt );
	vsprintf( buff, fmt, va_alist );
	va_end( va_alist );

	g_logFunc( sys, LOG_FATAL, buff );
}

void log_info( LOG_SYSTEM sys, const char* fmt, ... )
{
	if ( g_logFunc == NULL || fmt == NULL )
		return;

	char buff[2048];
	memset( buff, 0, 2048 );

	va_list va_alist;
	va_start( va_alist, fmt );
	vsprintf( buff, fmt, va_alist );
	va_end( va_alist );

	g_logFunc( sys, LOG_INFO, buff );
}



void SetLogOutFunc( LogOutFunc func )
{
	g_logFunc = func;
}

