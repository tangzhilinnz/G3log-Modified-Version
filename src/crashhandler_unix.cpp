/** ==========================================================================
 * 2011 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties. This code is yours to share, use and modify with no
 * strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
 * ============================================================================*/

#include "g3log/crashhandler.hpp"
#include "g3log/logmessage.hpp"
#include "g3log/logcapture.hpp"
#include "g3log/loglevels.hpp"

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#error "crashhandler_unix.cpp used but it's a windows system"
#endif


#include <csignal>
#include <cstring>
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <map>
#include <mutex>

// Linux/Clang, OSX/Clang, OSX/gcc
#if (defined(__clang__) || defined(__APPLE__))
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif


namespace {

   const std::map<int, std::string> kSignals = {
      {SIGABRT, "SIGABRT"},
      {SIGFPE, "SIGFPE"},
      {SIGILL, "SIGILL"},
      {SIGSEGV, "SIGSEGV"},
      {SIGTERM, "SIGTERM"},
   };

   std::map<int, std::string> gSignals = kSignals;
   std::map<int, struct sigaction> gSavedSigActions;

   //bool shouldDoExit() {
   //   static std::atomic<uint64_t> firstExit{0};
   //   auto const count = firstExit.fetch_add(1, std::memory_order_relaxed);
   //   return /*(0 == count)*/(2 >= count);
   //}

   // Dump of stack,. then exit through g3log background worker
   // ALL thanks to this thread at StackOverflow. Pretty much borrowed from:
   // Ref: http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
   void signalHandler(int signal_number, siginfo_t* /*info*/, void* /*unused_context*/) {

      // Only three/one signal will be allowed past this point
      //if (false == shouldDoExit()) {
      //   while (true) {
      //      std::this_thread::sleep_for(std::chrono::seconds(1));
      //   }
      //}
      // allow any number of signals passing this handler before one fatal crash
      using namespace g3::internal;
      {
         //=============================== for test =================================
         /*std::cout << "entering handler: " << std::this_thread::get_id() << std::endl;*/
         //=============================== for test =================================

         const auto dump = stackdump();
         std::ostringstream fatal_stream;
         const auto fatal_reason = exitReasonName(g3::internal::FATAL_SIGNAL, signal_number);
         fatal_stream << "Received fatal signal: " << fatal_reason;
         fatal_stream << "(" << signal_number << ")\tPID: " << getpid() << std::endl;
         fatal_stream << "\n***** SIGNAL " << fatal_reason << "(" << signal_number << ")" << std::endl;
         LogCapture trigger(FATAL_SIGNAL, static_cast<g3::SignalType>(signal_number), dump.c_str());
         trigger.stream() << fatal_stream.str();

         //=============================== for test =================================
         /*std::cout << "exiting handler: " << std::this_thread::get_id() << std::endl;*/
         //=============================== for test =================================

      } // message sent to g3LogWorker, wait to die
   }

   //
   // Installs FATAL signal handler that is enough to handle most fatal events
   // on *NIX systems
   void installSignalHandler() {
#if !(defined(DISABLE_FATAL_SIGNALHANDLING))
      struct sigaction action;
      // sa_mask: specifies a mask of signals which should be blocked
      // (i.e., added to the signal mask of the thread in which the signal
      // handler is invoked) during execution of the signal handler.
      // In addition, the signal which triggered the handler will be blocked
      // (automatically added to the signal mask of the thread in which the
      // signal handler is invoked), unless the SA_NODEFER flag is used.
      // 
      // When the signal handler is invoked, any signals in sa_mask that
      // are not currently part of the signal mask (thread/process) are 
      // automatically added to the signal mask before the handler is called. 
      // These signals remain in the signal mask until the signal handler 
      // returns, at which time they are automatically removed.
      // In multi-threaded processes, each thread has its own signal mask 
      // and there is no single process signal mask.
      // The sa_mask field specified in act is not allowed to block SIGKILL 
      // or SIGSTOP. Any attempt to do so will be silently ignored.
      sigemptyset(&action.sa_mask);
      // sigaddset(&action.sa_mask, SIGSEGV);

      // Use the sa_sigaction field because the handles has two additional parameters 
      action.sa_sigaction = &signalHandler; // callback to crashHandler for fatal signals
      // sigaction to use sa_sigaction file. ref: 
      // http://www.linuxprogrammingblog.com/code-examples/sigaction
      action.sa_flags = SA_SIGINFO /*| SA_NODEFER*//* | SA_RESETHAND*/;
      // The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, 
      // not sa_handler
      // By default, while signal is being handled it is masked, so it can't be
      // triggered recursively in signal handlers. If masked signal is triggered 
      // by program execution (invalid memory access, segfault, division by 0 etc.),
      // the behavior is undefined.
      // If SIGBUS, SIGFPE, SIGILL, or SIGSEGV are generated while they are 
      // blocked, the result is undefined, unless the signal was generated by
      // kill, sigqueue, or raise.
      // With SA_NODEFER there is no masking, so signal can be handled recursively
      // until stack overflows. And adding SA_RESETHAND would restore default action.
      // If this flag is set, the signal-handling action for the signal is reset to 
      // SIG_DFL and the SA_SIGINFO flag is cleared on entry to the signal-catching 
      // function.
      // (sa_flags regulates how to handle signals to this process)
       
      // do it verbose style - install all signal actions
      for (const auto& sig_pair : gSignals) {
         struct sigaction old_action;
         memset(&old_action, 0, sizeof (old_action));
         /* #include <signal.h>
          * int sigaction(int signum, const struct sigaction* restrict act,
          *   struct sigaction* restrict oldact); 
          * 
          * The sigaction() system call is used to change the action taken by 
          * a process/thread on receipt of a specific signal.
          * signum specifies the signal and can be any valid signal except
          * SIGKILL and SIGSTOP.
          * If act is non-NULL, the new action for signal signum is installed
          * from act. If oldact is non-NULL, the previous action is saved in
          * oldact.
          * sigaction() returns 0 on success; on error, -1 is returned, and
          * errno is set to indicate the error.
          */
         if (sigaction(sig_pair.first, &action, &old_action) < 0) {
            std::string signalerror = "sigaction - " +  sig_pair.second;
            perror(signalerror.c_str());
         } else {
            gSavedSigActions[sig_pair.first] = old_action;
         }
      }
#endif
   }

} // end anonymous namespace




