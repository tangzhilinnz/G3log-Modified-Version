/** ==========================================================================
* 2012 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
*
* For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/

#include "g3log/time.hpp"

#include <sstream>
#include <string>
#include <cstring>
#include <cmath>
#include <chrono>
#include <cassert>
#include <iomanip>
// __MACH__ is a built in compiler macro that indicates if you are on Macintosh
// operating system. It is defined when compiling on a mac machine.
#ifdef __MACH__
#include <sys/time.h>
#endif

/*
 * typedef ratio<1, 1000000000000000000> atto;
 * typedef ratio<1, 1000000000000000> femto;
 * typedef ratio<1, 1000000000000> pico;
 * typedef ratio<1, 1000000000> nano;
 * typedef ratio<1, 1000000> micro;
 * typedef ratio<1, 1000> milli;
 * typedef ratio<1, 100> centi;
 * typedef ratio<1, 10> deci;
 * typedef ratio<10, 1> deca;
 * typedef ratio<100, 1> hecto;
 * typedef ratio<1000, 1> kilo;
 * typedef ratio<1000000, 1> mega;
 * typedef ratio<1000000000, 1> giga;
 * typedef ratio<1000000000000, 1> tera;
 * typedef ratio<1000000000000000, 1> peta;
 * typedef ratio<1000000000000000000, 1> exa;
 *
 * typedef duration<int64_t, nano>     nanoseconds;
 * typedef duration<int64_t, micro>    microseconds;
 * typedef duration<int64_t, milli>    milliseconds;
 * typedef duration<int64_t>           seconds;
 * typedef duration<int, ratio<60>>    minutes;
 * typedef duration<int, ratio<3600>>  hours;
 */

namespace g3 {
   namespace internal {
      const std::string kFractionalIdentier   = "%f";
      const size_t kFractionalIdentierSize = 2;

      Fractional getFractional(const std::string& format_buffer, size_t pos) {
         // format_buffer.size() == pos + kFractionalIdentierSize 
         // when "%f" is the tail of format_buffer 
         char  ch  = (format_buffer.size() > pos + kFractionalIdentierSize ? 
                      format_buffer.at(pos + kFractionalIdentierSize) : '\0');
         Fractional type = Fractional::NanosecondDefault;
         switch (ch) {
            case '3': type = Fractional::Millisecond; break;
            case '6': type = Fractional::Microsecond; break;
            case '9': type = Fractional::Nanosecond; break;
            default: type = Fractional::NanosecondDefault; break;
         }
         return type;
      }

      // Returns the fractional as a string with padded zeroes
      // 1 ms --> 001
      // 1 us --> 000001
      // 1 ns --> 000000001
      std::string to_string(const g3::system_time_point& ts, Fractional fractional) {
         // duration time_since_epoch() const;
         // Returns a duration object with the time span value between the epoch and the time point.
         // The value returned is the current value of the internal duration object.
         auto duration = ts.time_since_epoch();
         // template <class ToDuration, class Rep, class Period>
         //    constexpr ToDuration duration_cast(const duration<Rep, Period>&dtn);
         // Converts the value of dtn into some other duration type, taking into account 
         // differences in their periods.
         // constexpr rep count() const;
         auto sec_duration = std::chrono::duration_cast<std::chrono::seconds>(duration);
         duration -= sec_duration;
         auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

         auto zeroes = 9; // default ns
         auto digitsToCut = 1; // default ns, divide by 1 makes no change
         switch (fractional) {
            case Fractional::Millisecond : {
               zeroes = 3;
               digitsToCut = 1000000;
               break;
            }
            case Fractional::Microsecond : {
               zeroes = 6;
               digitsToCut = 1000;
               break;
            }
            case Fractional::Nanosecond :
            case Fractional::NanosecondDefault:
            default:
               zeroes = 9;
               digitsToCut = 1;
         }

         ns /= digitsToCut;
         auto value = std::string(std::to_string(ns));
         return std::string(zeroes - value.size(), '0') + value;
      }

      std::string localtime_formatted_fractions(const g3::system_time_point& ts, std::string format_buffer) {
         // iterating through every "%f" instance in the format string
         auto identifierExtraSize = 0;
         for (size_t pos = 0; // static const size_t npos = -1; As a return value, it is usually used to indicate no matches.
              (pos = format_buffer.find(g3::internal::kFractionalIdentier, pos)) != std::string::npos;
              pos += g3::internal::kFractionalIdentierSize + identifierExtraSize) {
            // figuring out whether this is nano, micro or milli identifier
            auto type = g3::internal::getFractional(format_buffer, pos);
            auto value = g3::internal::to_string(ts, type);
            auto padding = 0;
            if (type != g3::internal::Fractional::NanosecondDefault) {
               padding = 1;
            }

            // replacing "%f[3|6|9]" with sec fractional part value
            format_buffer.replace(pos, g3::internal::kFractionalIdentier.size() + padding, value);
         }
         return format_buffer;
      }

   } // internal
} // g3



namespace g3 {
   // This mimics the original "std::put_time(const std::tm* tmb, const charT* fmt)"
   // This is needed since latest version (at time of writing) of gcc4.7 does not implement this library function yet.
   // return value is SIMPLIFIED to only return a std::string
   std::string put_time(const struct tm* tmb, const char* c_time_format) {
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__)) && !defined(__MINGW32__)
      std::ostringstream oss;
      oss.fill('0');
      // BOGUS hack done for VS2012: C++11 non-conformant since it SHOULD take a "const struct tm*  "
      oss << std::put_time(const_cast<struct tm*> (tmb), c_time_format);
      return oss.str();
#else    // LINUX
      const size_t size = 1024;
      char buffer[size]; // IMPORTANT: check now and then for when gcc will implement std::put_time.
      //                    ... also ... This is way more buffer space than we need

      auto success = std::strftime(buffer, size, c_time_format, tmb);
      // In DEBUG the assert will trigger a process exit. Once inside the if-statement
      // the 'always true' expression will be displayed as reason for the exit
      //
      // In Production mode
      // the assert will do nothing but the format string will instead be returned
      if (0 == success) {
         assert((0 != success) && "strftime fails with illegal formatting");
         return c_time_format;
      }
      return buffer;
#endif
   }



   tm localtime(const std::time_t& ts) {
      // parameters are in the opposite order in localtime_s() for windows platforms 
      // and in localtime_r() for unix systems 
      struct tm tm_snapshot;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
      localtime_s(&tm_snapshot, &ts); // windsows
#else
      localtime_r(&ts, &tm_snapshot); // POSIX
#endif
      return tm_snapshot;
   }


   std::string localtime_formatted(const g3::system_time_point& ts, const std::string& time_format) {
      auto format_buffer = internal::localtime_formatted_fractions(ts, time_format);
      // static time_t to_time_t (const time_point& tp) noexcept;
      // Converts tp into its equivalent of type time_t.
      auto time_point = std::chrono::system_clock::to_time_t(ts);
      // struct tm * localtime (const time_t * timer);
      // Uses the value pointed by timer to fill a tm structure with the values
      // that represent the corresponding time, expressed for the local timezone.
      std::tm t = localtime(time_point);
      return g3::put_time(&t, format_buffer.c_str()); // format example: //"%Y/%m/%d %H:%M:%S");
   }
} // g3
