; RUN: opt -basicaa -functionattrs -S < %s | FileCheck %s

; Atomic load/store to local doesn't affect whether a function is
; readnone/readonly.
define i32 @test1(i32 %x) uwtable ssp {
; CHECK: define i32 @test1(i32 %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  store atomic i32 %x, i32* %x.addr seq_cst, align 4
  %r = load atomic i32, i32* %x.addr seq_cst, align 4
  ret i32 %r
}

; A function with an Acquire load is not readonly.
define i32 @test2(i32* %x) uwtable ssp {
; CHECK: define i32 @test2(i32* nocapture readonly %x) #1 {
entry:
  %r = load atomic i32, i32* %x seq_cst, align 4
  ret i32 %r
}

; CHECK: attributes #0 = { readnone ssp uwtable }
; CHECK: attributes #1 = { ssp uwtable }
