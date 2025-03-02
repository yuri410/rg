// RUN: %clang_cc1 %s -verify -ffreestanding -fsyntax-only -triple=i686-linux-gnu -std=c11

// Basic parsing/Sema tests for __c11_atomic_*

#include <stdatomic.h>

struct S { char c[3]; };

_Static_assert(__GCC_ATOMIC_BOOL_LOCK_FREE == 2, "");
_Static_assert(__GCC_ATOMIC_CHAR_LOCK_FREE == 2, "");
_Static_assert(__GCC_ATOMIC_CHAR16_T_LOCK_FREE == 2, "");
_Static_assert(__GCC_ATOMIC_CHAR32_T_LOCK_FREE == 2, "");
_Static_assert(__GCC_ATOMIC_WCHAR_T_LOCK_FREE == 2, "");
_Static_assert(__GCC_ATOMIC_SHORT_LOCK_FREE == 2, "");
_Static_assert(__GCC_ATOMIC_INT_LOCK_FREE == 2, "");
_Static_assert(__GCC_ATOMIC_LONG_LOCK_FREE == 2, "");
#ifdef __i386__
_Static_assert(__GCC_ATOMIC_LLONG_LOCK_FREE == 1, "");
#else
_Static_assert(__GCC_ATOMIC_LLONG_LOCK_FREE == 2, "");
#endif
_Static_assert(__GCC_ATOMIC_POINTER_LOCK_FREE == 2, "");

_Static_assert(__c11_atomic_is_lock_free(1), "");
_Static_assert(__c11_atomic_is_lock_free(2), "");
_Static_assert(__c11_atomic_is_lock_free(3), ""); // expected-error {{not an integral constant expression}}
_Static_assert(__c11_atomic_is_lock_free(4), "");
_Static_assert(__c11_atomic_is_lock_free(8), "");
_Static_assert(__c11_atomic_is_lock_free(16), ""); // expected-error {{not an integral constant expression}}
_Static_assert(__c11_atomic_is_lock_free(17), ""); // expected-error {{not an integral constant expression}}

_Static_assert(__atomic_is_lock_free(1, 0), "");
_Static_assert(__atomic_is_lock_free(2, 0), "");
_Static_assert(__atomic_is_lock_free(3, 0), ""); // expected-error {{not an integral constant expression}}
_Static_assert(__atomic_is_lock_free(4, 0), "");
_Static_assert(__atomic_is_lock_free(8, 0), "");
_Static_assert(__atomic_is_lock_free(16, 0), ""); // expected-error {{not an integral constant expression}}
_Static_assert(__atomic_is_lock_free(17, 0), ""); // expected-error {{not an integral constant expression}}

_Static_assert(atomic_is_lock_free((atomic_char*)0), "");
_Static_assert(atomic_is_lock_free((atomic_short*)0), "");
_Static_assert(atomic_is_lock_free((atomic_int*)0), "");
_Static_assert(atomic_is_lock_free((atomic_long*)0), "");
// expected-error@+1 {{__int128 is not supported on this target}}
_Static_assert(atomic_is_lock_free((_Atomic(__int128)*)0), ""); // expected-error {{not an integral constant expression}}
_Static_assert(atomic_is_lock_free(0 + (atomic_char*)0), "");

char i8;
short i16;
int i32;
int __attribute__((vector_size(8))) i64;
struct Incomplete *incomplete; // expected-note {{forward declaration of 'struct Incomplete'}}

_Static_assert(__atomic_is_lock_free(1, &i8), "");
_Static_assert(__atomic_is_lock_free(1, &i64), "");
_Static_assert(__atomic_is_lock_free(2, &i8), ""); // expected-error {{not an integral constant expression}}
_Static_assert(__atomic_is_lock_free(2, &i16), "");
_Static_assert(__atomic_is_lock_free(2, &i64), "");
_Static_assert(__atomic_is_lock_free(4, &i16), ""); // expected-error {{not an integral constant expression}}
_Static_assert(__atomic_is_lock_free(4, &i32), "");
_Static_assert(__atomic_is_lock_free(4, &i64), "");
_Static_assert(__atomic_is_lock_free(8, &i32), ""); // expected-error {{not an integral constant expression}}
_Static_assert(__atomic_is_lock_free(8, &i64), "");

