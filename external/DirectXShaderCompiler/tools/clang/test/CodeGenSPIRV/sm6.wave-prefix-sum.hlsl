// Run: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1

// CHECK: ; Version: 1.3

struct S {
     int4 val1;
    uint2 val2;
    float val3;
};

RWStructuredBuffer<S> values;

// CHECK: OpCapability GroupNonUniformArithmetic

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    uint x = id.x;
     int4 val1 = values[x].val1;
    uint2 val2 = values[x].val2;
    float val3 = values[x].val3;

// CHECK:      [[val1:%\d+]] = OpLoad %v4int %val1
// CHECK-NEXT:      {{%\d+}} = OpGroupNonUniformIAdd %v4int %uint_3 ExclusiveScan [[val1]]
    values[x].val1 = WavePrefixSum(val1);
// CHECK:      [[val2:%\d+]] = OpLoad %v2uint %val2
// CHECK-NEXT:      {{%\d+}} = OpGroupNonUniformIAdd %v2uint %uint_3 ExclusiveScan [[val2]]
    values[x].val2 = WavePrefixSum(val2);
// CHECK:      [[val3:%\d+]] = OpLoad %float %val3
// CHECK-NEXT:      {{%\d+}} = OpGroupNonUniformFAdd %float %uint_3 ExclusiveScan [[val3]]
    values[x].val3 = WavePrefixSum(val3);
}
