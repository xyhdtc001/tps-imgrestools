#if !defined TIME_VAL_DOT_H
#define TIME_VAL_DOT_H

#ifdef WINDOWS
#include "windows.h"
#else
#if defined METRO

#else
#include <sys/time.h>
#include <unistd.h>
#endif

#endif

#include "basecode/basehdr.h"
#include "basecode/StringData.h"

class TimeVal
{
public:
	TimeVal();	
	TimeVal( int );
	virtual ~TimeVal();
	
	TimeVal( const TimeVal& src );
	TimeVal& operator=( const TimeVal & );
	
	const TimeVal& now();

	int getMonth();
	int getMonthDay();
	int getWeekDay();
	int getYear();
	int getMinute();
	int getSecond();
	int getHour();
	
	uint64 milliseconds() const;
	uint32 seconds() const;
	
	TimeVal operator+( const TimeVal & ) const;
	TimeVal operator+( int ) const;
	TimeVal & operator+=( const TimeVal & );
	TimeVal & operator+=( int );
	TimeVal operator-( const TimeVal & ) const;
	TimeVal operator-( int ) const;
	TimeVal & operator-=( const TimeVal & );
	TimeVal & operator-=( int );
	bool	operator==( const TimeVal & ) const;
	bool	operator< ( const TimeVal & ) const;
	bool	operator<=( const TimeVal & ) const;
	bool	operator< ( int right ) const;
	bool	operator<=( int right ) const;
	bool	operator> (const TimeVal &) const;
	bool	operator>=(const TimeVal &) const;
	bool	operator> ( int right ) const;
	bool	operator>= ( int right ) const;
	
	void	clear();
	bool	isClear();
	void	copy( const TimeVal & );
	
	void	normalize();

	Data strftime( const char* format );

	long tv_sec;
	long tv_usec;

};

#ifdef WINDOWS
#ifndef METRO
int gettimeofday2( struct timeval *tv, struct timezone *tz );
#endif
#endif

#endif

