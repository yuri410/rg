; This test checks that we are not instrumenting globals in llvm.metadata
; and other llvm internal globals.
; RUN: opt < %s -asan -asan-module -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.10.0"

@.str_noinst = private unnamed_addr constant [4 x i8] c"aaa\00", section "llvm.metadata"
@.str_noinst_prof = private unnamed_addr constant [4 x i8] c"aaa\00", section "__DATA,__llvm_covmap"
@.str_inst = private unnamed_addr constant [4 x i8] c"aaa\00"

; CHECK-NOT: {{asan_gen.*str_noinst}}
; CHECK-NOT: {{asan_gen.*str_noinst_prof}}
; CHECK: {{asan_gen.*str_inst}}
; CHECK: @asan.module_ctor
