/** ==========================================================================
* 2015 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
*
* For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================*/

#pragma once

#include <atomic>

namespace g3 {
   /// As suggested in: http://stackoverflow.com/questions/13193484/how-to-declare-a-vector-of-atomic-in-c
   struct atomicbool {
   private:
      // std::atomic<T> isn't copy-constructible, nor copy-assignable.
      // Object types that don't have these properties cannot be used as
      // elements of std::vector
      std::atomic<bool> value_;
   public:
      // std::atomic<T>::load
      // Atomically loads and returns the current value of the atomic variable. 
      // Memory is affected according to the value of order.
      // order must be one of std::memory_order_relaxed, std::memory_order_consume, 
      // std::memory_order_acquire or std::memory_order_seq_cst. 
      // Otherwise the behavior is undefined.
      //
      // std::atomic<T>::store
      // Atomically replaces the current value with desired. Memory is affected 
      // according to the value of order.
      // order must be one of std::memory_order_relaxed, std::memory_order_release 
      // or std::memory_order_seq_cst. Otherwise the behavior is undefined.
      atomicbool(): value_ {false} {}
      atomicbool(const bool& value): value_ {value} {}
      atomicbool(const std::atomic<bool>& value) : value_ {value.load(std::memory_order_acquire)} {}
      atomicbool(const atomicbool& other): value_ {other.value_.load(std::memory_order_acquire)} {}

      atomicbool& operator=(const atomicbool& other) {
         value_.store(other.value_.load(std::memory_order_acquire), std::memory_order_release);
         return *this;
      }

      atomicbool& operator=(const bool other) {
         value_.store(other, std::memory_order_release);
         return *this;
      }

   bool operator==(const atomicbool& rhs)  const {
      return (value_.load(std::memory_order_acquire) == rhs.value_.load(std::memory_order_acquire));
   }

   bool value() {
      return value_.load(std::memory_order_acquire);
   }

   std::atomic<bool>& get() { return value_; }
   };
} // g3
// explicit whitespace/EOF for VS15 