_Static_assert(__atomic_always_lock_free(1, 0), "");
_Static_assert(__atomic_always_lock_free(2, 0), "");
_Static_assert(!__atomic_always_lock_free(3, 0), "");
_Static_assert(__atomic_always_lock_free(4, 0), "");
_Static_assert(__atomic_always_lock_free(8, 0), "");
_Static_assert(!__atomic_always_lock_free(16, 0), "");
_Static_assert(!__atomic_always_lock_free(17, 0), "");

_Static_assert(__atomic_always_lock_free(1, incomplete), "");
_Static_assert(!__atomic_always_lock_free(2, incomplete), "");
_Static_assert(!__atomic_always_lock_free(4, incomplete), "");

_Static_assert(__atomic_always_lock_free(1, &i8), "");
_Static_assert(__atomic_always_lock_free(1, &i64), "");
_Static_assert(!__atomic_always_lock_free(2, &i8), "");
_Static_assert(__atomic_always_lock_free(2, &i16), "");
_Static_assert(__atomic_always_lock_free(2, &i64), "");
_Static_assert(!__atomic_always_lock_free(4, &i16), "");
_Static_assert(__atomic_always_lock_free(4, &i32), "");
_Static_assert(__atomic_always_lock_free(4, &i64), "");
_Static_assert(!__atomic_always_lock_free(8, &i32), "");
_Static_assert(__atomic_always_lock_free(8, &i64), "");

