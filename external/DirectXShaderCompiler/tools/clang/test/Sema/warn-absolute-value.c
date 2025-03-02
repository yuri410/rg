// RUN: %clang_cc1 -triple i686-pc-linux-gnu -fsyntax-only -verify %s -Wabsolute-value
// RUN: %clang_cc1 -triple i686-pc-linux-gnu -fsyntax-only %s -Wabsolute-value -fdiagnostics-parseable-fixits 2>&1 | FileCheck %s

int abs(int);
long int labs(long int);
long long int llabs(long long int);

float fabsf(float);
double fabs(double);
long double fabsl(long double);

float cabsf(float _Complex);
double cabs(double _Complex);
long double cabsl(long double _Complex);

void test_int(int x) {
  (void)abs(x);
  (void)labs(x);
  (void)llabs(x);

  (void)fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsf' when argument is of integer type}}
  // expected-note@-2 {{use function 'abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"abs"
  (void)fabs(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabs' when argument is of integer type}}
  // expected-note@-2 {{use function 'abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"abs"
  (void)fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsl' when argument is of integer type}}
  // expected-note@-2 {{use function 'abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"abs"

  (void)cabsf(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsf' when argument is of integer type}}
  // expected-note@-2 {{use function 'abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"abs"
  (void)cabs(x);
  // expected-warning@-1 {{using complex absolute value function 'cabs' when argument is of integer type}}
  // expected-note@-2 {{use function 'abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"abs"
  (void)cabsl(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsl' when argument is of integer type}}
  // expected-note@-2 {{use function 'abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"abs"

  (void)__builtin_abs(x);
  (void)__builtin_labs(x);
  (void)__builtin_llabs(x);

  (void)__builtin_fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsf' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_abs"
  (void)__builtin_fabs(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabs' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_abs"
  (void)__builtin_fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsl' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_abs"

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsf' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_abs"
  (void)__builtin_cabs(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabs' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_abs"
  (void)__builtin_cabsl(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsl' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_abs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_abs"
}

void test_long(long x) {
  (void)abs(x);  // no warning - int and long are same length for this target
  (void)labs(x);
  (void)llabs(x);

  (void)fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsf' when argument is of integer type}}
  // expected-note@-2 {{use function 'labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"labs"
  (void)fabs(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabs' when argument is of integer type}}
  // expected-note@-2 {{use function 'labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"labs"
  (void)fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsl' when argument is of integer type}}
  // expected-note@-2 {{use function 'labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"labs"

  (void)cabsf(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsf' when argument is of integer type}}
  // expected-note@-2 {{use function 'labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"labs"
  (void)cabs(x);
  // expected-warning@-1 {{using complex absolute value function 'cabs' when argument is of integer type}}
  // expected-note@-2 {{use function 'labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"labs"
  (void)cabsl(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsl' when argument is of integer type}}
  // expected-note@-2 {{use function 'labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"labs"

  (void)__builtin_abs(x);  // no warning - int and long are same length for
                           // this target
  (void)__builtin_labs(x);
  (void)__builtin_llabs(x);

  (void)__builtin_fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsf' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_labs"
  (void)__builtin_fabs(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabs' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_labs"
  (void)__builtin_fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsl' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_labs"

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsf' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_labs"
  (void)__builtin_cabs(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabs' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_labs"
  (void)__builtin_cabsl(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsl' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_labs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_labs"
}

void test_long_long(long long x) {
  (void)abs(x);
  // expected-warning@-1{{absolute value function 'abs' given an argument of type 'long long' but has parameter of type 'int' which may cause truncation of value}}
  // expected-note@-2{{use function 'llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:12}:"llabs"
  (void)labs(x);
  // expected-warning@-1{{absolute value function 'labs' given an argument of type 'long long' but has parameter of type 'long' which may cause truncation of value}}
  // expected-note@-2{{use function 'llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"llabs"
  (void)llabs(x);

  (void)fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsf' when argument is of integer type}}
  // expected-note@-2 {{use function 'llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"llabs"
  (void)fabs(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabs' when argument is of integer type}}
  // expected-note@-2 {{use function 'llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"llabs"
  (void)fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsl' when argument is of integer type}}
  // expected-note@-2 {{use function 'llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"llabs"

  (void)cabsf(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsf' when argument is of integer type}}
  // expected-note@-2 {{use function 'llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"llabs"
  (void)cabs(x);
  // expected-warning@-1 {{using complex absolute value function 'cabs' when argument is of integer type}}
  // expected-note@-2 {{use function 'llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"llabs"
  (void)cabsl(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsl' when argument is of integer type}}
  // expected-note@-2 {{use function 'llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"llabs"

  (void)__builtin_abs(x);
  // expected-warning@-1{{absolute value function '__builtin_abs' given an argument of type 'long long' but has parameter of type 'int' which may cause truncation of value}}
  // expected-note@-2{{use function '__builtin_llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:22}:"__builtin_llabs"
  (void)__builtin_labs(x);
  // expected-warning@-1{{absolute value function '__builtin_labs' given an argument of type 'long long' but has parameter of type 'long' which may cause truncation of value}}
  // expected-note@-2{{use function '__builtin_llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_llabs"
  (void)__builtin_llabs(x);

  (void)__builtin_fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsf' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_llabs"
  (void)__builtin_fabs(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabs' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_llabs"
  (void)__builtin_fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsl' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_llabs"

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsf' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_llabs"
  (void)__builtin_cabs(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabs' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_llabs"
  (void)__builtin_cabsl(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsl' when argument is of integer type}}
  // expected-note@-2 {{use function '__builtin_llabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_llabs"
}

void test_float(float x) {
  (void)abs(x);
  // expected-warning@-1 {{using integer absolute value function 'abs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:12}:"fabsf"
  (void)labs(x);
  // expected-warning@-1 {{using integer absolute value function 'labs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"fabsf"
  (void)llabs(x);
  // expected-warning@-1 {{using integer absolute value function 'llabs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabsf"

  (void)fabsf(x);
  (void)fabs(x);
  (void)fabsl(x);

  (void)cabsf(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsf' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabsf"
  (void)cabs(x);
  // expected-warning@-1 {{using complex absolute value function 'cabs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"fabsf"
  (void)cabsl(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsl' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabsf"

  (void)__builtin_abs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_abs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:22}:"__builtin_fabsf"
  (void)__builtin_labs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_labs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_fabsf"
  (void)__builtin_llabs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_llabs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabsf"

  (void)__builtin_fabsf(x);
  (void)__builtin_fabs(x);
  (void)__builtin_fabsl(x);

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsf' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabsf"
  (void)__builtin_cabs(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_fabsf"
  (void)__builtin_cabsl(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsl' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabsf"
}

void test_double(double x) {
  (void)abs(x);
  // expected-warning@-1 {{using integer absolute value function 'abs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:12}:"fabs"
  (void)labs(x);
  // expected-warning@-1 {{using integer absolute value function 'labs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"fabs"
  (void)llabs(x);
  // expected-warning@-1 {{using integer absolute value function 'llabs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabs"

  (void)fabsf(x);
  // expected-warning@-1{{absolute value function 'fabsf' given an argument of type 'double' but has parameter of type 'float' which may cause truncation of value}}
  // expected-note@-2{{use function 'fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabs"
  (void)fabs(x);
  (void)fabsl(x);

  (void)cabsf(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsf' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabs"
  (void)cabs(x);
  // expected-warning@-1 {{using complex absolute value function 'cabs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"fabs"
  (void)cabsl(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsl' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabs"

  (void)__builtin_abs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_abs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:22}:"__builtin_fabs"
  (void)__builtin_labs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_labs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_fabs"
  (void)__builtin_llabs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_llabs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabs"

  (void)__builtin_fabsf(x);
  // expected-warning@-1{{absolute value function '__builtin_fabsf' given an argument of type 'double' but has parameter of type 'float' which may cause truncation of value}}
  // expected-note@-2{{use function '__builtin_fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabs"
  (void)__builtin_fabs(x);
  (void)__builtin_fabsl(x);

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsf' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabs"
  (void)__builtin_cabs(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_fabs"
  (void)__builtin_cabsl(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsl' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabs"
}

void test_long_double(long double x) {
  (void)abs(x);
  // expected-warning@-1 {{using integer absolute value function 'abs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:12}:"fabsl"
  (void)labs(x);
  // expected-warning@-1 {{using integer absolute value function 'labs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"fabsl"
  (void)llabs(x);
  // expected-warning@-1 {{using integer absolute value function 'llabs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabsl"

  (void)fabsf(x);
  // expected-warning@-1{{absolute value function 'fabsf' given an argument of type 'long double' but has parameter of type 'float' which may cause truncation of value}}
  // expected-note@-2{{use function 'fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabsl"
  (void)fabs(x);
  // expected-warning@-1{{absolute value function 'fabs' given an argument of type 'long double' but has parameter of type 'double' which may cause truncation of value}}
  // expected-note@-2{{use function 'fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"fabsl"
  (void)fabsl(x);

  (void)cabsf(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsf' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabsl"
  (void)cabs(x);
  // expected-warning@-1 {{using complex absolute value function 'cabs' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"fabsl"
  (void)cabsl(x);
  // expected-warning@-1 {{using complex absolute value function 'cabsl' when argument is of floating point type}}
  // expected-note@-2 {{use function 'fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"fabsl"

  (void)__builtin_abs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_abs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:22}:"__builtin_fabsl"
  (void)__builtin_labs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_labs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_fabsl"
  (void)__builtin_llabs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_llabs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabsl"

  (void)__builtin_fabsf(x);
  // expected-warning@-1{{absolute value function '__builtin_fabsf' given an argument of type 'long double' but has parameter of type 'float' which may cause truncation of value}}
  // expected-note@-2{{use function '__builtin_fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabsl"
  (void)__builtin_fabs(x);
  // expected-warning@-1{{absolute value function '__builtin_fabs' given an argument of type 'long double' but has parameter of type 'double' which may cause truncation of value}}
  // expected-note@-2{{use function '__builtin_fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_fabsl"
  (void)__builtin_fabsl(x);

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsf' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabsl"
  (void)__builtin_cabs(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabs' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_fabsl"
  (void)__builtin_cabsl(x);
  // expected-warning@-1 {{using complex absolute value function '__builtin_cabsl' when argument is of floating point type}}
  // expected-note@-2 {{use function '__builtin_fabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_fabsl"
}

void test_complex_float(_Complex float x) {
  (void)abs(x);
  // expected-warning@-1 {{using integer absolute value function 'abs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:12}:"cabsf"
  (void)labs(x);
  // expected-warning@-1 {{using integer absolute value function 'labs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"cabsf"
  (void)llabs(x);
  // expected-warning@-1 {{using integer absolute value function 'llabs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabsf"

  (void)fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsf' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabsf"
  (void)fabs(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"cabsf"
  (void)fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsl' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabsf"

  (void)cabsf(x);
  (void)cabs(x);
  (void)cabsl(x);

  (void)__builtin_abs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_abs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:22}:"__builtin_cabsf"
  (void)__builtin_labs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_labs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_cabsf"
  (void)__builtin_llabs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_llabs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabsf"

  (void)__builtin_fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsf' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabsf"
  (void)__builtin_fabs(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_cabsf"
  (void)__builtin_fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsl' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsf' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabsf"

  (void)__builtin_cabsf(x);
  (void)__builtin_cabs(x);
  (void)__builtin_cabsl(x);
}

void test_complex_double(_Complex double x) {
  (void)abs(x);
  // expected-warning@-1 {{using integer absolute value function 'abs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:12}:"cabs"
  (void)labs(x);
  // expected-warning@-1 {{using integer absolute value function 'labs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"cabs"
  (void)llabs(x);
  // expected-warning@-1 {{using integer absolute value function 'llabs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabs"

  (void)fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsf' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabs"
  (void)fabs(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"cabs"
  (void)fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsl' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabs"

  (void)cabsf(x);
  // expected-warning@-1 {{absolute value function 'cabsf' given an argument of type '_Complex double' but has parameter of type '_Complex float' which may cause truncation of value}}
  // expected-note@-2 {{use function 'cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabs"
  (void)cabs(x);
  (void)cabsl(x);

  (void)__builtin_abs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_abs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:22}:"__builtin_cabs"
  (void)__builtin_labs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_labs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_cabs"
  (void)__builtin_llabs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_llabs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabs"

  (void)__builtin_fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsf' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabs"
  (void)__builtin_fabs(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_cabs"
  (void)__builtin_fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsl' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabs"

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{absolute value function '__builtin_cabsf' given an argument of type '_Complex double' but has parameter of type '_Complex float' which may cause truncation of value}}
  // expected-note@-2 {{use function '__builtin_cabs' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabs"
  (void)__builtin_cabs(x);
  (void)__builtin_cabsl(x);
}

void test_complex_long_double(_Complex long double x) {
  (void)abs(x);
  // expected-warning@-1 {{using integer absolute value function 'abs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:12}:"cabsl"
  (void)labs(x);
  // expected-warning@-1 {{using integer absolute value function 'labs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"cabsl"
  (void)llabs(x);
  // expected-warning@-1 {{using integer absolute value function 'llabs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabsl"

  (void)fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsf' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabsl"
  (void)fabs(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabs' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"cabsl"
  (void)fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function 'fabsl' when argument is of complex type}}
  // expected-note@-2 {{use function 'cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabsl"

  (void)cabsf(x);
  // expected-warning@-1 {{absolute value function 'cabsf' given an argument of type '_Complex long double' but has parameter of type '_Complex float' which may cause truncation of value}}
  // expected-note@-2 {{use function 'cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:"cabsl"
  (void)cabs(x);
  // expected-warning@-1 {{absolute value function 'cabs' given an argument of type '_Complex long double' but has parameter of type '_Complex double' which may cause truncation of value}}
  // expected-note@-2 {{use function 'cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:"cabsl"
  (void)cabsl(x);

  (void)__builtin_abs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_abs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:22}:"__builtin_cabsl"
  (void)__builtin_labs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_labs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_cabsl"
  (void)__builtin_llabs(x);
  // expected-warning@-1 {{using integer absolute value function '__builtin_llabs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabsl"

  (void)__builtin_fabsf(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsf' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabsl"
  (void)__builtin_fabs(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabs' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_cabsl"
  (void)__builtin_fabsl(x);
  // expected-warning@-1 {{using floating point absolute value function '__builtin_fabsl' when argument is of complex type}}
  // expected-note@-2 {{use function '__builtin_cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabsl"

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{absolute value function '__builtin_cabsf' given an argument of type '_Complex long double' but has parameter of type '_Complex float' which may cause truncation of value}}
  // expected-note@-2 {{use function '__builtin_cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:"__builtin_cabsl"
  (void)__builtin_cabs(x);
  // expected-warning@-1 {{absolute value function '__builtin_cabs' given an argument of type '_Complex long double' but has parameter of type '_Complex double' which may cause truncation of value}}
  // expected-note@-2 {{use function '__builtin_cabsl' instead}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:"__builtin_cabsl"
  (void)__builtin_cabsl(x);
}

void test_unsigned_int(unsigned int x) {
  (void)abs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to 'abs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:12}:""
  (void)labs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to 'labs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:""
  (void)llabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to 'llabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""

  (void)fabsf(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to 'fabsf' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""
  (void)fabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to 'fabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:""
  (void)fabsl(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to 'fabsl' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""

  (void)cabsf(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to 'cabsf' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""
  (void)cabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to 'cabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:""
  (void)cabsl(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to 'cabsl' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""

  (void)__builtin_abs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_abs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:22}:""
  (void)__builtin_labs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_labs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:""
  (void)__builtin_llabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_llabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""

  (void)__builtin_fabsf(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_fabsf' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""
  (void)__builtin_fabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_fabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:""
  (void)__builtin_fabsl(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_fabsl' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_cabsf' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""
  (void)__builtin_cabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_cabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:""
  (void)__builtin_cabsl(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned int' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_cabsl' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""
}

void test_unsigned_long(unsigned long x) {
  (void)abs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to 'abs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:12}:""
  (void)labs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to 'labs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:""
  (void)llabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to 'llabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""

  (void)fabsf(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to 'fabsf' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""
  (void)fabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to 'fabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:""
  (void)fabsl(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to 'fabsl' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""

  (void)cabsf(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to 'cabsf' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""
  (void)cabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to 'cabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:13}:""
  (void)cabsl(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to 'cabsl' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:14}:""

  (void)__builtin_abs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_abs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:22}:""
  (void)__builtin_labs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_labs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:""
  (void)__builtin_llabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_llabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""

  (void)__builtin_fabsf(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_fabsf' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""
  (void)__builtin_fabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_fabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:""
  (void)__builtin_fabsl(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_fabsl' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""

  (void)__builtin_cabsf(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_cabsf' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""
  (void)__builtin_cabs(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_cabs' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:23}:""
  (void)__builtin_cabsl(x);
  // expected-warning@-1 {{taking the absolute value of unsigned type 'unsigned long' has no effect}}
  // expected-note@-2 {{remove the call to '__builtin_cabsl' since unsigned values cannot be negative}}
  // CHECK: fix-it:"{{.*}}":{[[@LINE-3]]:9-[[@LINE-3]]:24}:""
}

