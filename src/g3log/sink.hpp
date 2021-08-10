/** ==========================================================================
* 2013 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/

#pragma once


#include "g3log/sinkwrapper.hpp"
#include "g3log/active.hpp"
#include "g3log/future.hpp"
#include "g3log/logmessage.hpp"

#include <memory>
#include <functional>
#include <type_traits>

namespace g3 {
namespace internal {


   typedef std::function<void(LogMessageMover) > AsyncMessageCall;

   /// The asynchronous Sink has an active object, incoming requests for actions
   //  will be processed in the background by the specific object the Sink represents.
   //
   // The Sink will wrap either
   //     a Sink with Message object receiving call
   // or  a Sink with a LogEntry (string) receiving call
   //
   // The Sink can also be used through the SinkHandler to call Sink specific function calls
   // Ref: send(Message) deals with incoming log entries (converted if necessary to string)
   // Ref: send(Call call, Args... args) deals with calls
   //           to the real sink's API

   template<class T>
   struct Sink : public SinkWrapper {
      std::unique_ptr<T> _real_sink;
      std::unique_ptr<kjellkod::Active> _bg;
      AsyncMessageCall _default_log_call;

      template<typename DefaultLogCall >
      Sink(std::unique_ptr<T> sink, DefaultLogCall call)
         : SinkWrapper()
         , _real_sink {std::move(sink)}
         , _bg(kjellkod::Active::createActive())
         , _default_log_call(std::bind(call, _real_sink.get(), std::placeholders::_1)) 
      { } // a Sink with LogMessageMover object receiving call (class T member function)


      Sink(std::unique_ptr<T> sink, void(T::*Call)(std::string) )
         : SinkWrapper()
         , _real_sink {std::move(sink)}
         , _bg(kjellkod::Active::createActive()) 
      {
         std::function<void(std::string)> adapter = 
            std::bind(Call, _real_sink.get(), std::placeholders::_1);

         _default_log_call = [ = ](LogMessageMover m) {
            adapter(m.get().toString());
         };
      } // a Sink with a LogEntry (string) receiving call (class T member function)

      virtual ~Sink() {
         _bg.reset(); // TODO: to remove
      }

      void send(LogMessageMover msg) override {
         _bg->send([this, msg] {
            _default_log_call(msg);
         });
      }

      // using invoke_result_t = typename invoke_result<F, ArgTypes...>::type;
      // F must be a callable type, reference to function, or reference to callable type
      // type: the return type of the Callable type F if invoked with the arguments ArgTypes.... 
      // Only defined if F can be called with the arguments ArgTypes...
      template<typename Call, typename... Args>
      auto async(Call call, Args &&... args) -> std::future<std::invoke_result_t<decltype(call), T*, Args...>> {
         return g3::spawn_task(std::bind(call, _real_sink.get(), std::forward<Args>(args)...), _bg.get());
      } // The type of Call must be a member function type of class T
   };


} // internal
} // g3

// In lambda expression, you can capture by both reference and value, which you
// can specify using & and = respectively: 
//[&epsilon] captures by reference
//[&] captures all variables used in the lambda by reference
//[=] captures all variables used in the lambda by value
//[&, epsilon] captures variables like with[&], but epsilon by value
//[=, &epsilon] captures variables like with[=], but epsilon by reference

// In C++14 lambdas have been extended by various proposals.
// Initialized Lambda Captures
// An element of the capture list can now be initialized with =. This allows
// renaming of variables and to capture by moving. 
// 
// An example taken from the standard:
/*
 * int x = 4;
 * auto y = [&r = x, x = x+1]()->int {
 *             r += 2;
 *             return x+2;
 *          }();  // Updates ::x to 6, and initializes y to 7.
 * 
 */

// and one taken from Wikipedia showing how to capture with std::move:
/*
 * auto ptr = std::make_unique<int>(10);
 * auto lambda = [ptr = std::move(ptr)] { return *ptr; };
 *
 */
