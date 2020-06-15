// RUN: %clang_cc1 -E -ffreestanding -triple=arm-none-none %s | FileCheck -check-prefix ARM %s
//
// ARM:typedef long long int int64_t;
// ARM:typedef long long unsigned int uint64_t;
// ARM:typedef int64_t int_least64_t;
// ARM:typedef uint64_t uint_least64_t;
// ARM:typedef int64_t int_fast64_t;
// ARM:typedef uint64_t uint_fast64_t;
//
// ARM:typedef int int32_t;
// ARM:typedef unsigned int uint32_t;
// ARM:typedef int32_t int_least32_t;
// ARM:typedef uint32_t uint_least32_t;
// ARM:typedef int32_t int_fast32_t;
// ARM:typedef uint32_t uint_fast32_t;
// 
// ARM:typedef short int16_t;
// ARM:typedef unsigned short uint16_t;
// ARM:typedef int16_t int_least16_t;
// ARM:typedef uint16_t uint_least16_t;
// ARM:typedef int16_t int_fast16_t;
// ARM:typedef uint16_t uint_fast16_t;
//
// ARM:typedef signed char int8_t;
// ARM:typedef unsigned char uint8_t;
// ARM:typedef int8_t int_least8_t;
// ARM:typedef uint8_t uint_least8_t;
// ARM:typedef int8_t int_fast8_t;
// ARM:typedef uint8_t uint_fast8_t;
//
// ARM:typedef int32_t intptr_t;
// ARM:typedef uint32_t uintptr_t;
// 
// ARM:typedef long long int intmax_t;
// ARM:typedef long long unsigned int uintmax_t;
//
// ARM:INT8_MAX_ 127
// ARM:INT8_MIN_ (-127 -1)
// ARM:UINT8_MAX_ 255
// ARM:INT_LEAST8_MIN_ (-127 -1)
// ARM:INT_LEAST8_MAX_ 127
// ARM:UINT_LEAST8_MAX_ 255
// ARM:INT_FAST8_MIN_ (-127 -1)
// ARM:INT_FAST8_MAX_ 127
// ARM:UINT_FAST8_MAX_ 255
//
// ARM:INT16_MAX_ 32767
// ARM:INT16_MIN_ (-32767 -1)
// ARM:UINT16_MAX_ 65535
// ARM:INT_LEAST16_MIN_ (-32767 -1)
// ARM:INT_LEAST16_MAX_ 32767
// ARM:UINT_LEAST16_MAX_ 65535
// ARM:INT_FAST16_MIN_ (-32767 -1)
// ARM:INT_FAST16_MAX_ 32767
// ARM:UINT_FAST16_MAX_ 65535
//
// ARM:INT32_MAX_ 2147483647
// ARM:INT32_MIN_ (-2147483647 -1)
// ARM:UINT32_MAX_ 4294967295U
// ARM:INT_LEAST32_MIN_ (-2147483647 -1)
// ARM:INT_LEAST32_MAX_ 2147483647
// ARM:UINT_LEAST32_MAX_ 4294967295U
// ARM:INT_FAST32_MIN_ (-2147483647 -1)
// ARM:INT_FAST32_MAX_ 2147483647
// ARM:UINT_FAST32_MAX_ 4294967295U
//
// ARM:INT64_MAX_ 9223372036854775807LL
// ARM:INT64_MIN_ (-9223372036854775807LL -1)
// ARM:UINT64_MAX_ 18446744073709551615ULL
// ARM:INT_LEAST64_MIN_ (-9223372036854775807LL -1)
// ARM:INT_LEAST64_MAX_ 9223372036854775807LL
// ARM:UINT_LEAST64_MAX_ 18446744073709551615ULL
// ARM:INT_FAST64_MIN_ (-9223372036854775807LL -1)
// ARM:INT_FAST64_MAX_ 9223372036854775807LL
// ARM:UINT_FAST64_MAX_ 18446744073709551615ULL
//
// ARM:INTPTR_MIN_ (-2147483647 -1)
// ARM:INTPTR_MAX_ 2147483647
// ARM:UINTPTR_MAX_ 4294967295U
// ARM:PTRDIFF_MIN_ (-2147483647 -1)
// ARM:PTRDIFF_MAX_ 2147483647
// ARM:SIZE_MAX_ 4294967295U
//
// ARM:INTMAX_MIN_ (-9223372036854775807LL -1)
// ARM:INTMAX_MAX_ 9223372036854775807LL
// ARM:UINTMAX_MAX_ 18446744073709551615ULL
//
// ARM:SIG_ATOMIC_MIN_ (-2147483647 -1)
// ARM:SIG_ATOMIC_MAX_ 2147483647
// ARM:WINT_MIN_ (-2147483647 -1)
// ARM:WINT_MAX_ 2147483647
//
// ARM:WCHAR_MAX_ 4294967295U
// ARM:WCHAR_MIN_ 0U
//
// ARM:INT8_C_(0) 0
// ARM:UINT8_C_(0) 0U
// ARM:INT16_C_(0) 0
// ARM:UINT16_C_(0) 0U
// ARM:INT32_C_(0) 0
// ARM:UINT32_C_(0) 0U
// ARM:INT64_C_(0) 0LL
// ARM:UINT64_C_(0) 0ULL
//
// ARM:INTMAX_C_(0) 0LL
// ARM:UINTMAX_C_(0) 0ULL
//
//
// RUN: %clang_cc1 -E -ffreestanding -triple=i386-none-none %s | FileCheck -check-prefix I386 %s
//
// I386:typedef long long int int64_t;
// I386:typedef long long unsigned int uint64_t;
// I386:typedef int64_t int_least64_t;
// I386:typedef uint64_t uint_least64_t;
// I386:typedef int64_t int_fast64_t;
// I386:typedef uint64_t uint_fast64_t;
//
// I386:typedef int int32_t;
// I386:typedef unsigned int uint32_t;
// I386:typedef int32_t int_least32_t;
// I386:typedef uint32_t uint_least32_t;
// I386:typedef int32_t int_fast32_t;
// I386:typedef uint32_t uint_fast32_t;
//
// I386:typedef short int16_t;
// I386:typedef unsigned short uint16_t;
// I386:typedef int16_t int_least16_t;
// I386:typedef uint16_t uint_least16_t;
// I386:typedef int16_t int_fast16_t;
// I386:typedef uint16_t uint_fast16_t;
//
// I386:typedef signed char int8_t;
// I386:typedef unsigned char uint8_t;
// I386:typedef int8_t int_least8_t;
// I386:typedef uint8_t uint_least8_t;
// I386:typedef int8_t int_fast8_t;
// I386:typedef uint8_t uint_fast8_t;
//
// I386:typedef int32_t intptr_t;
// I386:typedef uint32_t uintptr_t;
//
// I386:typedef long long int intmax_t;
// I386:typedef long long unsigned int uintmax_t;
//
// I386:INT8_MAX_ 127
// I386:INT8_MIN_ (-127 -1)
// I386:UINT8_MAX_ 255
// I386:INT_LEAST8_MIN_ (-127 -1)
// I386:INT_LEAST8_MAX_ 127
// I386:UINT_LEAST8_MAX_ 255
// I386:INT_FAST8_MIN_ (-127 -1)
// I386:INT_FAST8_MAX_ 127
// I386:UINT_FAST8_MAX_ 255
//
// I386:INT16_MAX_ 32767
// I386:INT16_MIN_ (-32767 -1)
// I386:UINT16_MAX_ 65535
// I386:INT_LEAST16_MIN_ (-32767 -1)
// I386:INT_LEAST16_MAX_ 32767
// I386:UINT_LEAST16_MAX_ 65535
// I386:INT_FAST16_MIN_ (-32767 -1)
// I386:INT_FAST16_MAX_ 32767
// I386:UINT_FAST16_MAX_ 65535
//
// I386:INT32_MAX_ 2147483647
// I386:INT32_MIN_ (-2147483647 -1)
// I386:UINT32_MAX_ 4294967295U
// I386:INT_LEAST32_MIN_ (-2147483647 -1)
// I386:INT_LEAST32_MAX_ 2147483647
// I386:UINT_LEAST32_MAX_ 4294967295U
// I386:INT_FAST32_MIN_ (-2147483647 -1)
// I386:INT_FAST32_MAX_ 2147483647
// I386:UINT_FAST32_MAX_ 4294967295U
//
// I386:INT64_MAX_ 9223372036854775807LL
// I386:INT64_MIN_ (-9223372036854775807LL -1)
// I386:UINT64_MAX_ 18446744073709551615ULL
// I386:INT_LEAST64_MIN_ (-9223372036854775807LL -1)
// I386:INT_LEAST64_MAX_ 9223372036854775807LL
// I386:UINT_LEAST64_MAX_ 18446744073709551615ULL
// I386:INT_FAST64_MIN_ (-9223372036854775807LL -1)
// I386:INT_FAST64_MAX_ 9223372036854775807LL
// I386:UINT_FAST64_MAX_ 18446744073709551615ULL
//
// I386:INTPTR_MIN_ (-2147483647 -1)
// I386:INTPTR_MAX_ 2147483647
// I386:UINTPTR_MAX_ 4294967295U
// I386:PTRDIFF_MIN_ (-2147483647 -1)
// I386:PTRDIFF_MAX_ 2147483647
// I386:SIZE_MAX_ 4294967295U
//
// I386:INTMAX_MIN_ (-9223372036854775807LL -1)
// I386:INTMAX_MAX_ 9223372036854775807LL
// I386:UINTMAX_MAX_ 18446744073709551615ULL
//
// I386:SIG_ATOMIC_MIN_ (-2147483647 -1)
// I386:SIG_ATOMIC_MAX_ 2147483647
// I386:WINT_MIN_ (-2147483647 -1)
// I386:WINT_MAX_ 2147483647
//
// I386:WCHAR_MAX_ 2147483647
// I386:WCHAR_MIN_ (-2147483647 -1)
//
// I386:INT8_C_(0) 0
// I386:UINT8_C_(0) 0U
// I386:INT16_C_(0) 0
// I386:UINT16_C_(0) 0U
// I386:INT32_C_(0) 0
// I386:UINT32_C_(0) 0U
// I386:INT64_C_(0) 0LL
// I386:UINT64_C_(0) 0ULL
//
// I386:INTMAX_C_(0) 0LL
// I386:UINTMAX_C_(0) 0ULL
//
// RUN: %clang_cc1 -E -ffreestanding -triple=mips-none-none %s | FileCheck -check-prefix MIPS %s
//
// MIPS:typedef long long int int64_t;
// MIPS:typedef long long unsigned int uint64_t;
// MIPS:typedef int64_t int_least64_t;
// MIPS:typedef uint64_t uint_least64_t;
// MIPS:typedef int64_t int_fast64_t;
// MIPS:typedef uint64_t uint_fast64_t;
//
// MIPS:typedef int int32_t;
// MIPS:typedef unsigned int uint32_t;
// MIPS:typedef int32_t int_least32_t;
// MIPS:typedef uint32_t uint_least32_t;
// MIPS:typedef int32_t int_fast32_t;
// MIPS:typedef uint32_t uint_fast32_t;
//
// MIPS:typedef short int16_t;
// MIPS:typedef unsigned short uint16_t;
// MIPS:typedef int16_t int_least16_t;
// MIPS:typedef uint16_t uint_least16_t;
// MIPS:typedef int16_t int_fast16_t;
// MIPS:typedef uint16_t uint_fast16_t;
//
// MIPS:typedef signed char int8_t;
// MIPS:typedef unsigned char uint8_t;
// MIPS:typedef int8_t int_least8_t;
// MIPS:typedef uint8_t uint_least8_t;
// MIPS:typedef int8_t int_fast8_t;
// MIPS:typedef uint8_t uint_fast8_t;
//
// MIPS:typedef int32_t intptr_t;
// MIPS:typedef uint32_t uintptr_t;
//
// MIPS:typedef long long int intmax_t;
// MIPS:typedef long long unsigned int uintmax_t;
//
// MIPS:INT8_MAX_ 127
// MIPS:INT8_MIN_ (-127 -1)
// MIPS:UINT8_MAX_ 255
// MIPS:INT_LEAST8_MIN_ (-127 -1)
// MIPS:INT_LEAST8_MAX_ 127
// MIPS:UINT_LEAST8_MAX_ 255
// MIPS:INT_FAST8_MIN_ (-127 -1)
// MIPS:INT_FAST8_MAX_ 127
// MIPS:UINT_FAST8_MAX_ 255
//
// MIPS:INT16_MAX_ 32767
// MIPS:INT16_MIN_ (-32767 -1)
// MIPS:UINT16_MAX_ 65535
// MIPS:INT_LEAST16_MIN_ (-32767 -1)
// MIPS:INT_LEAST16_MAX_ 32767
// MIPS:UINT_LEAST16_MAX_ 65535
// MIPS:INT_FAST16_MIN_ (-32767 -1)
// MIPS:INT_FAST16_MAX_ 32767
// MIPS:UINT_FAST16_MAX_ 65535
//
// MIPS:INT32_MAX_ 2147483647
// MIPS:INT32_MIN_ (-2147483647 -1)
// MIPS:UINT32_MAX_ 4294967295U
// MIPS:INT_LEAST32_MIN_ (-2147483647 -1)
// MIPS:INT_LEAST32_MAX_ 2147483647
// MIPS:UINT_LEAST32_MAX_ 4294967295U
// MIPS:INT_FAST32_MIN_ (-2147483647 -1)
// MIPS:INT_FAST32_MAX_ 2147483647
// MIPS:UINT_FAST32_MAX_ 4294967295U
//
// MIPS:INT64_MAX_ 9223372036854775807LL
// MIPS:INT64_MIN_ (-9223372036854775807LL -1)
// MIPS:UINT64_MAX_ 18446744073709551615ULL
// MIPS:INT_LEAST64_MIN_ (-9223372036854775807LL -1)
// MIPS:INT_LEAST64_MAX_ 9223372036854775807LL
// MIPS:UINT_LEAST64_MAX_ 18446744073709551615ULL
// MIPS:INT_FAST64_MIN_ (-9223372036854775807LL -1)
// MIPS:INT_FAST64_MAX_ 9223372036854775807LL
// MIPS:UINT_FAST64_MAX_ 18446744073709551615ULL
//
// MIPS:INTPTR_MIN_ (-2147483647 -1)
// MIPS:INTPTR_MAX_ 2147483647
// MIPS:UINTPTR_MAX_ 4294967295U
// MIPS:PTRDIFF_MIN_ (-2147483647 -1)
// MIPS:PTRDIFF_MAX_ 2147483647
// MIPS:SIZE_MAX_ 4294967295U
//
// MIPS:INTMAX_MIN_ (-9223372036854775807LL -1)
// MIPS:INTMAX_MAX_ 9223372036854775807LL
// MIPS:UINTMAX_MAX_ 18446744073709551615ULL
//
// MIPS:SIG_ATOMIC_MIN_ (-2147483647 -1)
// MIPS:SIG_ATOMIC_MAX_ 2147483647
// MIPS:WINT_MIN_ (-2147483647 -1)
// MIPS:WINT_MAX_ 2147483647
//
// MIPS:WCHAR_MAX_ 2147483647
// MIPS:WCHAR_MIN_ (-2147483647 -1)
//
// MIPS:INT8_C_(0) 0
// MIPS:UINT8_C_(0) 0U
// MIPS:INT16_C_(0) 0
// MIPS:UINT16_C_(0) 0U
// MIPS:INT32_C_(0) 0
// MIPS:UINT32_C_(0) 0U
// MIPS:INT64_C_(0) 0LL
// MIPS:UINT64_C_(0) 0ULL
//
// MIPS:INTMAX_C_(0) 0LL
// MIPS:UINTMAX_C_(0) 0ULL
//
// RUN: %clang_cc1 -E -ffreestanding -triple=mips64-none-none %s | FileCheck -check-prefix MIPS64 %s
//
// MIPS64:typedef long int int64_t;
// MIPS64:typedef long unsigned int uint64_t;
// MIPS64:typedef int64_t int_least64_t;
// MIPS64:typedef uint64_t uint_least64_t;
// MIPS64:typedef int64_t int_fast64_t;
// MIPS64:typedef uint64_t uint_fast64_t;
//
// MIPS64:typedef int int32_t;
// MIPS64:typedef unsigned int uint32_t;
// MIPS64:typedef int32_t int_least32_t;
// MIPS64:typedef uint32_t uint_least32_t;
// MIPS64:typedef int32_t int_fast32_t;
// MIPS64:typedef uint32_t uint_fast32_t;
//
// MIPS64:typedef short int16_t;
// MIPS64:typedef unsigned short uint16_t;
// MIPS64:typedef int16_t int_least16_t;
// MIPS64:typedef uint16_t uint_least16_t;
// MIPS64:typedef int16_t int_fast16_t;
// MIPS64:typedef uint16_t uint_fast16_t;
//
// MIPS64:typedef signed char int8_t;
// MIPS64:typedef unsigned char uint8_t;
// MIPS64:typedef int8_t int_least8_t;
// MIPS64:typedef uint8_t uint_least8_t;
// MIPS64:typedef int8_t int_fast8_t;
// MIPS64:typedef uint8_t uint_fast8_t;
//
// MIPS64:typedef int64_t intptr_t;
// MIPS64:typedef uint64_t uintptr_t;
//
// MIPS64:typedef long int intmax_t;
// MIPS64:typedef long unsigned int uintmax_t;
//
// MIPS64:INT8_MAX_ 127
// MIPS64:INT8_MIN_ (-127 -1)
// MIPS64:UINT8_MAX_ 255
// MIPS64:INT_LEAST8_MIN_ (-127 -1)
// MIPS64:INT_LEAST8_MAX_ 127
// MIPS64:UINT_LEAST8_MAX_ 255
// MIPS64:INT_FAST8_MIN_ (-127 -1)
// MIPS64:INT_FAST8_MAX_ 127
// MIPS64:UINT_FAST8_MAX_ 255
//
// MIPS64:INT16_MAX_ 32767
// MIPS64:INT16_MIN_ (-32767 -1)
// MIPS64:UINT16_MAX_ 65535
// MIPS64:INT_LEAST16_MIN_ (-32767 -1)
// MIPS64:INT_LEAST16_MAX_ 32767
// MIPS64:UINT_LEAST16_MAX_ 65535
// MIPS64:INT_FAST16_MIN_ (-32767 -1)
// MIPS64:INT_FAST16_MAX_ 32767
// MIPS64:UINT_FAST16_MAX_ 65535
//
// MIPS64:INT32_MAX_ 2147483647
// MIPS64:INT32_MIN_ (-2147483647 -1)
// MIPS64:UINT32_MAX_ 4294967295U
// MIPS64:INT_LEAST32_MIN_ (-2147483647 -1)
// MIPS64:INT_LEAST32_MAX_ 2147483647
// MIPS64:UINT_LEAST32_MAX_ 4294967295U
// MIPS64:INT_FAST32_MIN_ (-2147483647 -1)
// MIPS64:INT_FAST32_MAX_ 2147483647
// MIPS64:UINT_FAST32_MAX_ 4294967295U
//
// MIPS64:INT64_MAX_ 9223372036854775807L
// MIPS64:INT64_MIN_ (-9223372036854775807L -1)
// MIPS64:UINT64_MAX_ 18446744073709551615UL
// MIPS64:INT_LEAST64_MIN_ (-9223372036854775807L -1)
// MIPS64:INT_LEAST64_MAX_ 9223372036854775807L
// MIPS64:UINT_LEAST64_MAX_ 18446744073709551615UL
// MIPS64:INT_FAST64_MIN_ (-9223372036854775807L -1)
// MIPS64:INT_FAST64_MAX_ 9223372036854775807L
// MIPS64:UINT_FAST64_MAX_ 18446744073709551615UL
//
// MIPS64:INTPTR_MIN_ (-9223372036854775807L -1)
// MIPS64:INTPTR_MAX_ 9223372036854775807L
// MIPS64:UINTPTR_MAX_ 18446744073709551615UL
// MIPS64:PTRDIFF_MIN_ (-9223372036854775807L -1)
// MIPS64:PTRDIFF_MAX_ 9223372036854775807L
// MIPS64:SIZE_MAX_ 18446744073709551615UL
//
// MIPS64:INTMAX_MIN_ (-9223372036854775807L -1)
// MIPS64:INTMAX_MAX_ 9223372036854775807L
// MIPS64:UINTMAX_MAX_ 18446744073709551615UL
//
// MIPS64:SIG_ATOMIC_MIN_ (-2147483647 -1)
// MIPS64:SIG_ATOMIC_MAX_ 2147483647
// MIPS64:WINT_MIN_ (-2147483647 -1)
// MIPS64:WINT_MAX_ 2147483647
//
// MIPS64:WCHAR_MAX_ 2147483647
// MIPS64:WCHAR_MIN_ (-2147483647 -1)
//
// MIPS64:INT8_C_(0) 0
// MIPS64:UINT8_C_(0) 0U
// MIPS64:INT16_C_(0) 0
// MIPS64:UINT16_C_(0) 0U
// MIPS64:INT32_C_(0) 0
// MIPS64:UINT32_C_(0) 0U
// MIPS64:INT64_C_(0) 0L
// MIPS64:UINT64_C_(0) 0UL
//
// MIPS64:INTMAX_C_(0) 0L
// MIPS64:UINTMAX_C_(0) 0UL
//
// RUN: %clang_cc1 -E -ffreestanding -triple=msp430-none-none %s | FileCheck -check-prefix MSP430 %s
//
// MSP430:typedef long int int32_t;
// MSP430:typedef long unsigned int uint32_t;
// MSP430:typedef int32_t int_least32_t;
// MSP430:typedef uint32_t uint_least32_t;
// MSP430:typedef int32_t int_fast32_t;
// MSP430:typedef uint32_t uint_fast32_t;
//
// MSP430:typedef short int16_t;
// MSP430:typedef unsigned short uint16_t;
// MSP430:typedef int16_t int_least16_t;
// MSP430:typedef uint16_t uint_least16_t;
// MSP430:typedef int16_t int_fast16_t;
// MSP430:typedef uint16_t uint_fast16_t;
//
// MSP430:typedef signed char int8_t;
// MSP430:typedef unsigned char uint8_t;
// MSP430:typedef int8_t int_least8_t;
// MSP430:typedef uint8_t uint_least8_t;
// MSP430:typedef int8_t int_fast8_t;
// MSP430:typedef uint8_t uint_fast8_t;
//
// MSP430:typedef int16_t intptr_t;
// MSP430:typedef uint16_t uintptr_t;
//
// MSP430:typedef long long int intmax_t;
// MSP430:typedef long long unsigned int uintmax_t;
//
// MSP430:INT8_MAX_ 127
// MSP430:INT8_MIN_ (-127 -1)
// MSP430:UINT8_MAX_ 255
// MSP430:INT_LEAST8_MIN_ (-127 -1)
// MSP430:INT_LEAST8_MAX_ 127
// MSP430:UINT_LEAST8_MAX_ 255
// MSP430:INT_FAST8_MIN_ (-127 -1)
// MSP430:INT_FAST8_MAX_ 127
// MSP430:UINT_FAST8_MAX_ 255
//
// MSP430:INT16_MAX_ 32767
// MSP430:INT16_MIN_ (-32767 -1)
// MSP430:UINT16_MAX_ 65535
// MSP430:INT_LEAST16_MIN_ (-32767 -1)
// MSP430:INT_LEAST16_MAX_ 32767
// MSP430:UINT_LEAST16_MAX_ 65535
// MSP430:INT_FAST16_MIN_ (-32767 -1)
// MSP430:INT_FAST16_MAX_ 32767
// MSP430:UINT_FAST16_MAX_ 65535
//
// MSP430:INT32_MAX_ 2147483647L
// MSP430:INT32_MIN_ (-2147483647L -1)
// MSP430:UINT32_MAX_ 4294967295UL
// MSP430:INT_LEAST32_MIN_ (-2147483647L -1)
// MSP430:INT_LEAST32_MAX_ 2147483647L
// MSP430:UINT_LEAST32_MAX_ 4294967295UL
// MSP430:INT_FAST32_MIN_ (-2147483647L -1)
// MSP430:INT_FAST32_MAX_ 2147483647L
// MSP430:UINT_FAST32_MAX_ 4294967295UL
//
// MSP430:INT64_MAX_ 9223372036854775807LL
// MSP430:INT64_MIN_ (-9223372036854775807LL -1)
// MSP430:UINT64_MAX_ 18446744073709551615ULL
// MSP430:INT_LEAST64_MIN_ (-9223372036854775807LL -1)
// MSP430:INT_LEAST64_MAX_ 9223372036854775807LL
// MSP430:UINT_LEAST64_MAX_ 18446744073709551615ULL
// MSP430:INT_FAST64_MIN_ (-9223372036854775807LL -1)
// MSP430:INT_FAST64_MAX_ 9223372036854775807LL
// MSP430:UINT_FAST64_MAX_ 18446744073709551615ULL
//
// MSP430:INTPTR_MIN_ (-32767 -1)
// MSP430:INTPTR_MAX_ 32767
// MSP430:UINTPTR_MAX_ 65535
// MSP430:PTRDIFF_MIN_ (-32767 -1)
// MSP430:PTRDIFF_MAX_ 32767
// MSP430:SIZE_MAX_ 65535
//
// MSP430:INTMAX_MIN_ (-9223372036854775807LL -1)
// MSP430:INTMAX_MAX_ 9223372036854775807LL
// MSP430:UINTMAX_MAX_ 18446744073709551615ULL
//
// MSP430:SIG_ATOMIC_MIN_ (-2147483647L -1)
// MSP430:SIG_ATOMIC_MAX_ 2147483647L
// MSP430:WINT_MIN_ (-32767 -1)
// MSP430:WINT_MAX_ 32767
//
// MSP430:WCHAR_MAX_ 32767
// MSP430:WCHAR_MIN_ (-32767 -1)
//
// MSP430:INT8_C_(0) 0
// MSP430:UINT8_C_(0) 0U
// MSP430:INT16_C_(0) 0
// MSP430:UINT16_C_(0) 0U
// MSP430:INT32_C_(0) 0L
// MSP430:UINT32_C_(0) 0UL
// MSP430:INT64_C_(0) 0LL
// MSP430:UINT64_C_(0) 0ULL
//
// MSP430:INTMAX_C_(0) 0L
// MSP430:UINTMAX_C_(0) 0UL
//
// RUN: %clang_cc1 -E -ffreestanding -triple=powerpc64-none-none %s | FileCheck -check-prefix PPC64 %s
//
// PPC64:typedef long int int64_t;
// PPC64:typedef long unsigned int uint64_t;
// PPC64:typedef int64_t int_least64_t;
// PPC64:typedef uint64_t uint_least64_t;
// PPC64:typedef int64_t int_fast64_t;
// PPC64:typedef uint64_t uint_fast64_t;
//
// PPC64:typedef int int32_t;
// PPC64:typedef unsigned int uint32_t;
// PPC64:typedef int32_t int_least32_t;
// PPC64:typedef uint32_t uint_least32_t;
// PPC64:typedef int32_t int_fast32_t;
// PPC64:typedef uint32_t uint_fast32_t;
//
// PPC64:typedef short int16_t;
// PPC64:typedef unsigned short uint16_t;
// PPC64:typedef int16_t int_least16_t;
// PPC64:typedef uint16_t uint_least16_t;
// PPC64:typedef int16_t int_fast16_t;
// PPC64:typedef uint16_t uint_fast16_t;
//
// PPC64:typedef signed char int8_t;
// PPC64:typedef unsigned char uint8_t;
// PPC64:typedef int8_t int_least8_t;
// PPC64:typedef uint8_t uint_least8_t;
// PPC64:typedef int8_t int_fast8_t;
// PPC64:typedef uint8_t uint_fast8_t;
//
// PPC64:typedef int64_t intptr_t;
// PPC64:typedef uint64_t uintptr_t;
//
// PPC64:typedef long int intmax_t;
// PPC64:typedef long unsigned int uintmax_t;
//
// PPC64:INT8_MAX_ 127
// PPC64:INT8_MIN_ (-127 -1)
// PPC64:UINT8_MAX_ 255
// PPC64:INT_LEAST8_MIN_ (-127 -1)
// PPC64:INT_LEAST8_MAX_ 127
// PPC64:UINT_LEAST8_MAX_ 255
// PPC64:INT_FAST8_MIN_ (-127 -1)
// PPC64:INT_FAST8_MAX_ 127
// PPC64:UINT_FAST8_MAX_ 255
//
// PPC64:INT16_MAX_ 32767
// PPC64:INT16_MIN_ (-32767 -1)
// PPC64:UINT16_MAX_ 65535
// PPC64:INT_LEAST16_MIN_ (-32767 -1)
// PPC64:INT_LEAST16_MAX_ 32767
// PPC64:UINT_LEAST16_MAX_ 65535
// PPC64:INT_FAST16_MIN_ (-32767 -1)
// PPC64:INT_FAST16_MAX_ 32767
// PPC64:UINT_FAST16_MAX_ 65535
//
// PPC64:INT32_MAX_ 2147483647
// PPC64:INT32_MIN_ (-2147483647 -1)
// PPC64:UINT32_MAX_ 4294967295U
// PPC64:INT_LEAST32_MIN_ (-2147483647 -1)
// PPC64:INT_LEAST32_MAX_ 2147483647
// PPC64:UINT_LEAST32_MAX_ 4294967295U
// PPC64:INT_FAST32_MIN_ (-2147483647 -1)
// PPC64:INT_FAST32_MAX_ 2147483647
// PPC64:UINT_FAST32_MAX_ 4294967295U
//
// PPC64:INT64_MAX_ 9223372036854775807L
// PPC64:INT64_MIN_ (-9223372036854775807L -1)
// PPC64:UINT64_MAX_ 18446744073709551615UL
// PPC64:INT_LEAST64_MIN_ (-9223372036854775807L -1)
// PPC64:INT_LEAST64_MAX_ 9223372036854775807L
// PPC64:UINT_LEAST64_MAX_ 18446744073709551615UL
// PPC64:INT_FAST64_MIN_ (-9223372036854775807L -1)
// PPC64:INT_FAST64_MAX_ 9223372036854775807L
// PPC64:UINT_FAST64_MAX_ 18446744073709551615UL
//
// PPC64:INTPTR_MIN_ (-9223372036854775807L -1)
// PPC64:INTPTR_MAX_ 9223372036854775807L
// PPC64:UINTPTR_MAX_ 18446744073709551615UL
// PPC64:PTRDIFF_MIN_ (-9223372036854775807L -1)
// PPC64:PTRDIFF_MAX_ 9223372036854775807L
// PPC64:SIZE_MAX_ 18446744073709551615UL
//
// PPC64:INTMAX_MIN_ (-9223372036854775807L -1)
// PPC64:INTMAX_MAX_ 9223372036854775807L
// PPC64:UINTMAX_MAX_ 18446744073709551615UL
//
// PPC64:SIG_ATOMIC_MIN_ (-2147483647 -1)
// PPC64:SIG_ATOMIC_MAX_ 2147483647
// PPC64:WINT_MIN_ (-2147483647 -1)
// PPC64:WINT_MAX_ 2147483647
//
// PPC64:WCHAR_MAX_ 2147483647
// PPC64:WCHAR_MIN_ (-2147483647 -1)
//
// PPC64:INT8_C_(0) 0
// PPC64:UINT8_C_(0) 0U
// PPC64:INT16_C_(0) 0
// PPC64:UINT16_C_(0) 0U
// PPC64:INT32_C_(0) 0
// PPC64:UINT32_C_(0) 0U
// PPC64:INT64_C_(0) 0L
// PPC64:UINT64_C_(0) 0UL
//
// PPC64:INTMAX_C_(0) 0L
// PPC64:UINTMAX_C_(0) 0UL
//
// RUN: %clang_cc1 -E -ffreestanding -triple=powerpc64-none-netbsd %s | FileCheck -check-prefix PPC64-NETBSD %s
//
// PPC64-NETBSD:typedef long long int int64_t;
// PPC64-NETBSD:typedef long long unsigned int uint64_t;
// PPC64-NETBSD:typedef int64_t int_least64_t;
// PPC64-NETBSD:typedef uint64_t uint_least64_t;
// PPC64-NETBSD:typedef int64_t int_fast64_t;
// PPC64-NETBSD:typedef uint64_t uint_fast64_t;
//
// PPC64-NETBSD:typedef int int32_t;
// PPC64-NETBSD:typedef unsigned int uint32_t;
// PPC64-NETBSD:typedef int32_t int_least32_t;
// PPC64-NETBSD:typedef uint32_t uint_least32_t;
// PPC64-NETBSD:typedef int32_t int_fast32_t;
// PPC64-NETBSD:typedef uint32_t uint_fast32_t;
//
// PPC64-NETBSD:typedef short int16_t;
// PPC64-NETBSD:typedef unsigned short uint16_t;
// PPC64-NETBSD:typedef int16_t int_least16_t;
// PPC64-NETBSD:typedef uint16_t uint_least16_t;
// PPC64-NETBSD:typedef int16_t int_fast16_t;
// PPC64-NETBSD:typedef uint16_t uint_fast16_t;
//
// PPC64-NETBSD:typedef signed char int8_t;
// PPC64-NETBSD:typedef unsigned char uint8_t;
// PPC64-NETBSD:typedef int8_t int_least8_t;
// PPC64-NETBSD:typedef uint8_t uint_least8_t;
// PPC64-NETBSD:typedef int8_t int_fast8_t;
// PPC64-NETBSD:typedef uint8_t uint_fast8_t;
//
// PPC64-NETBSD:typedef int64_t intptr_t;
// PPC64-NETBSD:typedef uint64_t uintptr_t;
//
// PPC64-NETBSD:typedef long long int intmax_t;
// PPC64-NETBSD:typedef long long unsigned int uintmax_t;
//
// PPC64-NETBSD:INT8_MAX_ 127
// PPC64-NETBSD:INT8_MIN_ (-127 -1)
// PPC64-NETBSD:UINT8_MAX_ 255
// PPC64-NETBSD:INT_LEAST8_MIN_ (-127 -1)
// PPC64-NETBSD:INT_LEAST8_MAX_ 127
// PPC64-NETBSD:UINT_LEAST8_MAX_ 255
// PPC64-NETBSD:INT_FAST8_MIN_ (-127 -1)
// PPC64-NETBSD:INT_FAST8_MAX_ 127
// PPC64-NETBSD:UINT_FAST8_MAX_ 255
//
// PPC64-NETBSD:INT16_MAX_ 32767
// PPC64-NETBSD:INT16_MIN_ (-32767 -1)
// PPC64-NETBSD:UINT16_MAX_ 65535
// PPC64-NETBSD:INT_LEAST16_MIN_ (-32767 -1)
// PPC64-NETBSD:INT_LEAST16_MAX_ 32767
// PPC64-NETBSD:UINT_LEAST16_MAX_ 65535
// PPC64-NETBSD:INT_FAST16_MIN_ (-32767 -1)
// PPC64-NETBSD:INT_FAST16_MAX_ 32767
// PPC64-NETBSD:UINT_FAST16_MAX_ 65535
//
// PPC64-NETBSD:INT32_MAX_ 2147483647
// PPC64-NETBSD:INT32_MIN_ (-2147483647 -1)
// PPC64-NETBSD:UINT32_MAX_ 4294967295U
// PPC64-NETBSD:INT_LEAST32_MIN_ (-2147483647 -1)
// PPC64-NETBSD:INT_LEAST32_MAX_ 2147483647
// PPC64-NETBSD:UINT_LEAST32_MAX_ 4294967295U
// PPC64-NETBSD:INT_FAST32_MIN_ (-2147483647 -1)
// PPC64-NETBSD:INT_FAST32_MAX_ 2147483647
// PPC64-NETBSD:UINT_FAST32_MAX_ 4294967295U
//
// PPC64-NETBSD:INT64_MAX_ 9223372036854775807LL
// PPC64-NETBSD:INT64_MIN_ (-9223372036854775807LL -1)
// PPC64-NETBSD:UINT64_MAX_ 18446744073709551615ULL
// PPC64-NETBSD:INT_LEAST64_MIN_ (-9223372036854775807LL -1)
// PPC64-NETBSD:INT_LEAST64_MAX_ 9223372036854775807LL
// PPC64-NETBSD:UINT_LEAST64_MAX_ 18446744073709551615ULL
// PPC64-NETBSD:INT_FAST64_MIN_ (-9223372036854775807LL -1)
// PPC64-NETBSD:INT_FAST64_MAX_ 9223372036854775807LL
// PPC64-NETBSD:UINT_FAST64_MAX_ 18446744073709551615ULL
//
// PPC64-NETBSD:INTPTR_MIN_ (-9223372036854775807LL -1)
// PPC64-NETBSD:INTPTR_MAX_ 9223372036854775807LL
// PPC64-NETBSD:UINTPTR_MAX_ 18446744073709551615ULL
// PPC64-NETBSD:PTRDIFF_MIN_ (-9223372036854775807LL -1)
// PPC64-NETBSD:PTRDIFF_MAX_ 9223372036854775807LL
// PPC64-NETBSD:SIZE_MAX_ 18446744073709551615ULL
//
// PPC64-NETBSD:INTMAX_MIN_ (-9223372036854775807LL -1)
// PPC64-NETBSD:INTMAX_MAX_ 9223372036854775807LL
// PPC64-NETBSD:UINTMAX_MAX_ 18446744073709551615ULL
//
// PPC64-NETBSD:SIG_ATOMIC_MIN_ (-2147483647 -1)
// PPC64-NETBSD:SIG_ATOMIC_MAX_ 2147483647
// PPC64-NETBSD:WINT_MIN_ (-2147483647 -1)
// PPC64-NETBSD:WINT_MAX_ 2147483647
//
// PPC64-NETBSD:WCHAR_MAX_ 2147483647
// PPC64-NETBSD:WCHAR_MIN_ (-2147483647 -1)
//
// PPC64-NETBSD:INT8_C_(0) 0
// PPC64-NETBSD:UINT8_C_(0) 0U
// PPC64-NETBSD:INT16_C_(0) 0
// PPC64-NETBSD:UINT16_C_(0) 0U
// PPC64-NETBSD:INT32_C_(0) 0
// PPC64-NETBSD:UINT32_C_(0) 0U
// PPC64-NETBSD:INT64_C_(0) 0LL
// PPC64-NETBSD:UINT64_C_(0) 0ULL
//
// PPC64-NETBSD:INTMAX_C_(0) 0LL
// PPC64-NETBSD:UINTMAX_C_(0) 0ULL
//
// RUN: %clang_cc1 -E -ffreestanding -triple=powerpc-none-none %s | FileCheck -check-prefix PPC %s
//
//
// PPC:typedef long long int int64_t;
// PPC:typedef long long unsigned int uint64_t;
// PPC:typedef int64_t int_least64_t;
// PPC:typedef uint64_t uint_least64_t;
// PPC:typedef int64_t int_fast64_t;
// PPC:typedef uint64_t uint_fast64_t;
//
// PPC:typedef int int32_t;
// PPC:typedef unsigned int uint32_t;
// PPC:typedef int32_t int_least32_t;
// PPC:typedef uint32_t uint_least32_t;
// PPC:typedef int32_t int_fast32_t;
// PPC:typedef uint32_t uint_fast32_t;
//
// PPC:typedef short int16_t;
// PPC:typedef unsigned short uint16_t;
// PPC:typedef int16_t int_least16_t;
// PPC:typedef uint16_t uint_least16_t;
// PPC:typedef int16_t int_fast16_t;
// PPC:typedef uint16_t uint_fast16_t;
//
// PPC:typedef signed char int8_t;
// PPC:typedef unsigned char uint8_t;
// PPC:typedef int8_t int_least8_t;
// PPC:typedef uint8_t uint_least8_t;
// PPC:typedef int8_t int_fast8_t;
// PPC:typedef uint8_t uint_fast8_t;
//
// PPC:typedef int32_t intptr_t;
// PPC:typedef uint32_t uintptr_t;
//
// PPC:typedef long long int intmax_t;
// PPC:typedef long long unsigned int uintmax_t;
//
// PPC:INT8_MAX_ 127
// PPC:INT8_MIN_ (-127 -1)
// PPC:UINT8_MAX_ 255
// PPC:INT_LEAST8_MIN_ (-127 -1)
// PPC:INT_LEAST8_MAX_ 127
// PPC:UINT_LEAST8_MAX_ 255
// PPC:INT_FAST8_MIN_ (-127 -1)
// PPC:INT_FAST8_MAX_ 127
// PPC:UINT_FAST8_MAX_ 255
//
// PPC:INT16_MAX_ 32767
// PPC:INT16_MIN_ (-32767 -1)
// PPC:UINT16_MAX_ 65535
// PPC:INT_LEAST16_MIN_ (-32767 -1)
// PPC:INT_LEAST16_MAX_ 32767
// PPC:UINT_LEAST16_MAX_ 65535
// PPC:INT_FAST16_MIN_ (-32767 -1)
// PPC:INT_FAST16_MAX_ 32767
// PPC:UINT_FAST16_MAX_ 65535
//
// PPC:INT32_MAX_ 2147483647
// PPC:INT32_MIN_ (-2147483647 -1)
// PPC:UINT32_MAX_ 4294967295U
// PPC:INT_LEAST32_MIN_ (-2147483647 -1)
// PPC:INT_LEAST32_MAX_ 2147483647
// PPC:UINT_LEAST32_MAX_ 4294967295U
// PPC:INT_FAST32_MIN_ (-2147483647 -1)
// PPC:INT_FAST32_MAX_ 2147483647
// PPC:UINT_FAST32_MAX_ 4294967295U
//
// PPC:INT64_MAX_ 9223372036854775807LL
// PPC:INT64_MIN_ (-9223372036854775807LL -1)
// PPC:UINT64_MAX_ 18446744073709551615ULL
// PPC:INT_LEAST64_MIN_ (-9223372036854775807LL -1)
// PPC:INT_LEAST64_MAX_ 9223372036854775807LL
// PPC:UINT_LEAST64_MAX_ 18446744073709551615ULL
// PPC:INT_FAST64_MIN_ (-9223372036854775807LL -1)
// PPC:INT_FAST64_MAX_ 9223372036854775807LL
// PPC:UINT_FAST64_MAX_ 18446744073709551615ULL
//
// PPC:INTPTR_MIN_ (-2147483647 -1)
// PPC:INTPTR_MAX_ 2147483647
// PPC:UINTPTR_MAX_ 4294967295U
// PPC:PTRDIFF_MIN_ (-2147483647 -1)
// PPC:PTRDIFF_MAX_ 2147483647
// PPC:SIZE_MAX_ 4294967295U
//
// PPC:INTMAX_MIN_ (-9223372036854775807LL -1)
// PPC:INTMAX_MAX_ 9223372036854775807LL
// PPC:UINTMAX_MAX_ 18446744073709551615ULL
//
// PPC:SIG_ATOMIC_MIN_ (-2147483647 -1)
// PPC:SIG_ATOMIC_MAX_ 2147483647
// PPC:WINT_MIN_ (-2147483647 -1)
// PPC:WINT_MAX_ 2147483647
//
// PPC:WCHAR_MAX_ 2147483647
// PPC:WCHAR_MIN_ (-2147483647 -1)
//
// PPC:INT8_C_(0) 0
// PPC:UINT8_C_(0) 0U
// PPC:INT16_C_(0) 0
// PPC:UINT16_C_(0) 0U
// PPC:INT32_C_(0) 0
// PPC:UINT32_C_(0) 0U
// PPC:INT64_C_(0) 0LL
// PPC:UINT64_C_(0) 0ULL
//
// PPC:INTMAX_C_(0) 0LL
// PPC:UINTMAX_C_(0) 0ULL
//
// RUN: %clang_cc1 -E -ffreestanding -triple=s390x-none-none %s | FileCheck -check-prefix S390X %s
//
// S390X:typedef long int int64_t;
// S390X:typedef long unsigned int uint64_t;
// S390X:typedef int64_t int_least64_t;
// S390X:typedef uint64_t uint_least64_t;
// S390X:typedef int64_t int_fast64_t;
// S390X:typedef uint64_t uint_fast64_t;
//
// S390X:typedef int int32_t;
// S390X:typedef unsigned int uint32_t;
// S390X:typedef int32_t int_least32_t;
// S390X:typedef uint32_t uint_least32_t;
// S390X:typedef int32_t int_fast32_t;
// S390X:typedef uint32_t uint_fast32_t;
//
// S390X:typedef short int16_t;
// S390X:typedef unsigned short uint16_t;
// S390X:typedef int16_t int_least16_t;
// S390X:typedef uint16_t uint_least16_t;
// S390X:typedef int16_t int_fast16_t;
// S390X:typedef uint16_t uint_fast16_t;
//
// S390X:typedef signed char int8_t;
// S390X:typedef unsigned char uint8_t;
// S390X:typedef int8_t int_least8_t;
// S390X:typedef uint8_t uint_least8_t;
// S390X:typedef int8_t int_fast8_t;
// S390X:typedef uint8_t uint_fast8_t;
//
// S390X:typedef int64_t intptr_t;
// S390X:typedef uint64_t uintptr_t;
//
// S390X:typedef long int intmax_t;
// S390X:typedef long unsigned int uintmax_t;
//
// S390X:INT8_MAX_ 127
// S390X:INT8_MIN_ (-127 -1)
// S390X:UINT8_MAX_ 255
// S390X:INT_LEAST8_MIN_ (-127 -1)
// S390X:INT_LEAST8_MAX_ 127
// S390X:UINT_LEAST8_MAX_ 255
// S390X:INT_FAST8_MIN_ (-127 -1)
// S390X:INT_FAST8_MAX_ 127
// S390X:UINT_FAST8_MAX_ 255
//
// S390X:INT16_MAX_ 32767
// S390X:INT16_MIN_ (-32767 -1)
// S390X:UINT16_MAX_ 65535
// S390X:INT_LEAST16_MIN_ (-32767 -1)
// S390X:INT_LEAST16_MAX_ 32767
// S390X:UINT_LEAST16_MAX_ 65535
// S390X:INT_FAST16_MIN_ (-32767 -1)
// S390X:INT_FAST16_MAX_ 32767
// S390X:UINT_FAST16_MAX_ 65535
//
// S390X:INT32_MAX_ 2147483647
// S390X:INT32_MIN_ (-2147483647 -1)
// S390X:UINT32_MAX_ 4294967295U
// S390X:INT_LEAST32_MIN_ (-2147483647 -1)
// S390X:INT_LEAST32_MAX_ 2147483647
// S390X:UINT_LEAST32_MAX_ 4294967295U
// S390X:INT_FAST32_MIN_ (-2147483647 -1)
// S390X:INT_FAST32_MAX_ 2147483647
// S390X:UINT_FAST32_MAX_ 4294967295U
//
// S390X:INT64_MAX_ 9223372036854775807L
// S390X:INT64_MIN_ (-9223372036854775807L -1)
// S390X:UINT64_MAX_ 18446744073709551615UL
// S390X:INT_LEAST64_MIN_ (-9223372036854775807L -1)
// S390X:INT_LEAST64_MAX_ 9223372036854775807L
// S390X:UINT_LEAST64_MAX_ 18446744073709551615UL
// S390X:INT_FAST64_MIN_ (-9223372036854775807L -1)
// S390X:INT_FAST64_MAX_ 9223372036854775807L
// S390X:UINT_FAST64_MAX_ 18446744073709551615UL
//
// S390X:INTPTR_MIN_ (-9223372036854775807L -1)
// S390X:INTPTR_MAX_ 9223372036854775807L
// S390X:UINTPTR_MAX_ 18446744073709551615UL
// S390X:PTRDIFF_MIN_ (-9223372036854775807L -1)
// S390X:PTRDIFF_MAX_ 9223372036854775807L
// S390X:SIZE_MAX_ 18446744073709551615UL
//
// S390X:INTMAX_MIN_ (-9223372036854775807L -1)
// S390X:INTMAX_MAX_ 9223372036854775807L
// S390X:UINTMAX_MAX_ 18446744073709551615UL
//
// S390X:SIG_ATOMIC_MIN_ (-2147483647 -1)
// S390X:SIG_ATOMIC_MAX_ 2147483647
// S390X:WINT_MIN_ (-2147483647 -1)
// S390X:WINT_MAX_ 2147483647
//
// S390X:WCHAR_MAX_ 2147483647
// S390X:WCHAR_MIN_ (-2147483647 -1)
//
// S390X:INT8_C_(0) 0
// S390X:UINT8_C_(0) 0U
// S390X:INT16_C_(0) 0
// S390X:UINT16_C_(0) 0U
// S390X:INT32_C_(0) 0
// S390X:UINT32_C_(0) 0U
// S390X:INT64_C_(0) 0L
// S390X:UINT64_C_(0) 0UL
//
// S390X:INTMAX_C_(0) 0L
// S390X:UINTMAX_C_(0) 0UL
//
// RUN: %clang_cc1 -E -ffreestanding -triple=sparc-none-none %s | FileCheck -check-prefix SPARC %s
//
// SPARC:typedef long long int int64_t;
// SPARC:typedef long long unsigned int uint64_t;
// SPARC:typedef int64_t int_least64_t;
// SPARC:typedef uint64_t uint_least64_t;
// SPARC:typedef int64_t int_fast64_t;
// SPARC:typedef uint64_t uint_fast64_t;
//
// SPARC:typedef int int32_t;
// SPARC:typedef unsigned int uint32_t;
// SPARC:typedef int32_t int_least32_t;
// SPARC:typedef uint32_t uint_least32_t;
// SPARC:typedef int32_t int_fast32_t;
// SPARC:typedef uint32_t uint_fast32_t;
//
// SPARC:typedef short int16_t;
// SPARC:typedef unsigned short uint16_t;
// SPARC:typedef int16_t int_least16_t;
// SPARC:typedef uint16_t uint_least16_t;
// SPARC:typedef int16_t int_fast16_t;
// SPARC:typedef uint16_t uint_fast16_t;
//
// SPARC:typedef signed char int8_t;
// SPARC:typedef unsigned char uint8_t;
// SPARC:typedef int8_t int_least8_t;
// SPARC:typedef uint8_t uint_least8_t;
// SPARC:typedef int8_t int_fast8_t;
// SPARC:typedef uint8_t uint_fast8_t;
//
// SPARC:typedef int32_t intptr_t;
// SPARC:typedef uint32_t uintptr_t;
//
// SPARC:typedef long long int intmax_t;
// SPARC:typedef long long unsigned int uintmax_t;
//
// SPARC:INT8_MAX_ 127
// SPARC:INT8_MIN_ (-127 -1)
// SPARC:UINT8_MAX_ 255
// SPARC:INT_LEAST8_MIN_ (-127 -1)
// SPARC:INT_LEAST8_MAX_ 127
// SPARC:UINT_LEAST8_MAX_ 255
// SPARC:INT_FAST8_MIN_ (-127 -1)
// SPARC:INT_FAST8_MAX_ 127
// SPARC:UINT_FAST8_MAX_ 255
//
// SPARC:INT16_MAX_ 32767
// SPARC:INT16_MIN_ (-32767 -1)
// SPARC:UINT16_MAX_ 65535
// SPARC:INT_LEAST16_MIN_ (-32767 -1)
// SPARC:INT_LEAST16_MAX_ 32767
// SPARC:UINT_LEAST16_MAX_ 65535
// SPARC:INT_FAST16_MIN_ (-32767 -1)
// SPARC:INT_FAST16_MAX_ 32767
// SPARC:UINT_FAST16_MAX_ 65535
//
// SPARC:INT32_MAX_ 2147483647
// SPARC:INT32_MIN_ (-2147483647 -1)
// SPARC:UINT32_MAX_ 4294967295U
// SPARC:INT_LEAST32_MIN_ (-2147483647 -1)
// SPARC:INT_LEAST32_MAX_ 2147483647
// SPARC:UINT_LEAST32_MAX_ 4294967295U
// SPARC:INT_FAST32_MIN_ (-2147483647 -1)
// SPARC:INT_FAST32_MAX_ 2147483647
// SPARC:UINT_FAST32_MAX_ 4294967295U
//
// SPARC:INT64_MAX_ 9223372036854775807LL
// SPARC:INT64_MIN_ (-9223372036854775807LL -1)
// SPARC:UINT64_MAX_ 18446744073709551615ULL
// SPARC:INT_LEAST64_MIN_ (-9223372036854775807LL -1)
// SPARC:INT_LEAST64_MAX_ 9223372036854775807LL
// SPARC:UINT_LEAST64_MAX_ 18446744073709551615ULL
// SPARC:INT_FAST64_MIN_ (-9223372036854775807LL -1)
// SPARC:INT_FAST64_MAX_ 9223372036854775807LL
// SPARC:UINT_FAST64_MAX_ 18446744073709551615ULL
//
// SPARC:INTPTR_MIN_ (-2147483647 -1)
// SPARC:INTPTR_MAX_ 2147483647
// SPARC:UINTPTR_MAX_ 4294967295U
// SPARC:PTRDIFF_MIN_ (-2147483647 -1)
// SPARC:PTRDIFF_MAX_ 2147483647
// SPARC:SIZE_MAX_ 4294967295U
//
// SPARC:INTMAX_MIN_ (-9223372036854775807LL -1)
// SPARC:INTMAX_MAX_ 9223372036854775807LL
// SPARC:UINTMAX_MAX_ 18446744073709551615ULL
//
// SPARC:SIG_ATOMIC_MIN_ (-2147483647 -1)
// SPARC:SIG_ATOMIC_MAX_ 2147483647
// SPARC:WINT_MIN_ (-2147483647 -1)
// SPARC:WINT_MAX_ 2147483647
//
// SPARC:WCHAR_MAX_ 2147483647
// SPARC:WCHAR_MIN_ (-2147483647 -1)
//
// SPARC:INT8_C_(0) 0
// SPARC:UINT8_C_(0) 0U
// SPARC:INT16_C_(0) 0
// SPARC:UINT16_C_(0) 0U
// SPARC:INT32_C_(0) 0
// SPARC:UINT32_C_(0) 0U
// SPARC:INT64_C_(0) 0LL
// SPARC:UINT64_C_(0) 0ULL
//
// SPARC:INTMAX_C_(0) 0LL
// SPARC:UINTMAX_C_(0) 0ULL
//
// RUN: %clang_cc1 -E -ffreestanding -triple=tce-none-none %s | FileCheck -check-prefix TCE %s
//
// TCE:typedef int int32_t;
// TCE:typedef unsigned int uint32_t;
// TCE:typedef int32_t int_least32_t;
// TCE:typedef uint32_t uint_least32_t;
// TCE:typedef int32_t int_fast32_t;
// TCE:typedef uint32_t uint_fast32_t;
//
// TCE:typedef short int16_t;
// TCE:typedef unsigned short uint16_t;
// TCE:typedef int16_t int_least16_t;
// TCE:typedef uint16_t uint_least16_t;
// TCE:typedef int16_t int_fast16_t;
// TCE:typedef uint16_t uint_fast16_t;
//
// TCE:typedef signed char int8_t;
// TCE:typedef unsigned char uint8_t;
// TCE:typedef int8_t int_least8_t;
// TCE:typedef uint8_t uint_least8_t;
// TCE:typedef int8_t int_fast8_t;
// TCE:typedef uint8_t uint_fast8_t;
//
// TCE:typedef int32_t intptr_t;
// TCE:typedef uint32_t uintptr_t;
//
// TCE:typedef long int intmax_t;
// TCE:typedef long unsigned int uintmax_t;
//
// TCE:INT8_MAX_ 127
// TCE:INT8_MIN_ (-127 -1)
// TCE:UINT8_MAX_ 255
// TCE:INT_LEAST8_MIN_ (-127 -1)
// TCE:INT_LEAST8_MAX_ 127
// TCE:UINT_LEAST8_MAX_ 255
// TCE:INT_FAST8_MIN_ (-127 -1)
// TCE:INT_FAST8_MAX_ 127
// TCE:UINT_FAST8_MAX_ 255
//
// TCE:INT16_MAX_ 32767
// TCE:INT16_MIN_ (-32767 -1)
// TCE:UINT16_MAX_ 65535
// TCE:INT_LEAST16_MIN_ (-32767 -1)
// TCE:INT_LEAST16_MAX_ 32767
// TCE:UINT_LEAST16_MAX_ 65535
// TCE:INT_FAST16_MIN_ (-32767 -1)
// TCE:INT_FAST16_MAX_ 32767
// TCE:UINT_FAST16_MAX_ 65535
//
// TCE:INT32_MAX_ 2147483647
// TCE:INT32_MIN_ (-2147483647 -1)
// TCE:UINT32_MAX_ 4294967295U
// TCE:INT_LEAST32_MIN_ (-2147483647 -1)
// TCE:INT_LEAST32_MAX_ 2147483647
// TCE:UINT_LEAST32_MAX_ 4294967295U
// TCE:INT_FAST32_MIN_ (-2147483647 -1)
// TCE:INT_FAST32_MAX_ 2147483647
// TCE:UINT_FAST32_MAX_ 4294967295U
//
// TCE:INT64_MAX_ INT64_MAX
// TCE:INT64_MIN_ INT64_MIN
// TCE:UINT64_MAX_ UINT64_MAX
// TCE:INT_LEAST64_MIN_ INT_LEAST64_MIN
// TCE:INT_LEAST64_MAX_ INT_LEAST64_MAX
// TCE:UINT_LEAST64_MAX_ UINT_LEAST64_MAX
// TCE:INT_FAST64_MIN_ INT_FAST64_MIN
// TCE:INT_FAST64_MAX_ INT_FAST64_MAX
// TCE:UINT_FAST64_MAX_ UINT_FAST64_MAX
//
// TCE:INTPTR_MIN_ (-2147483647 -1)
// TCE:INTPTR_MAX_ 2147483647
// TCE:UINTPTR_MAX_ 4294967295U
// TCE:PTRDIFF_MIN_ (-2147483647 -1)
// TCE:PTRDIFF_MAX_ 2147483647
// TCE:SIZE_MAX_ 4294967295U
//
// TCE:INTMAX_MIN_ (-2147483647 -1)
// TCE:INTMAX_MAX_ 2147483647
// TCE:UINTMAX_MAX_ 4294967295U
//
// TCE:SIG_ATOMIC_MIN_ (-2147483647 -1)
// TCE:SIG_ATOMIC_MAX_ 2147483647
// TCE:WINT_MIN_ (-2147483647 -1)
// TCE:WINT_MAX_ 2147483647
//
// TCE:WCHAR_MAX_ 2147483647
// TCE:WCHAR_MIN_ (-2147483647 -1)
//
// TCE:INT8_C_(0) 0
// TCE:UINT8_C_(0) 0U
// TCE:INT16_C_(0) 0
// TCE:UINT16_C_(0) 0U
// TCE:INT32_C_(0) 0
// TCE:UINT32_C_(0) 0U
// TCE:INT64_C_(0) INT64_C(0)
// TCE:UINT64_C_(0) UINT64_C(0)
//
// TCE:INTMAX_C_(0) 0
// TCE:UINTMAX_C_(0) 0U
//
// RUN: %clang_cc1 -E -ffreestanding -triple=x86_64-none-none %s | FileCheck -check-prefix X86_64 %s
//
//
// X86_64:typedef long int int64_t;
// X86_64:typedef long unsigned int uint64_t;
// X86_64:typedef int64_t int_least64_t;
// X86_64:typedef uint64_t uint_least64_t;
// X86_64:typedef int64_t int_fast64_t;
// X86_64:typedef uint64_t uint_fast64_t;
//
// X86_64:typedef int int32_t;
// X86_64:typedef unsigned int uint32_t;
// X86_64:typedef int32_t int_least32_t;
// X86_64:typedef uint32_t uint_least32_t;
// X86_64:typedef int32_t int_fast32_t;
// X86_64:typedef uint32_t uint_fast32_t;
//
// X86_64:typedef short int16_t;
// X86_64:typedef unsigned short uint16_t;
// X86_64:typedef int16_t int_least16_t;
// X86_64:typedef uint16_t uint_least16_t;
// X86_64:typedef int16_t int_fast16_t;
// X86_64:typedef uint16_t uint_fast16_t;
//
// X86_64:typedef signed char int8_t;
// X86_64:typedef unsigned char uint8_t;
// X86_64:typedef int8_t int_least8_t;
// X86_64:typedef uint8_t uint_least8_t;
// X86_64:typedef int8_t int_fast8_t;
// X86_64:typedef uint8_t uint_fast8_t;
//
// X86_64:typedef int64_t intptr_t;
// X86_64:typedef uint64_t uintptr_t;
//
// X86_64:typedef long int intmax_t;
// X86_64:typedef long unsigned int uintmax_t;
//
// X86_64:INT8_MAX_ 127
// X86_64:INT8_MIN_ (-127 -1)
// X86_64:UINT8_MAX_ 255
// X86_64:INT_LEAST8_MIN_ (-127 -1)
// X86_64:INT_LEAST8_MAX_ 127
// X86_64:UINT_LEAST8_MAX_ 255
// X86_64:INT_FAST8_MIN_ (-127 -1)
// X86_64:INT_FAST8_MAX_ 127
// X86_64:UINT_FAST8_MAX_ 255
//
// X86_64:INT16_MAX_ 32767
// X86_64:INT16_MIN_ (-32767 -1)
// X86_64:UINT16_MAX_ 65535
// X86_64:INT_LEAST16_MIN_ (-32767 -1)
// X86_64:INT_LEAST16_MAX_ 32767
// X86_64:UINT_LEAST16_MAX_ 65535
// X86_64:INT_FAST16_MIN_ (-32767 -1)
// X86_64:INT_FAST16_MAX_ 32767
// X86_64:UINT_FAST16_MAX_ 65535
//
// X86_64:INT32_MAX_ 2147483647
// X86_64:INT32_MIN_ (-2147483647 -1)
// X86_64:UINT32_MAX_ 4294967295U
// X86_64:INT_LEAST32_MIN_ (-2147483647 -1)
// X86_64:INT_LEAST32_MAX_ 2147483647
// X86_64:UINT_LEAST32_MAX_ 4294967295U
// X86_64:INT_FAST32_MIN_ (-2147483647 -1)
// X86_64:INT_FAST32_MAX_ 2147483647
// X86_64:UINT_FAST32_MAX_ 4294967295U
//
// X86_64:INT64_MAX_ 9223372036854775807L
// X86_64:INT64_MIN_ (-9223372036854775807L -1)
// X86_64:UINT64_MAX_ 18446744073709551615UL
// X86_64:INT_LEAST64_MIN_ (-9223372036854775807L -1)
// X86_64:INT_LEAST64_MAX_ 9223372036854775807L
// X86_64:UINT_LEAST64_MAX_ 18446744073709551615UL
// X86_64:INT_FAST64_MIN_ (-9223372036854775807L -1)
// X86_64:INT_FAST64_MAX_ 9223372036854775807L
// X86_64:UINT_FAST64_MAX_ 18446744073709551615UL
//
// X86_64:INTPTR_MIN_ (-9223372036854775807L -1)
// X86_64:INTPTR_MAX_ 9223372036854775807L
// X86_64:UINTPTR_MAX_ 18446744073709551615UL
// X86_64:PTRDIFF_MIN_ (-9223372036854775807L -1)
// X86_64:PTRDIFF_MAX_ 9223372036854775807L
// X86_64:SIZE_MAX_ 18446744073709551615UL
//
// X86_64:INTMAX_MIN_ (-9223372036854775807L -1)
// X86_64:INTMAX_MAX_ 9223372036854775807L
// X86_64:UINTMAX_MAX_ 18446744073709551615UL
//
// X86_64:SIG_ATOMIC_MIN_ (-2147483647 -1)
// X86_64:SIG_ATOMIC_MAX_ 2147483647
// X86_64:WINT_MIN_ (-2147483647 -1)
// X86_64:WINT_MAX_ 2147483647
//
// X86_64:WCHAR_MAX_ 2147483647
// X86_64:WCHAR_MIN_ (-2147483647 -1)
//
// X86_64:INT8_C_(0) 0
// X86_64:UINT8_C_(0) 0U
// X86_64:INT16_C_(0) 0
// X86_64:UINT16_C_(0) 0U
// X86_64:INT32_C_(0) 0
// X86_64:UINT32_C_(0) 0U
// X86_64:INT64_C_(0) 0L
// X86_64:UINT64_C_(0) 0UL
//
// X86_64:INTMAX_C_(0) 0L
// X86_64:UINTMAX_C_(0) 0UL
//
//
// RUN: %clang_cc1 -E -ffreestanding -triple=x86_64-pc-linux-gnu %s | FileCheck -check-prefix X86_64_LINUX %s
//
// X86_64_LINUX:WINT_MIN_ 0U
// X86_64_LINUX:WINT_MAX_ 4294967295U
//
//
// RUN: %clang_cc1 -E -ffreestanding -triple=i386-mingw32 %s | FileCheck -check-prefix I386_MINGW32 %s
//
// I386_MINGW32:WCHAR_MAX_ 65535
// I386_MINGW32:WCHAR_MIN_ 0
//
//
// RUN: %clang_cc1 -E -ffreestanding -triple=xcore-none-none %s | FileCheck -check-prefix XCORE %s
//
// XCORE:typedef long long int int64_t;
// XCORE:typedef long long unsigned int uint64_t;
// XCORE:typedef int64_t int_least64_t;
// XCORE:typedef uint64_t uint_least64_t;
// XCORE:typedef int64_t int_fast64_t;
// XCORE:typedef uint64_t uint_fast64_t;
//
// XCORE:typedef int int32_t;
// XCORE:typedef unsigned int uint32_t;
// XCORE:typedef int32_t int_least32_t;
// XCORE:typedef uint32_t uint_least32_t;
// XCORE:typedef int32_t int_fast32_t;
// XCORE:typedef uint32_t uint_fast32_t;
//
// XCORE:typedef short int16_t;
// XCORE:typedef unsigned short uint16_t;
// XCORE:typedef int16_t int_least16_t;
// XCORE:typedef uint16_t uint_least16_t;
// XCORE:typedef int16_t int_fast16_t;
// XCORE:typedef uint16_t uint_fast16_t;
//
// XCORE:typedef signed char int8_t;
// XCORE:typedef unsigned char uint8_t;
// XCORE:typedef int8_t int_least8_t;
// XCORE:typedef uint8_t uint_least8_t;
// XCORE:typedef int8_t int_fast8_t;
// XCORE:typedef uint8_t uint_fast8_t;
//
// XCORE:typedef int32_t intptr_t;
// XCORE:typedef uint32_t uintptr_t;
//
// XCORE:typedef long long int intmax_t;
// XCORE:typedef long long unsigned int uintmax_t;
//
// XCORE:INT8_MAX_ 127
// XCORE:INT8_MIN_ (-127 -1)
// XCORE:UINT8_MAX_ 255
// XCORE:INT_LEAST8_MIN_ (-127 -1)
// XCORE:INT_LEAST8_MAX_ 127
// XCORE:UINT_LEAST8_MAX_ 255
// XCORE:INT_FAST8_MIN_ (-127 -1)
// XCORE:INT_FAST8_MAX_ 127
// XCORE:UINT_FAST8_MAX_ 255
//
// XCORE:INT16_MAX_ 32767
// XCORE:INT16_MIN_ (-32767 -1)
// XCORE:UINT16_MAX_ 65535
// XCORE:INT_LEAST16_MIN_ (-32767 -1)
// XCORE:INT_LEAST16_MAX_ 32767
// XCORE:UINT_LEAST16_MAX_ 65535
// XCORE:INT_FAST16_MIN_ (-32767 -1)
// XCORE:INT_FAST16_MAX_ 32767
// XCORE:UINT_FAST16_MAX_ 65535
//
// XCORE:INT32_MAX_ 2147483647
// XCORE:INT32_MIN_ (-2147483647 -1)
// XCORE:UINT32_MAX_ 4294967295U
// XCORE:INT_LEAST32_MIN_ (-2147483647 -1)
// XCORE:INT_LEAST32_MAX_ 2147483647
// XCORE:UINT_LEAST32_MAX_ 4294967295U
// XCORE:INT_FAST32_MIN_ (-2147483647 -1)
// XCORE:INT_FAST32_MAX_ 2147483647
// XCORE:UINT_FAST32_MAX_ 4294967295U
//
// XCORE:INT64_MAX_ 9223372036854775807LL
// XCORE:INT64_MIN_ (-9223372036854775807LL -1)
// XCORE:UINT64_MAX_ 18446744073709551615ULL
// XCORE:INT_LEAST64_MIN_ (-9223372036854775807LL -1)
// XCORE:INT_LEAST64_MAX_ 9223372036854775807LL
// XCORE:UINT_LEAST64_MAX_ 18446744073709551615ULL
// XCORE:INT_FAST64_MIN_ (-9223372036854775807LL -1)
// XCORE:INT_FAST64_MAX_ 9223372036854775807LL
// XCORE:UINT_FAST64_MAX_ 18446744073709551615ULL
//
// XCORE:INTPTR_MIN_ (-2147483647 -1)
// XCORE:INTPTR_MAX_ 2147483647
// XCORE:UINTPTR_MAX_ 4294967295U
// XCORE:PTRDIFF_MIN_ (-2147483647 -1)
// XCORE:PTRDIFF_MAX_ 2147483647
// XCORE:SIZE_MAX_ 4294967295U
//
// XCORE:INTMAX_MIN_ (-9223372036854775807LL -1)
// XCORE:INTMAX_MAX_ 9223372036854775807LL
// XCORE:UINTMAX_MAX_ 18446744073709551615ULL
//
// XCORE:SIG_ATOMIC_MIN_ (-2147483647 -1)
// XCORE:SIG_ATOMIC_MAX_ 2147483647
// XCORE:WINT_MIN_ 0U
// XCORE:WINT_MAX_ 4294967295U
//
// XCORE:WCHAR_MAX_ 255
// XCORE:WCHAR_MIN_ 0
//
// XCORE:INT8_C_(0) 0
// XCORE:UINT8_C_(0) 0U
// XCORE:INT16_C_(0) 0
// XCORE:UINT16_C_(0) 0U
// XCORE:INT32_C_(0) 0
// XCORE:UINT32_C_(0) 0U
// XCORE:INT64_C_(0) 0LL
// XCORE:UINT64_C_(0) 0ULL
//
// XCORE:INTMAX_C_(0) 0LL
// XCORE:UINTMAX_C_(0) 0ULL
//
//
// stdint.h forms several macro definitions by pasting together identifiers
// to form names (eg. int32_t is formed from int ## 32 ## _t). The following 
// case tests that these joining operations are performed correctly even if
// the identifiers used in the operations (int, uint, _t, INT, UINT, _MIN,
// _MAX, and _C(v)) are themselves macros.
//
// RUN: %clang_cc1 -E -ffreestanding -U__UINTMAX_TYPE__ -U__INTMAX_TYPE__ -Dint=a -Duint=b -D_t=c -DINT=d -DUINT=e -D_MIN=f -D_MAX=g '-D_C(v)=h' -triple=i386-none-none %s | FileCheck -check-prefix JOIN %s
// JOIN:typedef int32_t intptr_t;
// JOIN:typedef uint32_t uintptr_t;
// JOIN:typedef __INTMAX_TYPE__ intmax_t;
// JOIN:typedef __UINTMAX_TYPE__ uintmax_t;
// JOIN:INTPTR_MIN_ (-2147483647 -1)
// JOIN:INTPTR_MAX_ 2147483647
// JOIN:UINTPTR_MAX_ 4294967295U
// JOIN:PTRDIFF_MIN_ (-2147483647 -1)
// JOIN:PTRDIFF_MAX_ 2147483647
// JOIN:SIZE_MAX_ 4294967295U
// JOIN:INTMAX_MIN_ (-9223372036854775807LL -1)
// JOIN:INTMAX_MAX_ 9223372036854775807LL
// JOIN:UINTMAX_MAX_ 18446744073709551615ULL
// JOIN:SIG_ATOMIC_MIN_ (-2147483647 -1)
// JOIN:SIG_ATOMIC_MAX_ 2147483647
// JOIN:WINT_MIN_ (-2147483647 -1)
// JOIN:WINT_MAX_ 2147483647
// JOIN:WCHAR_MAX_ 2147483647
// JOIN:WCHAR_MIN_ (-2147483647 -1)
// JOIN:INTMAX_C_(0) 0LL
// JOIN:UINTMAX_C_(0) 0ULL

