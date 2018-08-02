#include "assert.h"
#include "basecode/VNameMutex.h"

VNameMutex::VNameMutex()
{
	#if !defined METRO
	_hMutex = NULL;
	#endif
}

void VNameMutex::create( const char* name )
{
#ifdef WINDOWS
#if defined METRO
#else
	_hMutex = CreateMutex( NULL, FALSE, name );
#endif
#endif
}

VNameMutex::~VNameMutex()
{
#if defined WINDOWS
#if defined METRO
	if ( _hMutex )
		CloseHandle( _hMutex );
#endif
#else
#endif
}

VNameMutex::WAIT_RESULT VNameMutex::wait( uint32 timeout )
{
#ifdef WINDOWS
#ifdef METRO
#else
	DWORD res = WaitForSingleObject(_hMutex, timeout);
	switch( res )
	{
	case WAIT_OBJECT_0:
		return WAIT_RES_SIGNAL;
		break;
	case WAIT_TIMEOUT:
		return WAIT_RES_TIMEOUT;
		break;
	default:
		break;
	}
#endif
#else
    assert(false);
#endif
	return WAIT_RES_FAIL;
}

void VNameMutex::release()
{
#ifdef WINDOWS
	ReleaseMutex( _hMutex );
#else
    assert(false);
#endif
}
