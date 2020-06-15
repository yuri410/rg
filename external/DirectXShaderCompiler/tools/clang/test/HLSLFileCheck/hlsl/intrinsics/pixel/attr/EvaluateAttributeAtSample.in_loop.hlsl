// RUN: %dxc -T ps_6_0 -E main %s -O3 | FileCheck %s

// CHECK: @main()

Texture2D tex0 : register(t0);
SamplerState samp0 : register(s0);

float4 main(float2 uv : TEXCOORD0) : SV_TARGET {
  float4 result = float4(0,0,0,0);
  [loop]
  for (int i = 0; i < 4; ++i)	{
    float2 coord = EvaluateAttributeAtSample(uv, i);
    result += tex0.Sample(samp0, coord);
  }
  return result;
}


