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
#include <utility>
#include <stdexcept>


// ======================== Foreground Colors ==============================
#define  FG_256(code)     termcolor::color<code>
#define  FG_RGB(r, g, b)  termcolor::color<r, g, b>

#define  FG_black         termcolor::black
#define  FG_gray          termcolor::grey // bright black
#define  FG_grey          termcolor::grey // bright black

#define  FG_red           termcolor::red
#define  FG_green         termcolor::green
#define  FG_yellow        termcolor::yellow
#define  FG_blue          termcolor::blue
#define  FG_magenta       termcolor::magenta
#define  FG_cyan          termcolor::cyan
#define  FG_white         termcolor::white
// bright counterparts
#define  FG_redB          termcolor::bright_red
#define  FG_greenB        termcolor::bright_green
#define  FG_yellowB       termcolor::bright_yellow
#define  FG_blueB         termcolor::bright_blue
#define  FG_magentaB      termcolor::bright_magenta
#define  FG_cyanB         termcolor::bright_cyan
#define  FG_whiteB        termcolor::bright_white

// ======================== Background Colors ==============================
#define  BG_256(code)     termcolor::on_color<code>
#define  BG_RGB(r, g, b)  termcolor::on_color<r, g, b>

#define  BG_black         termcolor::on_black
#define  BG_gray          termcolor::on_grey // bright black
#define  BG_grey          termcolor::on_grey // bright black
   
#define  BG_red           termcolor::on_red
#define  BG_green         termcolor::on_green
#define  BG_yellow        termcolor::on_yellow
#define  BG_blue          termcolor::on_blue
#define  BG_magenta       termcolor::on_magenta
#define  BG_cyan          termcolor::on_cyan
#define  BG_white         termcolor::on_white
// bright counterparts
#define  BG_redB          termcolor::on_bright_red
#define  BG_greenB        termcolor::on_bright_green
#define  BG_yellowB       termcolor::on_bright_yellow
#define  BG_blueB         termcolor::on_bright_blue
#define  BG_magentaB      termcolor::on_bright_magenta
#define  BG_cyanB         termcolor::on_bright_cyan
#define  BG_whiteB        termcolor::on_bright_white

// ======================== Attribute Manipulators =========================
// Windows API only supports underline and reverse when 
// TERMCOLOR_USE_WINDOWS_API is defined
#define  ATTR_bold        termcolor::bold
#define  ATTR_dark        termcolor::dark
#define  ATTR_italic      termcolor::italic
#define  ATTR_underline   termcolor::underline
#define  ATTR_blink       termcolor::blink
#define  ATTR_reverse     termcolor::reverse
#define  ATTR_hide        termcolor::concealed
#define  ATTR_crossed     termcolor::crossed

// ============================ Reset Manipulator ===========================
#define  RESET            termcolor::reset


// Detect target's platform and set some macros in order to wrap platform
// specific code this library depends on.
#if defined(_WIN32) || defined(_WIN64)
   #define TERMCOLOR_TARGET_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
   #define TERMCOLOR_TARGET_POSIX
#endif


#define TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES
// If implementation has not been explicitly set, try to choose one based on
// target platform.
//#if !defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES) && !defined(TERMCOLOR_USE_WINDOWS_API) && !defined(TERMCOLOR_USE_NOOP)
//   #if defined(TERMCOLOR_TARGET_POSIX)
//      #define TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES
//      #define TERMCOLOR_AUTODETECTED_IMPLEMENTATION
//   #elif defined(TERMCOLOR_TARGET_WINDOWS)
//      #define TERMCOLOR_USE_WINDOWS_API
//      #define TERMCOLOR_AUTODETECTED_IMPLEMENTATION
//   #endif
//#endif

// These headers provide isatty()/fileno() functions, which are used for
// testing whether a standard stream refers to the terminal.
#if defined(TERMCOLOR_TARGET_POSIX)
   #include <unistd.h>
#elif defined(TERMCOLOR_TARGET_WINDOWS)
   #include <io.h>
   #include <windows.h>
#endif


namespace g3 {
   struct Attributes {
      explicit Attributes(std::ostream* pStream);
      Attributes(const Attributes& other) noexcept;
      Attributes(Attributes&& other) noexcept;
      Attributes& operator=(Attributes other);

      std::string fgColorEscSeqs_;
      std::string bgColorEscSeqs_;
      std::string attrEscSeqs_;
      
      friend void swap(Attributes& first, Attributes& second) {
         using std::swap; // enable ADL
         swap(first.fgColorEscSeqs_, second.fgColorEscSeqs_);
         swap(first.bgColorEscSeqs_, second.bgColorEscSeqs_);
         swap(first.attrEscSeqs_, second.attrEscSeqs_);

#if defined(TERMCOLOR_TARGET_WINDOWS)
         swap(first.pStream_, second.pStream_);
         swap(first.winAttributes_, second.winAttributes_);
#endif
      }

#if defined(TERMCOLOR_TARGET_WINDOWS)
      std::ostream* pStream_;
      WORD winAttributes_;

