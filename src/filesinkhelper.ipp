/** ==========================================================================
 * 2013 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties. This code is yours to share, use and modify with no
 * strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
 * ============================================================================*/

#pragma once


#include <memory>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


namespace g3 {
   namespace internal {
      static const std::string file_name_time_formatted = "%Y%m%d-%H%M%S";

      // check for filename validity - filename should not be part of PATH
      bool isValidFilename(const std::string& prefix_filename) {
         std::string illegal_characters("/,|<>:#$%{}[]\'\"^!?+* "); // no '(' and ')'
         // Searches the string for the first character that matches any of the 
         // characters specified in illegal_characters.
         // When pos is specified, the search only includes characters at or 
         // after position pos, ignoring any possible occurrences before pos.
         // Return the position of the first character that matches.
         // If no matches are found, the function returns string::npos.
         size_t pos = prefix_filename.find_first_of(illegal_characters, 0);
         if (pos != std::string::npos) {
            std::cerr << "Illegal character [" << prefix_filename.at(pos) 
                      << "] in logname prefix: " << "[" << prefix_filename << "]" 
                      << std::endl;
            return false;
         } else if (prefix_filename.empty()) {
            std::cerr << "Empty filename prefix is not allowed" << std::endl;
            return false;
         }

         return true;
      }

      std::string prefixSanityFix(std::string prefix) {
         // Erases the sequence of characters in the range [first,last).
         prefix.erase(std::remove_if(prefix.begin(), prefix.end(), ::isspace), prefix.end());
         prefix.erase(std::remove(prefix.begin(), prefix.end(), '/'), prefix.end());
         prefix.erase(std::remove(prefix.begin(), prefix.end(), '\\'), prefix.end());
         prefix.erase(std::remove(prefix.begin(), prefix.end(), '.'), prefix.end());
         prefix.erase(std::remove(prefix.begin(), prefix.end(), ':'), prefix.end());
         if (!isValidFilename(prefix)) {
            return {};
         }
         return prefix;
      }

      /*
       * template<class ForwardIt, class UnaryPredicate> 
       * ForwardIt remove_if(ForwardIt first, ForwardIt last, UnaryPredicate p) {
       *     first = std::find_if(first, last, p);
       *     if (first != last) {
       *         for(ForwardIt i = first; ++i != last; )
       *             if (!p(*i))
       *                 *first++ = std::move(*i);
       *     }
       *     return first;
       * }
       * 
       * template< class ForwardIt, class T >
       * ForwardIt remove(ForwardIt first, ForwardIt last, const T& value) {
       *     first = std::find(first, last, value);
       *     if (first != last) {
       *         for(ForwardIt i = first; ++i != last; )
       *             if (!(*i == value))
       *                 *first++ = std::move(*i);
       *     }
       *     return first;
       * }
       * 
       */


      std::string pathSanityFix(std::string path, std::string file_name) {
         // Unify the delimeters,. maybe sketchy solution but it seems to work
         // on at least win7 + ubuntu. All bets are off for older windows
         std::replace(path.begin(), path.end(), '\\', '/');

         // clean up in case of multiples
         auto contains_end = [&](std::string& in) -> bool {
            size_t size = in.size();
            if (!size) return false;
            char end = in[size - 1];
            return (end == '/' || end == ' ');
         };

         while (contains_end(path)) {
            path.erase(path.size() - 1);
         }

         if (!path.empty()) {
            path.insert(path.end(), '/');
         }

         path.insert(path.size(), file_name);
         return path;
      }

      std::string header(const std::string& headerFormat) {
         std::ostringstream ss_entry;
         // Day Month Date Time Year: is written as "%a %b %d %H:%M:%S %Y" and
         // formatted output as : Wed Sep 19 08:28:16 2012
         auto now = std::chrono::system_clock::now();
         ss_entry << "\t\tg3log created log at: " << g3::localtime_formatted(now, "%a %b %d %H:%M:%S %Y") << "\n";
         ss_entry << headerFormat;         
         return ss_entry.str();
      }

      std::string createLogFileName(const std::string& verified_prefix, const std::string& logger_id) {
         std::stringstream oss_name;
         oss_name << verified_prefix << ".";
         if( logger_id != "" ) {
            oss_name << logger_id << ".";
         }
         auto now = std::chrono::system_clock::now();
         oss_name << g3::localtime_formatted(now, file_name_time_formatted);
         oss_name << ".log";
         return oss_name.str();
      }

      // class std::ofstream in <fstream>
      // Output stream class to operate on files.
      // Objects of this class maintain a filebuf object as their internal
      // stream buffer, which performs input/output operations on the file
      // they are associated with (if any).
      bool openLogFile(const std::string& complete_file_with_path, std::ofstream& outstream) {
         // out: open for writing    trunc: discard the contents of the stream when opening
         std::ios_base::openmode mode = std::ios_base::out; // for clarity: it's really overkill since it's an ofstream
         mode |= std::ios_base::trunc; 
         outstream.open(complete_file_with_path, mode);
         if (!outstream.is_open()) {
            std::ostringstream ss_error;
            ss_error << "FILE ERROR:  could not open log file:[" << complete_file_with_path << "]";
            ss_error << "\n\t\t std::ios_base state = " << outstream.rdstate(); // Returns the current internal error state flags of the stream.
            std::cerr << ss_error.str().c_str() << std::endl;
            outstream.close();
            return false;
         }
         return true;
      }

      std::unique_ptr<std::ofstream> createLogFile(const std::string& file_with_full_path) {
         std::unique_ptr<std::ofstream> out(new std::ofstream);
         std::ofstream& stream(*(out.get()));
         bool success_with_open_file = openLogFile(file_with_full_path, stream);
         if (false == success_with_open_file) {
            // out.release();
            // out.reset(nullptr) destroys the dynamic std::ofstream object currently 
            // managed by out and out becomes empty, managing no object after the call.
            out.reset(nullptr);
         }
         // unique_ptr<T> does not allow copy construction, instead it supports
         // move semantics.
         // A value that is returned from a function is treated as an rvalue,
         // so the move constructor is called automatically.
         return out;
      }

   }
}


/*
 * Before templates, you put the declarations of methods in the header file and 
 * the implementation went to a .cpp file. These files were compiled separately 
 * as their own compilation unit.
 * 
 * With templates, this is no longer possible, almost all template methods need 
 * to be defined in the header. To separate them at least on a logical level, 
 * some people put the declarations in the header but move all implementations 
 * of template methods to .ipp files (i for "inline") and include the .ipp file 
 * at the end of the header.
 */
