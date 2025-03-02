// Run: %dxc -T cs_6_0 -E main

struct struct_with_bool {
  bool2 elem_v2bool;
  int2 elem_v2int;
  float2 elem_v2float;
  bool elem_bool;
  int elem_int;
  float elem_float;
};

ConsumeStructuredBuffer<bool2> consume_v2bool;
ConsumeStructuredBuffer<float2> consume_v2float;
ConsumeStructuredBuffer<int2> consume_v2int;
ConsumeStructuredBuffer<struct_with_bool> consume_struct_with_bool;

ConsumeStructuredBuffer<bool> consume_bool;
ConsumeStructuredBuffer<float> consume_float;
ConsumeStructuredBuffer<int> consume_int;

AppendStructuredBuffer<bool2> append_v2bool;
AppendStructuredBuffer<float2> append_v2float;
AppendStructuredBuffer<int2> append_v2int;
AppendStructuredBuffer<struct_with_bool> append_struct_with_bool;

AppendStructuredBuffer<bool> append_bool;
AppendStructuredBuffer<float> append_float;
AppendStructuredBuffer<int> append_int;

RWStructuredBuffer<bool> rw_bool;
RWStructuredBuffer<bool2> rw_v2bool;

ConsumeStructuredBuffer<float2x2> consume_float2x2;
AppendStructuredBuffer<float4> append_v4float;

