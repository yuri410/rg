// RUN: %clang_cc1 -fno-rtti -emit-llvm-only -triple i686-pc-win32 -fdump-record-layouts -fsyntax-only -mms-bitfields -verify %s 2>&1

struct A {
  char a : 9; // expected-error{{size of bit-field 'a' (9 bits) exceeds size of its type (8 bits)}}
  int b : 33; // expected-error{{size of bit-field 'b' (33 bits) exceeds size of its type (32 bits)}}
  bool c : 9; // expected-error{{size of bit-field 'c' (9 bits) exceeds size of its type (8 bits)}}
};

int a[sizeof(A) == 1 ? 1 : -1];
