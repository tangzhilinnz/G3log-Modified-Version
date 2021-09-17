/** ==========================================================================
 * 2013 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties. This code is yours to share, use and modify with no
 * strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
 * ============================================================================*/

#include "g3log/filesinkhelper.hpp"
#include <memory>
#include <string>
#include "g3log/time.hpp"
#include <algorithm>
#include <regex>
#include <filesystem> // std::filesystem C++ 17
#include <ios>
#include <iomanip>
#include <utility> // std::pair, std::make_pair


namespace g3 {
   namespace internal {
      //static const std::string file_name_time_formatted = "%Y%m%d-%H%M%S";

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


      /// create the file name with creation time and logger_id
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


      /// create the file name without creation time and logger_id
      std::string addLogSuffix(const std::string& verified_prefix) {
         std::stringstream oss_name;
         oss_name << verified_prefix << ".log";
         return oss_name.str();
      }


      // class std::ofstream in <fstream>
      // Output stream class to operate on files.
      // Objects of this class maintain a filebuf object as their internal
      // stream buffer, which performs input/output operations on the file
      // they are associated with (if any).
      bool openLogFile(const std::string& complete_file_with_path, std::ofstream& outstream) {
         // std::ios_base::app      seek to the end of stream before each write
         // std::ios_base::binary   open in binary mode
         // std::ios_base::in       open for reading
         // std::ios_base::out	    open for writing
         // std::ios_base::trunc    discard the contents of the stream when opening
         // std::ios_base::ate      seek to the end of stream immediately after open
         std::ios_base::openmode mode = std::ios_base::out; // for clarity: it's really overkill since it's an ofstream
         mode |= std::ios_base::/*trunc*/app; 
         outstream.open(complete_file_with_path, mode);
         if (!outstream.is_open()) {
            std::ostringstream ss_error;
            ss_error << "FILE ERROR:  could not open log file:[" << complete_file_with_path << "]";
            ss_error << "\n\t\t std::ios_base state = " << outstream.rdstate(); // Returns the current internal error state flags of the stream.
            std::cerr << ss_error.str().c_str() << std::endl;
            // If outstream.close() fails (including if no file was open before the call), 
            // the failbit state flag is set for the stream (which may throw ios_base::failure 
            // if that state flag was registered using member exceptions).
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
            // std::unique_ptr<T, Deleter>::operator bool
            // true if out owns an object, false otherwise.
            out.reset(nullptr); // out.get() == nullptr
         }
         // unique_ptr<T> does not allow copy construction, instead it supports
         // move semantics.
         // A local value that is returned from a function is treated as an rvalue,
         // so the move constructor is called automatically.
         return out;
      }


#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__)) && !defined(__MINGW32__)
      //http://stackoverflow.com/questions/321849/strptime-equivalent-on-windows
      char* strptime(const char* s, const char* f, struct tm* tm) {
         // Isn't the C++ standard lib nice? std::get_time is defined such that its
         // format parameters are the exact same as strptime. Of course, we have to
         // create a string stream first, and imbue it with the current C locale, and
         // we also have to make sure we return the right things if it fails, or
         // if it succeeds, but this is still far simpler an implementation than any
         // of the versions in any of the C standard libraries.
         std::istringstream input(s);
         // std::ios_base::imbue -- locale imbue (const locale& loc);
         // Associates loc to both the stream and its associated stream buffer (if any)
         // as the new locale object to be used with locale-sensitive operations.
         // std::locale::locale -- explicit locale( const char* std_name );
         // Constructs a copy of the system locale with specified std_name (such as "C",
         // or "POSIX", or "en_US.UTF-8", or "English_US.1251"), if such locale is
         // supported by the operating system. The locale constructed in this manner
         // has a name.
         // std::setlocale -- char* setlocale( int category, const char* locale);
         // The setlocale function installs the specified system locale or its portion
         // as the new C locale. The modifications remain in effect and influences the
         // execution of all locale-sensitive C library functions until the next call
         // to setlocale. If locale is a null pointer, setlocale queries the current
         // C locale without modifying it.
         // LC_ALL -- selects the entire C locale
         input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
         // std::get_time -- template< class CharT >
         // /*unspecified*/ get_time(std::tm * tmb, const CharT * fmt);
         // When used in an expression in >> get_time(tmb, fmt), parses the character 
         // input as a date/time value according to format string fmt according to the
         // std::time_get facet of the locale currently imbued in the input stream in. 
         // The resultant value is stored in a std::tm object pointed to by tmb.
         input >> std::get_time(tm, f);
         if (input.fail()) {
            return nullptr;
         }
         // std::istream::tellg -- streampos tellg();
         // Returns the position of the current character in the input stream.
         return (char*)(s + input.tellg());
      }
#endif

