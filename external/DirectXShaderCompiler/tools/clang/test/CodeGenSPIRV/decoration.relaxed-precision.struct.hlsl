// Run: %dxc -T ps_6_0 -E main

min16uint foo(min12int param);

// CHECK:              OpMemberDecorate %S 1 RelaxedPrecision
// CHECK-NEXT:         OpMemberDecorate %S 2 RelaxedPrecision
// CHECK-NEXT:         OpMemberDecorate %S 3 RelaxedPrecision
// CHECK-NEXT:         OpMemberDecorate %S 4 RelaxedPrecision
struct S {
  int a;
  min12int b;
  min16int c;
  min16float d;
  min10float e;
  float f;
};

void main() {
// CHECK-NEXT:         OpDecorate %h RelaxedPrecision
// CHECK-NEXT:         OpDecorate %i RelaxedPrecision
// CHECK-NEXT:         OpDecorate %j RelaxedPrecision
// CHECK-NEXT:         OpDecorate %param_var_param RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[sb:%\d+]] RelaxedPrecision
// Note: Decoration is missing for sa_plus_sb.
// CHECK-NEXT:         OpDecorate [[sb2:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[sb2_int:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[sc:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[sb_plus_sc:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[sd:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[se:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[sd_plus_se:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[sb3:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[foo_result:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate %foo RelaxedPrecision
// CHECK-NEXT:         OpDecorate %param RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[param_value:%\d+]] RelaxedPrecision
// CHECK-NEXT:         OpDecorate [[param_plus_1:%\d+]] RelaxedPrecision

// CHECK:         %S = OpTypeStruct %int %int %int %float %float %float
  S s;
// src_main function:
// CHECK:              %h = OpVariable %_ptr_Function_int Function
// CHECK:              %i = OpVariable %_ptr_Function_float Function
// CHECK:              %j = OpVariable %_ptr_Function_uint Function
// CHECK: param_var_param = OpVariable %_ptr_Function_int Function

// CHECK:      [[s1ptr:%\d+]] = OpAccessChain %_ptr_Function_int %s %int_1
// CHECK:              [[sb]] = OpLoad %int [[s1ptr]]
// CHECK: [[sa_plus_sb:%\d+]] = OpIAdd %int
// While s.b is RelaxedPrecision (min12int), s.a is not (int).
// We can only annotate the OpIAdd as RelaxedPrecision if both were.
  int g = s.a + s.b;

// CHECK:  [[s1ptr:%\d+]] = OpAccessChain %_ptr_Function_int %s %int_1
// CHECK:         [[sb2]] = OpLoad %int [[s1ptr]]
// CHECK:     [[sb2_int]] = OpBitcast %int [[sb2]]
// CHECK:  [[s2ptr:%\d+]] = OpAccessChain %_ptr_Function_int %s %int_2
// CHECK:          [[sc]] = OpLoad %int [[s2ptr]]
// CHECK:  [[sb_plus_sc]] = OpIAdd %int [[sb2_int]] [[sc]]
  min16int h = s.b + s.c;

// CHECK:  [[s3ptr:%\d+]] = OpAccessChain %_ptr_Function_float %s %int_3
// CHECK:          [[sd]] = OpLoad %float [[s3ptr]]
// CHECK:  [[s4ptr:%\d+]] = OpAccessChain %_ptr_Function_float %s %int_4
// CHECK:          [[se]] = OpLoad %float [[s4ptr]]
// CHECK:  [[sd_plus_se]] = OpFAdd %float [[sd]] [[se]]
  min16float i = s.d + s.e;

// CHECK:  [[s1ptr:%\d+]] = OpAccessChain %_ptr_Function_int %s %int_1
// CHECK:         [[sb3]] = OpLoad %int [[s1ptr]]
// CHECK:  [[foo_result]] = OpFunctionCall %uint %foo %param_var_param
  min16uint j = foo(s.b);
}

// CHECK:            %foo = OpFunction %uint None %48
// CHECK:          %param = OpFunctionParameter %_ptr_Function_int
min16uint foo(min12int param) {
// CHECK: [[param_value]] = OpLoad %int %param
// CHECK:[[param_plus_1]] = OpIAdd %int
  return param + 1;
}

