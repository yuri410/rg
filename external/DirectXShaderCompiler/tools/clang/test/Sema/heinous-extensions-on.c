// RUN: %clang_cc1 %s -verify -fheinous-gnu-extensions

void foo() {
  int a;
  // PR3788
  asm("nop" : : "m"((int)(a))); // expected-warning {{cast in an inline asm context requiring an l-value}}
  // PR3794
  asm("nop" : "=r"((unsigned)a)); // expected-warning {{cast in an inline asm context requiring an l-value}}
}
