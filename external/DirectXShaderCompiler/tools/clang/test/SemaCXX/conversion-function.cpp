// RUN: %clang_cc1 -fsyntax-only -Wbind-to-temporary-copy -verify %s 
class X { 
public:
  operator bool();
  operator int() const;

  bool f() {
    return operator bool();
  }

  float g() {
    return operator float(); // expected-error{{use of undeclared 'operator float'}}
  }

  static operator short(); // expected-error{{conversion function must be a non-static member function}}
};

operator int(); // expected-error{{conversion function must be a non-static member function}}

operator int; // expected-error{{'operator int' cannot be the name of a variable or data member}}

typedef int func_type(int);
typedef int array_type[10];

class Y {
public:
  void operator bool(int, ...) const; // expected-error{{conversion function cannot have a return type}} \
  // expected-error{{conversion function cannot have any parameters}}

  operator bool(int a = 4, int b = 6) const; // expected-error{{conversion function cannot have any parameters}}
  
  
  operator float(...) const;  // expected-error{{conversion function cannot be variadic}}
  
  
  operator func_type(); // expected-error{{conversion function cannot convert to a function type}}
  operator array_type(); // expected-error{{conversion function cannot convert to an array type}}
};


typedef int INT;
typedef INT* INT_PTR;

class Z { 
  operator int(); // expected-note {{previous declaration is here}}
  operator int**(); // expected-note {{previous declaration is here}}
  
  operator INT();  // expected-error{{conversion function cannot be redeclared}}
  operator INT_PTR*(); // expected-error{{conversion function cannot be redeclared}}
};


class A { };

class B : public A {
public:
  operator A&() const; // expected-warning{{conversion function converting 'B' to its base class 'A' will never be used}}
  operator const void() const; // expected-warning{{conversion function converting 'B' to 'const void' will never be used}}
  operator const B(); // expected-warning{{conversion function converting 'B' to itself will never be used}}
};

// This used to crash Clang.
struct Flip;
struct Flop {
  Flop();
  Flop(const Flip&); // expected-note{{candidate constructor}}
};
struct Flip {
  operator Flop() const; // expected-note{{candidate function}}
};
Flop flop = Flip(); // expected-error {{conversion from 'Flip' to 'Flop' is ambiguous}}

// This tests that we don't add the second conversion declaration to the list of user conversions
struct C {
  operator const char *() const;
};

C::operator const char*() const { return 0; }

void f(const C& c) {
  const char* v = c;
}

// Test. Conversion in base class is visible in derived class.
class XB { 
public:
  operator int(); // expected-note {{candidate function}}
};

class Yb : public XB { 
public:
  operator char(); // expected-note {{candidate function}}
};

void f(Yb& a) {
  if (a) { } // expected-error {{conversion from 'Yb' to 'bool' is ambiguous}}
  int i = a; // OK. calls XB::operator int();
  char ch = a;  // OK. calls Yb::operator char();
}

// Test conversion + copy construction.
class AutoPtrRef { };

class AutoPtr {
  AutoPtr(AutoPtr &); // expected-note{{declared private here}}
  
public:
  AutoPtr();
  AutoPtr(AutoPtrRef);
  
  operator AutoPtrRef();
};

AutoPtr make_auto_ptr();

AutoPtr test_auto_ptr(bool Cond) {
  AutoPtr p1( make_auto_ptr() );
  
  AutoPtr p;
  if (Cond)
    return p; // expected-error{{calling a private constructor}}
  
  return AutoPtr();
}

struct A1 {
  A1(const char *);
  ~A1();

private:
  A1(const A1&); // expected-note 2 {{declared private here}}
};

A1 f() {
  // FIXME: redundant diagnostics!
  return "Hello"; // expected-error {{calling a private constructor}} expected-warning {{an accessible copy constructor}}
}

namespace source_locations {
  template<typename T>
  struct sneaky_int {
    typedef int type;
  };

  template<typename T, typename U>
  struct A { };

  template<typename T>
  struct A<T, T> : A<T, int> { };

  struct E {
    template<typename T>
    operator A<T, typename sneaky_int<T>::type>&() const; // expected-note{{candidate function}}
  };

  void f() {
    A<float, float> &af = E(); // expected-error{{no viable conversion}}
    A<float, int> &af2 = E();
    const A<float, int> &caf2 = E();
  }

  // Check 
  template<typename T>
  struct E2 {
    operator T
    * // expected-error{{pointer to a reference}}
    () const;
  };

  E2<int&> e2i; // expected-note{{in instantiation}}
}

namespace crazy_declarators {
  struct A {
    (&operator bool())(); // expected-error {{use a typedef to declare a conversion to 'bool (&)()'}}
    *operator int();  // expected-error {{put the complete type after 'operator'}}
    // No suggestion of using a typedef here; that's not possible.
    template<typename T> (&operator T())(); // expected-error-re {{cannot specify any part of a return type in the declaration of a conversion function{{$}}}}
  };
}