#include <stdint.h>

INT8_MAX_ INT8_MAX
INT8_MIN_ INT8_MIN
UINT8_MAX_ UINT8_MAX
INT_LEAST8_MIN_ INT_LEAST8_MIN
INT_LEAST8_MAX_ INT_LEAST8_MAX
UINT_LEAST8_MAX_ UINT_LEAST8_MAX
INT_FAST8_MIN_ INT_FAST8_MIN
INT_FAST8_MAX_ INT_FAST8_MAX
UINT_FAST8_MAX_ UINT_FAST8_MAX

INT16_MAX_ INT16_MAX
INT16_MIN_ INT16_MIN
UINT16_MAX_ UINT16_MAX
INT_LEAST16_MIN_ INT_LEAST16_MIN
INT_LEAST16_MAX_ INT_LEAST16_MAX
UINT_LEAST16_MAX_ UINT_LEAST16_MAX
INT_FAST16_MIN_ INT_FAST16_MIN
INT_FAST16_MAX_ INT_FAST16_MAX
UINT_FAST16_MAX_ UINT_FAST16_MAX

INT32_MAX_ INT32_MAX
INT32_MIN_ INT32_MIN
UINT32_MAX_ UINT32_MAX
INT_LEAST32_MIN_ INT_LEAST32_MIN
INT_LEAST32_MAX_ INT_LEAST32_MAX
UINT_LEAST32_MAX_ UINT_LEAST32_MAX
INT_FAST32_MIN_ INT_FAST32_MIN
INT_FAST32_MAX_ INT_FAST32_MAX
UINT_FAST32_MAX_ UINT_FAST32_MAX

