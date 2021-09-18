#pragma once

#include "g3log/logmessage.hpp"

#include <string>
#include <memory>
#include <ios>


/** The Real McCoy Background worker, while g3::LogWorker gives the
 * asynchronous API to put job in the background the FileRotateSink
 * does the actual background thread work
 *
 * Flushing of log entries will happen according to flush policy:
 * 0 is never (system decides, and when there is a log rotation)
 * 1 ... N means every x entry (1 is every time, 2 is every other time etc)
 * Default is to flush every single time
 */
namespace g3 {

   class FileRotateSink {
   public:
      FileRotateSink(const std::string& log_prefix, const std::string& log_directory, size_t flush_policy = 1);
      virtual ~FileRotateSink();

      void setMaxArchiveLogCount(int size);
      int getMaxArchiveLogCount();
      void setMaxLogSize(int size);
      int getMaxLogSize();

      void fileWrite(LogMessageMover message);
      void fileWriteWithoutRotate(std::string message);
      void setFlushPolicy(size_t flush_policy);
      void flush();

      std::string changeLogFile(const std::string& directory, const std::string& new_name = "");
      std::string logFileName();
      bool rotateLog();
      void setLogSizeCounter();
      //bool createCompressedFile(std::string file_name, std::string gzip_file_name);

      void overrideLogDetails(LogMessage::LogDetailsFunc func);
      void overrideLogHeader(const std::string& change);

   private:
      LogMessage::LogDetailsFunc log_details_func_;

      std::string log_file_with_path_;
      std::string log_directory_;
      std::string log_prefix_backup_;
      std::unique_ptr<std::ofstream> outptr_;
      std::string header_;
      //steady_time_point steady_start_time_; // std::chrono::time_point<std::chrono::steady_clock>;
      int max_log_size_;
      int max_archive_log_count_;
      long archive_unique_num_;
      std::streamoff cur_log_size_;
      size_t flush_policy_;
      size_t flush_policy_counter_;

      void flushPolicy();

      void addLogFileHeader();
      std::ofstream& filestream() {
         return *(outptr_.get());
      }

      FileRotateSink& operator=(const FileRotateSink&) = delete;
      FileRotateSink(const FileRotateSink& other) = delete;

   };
} // g3 