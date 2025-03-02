; RUN: llvm-link %s -S -o - | FileCheck %s

@g1 = global void()* @f2
; CHECK: @g1 = global void ()* @f2

@p1 = global i8 42
; CHECK: @p1 = global i8 42

@p2 = internal global i8 43
; CHECK: @p2 = internal global i8 43

define void @f1() prologue i8* @p1 {
  ret void
}
; CHECK: define void @f1() prologue i8* @p1 {

define internal void @f2() prologue i8* @p2 {
  ret void
}

; CHECK: define internal void @f2() prologue i8* @p2 {
