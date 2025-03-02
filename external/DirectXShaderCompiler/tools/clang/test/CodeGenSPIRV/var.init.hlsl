// Run: %dxc -T ps_6_0 -E main

// Constants
// CHECK: [[float4constant:%\d+]] = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
// CHECK: [[int2constant:%\d+]] = OpConstantComposite %v2int %int_1 %int_2

float4 main(float component: COLOR) : SV_TARGET {
// CHECK-LABEL: %bb_entry = OpLabel

// CHECK: OpStore %a %int_0
    int a = 0; // From constant
// CHECK:      [[a0:%\d+]] = OpLoad %int %a
// CHECK-NEXT: OpStore %b [[a0]]
    int b = a; // From local variable

// CHECK-NEXT: OpStore %i %float_3
    float i = 1. + 2.;   // From const expr
// CHECK-NEXT: [[component0:%\d+]] = OpLoad %float %component
// CHECK-NEXT: OpStore %j [[component0]]
    float j = component; // From stage variable

// CHECK-NEXT: OpStore %m [[float4constant]]
    float4 m = float4(1.0, 2.0, 3.0, 4.0);  // All components are constants
// CHECK: [[j0:%\d+]] = OpLoad %float %j
// CHECK-NEXT: [[j1:%\d+]] = OpLoad %float %j
// CHECK-NEXT: [[j2:%\d+]] = OpLoad %float %j
// CHECK-NEXT: [[j3:%\d+]] = OpLoad %float %j
// CHECK-NEXT: [[ninit:%\d+]] = OpCompositeConstruct %v4float [[j0]] [[j1]] [[j2]] [[j3]]
// CHECK-NEXT: OpStore %n [[ninit]]
    float4 n = float4(j, j, j, j);          // All components are variables
// CHECK-NEXT: [[j4:%\d+]] = OpLoad %float %j
// CHECK-NEXT: [[j5:%\d+]] = OpLoad %float %j
// CHECK-NEXT: [[oinit:%\d+]] = OpCompositeConstruct %v4float %float_1 [[j4]] %float_3 [[j5]]
// CHECK-NEXT: OpStore %o [[oinit]]
    float4 o = float4(1.0, j, 3.0, j);      // Mixed case

// CHECK-NEXT: OpStore %p [[int2constant]]
    int2 p = {1, 2}; // All components are constants
// CHECK-NEXT: [[b1:%\d+]] = OpLoad %int %b
// CHECK-NEXT: [[a1:%\d+]] = OpLoad %int %a
// CHECK-NEXT: [[qinit:%\d+]] = OpCompositeConstruct %v3int %int_4 [[b1]] [[a1]]
// CHECK-NEXT: OpStore %q [[qinit]]
    int3 q = {4, b, a}; // Mixed cases

// CHECK-NEXT: OpStore %x %uint_1
    uint1 x = uint1(1); // Special case: vector of size 1

    float2 y;
// CHECK-NEXT: [[y0:%\d+]] = OpLoad %v2float %y
// CHECK-NEXT: [[y1:%\d+]] = OpLoad %v2float %y
// CHECK-NEXT: [[ce0:%\d+]] = OpCompositeExtract %float [[y0]] 0
// CHECK-NEXT: [[ce1:%\d+]] = OpCompositeExtract %float [[y0]] 1
// CHECK-NEXT: [[ce2:%\d+]] = OpCompositeExtract %float [[y1]] 0
// CHECK-NEXT: [[ce3:%\d+]] = OpCompositeExtract %float [[y1]] 1
// CHECK-NEXT: [[zinit:%\d+]] = OpCompositeConstruct %v4float [[ce0]] [[ce1]] [[ce2]] [[ce3]]
// CHECK-NEXT: OpStore %z [[zinit]]
    float4 z = {y, y};

    return o;
}
