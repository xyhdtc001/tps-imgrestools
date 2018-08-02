#include <cassert>
#include <sys/timeb.h>
#include <time.h>
#include "basecode/TimeVal.h"

const char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

TimeVal::TimeVal()
{
    clear();
}


TimeVal::TimeVal( int src )
{
    clear();
    operator+=( src );
}


TimeVal::TimeVal( const TimeVal& src )
{
    copy( src );
    normalize();
}


TimeVal::~TimeVal()
{
}


TimeVal &
TimeVal::operator=( const TimeVal & src )
{
    if ( this != & src )
    {
        copy( src );
        normalize();
    }
    return ( *this );
}


const TimeVal &
TimeVal::now()
{
#ifdef WINDOWS
#ifndef METRO
	timeval v;
    int rc = gettimeofday2( &v, 0 );
    assert( rc == 0 );
	tv_sec = v.tv_sec;
	tv_usec = v.tv_usec;
#endif
    
#else   //ios
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv_sec = tv.tv_sec;
    tv_usec = tv.tv_usec;
#endif
	return ( *this );
}


uint64
TimeVal::milliseconds() const
{
    return ( (uint64)tv_sec*1000 + (uint64)tv_usec/1000 );
}

uint32
TimeVal::seconds() const
{
	return tv_sec;
}

TimeVal
TimeVal::operator+( const TimeVal & right ) const
{
    TimeVal left( *this );
    return ( left += right );
}


TimeVal
TimeVal::operator+( int right ) const
{
    TimeVal left( *this );
    return ( left += right );
}


TimeVal &
TimeVal::operator+=( const TimeVal & src )
{
    tv_sec += src.tv_sec;
    tv_usec += src.tv_usec;
    normalize();
    return ( *this );
}


TimeVal &
TimeVal::operator+=( int src )
{
    tv_sec += src / 1000;
    tv_usec += (src%1000)*1000;
    normalize();
    return ( *this );
}


TimeVal
TimeVal::operator-( const TimeVal & right ) const
{
    TimeVal left( *this );
    return ( left -= right );
}


TimeVal
TimeVal::operator-( int right ) const
{
    TimeVal left( *this );
    return ( left -= right );
}


TimeVal&
TimeVal::operator-=( const TimeVal & src )
{
    tv_sec -= src.tv_sec;
    tv_usec -= src.tv_usec;
    normalize();
    return ( *this );
}


TimeVal &
TimeVal::operator-=( int src )
{
    tv_sec -= src / 1000;
    tv_usec -= (src%1000)*1000;
    normalize();
    return (*this);
}


bool
TimeVal::operator==( const TimeVal & src ) const
{
    TimeVal right( src );
    return ( tv_sec==right.tv_sec && tv_usec==right.tv_usec );
}


bool
TimeVal::operator<( const TimeVal & src ) const
{
    TimeVal right( src );
    if ( tv_sec < right.tv_sec )
    {
        return true;
    }
    else if ( (tv_sec == right.tv_sec) && (tv_usec < right.tv_usec) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool
TimeVal::operator>( const TimeVal &src ) const
{
	TimeVal right( src );
	if ( tv_sec > right.tv_sec )
	{
		return true;
	}
	else if ( (tv_sec == right.tv_sec) && ( tv_usec > right.tv_usec ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool
TimeVal::operator<( int right ) const
{
    if ( (tv_sec * 1000 + tv_usec/1000 ) < right )
    	return true;
    else
    	return false;
}

bool
TimeVal::operator>( int right ) const
{
    if ( (tv_sec * 1000 + tv_usec/1000 ) > right )
    	return true;
    else
    	return false;
}

bool
TimeVal::operator<=( int right ) const
{
    if ( (tv_sec * 1000 + tv_usec/1000 ) <= right )
    	return true;
    else
    	return false;
}

bool
TimeVal::operator>=( int right ) const
{
    if ( (tv_sec * 1000 + tv_usec/1000 ) >= right )
    	return true;
    else
    	return false;
}

bool
TimeVal::operator<=( const TimeVal & src ) const
{
    // Create a normalize value for the timeval to simplify comparison.
    TimeVal right( src );

    if ( tv_sec < right.tv_sec )
    {
        return ( true );
    }
    else if ( (tv_sec == right.tv_sec) && (tv_usec <= right.tv_usec) )
    {
        return ( true );
    }
    else
    {
        return ( false );
    }
}


bool
TimeVal::operator>=( const TimeVal & src ) const
{
    TimeVal right(src);
    if ( tv_sec > right.tv_sec )
    {
        return ( true );
    }
    else if ( (tv_sec == right.tv_sec) && (tv_usec >= right.tv_usec) )
    {
        return true;
    }
    else
    {
        return false;
    }
}


void
TimeVal::clear()
{
    tv_sec = tv_usec = 0;
}


bool
TimeVal::isClear()
{
	if ( tv_sec==0 && tv_usec==0 )
		return true;
	else
		return false;
}


void
TimeVal::copy( const TimeVal & src )
{
    tv_sec = src.tv_sec;
    tv_usec = src.tv_usec;
}


void
TimeVal::normalize()
{
    if ( tv_usec < 0 )
    {
        long num_sec = ( -tv_usec / 1000000) + 1;
        assert(num_sec > 0);
        tv_sec -= num_sec;
        tv_usec += num_sec * 1000000;
    }

    if ( tv_usec >= 1000000 )
    {
        tv_sec += tv_usec / 1000000;
        tv_usec %= 1000000;
    }
}


Data
TimeVal::strftime( const char* format )
{
    time_t now = tv_sec;
    char datebuf[256];
	struct tm* ptm = localtime( &now );
    ::strftime( datebuf, 256, format, ptm );
	return Data::ToString( datebuf );
}

int
TimeVal::getMinute()
{
	time_t t = tv_sec;
	struct tm* ptm = localtime( &t );
	return ptm->tm_min;
}

int
TimeVal::getHour()
{
	time_t t = tv_sec;
	struct tm* ptm = localtime( &t );
	return ptm->tm_hour;
}

int
TimeVal::getYear()
{
	time_t t = tv_sec;
	struct tm* ptm = localtime( &t );
	int val = 1900 + ptm->tm_year; 
	return val;
}

int
TimeVal::getSecond()
{
	time_t t = tv_sec;
	struct tm* ptm = localtime( &t );
	return ptm->tm_sec;
}

int
TimeVal::getMonth()
{
	time_t t = tv_sec;
	struct tm* ptm = localtime( &t );
	return ptm->tm_mon+1;
}

int
TimeVal::getMonthDay()
{
	time_t t = tv_sec;
	struct tm* ptm = localtime( &t );
	return ptm->tm_mday;
}

int
TimeVal::getWeekDay()
{
	time_t t = tv_sec;
	struct tm* ptm = localtime( &t );
	return ptm->tm_wday;	
}


#ifdef WINDOWS
#ifndef METRO
int
gettimeofday2( struct timeval *tv, struct timezone* )
{
	struct _timeb currSysTime;
	_ftime( &currSysTime );
	tv->tv_sec = (long)currSysTime.time;
	tv->tv_usec = currSysTime.millitm * 1000;
    return 0;
}
#endif
#endif
