/** ==========================================================================
* 2015 by KjellKod.cc
*
* This code is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
* ============================================================================*
* PUBLIC DOMAIN and Not copywrited. First published at KjellKod.cc
* ********************************************* */

#include "g3log/filerotatewithfiltersink.hpp"
#include "g3log/filerotatesink.hpp"
#include "test_rotate_helper.h"
#include "test_filter_sink.h"
#include <iostream>
#include <cerrno>
#include <memory>


#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__)) && !defined(__MINGW32__)
#define F_OK 0
#else
#include <unistd.h>
#endif

using namespace g3;
using namespace RotateTestHelper;

namespace { // anonymous
   LogMessageMover CreateLogEntry(const LEVELS level, std::string content, std::string file, int line, std::string function) {
      auto message = LogMessage(file, line, function, level);
      message.write().append(content);
      return MoveOnCopy<LogMessage>(std::move(message));
   }

#define CREATE_LOG_ENTRY(level, content) CreateLogEntry(level, content, __FILE__, __LINE__, __FUNCTION__)

} // anonymous

TEST_F(FilterTest, CreateObject) {
   std::string logfilename;
   {
      auto logRotatePtr = std::make_unique<FileRotateSink>(_filename, _directory);

      FileRotateWithFilter logWithFilter(std::move(logRotatePtr), {});
   } // RAII flush of log
   auto name = std::string{_directory + _filename + ".log"};
   int check = access(name.c_str(), F_OK); // check that the file exists
   EXPECT_EQ(check, 0) << std::strerror(errno) << " : " << name;
}

TEST_F(FilterTest, CreateObjectUsingHelper) {
   auto name = std::string{_directory +"/" + _filename + ".log"};
   int check = access(name.c_str(), F_OK); // check that the file exists
   EXPECT_NE(check, 0) << std::strerror(errno) << " : " << name;
   {
      auto filterSinkPtr = FileRotateWithFilter::CreateLogRotateWithFilter(_filename, _directory, {});
   } // raii
   check = access(name.c_str(), F_OK); // check that the file exists
   EXPECT_EQ(check, 0) << std::strerror(errno) << " : " << name;
   auto content = ReadContent(name);
   EXPECT_TRUE(Exists(content, "g3log file created log at:")) << ", content: [" << "]" << ", name: " << name;
   EXPECT_TRUE(Exists(content, "g3log file shutdown at:")) << ", content: [" << "]"<< ", name: " << name;
}

 /**
  * The default for log details can be found at g3log. 
  * It's something like this 
  *    std::string LogMessage::DefaultLogDetailsToString(const LogMessage& msg) {
      std::string out;
      out.append(msg.timestamp() + "\t"
                 + msg.level() 
                 + " [" 
                 + msg.file() 
                 + "->" 
                 + msg.function() 
                 + ":" + msg.line() + "]\t");
      return out;
      }


      We will override this to only have level
   }
  * */

TEST_F(FilterTest, OverrideLogDetails) {

   auto formatting = [](const g3::LogMessage& msg) -> std::string {
      std::string out;
      out.append(std::string(" === ") + msg.level() + (" !!! "));
      return out;
   };

   {
      auto filterSinkPtr = FileRotateWithFilter::CreateLogRotateWithFilter(_filename, _directory, {});
      filterSinkPtr->overrideLogDetails(formatting);

      auto message0 = CREATE_LOG_ENTRY(INFO, "Hello World");
      filterSinkPtr->save(message0);
   } // raii

   auto name = std::string{_directory + _filename + ".log"};
   auto content = ReadContent(name);
   EXPECT_TRUE(Exists(content, "=== INFO !!! Hello World")) << content;
}

TEST_F(FilterTest, NothingFiltered) {
   {
      auto filterSinkPtr = FileRotateWithFilter::CreateLogRotateWithFilter(_filename, _directory, {});
      auto message0 = CREATE_LOG_ENTRY(INFO, "Hello World");
      filterSinkPtr->save(message0);

      auto message1 = CREATE_LOG_ENTRY(G3LOG_DEBUG, "Hello D World");
      filterSinkPtr->save(message1);

      auto message2 = CREATE_LOG_ENTRY(WARNING, "Hello W World");
      filterSinkPtr->save(message2);

      auto message3 = CREATE_LOG_ENTRY(FATAL, "Hello F World");
      filterSinkPtr->save(message3);
   } // raii

   auto name = std::string{_directory + _filename + ".log"};
   auto content = ReadContent(name);
   EXPECT_TRUE(Exists(content, "Hello World")) << content;
   EXPECT_TRUE(Exists(content, "Hello D World")) << content;
   EXPECT_TRUE(Exists(content, "Hello W World")) << content;
   EXPECT_TRUE(Exists(content, "Hello F World")) << content; // exit the whole process ?
}

