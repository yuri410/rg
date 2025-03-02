; RUN: opt -S -disable-output -passes=print-cg < %s 2>&1 | FileCheck %s

declare void @llvm.experimental.patchpoint.void(i64, i32, i8*, i32, ...)
declare i32 @llvm.experimental.gc.statepoint.p0f_isVoidf(i64, i32, void ()*, i32, i32, ...)

define private void @f() {
  ret void
}

define void @calls_statepoint(i8 addrspace(1)* %arg) gc "statepoint-example" {
; CHECK: Call edges in function: calls_statepoint
; CHECK-NEXT:  -> f
entry:
  %cast = bitcast i8 addrspace(1)* %arg to i64 addrspace(1)*
  %safepoint_token = call i32 (i64, i32, void ()*, i32, i32, ...) @llvm.experimental.gc.statepoint.p0f_isVoidf(i64 0, i32 0, void ()* @f, i32 0, i32 0, i32 0, i32 5, i32 0, i32 0, i32 0, i32 10, i32 0, i8 addrspace(1)* %arg, i64 addrspace(1)* %cast, i8 addrspace(1)* %arg, i8 addrspace(1)* %arg)
  ret void
}

define void @calls_patchpoint() {
; CHECK:  Call edges in function: calls_patchpoint
; CHECK-NEXT:    -> f
entry:
  %c = bitcast void()* @f to i8*
  tail call void (i64, i32, i8*, i32, ...) @llvm.experimental.patchpoint.void(i64 1, i32 15, i8* %c, i32 0, i16 65535, i16 -1, i32 65536, i32 2000000000, i32 2147483647, i32 -1, i32 4294967295, i32 4294967296, i64 2147483648, i64 4294967295, i64 4294967296, i64 -1)
  ret void
}
