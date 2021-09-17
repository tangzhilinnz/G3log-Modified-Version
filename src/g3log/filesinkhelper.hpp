#pragma once


#include <string>
#include <memory>
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>


namespace g3 {
   namespace internal {
      static const std::string file_name_time_formatted = "%Y%m%d-%H%M%S";
      class comp {
      public:
         bool operator()(const std::pair<long, long>& a, const std::pair<long, long>& b) const {
            if (a.first < b.first) return true;
            else if (a.first == b.first) return a.second < b.second;
            else return false;
         }
      };
      //static auto comp = [](const std::pair<long, long>& a, const std::pair<long, long>& b) {
      //   if (a.first < b.first) return true;
      //   else if (a.first == b.first) return a.second < b.second;
      //   else return false; };

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__)) && !defined(__MINGW32__)
      char* strptime(const char* s, const char* f, struct tm* tm);
#endif

      // check for filename validity -  filename should not be part of PATH
      bool isValidFilename(const std::string& prefix_filename);

      /// @return a corrected prefix, if needed,
      /// illegal characters are removed from @param prefix input
      std::string prefixSanityFix(std::string prefix);

      std::string pathSanityFix(std::string path, std::string file_name);

      /// @return the file header
      std::string header(const std::string& headerFormat);

      /// @return result as time from the file name
      bool getDateFromFileName(const std::string& app_name, const std::string& file_name, long& unique_num, long& time_result);

      /**
       * Loop through the files in the folder
       * @param dir
       * @param file_name
       */
      void expireArchives(const std::string& dir, const std::string& app_name, unsigned long max_log_count);

      /// @return all the found files in the directory that follow the expected log name pattern
      /// std::map<long: timestamp, std::string : name>
      std::map<std::pair<long, long>, std::string, comp> getArchiveLogFilesInDirectory(const std::string& dir, const std::string& app_name);

      /// create the file name with creation time and logger_id
      std::string createLogFileName(const std::string& verified_prefix, const std::string& logger_id);

      /// create the file name without creation time and logger_id
      std::string addLogSuffix(const std::string& raw_file_name);

      /// @return true if @param complete_file_with_path could be opened
      /// @param outstream is the file stream
      bool openLogFile(const std::string& complete_file_with_path, std::ofstream& outstream);

      /// create the file
      std::unique_ptr<std::ofstream> createLogFile(const std::string& file_with_full_path);
   

   } // namespace internal
} // namespace g3