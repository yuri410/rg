// Run: %dxc -T vs_6_0 -E main

// CHECK: [[v4f32c:%\d+]] = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
// CHECK: [[v3f32c:%\d+]] = OpConstantComposite %v3float %float_2 %float_2 %float_2
// CHECK: [[v4fc_2:%\d+]] = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
// CHECK: [[v4fc_3:%\d+]] = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    // From constant
// CHECK: OpStore %vf4 [[v4f32c]]
    float4 vf4 = 1;

// CHECK-NEXT: OpStore %vf3 [[v3f32c]]
    float3 vf3;
    vf3 = float1(2);

// CHECK-NEXT: [[si:%\d+]] = OpLoad %int %si
// CHECK-NEXT: [[vi4:%\d+]] = OpCompositeConstruct %v4int [[si]] [[si]] [[si]] [[si]]
// CHECK-NEXT: OpStore %vi4 [[vi4]]
    int si;
    int4 vi4 = si;
// CHECK-NEXT: [[si1:%\d+]] = OpLoad %int %si1
// CHECK-NEXT: [[vi3:%\d+]] = OpCompositeConstruct %v3int [[si1]] [[si1]] [[si1]]
// CHECK-NEXT: OpStore %vi3 [[vi3]]
    int1 si1;
    int3 vi3;
    vi3 = si1;

// CHECK-NEXT: [[v0p5:%\d+]] = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
// CHECK-NEXT: OpStore %vf4 [[v0p5]]
    vf4 = float4(0.5.xxxx);

// CHECK-NEXT: [[v3:%\d+]] = OpCompositeConstruct %v3int %int_3 %int_3 %int_3
// CHECK-NEXT: OpStore %vi3 [[v3]]
    vi3 = int3(3.xxx);

    float sf;

// CHECK:         [[sf:%\d+]] = OpLoad %float %sf
// CHECK-NEXT: [[sf_v4:%\d+]] = OpCompositeConstruct %v4float [[sf]] [[sf]] [[sf]] [[sf]]
// CHECK-NEXT:                  OpStore %vf4_0 [[sf_v4]]
    float4 vf4_0 = (sf, sf, sf, sf);

// CHECK:         [[sf:%\d+]] = OpLoad %float %sf
// CHECK-NEXT: [[sf_v4:%\d+]] = OpCompositeConstruct %v4float [[sf]] [[sf]] [[sf]] [[sf]]
// CHECK-NEXT:                  OpStore %vf4 [[sf_v4]]
    vf4 = (sf, sf, sf, sf);

// CHECK: OpStore %vf4_1 [[v4fc_2]]
    float4 vf4_1 = (2.0f, 2.0f, 2.0f, 2.0f);

// CHECK: OpStore %vf4 [[v4fc_3]]
    vf4 = (3.0f, 3.0f, 3.0f, 3.0f);
}
