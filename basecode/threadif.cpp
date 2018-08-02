#include "BaseCode/ThreadIf.h"
#include <assert.h>

#ifndef WINDOWS

VThread::VThread()
{
	_bShutdown = false;
	bThreadCreated = false;
}


VThread::~VThread()
{
	
}

bool
VThread::spawn( void *( *startFunc )( void * ),
			   void *startArgs,
			   unsigned long flags,
			   unsigned long priority,
			   int stack_size)
{
    struct sched_param priorityParams;
    if ( pthread_attr_init(&myAttributes) != 0 )
		return false;
	
    switch( flags & VTHREAD_SCHED_MASK )
    {
        case VTHREAD_SCHED_FIFO:
        {
#ifndef __ANDROID__
            if ( pthread_attr_setinheritsched(&myAttributes,PTHREAD_EXPLICIT_SCHED) != 0 )
				return false;
#endif
            if ( pthread_attr_setschedpolicy(&myAttributes,SCHED_FIFO) != 0 )
				return false;
        }
			break;
        case VTHREAD_SCHED_RR:
        {
#ifndef __ANDROID__
            if ( pthread_attr_setinheritsched(&myAttributes,PTHREAD_EXPLICIT_SCHED) != 0 )
				return false;
#endif
            if ( pthread_attr_setschedpolicy(&myAttributes,SCHED_RR) != 0 )
				return false;
        }
			break;
        case VTHREAD_SCHED_DEFAULT:
			break;
        default:
			break;
    }
	
    // if anything expect default, set scheduling priority explicitly;
    // note that by default the priority of the parent thread is inherited
    if ( (int)priority != VTHREAD_PRIORITY_DEFAULT )
    {
        // probably should improve to use relative values
        priorityParams.sched_priority = priority;
        if ( pthread_attr_setschedparam(&myAttributes,&priorityParams) != 0 )
			return false;
    }
    // spawn the thread
	bThreadCreated = true;
    if ( pthread_create(&myId,&myAttributes,startFunc,startArgs) != 0 )
		return false;
	else
		return true;
}


int
VThread::join( void **status )
{
	if ( !bThreadCreated )
		return 0;
    int retVal = pthread_join( myId, status );
    return retVal;
}


int
VThread::getPriority() const
{
    struct sched_param priorityParams;
    assert( pthread_attr_getschedparam( &myAttributes, &priorityParams ) == 0 );
    return priorityParams.sched_priority;
}


const vthread_t
VThread::getId() const
{
    return myId;
}


const vthread_attr_t*
VThread::getAttributes() const
{
    return &myAttributes;
}

void
VThread::exit()
{
#ifndef __ANDROID__
	pthread_cancel( myId );
#else

#endif
}


const vthread_t
VThread::selfId()
{
    return pthread_self();
}

bool
VThread::isShutdown()
{
	return _bShutdown;
}

void
VThread::shutdown()
{
	_bShutdown = true;
}


void VThread::setparam( int param )
{
	_param = param;
}


int VThread::getparam()
{
	return _param;
}

static void*
threadWrapper( void* threadParm )
{
    assert( threadParm );
    ThreadIf* t = static_cast< ThreadIf* >( threadParm );
    assert( t );
    t->thread();
    return 0;
}

#endif

ThreadIf::ThreadIf() : _shutdown(true),_bRun(false)
{
	#if defined WINDOWS
	_hThread = NULL;
	#else
	#if defined METRO
	#else
	#endif
	#endif
}


ThreadIf::~ThreadIf()
{

}

//#ifndef WINDOWS
//#define __stdcall
//#endif

#ifdef WINDOWS
DWORD __stdcall ThreadIf_ThreadFunc( void* lpThreadParameter )
{
	ThreadIf* pThread = (ThreadIf*)lpThreadParameter;
	pThread->thread();
	pThread->_bRun = false;
	return 0;
}
#endif


void
ThreadIf::run( unsigned long priority )
{
	_shutdown = false;
	#if defined WINDOWS
	_bRun = true;
    _hThread = CreateThread( NULL, NULL, ThreadIf_ThreadFunc, this, 0, &_threadId );
	#else
	itsThread.spawn( threadWrapper, this, VTHREAD_SCHED_DEFAULT, priority );
	#endif
}


void
ThreadIf::join()
{
#if defined WINDOWS
    if ( _hThread != NULL )
	{
		WaitForSingleObject( _hThread, INFINITE );
		CloseHandle( _hThread );
	}
#else
    itsThread.join();
#endif
}