      // In c++11 regex, regex compilation (building up a regex object of string) is really done 
      // at program runtime. The best thing you can do for the sake of speed is to construct a 
      // corresponding regex object just once per program run, say, having it declared as a static
      // variable.
      static std::regex date_regex("\\.(\\d{4}-\\d{2}-\\d{2}-\\d{2}-\\d{2}-\\d{2})\\.arc([0-9]+)\\.log");

      /// @return result as time from the file name
      bool getDateFromFileName(const std::string& app_name, const std::string& file_name, long& unique_num, long& time_result) {

         if (file_name.find(app_name) != std::string::npos) {
            std::string suffix = file_name.substr(app_name.size());
            if (suffix.empty()) {
               // this is the main log file
               return false;
            }
            using namespace std;

            // class template -- std::match_results
            // typedef match_results<string::const_iterator> smatch;
            // match_results that holds the results of searching a string    
            // class template -- std::sub_match
            // typedef sub_match<string::const_iterator> ssub_match;
            // sub_match for a matched subexpression in a string    
            // regex -- Class that represents a regular expression
            // regex_match -- Matches a sequence of characters against a regular expression and 
            //                returns true if the entire input sequence matches the expression
            // regex_search -- Finds the first subsequence that matches the regular expression and 
            //                 returns true if there is a substring in the input sequence that matches
            // regex_replace -- Replaces a regular expression using a give format
            // regex_iterator -- Iterator adaptor that calls regex_search to iterate through
            //                   the matches in a string
            // Regular-expression grammars typically use parentheses to denote subexpressions.
            // In addition to providing information about the overall match, the match objects
            // provide access to each matched subexpression in patterns. The submatches are 
            // accessd positionally. The first submatch, which is at position 0, represents 
            // the match for the entire pattern. Each subexpression appears in order thereafter.
            // smatch operations: (sm) 
            // sm.size() -- Zero if the match failed; otherwise, one plus the number of
            //              subexpressions in the most recently matched regular expression.
            // sm.empty() -- true if sm.size() is zero.
            // sm.prefix() -- An ssub_match representing the sequence before the match.
            // sm.suffix() -- An ssub_match representing the sequence after the end of the match.
            // sm.length(n) -- Size of the nth matched subexpression
            // sm.position(n) -- Distance of nth subexpression from the start of the sequence
            // sm.str(n) -- The matched string for the nth subexpression
            // sm[n] -- ssub_match object corresponding to the nth subexpression
            // sm.begin(), sm.end() -- Iterators across the ssub_match elements in sm. 
            // sm.cbegin(), sm.cend() -- As usual, cbegin and cend return const_iterators
            // A few aspects of the ECMAScript regular - expression language:
            // 1) \\{d} represents a single digit and \\{d}{n} represents a sequence of n digits
            // 2) A collection of characters inside square brackets allows a match to any of those
            //    characters. (E.g., [-.] matches a dash, a dot, or a space.Note that a dot has no
            //    special meaning inside brackets.)
            // 3) [^c] says that we want any character that is not 'c'
            // 4) A component followed by ��?�� is optional
            smatch date_match;
            if (regex_match(suffix, date_match, date_regex)) {
               if (date_match.size() == 3) {
                  std::string arc_unique_num_str = date_match[2].str();
                  long arc_unique_num = std::stol(arc_unique_num_str, nullptr);
                  unique_num = arc_unique_num;
                  //return true;
                  std::string date = date_match[1].str();
                  struct tm tm = { 0 };
                  time_t t;
                  // char* strptime(const char *restrict s, const char *restrict format, struct tm* restrict tm);
                  // The strptime() function is the converse of strftime(); 
                  // it converts the character string pointed to by s to values
                  // which are stored in the "broken-down time" structure pointed
                  // to by tm, using the format specified by format.
                  // The strptime() function processes the input string from left to
                  // right. Each of the three possible input elements (whitespace,
                  // literal, or format) are handled one after the other. If the
                  // input cannot be matched to the format string, the function stops.
                  // The remainder of the format and input strings are not processed.
                  // The return value of the function is a pointer to the first
                  // character not processed in this function call. In case the 
                  // input string contains more characters than required by the 
                  // format string, the return value points right after the last
                  // consumed input character. In case the whole input string is 
                  // consumed, the return value points to the null byte at the end
                  // of the string. If strptime() fails to match all of the format
                  // string and therefore an error occurred, the function returns NULL.
                  if (strptime(date.c_str(), "%Y-%m-%d-%H-%M-%S", &tm) == nullptr) {
                     return false;
                  }
                  // time_t mktime (struct tm * timeptr);
                  // Returns the value of type time_t that represents the local time 
                  // described by the tm structure pointed by timeptr (which may be 
                  // modified). If the calendar time cannot be represented, a value 
                  // of -1 is returned.
                  // time_t is generally implemented as an integral value representing
                  // the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC 
                  // (i.e., a unix timestamp).
                  t = mktime(&tm);
                  if (t == -1) {
                     return false;
                  }
                  time_result = (long)t;
                  return true;
               }
            }
         }
         return false;
      }

