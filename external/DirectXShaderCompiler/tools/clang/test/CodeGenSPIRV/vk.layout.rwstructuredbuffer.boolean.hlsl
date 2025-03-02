// Run: %dxc -T vs_6_0 -E main

// CHECK: [[v3uint1:%\d+]] = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
// CHECK: [[v3uint0:%\d+]] = OpConstantComposite %v3uint %uint_0 %uint_0 %uint_0

// CHECK: %T = OpTypeStruct %_arr_uint_uint_1
struct T {
  bool boolArray[1];
};

// CHECK: %S = OpTypeStruct %uint %v3uint %_arr_v3uint_uint_2 %T
struct S
{
  bool  boolScalar;
  bool3 boolVec;
  row_major bool2x3 boolMat;
  T t;
};

RWStructuredBuffer<S> values;

// These are the types that hold SPIR-V booleans, rather than Uints.
// CHECK: %T_0 = OpTypeStruct %_arr_bool_uint_1
// CHECK: %S_0 = OpTypeStruct %bool %v3bool %_arr_v3bool_uint_2 %T_0

void main()
{
  bool3 boolVecVar;
  bool2 boolVecVar2;
  row_major bool2x3 boolMatVar;

// CHECK:                [[uintPtr:%\d+]] = OpAccessChain %_ptr_Uniform_uint %values %int_0 %uint_0 %int_0
// CHECK-NEXT: [[convertTrueToUint:%\d+]] = OpSelect %uint %true %uint_1 %uint_0
// CHECK-NEXT:                              OpStore [[uintPtr]] [[convertTrueToUint]]
  values[0].boolScalar = true;

// CHECK:      [[boolVecVar:%\d+]] = OpLoad %v3bool %boolVecVar
// CHECK-NEXT: [[uintVecPtr:%\d+]] = OpAccessChain %_ptr_Uniform_v3uint %values %int_0 %uint_1 %int_1
// CHECK-NEXT: [[uintVecVar:%\d+]] = OpSelect %v3uint [[boolVecVar]] [[v3uint1]] [[v3uint0]]
// CHECK-NEXT:                       OpStore [[uintVecPtr]] [[uintVecVar]]
  values[1].boolVec = boolVecVar;

  // TODO: In the following cases, OpAccessChain runs into type mismatch issues due to decoration differences.
  // values[2].boolMat = boolMatVar;
  // values[0].boolVec.yzx = boolVecVar;
  // values[0].boolMat._m12_m11 = boolVecVar2;

// CHECK:              [[sPtr:%\d+]] = OpAccessChain %_ptr_Uniform_S %values %int_0 %uint_0
// CHECK-NEXT:            [[s:%\d+]] = OpLoad %S [[sPtr]]
// CHECK-NEXT:           [[s0:%\d+]] = OpCompositeExtract %uint [[s]] 0
// CHECK-NEXT:      [[s0_bool:%\d+]] = OpINotEqual %bool [[s0]] %uint_0
// CHECK-NEXT:           [[s1:%\d+]] = OpCompositeExtract %v3uint [[s]] 1
// CHECK-NEXT:      [[s1_bool:%\d+]] = OpINotEqual %v3bool [[s1]] [[v3uint0]]
// CHECK-NEXT:           [[s2:%\d+]] = OpCompositeExtract %_arr_v3uint_uint_2 [[s]] 2
// CHECK-NEXT:      [[s2_row0:%\d+]] = OpCompositeExtract %v3uint [[s2]] 0
// CHECK-NEXT: [[s2_row0_bool:%\d+]] = OpINotEqual %v3bool [[s2_row0]] [[v3uint0]]
// CHECK-NEXT:      [[s2_row1:%\d+]] = OpCompositeExtract %v3uint [[s2]] 1
// CHECK-NEXT: [[s2_row1_bool:%\d+]] = OpINotEqual %v3bool [[s2_row1]] [[v3uint0]]
// CHECK-NEXT:      [[s2_bool:%\d+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[s2_row0_bool]] [[s2_row1_bool]]
// CHECK-NEXT:            [[t:%\d+]] = OpCompositeExtract %T [[s]] 3
// CHECK-NEXT:  [[t0_uint_arr:%\d+]] = OpCompositeExtract %_arr_uint_uint_1 [[t]] 0
// CHECK-NEXT:    [[t0_0_uint:%\d+]] = OpCompositeExtract %uint [[t0_uint_arr]] 0
// CHECK-NEXT:    [[t0_0_bool:%\d+]] = OpINotEqual %bool [[t0_0_uint]] %uint_0
// CHECK-NEXT:      [[t0_bool:%\d+]] = OpCompositeConstruct %_arr_bool_uint_1 [[t0_0_bool]]
// CHECK-NEXT:       [[t_bool:%\d+]] = OpCompositeConstruct %T_0 [[t0_bool]]
// CHECK-NEXT:       [[s_bool:%\d+]] = OpCompositeConstruct %S_0 [[s0_bool]] [[s1_bool]] [[s2_bool]] [[t_bool]]
// CHECK-NEXT:                         OpStore %s [[s_bool]]
  S s = values[0];

// CHECK:                         [[s:%\d+]] = OpLoad %S_0 %s
// CHECK-NEXT:            [[resultPtr:%\d+]] = OpAccessChain %_ptr_Uniform_S %values %int_0 %uint_1
// CHECK-NEXT:              [[s0_bool:%\d+]] = OpCompositeExtract %bool [[s]] 0
// CHECK-NEXT:              [[s0_uint:%\d+]] = OpSelect %uint [[s0_bool]] %uint_1 %uint_0
// CHECK-NEXT:           [[s1_boolVec:%\d+]] = OpCompositeExtract %v3bool [[s]] 1
// CHECK-NEXT:           [[s1_uintVec:%\d+]] = OpSelect %v3uint [[s1_boolVec]]
// CHECK-NEXT:           [[s2_boolMat:%\d+]] = OpCompositeExtract %_arr_v3bool_uint_2 [[s]] 2
// CHECK-NEXT:      [[s2_boolMat_row0:%\d+]] = OpCompositeExtract %v3bool [[s2_boolMat]] 0
// CHECK-NEXT: [[s2_boolMat_row0_uint:%\d+]] = OpSelect %v3uint [[s2_boolMat_row0]]
// CHECK-NEXT:      [[s2_boolMat_row1:%\d+]] = OpCompositeExtract %v3bool [[s2_boolMat]] 1
// CHECK-NEXT: [[s2_boolMat_row1_uint:%\d+]] = OpSelect %v3uint [[s2_boolMat_row1]]
// CHECK-NEXT:           [[s2_uintMat:%\d+]] = OpCompositeConstruct %_arr_v3uint_uint_2 [[s2_boolMat_row0_uint]] [[s2_boolMat_row1_uint]]
// CHECK-NEXT:                    [[t:%\d+]] = OpCompositeExtract %T_0 [[s]] 3
// CHECK-NEXT:          [[t0_bool_arr:%\d+]] = OpCompositeExtract %_arr_bool_uint_1 [[t]] 0
// CHECK-NEXT:        [[t0_bool_arr_0:%\d+]] = OpCompositeExtract %bool [[t0_bool_arr]] 0
// CHECK-NEXT:   [[t0_bool_arr_0_uint:%\d+]] = OpSelect %uint [[t0_bool_arr_0]] %uint_1 %uint_0
// CHECK-NEXT:          [[t0_uint_arr:%\d+]] = OpCompositeConstruct %_arr_uint_uint_1 [[t0_bool_arr_0_uint]]
// CHECK-NEXT:               [[t_uint:%\d+]] = OpCompositeConstruct %T [[t0_uint_arr]]
// CHECK-NEXT:               [[s_uint:%\d+]] = OpCompositeConstruct %S [[s0_uint]] [[s1_uintVec]] [[s2_uintMat]] [[t_uint]]
// CHECK-NEXT:                                 OpStore [[resultPtr:%\d+]] [[s_uint]]
  values[1] = s;
}
