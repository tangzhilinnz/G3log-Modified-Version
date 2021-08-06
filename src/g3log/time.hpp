#pragma once
/** ==========================================================================
* 2012 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================
* Filename:g3time.h cross-platform, thread-safe replacement for C++11 non-thread-safe
*                   localtime (and similar)
* Created: 2012 by Kjell Hedström
*
* PUBLIC DOMAIN and Not under copywrite protection. First published for g3log at KjellKod.cc
* ********************************************* */

#include <ctime> // time_t defined in header <ctime>
#include <string>
#include <chrono>

// FYI:
// namespace g3::internal ONLY in g3time.cpp
//          std::string put_time(const struct tm* tmb, const char* c_time_format)

namespace g3 {
   // template <class Clock, class Duration = typename Clock::duration>  class time_point;
   // Clock (class):
   //     steady_clock -> monotonic clock that will never be adjusted
   //     high_resolution_clock -> the clock with the shortest tick period available
   //     system_clock -> wall clock time from the system-wide realtime clock
   // 
   // template <class Rep, class Period = ratio<1> > class duration;
   // template <intmax_t N, intmax_t D = 1> class ratio;
   // std::chrono::microseconds	-> duration</*signed integer type of at least 55 bits*/, std::micro>
   // micro -> std::ratio<1, 1000000>
   // std::chrono::milliseconds	-> duration</*signed integer type of at least 45 bits*/, std::milli>
   // milli	-> std::ratio<1, 1000>
   // std::chrono::nanoseconds -> duration</*signed integer type of at least 64 bits*/, std::nano>
   // nano -> std::ratio<1, 1000000000>
   // 
   // Constructs a new time_point from one of several optional data sources.
   //     1) Default constructor, creates a time_point representing the Clock's epoch 
   //        (i.e., time_since_epoch() is zero).
   //     2) Constructs a time_point at Clock's epoch plus d.
   //     3) Constructs a time_point by converting t to duration. This constructor only participates 
   //        in overload resolution if Duration2 is implicitly convertible to duration.
   // 
   // time_point();
   // constexpr time_point();
   // explicit time_point( const duration& d );
   // constexpr explicit time_point( const duration& d );
   // template< class Duration2 > time_point(const time_point<Clock, Duration2>& t);
   // template< class Duration2 > constexpr time_point(const time_point<Clock, Duration2>& t);
   // 
   typedef std::chrono::time_point<std::chrono::system_clock> system_time_point;
   typedef std::chrono::time_point<std::chrono::high_resolution_clock> high_resolution_time_point;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::microseconds microseconds;

   namespace internal {
      enum class Fractional { Millisecond, Microsecond, Nanosecond, NanosecondDefault };
      Fractional getFractional(const std::string& format_buffer, size_t pos);
      std::string to_string(const g3::system_time_point& ts, Fractional fractional);
      std::string localtime_formatted_fractions(const g3::system_time_point& ts, std::string format_buffer);
      static const std::string date_formatted = "%Y/%m/%d";
      // %f: fractions of seconds (%f is nanoseconds)
      // %f3: milliseconds, 3 digits: 001
      // %f6: microseconds: 6 digits: 000001  --- default for the time_format
      // %f9, %f: nanoseconds, 9 digits: 000000001
      static const std::string time_formatted = "%H:%M:%S %f6";
   } // internal


   // This mimics the original "std::put_time(const std::tm* tmb, const charT* fmt)"
   // This is needed since latest version (at time of writing) of gcc4.7 does not implement this library function yet.
   // return value is SIMPLIFIED to only return a std::string
   std::string put_time(const struct tm* tmb, const char* c_time_format);

   /** return time representing POD struct (ref ctime + wchar) that is normally
   * retrieved with std::localtime. g3::localtime is threadsafe which std::localtime is not.
   * g3::localtime is probably used together with @ref g3::systemtime_now */
   tm localtime(const std::time_t& time);

   /** format string must conform to std::put_time's demands.
   * WARNING: At time of writing there is only so-so compiler support for
   * std::put_time. A possible fix if your c++11 library is not updated is to
   * modify this to use std::strftime instead */
   std::string localtime_formatted(const system_time_point& ts, const std::string& time_format) ;

   inline system_time_point to_system_time(const high_resolution_time_point& ts)
   {
	   // On some (windows) systems, the system_clock does not provide the highest possible time
	   // resolution. Thus g3log uses high_resolution_clock for message time stamps. However,
	   // unlike system_clock, high_resolution_clock cannot be converted to a time and date as
	   // it usually measures reflects the time since power-up. 
	   // Thus, hrs_now and sys_now are recorded once when the program starts to be able to convert
	   // timestamps to time and date using to_system_time(). The precision of the absolute time is
	   // of course that of system_clock() with some error added due to the non-simultaneous initialization
	   // of the two static variables but relative times within one log will be as precise as 
	   // high_resolution_clock.
	   using namespace std::chrono;
	   // static time_point now() noexcept;
	   static const auto hrs_now = high_resolution_clock::now();
	   static const auto sys_now = system_clock::now();
	   // template <class ToDuration, class Clock, class Duration>
	   // time_point<Clock, ToDuration> time_point_cast(const time_point<Clock, Duration>&tp);
	   //
	   // The value of tp converted into an object of type time_point<Clock,ToDuration>.
	   // time_point<Clock, ToDuration> is a time_point with the same Clock type as tp.
       return time_point_cast<system_clock::duration>(sys_now + (ts - hrs_now));
   }
}



