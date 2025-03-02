// Run: %dxc -T vs_6_0 -E main

// CHECK: [[glsl:%\d+]] = OpExtInstImport "GLSL.std.450"

// CHECK: OpDecorate [[fma1:%\d+]] NoContraction
// CHECK: OpDecorate [[fma2:%\d+]] NoContraction
// CHECK: OpDecorate [[fma3:%\d+]] NoContraction
// CHECK: OpDecorate [[fma4:%\d+]] NoContraction
// CHECK: OpDecorate [[mul1:%\d+]] NoContraction
// CHECK: OpDecorate [[add1:%\d+]] NoContraction
// CHECK: OpDecorate [[mul2:%\d+]] NoContraction
// CHECK: OpDecorate [[add2:%\d+]] NoContraction
// CHECK: OpDecorate [[mul3:%\d+]] NoContraction
// CHECK: OpDecorate [[add3:%\d+]] NoContraction
// CHECK: OpDecorate [[mul4:%\d+]] NoContraction
// CHECK: OpDecorate [[add4:%\d+]] NoContraction

void main() {
  float    a1, a2, a3, fma_a;
  float4   b1, b2, b3, fma_b;
  float2x3 c1, c2, c3, fma_c;

  int    d1, d2, d3, fma_d;
  int4   e1, e2, e3, fma_e;
  int2x3 f1, f2, f3, fma_f;

// CHECK:      [[a1:%\d+]] = OpLoad %float %a1
// CHECK-NEXT: [[a2:%\d+]] = OpLoad %float %a2
// CHECK-NEXT: [[a3:%\d+]] = OpLoad %float %a3
// CHECK-NEXT:    [[fma1]] = OpExtInst %float [[glsl]] Fma [[a1]] [[a2]] [[a3]]
  fma_a = mad(a1, a2, a3);

// CHECK:      [[b1:%\d+]] = OpLoad %v4float %b1
// CHECK-NEXT: [[b2:%\d+]] = OpLoad %v4float %b2
// CHECK-NEXT: [[b3:%\d+]] = OpLoad %v4float %b3
// CHECK-NEXT:    [[fma2]] = OpExtInst %v4float [[glsl]] Fma [[b1]] [[b2]] [[b3]]
  fma_b = mad(b1, b2, b3);

// CHECK:            [[c1:%\d+]] = OpLoad %mat2v3float %c1
// CHECK-NEXT:       [[c2:%\d+]] = OpLoad %mat2v3float %c2
// CHECK-NEXT:       [[c3:%\d+]] = OpLoad %mat2v3float %c3
// CHECK-NEXT:  [[c1_row0:%\d+]] = OpCompositeExtract %v3float [[c1]] 0
// CHECK-NEXT:  [[c2_row0:%\d+]] = OpCompositeExtract %v3float [[c2]] 0
// CHECK-NEXT:  [[c3_row0:%\d+]] = OpCompositeExtract %v3float [[c3]] 0
// CHECK-NEXT:          [[fma3]] = OpExtInst %v3float [[glsl]] Fma [[c1_row0]] [[c2_row0]] [[c3_row0]]
// CHECK-NEXT:  [[c1_row1:%\d+]] = OpCompositeExtract %v3float [[c1]] 1
// CHECK-NEXT:  [[c2_row1:%\d+]] = OpCompositeExtract %v3float [[c2]] 1
// CHECK-NEXT:  [[c3_row1:%\d+]] = OpCompositeExtract %v3float [[c3]] 1
// CHECK-NEXT:          [[fma4]] = OpExtInst %v3float [[glsl]] Fma [[c1_row1]] [[c2_row1]] [[c3_row1]]
// CHECK-NEXT:          {{%\d+}} = OpCompositeConstruct %mat2v3float [[fma3]] [[fma4]]
  fma_c = mad(c1, c2, c3);

// CHECK:       [[d1:%\d+]] = OpLoad %int %d1
// CHECK-NEXT:  [[d2:%\d+]] = OpLoad %int %d2
// CHECK-NEXT:  [[d3:%\d+]] = OpLoad %int %d3
// CHECK-NEXT:     [[mul1]] = OpIMul %int [[d1]] [[d2]]
// CHECK-NEXT:     [[add1]] = OpIAdd %int [[mul1]] [[d3]]
  fma_d = mad(d1, d2, d3);

// CHECK:       [[e1:%\d+]] = OpLoad %v4int %e1
// CHECK-NEXT:  [[e2:%\d+]] = OpLoad %v4int %e2
// CHECK-NEXT:  [[e3:%\d+]] = OpLoad %v4int %e3
// CHECK-NEXT:     [[mul2]] = OpIMul %v4int [[e1]] [[e2]]
// CHECK-NEXT:     [[add2]] = OpIAdd %v4int [[mul2]] [[e3]]
  fma_e = mad(e1, e2, e3);

// CHECK:           [[f1:%\d+]] = OpLoad %_arr_v3int_uint_2 %f1
// CHECK-NEXT:      [[f2:%\d+]] = OpLoad %_arr_v3int_uint_2 %f2
// CHECK-NEXT:      [[f3:%\d+]] = OpLoad %_arr_v3int_uint_2 %f3
// CHECK-NEXT:  [[f1row0:%\d+]] = OpCompositeExtract %v3int [[f1]] 0
// CHECK-NEXT:  [[f2row0:%\d+]] = OpCompositeExtract %v3int [[f2]] 0
// CHECK-NEXT:  [[f3row0:%\d+]] = OpCompositeExtract %v3int [[f3]] 0
// CHECK-NEXT:         [[mul3]] = OpIMul %v3int [[f1row0]] [[f2row0]]
// CHECK-NEXT:         [[add3]] = OpIAdd %v3int [[mul3]] [[f3row0]]
// CHECK-NEXT:  [[f1row1:%\d+]] = OpCompositeExtract %v3int [[f1]] 1
// CHECK-NEXT:  [[f2row1:%\d+]] = OpCompositeExtract %v3int [[f2]] 1
// CHECK-NEXT:  [[f3row1:%\d+]] = OpCompositeExtract %v3int [[f3]] 1
// CHECK-NEXT:         [[mul4]] = OpIMul %v3int [[f1row1]] [[f2row1]]
// CHECK-NEXT:         [[add4]] = OpIAdd %v3int [[mul4]] [[f3row1]]
// CHECK-NEXT:         {{%\d+}} = OpCompositeConstruct %_arr_v3int_uint_2 [[add3]] [[add4]]
  fma_f = mad(f1, f2, f3);
}

