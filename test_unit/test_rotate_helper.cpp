/** ==========================================================================
 * 2015 by KjellKod.cc
 *
 * This code is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties. This code is yours to share, use and modify with no
 * strings attached and no restrictions or obligations.
 * ===========================================================================
 * PUBLIC DOMAIN and Not copywrited. First published at KjellKod.cc
 * ********************************************* */

#include "test_rotate_helper.h"
#include <memory>
#include <fstream>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <cerrno>

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__)) && !defined(__MINGW32__)
#include  <io.h>
#define F_OK 0
#else
#include <unistd.h>
#endif

namespace RotateTestHelper {

   /// Ref: StackOverflow + http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
   std::string ReadContent(const std::string filename) {
      // Objects of std::ifstream maintain a filebuf object as their internal 
      // stream buffer, which performs input/output operations on the file they
      // are associated with (if any).
      // File streams are associated with files either on construction, or by
      // calling member open.
      std::ifstream readIn(filename.c_str(), std::ios::in | std::ios::binary);
      // std::ios::operator bool
      // Returns true if readIn has no errors and is ready for I/O operations
      // false otherwise.
      if (readIn) {
         std::shared_ptr<void> raii(nullptr, [&](void*) { readIn.close(); } );

         std::string contents;
         readIn.seekg(0, std::ios::end);
         // std::string::resize
         // void resize (size_t n);
         // If n is smaller than the current string length, the current value 
         // is shortened to its first n character, removing the characters 
         // beyond the nth. 
         // If n is greater than the current string length, the current content
         // is extended by inserting at the end as many characters 
         // (value-initialized - null characters) as needed to reach a size of n
         contents.resize(readIn.tellg());
         readIn.seekg(0, std::ios::beg);
         // std::istream::read
         // istream& read (char* s, streamsize n);
         // Extracts n characters from the stream and stores them in the array 
         // pointed to by s.
         readIn.read(&contents[0], contents.size());
         return contents;
      }
      throw errno;
   }


    std::string ExtractContent(std::map< std::pair<long, long>, std::string, Comparator>& content) {
       std::string extracted = "\n ";
       for (const auto& pair : content) {
          std::string file = pair.second;
          extracted += file + ", \n";
       }
       extracted += "\n";
       return extracted;
    }



    bool Exists(const std::string content, const std::string expected) {
       auto found = content.find(expected);
       return found != std::string::npos;
    }


    bool DoesFileEntityExist(const std::string& pathToFile) {
       // int access(const char *pathname, int mode);
       // access() checks whether the calling process can access the file 
       // pathname. If pathname is a symbolic link, it is dereferenced.
       // The mode specifies the accessibility check(s) to be performed, 
       // and is either the value F_OK, or a mask consisting of the bitwise
       // OR of one or more of R_OK, W_OK, and X_OK. F_OK tests for the 
       // existence of the file. R_OK, W_OK, and X_OK test whether the file 
       // exists and grants read, write, and execute permissions, respectively.
       // On success (all requested permissions granted), zero is returned. 
       // On error (at least one bit in mode asked for a permission that is 
       // denied, or some other error occurred), -1 is returned, and errno is 
       // set appropriately.
       int check = access(pathToFile.c_str(), F_OK);
       bool found = (0 == check);
       if (!found) {
          std::cerr << pathToFile << " was not found: " << std::strerror(errno) 
                    << std::endl;
       }
       return found;
    }

} // RotateTestHelper