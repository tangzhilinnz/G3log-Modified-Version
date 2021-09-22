#pragma once

#include "g3log/loglevels.hpp"
#include "g3log/logmessage.hpp"
#include "g3log/filerotatesink.hpp"

#include <utility>
#include <memory>
#include <vector>
#include <algorithm>

/**
* Wraps a LogRotate file logger. It only forwareds log LEVELS
* that are NOT in the filter
*/

namespace g3 {

   class FileRotateWithFilter {
      //using FileRotateUptr = std::unique_ptr<FileRotateSink>;
      //using IgnoreLogLevels = std::vector<LEVELS>;
      
   public:
       // helper function to create an logging sink with filter
      static std::unique_ptr<FileRotateWithFilter>  CreateLogRotateWithFilter(
         std::string filename, std::string directory, std::vector<LEVELS> filter) {
         auto fileRotateUptr = std::make_unique<FileRotateSink>(filename, directory);
         return std::make_unique<FileRotateWithFilter>(std::move(fileRotateUptr), filter);
      }

      /// @param logToFile rotate file logging sink
      /// @param removes all log entries with LEVELS in this filter
      FileRotateWithFilter(std::unique_ptr<FileRotateSink> logToFile, std::vector<LEVELS> ignoreLevels)
         : logger_(std::move(logToFile))
         , filter_(std::move(ignoreLevels))
      {}

      virtual ~FileRotateWithFilter() {}
      
      void setMaxArchiveLogCount(int max_size) {
         logger_->setMaxArchiveLogCount(max_size);
      }

      int getMaxArchiveLogCount() {
         return logger_->getMaxArchiveLogCount();
      }

      void setMaxLogSize(int max_file_size) {
         logger_->setMaxLogSize(max_file_size);
      }

      int getMaxLogSize() {
         return logger_->getMaxLogSize();
      }

      /// @param logEntry saves log entry that are not in the filter
      void save(g3::LogMessageMover logEntry) {
         auto level = logEntry.get()._level;
         bool isNotInFilter_ = (filter_.end() == std::find(filter_.begin(), filter_.end(), level));

         if (isNotInFilter_) {
            //logger_->save(logEntry.get().toString(_log_details_func));
            logger_->fileWrite(logEntry);
         }
      }

      /**
       * Flush policy: Default is every single time (i.e. policy of 1).
       *
       * If the system logs A LOT then it is likely better to allow for the system to buffer and write
       * all the entries at once.
       *
       * 0: System decides, potentially very long time
       * 1....N: Flush logs every n entry
       */
      void setFlushPolicy(size_t flush_policy) {
         logger_->setFlushPolicy(flush_policy);
      }
      
      /**
       * Force flush of log entries. This should normally be policed with the @ref setFlushPolicy
       * but is great for unit testing and if there are special circumstances where you want to see
       * the logs faster than the flush_policy
       */
      void flush() {
         logger_->flush();
      }

      std::string changeLogFile(const std::string& log_directory) {
         return logger_->changeLogFile(log_directory);
      }

      /// @return the current filename
      std::string logFileName() {
         return logger_->logFileName();
      }

      bool rotateLog() {
         return logger_->rotateLog();
      }

      void setLogSizeCounter() {
         logger_->setLogSizeCounter();
      }

      /**
       * Override the defualt log formatting.
       * Please see https://github.com/KjellKod/g3log/API.markdown for more details
       */
      void overrideLogDetails(LogMessage::LogDetailsFunc func) {
         logger_->overrideLogDetails(func);
      }

      void overrideLogHeader(const std::string& change) {
         logger_->overrideLogHeader(change);
      }

   private:
      std::unique_ptr<FileRotateSink> logger_;
      std::vector<LEVELS> filter_;
   };


} // g3 namespace