; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @Loop_Begin
; CHECK:         loop0
; CHECK:         @If_Begin
; CHECK:             dx.LoopExitHelper
; CHECK:             @Loop_Break
; CHECK:         @If_Else
; CHECK:             loop0.body0
; CHECK:             @If_Begin
; CHECK:                 dx.LoopExitHelper.1
; CHECK:                 @Loop_Break
; CHECK:             @If_Else
; CHECK:                 loop0.latch
; CHECK:                 dx.LoopContinue
; CHECK:                 @Loop_Continue
; CHECK:             @If_End
; CHECK:         @If_End
; CHECK:         dx.LoopLatch
; CHECK:     @Loop_End
; CHECK:     dx.LoopExit
; CHECK:     dx.LoopExitHelperIf
; CHECK:     @If_Begin
; CHECK:         loop0.exit
; CHECK:     @If_Else
; CHECK:         loop0.body1
; CHECK:         @Switch_Begin
; CHECK:         @Switch_Case
; CHECK:             switch0.default
; CHECK:             @Switch_Break
; CHECK:         @Switch_Case
; CHECK:             switch0.case0
; CHECK:             @Switch_Break
; CHECK:         @Switch_Case
; CHECK:             switch0.case1
; CHECK:             @Switch_Break
; CHECK:         @Switch_Case
; CHECK:             switch0.case2
; CHECK:             @Switch_Break
; CHECK:         @Switch_End
; CHECK:     @If_End
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  br label %loop0

loop0:
  %loop0.0 = load i32, i32 *%res
  %loop0.1 = icmp eq i32 %loop0.0, 20
  br i1 %loop0.1, label %loop0.exit, label %loop0.body0

loop0.body0:
  %cond2 = icmp eq i32 %i2, 10
  br i1 %cond2, label %loop0.body1, label %loop0.latch

loop0.body1:
;;--------------
  switch i32 %i1, label %switch0.default [ i32 0, label %switch0.case0
                                           i32 1, label %switch0.case1
                                           i32 12, label %switch0.case2
                                           i32 32, label %switch0.default
                                           i32 21, label %switch0.case1
                                           i32 33, label %switch0.default
                                           i32 11, label %switch0.case1
                                           i32 2, label %switch0.case2
                                         ]

switch0.case0:
  store i32 0, i32 *%res
  ret i32 0

switch0.case1:
  store i32 22, i32 *%res
  ret i32 43

switch0.case2:
  store i32 2, i32 *%res
  ret i32 2

switch0.default:
  store i32 -1, i32 *%res
  ret i32 -1
;;-----------
  
loop0.latch:
  store i32 %i2, i32 *%res
  br label %loop0

loop0.exit:
  %loop0.2 = load i32, i32 *%res
  %loop0.3 = add i32 %loop0.2, 1
  store i32 %loop0.3, i32 *%res
  ret i32 %loop0.3
}

