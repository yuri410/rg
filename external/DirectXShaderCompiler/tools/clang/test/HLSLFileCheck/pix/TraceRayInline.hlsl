// RUN: %dxc -T vs_6_5 -E main %s | %opt -S -hlsl-dxil-pix-shader-access-instrumentation,config=S0:1:1i1;U0:2:10i0;.. | FileCheck %s

// CHECK: call void @dx.op.rayQuery_TraceRayInline
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle

RaytracingAccelerationStructure RTAS;

void DoTrace(RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery, RayDesc rayDesc) {
  rayQuery.TraceRayInline(RTAS, 0, 1, rayDesc);
}

float main(RayDesc rayDesc : RAYDESC) : OUT {
  RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery;
  DoTrace(rayQuery, rayDesc);
  rayQuery.TraceRayInline(RTAS, 1, 2, rayDesc);
  return 0;
}
