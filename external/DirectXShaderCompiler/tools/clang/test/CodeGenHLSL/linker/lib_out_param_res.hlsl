// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// resources in return/params disallowed for lib_6_3
// CHECK: error: Exported function
// CHECK: GetBuf
// CHECK: must not contain a resource in parameter or return type

Buffer<float4> GetBuf();

[shader("pixel")]
float4 test(uint i:I) : SV_Target {
  Buffer<float4> buf = GetBuf();
  return buf[i];
}