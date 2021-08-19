/** ==========================================================================
 * Original code made by Robert Engeln. Given as a PUBLIC DOMAIN dedication for
 * the benefit of g3log.  It was originally published at:
 * http://code-freeze.blogspot.com/2012/01/generating-stack-traces-from-c.html

 * 2014-2015: adapted for g3log by Kjell Hedstrom (KjellKod).
 *
 * This is PUBLIC DOMAIN to use at your own risk and comes
 * with no warranties. This code is yours to share, use and modify with no
 * strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
 * ============================================================================*/

#include "g3log/stacktrace_windows.hpp"

#include <windows.h>
#include <dbghelp.h>
#include <map>
#include <memory>
#include <cassert>
#include <vector>
#include <mutex>
#include <g3log/g3log.hpp>

// Use an a.lib file in the project
// #pragma comment(lib, "a.lib")
#pragma comment(lib, "dbghelp.lib")


#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#error "stacktrace_win.cpp used but not on a windows system"
#endif


#define g3_MAP_PAIR_STRINGIFY(x) {x, #x}

namespace {
   // When you declare a variable prefixed with thread_local then each thread
   // has its own copy. When you refer to it by name, then the copy associated
   // with the current thread is used
   thread_local size_t g_thread_local_recursive_crash_check = 0;

   const std::map<g3::SignalType, std::string> kExceptionsAsText = {
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_ACCESS_VIOLATION),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_ARRAY_BOUNDS_EXCEEDED),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_DATATYPE_MISALIGNMENT),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_FLT_DENORMAL_OPERAND),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_FLT_DIVIDE_BY_ZERO),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_FLT_INEXACT_RESULT),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_FLT_INEXACT_RESULT),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_FLT_INVALID_OPERATION),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_FLT_OVERFLOW),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_FLT_STACK_CHECK),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_FLT_UNDERFLOW),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_ILLEGAL_INSTRUCTION),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_IN_PAGE_ERROR),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_INT_DIVIDE_BY_ZERO),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_INT_OVERFLOW),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_INVALID_DISPOSITION),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_NONCONTINUABLE_EXCEPTION),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_PRIV_INSTRUCTION),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_STACK_OVERFLOW),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_BREAKPOINT),
      g3_MAP_PAIR_STRINGIFY(EXCEPTION_SINGLE_STEP)

   };


   // Using the given context, fill in all the stack frames.
   // Which then later can be interpreted to human readable text
   void captureStackTrace(CONTEXT* context, std::vector<uint64_t>& frame_pointers) {
      DWORD machine_type = 0;  // typedef unsigned long  DWORD;
      STACKFRAME64 frame = {}; // force zeroing
      frame.AddrPC.Mode = AddrModeFlat;
      frame.AddrFrame.Mode = AddrModeFlat;
      frame.AddrStack.Mode = AddrModeFlat;
#if defined(_M_ARM64)
      frame.AddrPC.Offset = context->Pc;
      frame.AddrFrame.Offset = context->Fp;
      frame.AddrStack.Offset = context->Sp;
      machine_type = IMAGE_FILE_MACHINE_ARM64;
#elif defined(_M_ARM)
      frame.AddrPC.Offset = context->Pc;
      frame.AddrFrame.Offset = context->R11;
      frame.AddrStack.Offset = context->Sp;
      machine_type = IMAGE_FILE_MACHINE_ARM;
#elif defined(_M_X64)
      frame.AddrPC.Offset = context->Rip;
      frame.AddrFrame.Offset = context->Rbp;
      frame.AddrStack.Offset = context->Rsp;
      machine_type = IMAGE_FILE_MACHINE_AMD64;
#else
      frame.AddrPC.Offset = context->Eip;
      frame.AddrFrame.Offset = context->Ebp;
      frame.AddrStack.Offset = context->Esp;
      machine_type = IMAGE_FILE_MACHINE_I386;
#endif
      for (size_t index = 0; index < frame_pointers.size(); ++index) {
         // StackWalk64 - Provides a convenient and platform-independent way to 
         // walk the stack of a thread
         // You cannot really distinguish between StackWalk64 returning FALSE 
         // because of a failure or because of the bottom of the stack having 
         // been reached as StackWalk64 does not reliably set the last Win32 
         // error.
         // Note: You cannot expect to reliably walk the stack on optimized code 
         // (Released Build). Eliminating stack frames is on the top of the hit
         // list for the code optimizer. The "Omit frame pointer" optimization 
         // is an important one, that frees up an extra register (EBP), always 
         // important for x86 code. It is usually off by default but the code 
         // generator applies it anyway when it can inline functions.
         if (StackWalk64(machine_type,
                         GetCurrentProcess(),
                         GetCurrentThread(),
                         &frame,
                         context,
                         NULL,
                         SymFunctionTableAccess64,
                         SymGetModuleBase64,
                         NULL)) { // Most callers of StackWalk64 can safely pass NULL for this parameter
            frame_pointers[index] = frame.AddrPC.Offset;
         } else {
            break; // Maybe it failed, maybe we have finished walking the stack 
         }
      }
   }


   // extract readable text from a given stack frame. All thanks to
   // using SymFromAddr and SymGetLineFromAddr64 with the stack pointer
   std::string getSymbolInformation(const size_t index, const std::vector<uint64_t>& frame_pointers) {
      auto addr = frame_pointers[index];
      std::string frame_dump = "stack dump [" + std::to_string(index) + "]\t";

      DWORD64 displacement64;
      DWORD displacement;
      char symbol_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
      SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbol_buffer);
      symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
      symbol->MaxNameLen = MAX_SYM_NAME;

      IMAGEHLP_LINE64 line;
      line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
      std::string lineInformation;
      std::string callInformation;
      if (SymFromAddr(GetCurrentProcess(), addr, &displacement64, symbol)) {
         // string (const char* s, size_t n);
         callInformation.append(" ").append(std::string(symbol->Name, symbol->NameLen));
         if (SymGetLineFromAddr64(GetCurrentProcess(), addr, &displacement, &line)) {
            lineInformation.append("\t").append(line.FileName).append(" L: ");
            lineInformation.append(std::to_string(line.LineNumber));
         }
      }
      frame_dump.append(lineInformation).append(callInformation);
      return frame_dump;
   }


   // Retrieves all the symbols for the stack frames, fills them within a text
   // representation and returns it
   std::string convertFramesToText(std::vector<uint64_t>& frame_pointers) {
      std::string dump; // slightly more efficient than ostringstream
      const size_t kSize = frame_pointers.size();
      for (size_t index = 0; index < kSize && frame_pointers[index]; ++index) {
         dump += getSymbolInformation(index, frame_pointers);
         dump += "\n";
      }
      return dump;
   }
} // anonymous (unnamed namespace)
// All content declared in an unnamed namespace is treated as if it is part of 
// the parent namespace. The other effect of unnamed namespaces is that all 
// identifiers inside an unnamed namespace are treated as if they had internal
// linkage, which means that the content of an unnamed namespace can¡¯t be accessed
// outside of the file in which the unnamed namespace is defined.
//  
// keyword static only applies to names of objects, functions, and anonymous unions, 
// not to type declarations




