#pragma once

#include "basecode/Referenced.h"

class Operate : public CReferenced
{
public:
	Operate();
	virtual ~Operate();
	virtual bool doWork() = 0;
	bool isFinish()
	{
		return _bfinish;
	}
	bool isErr()
	{
		return _bErr;
	}

	void finish()
	{
		_bfinish = true;
	}

	void err()
	{
		_bErr = true;
	}

	virtual void cancel() = 0;

protected:
	volatile bool _bfinish;
	bool _bErr;
};
