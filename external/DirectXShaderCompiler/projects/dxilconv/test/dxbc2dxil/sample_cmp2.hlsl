// RUN: %fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




SamplerComparisonState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);

float4 main(float2 a : A) : SV_Target
{
  uint status;
  uint cmp = a.y + a.x;
  float4 r = 0;
  r += text1.SampleCmpLevelZero(samp1, a, cmp);
  r += text1.SampleCmpLevelZero(samp1, a, cmp, uint2(-5, 7));
  r += text1.SampleCmpLevelZero(samp1, a, cmp, uint2(-4, 1));
  r += text1.SampleCmpLevelZero(samp1, a, cmp, uint2(-3, 2), status); r += status;
  r += text1.SampleCmpLevelZero(samp1, a, cmp, uint2(-3, 2), status); r += status;
  return r;
}
