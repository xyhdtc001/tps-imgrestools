#pragma once

#include "basecode/vmutex.h"

class LockHelper
{
public:
	LockHelper( const VMutex* mutex )
	{
		_mutex = mutex;
		mutex->lock();
	}
	~LockHelper()
	{
		_mutex->unlock();
	}

protected:
	const VMutex* _mutex;
};