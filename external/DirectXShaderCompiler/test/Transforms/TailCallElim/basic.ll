; RUN: opt < %s -tailcallelim -S | FileCheck %s

declare void @noarg()
declare void @use(i32*)
declare void @use_nocapture(i32* nocapture)
declare void @use2_nocapture(i32* nocapture, i32* nocapture)

; Trivial case. Mark @noarg with tail call.
define void @test0() {
; CHECK: tail call void @noarg()
	call void @noarg()
	ret void
}

; PR615. Make sure that we do not move the alloca so that it interferes with the tail call.
define i32 @test1() {
; CHECK: i32 @test1()
; CHECK-NEXT: alloca
	%A = alloca i32		; <i32*> [#uses=2]
	store i32 5, i32* %A
	call void @use(i32* %A)
; CHECK: tail call i32 @test1
	%X = tail call i32 @test1()		; <i32> [#uses=1]
	ret i32 %X
}

; This function contains intervening instructions which should be moved out of the way
define i32 @test2(i32 %X) {
; CHECK: i32 @test2
; CHECK-NOT: call
; CHECK: ret i32
entry:
	%tmp.1 = icmp eq i32 %X, 0		; <i1> [#uses=1]
	br i1 %tmp.1, label %then.0, label %endif.0
then.0:		; preds = %entry
	%tmp.4 = add i32 %X, 1		; <i32> [#uses=1]
	ret i32 %tmp.4
endif.0:		; preds = %entry
	%tmp.10 = add i32 %X, -1		; <i32> [#uses=1]
	%tmp.8 = call i32 @test2(i32 %tmp.10)		; <i32> [#uses=1]
	%DUMMY = add i32 %X, 1		; <i32> [#uses=0]
	ret i32 %tmp.8
}

; Though this case seems to be fairly unlikely to occur in the wild, someone
; plunked it into the demo script, so maybe they care about it.
define i32 @test3(i32 %c) {
; CHECK: i32 @test3
; CHECK-NOT: call
; CHECK: ret i32 0
entry:
	%tmp.1 = icmp eq i32 %c, 0		; <i1> [#uses=1]
	br i1 %tmp.1, label %return, label %else
else:		; preds = %entry
	%tmp.5 = add i32 %c, -1		; <i32> [#uses=1]
	%tmp.3 = call i32 @test3(i32 %tmp.5)		; <i32> [#uses=0]
	ret i32 0
return:		; preds = %entry
	ret i32 0
}

; Make sure that a nocapture pointer does not stop adding a tail call marker to
; an unrelated call and additionally that we do not mark the nocapture call with
; a tail call.
;
; rdar://14324281
define void @test4() {
; CHECK: void @test4
; CHECK-NOT: tail call void @use_nocapture
; CHECK: tail call void @noarg()
; CHECK: ret void
  %a = alloca i32
  call void @use_nocapture(i32* %a)
  call void @noarg()
  ret void
}

; Make sure that we do not perform TRE even with a nocapture use. This is due to
; bad codegen caused by PR962.
;
; rdar://14324281.
define i32* @test5(i32* nocapture %A, i1 %cond) {
; CHECK: i32* @test5
; CHECK-NOT: tailrecurse:
; CHECK: ret i32* null
  %B = alloca i32
  br i1 %cond, label %cond_true, label %cond_false
cond_true:
  call i32* @test5(i32* %B, i1 false)
  ret i32* null
cond_false:
  call void @use2_nocapture(i32* %A, i32* %B)
  call void @noarg()
  ret i32* null
}

; PR14143: Make sure that we do not mark functions with nocapture allocas with tail.
;
; rdar://14324281.
define void @test6(i32* %a, i32* %b) {
; CHECK-LABEL: @test6(
; CHECK-NOT: tail call
; CHECK: ret void
  %c = alloca [100 x i8], align 16
  %tmp = bitcast [100 x i8]* %c to i32*
  call void @use2_nocapture(i32* %b, i32* %tmp)
  ret void
}

; PR14143: Make sure that we do not mark functions with nocapture allocas with tail.
;
; rdar://14324281
define void @test7(i32* %a, i32* %b) nounwind uwtable {
entry:
; CHECK-LABEL: @test7(
; CHECK-NOT: tail call
; CHECK: ret void
  %c = alloca [100 x i8], align 16
  %0 = bitcast [100 x i8]* %c to i32*
  call void @use2_nocapture(i32* %0, i32* %a)
  call void @use2_nocapture(i32* %b, i32* %0)
  ret void
}

; If we have a mix of escaping captured/non-captured allocas, ensure that we do
; not do anything including marking callsites with the tail call marker.
;
; rdar://14324281.
define i32* @test8(i32* nocapture %A, i1 %cond) {
; CHECK: i32* @test8
; CHECK-NOT: tailrecurse:
; CHECK-NOT: tail call
; CHECK: ret i32* null
  %B = alloca i32
  %B2 = alloca i32
  br i1 %cond, label %cond_true, label %cond_false
cond_true:
  call void @use(i32* %B2)
  call i32* @test8(i32* %B, i1 false)
  ret i32* null
cond_false:
  call void @use2_nocapture(i32* %A, i32* %B)
  call void @noarg()
  ret i32* null
}

; Don't tail call if a byval arg is captured.
define void @test9(i32* byval %a) {
; CHECK-LABEL: define void @test9(
; CHECK: {{^ *}}call void @use(
  call void @use(i32* %a)
  ret void
}

%struct.X = type { i8* }

declare void @ctor(%struct.X*)
define void @test10(%struct.X* noalias sret %agg.result, i1 zeroext %b) {
; CHECK-LABEL @test10
entry:
  %x = alloca %struct.X, align 8
  br i1 %b, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @ctor(%struct.X* %agg.result)
; CHECK: tail call void @ctor
  br label %return

if.end:
  call void @ctor(%struct.X* %x)
; CHECK: call void @ctor
  br label %return

return:
  ret void
}

declare void @test11_helper1(i8** nocapture, i8*)
declare void @test11_helper2(i8*)
define void @test11() {
; CHECK-LABEL: @test11
; CHECK-NOT: tail
  %a = alloca i8*
  %b = alloca i8
  call void @test11_helper1(i8** %a, i8* %b)  ; a = &b
  %c = load i8*, i8** %a
  call void @test11_helper2(i8* %c)
; CHECK: call void @test11_helper2
  ret void
}
