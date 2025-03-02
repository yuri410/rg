// Run: %dxc -T gs_6_0 -E main

// CHECK: OpExecutionMode %main Triangles

struct S { float4 val : VAL; };

[maxvertexcount(3)]
void main(triangle in uint id[3] : VertexID, inout LineStream<S> outData) {
}