void f(_Atomic(int) *i, _Atomic(int*) *p, _Atomic(float) *d,
       int *I, int **P, float *D, struct S *s1, struct S *s2) {
  __c11_atomic_init(I, 5); // expected-error {{pointer to _Atomic}}
  __c11_atomic_load(0); // expected-error {{too few arguments to function}}
  __c11_atomic_load(0,0,0); // expected-error {{too many arguments to function}}
  __c11_atomic_store(0,0,0); // expected-error {{address argument to atomic builtin must be a pointer}}
  __c11_atomic_store((int*)0,0,0); // expected-error {{address argument to atomic operation must be a pointer to _Atomic}}

  __c11_atomic_load(i, memory_order_seq_cst);
  __c11_atomic_load(p, memory_order_seq_cst);
  __c11_atomic_load(d, memory_order_seq_cst);

  int load_n_1 = __atomic_load_n(I, memory_order_relaxed);
  int *load_n_2 = __atomic_load_n(P, memory_order_relaxed);
  float load_n_3 = __atomic_load_n(D, memory_order_relaxed); // expected-error {{must be a pointer to integer or pointer}}
  __atomic_load_n(s1, memory_order_relaxed); // expected-error {{must be a pointer to integer or pointer}}

  __atomic_load(i, I, memory_order_relaxed); // expected-error {{must be a pointer to a trivially-copyable type}}
  __atomic_load(I, i, memory_order_relaxed); // expected-warning {{passing '_Atomic(int) *' to parameter of type 'int *'}}
  __atomic_load(I, *P, memory_order_relaxed);
  __atomic_load(I, *P, memory_order_relaxed, 42); // expected-error {{too many arguments}}
  (int)__atomic_load(I, I, memory_order_seq_cst); // expected-error {{operand of type 'void'}}
  __atomic_load(s1, s2, memory_order_acquire);

  __c11_atomic_store(i, 1, memory_order_seq_cst);
  __c11_atomic_store(p, 1, memory_order_seq_cst); // expected-warning {{incompatible integer to pointer conversion}}
  (int)__c11_atomic_store(d, 1, memory_order_seq_cst); // expected-error {{operand of type 'void'}}

  __atomic_store_n(I, 4, memory_order_release);
  __atomic_store_n(I, 4.0, memory_order_release);
  __atomic_store_n(I, P, memory_order_release); // expected-warning {{parameter of type 'int'}}
  __atomic_store_n(i, 1, memory_order_release); // expected-error {{must be a pointer to integer or pointer}}
  __atomic_store_n(s1, *s2, memory_order_release); // expected-error {{must be a pointer to integer or pointer}}

  __atomic_store(I, *P, memory_order_release);
  __atomic_store(s1, s2, memory_order_release);
  __atomic_store(i, I, memory_order_release); // expected-error {{trivially-copyable}}

  int exchange_1 = __c11_atomic_exchange(i, 1, memory_order_seq_cst);
  int exchange_2 = __c11_atomic_exchange(I, 1, memory_order_seq_cst); // expected-error {{must be a pointer to _Atomic}}
  int exchange_3 = __atomic_exchange_n(i, 1, memory_order_seq_cst); // expected-error {{must be a pointer to integer or pointer}}
  int exchange_4 = __atomic_exchange_n(I, 1, memory_order_seq_cst);

  __atomic_exchange(s1, s2, s2, memory_order_seq_cst);
  __atomic_exchange(s1, I, P, memory_order_seq_cst); // expected-warning 2{{parameter of type 'struct S *'}}
  (int)__atomic_exchange(s1, s2, s2, memory_order_seq_cst); // expected-error {{operand of type 'void'}}

  __c11_atomic_fetch_add(i, 1, memory_order_seq_cst);
  __c11_atomic_fetch_add(p, 1, memory_order_seq_cst);
  __c11_atomic_fetch_add(d, 1, memory_order_seq_cst); // expected-error {{must be a pointer to atomic integer or pointer}}

  __atomic_fetch_add(i, 3, memory_order_seq_cst); // expected-error {{pointer to integer or pointer}}
  __atomic_fetch_sub(I, 3, memory_order_seq_cst);
  __atomic_fetch_sub(P, 3, memory_order_seq_cst);
  __atomic_fetch_sub(D, 3, memory_order_seq_cst); // expected-error {{must be a pointer to integer or pointer}}
  __atomic_fetch_sub(s1, 3, memory_order_seq_cst); // expected-error {{must be a pointer to integer or pointer}}

  __c11_atomic_fetch_and(i, 1, memory_order_seq_cst);
  __c11_atomic_fetch_and(p, 1, memory_order_seq_cst); // expected-error {{must be a pointer to atomic integer}}
  __c11_atomic_fetch_and(d, 1, memory_order_seq_cst); // expected-error {{must be a pointer to atomic integer}}

  __atomic_fetch_and(i, 3, memory_order_seq_cst); // expected-error {{pointer to integer}}
  __atomic_fetch_or(I, 3, memory_order_seq_cst);
  __atomic_fetch_xor(P, 3, memory_order_seq_cst); // expected-error {{must be a pointer to integer}}
  __atomic_fetch_or(D, 3, memory_order_seq_cst); // expected-error {{must be a pointer to integer}}
  __atomic_fetch_and(s1, 3, memory_order_seq_cst); // expected-error {{must be a pointer to integer}}

  _Bool cmpexch_1 = __c11_atomic_compare_exchange_strong(i, 0, 1, memory_order_seq_cst, memory_order_seq_cst);
  _Bool cmpexch_2 = __c11_atomic_compare_exchange_strong(p, 0, (int*)1, memory_order_seq_cst, memory_order_seq_cst);
  _Bool cmpexch_3 = __c11_atomic_compare_exchange_strong(d, (int*)0, 1, memory_order_seq_cst, memory_order_seq_cst); // expected-warning {{incompatible pointer types}}

  _Bool cmpexch_4 = __atomic_compare_exchange_n(I, I, 5, 1, memory_order_seq_cst, memory_order_seq_cst);
  _Bool cmpexch_5 = __atomic_compare_exchange_n(I, P, 5, 0, memory_order_seq_cst, memory_order_seq_cst); // expected-warning {{; dereference with *}}
  _Bool cmpexch_6 = __atomic_compare_exchange_n(I, I, P, 0, memory_order_seq_cst, memory_order_seq_cst); // expected-warning {{passing 'int **' to parameter of type 'int'}}

  _Bool cmpexch_7 = __atomic_compare_exchange(I, I, 5, 1, memory_order_seq_cst, memory_order_seq_cst); // expected-warning {{passing 'int' to parameter of type 'int *'}}
  _Bool cmpexch_8 = __atomic_compare_exchange(I, P, I, 0, memory_order_seq_cst, memory_order_seq_cst); // expected-warning {{; dereference with *}}
  _Bool cmpexch_9 = __atomic_compare_exchange(I, I, I, 0, memory_order_seq_cst, memory_order_seq_cst);

  const volatile int flag_k = 0;
  volatile int flag = 0;
  (void)(int)__atomic_test_and_set(&flag_k, memory_order_seq_cst); // expected-warning {{passing 'const volatile int *' to parameter of type 'volatile void *'}}
  (void)(int)__atomic_test_and_set(&flag, memory_order_seq_cst);
  __atomic_clear(&flag_k, memory_order_seq_cst); // expected-warning {{passing 'const volatile int *' to parameter of type 'volatile void *'}}
  __atomic_clear(&flag, memory_order_seq_cst);
  (int)__atomic_clear(&flag, memory_order_seq_cst); // expected-error {{operand of type 'void'}}

  const _Atomic(int) const_atomic;
  __c11_atomic_init(&const_atomic, 0); // expected-error {{address argument to atomic operation must be a pointer to non-const _Atomic type ('const _Atomic(int) *' invalid)}}
  __c11_atomic_store(&const_atomic, 0, memory_order_release); // expected-error {{address argument to atomic operation must be a pointer to non-const _Atomic type ('const _Atomic(int) *' invalid)}}
  __c11_atomic_load(&const_atomic, memory_order_acquire); // expected-error {{address argument to atomic operation must be a pointer to non-const _Atomic type ('const _Atomic(int) *' invalid)}}

  // Ensure the <stdatomic.h> macros behave appropriately.
  atomic_int n = ATOMIC_VAR_INIT(123);
  atomic_init(&n, 456);
  atomic_init(&n, (void*)0); // expected-warning {{passing 'void *' to parameter of type 'int'}}

  const atomic_wchar_t cawt;
  atomic_init(&cawt, L'x'); // expected-error {{non-const}}
  atomic_wchar_t awt;
  atomic_init(&awt, L'x');

  int x = kill_dependency(12);

  atomic_thread_fence(); // expected-error {{too few arguments to function call}}
  atomic_thread_fence(memory_order_seq_cst);
  atomic_signal_fence(memory_order_seq_cst);
  void (*pfn)(memory_order) = &atomic_thread_fence;
  pfn = &atomic_signal_fence;

  int k = atomic_load_explicit(&n, memory_order_relaxed);
  atomic_store_explicit(&n, k, memory_order_relaxed);
  atomic_store(&n, atomic_load(&n));

  k = atomic_exchange(&n, 72);
  k = atomic_exchange_explicit(&n, k, memory_order_release);

  atomic_compare_exchange_strong(&n, k, k); // expected-warning {{take the address with &}}
  atomic_compare_exchange_weak(&n, &k, k);
  atomic_compare_exchange_strong_explicit(&n, &k, k, memory_order_seq_cst); // expected-error {{too few arguments}}
  atomic_compare_exchange_weak_explicit(&n, &k, k, memory_order_seq_cst, memory_order_acquire);

  atomic_fetch_add(&k, n); // expected-error {{must be a pointer to _Atomic}}
  k = atomic_fetch_add(&n, k);
  k = atomic_fetch_sub(&n, k);
  k = atomic_fetch_and(&n, k);
  k = atomic_fetch_or(&n, k);
  k = atomic_fetch_xor(&n, k);
  k = atomic_fetch_add_explicit(&n, k, memory_order_acquire);
  k = atomic_fetch_sub_explicit(&n, k, memory_order_release);
  k = atomic_fetch_and_explicit(&n, k, memory_order_acq_rel);
  k = atomic_fetch_or_explicit(&n, k, memory_order_consume);
  k = atomic_fetch_xor_explicit(&n, k, memory_order_relaxed);

  // C11 7.17.1/4: atomic_flag is a structure type.
  struct atomic_flag must_be_struct = ATOMIC_FLAG_INIT;
  // C11 7.17.8/5 implies that it is also a typedef type.
  atomic_flag guard = ATOMIC_FLAG_INIT;
  _Bool old_val = atomic_flag_test_and_set(&guard);
  if (old_val) atomic_flag_clear(&guard);

  old_val = (atomic_flag_test_and_set)(&guard);
  if (old_val) (atomic_flag_clear)(&guard);

  const atomic_flag const_guard;
  atomic_flag_test_and_set(&const_guard); // expected-error {{address argument to atomic operation must be a pointer to non-const _Atomic type ('const atomic_bool *' (aka 'const _Atomic(_Bool) *') invalid)}}
  atomic_flag_clear(&const_guard); // expected-error {{address argument to atomic operation must be a pointer to non-const _Atomic type ('const atomic_bool *' (aka 'const _Atomic(_Bool) *') invalid)}}
}

