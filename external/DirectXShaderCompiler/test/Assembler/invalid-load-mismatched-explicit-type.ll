; RUN: not llvm-as < %s 2>&1 | FileCheck %s
; CHECK: <stdin>:4:13: error: explicit pointee type doesn't match operand's pointee type
define void @test(i32* %t) {
  %x = load i16, i32* %t
  ret void
}
