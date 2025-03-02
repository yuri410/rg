// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} i32 -2

[RootSignature("")]
uint main() : SV_Target {
    uint ux = 5;
    uint uy = 25;
    uint uz = -1;
    
    uint64_t lx = 7ULL;
    uint64_t ly = 25ULL;
    uint64_t lz = -1ULL;

    return max(max(ux, uy), uz) + max(max(lx, ly), lz);
}
