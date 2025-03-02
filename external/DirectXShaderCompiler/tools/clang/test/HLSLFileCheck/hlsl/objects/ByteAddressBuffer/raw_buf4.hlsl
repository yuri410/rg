// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -HV 2018 %s | FileCheck %s

// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf1_texture_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 15, i32 4)

// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 1, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 3, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 7, i32 4)
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %buf2_UAV_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 15, i32 4)

// CHECK-NOT: call %dx.types.ResRet.f16

// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf1_texture_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 15, i32 8)
// CHECK: call double @dx.op.makeDouble.f64

// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 3, i32 8)
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf2_UAV_rawbuf, i32 %{{[0-9]+}}, i32 undef, i8 15, i32 8)
// CHECK: call double @dx.op.makeDouble.f64

// CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, float %{{.*}}, float undef, float undef, float undef, i8 1, i32 4)
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, i32 %{{.*}}, i32 %{{.*}}, i32 undef, i32 undef, i8 3, i32 4)
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i32 undef, i8 7, i32 4)
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i8 15, i32 4)
// CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, float %{{.*}}, float undef, float undef, float undef, i8 1, i32 4)
// CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, float %{{.*}}, float %{{.*}}, float undef, float undef, i8 3, i32 4)
// CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float undef, i8 7, i32 4)
// CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, i8 15, i32 4)
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, i32 %{{.*}}, i32 %{{.*}}, i32 undef, i32 undef, i8 3, i32 8)
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %buf2_UAV_rawbuf, i32 1, i32 undef, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i32 %{{.*}}, i8 15, i32 8)

ByteAddressBuffer buf1;
RWByteAddressBuffer buf2;

float4 main(uint idx1 : IDX1, uint idx2 : IDX2) : SV_Target {
  uint status;
  float4 r = float4(0,0,0,0);

  r.x += buf1.Load<float>(idx1, status);
  r.xy += buf1.Load<float2>(idx1);
  r.xyz += buf1.Load<float3>(idx1, status);
  r.xyzw += buf1.Load<float4>(idx1);

  r.x += buf2.Load<float>(idx2);
  r.xy += buf2.Load<float2>(idx2, status);
  r.xyz += buf2.Load<float3>(idx2);
  r.xyzw += buf2.Load<float4>(idx2, status);

  r.x += buf1.Load<double>(idx1);
  r.xy += buf1.Load<double2>(idx1, status);

  r.x += buf2.Load<double>(idx2, status);
  r.xy += buf2.Load<double2>(idx2);

  buf2.Store(1, r.x);
  buf2.Store2(1, r.xy);
  buf2.Store3(1, r.xyz);
  buf2.Store4(1, r);

  buf2.Store(1, r.x);
  buf2.Store(1, r.xy);
  buf2.Store(1, r.xyz);
  buf2.Store(1, r);

  buf2.Store(1, (double)r.x);
  buf2.Store(1, (double2)r.xy);

  return r;
}