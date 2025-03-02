// RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s
// RUN: %clang_cc1 -std=c++11 -fsyntax-only -verify -pedantic %s
float f = 0x1p+1; // expected-warning{{hexadecimal floating constants are a C99 feature}}
double e = 0x.p0; //expected-error{{hexadecimal floating constants require a significand}}
double d = 0x.2p2; // expected-warning{{hexadecimal floating constants are a C99 feature}}
float g = 0x1.2p2; // expected-warning{{hexadecimal floating constants are a C99 feature}}
double h = 0x1.p2; // expected-warning{{hexadecimal floating constants are a C99 feature}}

// PR12717: In order to minimally diverge from the C++ standard, we do not lex
// 'p[+-]' as part of a pp-number unless the token starts 0x and doesn't contain
// an underscore.
double i = 0p+3; // expected-error{{invalid suffix 'p' on integer constant}}
#define PREFIX(x) foo ## x
double foo0p = 1, j = PREFIX(0p+3); // ok
double k = 0x42_amp+3; // expected-error-re{{{{invalid suffix '_amp' on integer constant|no matching literal operator for call to 'operator "" _amp'}}}}
