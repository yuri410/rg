// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Tests that non-entry point functions can return arrays.

typedef int A[2];
A getA()
{
  A a = { 1, 2 };
  return a;
}

int2 main() : OUT
{
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 2)
  return (int2)getA();
}