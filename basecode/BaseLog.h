#if  !defined LOG_OUT_H__
#define LOG_OUT_H__

#ifdef _DEBUG
	#if !defined LOG_SUPPORT
	#define LOG_SUPPORT
	#endif
#endif

enum LOG_SYSTEM
{
	MAIN = 0,
	GRAPHIC,
	ROLE,
	ANIMATION,
	SCRIPT,
	NETWORK,
	PHYX,
	PLUGIN,
	RES,
	GUI,
	SOUND,
	MAX_LOG_SYSTEM,
};

#define LogOut log_out

//#define LOG_SUPPORT
#if defined LOG_SUPPORT
#define LogDebug log_debug
#else 
#define LogDebug(...)
#endif

#define LogErr log_err
#define LogFatal log_fatal
#define LogFatalOnce(TYPE,STR)		{static bool bFatalReport = false;		\
									if ( !bFatalReport )					\
									{										\
										LogFatal( TYPE, STR );				\
										bFatalReport = true;				\
									}}

#define LogFatalOnceWithOpenid(TYPE,STR,ID)		{static bool bFatalReport = false;		\
												if ( !bFatalReport )					\
												{										\
													LogFatal( TYPE, STR, ID );			\
													bFatalReport = true;				\
												}}

#define LogFatalErrOnceWithOpenid(TYPE,STR,ID)		{static bool bFatalReport = false;		\
													if ( !bFatalReport )					\
													{										\
														LogFatal( TYPE, STR, ID );			\
														LogErr( TYPE, STR, ID );			\
														bFatalReport = true;				\
													}}


enum LOG_TYPE
{
	LOG_FATAL=0,		// 表示输出的日志是一个导致系统崩溃严重错误 
	LOG_ERR,			// 表示输出的日志是一个系统错误 
	LOG_WARN,			// 表示输出的日志是一个警告信息 
	LOG_DEBUG,			// 表示输出的日志为一个调试信息 
	LOG_INFO,			// 表示输出的日志是一个系统提示 
	MAX_LOG_TYPE,
};

void log_out( LOG_SYSTEM sys, LOG_TYPE pri, const char* fmt, ... );
void log_debug( LOG_SYSTEM sys, const char* fmt, ... );
void log_err( LOG_SYSTEM sys, const char* fmt, ... );
void log_info( LOG_SYSTEM sys, const char* fmt, ... );
void log_fatal( LOG_SYSTEM sys, const char* fmt, ... );

typedef void ( *LogOutFunc )( LOG_SYSTEM sys, LOG_TYPE type, char* buff );

extern const char* logSysNames[];
extern const char* logTypeNames[];

extern LogOutFunc g_logFunc;

/*PLATFORM_API*/ void SetLogOutFunc( LogOutFunc func );

#endif