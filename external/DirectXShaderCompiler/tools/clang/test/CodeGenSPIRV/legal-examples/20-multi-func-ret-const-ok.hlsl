// Run: %dxc -T cs_6_0 -E main -O3

// CHECK:      [[src:%\d+]] = OpAccessChain %_ptr_Uniform_S %gSBuffer1
// CHECK-NEXT: [[val:%\d+]] = OpLoad %S [[src]]
// CHECK-NEXT: [[dst:%\d+]] = OpAccessChain %_ptr_Uniform_S %gRWSBuffer1
// CHECK-NEXT:                OpStore [[dst]] [[val]]
// CHECK:      [[src:%\d+]] = OpAccessChain %_ptr_Uniform_S %gSBuffer2
// CHECK-NEXT: [[val:%\d+]] = OpLoad %S [[src]]
// CHECK-NEXT: [[dst:%\d+]] = OpAccessChain %_ptr_Uniform_S %gRWSBuffer2
// CHECK-NEXT:                OpStore [[dst]] [[val]]

struct S {
  float4 f;
};

int i;

StructuredBuffer<S> gSBuffer1;
StructuredBuffer<S> gSBuffer2;
RWStructuredBuffer<S> gRWSBuffer1;
RWStructuredBuffer<S> gRWSBuffer2;

StructuredBuffer<S> foo(int l) {
  if (l == 0) {
    return gSBuffer1;
  } else {
    return gSBuffer2;
  }
}

void main() {
  gRWSBuffer1[i] = foo(0)[i];
  gRWSBuffer2[i] = foo(1)[i];
}
