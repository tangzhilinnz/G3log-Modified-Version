#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <g3log/ColorCoutSink.hpp>

#include <iomanip>
#include <thread>
#include <iostream>
#include <fstream> 
#include <memory>


namespace example_fatal
{
   // on Ubunti this caused get a compiler warning with gcc4.6
   // from gcc 4.7.2 (at least) it causes a crash (as expected)
   // On windows it'll probably crash too.
   void tryToKillWithIllegalPrintout() {
      std::cout << "\n\n***** Be ready this last example may 'abort' if on Windows/Linux_gcc4.7 " << std::endl << std::flush;
      std::cout << "************************************************************\n\n" << std::endl << std::flush;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      const std::string logging = "logging";
      LOGF(G3LOG_DEBUG,
           "ILLEGAL PRINTF_SYNTAX EXAMPLE. WILL GENERATE compiler warning.\n\nbadly formatted message:[Printf-type %s is the number 1 for many %s]",
           logging.c_str());
   }


   // The function above 'tryToKillWithIllegalPrintout' IS system / compiler dependent. Older compilers sometimes did NOT generate a SIGSEGV
   // fault as expected by the illegal printf-format usage. just in case we exit by zero division"
   void  killByZeroDivision(int value) {
      int zero = 0; // trying to fool the compiler to automatically warn
      LOG(INFO) << "This is a bad operation [value/zero] : " << value / zero;
   }


   void tryToKillWithAccessingIllegalPointer(std::unique_ptr<std::string> badStringPtr) {
      auto badPtr = std::move(badStringPtr);
      LOG(INFO) << "Function calls through a nullptr object will trigger SIGSEGV";
      badStringPtr->append("crashing");
   }

} // example fatal


