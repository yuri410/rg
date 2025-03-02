// RUN: %clang_cc1 -fsyntax-only -verify %s
// RUN: cp %s %t
// RUN: not %clang_cc1 -x c++ -fixit %t -DFIXING
// RUN: %clang_cc1 -x c++ %t -DFIXING

template<typename T> void f(T) { }
template<typename T> void g(T) { }
template<typename T> struct x { };
template<typename T> struct y { };  // expected-note {{declared here}}

namespace good {
  template void f<int>(int);
  template void g(int);
  template struct x<int>;
}

namespace unsupported {
#ifndef FIXING
 template struct y;     // expected-error {{elaborated type refers to a template}}
#endif
}

template<typename T> void f0(T) { }
template<typename T> void g0(T) { }
template<typename T> struct x0 { }; // expected-note {{explicitly specialized declaration is here}}
template<typename T> struct y0 { };

// Should recover as if definition
namespace noargs_body {
#ifndef FIXING
  template void g0(int) { } // expected-error {{function cannot be defined in an explicit instantiation; if this declaration is meant to be a function definition, remove the 'template' keyword}}
#endif
  template struct y0 { };     // expected-error {{class cannot be defined in an explicit instantiation; if this declaration is meant to be a class definition, remove the 'template' keyword}}
}

// Explicit specializations expected in global scope
namespace exp_spec {
#ifndef FIXING
  template<> void f0<int>(int) { }  // expected-error {{no function template matches function template specialization 'f0'}}
  template<> struct x0<int> { };    // expected-error {{class template specialization of 'x0' must occur at global scope}}
#endif
}

template<typename T> void f1(T) { }
template<typename T> struct x1 { };  // expected-note {{explicitly specialized declaration is here}}

// Should recover as if specializations, 
// thus also complain about not being in global scope.
namespace args_bad {
#ifndef FIXING
  template void f1<int>(int) { }    // expected-error {{explicit template instantiation cannot have a definition; if this definition is meant to be an explicit specialization, add '<>' after the 'template' keyword}} \
                                       expected-error {{no function template matches function template specialization 'f1'}}
  template struct x1<int> { };      // expected-error {{explicit template instantiation cannot have a definition; if this definition is meant to be an explicit specialization, add '<>' after the 'template' keyword}} \
                                       expected-error {{class template specialization of 'x1' must occur at global scope}}
#endif
}

template<typename T> void f2(T) { }
template<typename T> struct x2 { };

// Should recover as if specializations
template void f2<int>(int) { }    // expected-error {{explicit template instantiation cannot have a definition; if this definition is meant to be an explicit specialization, add '<>' after the 'template' keyword}}
template struct x2<int> { };      // expected-error {{explicit template instantiation cannot have a definition; if this definition is meant to be an explicit specialization, add '<>' after the 'template' keyword}}
