/** ==========================================================================
* 2013 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/

#pragma once

#include "g3log/sink.hpp"

#include <memory>
#include <type_traits>

namespace g3 {

   // The Sinkhandle is the client's access point to the specific sink instance.
   // Only through the Sinkhandle can, and should, the real sink's specific API
   // be called.
   //
   // The real sink will be owned by g3log. If the real sink is deleted
   // calls to sink's API through the SinkHandle will return an exception embedded
   // in the resulting future. Ref: SinkHandle::call
   template<class T> // (T can be instantiated by g3::FileSink)
   class SinkHandle {
      // Results in no effect to the managed object when destroys the weak_ptr object.
      std::weak_ptr<internal::Sink<T>> _sink;

   public:
      // std::weak_ptr<T>::weak_ptr
      // template< class Y >
      // weak_ptr(const std::shared_ptr<Y>& r) noexcept;
      //
      // Constructs new weak_ptr which shares an object managed by r. If r 
      // manages no object, *this manages no object too. The templated 
      // overloads don't participate in the overload resolution unless 
      // Y* is implicitly convertible to T* (where T is shared_ptr's template parameter)
      SinkHandle(std::shared_ptr<internal::Sink<T>> sink)
         : _sink(sink) {}

      ~SinkHandle() {}


      // Asynchronous call to the real sink. If the real sink is already deleted
      // the returned future will contain a bad_weak_ptr exception instead of the
      // call result.
      // 
      // std::shared_ptr<T>::shared_ptr
      // template< class Y >
      // explicit shared_ptr(const std::weak_ptr<Y>& r);
      // 
      // Constructs a shared_ptr which shares ownership of the object managed by r. 
      // Y* must be implicitly convertible to T*. (until C++17) This overload 
      // participates in overload resolution only if Y* is compatible with T*. 
      // (since C++17) Note that r.lock() may be used for the same purpose: 
      // the difference is that this constructor throws an bad_weak_ptr 
      // exception if the argument is empty, while std::weak_ptr<T>::lock() 
      // constructs an empty std::shared_ptr in that case.
      template<typename AsyncCall, typename... Args>
      auto call(AsyncCall func , Args&& ... args) -> std::future<std::invoke_result_t<decltype(func), T*, Args...>> {
         try {
            std::shared_ptr<internal::Sink<T>> sink(_sink);
            return sink->async(func, std::forward<Args>(args)...);
         } catch (const std::bad_weak_ptr& e) {
            typedef std::invoke_result_t<decltype(func), T*, Args...> PromiseType;
            std::promise<PromiseType> promise;
            promise.set_exception(std::make_exception_ptr(e));
            return std::move(promise.get_future());
         }
      } // func must be a member function pointer of class T

      /// Get weak_ptr access to the sink(). Make sure to check that the returned pointer is valid,
      /// auto p = sink(); auto ptr = p.lock(); if (ptr) { .... }
      /// ref: https://en.cppreference.com/w/cpp/memory/weak_ptr/lock
      // std::weak_ptr<T>::lock
      // Creates a new std::shared_ptr that shares ownership of the managed object. 
      // If there is no managed object, i.e. *this is empty, then the returned 
      // shared_ptr also is empty.
      // Effectively returns expired() ? shared_ptr<T>() : shared_ptr<T>(*this), 
      // executed atomically.
      std::weak_ptr<internal::Sink<T>> sink() {
         // shared_ptr objects can be assigned to weak_ptr objects directly, 
         // but in order to assign a weak_ptr object to a shared_ptr it shall
         // be done using member lock or constructor.
         return _sink.lock();
      }
   };

} // g3