TEST_F(FilterTest, FilteredAndNotFiltered) {
   {
      auto filterSinkPtr = FileRotateWithFilter::CreateLogRotateWithFilter(_filename, _directory, {G3LOG_DEBUG, INFO, WARNING, FATAL});
      auto message0 = CREATE_LOG_ENTRY(INFO, "Hello World");
      filterSinkPtr->save(message0);
    
      auto message1 = CREATE_LOG_ENTRY(G3LOG_DEBUG, "Hello D World");
      filterSinkPtr->save(message1);

      auto message2 = CREATE_LOG_ENTRY(WARNING, "Hello W World");
      filterSinkPtr->save(message2);

      auto message3 = CREATE_LOG_ENTRY(FATAL, "Hello F World");
      filterSinkPtr->save(message3);

      auto newLevel = LEVELS{123, "MadeUpLevel"};
      auto message4 = CREATE_LOG_ENTRY(newLevel, "Hello New World");
      filterSinkPtr->save(message4);
   } // raii

   auto name = std::string{_directory + _filename + ".log"};
   auto content = ReadContent(name);
   EXPECT_FALSE(Exists(content, "Hello World")) << content;
   EXPECT_FALSE(Exists(content, "Hello D World")) << content;
   EXPECT_FALSE(Exists(content, "Hello W World")) << content;
   EXPECT_FALSE(Exists(content, "Hello F World")) << content;

   // Not filtered will exist
   EXPECT_TRUE(Exists(content, "Hello New World")) << content;

   const auto level1 = INFO;
   const auto level2 = INFO;
   EXPECT_TRUE(level1 == level2);
}

TEST_F(FilterTest, setFlushPolicy__default__every_time) {
   auto filterSinkPtr = FileRotateWithFilter::CreateLogRotateWithFilter(_filename, _directory, {});
   auto logfilename = filterSinkPtr->logFileName();

   for (size_t i = 0; i < 10; ++i) {
      std::string msg{"message: "};
      msg +=  std::to_string(i) + "\n";
      filterSinkPtr->save(CREATE_LOG_ENTRY(INFO, msg));
      auto content = ReadContent(logfilename);
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__)) && !defined(__MINGW32__)
	  msg.replace(msg.find("\n") , 2, "\r\n");
      auto exists = Exists(content, msg);
#else
	  auto exists = Exists(content, msg);
#endif
      ASSERT_TRUE(exists) << "\n\tcontent:" << content << "-\n\tentry: " << msg;
   }
}

TEST_F(FilterTest, setFlushPolicy__only_when_buffer_is_full) {
   auto filterSinkPtr = FileRotateWithFilter::CreateLogRotateWithFilter(_filename, _directory, {});
   auto logfilename = filterSinkPtr->logFileName();
   filterSinkPtr->setFlushPolicy(0);

   // auto buffer size if by default 1024
   for(int i = 0; i < 10; ++i) {
      filterSinkPtr->save(CREATE_LOG_ENTRY(INFO, "this is a messagen\n"));
   }

   auto content = ReadContent(logfilename);
   auto exists = Exists(content, "this is a message");
   ASSERT_FALSE(exists) << "\n\tcontent:" << content << "-\n\tentry: " << "Y" << ", content.size(): " << content.size();
}

TEST_F(FilterTest, setFlushPolicy__every_third_write) {
   auto filterSinkPtr = FileRotateWithFilter::CreateLogRotateWithFilter(_filename, _directory, {});
   auto logfilename = filterSinkPtr->logFileName();
   filterSinkPtr->setFlushPolicy(3);

   std::string content;
   auto checkIfExist = [&](std::string expected) -> bool {
      content = ReadContent(logfilename);
      bool exists = Exists(content, expected);
      return exists;
   };

   // auto buffer size if by default 1024
   filterSinkPtr->save(CREATE_LOG_ENTRY(INFO, "msg1\n"));
   ASSERT_FALSE(checkIfExist("msg1")) << "\n\tcontent:" << content;

   filterSinkPtr->save(CREATE_LOG_ENTRY(INFO,"msg2\n"));
   ASSERT_FALSE(checkIfExist("msg2")) << "\n\tcontent:" << content;
   
   filterSinkPtr->save(CREATE_LOG_ENTRY(INFO,"msg3\n"));
   ASSERT_TRUE(checkIfExist("msg3")) << "\n\tcontent:" << content;   // 3rd write flushes it + previous

   filterSinkPtr->save(CREATE_LOG_ENTRY(INFO, "msg4\n"));
   ASSERT_FALSE(checkIfExist("msg4")) << "\n\tcontent:" << content;
}


TEST_F(FilterTest, setFlushPolicy__force_flush) {
   auto filterSinkPtr = FileRotateWithFilter::CreateLogRotateWithFilter(_filename, _directory, {});
   auto logfilename = filterSinkPtr->logFileName();
   filterSinkPtr->setFlushPolicy(100);

   std::string content;
   auto checkIfExist = [&](std::string expected) -> bool {
      content = ReadContent(logfilename);
      bool exists = Exists(content, expected);
      return exists;
   };

   // auto buffer size if by default 1024
   filterSinkPtr->save(CREATE_LOG_ENTRY(INFO,"msg1\n"));
   ASSERT_FALSE(checkIfExist("msg1")) << "\n\tcontent:" << content;

   filterSinkPtr->save(CREATE_LOG_ENTRY(INFO, "msg2\n"));
   ASSERT_FALSE(checkIfExist("msg2")) << "\n\tcontent:" << content;
   
   filterSinkPtr->save(CREATE_LOG_ENTRY(INFO, "msg3\n"));
   filterSinkPtr->flush();
   ASSERT_TRUE(checkIfExist("msg3")) << "\n\tcontent:" << content;   // 3rd write flushes it + previous

   filterSinkPtr->save(CREATE_LOG_ENTRY(INFO, "msg4\n"));
   filterSinkPtr->flush();
   ASSERT_TRUE(checkIfExist("msg4")) << "\n\tcontent:" << content;   // 3rd write flushes it + previous
}
