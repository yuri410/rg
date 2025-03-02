; RUN: opt -S -loop-vectorize -dce -instcombine -force-vector-width=2 -force-vector-interleave=1  < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

@A = common global [1024 x i32] zeroinitializer, align 16
@fA = common global [1024 x float] zeroinitializer, align 16
@dA = common global [1024 x double] zeroinitializer, align 16

; Signed tests.

; Turn this into a max reduction. Make sure we use a splat to initialize the
; vector for the reduction.
; CHECK-LABEL: @max_red(
; CHECK: %[[VAR:.*]] = insertelement <2 x i32> undef, i32 %max, i32 0
; CHECK: {{.*}} = shufflevector <2 x i32> %[[VAR]], <2 x i32> undef, <2 x i32> zeroinitializer
; CHECK: icmp sgt <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp sgt <2 x i32>
; CHECK: select i1

define i32 @max_red(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp sgt i32 %0, %max.red.08
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; Turn this into a max reduction. The select has its inputs reversed therefore
; this is a max reduction.
; CHECK-LABEL: @max_red_inverse_select(
; CHECK: icmp slt <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp sgt <2 x i32>
; CHECK: select i1

define i32 @max_red_inverse_select(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp slt i32 %max.red.08, %0
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; Turn this into a min reduction.
; CHECK-LABEL: @min_red(
; CHECK: icmp slt <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp slt <2 x i32>
; CHECK: select i1

define i32 @min_red(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp slt i32 %0, %max.red.08
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; Turn this into a min reduction. The select has its inputs reversed therefore
; this is a min reduction.
; CHECK-LABEL: @min_red_inverse_select(
; CHECK: icmp sgt <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp slt <2 x i32>
; CHECK: select i1

define i32 @min_red_inverse_select(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp sgt i32 %max.red.08, %0
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; Unsigned tests.

; Turn this into a max reduction.
; CHECK-LABEL: @umax_red(
; CHECK: icmp ugt <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp ugt <2 x i32>
; CHECK: select i1

define i32 @umax_red(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp ugt i32 %0, %max.red.08
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; Turn this into a max reduction. The select has its inputs reversed therefore
; this is a max reduction.
; CHECK-LABEL: @umax_red_inverse_select(
; CHECK: icmp ult <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp ugt <2 x i32>
; CHECK: select i1

define i32 @umax_red_inverse_select(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp ult i32 %max.red.08, %0
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; Turn this into a min reduction.
; CHECK-LABEL: @umin_red(
; CHECK: icmp ult <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp ult <2 x i32>
; CHECK: select i1

define i32 @umin_red(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp ult i32 %0, %max.red.08
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; Turn this into a min reduction. The select has its inputs reversed therefore
; this is a min reduction.
; CHECK-LABEL: @umin_red_inverse_select(
; CHECK: icmp ugt <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp ult <2 x i32>
; CHECK: select i1

define i32 @umin_red_inverse_select(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp ugt i32 %max.red.08, %0
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; SGE -> SLT
; Turn this into a min reduction (select inputs are reversed).
; CHECK-LABEL: @sge_min_red(
; CHECK: icmp sge <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp slt <2 x i32>
; CHECK: select i1

define i32 @sge_min_red(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp sge i32 %0, %max.red.08
  %max.red.0 = select i1 %cmp3, i32 %max.red.08, i32 %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; SLE -> SGT
; Turn this into a max reduction (select inputs are reversed).
; CHECK-LABEL: @sle_min_red(
; CHECK: icmp sle <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp sgt <2 x i32>
; CHECK: select i1

define i32 @sle_min_red(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp sle i32 %0, %max.red.08
  %max.red.0 = select i1 %cmp3, i32 %max.red.08, i32 %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; UGE -> ULT
; Turn this into a min reduction (select inputs are reversed).
; CHECK-LABEL: @uge_min_red(
; CHECK: icmp uge <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp ult <2 x i32>
; CHECK: select i1

define i32 @uge_min_red(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp uge i32 %0, %max.red.08
  %max.red.0 = select i1 %cmp3, i32 %max.red.08, i32 %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; ULE -> UGT
; Turn this into a max reduction (select inputs are reversed).
; CHECK-LABEL: @ule_min_red(
; CHECK: icmp ule <2 x i32>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: icmp ugt <2 x i32>
; CHECK: select i1

define i32 @ule_min_red(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp3 = icmp ule i32 %0, %max.red.08
  %max.red.0 = select i1 %cmp3, i32 %max.red.08, i32 %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; No reduction.
; CHECK-LABEL: @no_red_1(
; CHECK-NOT: icmp <2 x i32>
define i32 @no_red_1(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %arrayidx1 = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 1, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = load i32, i32* %arrayidx1, align 4
  %cmp3 = icmp sgt i32 %0, %1
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; CHECK-LABEL: @no_red_2(
; CHECK-NOT: icmp <2 x i32>
define i32 @no_red_2(i32 %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi i32 [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %indvars.iv
  %arrayidx1 = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 1, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = load i32, i32* %arrayidx1, align 4
  %cmp3 = icmp sgt i32 %0, %max.red.08
  %max.red.0 = select i1 %cmp3, i32 %0, i32 %1
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 %max.red.0
}

; Float tests.

; Maximum.

; Turn this into a max reduction in the presence of a no-nans-fp-math attribute.
; CHECK-LABEL: @max_red_float(
; CHECK: fcmp ogt <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp ogt <2 x float>
; CHECK: select i1

define float @max_red_float(float %max) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi float [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ogt float %0, %max.red.08
  %max.red.0 = select i1 %cmp3, float %0, float %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %max.red.0
}

; CHECK-LABEL: @max_red_float_ge(
; CHECK: fcmp oge <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp ogt <2 x float>
; CHECK: select i1

define float @max_red_float_ge(float %max) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi float [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp oge float %0, %max.red.08
  %max.red.0 = select i1 %cmp3, float %0, float %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %max.red.0
}

; CHECK-LABEL: @inverted_max_red_float(
; CHECK: fcmp olt <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp ogt <2 x float>
; CHECK: select i1

define float @inverted_max_red_float(float %max) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi float [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp olt float %0, %max.red.08
  %max.red.0 = select i1 %cmp3, float %max.red.08, float %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %max.red.0
}

; CHECK-LABEL: @inverted_max_red_float_le(
; CHECK: fcmp ole <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp ogt <2 x float>
; CHECK: select i1

define float @inverted_max_red_float_le(float %max) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi float [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ole float %0, %max.red.08
  %max.red.0 = select i1 %cmp3, float %max.red.08, float %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %max.red.0
}

; CHECK-LABEL: @unordered_max_red_float(
; CHECK: fcmp ole <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp ogt <2 x float>
; CHECK: select i1

define float @unordered_max_red_float(float %max) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi float [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ugt float %0, %max.red.08
  %max.red.0 = select i1 %cmp3, float %0, float %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %max.red.0
}

; CHECK-LABEL: @unordered_max_red_float_ge(
; CHECK: fcmp olt <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp ogt <2 x float>
; CHECK: select i1

define float @unordered_max_red_float_ge(float %max) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi float [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp uge float %0, %max.red.08
  %max.red.0 = select i1 %cmp3, float %0, float %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %max.red.0
}

; CHECK-LABEL: @inverted_unordered_max_red_float(
; CHECK: fcmp oge <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp ogt <2 x float>
; CHECK: select i1

define float @inverted_unordered_max_red_float(float %max) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi float [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ult float %0, %max.red.08
  %max.red.0 = select i1 %cmp3, float %max.red.08, float %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %max.red.0
}

; CHECK-LABEL: @inverted_unordered_max_red_float_le(
; CHECK: fcmp ogt <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp ogt <2 x float>
; CHECK: select i1

define float @inverted_unordered_max_red_float_le(float %max) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi float [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ule float %0, %max.red.08
  %max.red.0 = select i1 %cmp3, float %max.red.08, float %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %max.red.0
}

; Minimum.

; Turn this into a min reduction in the presence of a no-nans-fp-math attribute.
; CHECK-LABEL: @min_red_float(
; CHECK: fcmp olt <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp olt <2 x float>
; CHECK: select i1

define float @min_red_float(float %min) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.red.08 = phi float [ %min, %entry ], [ %min.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp olt float %0, %min.red.08
  %min.red.0 = select i1 %cmp3, float %0, float %min.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %min.red.0
}

; CHECK-LABEL: @min_red_float_le(
; CHECK: fcmp ole <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp olt <2 x float>
; CHECK: select i1

define float @min_red_float_le(float %min) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.red.08 = phi float [ %min, %entry ], [ %min.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ole float %0, %min.red.08
  %min.red.0 = select i1 %cmp3, float %0, float %min.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %min.red.0
}

; CHECK-LABEL: @inverted_min_red_float(
; CHECK: fcmp ogt <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp olt <2 x float>
; CHECK: select i1

define float @inverted_min_red_float(float %min) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.red.08 = phi float [ %min, %entry ], [ %min.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ogt float %0, %min.red.08
  %min.red.0 = select i1 %cmp3, float %min.red.08, float %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %min.red.0
}

; CHECK-LABEL: @inverted_min_red_float_ge(
; CHECK: fcmp oge <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp olt <2 x float>
; CHECK: select i1

define float @inverted_min_red_float_ge(float %min) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.red.08 = phi float [ %min, %entry ], [ %min.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp oge float %0, %min.red.08
  %min.red.0 = select i1 %cmp3, float %min.red.08, float %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %min.red.0
}

; CHECK-LABEL: @unordered_min_red_float(
; CHECK: fcmp oge <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp olt <2 x float>
; CHECK: select i1

define float @unordered_min_red_float(float %min) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.red.08 = phi float [ %min, %entry ], [ %min.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ult float %0, %min.red.08
  %min.red.0 = select i1 %cmp3, float %0, float %min.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %min.red.0
}

; CHECK-LABEL: @unordered_min_red_float_le(
; CHECK: fcmp ogt <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp olt <2 x float>
; CHECK: select i1

define float @unordered_min_red_float_le(float %min) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.red.08 = phi float [ %min, %entry ], [ %min.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ule float %0, %min.red.08
  %min.red.0 = select i1 %cmp3, float %0, float %min.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %min.red.0
}

; CHECK-LABEL: @inverted_unordered_min_red_float(
; CHECK: fcmp ole <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp olt <2 x float>
; CHECK: select i1

define float @inverted_unordered_min_red_float(float %min) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.red.08 = phi float [ %min, %entry ], [ %min.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ugt float %0, %min.red.08
  %min.red.0 = select i1 %cmp3, float %min.red.08, float %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %min.red.0
}

; CHECK-LABEL: @inverted_unordered_min_red_float_ge(
; CHECK: fcmp olt <2 x float>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp olt <2 x float>
; CHECK: select i1

define float @inverted_unordered_min_red_float_ge(float %min) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.red.08 = phi float [ %min, %entry ], [ %min.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp uge float %0, %min.red.08
  %min.red.0 = select i1 %cmp3, float %min.red.08, float %0
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %min.red.0
}

; Make sure we handle doubles, too.
; CHECK-LABEL: @min_red_double(
; CHECK: fcmp olt <2 x double>
; CHECK: select <2 x i1>
; CHECK: middle.block
; CHECK: fcmp olt <2 x double>
; CHECK: select i1

define double @min_red_double(double %min) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.red.08 = phi double [ %min, %entry ], [ %min.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x double], [1024 x double]* @dA, i64 0, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 4
  %cmp3 = fcmp olt double %0, %min.red.08
  %min.red.0 = select i1 %cmp3, double %0, double %min.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret double %min.red.0
}


; Don't this into a max reduction. The no-nans-fp-math attribute is missing
; CHECK-LABEL: @max_red_float_nans(
; CHECK-NOT: <2 x float>

define float @max_red_float_nans(float %max) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.red.08 = phi float [ %max, %entry ], [ %max.red.0, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @fA, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp3 = fcmp ogt float %0, %max.red.08
  %max.red.0 = select i1 %cmp3, float %0, float %max.red.08
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret float %max.red.0
}


attributes #0 = { "no-nans-fp-math"="true" }
