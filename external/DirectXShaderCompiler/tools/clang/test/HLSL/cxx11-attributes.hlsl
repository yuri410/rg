// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s

[[vk::binding(4, 3)]] // expected-warning {{'binding' attribute ignored}}
cbuffer myCBuffer {
    float cbfield;
};

struct S {
    [[vk::location(1)]] // expected-warning {{'location' attribute ignored}}
    float a: A;
};

[[vk::binding(5, 3)]] // expected-warning {{'binding' attribute ignored}}
ConstantBuffer<S> myConstantBuffer;

[[maybe_unused]] // expected-warning {{unknown attribute 'maybe_unused' ignored}}
float main([[scope::attr(0, "str")]] // expected-warning {{unknown attribute 'attr' ignored}}
           float m: B,
           S s) : C {
    return m + s.a;
}
