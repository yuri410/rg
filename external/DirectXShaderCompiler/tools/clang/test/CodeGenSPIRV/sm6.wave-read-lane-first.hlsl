// Run: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1

// CHECK: ; Version: 1.3

struct S {
    uint4 val1;
     int2 val2;
    float val3;
};

RWStructuredBuffer<S> values;

// CHECK: OpCapability GroupNonUniformBallot

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    uint x = id.x;

    uint4 val1 = values[x].val1;
     int2 val2 = values[x].val2;
    float val3 = values[x].val3;

// CHECK:      [[val1:%\d+]] = OpLoad %v4uint %val1
// CHECK-NEXT:      {{%\d+}} = OpGroupNonUniformBroadcastFirst %v4uint %uint_3 [[val1]]
    values[x].val1 = WaveReadLaneFirst(val1);
// CHECK:      [[val2:%\d+]] = OpLoad %v2int %val2
// CHECK-NEXT:      {{%\d+}} = OpGroupNonUniformBroadcastFirst %v2int %uint_3 [[val2]]
    values[x].val2 = WaveReadLaneFirst(val2);
// CHECK:      [[val3:%\d+]] = OpLoad %float %val3
// CHECK-NEXT:      {{%\d+}} = OpGroupNonUniformBroadcastFirst %float %uint_3 [[val3]]
    values[x].val3 = WaveReadLaneFirst(val3);
}
