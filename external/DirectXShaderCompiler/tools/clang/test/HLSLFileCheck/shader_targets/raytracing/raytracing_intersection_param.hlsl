// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: error: return type for ray tracing shaders must be void
// CHECK: error: parameters are not allowed for intersection shader

[shader("intersection")]
float intersection_param(float4 extra)
{
  return extra.x;
}
