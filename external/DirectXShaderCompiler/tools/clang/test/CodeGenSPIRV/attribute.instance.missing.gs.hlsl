// Run: %dxc -T gs_6_0 -E main

// CHECK: OpExecutionMode %main Invocations 1

struct S { float4 val : VAL; };

[maxvertexcount(2)]
void main(point in uint id[1] : VertexID, inout LineStream<S> outData) {
}
