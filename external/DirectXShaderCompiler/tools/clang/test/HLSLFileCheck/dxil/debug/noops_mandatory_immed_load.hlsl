// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// Test that non-const arithmetic are not optimized away and
// do not impact things that require comstant (Like sample offset);

static const int2 offsets[] = {
  int2(-1,-1),
  int2(1,-1),
  int2(1,1),
  int2(7,-8),
};

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
SamplerState samp0 : register(s0);

[RootSignature("DescriptorTable(SRV(t0), SRV(t1)), DescriptorTable(Sampler(s0))")]
float4 main(float2 uv : TEXCOORD) : SV_Target {
  // CHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // CHECK-SAME: @dx.preserve.value
  // CHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  int a = 1;
  // CHECK: %[[a:.+]] = select i1 %[[p]], i32 1, i32 1

  int b = 2;
  // CHECK: %[[b:.+]] = select i1 %[[p]], i32 2, i32 2

  int d = a;
  // CHECK: %[[d:.+]] = select i1 %[[p]], i32 %[[a]], i32 %[[a]]

  int e = d + b;
  // CHECK: %[[add:.+]] = add

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60, 
  // CHECK-SAME: i32 7, i32 -8
  return tex0.Sample(samp0, uv, offsets[e]);
}

