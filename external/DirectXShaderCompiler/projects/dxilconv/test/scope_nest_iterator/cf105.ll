; RUN: %opt-exe %s -scopenested -scopenestinfo -analyze -S | %FileCheck %s
; CHECK: ScopeNestInfo:
; CHECK: @TopLevel_Begin
; CHECK:     entry
; CHECK:     @Switch_Begin
; CHECK:     @Switch_Case
; CHECK:         switch0.default
; CHECK:         dx.SwitchBreak
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case0
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case1
; CHECK:         @If_Begin
; CHECK:             if0.T
; CHECK:             @If_Begin
; CHECK:                 if1.T
; CHECK:                 dx.SwitchBreak.2
; CHECK:                 @Switch_Break
; CHECK:             @If_End
; CHECK:             dx.EndIfScope.1
; CHECK:             if1.F
; CHECK:         @If_Else
; CHECK:             if0.F
; CHECK:         @If_End
; CHECK:         dx.EndIfScope
; CHECK:         if0.end
; CHECK:         dx.SwitchBreak.3
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_Case
; CHECK:         switch0.case2
; CHECK:         dx.SwitchBreak.4
; CHECK:         @Switch_Break
; CHECK:         @Switch_Break
; CHECK:     @Switch_End
; CHECK:     dx.EndSwitchScope
; CHECK:     switch0.end
; CHECK: @TopLevel_End

define i32 @main(i1 %c1, i1 %c2, i1 %c3, i32 %i1, i32 %i2) {
entry:
  %res = alloca i32
  store i32 1, i32 *%res
  switch i32 %i1, label %switch0.default [ i32 0, label %switch0.case0
                                           i32 1, label %switch0.case1
                                           i32 11, label %switch0.case1
                                           i32 12, label %switch0.case2
                                           i32 2, label %switch0.case2 ]

switch0.case0:
  ret i32 33

switch0.case1:
  store i32 1, i32 *%res
  br i1 %c1, label %if0.T, label %if0.F

if0.T:
  store i32 23, i32 *%res
  br i1 %c2, label %if1.T, label %if1.F

if1.T:
  store i32 24, i32 *%res
  br label %switch0.end

if1.F:
  store i32 25, i32 *%res
  br label %if0.end

if0.F:
  store i32 21, i32 *%res
  br label %if0.end

if0.end:
  br label %switch0.end

switch0.case2:
  store i32 2, i32 *%res
  br label %switch0.end

switch0.default:
  store i32 -1, i32 *%res
  br label %switch0.end

switch0.end:
  %0 = load i32, i32 *%res
  ret i32 %0
}

