// RUN: %clang_cc1 -fsyntax-only -verify -Wc++11-compat %s

int& a();

void f() {
  decltype(a()) c; // expected-warning {{'decltype' is a keyword in C++11}} expected-error {{use of undeclared identifier 'decltype'}}
}
