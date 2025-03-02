// Run: %dxc -T cs_6_0 -E main

// CHECK: OpEntryPoint GLCompute %main "main" %gl_LocalInvocationID
// CHECK: OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
// CHECK: %gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input

// CHECK: [[gl_LocalInvocationID:%\d+]] = OpLoad %v3uint %gl_LocalInvocationID
// CHECK:   [[uint_GroupThreadID:%\d+]] = OpCompositeExtract %uint [[gl_LocalInvocationID]] 0
// CHECK:                                 OpStore %param_var_gtid [[uint_GroupThreadID]]

RWBuffer<uint> MyBuffer;

[numthreads(1, 1, 1)]
void main(uint gtid : SV_GroupThreadID) {
    MyBuffer[0] = gtid;
}
