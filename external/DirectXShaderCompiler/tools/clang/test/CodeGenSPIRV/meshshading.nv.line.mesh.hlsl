// Run: %dxc -T ms_6_5 -E main
// CHECK:  OpCapability MeshShadingNV
// CHECK:  OpExtension "SPV_NV_mesh_shader"
// CHECK:  OpEntryPoint MeshNV %main "main" %gl_GlobalInvocationID %gl_Position [[primind:%\d+]] [[primcount:%\d+]]
// CHECK:  OpExecutionMode %main LocalSize 128 1 1
// CHECK:  OpExecutionMode %main OutputLinesNV
// CHECK:  OpExecutionMode %main OutputVertices 256
// CHECK:  OpExecutionMode %main OutputPrimitivesNV 256

// CHECK:  OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
// CHECK:  OpDecorate %gl_Position BuiltIn Position
// CHECK:  OpDecorate [[primind]] BuiltIn PrimitiveIndicesNV
// CHECK:  OpDecorate [[primcount]] BuiltIn PrimitiveCountNV

// CHECK:  %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
// CHECK:  %gl_Position = OpVariable %_ptr_Output__arr_v4float_uint_256 Output
// CHECK:  [[primind]] = OpVariable %_ptr_Output__arr_uint_uint_512 Output
// CHECK:  [[primcount]] = OpVariable %_ptr_Output_uint Output

struct MeshPerVertex {
    float4 position : SV_Position;
};

#define NUM_THREADS 128
#define MAX_VERT 256
#define MAX_PRIM 256

[outputtopology("line")]
[numthreads(NUM_THREADS, 1, 1)]
void main(out vertices MeshPerVertex verts[MAX_VERT],
          out indices uint2 primitiveInd[MAX_PRIM],
          in uint tid : SV_DispatchThreadID)
{

// CHECK:  OpStore [[primcount]] %uint_256
    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);

// CHECK:  OpLoad %uint %tid
// CHECK:  OpAccessChain %_ptr_Output_v4float %gl_Position {{%\d+}}
// CHECK:  OpStore {{%\d+}} {{%\d+}}
    verts[tid].position = float4(4.0,5.0,6.0,7.0);

// CHECK:  OpIMul %uint %uint_6 %uint_2
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%\d+}}
// CHECK:  OpCompositeExtract %uint {{%\d+}} 0
// CHECK:  OpStore {{%\d+}} {{%\d+}}
// CHECK:  OpIAdd %uint {{%\d+}} %uint_1
// CHECK:  OpAccessChain %_ptr_Output_uint [[primind]] {{%\d+}}
// CHECK:  OpCompositeExtract %uint {{%\d+}} 1
// CHECK:  OpStore {{%\d+}} {{%\d+}}
    primitiveInd[6] = uint2(1,2);
}
