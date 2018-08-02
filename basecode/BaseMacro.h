#ifndef _BASE_MACRO_H_
#define _BASE_MACRO_H_

#include "basecode/BaseLog.h"

#undef OUT
#define OUT

#undef IN_OUT
#define IN_OUT

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p) delete p; p=NULL; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) delete[] (p); (p)=NULL; }
#endif

#undef SAFE_RELEASE
#define SAFE_RELEASE(ptr)	{ if(ptr){ ptr->Release(); ptr=NULL; } }

#undef	SAFE_FREE
#define SAFE_FREE(ptr)		{ if(ptr){ free(ptr); ptr = 0; } }

void AssertMsg(bool bOK,const char* pszExpr,const char* pszFile,int nLine,const char* fmt,...);

#if defined(_DEBUG) || defined (DEBUG) 
	#undef ASSERT
	#define ASSERT(x, ...) {bool bOK = (x); if(!bOK)LogOut(MAIN,LOG_ERR,"ASSS %s %d",__FILE__,__LINE__);assert(bOK);}
#endif

#ifndef VERIFY
	#if defined(_DEBUG) || defined(DEBUG)
		#define VERIFY(f) ASSERT(f)
	#else
		#define VERIFY(f) ((void)(f))
	#endif
#endif

#define BIT(x) (1 << (x)) 

#define TIME_RECORDER_START(name) \
	LARGE_INTEGER n64TimeStart##name;	\
	LARGE_INTEGER n64TimeEnd##name;	\
	LARGE_INTEGER litmp##name;	\
	double dfFreq##name = 0.0f;	\
	DWORD dwTimeStart##name = CStaticFunc::TimeGet();	\
	QueryPerformanceFrequency(&litmp##name);	\
	dfFreq##name = litmp##name.QuadPart;	\
	QueryPerformanceCounter(&n64TimeStart##name);	\
	dwTimeStart##name = CStaticFunc::TimeGet();	\

#define TIME_RECORDER_END(name) \
	QueryPerformanceCounter(&n64TimeEnd##name);	\
	double fTimeUse##name = (n64TimeEnd##name.LowPart - n64TimeStart##name.LowPart)  / dfFreq##name * 1000;	\
	DWORD dwTimePass##name = CStaticFunc::TimeGet() - dwTimeStart##name;	\
	LogOut( MAIN, DEBUG, #name##" time use:%.5f :%d", fTimeUse##name, dwTimePass##name );	\

#define BEGIN_TIME(name)\
	INT64 nFrequency##name, nBeginTime##name, nEndTime##name;\
	QueryPerformanceFrequency((LARGE_INTEGER *)&nFrequency##name);\
	QueryPerformanceCounter((LARGE_INTEGER *)&nBeginTime##name);

#define END_TIME(name)\
	QueryPerformanceCounter((LARGE_INTEGER *)&nEndTime##name);\
	double dCostTime##name = 1000.0*(nEndTime##name - nBeginTime##name)/(double)nFrequency##name;\
	static char tmp##name[256]={0};\
	_snprintf(tmp##name, 255, #name##" time use:%.5f ms\n", dCostTime##name );\
	::OutputDebugStr(tmp##name);
#endif


#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }
#define V(x)           { hr = (x); }
