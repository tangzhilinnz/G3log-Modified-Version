#include "g3log/filerotatesink.hpp"
#include "g3log/filesinkhelper.hpp"
//#include "filesinkhelper.ipp"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <string>
#include <memory>
#include <fstream>
#include <algorithm>
#include <map>
#include <vector>
#include <ctime>
#include <iostream>
#include <sstream>


namespace g3 {
   using namespace internal;

   FileRotateSink::FileRotateSink(const std::string& log_prefix, const std::string& log_directory, size_t flush_policy)
      : log_details_func_(&LogMessage::DefaultLogDetailsToString)
      , log_file_with_path_(log_directory)
      , log_directory_(log_directory)
      , log_prefix_backup_(log_prefix)
      //, outptr_(new std::ofstream)
      , header_("\t\tLOG format: [YYYY/MM/DD hh:mm:ss uuu* LEVEL FILE->FUNCTION:LINE] message\n\n\t\t(uuu*: microseconds fractions of the seconds value)\n\n")
      //, steady_start_time_(std::chrono::steady_clock::now())
      , flush_policy_(flush_policy)
      , flush_policy_counter_(flush_policy) {

      log_prefix_backup_ = prefixSanityFix(log_prefix);
      max_log_size_ = 524288000;
      max_archive_log_count_ = 10;
      // if (!isValidFilename(log_prefix_backup_)) {
      if (log_prefix_backup_.empty()) {
         std::cerr << "g3log: forced abort due to illegal log prefix [" << log_prefix << "]" << std::endl;
         abort();
      }

      std::string logfile = changeLogFile(log_directory, log_prefix_backup_);
      // assert((nullptr != outptr_) && "cannot open log file at startup");
      assert((!logfile.empty()) && "cannot open log file at startup");
   }


   FileRotateSink::~FileRotateSink() {
      std::string exit_msg{ "g3log FileRotateSink shutdown at: " };
      auto now = std::chrono::system_clock::now();
      exit_msg.append(g3::localtime_formatted(now, { g3::internal::date_formatted + " " + g3::internal::time_formatted })).append("\n");
      filestream() << exit_msg << std::flush;

      exit_msg.append("Log file at: [").append(log_file_with_path_).append("]\n");
      std::cerr << exit_msg << std::flush;
      // std::unique_ptr<std::ofstream> outptr_;
      // Note that any open file is automatically closed when the ofstream 
      // object outptr_ is destroyed as if std::ofstream::close() is called
   }


   std::string FileRotateSink::changeLogFile(const std::string& directory, const std::string& new_name) {
      std::string file_name = new_name;
      if (file_name.empty()) {
         file_name = log_prefix_backup_;
      }

      // e.g., file_name -- "tangzhilin"
      //       directory -- "/my_log_dir///  "
      //       prospect_log -- "/my_log_dir/tangzhilin.log"
      auto prospect_log = /*createPath*/pathSanityFix(directory, file_name);
      prospect_log = addLogSuffix(prospect_log);

      std::unique_ptr<std::ofstream> log_stream = createLogFile(prospect_log);
      // if (nullptr == log_stream) {
      if (!log_stream) {
         if (outptr_) 
            fileWriteWithoutRotate("Unable to change log file. Illegal filename or busy? Unsuccessful log name was:" + prospect_log);
         return ""; // no success
      }
      log_prefix_backup_ = file_name;     // "tangzhilin"
      log_file_with_path_ = prospect_log; // "/my_log_dir/tangzhilin.log"
      log_directory_ = directory;         // "/my_log_dir///  "

      // The dynamically-allocated ofstream object owned by outptr_ before
      // the call is deleted (as if unique_ptr's destructor was called),
      // and the associated open file is automatically closed.
      outptr_ = std::move(log_stream);

      addLogFileHeader();
      setLogSizeCounter();

      return log_file_with_path_;
   }


