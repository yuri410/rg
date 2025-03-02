// RUN: %clang -target x86_64-apple-darwin10 --relocatable-pch -o %t \
// RUN:   -isysroot %S/libroot %S/libroot/usr/include/reloc.h
// RUN: %clang -target x86_64-apple-darwin10 -fsyntax-only \
// RUN:   -include-pch %t -isysroot %S/libroot %s -Xclang -verify
// RUN: not %clang -target x86_64-apple-darwin10 -include-pch %t %s
// REQUIRES: x86-registered-target

#include <reloc.h>

int x = 2; // expected-error{{redefinition}}
int y = 5; // expected-error{{redefinition}}


// expected-note@libroot/usr/include/reloc.h:13{{previous definition}}
// expected-note@libroot/usr/include/reloc2.h:14{{previous definition}}
