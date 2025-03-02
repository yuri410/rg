; RUN: opt -analyze -scalar-evolution < %s | FileCheck %s

define void @f(i1* %condition) {
; CHECK-LABEL: Classifying expressions for: @f
 entry: 
  br label %loop

 loop:
  %idx = phi i32 [ 0, %entry ], [ %idx.inc, %loop ]
  %idx.inc = add nsw i32 %idx, 1

  %idx.inc2 = add i32 %idx.inc, 1
  %idx.inc2.zext = zext i32 %idx.inc2 to i64

; CHECK: %idx.inc2.zext = zext i32 %idx.inc2 to i64
; CHECK-NEXT: -->  {2,+,1}<nuw><%loop>

  %c = load volatile i1, i1* %condition
  br i1 %c, label %loop, label %exit

 exit:
  ret void
}

define void @g(i1* %condition) {
; CHECK-LABEL: Classifying expressions for: @g
 entry:
  br label %loop

 loop:
  %idx = phi i32 [ 0, %entry ], [ %idx.inc, %loop ]
  %idx.inc = add nsw i32 %idx, 3

  %idx.inc2 = add i32 %idx.inc, -1
  %idx.inc2.sext = sext i32 %idx.inc2 to i64
; CHECK: %idx.inc2.sext = sext i32 %idx.inc2 to i64
; CHECK-NEXT: -->  {2,+,3}<nuw><nsw><%loop>

  %c = load volatile i1, i1* %condition
  br i1 %c, label %loop, label %exit

 exit:
  ret void
}
