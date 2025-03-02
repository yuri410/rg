; RUN: opt < %s -instsimplify -S | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i64 @ptrdiff1(i8* %ptr) {
; CHECK-LABEL: @ptrdiff1(
; CHECK-NEXT: ret i64 42

  %first = getelementptr inbounds i8, i8* %ptr, i32 0
  %last = getelementptr inbounds i8, i8* %ptr, i32 42
  %first.int = ptrtoint i8* %first to i64
  %last.int = ptrtoint i8* %last to i64
  %diff = sub i64 %last.int, %first.int
  ret i64 %diff
}

define i64 @ptrdiff2(i8* %ptr) {
; CHECK-LABEL: @ptrdiff2(
; CHECK-NEXT: ret i64 42

  %first1 = getelementptr inbounds i8, i8* %ptr, i32 0
  %first2 = getelementptr inbounds i8, i8* %first1, i32 1
  %first3 = getelementptr inbounds i8, i8* %first2, i32 2
  %first4 = getelementptr inbounds i8, i8* %first3, i32 4
  %last1 = getelementptr inbounds i8, i8* %first2, i32 48
  %last2 = getelementptr inbounds i8, i8* %last1, i32 8
  %last3 = getelementptr inbounds i8, i8* %last2, i32 -4
  %last4 = getelementptr inbounds i8, i8* %last3, i32 -4
  %first.int = ptrtoint i8* %first4 to i64
  %last.int = ptrtoint i8* %last4 to i64
  %diff = sub i64 %last.int, %first.int
  ret i64 %diff
}

define i64 @ptrdiff3(i8* %ptr) {
; Don't bother with non-inbounds GEPs.
; CHECK-LABEL: @ptrdiff3(
; CHECK: getelementptr
; CHECK: sub
; CHECK: ret

  %first = getelementptr i8, i8* %ptr, i32 0
  %last = getelementptr i8, i8* %ptr, i32 42
  %first.int = ptrtoint i8* %first to i64
  %last.int = ptrtoint i8* %last to i64
  %diff = sub i64 %last.int, %first.int
  ret i64 %diff
}

define <4 x i32> @ptrdiff4(<4 x i8*> %arg) nounwind {
; Handle simple cases of vectors of pointers.
; CHECK-LABEL: @ptrdiff4(
; CHECK: ret <4 x i32> zeroinitializer
  %p1 = ptrtoint <4 x i8*> %arg to <4 x i32>
  %bc = bitcast <4 x i8*> %arg to <4 x i32*>
  %p2 = ptrtoint <4 x i32*> %bc to <4 x i32>
  %sub = sub <4 x i32> %p1, %p2
  ret <4 x i32> %sub
}

%struct.ham = type { i32, [2 x [2 x i32]] }

@global = internal global %struct.ham zeroinitializer, align 4

define i32 @ptrdiff5() nounwind {
bb:
  %tmp = getelementptr inbounds %struct.ham, %struct.ham* @global, i32 0, i32 1
  %tmp1 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %tmp, i32 0, i32 0
  %tmp2 = bitcast [2 x i32]* %tmp1 to i32*
  %tmp3 = ptrtoint i32* %tmp2 to i32
  %tmp4 = getelementptr inbounds %struct.ham, %struct.ham* @global, i32 0, i32 1
  %tmp5 = getelementptr inbounds [2 x [2 x i32]], [2 x [2 x i32]]* %tmp4, i32 0, i32 0
  %tmp6 = ptrtoint [2 x i32]* %tmp5 to i32
  %tmp7 = sub i32 %tmp3, %tmp6
  ret i32 %tmp7
; CHECK-LABEL: @ptrdiff5(
; CHECK: ret i32 0
}
