//!
//! termcolor
//! ~~~~~~~~~
//!
//! termcolor is a header-only c++ library for printing colored messages
//! to the terminal. Written just for fun with a help of the Force.
//!
//! :copyright: (c) 2013 by Ihor Kalnytskyi
//! :license: BSD, see LICENSE for details
//!

#pragma once

#include "g3log/logmessage.hpp"
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <map>

// Detect target's platform and set some macros in order to wrap platform
// specific code this library depends on.
#if defined(_WIN32) || defined(_WIN64)
   #define TERMCOLOR_TARGET_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
   #define TERMCOLOR_TARGET_POSIX
#endif

// If implementation has not been explicitly set, try to choose one based on
// target platform.
#if !defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES) && !defined(TERMCOLOR_USE_WINDOWS_API) && !defined(TERMCOLOR_USE_NOOP)
   #if defined(TERMCOLOR_TARGET_POSIX)
      #define TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES
      #define TERMCOLOR_AUTODETECTED_IMPLEMENTATION
   #elif defined(TERMCOLOR_TARGET_WINDOWS)
      #define TERMCOLOR_USE_WINDOWS_API
      #define TERMCOLOR_AUTODETECTED_IMPLEMENTATION
   #endif
#endif

// These headers provide isatty()/fileno() functions, which are used for
// testing whether a standard stream refers to the terminal.
#if defined(TERMCOLOR_TARGET_POSIX)
   #include <unistd.h>
#elif defined(TERMCOLOR_TARGET_WINDOWS)
   #include <io.h>
   #include <windows.h>
#endif


namespace termcolor {

   // Forward declaration of the `_internal` namespace.
   // All comments are below.
   namespace _internal {

      inline int colorize_index();
      inline FILE* get_standard_stream(const std::ostream& stream);
      inline bool is_colorized(std::ostream& stream);
      inline bool is_atty(const std::ostream& stream);

#if defined(TERMCOLOR_TARGET_WINDOWS)
      inline void win_change_attributes(std::ostream& stream, int foreground, int background = -1);
#endif
   }

   inline
   std::ostream& colorize(std::ostream& stream) {
      stream.iword(_internal::colorize_index()) = 1L;
      return stream;
   }

   inline
   std::ostream& nocolorize(std::ostream& stream) {
      stream.iword(_internal::colorize_index()) = 0L;
      return stream;
   }