      /**
       * Loop through the files in the folder
       * @param dir
       * @param file_name
       */
      // TODO, no need to check and record archiving time each entry through this func 
      void expireArchives(const std::string& dir, const std::string& app_name, unsigned long max_log_count) {
         std::map< std::pair<long, long>, std::string, comp > files;
         
         // Constructs the path from a character sequence 
         std::filesystem::path dir_path(dir);

         // directory_iterator defualt consructor -- constructs the end iterator.
         std::filesystem::directory_iterator end_itr;

         // Checks if the given path corresponds to an existing directory.
         if (!std::filesystem::exists(dir_path)) return;

         // std::filesystem::begin(directory_iterator), std::filesystem::end(directory_iterator)
         // directory_iterator begin( directory_iterator iter ) noexcept; 
         // -- Returns iter unchanged
         // directory_iterator end( const directory_iterator& ) noexcept; 
         // -- Returns a default-constructed directory_iterator, which serves
         //    as the end iterator. The argument is ignored.
         // These non-member functions enable the use of directory_iterators 
         // with range-based for loops.
         // std::filesystem::directory_entry::is_regular_file
         // bool is_regular_file() const;
         // -- Checks whether the pointed-to object is a regular file. 
         //    Effectively returns std::filesystem::is_regular_file(status())
         auto dirIter = std::filesystem::directory_iterator(dir_path);
         auto fileCount = std::count_if (
            std::filesystem::begin(dirIter),
            std::filesystem::end(dirIter),
            [](auto& entry) { return entry.is_regular_file(); }
         );

         if (fileCount - 1 <= max_log_count) return;

         // std::filesystem::directory_iterator::directory_iterator
         // explicit directory_iterator( const std::filesystem::path& p );
         // Constructs a directory iterator that refers to the first directory entry of a 
         // directory identified by p. If p refers to an non-existing file or not a 
         // directory, throws std::filesystem::filesystem_error.
         // directory_iterator is a LegacyInputIterator that iterates over the 
         // directory_entry elements of a directory (but does not visit the subdirectories). 
         // The iteration order is unspecified, except that each directory entry is visited
         // only once. The special pathnames dot and dot-dot are skipped.
         // If the directory_iterator reports an error or is advanced past the last 
         // directory entry, it becomes equal to the default-constructed iterator, also 
         // known as the end iterator. Two end iterators are always equal, dereferencing
         // or incrementing the end iterator is undefined behavior.
         // If a file or a directory is deleted or added to the directory tree after the
         // directory iterator has been created, it is unspecified whether the change 
         // would be observed through the iterator.
         // ++itr -- Advances the iterator to the next directory entry.
         for (std::filesystem::directory_iterator itr(dir_path); itr != end_itr; ++itr) {
            // std::filesystem::directory_entry::path
            // const std::filesystem::path& path() const noexcept;
            // -- Returns the full path the directory entry refers to.
            // std::filesystem::path::filename
            // path filename() const;
            // -- Returns the generic-format filename component of the path.
            // std::filesystem::path::string
            // std::string string() const;
            // -- Returns the internal pathname in native pathname format, 
            // converted to specific string type.
            std::string current_file(itr->path().filename().string());
            long unique_num = 0;
            long time = 0;
            if (getDateFromFileName(app_name, current_file, unique_num, time)) {
               std::pair<long, long> key = std::make_pair(time, unique_num);
               //files.insert(std::pair<long, std::string >(unique_num, current_file));
               files.insert(std::make_pair(key, current_file));
            }
         }

         // delete old logs.
         // ptrdiff_t -- Defined in header <stddef.h>
         // ptrdiff_t is the signed integer type of the result of subtracting two
         // pointers. Only pointers to elements of the same array (including the
         // pointer one past the end of the array) may be subtracted from each other.
         ptrdiff_t logs_to_delete = files.size() - max_log_count;
         if (logs_to_delete > 0) {
            // std::map is already sorted in ascending order of keys(time)
            for (auto it = files.begin(); it != files.end(); ++it) {
               if (logs_to_delete <= 0) {
                  break;
               }

               std::string filename_with_path(/*createPath*/pathSanityFix(dir, it->second));
               // int remove ( const char * filename );
               // Deletes the file whose name is specified in filename.
               remove(filename_with_path.c_str());
               --logs_to_delete;
            }
         }
      }


      //std::map<long, std::string> getArchiveLogFilesInDirectory(const std::string& dir, const std::string& app_name) {
      std::map<std::pair<long, long>, std::string, comp> getArchiveLogFilesInDirectory(const std::string& dir, const std::string& app_name) {
         //std::map<long, std::string> files;
         //auto comp = [](const std::pair<long, long>& a, const std::pair<long, long>& b) {
         //               if (a.first < b.first) return true;
         //               else if (a.first == b.first) return a.second < b.second;
         //               else return false; };
         std::map< std::pair<long, long>, std::string, comp > files;

         std::filesystem::path dir_path(dir);

         std::filesystem::directory_iterator end_itr;
         if (!std::filesystem::exists(dir_path)) return {};

         for (std::filesystem::directory_iterator itr(dir_path); itr != end_itr; ++itr) {
            std::string current_file(itr->path().filename().string());
            long unique_num = 0;
            long time = 0;
            if (getDateFromFileName(app_name, current_file, unique_num, time)) {
               std::pair<long, long> key = std::make_pair(time, unique_num);
               //files.insert(std::pair<long, std::string >(unique_num, current_file));
               files.insert(std::make_pair(key, current_file));
            }
         }

         return files;
      }


   } // namespace internal
} // namespace g3


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
