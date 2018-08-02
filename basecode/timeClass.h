

#ifndef _TIMECLASS_H_
#define _TIMECLASS_H_

#include "basecode/basetype.h"

#ifdef WINDOWS
#define SW_CONSTANT_S64(a) (a##I64)
#define SW_CONSTANT_U64(a) (a##UI64)
#else
#define SW_CONSTANT_S64(a) a
#define SW_CONSTANT_U64(a) a
#endif

class Time
{
   class Tester;
   
public:
   struct DateTime
   {
      int32 year;
      int32 month;
      int32 day;
      int32 hour;
      int32 minute;
      int32 second;
      int32 microsecond;
   };
 
   static void getCurrentDateTime(DateTime &dateTime);
   static Time getCurrentTime();

   static const int64 OneDay          = SW_CONSTANT_S64(8640000000);
   static const int64 OneHour         = SW_CONSTANT_S64( 360000000);
   static const int64 OneMinute       = SW_CONSTANT_S64(   6000000);
   static const int64 OneSecond       = SW_CONSTANT_S64(    100000);
   static const int64 OneMillisecond  = SW_CONSTANT_S64(       100);

   Time();
   explicit Time(int64 time);
   Time(int32 year, int32 month, int32 day, int32 hour, int32 minute, int32 second, int32 microsecond);
   Time(const DateTime &dateTime);

   bool set(int32 year, int32 month, int32 day, int32 hour, int32 minute, int32 second, int32 microsecond);
   void get(int32 *year, int32 *month, int32 *day, int32 *hour, int32 *minute, int32 *second, int32 *microsecond) const;

   Time operator+() const;
   Time operator-() const;
   Time operator+(const Time &time) const;
   Time operator-(const Time &time) const;
   int64 operator/(const Time &time) const;
   const Time& operator+=(const Time time);
   const Time& operator-=(const Time time);
   
   template<typename T> Time operator*(T scaler) const { return Time(_time * scaler); }
   template<typename T> Time operator/(T scaler) const { return Time(_time / scaler); }
   template<typename T> friend Time operator*(T scaler,Time t) { return t * scaler; }

   bool operator==(const Time &time) const;
   bool operator!=(const Time &time) const;
   bool operator<(const Time &time) const;
   bool operator>(const Time &time) const;
   bool operator<=(const Time &time) const;
   bool operator>=(const Time &time) const;

   operator Tester*() const
   {
      static Tester test;
      return (_time == 0)? 0: &test;
   }
   bool operator!() const;

   int64 getSeconds() const;
   int64 getMilliseconds() const;
   int64 getMicroseconds() const;
   int64 getInternalRepresentation() const;

private:
   class Tester
   {
      void operator delete(void*) {}
   };

   int64 _time;

   bool _isLeapYear(int32 year) const;
   int32  _daysInMonth(int32 month, int32 year) const;
};

namespace TimeConstant
{
   const Time OneDay          (Time::OneDay);
   const Time OneHour         (Time::OneHour);
   const Time OneMinute       (Time::OneMinute);
   const Time OneSecond       (Time::OneSecond);
   const Time OneMillisecond  (Time::OneMillisecond);
}

//-----------------------------------------------------------------------------

inline Time::Time()
{
   _time = 0;
}

inline Time::Time(int64 time)
{
   _time = time;
}

inline Time::Time(int32 year, int32 month, int32 day, int32 hour, int32 minute, int32 second, int32 microsecond)
{
   set(year, month, day, hour, minute, second, microsecond);
}

inline Time::Time(const Time::DateTime &dateTime)
{
   set(dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second, dateTime.microsecond);
}

inline Time Time::operator+() const
{
   return Time(_time);
}

inline Time Time::operator-() const
{
   return Time(-_time);
}

inline Time Time::operator+(const Time &time) const
{
   return Time(_time + time._time);
}

inline Time Time::operator-(const Time &time) const
{
   return Time(_time - time._time);
}

inline int64 Time::operator/(const Time &time) const
{
   return int64(_time / time._time);
}

inline const Time& Time::operator+=(const Time time)
{
   _time += time._time;
   return *this;
}

inline const Time& Time::operator-=(const Time time)
{
   _time -= time._time;
   return *this;
}

inline bool Time::operator==(const Time &time) const
{
   return (_time == time._time);
}

inline bool Time::operator!=(const Time &time) const
{
   return (_time != time._time);
}

inline bool Time::operator<(const Time &time) const
{
   return (_time < time._time);
}

inline bool Time::operator>(const Time &time) const
{
   return (_time > time._time);
}

inline bool Time::operator<=(const Time &time) const
{
   return (_time <= time._time);
}

inline bool Time::operator>=(const Time &time) const
{
   return (_time >= time._time);
}

inline bool Time::operator !() const
{
   return _time == 0;
}

inline int64 Time::getSeconds() const
{
   return _time / TimeConstant::OneSecond._time;
}

inline int64 Time::getMilliseconds() const
{
   return _time / TimeConstant::OneMillisecond._time;
}

inline int64 Time::getMicroseconds() const
{
   return _time * 10;
}

inline int64 Time::getInternalRepresentation() const
{
   return _time;
}

template<class S> inline bool read(S &stream, Time *theTime)
{
   int64 time;
   bool ret = read(stream, &time);
   *theTime = Time(time);
   return ret;
}

template<class S> inline bool write(S &stream, const Time &theTime)
{
   int64 time = theTime.getInternalRepresentation();
   return write(stream, time);
}

//-----------------------------------------------------------------------------

inline Time UnixTimeToTime(uint32 t)
{
   // Converts "unix" time, seconds since 00:00:00 UTC, January 1, 1970
   return Time(((int64)(t)) * 100000 + SW_CONSTANT_S64(6213568320000000));
}

inline Time Win32FileTimeToTime(uint32 low,uint32 high)
{
   // Converts Win32 "file" time, 100 nanosecond intervals since 00:00:00 UTC, January 1, 1601
   int64 t = (((int64)high) << 32) + low;
   return Time(t / 100 + SW_CONSTANT_S64(5049120960000000));
}


#endif
