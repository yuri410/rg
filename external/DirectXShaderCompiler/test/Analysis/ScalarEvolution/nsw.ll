; RUN: opt < %s -analyze -scalar-evolution | FileCheck %s

; The addrecs in this loop are analyzable only by using nsw information.

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64"

; CHECK: Classifying expressions for: @test1
define void @test1(double* %p) nounwind {
entry:
	%tmp = load double, double* %p, align 8		; <double> [#uses=1]
	%tmp1 = fcmp ogt double %tmp, 2.000000e+00		; <i1> [#uses=1]
	br i1 %tmp1, label %bb.nph, label %return

bb.nph:		; preds = %entry
	br label %bb

bb:		; preds = %bb1, %bb.nph
	%i.01 = phi i32 [ %tmp8, %bb1 ], [ 0, %bb.nph ]		; <i32> [#uses=3]
; CHECK: %i.01
; CHECK-NEXT: -->  {0,+,1}<nuw><nsw><%bb>
	%tmp2 = sext i32 %i.01 to i64		; <i64> [#uses=1]
	%tmp3 = getelementptr double, double* %p, i64 %tmp2		; <double*> [#uses=1]
	%tmp4 = load double, double* %tmp3, align 8		; <double> [#uses=1]
	%tmp5 = fmul double %tmp4, 9.200000e+00		; <double> [#uses=1]
	%tmp6 = sext i32 %i.01 to i64		; <i64> [#uses=1]
	%tmp7 = getelementptr double, double* %p, i64 %tmp6		; <double*> [#uses=1]
; CHECK: %tmp7
; CHECK-NEXT:   -->  {%p,+,8}<%bb>
	store double %tmp5, double* %tmp7, align 8
	%tmp8 = add nsw i32 %i.01, 1		; <i32> [#uses=2]
; CHECK: %tmp8
; CHECK-NEXT: -->  {1,+,1}<nuw><nsw><%bb>
	br label %bb1

bb1:		; preds = %bb
	%phitmp = sext i32 %tmp8 to i64		; <i64> [#uses=1]
; CHECK: %phitmp
; CHECK-NEXT: -->  {1,+,1}<nuw><nsw><%bb>
	%tmp9 = getelementptr double, double* %p, i64 %phitmp		; <double*> [#uses=1]
; CHECK: %tmp9
; CHECK-NEXT:  -->  {(8 + %p),+,8}<%bb>
	%tmp10 = load double, double* %tmp9, align 8		; <double> [#uses=1]
	%tmp11 = fcmp ogt double %tmp10, 2.000000e+00		; <i1> [#uses=1]
	br i1 %tmp11, label %bb, label %bb1.return_crit_edge

bb1.return_crit_edge:		; preds = %bb1
	br label %return

return:		; preds = %bb1.return_crit_edge, %entry
	ret void
}

; CHECK: Classifying expressions for: @test2
define void @test2(i32* %begin, i32* %end) ssp {
entry:
  %cmp1.i.i = icmp eq i32* %begin, %end
  br i1 %cmp1.i.i, label %_ZSt4fillIPiiEvT_S1_RKT0_.exit, label %for.body.lr.ph.i.i

for.body.lr.ph.i.i:                               ; preds = %entry
  br label %for.body.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %for.body.lr.ph.i.i
  %__first.addr.02.i.i = phi i32* [ %begin, %for.body.lr.ph.i.i ], [ %ptrincdec.i.i, %for.body.i.i ]
; CHECK: %__first.addr.02.i.i
; CHECK-NEXT: -->  {%begin,+,4}<nuw><%for.body.i.i>
  store i32 0, i32* %__first.addr.02.i.i, align 4
  %ptrincdec.i.i = getelementptr inbounds i32, i32* %__first.addr.02.i.i, i64 1
; CHECK: %ptrincdec.i.i
; CHECK-NEXT: -->  {(4 + %begin),+,4}<nuw><%for.body.i.i>
  %cmp.i.i = icmp eq i32* %ptrincdec.i.i, %end
  br i1 %cmp.i.i, label %for.cond.for.end_crit_edge.i.i, label %for.body.i.i

for.cond.for.end_crit_edge.i.i:                   ; preds = %for.body.i.i
  br label %_ZSt4fillIPiiEvT_S1_RKT0_.exit

_ZSt4fillIPiiEvT_S1_RKT0_.exit:                   ; preds = %entry, %for.cond.for.end_crit_edge.i.i
  ret void
}

; Various checks for inbounds geps.
define void @test3(i32* %begin, i32* %end) nounwind ssp {
entry:
  %cmp7.i.i = icmp eq i32* %begin, %end
  br i1 %cmp7.i.i, label %_ZSt4fillIPiiEvT_S1_RKT0_.exit, label %for.body.i.i

for.body.i.i:                                     ; preds = %entry, %for.body.i.i
  %indvar.i.i = phi i64 [ %tmp, %for.body.i.i ], [ 0, %entry ]
; CHECK: %indvar.i.i
; CHECK: {0,+,1}<nuw><nsw><%for.body.i.i>
  %tmp = add nsw i64 %indvar.i.i, 1
; CHECK: %tmp =
; CHECK: {1,+,1}<nuw><nsw><%for.body.i.i>
  %ptrincdec.i.i = getelementptr inbounds i32, i32* %begin, i64 %tmp
; CHECK: %ptrincdec.i.i =
; CHECK: {(4 + %begin),+,4}<nsw><%for.body.i.i>
  %__first.addr.08.i.i = getelementptr inbounds i32, i32* %begin, i64 %indvar.i.i
; CHECK: %__first.addr.08.i.i
; CHECK: {%begin,+,4}<nsw><%for.body.i.i>
  store i32 0, i32* %__first.addr.08.i.i, align 4
  %cmp.i.i = icmp eq i32* %ptrincdec.i.i, %end
  br i1 %cmp.i.i, label %_ZSt4fillIPiiEvT_S1_RKT0_.exit, label %for.body.i.i
; CHECK: Loop %for.body.i.i: backedge-taken count is ((-4 + (-1 * %begin) + %end) /u 4)
; CHECK: Loop %for.body.i.i: max backedge-taken count is ((-4 + (-1 * %begin) + %end) /u 4)
_ZSt4fillIPiiEvT_S1_RKT0_.exit:                   ; preds = %for.body.i.i, %entry
  ret void
}

; A single AddExpr exists for (%a + %b), which is not always <nsw>.
; CHECK: @addnsw
; CHECK-NOT: --> (%a + %b)<nsw>
define i32 @addnsw(i32 %a, i32 %b) nounwind ssp {
entry:
  %tmp = add i32 %a, %b
  %cmp = icmp sgt i32 %tmp, 0
  br i1 %cmp, label %greater, label %exit

greater:
  %tmp2 = add nsw i32 %a, %b
  br label %exit

exit:
  %result = phi i32 [ %a, %entry ], [ %tmp2, %greater ]
  ret i32 %result
}

; CHECK-LABEL: PR12375
; CHECK: -->  {(4 + %arg),+,4}<nuw><%bb1>{{ U: [^ ]+ S: [^ ]+}}{{ *}}Exits: (8 + %arg)<nsw>
define i32 @PR12375(i32* readnone %arg) {
bb:
  %tmp = getelementptr inbounds i32, i32* %arg, i64 2
  br label %bb1

bb1:                                              ; preds = %bb1, %bb
  %tmp2 = phi i32* [ %arg, %bb ], [ %tmp5, %bb1 ]
  %tmp3 = phi i32 [ 0, %bb ], [ %tmp4, %bb1 ]
  %tmp4 = add nsw i32 %tmp3, 1
  %tmp5 = getelementptr inbounds i32, i32* %tmp2, i64 1
  %tmp6 = icmp ult i32* %tmp5, %tmp
  br i1 %tmp6, label %bb1, label %bb7

bb7:                                              ; preds = %bb1
  ret i32 %tmp4
}

; CHECK-LABEL: PR12376
; CHECK: -->  {(4 + %arg),+,4}<nuw><%bb2>{{ U: [^ ]+ S: [^ ]+}}{{ *}}Exits: (4 + (4 * ((3 + (-1 * %arg) + (%arg umax %arg1)) /u 4)) + %arg)
define void @PR12376(i32* nocapture %arg, i32* nocapture %arg1)  {
bb:
  br label %bb2

bb2:                                              ; preds = %bb2, %bb
  %tmp = phi i32* [ %arg, %bb ], [ %tmp4, %bb2 ]
  %tmp3 = icmp ult i32* %tmp, %arg1
  %tmp4 = getelementptr inbounds i32, i32* %tmp, i64 1
  br i1 %tmp3, label %bb2, label %bb5

bb5:                                              ; preds = %bb2
  ret void
}

declare void @f(i32)

; CHECK-LABEL: nswnowrap
; CHECK: --> {(1 + %v),+,1}<nsw><%for.body>{{ U: [^ ]+ S: [^ ]+}}{{ *}}Exits: (2 + %v)
define void @nswnowrap(i32 %v) {
entry:
  %add = add nsw i32 %v, 1
  br label %for.body

for.body:
  %i.04 = phi i32 [ %v, %entry ], [ %inc, %for.body ]
  %inc = add nsw i32 %i.04, 1
  tail call void @f(i32 %i.04)
  %cmp = icmp slt i32 %i.04, %add
  br i1 %cmp, label %for.body, label %for.end

for.end:
  ret void
}
