/** ==========================================================================
* 2012 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
*
* For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/

#include "g3log/loglevels.hpp"
#include <cassert>

#include <iostream>

namespace g3 {
   namespace internal {
      bool wasFatal(const LEVELS& level) {
         return level.value >= FATAL.value;
      }

#ifdef G3_DYNAMIC_LOGGING
      const std::map<int, LoggingLevel> g_log_level_defaults = {
         // loglevels.hpp  g3::LoggingLevel::LoggingLevel
         // LoggingLevel(const LEVELS& lvl): status(true), level(lvl) {};
	     {G3LOG_DEBUG.value, {G3LOG_DEBUG}}, // std::pair
         {INFO.value, {INFO}},
         {WARNING.value, {WARNING}},
         {FATAL.value, {FATAL}}
      };

      std::map<int, g3::LoggingLevel> g_log_levels = g_log_level_defaults;
#endif
   } // internal

#ifdef G3_DYNAMIC_LOGGING
   namespace only_change_at_initialization {

      void addLogLevel(LEVELS lvl, bool enabled) {
         int value = lvl.value;
         internal::g_log_levels[value] = {lvl, enabled};
      }


      void addLogLevel(LEVELS level) {
         addLogLevel(level, true);
      }

      void reset() {
         // Copy assignment operator. Replaces the contents of g_log_levels 
         // with a copy of the contents of g_log_level_defaults.
         // If 
         // std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value 
         // is true, the allocator of g_log_levels is replaced by a copy of that of
         // g_log_level_defaults. If the allocator of g_log_levels after assignment would compare 
         // unequal to its old value, the old allocator is used to deallocate 
         // the memory, then the new allocator is used to allocate it before 
         // copying the elements. Otherwise, the memory owned by g_log_levels may be 
         // reused when possible. In any case, the elements originally belong 
         // to g_log_levels may be either destroyed or replaced by element-wise copy-assignment
         // with g_log_level_defaults.
         g3::internal::g_log_levels = g3::internal::g_log_level_defaults;
      }
   } // only_change_at_initialization


   namespace log_levels {

      void setHighest(LEVELS enabledFrom) {
         auto it = internal::g_log_levels.find(enabledFrom.value);
         if (it != internal::g_log_levels.end()) {
            for (auto& v : internal::g_log_levels) {
               if (v.first < enabledFrom.value) {
                  disable(v.second.level);
               } else {
                  enable(v.second.level);
               }
            }
         }
      }


      void set(LEVELS level, bool enabled) {
         auto it = internal::g_log_levels.find(level.value);
         if (it != internal::g_log_levels.end()) {
            // loglevels.hpp  g3::LoggingLevel::LoggingLevel
            // LoggingLevel(const LEVELS& lvl, bool enabled);
            // loglevels.hpp  g3::LoggingLevel::operator=
            // LoggingLevel& operator=(const LoggingLevel& other);
            internal::g_log_levels[level.value] = {level, enabled};
         }
      }


      void disable(LEVELS level) {
         set(level, false);
      }

      void enable(LEVELS level) {
         set(level, true);
      }


      void disableAll() {
         for (auto& v : internal::g_log_levels) {
            // atomicbool.hpp  g3::atomicbool::operator=
            // atomicbool& operator=(const bool other);
            v.second.status = false;
         }
      }


      void enableAll() {
         for (auto& v : internal::g_log_levels) {
            // atomicbool.hpp  g3::atomicbool::operator=
            // atomicbool& operator=(const bool other);
            v.second.status = true;
         }
      }


      std::string to_string(std::map<int, g3::LoggingLevel> levelsToPrint) {
         std::string levels;
         for (auto& v : levelsToPrint) {
            levels += "name: " + v.second.level.text + 
                      " level: " + std::to_string(v.first) + 
                      " status: " + std::to_string(v.second.status.value()) + "\n";
                      // atomicbool.hpp  g3::atomicbool::value
                      // bool value();
         }
         return levels;
      }

      std::string to_string() {
         return to_string(internal::g_log_levels);
      }


      std::map<int, g3::LoggingLevel> getAll() {
         return internal::g_log_levels;
      }

      // status : {Absent, Enabled, Disabled};
      status getStatus(LEVELS level) {
         const auto it = internal::g_log_levels.find(level.value);
         if (internal::g_log_levels.end() == it) {
            return status::Absent;
         }

         // std::atomic<T>::load
         // T load( std::memory_order order = std::memory_order_seq_cst ) const volatile noexcept;
         // atomicbool.hpp  g3::atomicbool::get
         // std::atomic<bool>& get();
         return (it->second.status.get().load() ? status::Enabled : status::Disabled);
      }
   } // log_levels

#endif

   /// Enabled status for the given logging level
   // If level does not match the key of any element in g_log_levels, the 
   // function g_log_levels[level] inserts a new LoggingLevel element and 
   // returns a reference to its mapped value. Notice that this always 
   // increases the container size by one, even if no mapped value is 
   // assigned to the element (the new LoggingLevel element is constructed 
   // using its default constructor: "LoggingLevel(): status(false), level(INFO) {}"
   // in which case the function logLevel() always returns false).
   bool logLevel(const LEVELS& log_level) {
#ifdef G3_DYNAMIC_LOGGING
      int level = log_level.value;
      bool status = internal::g_log_levels[level].status.value();
      return status;
#else
      return true;
#endif
   }
} // g3
