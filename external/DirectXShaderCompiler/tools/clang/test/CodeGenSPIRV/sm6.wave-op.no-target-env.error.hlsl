// Run: %dxc -T cs_6_0 -E main

RWStructuredBuffer<uint> values;
[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    values[id.x] = WaveIsFirstLane();
}

// CHECK: 6:20: error: Vulkan 1.1 is required for Wave Operation but not permitted to use
// CHECK: note: please specify your target environment via command line option