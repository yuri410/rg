// Run: %dxc -T ps_6_0 -E main

float func(float3 i) { return i.x; }

float4 main(float1 i : TEXCOORD0) : SV_Target {
// CHECK: OpConstantComposite %v3float %float_1 %float_2 %float_3
// CHECK: OpConstantComposite %v3float %float_0 %float_0 %float_0
// CHECK: OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
  return func((float3(1, 2, 3) * i) > 0 ? 0.5 : 0);
}

