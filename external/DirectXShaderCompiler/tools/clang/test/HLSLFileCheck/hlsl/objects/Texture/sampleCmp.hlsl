// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 -DTYPE=float4 %s                            | FileCheck %s -check-prefix=CHECK
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_2 -DTYPE=half4       -enable-16bit-types %s   | FileCheck %s -check-prefix=CHECK
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_2 -DTYPE=min16float4 -enable-16bit-types %s   | FileCheck %s -check-prefix=CHECK

SamplerComparisonState samp1 : register(s5);
Texture2D<TYPE> text1 : register(t3);
half cmpVal;
float main(float2 a
          : A) : SV_Target {
  uint status;
  float r = 0;
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  r += text1.SampleCmp(samp1, a, cmpVal);
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  r += text1.SampleCmp(samp1, a, cmpVal, uint2(-5, 7));
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  r += text1.SampleCmp(samp1, a, cmpVal, uint2(-5, 7), a.x);
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmp.f32(i32 64,
  r += text1.SampleCmp(samp1, a, cmpVal, uint2(-3, 2), a.y, status);
  r += status;
  return r;
}
