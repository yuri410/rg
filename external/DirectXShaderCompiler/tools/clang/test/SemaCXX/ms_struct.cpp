// RUN: %clang_cc1 -fsyntax-only -Wno-error=incompatible-ms-struct -verify -triple i686-apple-darwin9 -std=c++11 %s
// RUN: %clang_cc1 -fsyntax-only -Wno-error=incompatible-ms-struct -verify -triple armv7-apple-darwin9 -std=c++11 %s
// RUN: %clang_cc1 -fsyntax-only -DTEST_FOR_ERROR -verify -triple armv7-apple-darwin9 -std=c++11 %s

#pragma ms_struct on

struct A {
  unsigned long a:4;
  unsigned char b;
};

struct B : public A {
#ifdef TEST_FOR_ERROR
  // expected-error@-2 {{ms_struct may not produce MSVC-compatible layouts for classes with base classes or virtual functions}}
#else
  // expected-warning@-4 {{ms_struct may not produce MSVC-compatible layouts for classes with base classes or virtual functions}}
#endif
  unsigned long c:16;
	int d;
  B();
};

static_assert(__builtin_offsetof(B, d) == 12,
  "We can't allocate the bitfield into the padding under ms_struct");

// rdar://16178895
struct C {
#ifdef TEST_FOR_ERROR
  // expected-error@-2 {{ms_struct may not produce MSVC-compatible layouts for classes with base classes or virtual functions}}
#else
  // expected-warning@-4 {{ms_struct may not produce MSVC-compatible layouts for classes with base classes or virtual functions}}
#endif
  virtual void foo();
  long long n;
};

static_assert(__builtin_offsetof(C, n) == 8,
              "long long field in ms_struct should be 8-byte aligned");
