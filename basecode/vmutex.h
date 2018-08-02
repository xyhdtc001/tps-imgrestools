#pragma  once

#include "windows.h"
#include "basecode/basehdr.h"

class VMutex
{
public:
	VMutex()
	{
        #ifdef WINDOWS
		#ifdef METRO
		#else
		InitializeCriticalSection( &_critSec );
		#endif
        #endif
	}

	void lock() const
	{
        #ifdef WINDOWS
		#ifdef METRO
		#else
		EnterCriticalSection( &_critSec );
		#endif
        #endif
	}

	void unlock() const
	{
        #ifdef WINDOWS
		LeaveCriticalSection( &_critSec );
        #endif
	}
	
	~VMutex()
	{
        #ifdef WINDOWS
		DeleteCriticalSection( &_critSec );
        #endif
	}

protected:
#ifdef WINDOWS
	mutable CRITICAL_SECTION _critSec ;
#endif
};