// Redirecting and using signals. In case of fatal signals g3log should log the fatal signal
// and flush the log queue and then "rethrow" the signal to exit
namespace g3 {
   // References:
   // sigaction : change the default action if a specific signal is received
   //             http://linux.die.net/man/2/sigaction
   //             http://publib.boulder.ibm.com/infocenter/aix/v6r1/index.jsp?topic=%2Fcom.ibm.aix.basetechref%2Fdoc%2Fbasetrf2%2Fsigaction.html
   //
   // signal: http://linux.die.net/man/7/signal and
   //         http://msdn.microsoft.com/en-us/library/xdkz3x12%28vs.71%29.asp
   //
   // memset +  sigemptyset: Maybe unnecessary to do both but there seems to be some confusion here
   //          ,plenty of examples when both or either are used
   //          http://stackoverflow.com/questions/6878546/why-doesnt-parent-process-return-to-the-exact-location-after-handling-signal_number
   namespace internal {

      bool shouldBlockForFatalHandling() {
         return true;  // For windows we will after fatal processing change it to false
      }


      /// Generate stackdump. Or in case a stackdump was pre-generated and non-empty just use that one
      /// i.e. the latter case is only for Windows and test purposes
      std::string stackdump(const char* rawdump) {
         if (nullptr != rawdump && !std::string(rawdump).empty()) {
            return {rawdump};
         }

         const size_t max_dump_size = 50;
         void* dump[max_dump_size];
         // backtrace() returns a backtrace for the calling program, in the
         // array pointed to by dump. A backtrace is the series of currently
         // active function calls for the program.
         // 
         // Given the set of addresses returned by backtrace() in dump,
         // backtrace_symbols() translates the addresses into an array of
         // strings that describe the addresses symbolically.
         // The address of the array of string pointers is returned as the
         // function result of backtrace_symbols(). This array is malloced by 
         // backtrace_symbols(), and must be freed by the caller.
         // On success, backtrace_symbols() returns a pointer to the array 
         // malloced by the call; on error, NULL is returned.
         size_t size = backtrace(dump, max_dump_size);
         char** messages = backtrace_symbols(dump, static_cast<int>(size)); // overwrite sigaction with caller's address

         // dump stack: skip first frame, since that is here (stackdump)
         std::ostringstream oss;
         for (size_t idx = 1; idx < size && messages != nullptr; ++idx) {
            // e.g. non-static function -> ./prog(myfunc3+0x5c) [0x80487f0] 
            //      static function -> ./prog [0x8048871]   (cannot extract static func symbol)
            char* mangled_name = 0, *offset_begin = 0, *offset_end = 0;
            // find parentheses and +address offset surrounding mangled name
            for (char* p = messages[idx]; *p; ++p) {
               if (*p == '(') {
                  mangled_name = p;
               } else if (*p == '+') {
                  offset_begin = p;
               } else if (*p == ')') {
                  offset_end = p;
                  break;
               }
            }

            // if the line could be processed, attempt to demangle the symbol
            if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin) {
               *mangled_name++ = '\0';
               *offset_begin++ = '\0';
               *offset_end++ = '\0';

               int status = -1;
               // https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a01696.html
               char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);
               // if demangling is successful, output the demangled function name
               // Returns:
               // A pointer to the start of the NULL - terminated demangled name, 
               // or NULL if the demangling fails. The caller is responsible for 
               // deallocating this memory using free.
               if (status == 0) { // 0: The demangling operation succeeded
                  oss << "\n\tstack dump [" << idx << "]  " << messages[idx] << " : " << real_name << "+";
                  oss << offset_begin << offset_end << std::endl;
               }// otherwise, output the mangled function name
               else {
                  // oss << "\tstack dump [" << idx << "]  " << messages[idx] << mangled_name << "+";
                  oss << "\n\tstack dump [" << idx << "]  " << messages[idx] << " : " << mangled_name << "+";
                  oss << offset_begin << offset_end << std::endl;
               }
               free(real_name); // mallocated by abi::__cxa_demangle(...)
            } else {
               // no demangling done -- just dump the whole line
               // oss << "\tstack dump [" << idx << "]  " << messages[idx] << std::endl;
               oss << "\n\tstack dump [" << idx << "]  " << messages[idx] << std::endl;
            }
         } // END: for(size_t idx = 1; idx < size && messages != nullptr; ++idx)
         free(messages);

