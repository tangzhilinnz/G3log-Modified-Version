#include "g3log/ColorCoutSinkModified.hpp"
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

   static Initialize ___(ColorCoutSink::initWin);
   static Initialize __(ColorCoutSink::initWin);
   static Initialize _(ColorCoutSink::initWin); // make sure that global variable _ is initialized 
                                                // after stdoutDefaultAttrs_ and stderrDefaultAttrs_
                                                // initializations are finished.
#endif // TERMCOLOR_TARGET_WINDOWS

   const LevelsAndSettings ColorCoutSink::k_DEFAULT_SETTINGS = {
      { INFO,                      std::vector<Setting>{ FG_magenta, BG_blueB, ATR_crossed, ATR_blink} },
      { G3LOG_DEBUG,               std::vector<Setting>{ FG_greenB, BG_RGB(112, 23, 45) } },
      { WARNING,                   std::vector<Setting>{ FG_whiteB, ATR_underline, BG_blueB, BG_RGB(112, 23, 45), ATR_bold, ATR_reverse} },
      { FATAL,                     std::vector<Setting>{ FG_grey, BG_256(7), ATR_bold, ATR_crossed, ATR_blink, ATR_italic} },
      { internal::CONTRACT,        std::vector<Setting>{ FG_grey, BG_256(7), ATR_bold, ATR_crossed, ATR_blink, ATR_italic } },
      { internal::FATAL_SIGNAL,    std::vector<Setting>{ FG_grey, BG_256(7), ATR_bold, ATR_crossed, ATR_blink, ATR_italic} },
      { internal::FATAL_EXCEPTION, std::vector<Setting>{ ATR_underline, FG_grey, BG_256(7), ATR_bold, ATR_crossed, ATR_blink, ATR_italic } }
   };

#if defined(TERMCOLOR_TARGET_WINDOWS)
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

#if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         isVirtualTermSeqs_ = enable_virtual_terminal_processing(stdout) &&
                              enable_virtual_terminal_processing(stderr);
#endif

         std::cout << "ColorCoutSink::initWin done !!!" << std::endl;
      };

      std::call_once(g_initWin_flag, initWindows);
   }


   void Attributes::setWinDefaultAttrs() {
      if (pStream_ == &std::cout)
         winAttributes_ = ColorCoutSink::stdoutDefaultAttrs_;
      else if (pStream_ == &std::cerr) {
         winAttributes_ = ColorCoutSink::stderrDefaultAttrs_;
      }
   }

   WORD ColorCoutSink::getDefaultAttributes() {
      if (&stream_ == &std::cout)
         return stdoutDefaultAttrs_;
      else if (&stream_ == &std::cerr) {
          return stderrDefaultAttrs_;
      }
   }

#endif // TERMCOLOR_TARGET_WINDOWS



   Attributes::Attributes(std::ostream* pStream)
      : fgColorEscSeqs_("")
      , bgColorEscSeqs_("")
      , attrEscSeqs_("")
#if defined(TERMCOLOR_TARGET_WINDOWS)
      , pStream_(pStream)