int main(int argc, char** argv) {
   double pi_d = 3.1415926535897932384626433832795;
   float pi_f = 3.1415926535897932384626433832795f;

   using namespace g3;

   std::filebuf fb;
   fb.open("test.txt", std::ios::out);
   std::ostream os(&fb);

   std::unique_ptr<LogWorker> logworker{ LogWorker::createLogWorker() };

   auto coutSinkHandle = logworker->addSink(
      std::make_unique<ColorCoutSink>(
         /*os*//*std::cerr*/std::cout,
         LevelsAndSettings{
            { G3LOG_DEBUG,               std::vector<Setting>{ FG_cyanB} },
            { WARNING,                   std::vector<Setting>{ FG_yellow} },
            { FATAL,                     std::vector<Setting>{ FG_red} },
            { internal::CONTRACT,        std::vector<Setting>{ FG_red} },
            { internal::FATAL_SIGNAL,    std::vector<Setting>{ FG_red} },
            { internal::FATAL_EXCEPTION, std::vector<Setting>{ FG_red} } }),
      &ColorCoutSink::printLogMessage);

   initializeLogging(logworker.get());
   //std::future<std::string> log_file_name = sinkHandle->call(&FileSink::fileName);
   std::cout << "*   This is an example of g3log. It WILL exit by a FATAL trigger" << std::endl;
   std::cout << "*   Please see the generated log and compare to the code at" << std::endl;
   std::cout << "*   g3log/test_example/main.cpp" << std::endl;
   //std::cout << "*\n* Log file: [" << log_file_name.get() << "]\n\n" << std::endl;


   LOGF(INFO, "Hi log %d", 123);
   LOG(INFO) << "Test SLOG INFO";
   LOGF(WARNING, "Printf-style syntax is also %s", "available");
   LOG(G3LOG_DEBUG) << "Test SLOG DEBUG";
   LOG(INFO) << "one: " << 1;
   LOG(INFO) << "two: " << 2;
   LOG(INFO) << "one and two: " << 1 << " and " << 2;
   LOG(WARNING) << "Warning information";
   LOG(G3LOG_DEBUG) << "float 2.14: " << 1000 / 2.14f;
   LOG(G3LOG_DEBUG) << "pi double: " << pi_d;
   LOG(G3LOG_DEBUG) << "pi float: " << pi_f;
   LOG(G3LOG_DEBUG) << "pi float (width 10): " << std::setprecision(10) << pi_f;
   LOGF(INFO, "pi float printf:%f", pi_f);

   //
   // START: LOG Entris that were in the CodeProject article
   //
   //LOG(UNKNOWN_LEVEL) << "This log attempt will cause a compiler error";

   //coutSinkHandle->call(&ColorCoutSink::systemDefaultScheme).wait();
   logworker->hotUpdateSink(coutSinkHandle.get(), &ColorCoutSink::systemDefaultScheme);

   LOG(INFO) << "Simple to use with streaming syntax, easy as abc or " << 123;
   LOGF(WARNING, "Printf-style syntax is also %s", "available");
   LOG_IF(INFO, (1 < 2)) << "If true this text will be logged";
   LOGF_IF(INFO, (1 < 2), "if %d<%d : then this text will be logged", 1, 2);
   LOG_IF(FATAL, (2 > 3)) << "This message should NOT throw";
   LOGF(G3LOG_DEBUG, "This API is popular with some %s", "programmers");
   LOGF_IF(G3LOG_DEBUG, (1 < 2), "If true, then this %s will be logged", "message");
   
   //coutSinkHandle->call(&ColorCoutSink::defaultScheme).wait();
   logworker->hotUpdateSink(coutSinkHandle.get(), &ColorCoutSink::defaultScheme);

   LOGF(INFO, "Hi log %d", 123);
   LOG(INFO) << "Test SLOG INFO";
   LOGF(WARNING, "Printf-style syntax is also %s", "available");
   LOG(G3LOG_DEBUG) << "Test SLOG DEBUG";
   LOG(INFO) << "one: " << 1;
   LOG(INFO) << "two: " << 2;
   LOG(INFO) << "one and two: " << 1 << " and " << 2;
   LOG(WARNING) << "Warning information";
   LOG(G3LOG_DEBUG) << "float 2.14: " << 1000 / 2.14f;
   LOG(G3LOG_DEBUG) << "pi double: " << pi_d;
   LOG(G3LOG_DEBUG) << "pi float: " << pi_f;
   LOG(G3LOG_DEBUG) << "pi float (width 10): " << std::setprecision(10) << pi_f;
   LOGF(INFO, "pi float printf:%f", pi_f);

   logworker->hotUpdateSink(coutSinkHandle.get(), &ColorCoutSink::blackWhiteScheme);

   LOGF(INFO, "Hi log %d", 123);
   LOG(INFO) << "Test SLOG INFO";
   LOGF(WARNING, "Printf-style syntax is also %s", "available");
   LOG(G3LOG_DEBUG) << "Test SLOG DEBUG";
   LOG(INFO) << "one: " << 1;
   LOG(INFO) << "two: " << 2;
   LOG(INFO) << "one and two: " << 1 << " and " << 2;
   LOG(WARNING) << "Warning information";
   LOG(G3LOG_DEBUG) << "float 2.14: " << 1000 / 2.14f;
   LOG(G3LOG_DEBUG) << "pi double: " << pi_d;
   LOG(G3LOG_DEBUG) << "pi float: " << pi_f;
   LOG(G3LOG_DEBUG) << "pi float (width 10): " << std::setprecision(10) << pi_f;
   LOGF(INFO, "pi float printf:%f", pi_f);

   logworker->hotUpdateSink(coutSinkHandle.get(), &ColorCoutSink::systemDefaultScheme);

   LOGF(INFO, "Hi log %d", 123);
   LOG(INFO) << "Test SLOG INFO";
   LOGF(WARNING, "Printf-style syntax is also %s", "available");
   LOG(G3LOG_DEBUG) << "Test SLOG DEBUG";
   LOG(INFO) << "one: " << 1;
   LOG(INFO) << "two: " << 2;
   LOG(INFO) << "one and two: " << 1 << " and " << 2;
   LOG(WARNING) << "Warning information";
   LOG(G3LOG_DEBUG) << "float 2.14: " << 1000 / 2.14f;
   LOG(G3LOG_DEBUG) << "pi double: " << pi_d;
   LOG(G3LOG_DEBUG) << "pi float: " << pi_f;
   LOG(G3LOG_DEBUG) << "pi float (width 10): " << std::setprecision(10) << pi_f;
   LOGF(INFO, "pi float printf:%f", pi_f);

   // OK --- on Ubunti this caused get a compiler warning with gcc4.6
   // from gcc 4.7.2 (at least) it causes a crash (as expected)
   // On windows itll probably crash
   example_fatal::tryToKillWithIllegalPrintout();

   // try 2
   std::unique_ptr<std::string> badStringPtr;
   example_fatal::tryToKillWithAccessingIllegalPointer(std::move(badStringPtr));

   // what happened? OK. let us just exit with SIGFPE
   
   int value = 1;    // system dependent but it SHOULD never reach this line
   example_fatal::killByZeroDivision(value);
   return 0;
}
