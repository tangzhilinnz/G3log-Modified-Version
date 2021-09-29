#include "ColorCoutSinkModified.hpp"
#include <mutex>


namespace g3 {

using namespace termcolor::_internal;

#if defined(TERMCOLOR_TARGET_WINDOWS)
   // Global variables in a single translation unit (source file) are initialized
   // in the order in which they are defined. The order of initialization of 
   // global variables in different translation units is unspecified. The order 
   // of initialization of globals ignores all dependencies
   static std::once_flag g_initWin_flag;
   WORD ColorCoutSink::stdoutDefaultAttrs_ = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
   WORD ColorCoutSink::stderrDefaultAttrs_ = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
   bool ColorCoutSink::isVirtualTermSeqs_ = false;
   static Initialize _(ColorCoutSink::initWin); // _ is the last global variable to be initialized


   static
   bool enable_virtual_terminal_processing(FILE* stream) {
      HANDLE handle = (HANDLE)_get_osfhandle(_fileno(stream));
      DWORD mode = 0;
      if (!GetConsoleMode(handle, &mode)) {
         return false;
      }
      if (!SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
         return false;
      }
      return true;
   }


   void ColorCoutSink::initWin() {
      auto initWindows = [&] () {
         if (is_atty(std::cout)) {
            HANDLE hTerminal = INVALID_HANDLE_VALUE;
            CONSOLE_SCREEN_BUFFER_INFO info;
            hTerminal = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hTerminal != INVALID_HANDLE_VALUE
                && GetConsoleScreenBufferInfo(hTerminal, &info)) {
               stdoutDefaultAttrs_ = info.wAttributes;
               std::cout << "std::cout done !!!" << stdoutDefaultAttrs_  << std::endl;
            }
         }

         if (is_atty(std::cerr)) {
            HANDLE hTerminal = INVALID_HANDLE_VALUE;
            CONSOLE_SCREEN_BUFFER_INFO info;
            hTerminal = GetStdHandle(STD_ERROR_HANDLE);
            if (hTerminal != INVALID_HANDLE_VALUE
                && GetConsoleScreenBufferInfo(hTerminal, &info)) {
               stderrDefaultAttrs_ = info.wAttributes;
               std::cout << "std::cerr done !!!" << stderrDefaultAttrs_ << std::endl;
            }
         }

         isVirtualTermSeqs_ = enable_virtual_terminal_processing(stdout) &&
                              enable_virtual_terminal_processing(stderr);

         std::cout << "ColorCoutSink::initWin done !!!" << std::endl;
      };

      std::call_once(g_initWin_flag, initWindows);
   }


   void Attributes::setWinDefaultAttrs() {
      if (&stream_ == &std::cout)
         winAttributes_ = ColorCoutSink::stdoutDefaultAttrs_;
      else if (&stream_ == &std::cerr) {
         winAttributes_ = ColorCoutSink::stderrDefaultAttrs_;
      }
   }

#endif // TERMCOLOR_TARGET_WINDOWS



   Attributes::Attributes(std::ostream& stream)
      : fgColorEscSeqs_("")
      , bgColorEscSeqs_("")
      , attrEscSeqs_("")
#if defined(TERMCOLOR_TARGET_WINDOWS)
      , stream_(stream)
#endif
   {
#if defined(TERMCOLOR_TARGET_WINDOWS)
      if (&stream_ != &std::cout && &stream_ != &std::cerr) {
         std::cerr << "g3log: forced abort due to illegal std::ostream object used to construct Attributes!!!" << std::endl;
         abort();
      }
      setWinDefaultAttrs();
#endif
   }

   Attributes::Attributes(const Attributes& other) noexcept
      : fgColorEscSeqs_(other.fgColorEscSeqs_)
      , bgColorEscSeqs_(other.bgColorEscSeqs_)
      , attrEscSeqs_(other.attrEscSeqs_)
#if defined(TERMCOLOR_TARGET_WINDOWS)
      , winAttributes_(other.winAttributes_)
      , stream_(other.stream_)
#endif
      { }

   Attributes::Attributes(Attributes&& other) noexcept
      : fgColorEscSeqs_(std::move(other.fgColorEscSeqs_))
      , bgColorEscSeqs_(std::move(other.bgColorEscSeqs_))
      , attrEscSeqs_(std::move(other.attrEscSeqs_))
#if defined(TERMCOLOR_TARGET_WINDOWS)
      , winAttributes_(other.winAttributes_)
      , stream_(other.stream_)
#endif
      { }

} // namespace g3