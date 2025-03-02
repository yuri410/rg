// RUN: %dxc -Emain -Tcs_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation,parameter0=10,parameter1=20,parameter2=30 | %FileCheck %s

// Check that the CS thread IDs are added properly

// CHECK: %PIX_DebugUAV_Handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0, i1 false)
// CHECK: %ThreadIdX = call i32 @dx.op.threadId.i32(i32 93, i32 0)
// CHECK: %ThreadIdY = call i32 @dx.op.threadId.i32(i32 93, i32 1)
// CHECK: %ThreadIdZ = call i32 @dx.op.threadId.i32(i32 93, i32 2)
// CHECK: %CompareToThreadIdX = icmp eq i32 %ThreadIdX, 10
// CHECK: %CompareToThreadIdY = icmp eq i32 %ThreadIdY, 20
// CHECK: %CompareToThreadIdZ = icmp eq i32 %ThreadIdZ, 30
// CHECK: %CompareXAndY = and i1 %CompareToThreadIdX, %CompareToThreadIdY
// CHECK: %CompareAll = and i1 %CompareXAndY, %CompareToThreadIdZ

[RootSignature("")]
[numthreads(4, 4, 4)]
void main() {
}