void main() {
// CHECK:       [[p_0:%\d+]] = OpAccessChain %_ptr_Uniform_uint %append_bool %uint_0 {{%\d+}}

// CHECK:       [[p_1:%\d+]] = OpAccessChain %_ptr_Uniform_uint %consume_bool %uint_0 {{%\d+}}
// CHECK-NEXT:  [[i_0:%\d+]] = OpLoad %uint [[p_1]]
// CHECK-NEXT:                 OpStore [[p_0]] [[i_0]]
  append_bool.Append(consume_bool.Consume());

// CHECK:       [[p_2:%\d+]] = OpAccessChain %_ptr_Uniform_int %consume_int %uint_0 {{%\d+}}
// CHECK-NEXT:  [[i_2:%\d+]] = OpLoad %int [[p_2]]
// CHECK-NEXT:  [[b_2:%\d+]] = OpINotEqual %bool [[i_2]] %int_0
// CHECK-NEXT: [[bi_2:%\d+]] = OpSelect %uint [[b_2]] %uint_1 %uint_0
// CHECK-NEXT:                 OpStore {{%\d+}} [[bi_2]]
  append_bool.Append(consume_int.Consume());

// CHECK:       [[p_3:%\d+]] = OpAccessChain %_ptr_Uniform_float %consume_float %uint_0 {{%\d+}}
// CHECK-NEXT:  [[f_3:%\d+]] = OpLoad %float [[p_3]]
// CHECK-NEXT:  [[b_3:%\d+]] = OpFOrdNotEqual %bool [[f_3]] %float_0
// CHECK-NEXT: [[bi_3:%\d+]] = OpSelect %uint [[b_3]] %uint_1 %uint_0
// CHECK-NEXT:                 OpStore {{%\d+}} [[bi_3]]
  append_bool.Append(consume_float.Consume());

// CHECK:       [[p_4:%\d+]] = OpAccessChain %_ptr_Uniform_uint %append_bool %uint_0 {{%\d+}}

// CHECK:       [[p_5:%\d+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%\d+}}
// CHECK-NEXT:  [[p_6:%\d+]] = OpAccessChain %_ptr_Uniform_uint [[p_5]] %int_3
// CHECK-NEXT:  [[i_5:%\d+]] = OpLoad %uint [[p_6]]
// CHECK-NEXT: [[bi_5:%\d+]] = OpINotEqual %bool [[i_5]] %uint_0
// CHECK-NEXT:  [[i_5:%\d+]] = OpSelect %uint [[bi_5]] %uint_1 %uint_0
// CHECK-NEXT:                 OpStore [[p_4]] [[i_5]]
  append_bool.Append(consume_struct_with_bool.Consume().elem_bool);

  //
  // TODO(jaebaek): Uncomment this and all other commented lines after fixing type cast bug
  // https://github.com/Microsoft/DirectXShaderCompiler/issues/2031
  //
  // append_bool.Append(consume_struct_with_bool.Consume().elem_int);
  // append_bool.Append(consume_struct_with_bool.Consume().elem_float);

// CHECK:       [[p_7:%\d+]] = OpAccessChain %_ptr_Uniform_uint %consume_bool %uint_0 {{%\d+}}
// CHECK-NEXT:  [[i_7:%\d+]] = OpLoad %uint [[p_7]]
// CHECK-NEXT:  [[b_7:%\d+]] = OpINotEqual %bool [[i_7]] %uint_0
// CHECK-NEXT: [[bi_7:%\d+]] = OpSelect %int [[b_7]] %int_1 %int_0
// CHECK-NEXT:                 OpStore {{%\d+}} [[bi_7]]
  append_int.Append(consume_bool.Consume());

// CHECK:       [[p_8:%\d+]] = OpAccessChain %_ptr_Uniform_int %consume_int %uint_0 {{%\d+}}
// CHECK-NEXT:  [[i_8:%\d+]] = OpLoad %int [[p_8]]
// CHECK-NEXT:                 OpStore {{%\d+}} [[i_8]]
  append_int.Append(consume_int.Consume());

// CHECK:       [[p_9:%\d+]] = OpAccessChain %_ptr_Uniform_float %consume_float %uint_0 {{%\d+}}
// CHECK-NEXT:  [[f_9:%\d+]] = OpLoad %float [[p_9]]
// CHECK-NEXT:  [[i_9:%\d+]] = OpConvertFToS %int [[f_9]]
// CHECK-NEXT:                 OpStore {{%\d+}} [[i_9]]
  append_int.Append(consume_float.Consume());

  // append_int.Append(consume_struct_with_bool.Consume().elem_bool);

// CHECK:      [[p_10:%\d+]] = OpAccessChain %_ptr_Uniform_int {{%\d+}} %int_4
// CHECK-NEXT: [[i_10:%\d+]] = OpLoad %int [[p_10]]
// CHECK-NEXT:                 OpStore {{%\d+}} [[i_10]]
  append_int.Append(consume_struct_with_bool.Consume().elem_int);

  // append_int.Append(consume_struct_with_bool.Consume().elem_float);

// CHECK:      [[p_11:%\d+]] = OpAccessChain %_ptr_Uniform_uint %consume_bool %uint_0 {{%\d+}}
// CHECK-NEXT: [[i_11:%\d+]] = OpLoad %uint [[p_11]]
// CHECK-NEXT: [[b_11:%\d+]] = OpINotEqual %bool [[i_11]] %uint_0
// CHECK-NEXT: [[f_11:%\d+]] = OpSelect %float [[b_11]] %float_1 %float_0
// CHECK-NEXT:                 OpStore {{%\d+}} [[f_11]]
  append_float.Append(consume_bool.Consume());

// CHECK:      [[p_12:%\d+]] = OpAccessChain %_ptr_Uniform_int %consume_int %uint_0 {{%\d+}}
// CHECK-NEXT: [[i_12:%\d+]] = OpLoad %int [[p_12]]
// CHECK-NEXT: [[f_12:%\d+]] = OpConvertSToF %float [[i_12]]
// CHECK-NEXT:                 OpStore {{%\d+}} [[f_12]]
  append_float.Append(consume_int.Consume());

// CHECK:      [[p_13:%\d+]] = OpAccessChain %_ptr_Uniform_float %consume_float %uint_0 {{%\d+}}
// CHECK-NEXT: [[f_13:%\d+]] = OpLoad %float [[p_13]]
// CHECK-NEXT:                 OpStore {{%\d+}} [[f_13]]
  append_float.Append(consume_float.Consume());

  // append_float.Append(consume_struct_with_bool.Consume().elem_bool);
  // append_float.Append(consume_struct_with_bool.Consume().elem_int);

// CHECK:      [[p_14:%\d+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%\d+}}
// CHECK-NEXT: [[p_15:%\d+]] = OpAccessChain %_ptr_Uniform_float [[p_14]] %int_5
// CHECK-NEXT: [[f_15:%\d+]] = OpLoad %float [[p_15]]
// CHECK-NEXT:                 OpStore {{%\d+}} [[f_15]]
  append_float.Append(consume_struct_with_bool.Consume().elem_float);

// CHECK:       [[p_16:%\d+]] = OpAccessChain %_ptr_Uniform_v2uint %consume_v2bool %uint_0 {{%\d+}}
// CHECK-NEXT: [[vu_16:%\d+]] = OpLoad %v2uint [[p_16]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[vu_16]]
  append_v2bool.Append(consume_v2bool.Consume());

// CHECK:       [[p_17:%\d+]] = OpAccessChain %_ptr_Uniform_v2int %consume_v2int %uint_0 {{%\d+}}
// CHECK-NEXT: [[vi_17:%\d+]] = OpLoad %v2int [[p_17]]
// CHECK-NEXT: [[vb_17:%\d+]] = OpINotEqual %v2bool [[vi_17]] {{%\d+}}
// CHECK-NEXT: [[vu_17:%\d+]] = OpSelect %v2uint [[vb_17]] {{%\d+}} {{%\d+}}
// CHECK-NEXT:                  OpStore {{%\d+}} [[vu_17]]
  append_v2bool.Append(consume_v2int.Consume());

// CHECK:       [[p_18:%\d+]] = OpAccessChain %_ptr_Uniform_v2float %consume_v2float %uint_0 {{%\d+}}
// CHECK-NEXT: [[vf_18:%\d+]] = OpLoad %v2float [[p_18]]
// CHECK-NEXT: [[vb_18:%\d+]] = OpFOrdNotEqual %v2bool [[vf_18]] {{%\d+}}
// CHECK-NEXT: [[vu_18:%\d+]] = OpSelect %v2uint [[vb_18]] {{%\d+}} {{%\d+}}
// CHECK-NEXT:                  OpStore {{%\d+}} [[vu_18]]
  append_v2bool.Append(consume_v2float.Consume());

// CHECK:       [[p_19:%\d+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%\d+}}
// CHECK-NEXT:  [[p_20:%\d+]] = OpAccessChain %_ptr_Uniform_v2uint [[p_19]] %int_0
// CHECK-NEXT: [[vu_20:%\d+]] = OpLoad %v2uint [[p_20]]
// CHECK-NEXT: [[vb_20:%\d+]] = OpINotEqual %v2bool [[vu_20]] {{%\d+}}
// CHECK-NEXT: [[vu_20:%\d+]] = OpSelect %v2uint [[vb_20]] {{%\d+}} {{%\d+}}
// CHECK-NEXT:                  OpStore {{%\d+}} [[vu_20]]
  append_v2bool.Append(consume_struct_with_bool.Consume().elem_v2bool);

// CHECK:       [[p_21:%\d+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%\d+}}
// CHECK:       [[p_22:%\d+]] = OpAccessChain %_ptr_Uniform_v2int [[p_21]] %int_1
// CHECK-NEXT: [[vi_22:%\d+]] = OpLoad %v2int [[p_22]]
// CHECK-NEXT: [[vb_22:%\d+]] = OpINotEqual %v2bool [[vi_22]] {{%\d+}}
// CHECK-NEXT: [[vu_22:%\d+]] = OpSelect %v2uint [[vb_22]] {{%\d+}} {{%\d+}}
// CHECK-NEXT:                  OpStore {{%\d+}} [[vu_22]]
  append_v2bool.Append(consume_struct_with_bool.Consume().elem_v2int);

// CHECK:       [[p_23:%\d+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%\d+}}
// CHECK:       [[p_24:%\d+]] = OpAccessChain %_ptr_Uniform_v2float [[p_23]] %int_2
// CHECK-NEXT: [[vf_24:%\d+]] = OpLoad %v2float [[p_24]]
// CHECK-NEXT: [[vb_24:%\d+]] = OpFOrdNotEqual %v2bool [[vf_24]] {{%\d+}}
// CHECK-NEXT: [[vu_24:%\d+]] = OpSelect %v2uint [[vb_24]] {{%\d+}} {{%\d+}}
// CHECK-NEXT:                  OpStore {{%\d+}} [[vu_24]]
  append_v2bool.Append(consume_struct_with_bool.Consume().elem_v2float);

// CHECK:       [[p_25:%\d+]] = OpAccessChain %_ptr_Uniform_v2uint %consume_v2bool %uint_0 {{%\d+}}
// CHECK-NEXT: [[vu_25:%\d+]] = OpLoad %v2uint [[p_25]]
// CHECK-NEXT: [[vb_25:%\d+]] = OpINotEqual %v2bool [[vu_25]] {{%\d+}}
// CHECK-NEXT: [[vu_25:%\d+]] = OpSelect %v2int [[vb_25]] {{%\d+}} {{%\d+}}
// CHECK-NEXT:                  OpStore {{%\d+}} [[vu_25]]
  append_v2int.Append(consume_v2bool.Consume());

// CHECK:       [[p_26:%\d+]] = OpAccessChain %_ptr_Uniform_v2int %consume_v2int %uint_0 {{%\d+}}
// CHECK-NEXT: [[vi_26:%\d+]] = OpLoad %v2int [[p_26]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[vi_26]]
  append_v2int.Append(consume_v2int.Consume());

// CHECK:       [[p_27:%\d+]] = OpAccessChain %_ptr_Uniform_v2float %consume_v2float %uint_0 {{%\d+}}
// CHECK-NEXT: [[vf_27:%\d+]] = OpLoad %v2float [[p_27]]
// CHECK-NEXT: [[vi_27:%\d+]] = OpConvertFToS %v2int [[vf_27]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[vi_27]]
  append_v2int.Append(consume_v2float.Consume());

// CHECK:       [[p_28:%\d+]] = OpAccessChain %_ptr_Uniform_v2uint {{%\d+}} %int_0
// CHECK-NEXT: [[vu_28:%\d+]] = OpLoad %v2uint [[p_28]]
// CHECK-NEXT: [[vb_28:%\d+]] = OpINotEqual %v2bool [[vu_28]] {{%\d+}}
// CHECK-NEXT: [[vi_28:%\d+]] = OpSelect %v2int [[vb_28]] {{%\d+}} {{%\d+}}
// CHECK-NEXT:                  OpStore {{%\d+}} [[vi_28]]
  append_v2int.Append(consume_struct_with_bool.Consume().elem_v2bool);

// CHECK:       [[p_29:%\d+]] = OpAccessChain %_ptr_Uniform_v2int {{%\d+}} %int_1
// CHECK-NEXT: [[vi_29:%\d+]] = OpLoad %v2int [[p_29]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[vi_29]]
  append_v2int.Append(consume_struct_with_bool.Consume().elem_v2int);

// CHECK:       [[p_30:%\d+]] = OpAccessChain %_ptr_Uniform_v2float {{%\d+}} %int_2
// CHECK-NEXT: [[vf_30:%\d+]] = OpLoad %v2float [[p_30]]
// CHECK-NEXT: [[vi_30:%\d+]] = OpConvertFToS %v2int [[vf_30]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[vi_30]]
  append_v2int.Append(consume_struct_with_bool.Consume().elem_v2float);

// CHECK:       [[p_31:%\d+]] = OpAccessChain %_ptr_Uniform_v2uint %consume_v2bool %uint_0 {{%\d+}}
// CHECK-NEXT: [[vu_31:%\d+]] = OpLoad %v2uint [[p_31]]
// CHECK-NEXT: [[vb_31:%\d+]] = OpINotEqual %v2bool [[vu_31]] {{%\d+}}
// CHECK-NEXT: [[vf_31:%\d+]] = OpSelect %v2float [[vb_31]] {{%\d+}} {{%\d+}}
// CHECK-NEXT:                  OpStore {{%\d+}} [[vf_31]]
  append_v2float.Append(consume_v2bool.Consume());

// CHECK:       [[p_32:%\d+]] = OpAccessChain %_ptr_Uniform_v2int %consume_v2int %uint_0 {{%\d+}}
// CHECK-NEXT: [[vi_32:%\d+]] = OpLoad %v2int [[p_32]]
// CHECK-NEXT: [[vf_32:%\d+]] = OpConvertSToF %v2float [[vi_32]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[vf_32]]
  append_v2float.Append(consume_v2int.Consume());

// CHECK:       [[p_33:%\d+]] = OpAccessChain %_ptr_Uniform_v2float %consume_v2float %uint_0 {{%\d+}}
// CHECK-NEXT: [[vf_33:%\d+]] = OpLoad %v2float [[p_33]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[vf_33]]
  append_v2float.Append(consume_v2float.Consume());

// CHECK:       [[p_34:%\d+]] = OpAccessChain %_ptr_Uniform_v2uint {{%\d+}} %int_0
// CHECK-NEXT: [[vu_34:%\d+]] = OpLoad %v2uint [[p_34]]
// CHECK-NEXT: [[vb_34:%\d+]] = OpINotEqual %v2bool [[vu_34]] {{%\d+}}
// CHECK-NEXT: [[vf_34:%\d+]] = OpSelect %v2float [[vb_34]] {{%\d+}} {{%\d+}}
// CHECK-NEXT:                  OpStore {{%\d+}} [[vf_34]]
  append_v2float.Append(consume_struct_with_bool.Consume().elem_v2bool);

// CHECK:       [[p_35:%\d+]] = OpAccessChain %_ptr_Uniform_v2int {{%\d+}} %int_1
// CHECK-NEXT: [[vi_35:%\d+]] = OpLoad %v2int [[p_35]]
// CHECK-NEXT: [[vf_35:%\d+]] = OpConvertSToF %v2float [[vi_35]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[vf_35]]
  append_v2float.Append(consume_struct_with_bool.Consume().elem_v2int);

// CHECK:       [[p_36:%\d+]] = OpAccessChain %_ptr_Uniform_v2float {{%\w+}} %int_2
// CHECK-NEXT: [[vf_36:%\d+]] = OpLoad %v2float [[p_36]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[vf_36]]
  append_v2float.Append(consume_struct_with_bool.Consume().elem_v2float);

// CHECK:       [[p_37:%\d+]] = OpAccessChain %_ptr_Uniform_v2uint %consume_v2bool %uint_0 {{%\d+}}
// CHECK-NEXT:  [[p_38:%\d+]] = OpAccessChain %_ptr_Uniform_uint [[p_37]] %int_0
// CHECK-NEXT:  [[i_38:%\d+]] = OpLoad %uint [[p_38]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[i_38]]
  append_bool.Append(consume_v2bool.Consume().x);

// CHECK:       [[p_39:%\d+]] = OpAccessChain %_ptr_Uniform_int {{%\d+}} %int_0
// CHECK-NEXT:  [[i_39:%\d+]] = OpLoad %int [[p_39]]
// CHECK-NEXT:  [[b_39:%\d+]] = OpINotEqual %bool [[i_39]] %int_0
// CHECK-NEXT: [[bi_39:%\d+]] = OpSelect %uint [[b_39]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore {{%\d+}} [[bi_39]]
  append_bool.Append(consume_v2int.Consume().x);

// CHECK:       [[p_40:%\d+]] = OpAccessChain %_ptr_Uniform_float {{%\d+}} %int_0
// CHECK-NEXT:  [[f_40:%\d+]] = OpLoad %float [[p_40]]
// CHECK-NEXT:  [[b_40:%\d+]] = OpFOrdNotEqual %bool [[f_40]] %float_0
// CHECK-NEXT: [[bi_40:%\d+]] = OpSelect %uint [[b_40]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore {{%\d+}} [[bi_40]]
  append_bool.Append(consume_v2float.Consume().x);

// CHECK:       [[p_41:%\d+]] = OpAccessChain %_ptr_Uniform_uint {{%\d+}} %int_0
// CHECK-NEXT:  [[i_41:%\d+]] = OpLoad %uint [[p_41]]
// CHECK-NEXT:  [[b_41:%\d+]] = OpINotEqual %bool [[i_41]] %uint_0
// CHECK-NEXT:  [[i_41:%\d+]] = OpSelect %uint [[b_41]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore {{%\d+}} [[i_41]]
  append_bool.Append(consume_struct_with_bool.Consume().elem_v2bool.x);

  // append_bool.Append(consume_struct_with_bool.Consume().elem_v2int.x);
  // append_bool.Append(consume_struct_with_bool.Consume().elem_v2float.x);

// CHECK:       [[p_42:%\d+]] = OpAccessChain %_ptr_Uniform_uint {{%\d+}} %int_0
// CHECK-NEXT:  [[i_42:%\d+]] = OpLoad %uint [[p_42]]
// CHECK-NEXT:  [[b_42:%\d+]] = OpINotEqual %bool [[i_42]] %uint_0
// CHECK-NEXT: [[bi_42:%\d+]] = OpSelect %int [[b_42]] %int_1 %int_0
// CHECK-NEXT:                  OpStore {{%\d+}} [[bi_42]]
  append_int.Append(consume_v2bool.Consume().x);

// CHECK:       [[p_43:%\d+]] = OpAccessChain %_ptr_Uniform_int {{%\d+}} %int_0
// CHECK-NEXT:  [[i_43:%\d+]] = OpLoad %int [[p_43]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[i_43]]
  append_int.Append(consume_v2int.Consume().x);

// CHECK:       [[p_44:%\d+]] = OpAccessChain %_ptr_Uniform_float {{%\d+}} %int_0
// CHECK-NEXT:  [[f_44:%\d+]] = OpLoad %float [[p_44]]
// CHECK-NEXT:  [[i_44:%\d+]] = OpConvertFToS %int [[f_44]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[i_44]]
  append_int.Append(consume_v2float.Consume().x);

  // append_int.Append(consume_struct_with_bool.Consume().elem_v2bool.x);

// CHECK:       [[p_45:%\d+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%\d+}}
// CHECK-NEXT:  [[p_46:%\d+]] = OpAccessChain %_ptr_Uniform_v2int [[p_45]] %int_1
// CHECK-NEXT:  [[p_47:%\d+]] = OpAccessChain %_ptr_Uniform_int [[p_46]] %int_0
// CHECK-NEXT:  [[i_47:%\d+]] = OpLoad %int [[p_47]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[i_47]]
  append_int.Append(consume_struct_with_bool.Consume().elem_v2int.x);

  // append_int.Append(consume_struct_with_bool.Consume().elem_v2float.x);

// CHECK:       [[p_48:%\d+]] = OpAccessChain %_ptr_Uniform_uint {{%\d+}} %int_0
// CHECK-NEXT:  [[i_48:%\d+]] = OpLoad %uint [[p_48]]
// CHECK-NEXT:  [[b_48:%\d+]] = OpINotEqual %bool [[i_48]] %uint_0
// CHECK-NEXT:  [[f_48:%\d+]] = OpSelect %float [[b_48]] %float_1 %float_0
// CHECK-NEXT:                  OpStore {{%\d+}} [[f_48]]
  append_float.Append(consume_v2bool.Consume().x);

// CHECK:       [[p_49:%\d+]] = OpAccessChain %_ptr_Uniform_int {{%\d+}} %int_0
// CHECK-NEXT:  [[i_49:%\d+]] = OpLoad %int [[p_49]]
// CHECK-NEXT:  [[f_49:%\d+]] = OpConvertSToF %float [[i_49]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[f_49]]
  append_float.Append(consume_v2int.Consume().x);

// CHECK:       [[p_50:%\d+]] = OpAccessChain %_ptr_Uniform_float {{%\d+}} %int_0
// CHECK-NEXT:  [[f_50:%\d+]] = OpLoad %float [[p_50]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[f_50]]
  append_float.Append(consume_v2float.Consume().x);

  // append_float.Append(consume_struct_with_bool.Consume().elem_v2bool.x);
  // append_float.Append(consume_struct_with_bool.Consume().elem_v2int.x);

// CHECK:       [[p_51:%\d+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%\d+}}
// CHECK-NEXT:  [[p_52:%\d+]] = OpAccessChain %_ptr_Uniform_v2float [[p_51]] %int_2
// CHECK-NEXT:  [[p_53:%\d+]] = OpAccessChain %_ptr_Uniform_float [[p_52]] %int_0
// CHECK-NEXT:  [[f_53:%\d+]] = OpLoad %float [[p_53]]
// CHECK-NEXT:                  OpStore {{%\d+}} [[f_53]]
  append_float.Append(consume_struct_with_bool.Consume().elem_v2float.x);

// CHECK:       [[p_54:%\d+]] = OpAccessChain %_ptr_Uniform_uint %append_bool %uint_0 {{%\d+}}
// CHECK:       [[p_55:%\d+]] = OpAccessChain %_ptr_Uniform_uint %rw_bool %int_0 %uint_0
// CHECK-NEXT:  [[i_55:%\d+]] = OpLoad %uint [[p_55]]
// CHECK-NEXT:  [[b_55:%\d+]] = OpINotEqual %bool [[i_55]] %uint_0
// CHECK-NEXT: [[bi_55:%\d+]] = OpSelect %uint [[b_55]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore [[p_54]] [[bi_55]]
  append_bool.Append(rw_bool[0]);

// CHECK:       [[i_56:%\d+]] = OpLoad %uint {{%\d+}}
// CHECK-NEXT:  [[b_56:%\d+]] = OpINotEqual %bool [[i_56]] %uint_0
// CHECK-NEXT: [[bi_56:%\d+]] = OpSelect %uint [[b_56]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore {{%\d+}} [[bi_56]]
  append_bool.Append(rw_v2bool[0].x);

// CHECK:       [[p_57:%\d+]] = OpAccessChain %_ptr_Uniform_int %append_int %uint_0 {{%\d+}}
// CHECK:       [[p_58:%\d+]] = OpAccessChain %_ptr_Uniform_uint %rw_bool %int_0 %uint_0
// CHECK-NEXT:  [[i_58:%\d+]] = OpLoad %uint [[p_58]]
// CHECK-NEXT:  [[b_58:%\d+]] = OpINotEqual %bool [[i_58]] %uint_0
// CHECK-NEXT: [[bi_58:%\d+]] = OpSelect %int [[b_58]] %int_1 %int_0
// CHECK-NEXT:                  OpStore [[p_57]] [[bi_58]]
  append_int.Append(rw_bool[0]);

// CHECK:       [[i_59:%\d+]] = OpLoad %uint {{%\d+}}
// CHECK-NEXT:  [[b_59:%\d+]] = OpINotEqual %bool [[i_59]] %uint_0
// CHECK-NEXT: [[bi_59:%\d+]] = OpSelect %int [[b_59]] %int_1 %int_0
// CHECK-NEXT:                 OpStore {{%\d+}} [[bi_59]]
  append_int.Append(rw_v2bool[0].x);

// CHECK:      [[p_60:%\d+]] = OpAccessChain %_ptr_Uniform_float %append_float %uint_0 {{%\d+}}
// CHECK:      [[p_61:%\d+]] = OpAccessChain %_ptr_Uniform_uint %rw_bool %int_0 %uint_0
// CHECK-NEXT: [[i_61:%\d+]] = OpLoad %uint [[p_61]]
// CHECK-NEXT: [[b_61:%\d+]] = OpINotEqual %bool [[i_61]] %uint_0
// CHECK-NEXT: [[f_61:%\d+]] = OpSelect %float [[b_61]] %float_1 %float_0
// CHECK-NEXT:                 OpStore [[p_60]] [[f_61]]
  append_float.Append(rw_bool[0]);

// CHECK:       [[i_61:%\d+]] = OpLoad %uint {{%\d+}}
// CHECK-NEXT:  [[b_61:%\d+]] = OpINotEqual %bool [[i_61]] %uint_0
// CHECK-NEXT: [[bi_61:%\d+]] = OpSelect %float [[b_61]] %float_1 %float_0
// CHECK-NEXT:                  OpStore {{%\d+}} [[bi_61]]
  append_float.Append(rw_v2bool[0].x);

// CHECK:      [[matPtr:%\d+]] = OpAccessChain %_ptr_Uniform_mat2v2float %consume_float2x2 %uint_0 {{%\d+}}
// CHECK-NEXT:    [[mat:%\d+]] = OpLoad %mat2v2float [[matPtr]]
// CHECK-NEXT:   [[row0:%\d+]] = OpCompositeExtract %v2float [[mat]] 0
// CHECK-NEXT:   [[row1:%\d+]] = OpCompositeExtract %v2float [[mat]] 1
// CHECK-NEXT:   [[vec4:%\d+]] = OpVectorShuffle %v4float [[row0]] [[row1]] 0 1 2 3
// CHECK-NEXT:                   OpStore {{%\d+}} [[vec4]]
  append_v4float.Append(consume_float2x2.Consume());
}