      void setWinDefaultAttrs();
#endif
   };
}

namespace termcolor {

   // Forward declaration of the `_internal` namespace.
   // All comments are below.
   namespace _internal {

      inline int colorize_index();
      inline FILE* get_standard_stream(const std::ostream& stream);
      inline bool is_colorized(std::ostream& stream);
      inline bool is_atty(const std::ostream& stream);

#if defined(TERMCOLOR_TARGET_WINDOWS)
      //inline void win_change_attributes(std::ostream& stream, int foreground, int background = -1);
      inline void win_change_attributes(g3::Attributes& attrs, int foreground, int background = -1);
#endif
   }


   //inline
   //std::ostream& colorize(std::ostream& stream) {
   //   stream.iword(_internal::colorize_index()) = 1L;
   //   return stream;
   //}

   //inline
   //std::ostream& nocolorize(std::ostream& stream) {
   //   stream.iword(_internal::colorize_index()) = 0L;
   //   return stream;
   //}

   static inline
   void reset(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = "";
      attrs.bgColorEscSeqs_ = "";
      attrs.attrEscSeqs_ = "";
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1, -1);
#endif
   }

   static inline
   void bold(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.attrEscSeqs_.append("\033[1m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   static inline
   void dark(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.attrEscSeqs_.append("\033[2m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   static inline
   void italic(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.attrEscSeqs_.append("\033[3m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   static inline
   void underline(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.attrEscSeqs_.append("\033[4m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1, COMMON_LVB_UNDERSCORE);
#endif
   }

   static inline
   void blink(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.attrEscSeqs_.append("\033[5m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   static inline
   void reverse(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.attrEscSeqs_.append("\033[7m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1, COMMON_LVB_REVERSE_VIDEO);
#endif
   }

   static inline
   void concealed(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.attrEscSeqs_.append("\033[8m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   static inline
   void crossed(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.attrEscSeqs_.append("\033[9m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   template <uint8_t code> 
   static inline
   void color(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      char ansiEscSeqs[20];
      int n = std::snprintf(ansiEscSeqs, sizeof(ansiEscSeqs), "\033[38;5;%dm", code);
      if (n < 0) return;
      attrs.fgColorEscSeqs_ = std::string(ansiEscSeqs, n);
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   template <uint8_t code> 
   static inline
   void on_color(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      char ansiEscSeqs[20];
      int n =  std::snprintf(ansiEscSeqs, sizeof(ansiEscSeqs), "\033[48;5;%dm", code);
      if (n < 0) return;
      attrs.bgColorEscSeqs_ = std::string(ansiEscSeqs, n);
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   template <uint8_t r, uint8_t g, uint8_t b> 
   static inline
   void color(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      char ansiEscSeqs[20];
      int n = std::snprintf(ansiEscSeqs, sizeof(ansiEscSeqs), "\033[38;2;%d;%d;%dm", r, g, b);
      if (n < 0) return;
      attrs.fgColorEscSeqs_ = std::string(ansiEscSeqs, n);
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   template <uint8_t r, uint8_t g, uint8_t b>
   static inline
   void on_color(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      char ansiEscSeqs[20];
      int n = std::snprintf(ansiEscSeqs, sizeof(ansiEscSeqs), "\033[48;2;%d;%d;%dm", r, g, b);
      if (n < 0) return;
      attrs.bgColorEscSeqs_ = std::string(ansiEscSeqs, n);
#if defined(TERMCOLOR_TARGET_WINDOWS)
#endif
   }

   static inline
   void black(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[30m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         0
      );
#endif
   }

   static inline
   void red(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[31m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_RED
      );
#endif
   }

   static inline
   void green(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[32m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_GREEN
      );
#endif
   }

   static inline
   void yellow(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[33m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_GREEN | FOREGROUND_RED
      );
#endif
   }

   static inline
   void blue(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[34m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_BLUE
      );
#endif
   }

   static inline
   void magenta(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[35m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_BLUE | FOREGROUND_RED
      );
#endif
   }

   static inline
   void cyan(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[36m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_BLUE | FOREGROUND_GREEN
      );
#endif
   }

   static inline
   void white(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[37m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
      );
#endif
   }

   static inline
   void grey(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[90m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         0 | FOREGROUND_INTENSITY  // grey (bright black)
      );
#endif
   }

   static inline
   void bright_red(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[91m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_RED | FOREGROUND_INTENSITY
      );
#endif
   }

   static inline
   void bright_green(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[92m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_GREEN | FOREGROUND_INTENSITY
      );
#endif
   }

   static inline
   void bright_yellow(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[93m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
      );
#endif
   }

   static inline
   void bright_blue(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[94m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_BLUE | FOREGROUND_INTENSITY
      );
#endif
   }

   static inline
   void bright_magenta(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[95m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY
      );
#endif
   }

   static inline
   void bright_cyan(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[96m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY
      );
#endif
   }

   static inline
   void bright_white(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.fgColorEscSeqs_ = std::string("\033[97m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs,
         FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
      );
#endif
   }

   static inline
   void on_black(g3::Attributes& attrs) {
#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[40m");
#elif defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         0
      );
#endif
   }

   static inline
   void on_red(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[41m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_RED
      );
#endif
   }

   static inline
   void on_green(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[42m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_GREEN
      );
#endif
   }

   static inline
   void on_yellow(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[43m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_GREEN | BACKGROUND_RED
      );
#endif
   }

   static inline
   void on_blue(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[44m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_BLUE
      );
#endif
   }

   static inline
   void on_magenta(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[45m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_BLUE | BACKGROUND_RED
      );
#endif
   }

   static inline
   void on_cyan(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[46m");
#if defined(TERMCOLOR_TARGET_WINDOWS)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_GREEN | BACKGROUND_BLUE
      );
#endif
   }

   static inline
   void on_white(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[47m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED
      );
#endif
   }

   static inline
   void on_grey(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[100m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         0 | BACKGROUND_INTENSITY   // grey (bright black)
      );
#endif
   }

   static inline
   void on_bright_red(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[101m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_RED | BACKGROUND_INTENSITY
      );
#endif
   }

   static inline
   void on_bright_green(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[102m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_GREEN | BACKGROUND_INTENSITY
      );
#endif
   }

   static inline
   void on_bright_yellow(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[103m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY
      );
#endif
   }

   static inline
   void on_bright_blue(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[104m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_BLUE | BACKGROUND_INTENSITY
      );
#endif
   }

   static inline
   void on_bright_magenta(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[105m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY
      );
#endif
   }

   static inline
   void on_bright_cyan(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[106m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY
      );
#endif
   }

   static inline
   void on_bright_white(g3::Attributes& attrs) {
//#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
      attrs.bgColorEscSeqs_ = std::string("\033[107m");
#if defined(TERMCOLOR_TARGET_WINDOWS)
      _internal::win_change_attributes(attrs, -1,
         BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY
      );
#endif
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
      void win_change_attributes(/*std::ostream& stream*/g3::Attributes& attrs, int foreground, int background) {
         // yeah, i know.. it's ugly, it's windows.
         // static WORD defaultAttributes = 0;
        
         // Windows doesn't have ANSI escape sequences and so we use special
         // API to change Terminal output color. That means we can't
         // manipulate colors by means of "std::stringstream" and hence
         // should do nothing in this case.
         /*if (!_internal::is_atty(stream))
            return;*/

         // get terminal handle
         HANDLE hTerminal = INVALID_HANDLE_VALUE;
         if (attrs.pStream_ == &std::cout)
            hTerminal = GetStdHandle(STD_OUTPUT_HANDLE);
         else if (attrs.pStream_ == &std::cerr)
            hTerminal = GetStdHandle(STD_ERROR_HANDLE);
         if (hTerminal == INVALID_HANDLE_VALUE)
            return;

         // restore all default settings
         if (foreground == -1 && background == -1) {
            attrs.setWinDefaultAttrs();
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

         attrs.winAttributes_ = info.wAttributes;

         //SetConsoleTextAttribute(hTerminal, info.wAttributes);
      }
#endif // TERMCOLOR_TARGET_WINDOWS


   } // namespace _internal
} // namespace termcolor


namespace {

   /**
    * This class is used to manage once-only initialization that should occur
    * before main() is invoked, such as the creation of static variables.  It
    * also provides a mechanism for handling dependencies (where one class
    * needs to perform its once-only initialization before another).
    *
    * The simplest way to use an Initialize object is to define a static
    * initialization method for a class, say Foo::init().  Then, declare
    * a static Initialize object in the class:
    * "static Initialize initializer(Foo::init);".
    * The result is that Foo::init will be invoked when the object is
    * constructed (before main() is invoked).  Foo::init can create static
    * objects and perform any other once-only initialization needed by the
    * class.  Furthermore, if some other class needs to ensure that Foo has
    * been initialized (e.g. as part of its own initialization) it can invoke
    * Foo::init directly (Foo::init should contain an internal guard so that
    * it only performs its functions once, even if invoked several times).
    *
    * There is also a second form of constructor for Initialize that causes a
    * new object to be dynamically allocated and assigned to a pointer, instead
    * of invoking a function. This form allows for the creation of static objects
    * that are never destructed (thereby avoiding issues with the order of
    * destruction).
    */
   class Initialize {
   public:
      /**
       * This form of constructor causes its function argument to be invoked
       * when the object is constructed.  When used with a static Initialize
       * object, this will cause #func to run before main() runs, so that
       * #func can perform once-only initialization.
       *
       * \param func
       *      This function is invoked with no arguments when the object is
       *      constructed.  Typically the function will create static
       *      objects and/or invoke other initialization functions.  The
       *      function should normally contain an internal guard so that it
       *      only performs its initialization the first time it is invoked.
       */
       Initialize(void (*func)()) {
          (*func)();
       }

       /**
        * This form of constructor causes a new object of a particular class
        * to be constructed with a no-argument constructor and assigned to a
        * given pointer.  This form is typically used with a static Initialize
        * object: the result is that the object will be created and assigned
        * to the pointer before main() runs.
        *
        * \param p
        *      Pointer to an object of any type. If the pointer is NULL then
        *      it is replaced with a pointer to a newly allocated object of
        *      the given type.
        */
       template<typename T>
       explicit Initialize(T*& p) {
          if (p == NULL) {
             p = new T;
          }
       }
   };

} // anonymous namespace


namespace g3 {

   using Setting = void (*)(g3::Attributes& attrs);
   using LevelsAndSettings = std::map<LEVELS, std::vector<Setting> >;

   class ColorCoutSink {
      // friend class declaration
      friend struct Attributes;

      using LevelsWithAttributes = std::map<LEVELS, Attributes>;

   public:
      explicit ColorCoutSink(std::ostream& stream);
      ColorCoutSink(std::ostream& stream, const LevelsAndSettings& toCustomScheme);
      virtual ~ColorCoutSink();

      void defaultScheme();
      void customScheme();
      void blackWhiteScheme();
      void customizetWorkingScheme(const LevelsAndSettings& toWorkingScheme);

      void overrideLogDetails(LogMessage::LogDetailsFunc func);

      void printLogMessage(LogMessageMover logEntry);

   private:
      void settingsToWorkingScheme(const LevelsAndSettings& toWorkingScheme);

      ColorCoutSink& operator=(const ColorCoutSink&) = delete;
      ColorCoutSink(const ColorCoutSink& other) = delete;

      static const LevelsAndSettings k_DEFAULT_SETTINGS;
      LevelsAndSettings customSettings_;

      LevelsWithAttributes gWorkingScheme_;

      std::ostream& stream_;

      LogMessage::LogDetailsFunc logDetailsFunc_;

#if defined(TERMCOLOR_TARGET_WINDOWS)
   public:
      static void initWin();
      // for test
      static bool isVirtualTermSeqs() {
         return isVirtualTermSeqs_;
      }

   private:
      WORD getDefaultAttributes();

      static WORD stdoutDefaultAttrs_;
      static WORD stderrDefaultAttrs_;
      static bool isVirtualTermSeqs_;
#endif
   };


} // g3


//#undef TERMCOLOR_TARGET_POSIX
//#undef TERMCOLOR_TARGET_WINDOWS
//
//#if defined(TERMCOLOR_AUTODETECTED_IMPLEMENTATION)
//   #undef TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES
//   #undef TERMCOLOR_USE_WINDOWS_API
//#endif


// Some rules about definitions (types and functions) in c++ header files:

// Doesn¡¯t defining a class in a header file violate the one-definition rule?
// It shouldn¡¯t. If your header file has proper header guards, it shouldn¡¯t be 
// possible to include the class definition more than once into the same file.
// Types (which include classes), are exempt from the part of the one-definition 
// rule that says you can only have one definition per program. Therefore, there
// isn¡¯t an issue #including class definitions into multiple code files (if there
// was, classes wouldn¡¯t be of much use).

// Doesn¡¯t defining member functions in the header violate the one-definition rule?
// It depends. Member functions defined inside the class definition are considered 
// implicitly inline. Inline functions are exempt from the one definition per program
// part of the one-definition rule. This means there is no problem defining trivial 
// member functions (such as access functions) inside the class definition itself.
// Member functions defined outside the class definition are treated like normal 
// functions, and are subject to the one definition per program part of the 
// one-definition rule. Therefore, those functions should be defined in a .cpp code
// file, not inside the header. The one exception for this is for template functions.

// Default parameters:
// Default parameters for member functions should be declared in the class definition
// (in the header file), where they can be seen by whomever #includes the header.

// Libraries:
// Most 3rd party libraries provide only header files, along with a precompiled library
// file. There are several reasons for this: 
// 1) It¡¯s faster to link a precompiled library than to recompile it every time
//    you need it;
// 2) a single copy of a precompiled library can be shared by many applications, 
//    whereas compiled code gets compiled into every executable that uses it 
//    (inflating file sizes);
// 3) intellectual property reasons (you don¡¯t want people stealing your code).
//