namespace stacktrace {
   const std::string kUnknown = { "UNKNOWN EXCEPTION" };
   /// return the text description of a Windows exception code
   /// From MSDN GetExceptionCode 
   /// http://msdn.microsoft.com/en-us/library/windows/desktop/ms679356(v=vs.85).aspx
   std::string exceptionIdToText(g3::SignalType id) {
      const auto iter = kExceptionsAsText.find(id);
      if ( iter == kExceptionsAsText.end()) {
         std::string unknown = { kUnknown + ":" + std::to_string(id) };
         return unknown;
      }
      return iter->second;
   }

   /// Yes a double lookup: first for isKnownException and then exceptionIdToText
   /// for vectored exceptions we only deal with known exceptions so this tiny
   /// overhead we can live with
   bool isKnownException(g3::SignalType id) {
      return (kExceptionsAsText.end() != kExceptionsAsText.find(id));
   }

   /// helper function: retrieve stackdump from no excisting exception pointer
   std::string stackdump() {
      CONTEXT current_context;
      memset(&current_context, 0, sizeof(CONTEXT));
      // RtlCaptureContext - 
      // Retrieves a context record in the context of the caller stackdump
      RtlCaptureContext(&current_context);
      return stackdump(&current_context);
   }

   /// helper function: retrieve stackdump, starting from an exception pointer
   // EXCEPTION_POINTERS structure (winnt.h)
   //
   // Contains an exception record with a machine-independent description of an
   // exception and a context record with a machine-dependent description of the
   // processor context at the time of the exception.
   // 
   // Members - ContextRecord:
   // A pointer to a CONTEXT structure that contains a processor-specific 
   // description of the state of the processor at the time of the exception.
   std::string stackdump(EXCEPTION_POINTERS* info) {
      auto context = info->ContextRecord;
      return stackdump(context);
   }


   /// main stackdump function. retrieve stackdump, from the given context
   std::string stackdump(CONTEXT* context) {

      if (g_thread_local_recursive_crash_check >= 2) { // In Debug scenarios we allow one extra pass
         std::string recursive_crash = {"\n\n\n***** Recursive crash detected"};
         recursive_crash.append(", cannot continue stackdump traversal. *****\n\n\n");
         return recursive_crash;
      }
      ++g_thread_local_recursive_crash_check;

      static std::mutex m;
      std::lock_guard<std::mutex> lock(m);
      {
         const BOOL kLoadSymModules = TRUE;
         // SymInitialize - Initializes the symbol handler for a process
         // If kLoadSymModules is TRUE, enumerates the loaded modules for the 
         // process and effectively calls the SymLoadModule64 function for 
         // each module.
         const auto initialized = SymInitialize(GetCurrentProcess(), nullptr, kLoadSymModules);
         if (TRUE != initialized) {
            return { "Error: Cannot call SymInitialize(...) for retrieving symbols in stack" };
         }

         // An std::shared_ptr<void> can be used as a handle to an unspecified
         // data type in a non-intrusive
         // std::shared_ptr<void> can destroy a managed object of a specific type
         // despite holding a void*. It works because the object deletion procedure
         // in std::shared_ptr is type-erased, or it is made independent of the 
         // stored pointer type that is returned by the get().
         // An std::shared_ptr<T> can be constructed/initialized with a raw pointer
         // of type Y, as long as Y* is convertible to T*. In that case, the get() 
         // returns T*, and the deleter calls delete on Y*. Generally, it is said 
         // that the deleter of an std::shared_ptr is type-erased. To achieve this 
         // type-erasure, the std::shared_ptr stores the deleter as part of its
         // control block.
         std::shared_ptr<void> RaiiSymCleaner(nullptr, [&](void*) {
            // SymCleanup - 
            // Deallocates all resources associated with the process handle
            SymCleanup(GetCurrentProcess());
         }); // Raii sym cleanup


         const size_t kmax_frame_dump_size = 64;
         std::vector<uint64_t>  frame_pointers(kmax_frame_dump_size);
         // C++11: size set and values are zeroed

         assert(frame_pointers.size() == kmax_frame_dump_size);
         captureStackTrace(context, frame_pointers);
         return convertFramesToText(frame_pointers);
      }
   }

} // stacktrace