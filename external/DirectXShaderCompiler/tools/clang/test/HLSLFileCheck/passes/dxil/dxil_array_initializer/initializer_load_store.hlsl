// RUN: %dxc -E -O3 -E main -T ps_6_0 %s | FileCheck %s
// CHECK: = internal unnamed_addr global [4 x i32] [i32 0, i32 2, i32 4, i32 6]
// CHECK: load i32
// CHECK: store i32
// CHECK: load i32
// CHECK-NOT: store i32
// CHECK-NOT: load i32

uint f(uint a) {
  return a * 2;
}

static uint GLOB[4] = {
  f(0),
  f(1),
  f(2),
  f(3),
};

[RootSignature("")]
float main(uint a : A) : SV_Target {
  uint result = GLOB[a];
  GLOB[3] = f(10);
  return result + GLOB[a];
}