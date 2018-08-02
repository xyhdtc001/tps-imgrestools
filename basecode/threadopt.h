#pragma once

#include <vector>
#include <list>

#include "basecode/lockhelper.h"
#include "basecode/ref_ptr.h"
#include "basecode/vmutex.h"
#include "basecode/vcondition.h"
#include "basecode/threadif.h"
#include "basecode/Referenced.h"

template< class Opt >
class ThreadOpt : public ThreadIf, public CReferenced
{
public:
	ThreadOpt()
	{
		_bWait = false;
	}

	void addOpt( ref_ptr<Opt> opt )
	{
		LockHelper helper( &_mutex );
		_listOpt.push_back( opt );
		if ( _bWait )
			_cond.signal();
	}

	void thread()
	{
		while ( !isShutdown() )
		{
			{
				LockHelper helper( &_mutex );
				_opt = NULL;
				if ( _listOpt.size() > 0 )
				{
					_opt = *_listOpt.begin();
					_listOpt.pop_front();
				}
				else
				{
					_bWait = true;
				}
			}
			if ( _opt != NULL )
			{
				if ( _opt->doWork() )
					_opt->finish();
				else
					_opt->err();
			}
			else
			{
				_cond.wait();
			}
		}
	}

	void cancel()
	{
		LockHelper helper( &_mutex );
		shutdown();
		_cond.signal();
		if (_opt )
			_opt->cancel();
	}

protected:
	std::list< ref_ptr<Opt> > _listOpt;
	VMutex _mutex;
	VConditoin _cond;
	bool _bWait;
	ref_ptr< Opt > _opt;
};