_Atomic(int*) PR12527_a;
void PR12527() { int *b = PR12527_a; }

void PR16931(int* x) { // expected-note {{passing argument to parameter 'x' here}}
  typedef struct { _Atomic(_Bool) flag; } flag;
  flag flagvar = { 0 };
  PR16931(&flagvar); // expected-warning {{incompatible pointer types}}
}

void memory_checks(_Atomic(int) *Ap, int *p, int val) {
  (void)__c11_atomic_load(Ap, memory_order_relaxed);
  (void)__c11_atomic_load(Ap, memory_order_acquire);
  (void)__c11_atomic_load(Ap, memory_order_consume);
  (void)__c11_atomic_load(Ap, memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__c11_atomic_load(Ap, memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__c11_atomic_load(Ap, memory_order_seq_cst);
  (void)__c11_atomic_load(Ap, val);
  (void)__c11_atomic_load(Ap, -1); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__c11_atomic_load(Ap, 42); // expected-warning {{memory order argument to atomic operation is invalid}}

  (void)__c11_atomic_store(Ap, val, memory_order_relaxed);
  (void)__c11_atomic_store(Ap, val, memory_order_acquire); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__c11_atomic_store(Ap, val, memory_order_consume); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__c11_atomic_store(Ap, val, memory_order_release);
  (void)__c11_atomic_store(Ap, val, memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__c11_atomic_store(Ap, val, memory_order_seq_cst);

  (void)__c11_atomic_fetch_add(Ap, 1, memory_order_relaxed);
  (void)__c11_atomic_fetch_add(Ap, 1, memory_order_acquire);
  (void)__c11_atomic_fetch_add(Ap, 1, memory_order_consume);
  (void)__c11_atomic_fetch_add(Ap, 1, memory_order_release);
  (void)__c11_atomic_fetch_add(Ap, 1, memory_order_acq_rel);
  (void)__c11_atomic_fetch_add(Ap, 1, memory_order_seq_cst);

  (void)__c11_atomic_fetch_add(
      (struct Incomplete * _Atomic *)0, // expected-error {{incomplete type 'struct Incomplete'}}
      1, memory_order_seq_cst);

  (void)__c11_atomic_init(Ap, val);
  (void)__c11_atomic_init(Ap, val);
  (void)__c11_atomic_init(Ap, val);
  (void)__c11_atomic_init(Ap, val);
  (void)__c11_atomic_init(Ap, val);
  (void)__c11_atomic_init(Ap, val);

  (void)__c11_atomic_fetch_sub(Ap, val, memory_order_relaxed);
  (void)__c11_atomic_fetch_sub(Ap, val, memory_order_acquire);
  (void)__c11_atomic_fetch_sub(Ap, val, memory_order_consume);
  (void)__c11_atomic_fetch_sub(Ap, val, memory_order_release);
  (void)__c11_atomic_fetch_sub(Ap, val, memory_order_acq_rel);
  (void)__c11_atomic_fetch_sub(Ap, val, memory_order_seq_cst);

  (void)__c11_atomic_fetch_and(Ap, val, memory_order_relaxed);
  (void)__c11_atomic_fetch_and(Ap, val, memory_order_acquire);
  (void)__c11_atomic_fetch_and(Ap, val, memory_order_consume);
  (void)__c11_atomic_fetch_and(Ap, val, memory_order_release);
  (void)__c11_atomic_fetch_and(Ap, val, memory_order_acq_rel);
  (void)__c11_atomic_fetch_and(Ap, val, memory_order_seq_cst);

  (void)__c11_atomic_fetch_or(Ap, val, memory_order_relaxed);
  (void)__c11_atomic_fetch_or(Ap, val, memory_order_acquire);
  (void)__c11_atomic_fetch_or(Ap, val, memory_order_consume);
  (void)__c11_atomic_fetch_or(Ap, val, memory_order_release);
  (void)__c11_atomic_fetch_or(Ap, val, memory_order_acq_rel);
  (void)__c11_atomic_fetch_or(Ap, val, memory_order_seq_cst);

  (void)__c11_atomic_fetch_xor(Ap, val, memory_order_relaxed);
  (void)__c11_atomic_fetch_xor(Ap, val, memory_order_acquire);
  (void)__c11_atomic_fetch_xor(Ap, val, memory_order_consume);
  (void)__c11_atomic_fetch_xor(Ap, val, memory_order_release);
  (void)__c11_atomic_fetch_xor(Ap, val, memory_order_acq_rel);
  (void)__c11_atomic_fetch_xor(Ap, val, memory_order_seq_cst);

  (void)__c11_atomic_exchange(Ap, val, memory_order_relaxed);
  (void)__c11_atomic_exchange(Ap, val, memory_order_acquire);
  (void)__c11_atomic_exchange(Ap, val, memory_order_consume);
  (void)__c11_atomic_exchange(Ap, val, memory_order_release);
  (void)__c11_atomic_exchange(Ap, val, memory_order_acq_rel);
  (void)__c11_atomic_exchange(Ap, val, memory_order_seq_cst);

  (void)__c11_atomic_compare_exchange_strong(Ap, p, val, memory_order_relaxed, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_strong(Ap, p, val, memory_order_acquire, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_strong(Ap, p, val, memory_order_consume, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_strong(Ap, p, val, memory_order_release, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_strong(Ap, p, val, memory_order_acq_rel, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_strong(Ap, p, val, memory_order_seq_cst, memory_order_relaxed);

  (void)__c11_atomic_compare_exchange_weak(Ap, p, val, memory_order_relaxed, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_weak(Ap, p, val, memory_order_acquire, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_weak(Ap, p, val, memory_order_consume, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_weak(Ap, p, val, memory_order_release, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_weak(Ap, p, val, memory_order_acq_rel, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_weak(Ap, p, val, memory_order_seq_cst, memory_order_relaxed);

  (void)__atomic_load_n(p, memory_order_relaxed);
  (void)__atomic_load_n(p, memory_order_acquire);
  (void)__atomic_load_n(p, memory_order_consume);
  (void)__atomic_load_n(p, memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_load_n(p, memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_load_n(p, memory_order_seq_cst);

  (void)__atomic_load(p, p, memory_order_relaxed);
  (void)__atomic_load(p, p, memory_order_acquire);
  (void)__atomic_load(p, p, memory_order_consume);
  (void)__atomic_load(p, p, memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_load(p, p, memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_load(p, p, memory_order_seq_cst);

  (void)__atomic_store(p, p, memory_order_relaxed);
  (void)__atomic_store(p, p, memory_order_acquire); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_store(p, p, memory_order_consume); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_store(p, p, memory_order_release);
  (void)__atomic_store(p, p, memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_store(p, p, memory_order_seq_cst);

  (void)__atomic_store_n(p, val, memory_order_relaxed);
  (void)__atomic_store_n(p, val, memory_order_acquire); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_store_n(p, val, memory_order_consume); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_store_n(p, val, memory_order_release);
  (void)__atomic_store_n(p, val, memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
  (void)__atomic_store_n(p, val, memory_order_seq_cst);

  (void)__atomic_fetch_add(p, val, memory_order_relaxed);
  (void)__atomic_fetch_add(p, val, memory_order_acquire);
  (void)__atomic_fetch_add(p, val, memory_order_consume);
  (void)__atomic_fetch_add(p, val, memory_order_release);
  (void)__atomic_fetch_add(p, val, memory_order_acq_rel);
  (void)__atomic_fetch_add(p, val, memory_order_seq_cst);

  (void)__atomic_fetch_sub(p, val, memory_order_relaxed);
  (void)__atomic_fetch_sub(p, val, memory_order_acquire);
  (void)__atomic_fetch_sub(p, val, memory_order_consume);
  (void)__atomic_fetch_sub(p, val, memory_order_release);
  (void)__atomic_fetch_sub(p, val, memory_order_acq_rel);
  (void)__atomic_fetch_sub(p, val, memory_order_seq_cst);

  (void)__atomic_add_fetch(p, val, memory_order_relaxed);
  (void)__atomic_add_fetch(p, val, memory_order_acquire);
  (void)__atomic_add_fetch(p, val, memory_order_consume);
  (void)__atomic_add_fetch(p, val, memory_order_release);
  (void)__atomic_add_fetch(p, val, memory_order_acq_rel);
  (void)__atomic_add_fetch(p, val, memory_order_seq_cst);

  (void)__atomic_sub_fetch(p, val, memory_order_relaxed);
  (void)__atomic_sub_fetch(p, val, memory_order_acquire);
  (void)__atomic_sub_fetch(p, val, memory_order_consume);
  (void)__atomic_sub_fetch(p, val, memory_order_release);
  (void)__atomic_sub_fetch(p, val, memory_order_acq_rel);
  (void)__atomic_sub_fetch(p, val, memory_order_seq_cst);

  (void)__atomic_fetch_and(p, val, memory_order_relaxed);
  (void)__atomic_fetch_and(p, val, memory_order_acquire);
  (void)__atomic_fetch_and(p, val, memory_order_consume);
  (void)__atomic_fetch_and(p, val, memory_order_release);
  (void)__atomic_fetch_and(p, val, memory_order_acq_rel);
  (void)__atomic_fetch_and(p, val, memory_order_seq_cst);

  (void)__atomic_fetch_or(p, val, memory_order_relaxed);
  (void)__atomic_fetch_or(p, val, memory_order_acquire);
  (void)__atomic_fetch_or(p, val, memory_order_consume);
  (void)__atomic_fetch_or(p, val, memory_order_release);
  (void)__atomic_fetch_or(p, val, memory_order_acq_rel);
  (void)__atomic_fetch_or(p, val, memory_order_seq_cst);

  (void)__atomic_fetch_xor(p, val, memory_order_relaxed);
  (void)__atomic_fetch_xor(p, val, memory_order_acquire);
  (void)__atomic_fetch_xor(p, val, memory_order_consume);
  (void)__atomic_fetch_xor(p, val, memory_order_release);
  (void)__atomic_fetch_xor(p, val, memory_order_acq_rel);
  (void)__atomic_fetch_xor(p, val, memory_order_seq_cst);

  (void)__atomic_fetch_nand(p, val, memory_order_relaxed);
  (void)__atomic_fetch_nand(p, val, memory_order_acquire);
  (void)__atomic_fetch_nand(p, val, memory_order_consume);
  (void)__atomic_fetch_nand(p, val, memory_order_release);
  (void)__atomic_fetch_nand(p, val, memory_order_acq_rel);
  (void)__atomic_fetch_nand(p, val, memory_order_seq_cst);

  (void)__atomic_and_fetch(p, val, memory_order_relaxed);
  (void)__atomic_and_fetch(p, val, memory_order_acquire);
  (void)__atomic_and_fetch(p, val, memory_order_consume);
  (void)__atomic_and_fetch(p, val, memory_order_release);
  (void)__atomic_and_fetch(p, val, memory_order_acq_rel);
  (void)__atomic_and_fetch(p, val, memory_order_seq_cst);

  (void)__atomic_or_fetch(p, val, memory_order_relaxed);
  (void)__atomic_or_fetch(p, val, memory_order_acquire);
  (void)__atomic_or_fetch(p, val, memory_order_consume);
  (void)__atomic_or_fetch(p, val, memory_order_release);
  (void)__atomic_or_fetch(p, val, memory_order_acq_rel);
  (void)__atomic_or_fetch(p, val, memory_order_seq_cst);

  (void)__atomic_xor_fetch(p, val, memory_order_relaxed);
  (void)__atomic_xor_fetch(p, val, memory_order_acquire);
  (void)__atomic_xor_fetch(p, val, memory_order_consume);
  (void)__atomic_xor_fetch(p, val, memory_order_release);
  (void)__atomic_xor_fetch(p, val, memory_order_acq_rel);
  (void)__atomic_xor_fetch(p, val, memory_order_seq_cst);

  (void)__atomic_nand_fetch(p, val, memory_order_relaxed);
  (void)__atomic_nand_fetch(p, val, memory_order_acquire);
  (void)__atomic_nand_fetch(p, val, memory_order_consume);
  (void)__atomic_nand_fetch(p, val, memory_order_release);
  (void)__atomic_nand_fetch(p, val, memory_order_acq_rel);
  (void)__atomic_nand_fetch(p, val, memory_order_seq_cst);

  (void)__atomic_exchange_n(p, val, memory_order_relaxed);
  (void)__atomic_exchange_n(p, val, memory_order_acquire);
  (void)__atomic_exchange_n(p, val, memory_order_consume);
  (void)__atomic_exchange_n(p, val, memory_order_release);
  (void)__atomic_exchange_n(p, val, memory_order_acq_rel);
  (void)__atomic_exchange_n(p, val, memory_order_seq_cst);

  (void)__atomic_exchange(p, p, p, memory_order_relaxed);
  (void)__atomic_exchange(p, p, p, memory_order_acquire);
  (void)__atomic_exchange(p, p, p, memory_order_consume);
  (void)__atomic_exchange(p, p, p, memory_order_release);
  (void)__atomic_exchange(p, p, p, memory_order_acq_rel);
  (void)__atomic_exchange(p, p, p, memory_order_seq_cst);

  (void)__atomic_compare_exchange(p, p, p, 0, memory_order_relaxed, memory_order_relaxed);
  (void)__atomic_compare_exchange(p, p, p, 0, memory_order_acquire, memory_order_relaxed);
  (void)__atomic_compare_exchange(p, p, p, 0, memory_order_consume, memory_order_relaxed);
  (void)__atomic_compare_exchange(p, p, p, 0, memory_order_release, memory_order_relaxed);
  (void)__atomic_compare_exchange(p, p, p, 0, memory_order_acq_rel, memory_order_relaxed);
  (void)__atomic_compare_exchange(p, p, p, 0, memory_order_seq_cst, memory_order_relaxed);

  (void)__atomic_compare_exchange_n(p, p, val, 0, memory_order_relaxed, memory_order_relaxed);
  (void)__atomic_compare_exchange_n(p, p, val, 0, memory_order_acquire, memory_order_relaxed);
  (void)__atomic_compare_exchange_n(p, p, val, 0, memory_order_consume, memory_order_relaxed);
  (void)__atomic_compare_exchange_n(p, p, val, 0, memory_order_release, memory_order_relaxed);
  (void)__atomic_compare_exchange_n(p, p, val, 0, memory_order_acq_rel, memory_order_relaxed);
  (void)__atomic_compare_exchange_n(p, p, val, 0, memory_order_seq_cst, memory_order_relaxed);
}


