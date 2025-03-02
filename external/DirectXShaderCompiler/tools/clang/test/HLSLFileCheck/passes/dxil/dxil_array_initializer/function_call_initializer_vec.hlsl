// RUN: %dxc -E -O3 -E main -T ps_6_0 %s | FileCheck %s
// CHECK: = internal unnamed_addr constant [10 x float] [
// CHECK: = internal unnamed_addr constant [10 x float] [
// CHECK-NOT: store float

float2 f(float a) {
  return float2(a, a * 2);
}

static float2 GLOB[10] = {
  f(0),
  f(1),
  f(2),
  f(3),
  f(4),
  f(5),
  f(6),
  f(7),
  f(8),
  f(9),
};

[RootSignature("")]
float main(float a : A) : SV_Target {
  return GLOB[a].x + GLOB[a].y;
}