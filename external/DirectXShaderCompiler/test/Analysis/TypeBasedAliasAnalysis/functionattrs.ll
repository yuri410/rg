; RUN: opt < %s -tbaa -basicaa -functionattrs -S | FileCheck %s

; FunctionAttrs should make use of TBAA.

; Add the readnone attribute, since the only access is a store which TBAA
; says is to constant memory.
;
; It's unusual to see a store to constant memory, but it isn't necessarily
; invalid, as it's possible that this only happens after optimization on a
; code path which isn't ever executed.

; CHECK: define void @test0_yes(i32* nocapture %p) #0 {
define void @test0_yes(i32* %p) nounwind {
  store i32 0, i32* %p, !tbaa !1
  ret void
}

; CHECK: define void @test0_no(i32* nocapture %p) #1 {
define void @test0_no(i32* %p) nounwind {
  store i32 0, i32* %p, !tbaa !2
  ret void
}

; Add the readonly attribute, since there's just a call to a function which 
; TBAA says doesn't modify any memory.

; CHECK: define void @test1_yes(i32* nocapture %p) #2 {
define void @test1_yes(i32* %p) nounwind {
  call void @callee(i32* %p), !tbaa !1
  ret void
}

; CHECK: define void @test1_no(i32* %p) #1 {
define void @test1_no(i32* %p) nounwind {
  call void @callee(i32* %p), !tbaa !2
  ret void
}

; Add the readonly attribute, as above, but this time BasicAA will say
; that the function accesses memory through its arguments, which TBAA
; still says that the function doesn't write to memory.
;
; This is unusual, since the function is memcpy, but as above, this
; isn't necessarily invalid.

; CHECK: define void @test2_yes(i8* nocapture %p, i8* nocapture %q, i64 %n) #0 {
define void @test2_yes(i8* %p, i8* %q, i64 %n) nounwind {
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p, i8* %q, i64 %n, i32 1, i1 false), !tbaa !1
  ret void
}

; CHECK: define void @test2_no(i8* nocapture %p, i8* nocapture readonly %q, i64 %n) #1 {
define void @test2_no(i8* %p, i8* %q, i64 %n) nounwind {
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p, i8* %q, i64 %n, i32 1, i1 false), !tbaa !2
  ret void
}

; Similar to the others, va_arg only accesses memory through its operand.

; CHECK: define i32 @test3_yes(i8* nocapture %p) #0 {
define i32 @test3_yes(i8* %p) nounwind {
  %t = va_arg i8* %p, i32, !tbaa !1
  ret i32 %t
}

; CHECK: define i32 @test3_no(i8* nocapture %p) #1 {
define i32 @test3_no(i8* %p) nounwind {
  %t = va_arg i8* %p, i32, !tbaa !2
  ret i32 %t
}

declare void @callee(i32* %p) nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i32, i1) nounwind

; CHECK: attributes #0 = { nounwind readnone }
; CHECK: attributes #1 = { nounwind }
; CHECK: attributes #2 = { nounwind readonly }

; Root note.
!0 = !{ }

; Invariant memory.
!1 = !{!3, !3, i64 0, i1 1 }
; Not invariant memory.
!2 = !{!3, !3, i64 0, i1 0 }
!3 = !{ !"foo", !0 }
