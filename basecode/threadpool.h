#pragma once

#include <vector>
#include "basecode/basehdr.h"
#include "basecode/singleton.h"
#include "basecode/threadopt.h"

template< class Operate, class T >
class ThreadPool : public Singleton< T >
{
public:
	ThreadPool()
	{
		_index = 0;
	}

	void init( uint32 threadnum )
	{
		for ( int i=0; i<threadnum; i++ )
		{
			ThreadOpt<Operate>* pOpt = new ThreadOpt<Operate>();
			pOpt->run();
			_vecThread.push_back( pOpt );
		}
		_threadNum = threadnum;
	}

	void addOpt( Operate* pOpt )
	{
		_vecThread[_index++]->addOpt( pOpt );
		_index %= _threadNum;
	}

	void cancel()
	{
		for ( int i=0; i<_threadNum; i++ )
		{
			_vecThread[i]->cancel();
			_vecThread[i]->join();
		}
	}

protected:
	std::vector< ref_ptr<ThreadOpt<Operate>> > _vecThread;
	uint32 _threadNum;
	uint32 _index;
};