#endif
   {
#if defined(TERMCOLOR_TARGET_WINDOWS)
      if (pStream_ != &std::cout && pStream_ != &std::cerr) {
         std::cerr << "illegal address of std::ostream object used to construct Attributes!!!" << std::endl;
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
      , pStream_(other.pStream_)
#endif
      { }

   Attributes::Attributes(Attributes&& other) noexcept
      : fgColorEscSeqs_(std::move(other.fgColorEscSeqs_))
      , bgColorEscSeqs_(std::move(other.bgColorEscSeqs_))
      , attrEscSeqs_(std::move(other.attrEscSeqs_))
#if defined(TERMCOLOR_TARGET_WINDOWS)
      , winAttributes_(other.winAttributes_)
      , pStream_(other.pStream_)
#endif
      { }

   Attributes& Attributes::operator=(Attributes other) {
      swap(*this, other);
      return *this;
   }


   // -------------------------------------------------------------------------
   
   ColorCoutSink::ColorCoutSink(std::ostream& stream)
      : stream_(stream) 
      , logDetailsFunc_(&LogMessage::DefaultLogDetailsToString) 
      , defaultSettings_(k_DEFAULT_SETTINGS) {

      if (&stream_ != &std::cout && &stream_ != &std::cerr) {
         std::cerr << "illegal std::ostream object used to construct ColorCoutSink!!!" << std::endl;
         abort();
      }

      settingsToWorkingScheme(k_DEFAULT_SETTINGS, true);
   }


   ColorCoutSink::ColorCoutSink(std::ostream& stream, const LevelsAndSettings& defaultSettings)
      : stream_(stream)
      , logDetailsFunc_(&LogMessage::DefaultLogDetailsToString)
      , defaultSettings_(defaultSettings) {

      if (&stream_ != &std::cout && &stream_ != &std::cerr) {
         std::cerr << "illegal std::ostream object used to construct ColorCoutSink!!!" << std::endl;
         abort();
      }
      
      settingsToWorkingScheme(defaultSettings, true);
   }


   ColorCoutSink::~ColorCoutSink() {
      std::string exit_msg{ "g3log color sink shutdown at: " };
      auto now = std::chrono::system_clock::now();
      exit_msg.append(g3::localtime_formatted(now, { g3::internal::date_formatted + " " + g3::internal::time_formatted })).append("\n");
      std::cerr << exit_msg << std::flush;
   }


   void ColorCoutSink::settingsToWorkingScheme(const LevelsAndSettings& toWorkingScheme, bool clearFlag) {
      if (clearFlag) {
         gWorkingScheme_.clear();
      }
      // The insertion operation happens when gWorkingScheme_ does not have 
      // LEVELS (key) equivalent to the key of one element in toWorkingScheme, 
      // otherwise the attributes to LEVELS will be updated to new Settings.
      for (auto iter = toWorkingScheme.cbegin(); iter != toWorkingScheme.cend(); iter++) {

         LEVELS level = iter->first;
         const std::vector<Setting>& settings = iter->second;

         auto schemeItemIter = gWorkingScheme_.find(level);
         if (schemeItemIter != gWorkingScheme_.end()) {
            Attributes& attrs = schemeItemIter->second;           
            for (auto manipulation : settings) {
               (*manipulation)(attrs);
            }
         }
         else { // schemeItemIter == gWorkingScheme_.end()
            Attributes attrs(&stream_);
            for (auto manipulation : settings) {
               (*manipulation)(attrs);
            }

            gWorkingScheme_.insert(std::pair<LEVELS, Attributes>(level, attrs));
         }
      }
   }


   void ColorCoutSink::overrideLogDetails(LogMessage::LogDetailsFunc func) {
      logDetailsFunc_ = func;
   }


   void ColorCoutSink::printLogMessage(LogMessageMover logEntry) {
      LEVELS& level = logEntry.get()._level;
      std::string msg = logEntry.get().toString(logDetailsFunc_);
      auto iter = gWorkingScheme_.find(level);

      if (iter != gWorkingScheme_.end() && is_atty(stream_)) {
         msg = msg.substr(0, msg.size() - 1);
         std::string reset{ "\033[00m" };
         Attributes& attrs = iter->second;
#if defined(TERMCOLOR_TARGET_POSIX)
         std::string msgWithAttrs{ attrs.bgColorEscSeqs_ +
                                   attrs.fgColorEscSeqs_ +
                                   attrs.attrEscSeqs_ + 
                                   msg + reset + "\n" };
         stream_ << msgWithAttrs.c_str() << std::flush;
#elif defined(TERMCOLOR_TARGET_WINDOWS)
   #if defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
         if (isVirtualTermSeqs_) {
            std::string msgWithAttrs{ attrs.bgColorEscSeqs_ +
                                      attrs.fgColorEscSeqs_ +
                                      attrs.attrEscSeqs_ + 
                                      msg + reset+ "\n"};
            stream_ << msgWithAttrs.c_str() << std::flush;
         }
         else
   #endif // TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES
         {
            HANDLE hTerminal = INVALID_HANDLE_VALUE;

            if (&stream_ == &std::cout)
               hTerminal = GetStdHandle(STD_OUTPUT_HANDLE);
            else if (&stream_ == &std::cerr)
               hTerminal = GetStdHandle(STD_ERROR_HANDLE);

            if (hTerminal == INVALID_HANDLE_VALUE) {
               stream_ << msg << std::endl;
            }
            else {
               SetConsoleTextAttribute(hTerminal, attrs.winAttributes_);
               stream_ << msg /*+ "\n" << std::flush*/;
               SetConsoleTextAttribute(hTerminal, getDefaultAttributes());
               stream_ << std::endl;
            }
         }
#endif
      }
      else {
         stream_ << msg << std::flush;
      }
   }


   void ColorCoutSink::setWorkingScheme(const LevelsAndSettings& toWorkingScheme) {
      settingsToWorkingScheme(toWorkingScheme, false);
   }


   void ColorCoutSink::setDefaultScheme(const LevelsAndSettings& defaultSettings) {
      defaultSettings_ = defaultSettings;
   }


   void ColorCoutSink::defaultScheme() {
      settingsToWorkingScheme(defaultSettings_, true);
   }


   void ColorCoutSink::systemDefaultScheme() {
      settingsToWorkingScheme(k_DEFAULT_SETTINGS, true);
   }


   void ColorCoutSink::blackWhiteScheme() {
       gWorkingScheme_.clear();
   }

} // namespace g3