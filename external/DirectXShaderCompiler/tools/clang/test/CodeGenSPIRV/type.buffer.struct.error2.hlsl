// Run: %dxc -T ps_6_0 -E main

struct S {
    float2 a;
    int1   b;
};

Buffer<S> MyBuffer;

float4 main(): SV_Target {
    return MyBuffer[0].a.x;
}

// CHECK: :3:8: error: all struct members should have the same element type for resource template instantiation
