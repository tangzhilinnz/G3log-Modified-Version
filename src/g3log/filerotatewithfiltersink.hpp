#pragma once

#include "g3log/loglevels.hpp"
#include "g3log/logmessage.hpp"
#include "g3log/filerotatesink.hpp"

#include <utility>
#include <memory>
#include <vector>

/**
* Wraps a LogRotate file logger. It only forwareds log LEVELS
* that are NOT in the filter
*/

namespace g3 {

   class FileRotateWithFilter {
      using FileRotateUptr = std::unique_ptr<FileRotateSink>;
      using IgnoreLogLevels = std::vector<LEVELS>;
      
   public:
      static std::unique_ptr<FileRotateWithFilter>  CreateLogRotateWithFilter(
         std::string filename, std::string directory, std::vector<LEVELS> filter);

      FileRotateWithFilter(FileRotateUptr logToFile, IgnoreLogLevels ignoreLevels);
      virtual ~FileRotateWithFilter();
      
      void setMaxArchiveLogCount(int max_size);
      int getMaxArchiveLogCount();
      void setMaxLogSize(int max_file_size);
      int getMaxLogSize();

      void save(g3::LogMessageMover logEntry);
      void setFlushPolicy(size_t flush_policy); // 0: never (system auto flush), 1 ... N: every n times
      void flush();

      std::string changeLogFile(const std::string& log_directory);
      std::string logFileName();
      bool rotateLog();
      void setLogSizeCounter();
      
      void overrideLogDetails(LogMessage::LogDetailsFunc func);
      void overrideLogHeader(const std::string& change);

    private:
      FileRotateUptr logger_;
      IgnoreLogLevels filter_;
   };


} // g3 namespace