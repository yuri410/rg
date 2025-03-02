// Run: %dxc -T ps_6_0 -E main

struct B {
    float2 b;
};

struct S {
    float2 a;
    B      b;
};

Buffer<S> MyBuffer;

float4 main(): SV_Target {
    return MyBuffer[0].a.x;
}

// CHECK: :7:8: error: unsupported struct element type for resource template instantiation
