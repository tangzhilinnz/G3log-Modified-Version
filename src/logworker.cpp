/** ==========================================================================
* 2011 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
*
* For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/

#include "g3log/logworker.hpp"
#include "g3log/logmessage.hpp"
#include "g3log/active.hpp"
#include "g3log/g3log.hpp"
#include "g3log/future.hpp"
#include "g3log/crashhandler.hpp"

#include <iostream>

namespace g3 {

   LogWorkerImpl::LogWorkerImpl() : _bg(kjellkod::Active::createActive()) { }

   // typedef MoveOnCopy<std::unique_ptr<LogMessage>> LogMessagePtr;
   void LogWorkerImpl::bgSave(g3::LogMessagePtr msgPtr) {
      std::unique_ptr<LogMessage> uniqueMsg(std::move(msgPtr.get()));

      for (auto& sink : _sinks) {
         // LogMessage::LogMessage(const LogMessage& other)
         // *(uniqueMsg) returns a reference to the managed LogMessage object
         // copy construct a new LogMessage object on the stack
         LogMessage msg(*(uniqueMsg));
         // typedef MoveOnCopy<LogMessage> LogMessageMover;
         // template<typename Moveable>
         // explicit MoveOnCopy<Moveable>::MoveOnCopy(Moveable &&m);
         sink->send(LogMessageMover(std::move(msg)));
      }

      if (_sinks.empty()) {
         std::string err_msg {"g3logworker has no sinks. Message: ["};
         err_msg.append(uniqueMsg.get()->toString()).append("]\n");
         std::cerr << err_msg;
      }
   } // uniqueMsg that goes out of this scope will dispose of the LogMessage 
     // object dynamically-allocated through std::make_unique<LogMessage>(...) 
     // in g3log.cpp::saveMessage and the LogMessage object will be destroyed 
     // immediately.
     // uniqueMsg owns and manages this dynamic LogMessage object by a series
     // of ownership transfers from:
     //    <- std::unique_ptr<LogMessage> uniqueMsg(std::move(msgPtr.get()))
     //    <- LogWorkerImpl::bgSave(g3::LogMessagePtr msgPtr)
     //    <- LogWorker::save(LogMessagePtr msg)
     //    <- pushMessageToLogger(LogMessagePtr incoming)
     //    <- LogMessagePtr message {std::make_unique<LogMessage>(file, line, function, msgLevel)} 
     //    <- saveMessage(const char* entry, const char* file, int line, const char* function, 
     //                   const LEVELS& level, const char* boolean_expression, int fatal_signal, 
     //                   const char* stack_trace)
     // The same is true with uniqueMsg in LogWorkerImpl::bgFatal

   // typedef MoveOnCopy<std::unique_ptr<FatalMessage>> FatalMessagePtr;
   // struct FatalMessage : public LogMessage { ... }
   void LogWorkerImpl::bgFatal(FatalMessagePtr msgPtr) {
      // g3::internal::shutDownLogging();

      std::string reason = msgPtr.get()->reason();
      const auto level = msgPtr.get()->_level;
      const auto fatal_id = msgPtr.get()->_signal_id;


      std::unique_ptr<LogMessage> uniqueMsg(std::move(msgPtr.get()));
      uniqueMsg->write().append("\nExiting after fatal event  (").append(uniqueMsg->level());


      // Change output in case of a fatal signal (or windows exception)
      std::string exiting = {"Fatal type: "};

      uniqueMsg->write().append("). ").append(exiting).append(" ").append(reason)
      .append("\nLog content flushed successfully to sink\n\n");

      std::cerr << uniqueMsg->toString() << std::flush;
      for (auto& sink : _sinks) {
         LogMessage msg(*(uniqueMsg));
         sink->send(LogMessageMover(std::move(msg)));
      }

      // This clear is absolutely necessary
      // All sinks are forced to receive the fatal message above before we continue
      _sinks.clear(); // flush all queues and destroy all sinks dynamcially allocated
      // this will be the last message. Only the active logworker can receive a FATAL call so it's
      // safe to shutdown logging now. 
      // Abandon shutDownLogging() here to avoid uncertainties in multi-threaded processes
      /*g3::internal::shutDownLogging();*/
      internal::exitWithDefaultSignalHandler(level, fatal_id);

      // should never reach this point
      perror("g3log exited after receiving FATAL trigger. Flush message status: ");
   }

   LogWorker::~LogWorker() {
      g3::internal::shutDownLoggingForActiveOnly(this);

      // The sinks WILL automatically be cleared at exit of this destructor
      // The waiting inside removeAllSinks ensures that all messages until this point are
      // taken care of before any internals/LogWorkerImpl of LogWorker starts to be destroyed.
      // i.e. this avoids a race with another thread slipping through the "shutdownLogging" and
      // calling ::save or ::fatal through LOG/CHECK with lambda messages and "partly
      // deconstructed LogWorkerImpl"
      //
      //   Any messages put into the queue will be OK due to:
      //  *) If it is before the wait below then they will be executed
      //  *) If it is AFTER the wait below then they will be ignored and NEVER executed
      removeAllSinks();

      // The background worker WILL be automatically cleared at the exit of the destructor
      // However, the explicitly clearing of the background worker (below) makes sure that there can
      // be no thread that manages to add another sink after the call to clear the sinks above.
      //   i.e. this manages the extremely unlikely case of another thread calling
      // addWrappedSink after the sink clear above. Normally adding of sinks should be done in main.cpp
      // and be closely coupled with the existence of the LogWorker. Sharing this adding of sinks to
      // other threads that do not know the state of LogWorker is considered a bug but it is dealt with
      // nonetheless below.
      //
      // If sinks would already have been added after the sink clear above then this reset will deal with it
      // without risking lambda execution with a partially deconstructed LogWorkerImpl
      // Calling g3::spawn_task on a nullptr Active object will not crash but return
      // a future containing an appropriate exception.
      _impl._bg.reset(nullptr);
   }

   void LogWorker::save(LogMessagePtr msg) {
      _impl._bg->send([this, msg] {_impl.bgSave(msg); });
   }

   void LogWorker::fatal(FatalMessagePtr fatal_message) {
      _impl._bg->send([this, fatal_message] {_impl.bgFatal(fatal_message); });
   }

   void LogWorker::addWrappedSink(std::shared_ptr<g3::internal::SinkWrapper> sink) {
      auto bg_addsink_call = [this, sink] {_impl._sinks.push_back(sink);};
      auto token_done = g3::spawn_task(bg_addsink_call, _impl._bg.get());
      token_done.wait();
   }

   std::unique_ptr<LogWorker> LogWorker::createLogWorker() {
      // std::unique_ptr<LogWorker> move constructor is called automatically.
      return std::unique_ptr<LogWorker>(new LogWorker);
   }

   // std::unique_ptr<FileSinkHandle> 
   std::unique_ptr<g3::SinkHandle<g3::FileSink>> LogWorker::addDefaultLogger(
      const std::string& log_prefix, const std::string& log_directory, const std::string& default_id) {
      return addSink(std::make_unique<g3::FileSink>(log_prefix, log_directory, default_id), &FileSink::fileWrite);
   }

} // g3
