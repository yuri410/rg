// Run: %dxc -T vs_6_0 -E main -fspv-reflect

// CHECK: OpExtension "SPV_GOOGLE_hlsl_functionality1"

// CHECK: OpName %type_AppendStructuredBuffer_v4float "type.AppendStructuredBuffer.v4float"
// CHECK: OpName %buffer1 "buffer1"

// CHECK: OpName %type_ACSBuffer_counter "type.ACSBuffer.counter"
// CHECK: OpName %counter_var_buffer1 "counter.var.buffer1"

// CHECK: OpName %type_AppendStructuredBuffer_S "type.AppendStructuredBuffer.S"
// CHECK: OpName %buffer2 "buffer2"

// CHECK: OpName %counter_var_buffer2 "counter.var.buffer2"

// CHECK: OpDecorateId %buffer1 CounterBuffer %counter_var_buffer1
// CHECK: OpDecorateId %buffer2 CounterBuffer %counter_var_buffer2

// CHECK: %type_AppendStructuredBuffer_v4float = OpTypeStruct %_runtimearr_v4float
// CHECK: %_ptr_Uniform_type_AppendStructuredBuffer_v4float = OpTypePointer Uniform %type_AppendStructuredBuffer_v4float

// CHECK: %type_ACSBuffer_counter = OpTypeStruct %int
// CHECK: %_ptr_Uniform_type_ACSBuffer_counter = OpTypePointer Uniform %type_ACSBuffer_counter

// CHECK: %type_AppendStructuredBuffer_S = OpTypeStruct %_runtimearr_S
// CHECK: %_ptr_Uniform_type_AppendStructuredBuffer_S = OpTypePointer Uniform %type_AppendStructuredBuffer_S

// CHECK: %buffer1 = OpVariable %_ptr_Uniform_type_AppendStructuredBuffer_v4float Uniform
// CHECK: %counter_var_buffer1 = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
AppendStructuredBuffer<float4> buffer1;

struct S {
    float a;
    float3 b;
    float2x3 c;
};

// CHECK: %buffer2 = OpVariable %_ptr_Uniform_type_AppendStructuredBuffer_S Uniform
// CHECK: %counter_var_buffer2 = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
AppendStructuredBuffer<S> buffer2;

float main() : A {
    return 1.0;
}