         //===================== for test ====================
         /*oss << "thread_id: " << std::this_thread::get_id();*/
         //===================== for test ====================

         return oss.str();
      }


      /// string representation of signal ID
      std::string exitReasonName(const LEVELS& level, g3::SignalType fatal_id) {

         int signal_number = static_cast<int>(fatal_id);
         switch (signal_number) {
            case SIGABRT: return "SIGABRT";
               break;
            case SIGFPE: return "SIGFPE";
               break;
            case SIGSEGV: return "SIGSEGV";
               break;
            case SIGILL: return "SIGILL";
               break;
            case SIGTERM: return "SIGTERM";
               break;
            default:
               std::ostringstream oss;
               oss << "UNKNOWN SIGNAL(" << signal_number << ") for " << level.text;
               return oss.str();
         }
      }


      // Triggered by g3log->g3LogWorker after receiving a FATAL trigger
      // which is LOG(FATAL), CHECK(false) or a fatal signal our signalhandler caught.
      // --- If LOG(FATAL) or CHECK(false) the signal_number will be SIGABRT
      void exitWithDefaultSignalHandler(const LEVELS& level, g3::SignalType fatal_signal_id) {
         const int signal_number = static_cast<int>(fatal_signal_id);
         /*restoreSignalHandler(signal_number);*/
         restoreFatalHandlingToDefault();
         std::cerr << "\n\n" << __FUNCTION__ << ":" << __LINE__ << ". Exiting due to " 
                   << level.text << ", " << signal_number << "   \n\n" << std::flush;

         // int kill(pid_t pid, int sig);
         // The kill() function shall send a signal to a process or a group of 
         // processes specified by pid. The signal to be sent is specified by 
         // sig and is either one from the list given in <signal.h> or 0. If 
         // sig is 0 (the null signal), error checking is performed but no 
         // signal is actually sent. The null signal can be used to check the 
         // validity of pid.
         // getpid() returns the process ID (PID) of the calling process.
         // According to POSIX, all threads in a process should share the same PID
         // 
         // Signal     Meaning                                  Default
         // SIGABRT    Abnormal termination                     Terminate (core dump)
         // SIGFPE     Floating-point error                     Terminate (core dump)
         // SIGILL     Illegal instruction                      Terminate (core dump)
         // SIGSEGV	   Illegal storage access	                Terminate (core dump)
         // SIGTERM	   Termination request sent to the program	Terminate
         kill(getpid(), signal_number);
         exit(signal_number);
      }

      // restores the signal handler back to default for gSignals
      void restoreFatalHandlingToDefault() {
#if !(defined(DISABLE_FATAL_SIGNALHANDLING))
         overrideSetupSignals(/*kSignals*/gSignals);
#endif
      }

   } // end g3::internal


   std::string signalToStr(int signal_number) {
      std::string signal_name;
      // The strsignal() function returns a string describing the signal
      // number passed in signal_number
      const char* signal_name_sz = strsignal(signal_number);

      // From strsignal: On some systems (but not on Linux), NULL may instead
      // be returned for an invalid signal number.
      if (nullptr == signal_name_sz) {
         signal_name = "Unknown signal " + std::to_string(signal_number);
      } else {
         signal_name = signal_name_sz;
      }
      return signal_name;
   }


   void restoreSignalHandler(int signal_number) {
#if !(defined(DISABLE_FATAL_SIGNALHANDLING))
      auto old_action_it = gSavedSigActions.find(signal_number);
      if (old_action_it == gSavedSigActions.end()) {
         return;
      }

      if (sigaction(signal_number, &(old_action_it->second), nullptr) < 0) {
         auto signalname = std::string("sigaction - ") + signalToStr(signal_number);
         perror(signalname.c_str());
      }

      gSavedSigActions.erase(old_action_it);
#endif
   }


   // This will override the default signal handler setup and instead
   // install a custom set of signals to handle
   // restores the signal handler back to default when overrideSignals compares 
   // equal with gSignals
   void overrideSetupSignals(const std::map<int, std::string> overrideSignals) {
      static std::mutex signalLock;
      std::lock_guard<std::mutex> guard(signalLock);
      for (const auto& sig : gSignals) {
         restoreSignalHandler(sig.first);
      }

      if (gSignals == overrideSignals) return;

      gSignals = overrideSignals;
      installCrashHandler(); // installs all the signal handling for the new gSignals
   }


   // installs the signal handling for whatever signal set that is currently active
   // If you want to setup your own signal handling then
   // You should instead call overrideSetupSignals()
   void installCrashHandler() {
      installSignalHandler();
   }
} // end namespace g3
