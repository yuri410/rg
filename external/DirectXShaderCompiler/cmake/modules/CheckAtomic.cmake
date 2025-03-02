# atomic builtins are required for threading support.

INCLUDE(CheckCXXSourceCompiles)

# Sometimes linking against libatomic is required for atomic ops, if
# the platform doesn't support lock-free atomics.

function(check_working_cxx_atomics varname)
  set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS "-std=c++11")
  CHECK_CXX_SOURCE_COMPILES("
#include <atomic>
std::atomic<int> x;
int main() {
  return x;
}
" ${varname})
  set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endfunction(check_working_cxx_atomics)

# This isn't necessary on MSVC, so avoid command-line switch annoyance
# by only running on GCC-like hosts.
if (LLVM_COMPILER_IS_GCC_COMPATIBLE)
  # First check if atomics work without the library.
  check_working_cxx_atomics(HAVE_CXX_ATOMICS_WITHOUT_LIB)
  # If not, check if the library exists, and atomics work with it.
  if(NOT HAVE_CXX_ATOMICS_WITHOUT_LIB)
    check_library_exists(atomic __atomic_fetch_add_4 "" HAVE_LIBATOMIC)
    if( HAVE_LIBATOMIC )
      list(APPEND CMAKE_REQUIRED_LIBRARIES "atomic")
      check_working_cxx_atomics(HAVE_CXX_ATOMICS_WITH_LIB)
      if (NOT HAVE_CXX_ATOMICS_WITH_LIB)
	message(FATAL_ERROR "Host compiler must support std::atomic!")
      endif()
    else()
      message(FATAL_ERROR "Host compiler appears to require libatomic, but cannot find it.")
    endif()
  endif()
endif()

## TODO: This define is only used for the legacy atomic operations in
## llvm's Atomic.h, which should be replaced.  Other code simply
## assumes C++11 <atomic> works.
CHECK_CXX_SOURCE_COMPILES("
#ifdef _MSC_VER
#include <Intrin.h> /* Workaround for PR19898. */
#include <windows.h>
#endif
int main() {
#ifdef _MSC_VER
        volatile LONG val = 1;
        MemoryBarrier();
        InterlockedCompareExchange(&val, 0, 1);
        InterlockedIncrement(&val);
        InterlockedDecrement(&val);
#else
        volatile unsigned long val = 1;
        __sync_synchronize();
        __sync_val_compare_and_swap(&val, 1, 0);
        __sync_add_and_fetch(&val, 1);
        __sync_sub_and_fetch(&val, 1);
#endif
        return 0;
      }
" LLVM_HAS_ATOMICS)

if( NOT LLVM_HAS_ATOMICS )
  message(STATUS "Warning: LLVM will be built thread-unsafe because atomic builtins are missing")
endif()
