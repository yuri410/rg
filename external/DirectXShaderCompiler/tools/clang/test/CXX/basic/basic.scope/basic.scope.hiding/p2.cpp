// RUN: %clang_cc1 -fsyntax-only -verify %s

// rdar4641403
namespace N {
  struct X { // expected-note{{candidate found by name lookup}}
    float b;
  };
}

using namespace N;

typedef struct {
  int a;
} X; // expected-note{{candidate found by name lookup}}


struct Y { };
void Y(int) { }

void f() {
  X *x; // expected-error{{reference to 'X' is ambiguous}}
  Y(1); // okay
}

namespace PR17731 {
  void f() {
    struct S { S() {} };
    int S(void);
    int a = S();
    struct S b;
    {
      int S(void);
      int a = S();
      struct S c = b;
    }
    {
      struct S { S() {} }; // expected-note {{candidate}}
      int a = S(); // expected-error {{no viable conversion from 'S'}}
      struct S c = b; // expected-error {{no viable conversion from 'struct S'}}
    }
  }
  void g() {
    int S(void);
    struct S { S() {} };
    int a = S();
    struct S b;
    {
      int S(void);
      int a = S();
      struct S c = b;
    }
    {
      struct S { S() {} }; // expected-note {{candidate}}
      int a = S(); // expected-error {{no viable conversion from 'S'}}
      struct S c = b; // expected-error {{no viable conversion from 'struct S'}}
    }
  }

  struct A {
    struct B;
    void f();
    int B;
  };
  struct A::B {};
  void A::f() {
    B = 123;
    struct B b;
  }
}