INT64_MAX_ INT64_MAX
INT64_MIN_ INT64_MIN
UINT64_MAX_ UINT64_MAX
INT_LEAST64_MIN_ INT_LEAST64_MIN
INT_LEAST64_MAX_ INT_LEAST64_MAX
UINT_LEAST64_MAX_ UINT_LEAST64_MAX
INT_FAST64_MIN_ INT_FAST64_MIN
INT_FAST64_MAX_ INT_FAST64_MAX
UINT_FAST64_MAX_ UINT_FAST64_MAX

INTPTR_MIN_ INTPTR_MIN
INTPTR_MAX_ INTPTR_MAX
UINTPTR_MAX_ UINTPTR_MAX
PTRDIFF_MIN_ PTRDIFF_MIN
PTRDIFF_MAX_ PTRDIFF_MAX
SIZE_MAX_ SIZE_MAX

INTMAX_MIN_ INTMAX_MIN
INTMAX_MAX_ INTMAX_MAX
UINTMAX_MAX_ UINTMAX_MAX

SIG_ATOMIC_MIN_ SIG_ATOMIC_MIN
SIG_ATOMIC_MAX_ SIG_ATOMIC_MAX
WINT_MIN_ WINT_MIN
WINT_MAX_ WINT_MAX

WCHAR_MAX_ WCHAR_MAX
WCHAR_MIN_ WCHAR_MIN

INT8_C_(0) INT8_C(0)
UINT8_C_(0) UINT8_C(0)
INT16_C_(0) INT16_C(0)
UINT16_C_(0) UINT16_C(0)
INT32_C_(0) INT32_C(0)
UINT32_C_(0) UINT32_C(0)
INT64_C_(0) INT64_C(0)
UINT64_C_(0) UINT64_C(0)

INTMAX_C_(0) INTMAX_C(0)
UINTMAX_C_(0) UINTMAX_C(0)