namespace smart_ptr {
  class Y { 
    class YRef { };

    Y(Y&);

  public:
    Y();
    Y(YRef);

    operator YRef(); // expected-note{{candidate function}}
  };

  struct X { // expected-note{{candidate constructor (the implicit copy constructor) not}}
    explicit X(Y);
  };

  Y make_Y();

  X f() {
    X x = make_Y(); // expected-error{{no viable conversion from 'smart_ptr::Y' to 'smart_ptr::X'}}
    X x2(make_Y());
    return X(Y());
  }
}

struct Any {
  Any(...);
};

struct Other {
  Other(const Other &); 
  Other();
};

void test_any() {
  Any any = Other(); // expected-error{{cannot pass object of non-POD type 'Other' through variadic constructor; call will abort at runtime}}
}

namespace PR7055 {
  // Make sure that we don't allow too many conversions in an
  // auto_ptr-like template. In particular, we can't create multiple
  // temporary objects when binding to a reference.
  struct auto_ptr {
    struct auto_ptr_ref { };

    auto_ptr(auto_ptr&);
    auto_ptr(auto_ptr_ref);
    explicit auto_ptr(int *);

    operator auto_ptr_ref();
  };

  struct X {
    X(auto_ptr);
  };

  X f() {
    X x(auto_ptr(new int));
    return X(auto_ptr(new int));
  }

  auto_ptr foo();

  X e(foo());

  struct Y {
    Y(X);
  };
  
  Y f2(foo());
}

namespace PR7934 {
  typedef unsigned char uint8;

  struct MutablePtr {
    MutablePtr() : ptr(0) {}
    void *ptr;

    operator void*() { return ptr; }

  private:
    operator uint8*() { return reinterpret_cast<uint8*>(ptr); }
    operator const char*() const { return reinterpret_cast<const char*>(ptr); }
  };

  void fake_memcpy(const void *);

  void use() {
    MutablePtr ptr;
    fake_memcpy(ptr);
  }
}

namespace rdar8018274 {
  struct X { };
  struct Y {
    operator const struct X *() const;
  };

  struct Z : Y {
    operator struct X * ();
  };

  void test() {
    Z x;
    (void) (x != __null);
  }


  struct Base {
    operator int();
  };

  struct Derived1 : Base { };

  struct Derived2 : Base { };

  struct SuperDerived : Derived1, Derived2 { 
    using Derived1::operator int;
  };

  struct UeberDerived : SuperDerived {
    operator long();
  };

  void test2(UeberDerived ud) {
    int i = ud; // expected-error{{ambiguous conversion from derived class 'rdar8018274::SuperDerived' to base class 'rdar8018274::Base'}}
  }

  struct Base2 {
    operator int();
  };

  struct Base3 {
    operator int();
  };

  struct Derived23 : Base2, Base3 { 
    using Base2::operator int;
  };

  struct ExtraDerived23 : Derived23 { };

  void test3(ExtraDerived23 ed) {
    int i = ed;
  }
}

namespace PR8065 {
  template <typename T> struct Iterator;
  template <typename T> struct Container;

  template<>
  struct Iterator<int> {
    typedef Container<int> container_type;
  };

  template <typename T>
  struct Container {
    typedef typename Iterator<T>::container_type X;
    operator X(void) { return X(); }
  };

  Container<int> test;
}

namespace PR8034 {
  struct C {
    operator int();

  private:
    template <typename T> operator T();
  };
  int x = C().operator int();
}

namespace PR9336 {
  template<class T>
  struct generic_list
  {
    template<class Container>
    operator Container()
    { 
      Container ar;
      T* i;
      ar[0]=*i;
      return ar;
    }
  };

  template<class T>
  struct array
  {
    T& operator[](int);
    const T& operator[](int)const;
  };

  generic_list<generic_list<int> > l;
  array<array<int> > a = l;
}

namespace PR8800 {
  struct A;
  struct C {
    operator A&();
  };
  void f() {
    C c;
    A& a1(c);
    A& a2 = c;
    A& a3 = static_cast<A&>(c);
    A& a4 = (A&)c;
  }
}

namespace PR12712 {
  struct A {};
  struct B {
    operator A();
    operator A() const;
  };
  struct C : B {};

  A f(const C c) { return c; }
}

namespace PR18234 {
  struct A {
    operator enum E { e } (); // expected-error {{'PR18234::A::E' cannot be defined in a type specifier}}
    operator struct S { int n; } (); // expected-error {{'PR18234::A::S' cannot be defined in a type specifier}}
  } a;
  A::S s = a;
  A::E e = a; // expected-note {{here}}
  bool k1 = e == A::e; // expected-error {{no member named 'e'}}
  bool k2 = e.n == 0;
}
