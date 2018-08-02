#include <time.h>
#include "basecode/timeClass.h"


static int8  _DaysInMonth[13]    = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int8  _DaysInMonthLeap[13]= {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int32 _DayNumber[13]      = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
static int32 _DayNumberLeap[13]  = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};


/**----------------------------------------------------------------------------
* PRIVATE Test to see if a year is a leap year.
*
* @param year Year to test for leap year
* @return true if year is a leap year
*/
inline bool Time::_isLeapYear(int32 year) const
{
   return ((year & 3) == 0) && ( ((year % 100) != 0) || ((year % 400) == 0) );
}


/**----------------------------------------------------------------------------
* PRIVATE Determine the number of days in any given month/year
*
* @param month Month to be tested
* @param year Year to be tested
* @return Number of days in month/year
*/
int32 Time::_daysInMonth(int32 month, int32 year) const
{
   if (_isLeapYear(year))
      return _DaysInMonthLeap[month];
   else
      return _DaysInMonth[month];
}


//-----------------------------------------------------------------------------
void Time::getCurrentDateTime(DateTime &dateTime)
{
   time_t      long_time;

   time( &long_time );

   struct tm   *systime = localtime( &long_time );

   dateTime.year        = systime->tm_year;
   dateTime.month       = systime->tm_mon;
   dateTime.day         = systime->tm_mday;
   dateTime.hour        = systime->tm_hour;
   dateTime.minute      = systime->tm_min;
   dateTime.second      = systime->tm_sec;
   dateTime.microsecond = 0;
}

Time Time::getCurrentTime()
{
   return UnixTimeToTime( (uint32)time(NULL) );
}

bool Time::set(int32 year, int32 month, int32 day, int32 hour, int32 minute, int32 second, int32 microsecond)
{
   second += microsecond / 100000;
   microsecond %= 100000;
   minute += second / 60;
   second %= 60;
   hour += minute / 60;
   minute %= 60;
   int32 carryDays = hour / 24;
   hour %= 24;

   bool leapYear = _isLeapYear(year);

   year -= 1;     // all the next operations need (year-1) so do it ahead of time
   int32 gregorian = 365 * year             // number of days since the epoch
                   + (year/4)             // add Julian leap year days
                   - (year/100)           // subtract century leap years
                   + (year/400)           // add gregorian 400 year leap adjustment
                   + ((367*month-362)/12) // days in prior months
                   + day                  // add days
                   + carryDays;           // add days from time overflow/underflow

   // make days in this year adjustment if leap year
   if (leapYear)
   {
      if (month > 2)
         gregorian -= 1;
   }
   else
   {
      if (month > 2)
         gregorian -= 2;
   }

   _time  = int64(gregorian) * OneDay;
   _time += int64((hour * OneHour) +
                (minute * OneMinute) +
                (second * OneSecond) +
                microsecond);

   return true;
}


//-----------------------------------------------------------------------------
void Time::get(int32 *pyear, int32 *pmonth, int32 *pday, int32 *phour, int32 *pminute, int32 *psecond, int32 *pmicrosecond) const
{
   // extract date if requested
   if (pyear || pmonth || pday)
   {
      int32 gregorian = (int32)(_time / OneDay);

      int32 prior = gregorian - 1;           // prior days
      int32 years400 = prior / 146097L;      // number of 400 year cycles
      int32 days400 = prior % 146097L;       // days NOT in years400
      int32 years100 = days400 / 36524L;     // number 100 year cycles not checked
      int32 days100 =  days400 % 36524L;     // days NOT already included
      int32 years4 = days100 / 1461L;        // number 4 year cycles not checked
      int32 days4 = days100 % 1461L;         // days NOT already included
      int32 year1 = days4 / 365L;            // number years not already checked
      int32 day1  = days4 % 365L;            // days NOT already included
      int32 day;
      int32 year = (400 * years400) + (100 * years100) + (4 * years4) + year1;

      // December 31 of leap year
      if (years100 == 4 || year1 == 4)
      {
          day = 366;
      }
      else
      {
          year += 1;
          day = day1 + 1;
      }

      const int32 *dayNumber = _isLeapYear(year) ? _DayNumberLeap : _DayNumber;

      // find month and day in month given computed year and day number,
      int32 month = 1;
      while(day >= dayNumber[month])
         month++;

      day -= dayNumber[month-1];

      if(pyear)
         *pyear  = year;
      if(pmonth)
         *pmonth = month;
      if(pday)
         *pday   = day;
   }

   // extract time
   if (phour)
      *phour = (int32)((_time % OneDay) / OneHour);

   int32 time = (int32)(_time % OneHour);

   if (pminute)
      *pminute = time / (int32)OneMinute;
   time %= OneMinute;

   if (psecond)
      *psecond = time / (int32)OneSecond;
   if (pmicrosecond)
      *pmicrosecond = time % OneSecond;
}

