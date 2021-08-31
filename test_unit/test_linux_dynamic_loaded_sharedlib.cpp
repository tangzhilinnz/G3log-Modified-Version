/** ==========================================================================
* 2014 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
 * 
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/


#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <g3log/filesink.hpp>
#include <memory>

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "tester_sharedlib.h"
#include <dlfcn.h>

struct LogMessageCounter {
   std::vector<std::string>& bank;
   LogMessageCounter(std::vector<std::string>& storeMessages) : bank(storeMessages) {
   }

   void countMessages(std::string msg) {
      bank.push_back(msg);
   }
};

TEST(DynamicLoadOfLibrary, JustLoadAndExit) {
   std::vector<std::string> receiver;
   
   { // scope to flush logs at logworker exit
      auto worker = g3::LogWorker::createLogWorker();
      auto handle = worker->addSink(std::make_unique<LogMessageCounter>(std::ref(receiver)), &LogMessageCounter::countMessages);
      
      // add another sink just for more throughput of data
      auto fileHandle = worker->addSink(std::make_unique<g3::FileSink>("runtimeLoadOfDynamiclibs", "/tmp"), &g3::FileSink::fileWrite);
      g3::initializeLogging(worker.get());

      // dlopen() loads the dynamic shared object (shared library) file named 
      // by the null - terminated string filename and returns an opaque "handle"
      // for the loaded object. This handle is employed with other functions 
      // in the dlopen API, such as dlsym(), dladdr(), dlinfo(), and dlclose().
      // RTLD_LAZY
      // Perform lazy binding. Resolve symbols only as the code that references
      // them is executed. If the symbol is never referenced, then it is never
      // resolved. (Lazy binding is performed only for function references; 
      // references to variables are always immediately bound when the shared
      // object is loaded.)
      // RTLD_GLOBAL
      // The symbols defined by this shared object will be made available for 
      // symbol resolution of subsequently loaded shared objects.
      void* libHandle = dlopen("libtester_sharedlib.so", RTLD_LAZY | RTLD_GLOBAL);
      EXPECT_FALSE(nullptr == libHandle);
      // The function dlsym() takes a "handle" of a dynamic loaded shared object 
      // returned by dlopen() along with a null - terminated symbol name, and 
      // returns the address where that symbol is loaded into memory. If the 
      // symbol is not found, in the specified object or any of the shared objects
      // that were automatically loaded by dlopen() when that object was loaded, 
      // dlsym() returns NULL. (The search performed by dlsym() is breadth first
      // through the dependency tree of these shared objects.)
      LibraryFactory* factory = reinterpret_cast<LibraryFactory*> ((dlsym(libHandle, "testRealFactory")));
      EXPECT_FALSE(nullptr == factory);
      SomeLibrary* loadedLibrary = factory->CreateLibrary(); // c++ polymorphism

      for (auto i = 0; i < 300; ++i) {
         loadedLibrary->action(); // c++ polymorphism
      }

      delete loadedLibrary;
      // dlclose() decrements the reference count on the dynamic library handle. 
      // If the reference count drops to zero and no other loaded libraries use
      // symbols in it, then the dynamic library is unloaded.
      dlclose(libHandle);
   } // scope exit. All log entries must be flushed now
   const size_t numberOfMessages = 2 + 300 + 1; // 2 library construction, 300 loop, 1 destoyed library  
   EXPECT_EQ(receiver.size(), numberOfMessages);
}
