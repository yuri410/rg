// RUN: %clang_cc1 -fsyntax-only -verify -fblocks %s -Wno-error=non-pod-varargs

// Check that the warning is still there under -fms-compatibility.
// RUN: %clang_cc1 -fsyntax-only -verify -fblocks %s -Wno-error=non-pod-varargs -fms-compatibility

extern char version[];

class C {
public:
  C(int);
  void g(int a, ...);
  static void h(int a, ...);
};

void g(int a, ...);

void t1()
{
  C c(10);
  
  g(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic function; call will abort at runtime}}
  g(10, version);

  void (*ptr)(int, ...) = g;
  ptr(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic function; call will abort at runtime}}
  ptr(10, version);
}

void t2()
{
  C c(10);

  c.g(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic method; call will abort at runtime}}
  c.g(10, version);

  void (C::*ptr)(int, ...) = &C::g;
  (c.*ptr)(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic method; call will abort at runtime}}
  (c.*ptr)(10, version);
 
  C::h(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic function; call will abort at runtime}}
  C::h(10, version);

  void (*static_ptr)(int, ...) = &C::h; 
  static_ptr(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic function; call will abort at runtime}}
  static_ptr(10, version);
}

int (^block)(int, ...);

void t3()
{
  C c(10);
  
  block(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic block; call will abort at runtime}}
  block(10, version);
}

class D {
public:
  void operator() (int a, ...);
};

void t4()
{
  C c(10);

  D d;
  
  d(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic method; call will abort at runtime}}
  d(10, version);
}

class E {
  E(int, ...); // expected-note 2{{implicitly declared private here}}
};

void t5()
{
  C c(10);
  
  E e(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic constructor; call will abort at runtime}} \
    // expected-error{{calling a private constructor of class 'E'}}
  (void)E(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic constructor; call will abort at runtime}} \
    // expected-error{{calling a private constructor of class 'E'}}

}

// PR5761: unevaluated operands and the non-POD warning
class Foo {
 public:
  Foo() {}
};

int Helper(...);
const int size = sizeof(Helper(Foo()));

namespace std {
  class type_info { };
}

struct Base { virtual ~Base(); };
Base &get_base(...);
int eat_base(...);

void test_typeid(Base &base) {
  (void)typeid(get_base(base)); // expected-warning{{cannot pass object of non-POD type 'Base' through variadic function; call will abort at runtime}} expected-warning{{expression with side effects will be evaluated despite being used as an operand to 'typeid'}}
  (void)typeid(eat_base(base)); // okay
}


// rdar://7985267 - Shouldn't warn, doesn't actually use __builtin_va_start is
// magic.

void t6(Foo somearg, ... ) {
  __builtin_va_list list;
  __builtin_va_start(list, somearg);
}

void t7(int n, ...) {
  __builtin_va_list list;
  __builtin_va_start(list, n);
  (void)__builtin_va_arg(list, C); // expected-warning{{second argument to 'va_arg' is of non-POD type 'C'}}
  __builtin_va_end(list);
}

struct Abstract {
  virtual void doit() = 0; // expected-note{{unimplemented pure virtual method}}
};

void t8(int n, ...) {
  __builtin_va_list list;
  __builtin_va_start(list, n);
  (void)__builtin_va_arg(list, Abstract); // expected-error{{second argument to 'va_arg' is of abstract type 'Abstract'}}
  __builtin_va_end(list);
}

int t9(int n) {
  // Make sure the error works in potentially-evaluated sizeof
  return (int)sizeof(*(Helper(Foo()), (int (*)[n])0)); // expected-warning{{cannot pass object of non-POD type}}
}

// PR14057
namespace t10 {
  struct F {
    F();
  };

  struct S {
    void operator()(F, ...);
  };

  void foo() {
    S s;
    F f;
    s.operator()(f);
    s(f);
  }
}

namespace t11 {
  typedef void(*function_ptr)(int, ...);
  typedef void(C::*member_ptr)(int, ...);
  typedef void(^block_ptr)(int, ...);

  function_ptr get_f_ptr();
  member_ptr get_m_ptr();
  block_ptr get_b_ptr();

  function_ptr arr_f_ptr[5];
  member_ptr arr_m_ptr[5];
  block_ptr arr_b_ptr[5];

  void test() {
    C c(10);

    (get_f_ptr())(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic function; call will abort at runtime}}
    (get_f_ptr())(10, version);

    (c.*get_m_ptr())(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic method; call will abort at runtime}}
    (c.*get_m_ptr())(10, version);

    (get_b_ptr())(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic block; call will abort at runtime}}
    (get_b_ptr())(10, version);

    (arr_f_ptr[3])(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic function; call will abort at runtime}}
    (arr_f_ptr[3])(10, version);

    (c.*arr_m_ptr[3])(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic method; call will abort at runtime}}
    (c.*arr_m_ptr[3])(10, version);

    (arr_b_ptr[3])(10, c); // expected-warning{{cannot pass object of non-POD type 'C' through variadic block; call will abort at runtime}}
    (arr_b_ptr[3])(10, version);
  }
}
