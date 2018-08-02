#pragma once

class VConditoin
{
public:
	VConditoin()
	{
		#ifdef WINDOWS
		#ifndef METRO
		_event = CreateEvent( NULL, FALSE, FALSE, NULL );
		#endif
		#endif
	}

	~VConditoin()
	{
		#ifdef WINDOWS
		#ifndef METRO
		CloseHandle( _event );
		#endif
		#endif
	}

	void wait()
	{
		#ifdef WINDOWS
		#ifndef METRO
		WaitForSingleObject( _event, INFINITE );
		#endif
		#endif
	}

	void signal()
	{
		#ifdef WINDOWS
		#ifndef METRO
		SetEvent( _event );
		#endif
		#endif
	}

protected:
	HANDLE _event;
};