   /**
 * Rotate the logs once they have exceeded our set size.
 * @return
 */
   bool FileRotateSink::rotateLog() {
      std::ofstream& is(filestream());
      // std::fstream::is_open
      // Returns whether the stream is currently associated to a file.
      // Streams can be associated to files by a successful call to member open 
      // or directly on construction, and disassociated by calling close or on 
      // destruction. The file association of a stream is kept by its internal 
      // stream buffer: Internally, the function calls rdbuf()->is_open()
      if (is.is_open()) {
         is << std::flush;
         std::ostringstream archive_file_name;
         archive_file_name << log_file_with_path_ << ".";
         auto now = std::chrono::system_clock::now();
         archive_file_name << g3::localtime_formatted(now, "%Y-%m-%d-%H-%M-%S");
         archive_file_name << /*".gz"*/".archive"; // "/my_log_dir/tangzhilin.log.%Y-%m-%d-%H-%M-%S.archive"

         if (std::rename(log_file_with_path_.c_str(), archive_file_name.str().c_str())) {
            std::perror("error renaming the rotated file [the size of the file in use has exceeded the maximum value!!!]");
            return false;
         }

         std::string logfile = changeLogFile(log_directory_); // log_directory_ -- "/my_log_dir///  "
         if (logfile.empty()) {
            std::cerr << "cannot create new log file when rotating " 
                      << "[the size of the file in use has exceeded the maximum value!!!]" << std::endl;
            return false;
         }

         std::ostringstream ss;
         ss << "Log rotated Archived file name: " << archive_file_name.str().c_str() << "\n";
         fileWriteWithoutRotate(ss.str());
         ss.clear();
         ss.str("");
         ss << log_prefix_backup_ << ".log"; // ss.str() -- "tangzhilin.log"
         expireArchives(log_directory_, ss.str(), max_archive_log_count_);
         return true;
      }
      return false;
   }


   /**
    * Update the internal counter for the g3 log size
    */
   void FileRotateSink::setLogSizeCounter() {
      std::ofstream& is(filestream());
      // std::ostream::seekp
      // ostream& seekp (streamoff off, ios_base::seekdir way);
      // Sets the position where the next character is to be inserted into the 
      // output stream.
      // @param off -- Offset value, relative to the way parameter. streamoff 
      //               is an offset type(generally, a signed integral type).
      // @param way -- Object of type ios_base::seekdir. It may take any of the
      //               following constant values:
      // value	             offset is relative to...
      // ios_base::beg         beginning of the stream
      // ios_base::cur         current position in the stream
      // ios_base::end	       end of the stream
      // Return Value -- The ostream object (*this).
      is.seekp(0, std::ios::end);
      // std::ostream::tellp
      // streampos tellp();
      // Returns the position of the current character in the output stream.
      // Return Value -- The current position in the stream. If either the stream
      //                 buffer associated to the stream does not support the 
      //                 operation, or if it fails, the function returns -1.
      // streampos is an fpos type(it can be converted to / from integral types).
      cur_log_size_ = is.tellp();
      is.seekp(0, std::ios::beg);
      flush_policy_counter_ = flush_policy_;
   }


   void FileRotateSink::fileWrite(LogMessageMover message) {
      if (cur_log_size_ > max_log_size_) {
         rotateLog();
      }
      fileWriteWithoutRotate(message.get().toString(log_details_func_));
   }


   void FileRotateSink::fileWriteWithoutRotate(std::string message) {
      std::ofstream& out(filestream());
      out << message;
      flushPolicy();
      cur_log_size_ += message.size();
   }


   void FileRotateSink::flushPolicy() {
      if (0 == flush_policy_) return;
      if (0 == --flush_policy_counter_) {
         flush();
         flush_policy_counter_ = flush_policy_;
      }
   }


   void FileRotateSink::setFlushPolicy(size_t flush_policy) {
      flush();
      flush_policy_ = flush_policy;
      flush_policy_counter_ = flush_policy;
   }


   void FileRotateSink::flush() {
      // using std::flush causes the stream buffer to flush its output buffer. 
      // For example, when data is written to a console flushing causes the 
      // characters to appear at this point on the console.
      filestream() << std::flush;
   }


   std::string FileRotateSink::logFileName() {
      return log_file_with_path_;
   }


   void FileRotateSink::overrideLogHeader(const std::string& change) {
      header_ = change;
   }


   void FileRotateSink::overrideLogDetails(LogMessage::LogDetailsFunc func) {
      log_details_func_ = func;
   }


   void FileRotateSink::addLogFileHeader() {
      filestream() << header(header_);
   }


   /**
    * Max number of archived logs to keep.
    * @param max_size
   */
   void FileRotateSink::setMaxArchiveLogCount(int max_size) {
      max_archive_log_count_ = max_size;
   }


   int FileRotateSink::getMaxArchiveLogCount() {
      return max_archive_log_count_;
   }


   /**
    * Set the max file size in bytes.
    * @param max_size
    */
   void FileRotateSink::setMaxLogSize(int max_size) {
      max_log_size_ = max_size;
   }


   int FileRotateSink::getMaxLogSize() {
      return max_log_size_;
   }


} // g3 namespace