   inline
   std::ostream& reset(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[00m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1, -1);
#endif
      }
      return stream;
   }

   inline
   std::ostream& bold(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[1m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   inline
   std::ostream& dark(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[2m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   inline
   std::ostream& italic(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[3m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   inline
   std::ostream& underline(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[4m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1, COMMON_LVB_UNDERSCORE);
#endif
      }
      return stream;
   }

   inline
   std::ostream& blink(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[5m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   inline
   std::ostream& reverse(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[7m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1, COMMON_LVB_REVERSE_VIDEO);
#endif
      }
      return stream;
   }

   inline
   std::ostream& concealed(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[8m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   inline
   std::ostream& crossed(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[9m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   template <uint8_t code> inline
   std::ostream& color(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         char command[12];
         std::snprintf(command, sizeof(command), "\033[38;5;%dm", code);
         stream << command;
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   template <uint8_t code> inline
   std::ostream& on_color(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         char command[12];
         std::snprintf(command, sizeof(command), "\033[48;5;%dm", code);
         stream << command;
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   template <uint8_t r, uint8_t g, uint8_t b> inline
   std::ostream& color(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         char command[20];
         std::snprintf(command, sizeof(command), "\033[38;2;%d;%d;%dm", r, g, b);
         stream << command;
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   template <uint8_t r, uint8_t g, uint8_t b> inline
   std::ostream& on_color(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         char command[20];
         std::snprintf(command, sizeof(command), "\033[48;2;%d;%d;%dm", r, g, b);
         stream << command;
#elif defined(TERMCOLOR_USE_WINDOWS_API)
#endif
      }
      return stream;
   }

   inline
   std::ostream& black(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[30m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            0
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& red(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[31m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_RED
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& green(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[32m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_GREEN
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& yellow(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[33m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_GREEN | FOREGROUND_RED
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& blue(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[34m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_BLUE
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& magenta(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[35m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_BLUE | FOREGROUND_RED
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& cyan(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[36m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_BLUE | FOREGROUND_GREEN
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& white(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[37m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& grey(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[90m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            0 | FOREGROUND_INTENSITY  // grey (bright black)
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& bright_red(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[91m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_RED | FOREGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& bright_green(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[92m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_GREEN | FOREGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& bright_yellow(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[93m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& bright_blue(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[94m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_BLUE | FOREGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& bright_magenta(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[95m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& bright_cyan(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[96m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& bright_white(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[97m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream,
            FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_black(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[40m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            0
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_red(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[41m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_RED
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_green(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[42m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_GREEN
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_yellow(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[43m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_GREEN | BACKGROUND_RED
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_blue(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[44m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_BLUE
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_magenta(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[45m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_BLUE | BACKGROUND_RED
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_cyan(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[46m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_GREEN | BACKGROUND_BLUE
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_white(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[47m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_grey(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[100m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            0 | BACKGROUND_INTENSITY   // grey (bright black)
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_bright_red(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[101m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_RED | BACKGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_bright_green(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[102m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_GREEN | BACKGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_bright_yellow(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[103m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_bright_blue(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[104m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_BLUE | BACKGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_bright_magenta(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[105m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_bright_cyan(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[106m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }

   inline
   std::ostream& on_bright_white(std::ostream& stream) {
      if (_internal::is_colorized(stream)) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         stream << "\033[107m";
#elif defined(TERMCOLOR_USE_WINDOWS_API)
         _internal::win_change_attributes(stream, -1,
            BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY
         );
#endif
      }
      return stream;
   }


   //! Since C++ hasn't a way to hide something in the header from
   //! the outer access, I have to introduce this namespace which
   //! is used for internal purpose and should't be access from
   //! the user code.
   namespace _internal {
      // An index to be used to access a private storage of I/O streams. See
      // colorize / nocolorize I/O manipulators for details. Due to the fact
      // that static variables ain't shared between translation units, inline
      // function with local static variable is used to do the trick and share
      // the variable value between translation units.
      inline 
      int colorize_index() {
         // std::ios_base::xalloc
         // static int xalloc();
         // Returns a new index value to be used with member functions in the 
         // internal extensible array.
         // The internal extensible array is a general - purpose array of 
         // objects of type long (if accessed with member iword) or void* 
         // (if accessed with member pword).
         // This is a static member function
         static int colorize_index = std::ios_base::xalloc();
         return colorize_index;
      }

      //! Since C++ hasn't a true way to extract stream handler
      //! from the a given `std::ostream` object, I have to write
      //! this kind of hack.
      inline
      FILE* get_standard_stream(const std::ostream& stream) {
         if (&stream == &std::cout)
            return stdout;
         else if ((&stream == &std::cerr) || (&stream == &std::clog))
            return stderr;

         return nullptr;
      }

      // Say whether a given stream should be colorized or not. It's always
      // true for ATTY streams and may be true for streams marked with
      // colorize flag.
      inline
      bool is_colorized(std::ostream& stream) {
         // std::ios_base::iword
         // long& iword (int idx);
         // idx -- An index value for an element of the internal extensible array.
         //        Some implementations expect idx to be a value previously returned
         //        by member xalloc.
         // Returns a reference to the object of type long which corresponds to index
         // idx in the internal extensible array. On failure, a valid long& 
         // initialized to 0L is returned, and (if the stream object inherits from 
         // basic_ios) the badbit state flag is set.
         return is_atty(stream) || static_cast<bool>(stream.iword(colorize_index()));
      }

      //! Test whether a given `std::ostream` object refers to
      //! a terminal.
      inline
      bool is_atty(const std::ostream& stream) {
         FILE* std_stream = get_standard_stream(stream);

         // Unfortunately, fileno() ends with segmentation fault
         // if invalid file descriptor is passed. So we need to
         // handle this case gracefully and assume it's not a tty
         // if standard stream is not detected, and 0 is returned.
         if (!std_stream)
            return false;

#if defined(TERMCOLOR_TARGET_POSIX)
         return ::isatty(fileno(std_stream));
#elif defined(TERMCOLOR_TARGET_WINDOWS)
         return ::_isatty(_fileno(std_stream));
#else
         return false;
#endif
      }

#if defined(TERMCOLOR_TARGET_WINDOWS)
      //! Change Windows Terminal colors attribute. If some
      //! parameter is `-1` then attribute won't changed.
      inline 
      void win_change_attributes(std::ostream& stream, int foreground, int background) {
         // yeah, i know.. it's ugly, it's windows.
         // static WORD defaultAttributes = 0;
        
         // Windows doesn't have ANSI escape sequences and so we use special
         // API to change Terminal output color. That means we can't
         // manipulate colors by means of "std::stringstream" and hence
         // should do nothing in this case.
         if (!_internal::is_atty(stream))
            return;

         // get terminal handle
         HANDLE hTerminal = INVALID_HANDLE_VALUE;
         if (&stream == &std::cout)
            hTerminal = GetStdHandle(STD_OUTPUT_HANDLE);
         else if (&stream == &std::cerr)
            hTerminal = GetStdHandle(STD_ERROR_HANDLE);

         // save default terminal attributes if it unsaved
         //if (!defaultAttributes) {
         //   // If the function succeeds, the return value is nonzero.
         //   // If the function fails, the return value is zero.
         //   CONSOLE_SCREEN_BUFFER_INFO info;
         //   if (!GetConsoleScreenBufferInfo(hTerminal, &info))
         //      return;
         //   defaultAttributes = info.wAttributes;
         //}

         // save default terminal attributes if it unsaved
         static WORD defaultAttributes = [&]() -> WORD {
            CONSOLE_SCREEN_BUFFER_INFO info;
            // If the function succeeds, the return value is nonzero.
            // If the function fails, the return value is zero.
            if (!GetConsoleScreenBufferInfo(hTerminal, &info))
               return 0x07;
            return info.wAttributes;
         }();

         // restore all default settings
         if (foreground == -1 && background == -1) {
            SetConsoleTextAttribute(hTerminal, defaultAttributes);
            return;
         }

         // get current settings
         CONSOLE_SCREEN_BUFFER_INFO info;
         if (!GetConsoleScreenBufferInfo(hTerminal, &info))
            return;

         if (foreground != -1) {
            info.wAttributes &= ~(info.wAttributes & 0x0F);
            info.wAttributes |= static_cast<WORD>(foreground);
         }

         if (background == COMMON_LVB_REVERSE_VIDEO ||
             background == COMMON_LVB_UNDERSCORE) {
            info.wAttributes |= static_cast<WORD>(background);
         }

         if (background != -1 && background != COMMON_LVB_REVERSE_VIDEO &&
             background != COMMON_LVB_UNDERSCORE) {
            info.wAttributes &= ~(info.wAttributes & 0xF0);
            info.wAttributes |= static_cast<WORD>(background);
         }

         SetConsoleTextAttribute(hTerminal, info.wAttributes);
      }
#endif // TERMCOLOR_TARGET_WINDOWS

   } // namespace _internal
} // namespace termcolor


#undef TERMCOLOR_TARGET_POSIX
#undef TERMCOLOR_TARGET_WINDOWS

#if defined(TERMCOLOR_AUTODETECTED_IMPLEMENTATION)
   #undef TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES
   #undef TERMCOLOR_USE_WINDOWS_API
#endif


namespace g3 {

   using TextAttribute = std::ostream&(*)(std::ostream& stream);

   // ======================== Foreground Colors ==============================
   extern const TextAttribute FG_black;
   extern const TextAttribute FG_gray; // bright black
   extern const TextAttribute FG_grey; // bright black

   extern const TextAttribute FG_red;
   extern const TextAttribute FG_green;
   extern const TextAttribute FG_yellow;
   extern const TextAttribute FG_blue;
   extern const TextAttribute FG_magenta;
   extern const TextAttribute FG_cyan;
   extern const TextAttribute FG_white;
   // bright counterparts
   extern const TextAttribute FG_redB;
   extern const TextAttribute FG_greenB;
   extern const TextAttribute FG_yellowB;
   extern const TextAttribute FG_blueB;
   extern const TextAttribute FG_magentaB;
   extern const TextAttribute FG_cyanB;
   extern const TextAttribute FG_whiteB;
   // ======================== Background Colors ==============================
   extern const TextAttribute BG_black;
   extern const TextAttribute BG_gray; // bright black
   extern const TextAttribute BG_grey; // bright black

   extern const TextAttribute BG_red;
   extern const TextAttribute BG_green;
   extern const TextAttribute BG_yellow;
   extern const TextAttribute BG_blue;
   extern const TextAttribute BG_magenta;
   extern const TextAttribute BG_cyan;
   extern const TextAttribute BG_white;
   // bright counterparts
   extern const TextAttribute BG_redB;
   extern const TextAttribute BG_greenB;
   extern const TextAttribute BG_yellowB;
   extern const TextAttribute BG_blueB;
   extern const TextAttribute BG_magentaB;
   extern const TextAttribute BG_cyanB;
   extern const TextAttribute BG_whiteB;
   // ======================== Attribute Manipulators =========================
   // Windows API only supports underline and reverse
   extern const TextAttribute ATTR_bold;
   extern const TextAttribute ATTR_dark;
   extern const TextAttribute ATTR_italic;
   extern const TextAttribute ATTR_underline;
   extern const TextAttribute ATTR_blink;
   extern const TextAttribute ATTR_reverse;
   extern const TextAttribute ATTR_hide;
   extern const TextAttribute ATTR_crossed;
   // ============================ Reset Manipulator ===========================
   extern const TextAttribute reset;


   class ColorCoutSink {
   public:
      ColorCoutSink();
      virtual ~ColorCoutSink();

      void ReceiveLogMessage(LogMessageMover logEntry) {
         LEVELS& level = logEntry.get()._level;
         auto iter = gLevelWithTextAttributes_.find(level);
         if (iter != gLevelWithTextAttributes_.end()) {
            std::vector<TextAttribute>& attributes = iter->second;
            for (int i = 0; i < attributes.size(); i++) {
               std::cout << (*attributes)();
            }
         }

         std::cout << logEntry.get().toString() << termcolor::reset << std::endl;
      }

      void setColorToWhiteBlack();
      void setColorToDefault();
      void setColor(std::map<LEVELS, std::vector<TextAttribute>>& levelAttributes);

   private:
      std::map<LEVELS, std::vector<TextAttribute>> gLevelWithTextAttributes_;

      ColorCoutSink& operator=(const ColorCoutSink&) = delete;
      ColorCoutSink(const ColorCoutSink& other) = delete;
   };

} // namespace g3

