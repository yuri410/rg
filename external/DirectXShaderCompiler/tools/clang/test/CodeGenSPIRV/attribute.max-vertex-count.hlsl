// Run: %dxc -T gs_6_0 -E main

// CHECK: OpExecutionMode %main OutputVertices 3

struct S { float4 val : VAL; };

[maxvertexcount(3)]
void main(point in uint id[1] : VertexID, inout LineStream<S> outData) {
}
