; RUN: opt < %s -asan -asan-module -asan-use-after-return=0 -S | FileCheck %s

; Checks that llvm.dbg.declare instructions are updated 
; accordingly as we merge allocas.

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @_Z3zzzi(i32 %p) nounwind uwtable sanitize_address {
entry:
  %p.addr = alloca i32, align 4
  %r = alloca i32, align 4
  store volatile i32 %p, i32* %p.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %p.addr, metadata !10, metadata !DIExpression()), !dbg !11
  call void @llvm.dbg.declare(metadata i32* %r, metadata !12, metadata !DIExpression()), !dbg !14
  %0 = load i32, i32* %p.addr, align 4, !dbg !14
  %add = add nsw i32 %0, 1, !dbg !14
  store volatile i32 %add, i32* %r, align 4, !dbg !14
  %1 = load i32, i32* %r, align 4, !dbg !15
  ret i32 %1, !dbg !15
}

;   CHECK: define i32 @_Z3zzzi
;   CHECK: entry:
; Verify that llvm.dbg.declare calls are in the entry basic block.
;   CHECK-NOT: %entry
;   CHECK: call void @llvm.dbg.declare(metadata {{.*}}, metadata ![[ARG_ID:[0-9]+]], metadata ![[OPDEREF:[0-9]+]])
;   CHECK-NOT: %entry
;   CHECK: call void @llvm.dbg.declare(metadata {{.*}}, metadata ![[VAR_ID:[0-9]+]], metadata ![[OPDEREF:[0-9]+]])

declare void @llvm.dbg.declare(metadata, metadata, metadata) nounwind readnone

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!17}

!0 = !DICompileUnit(language: DW_LANG_C_plus_plus, producer: "clang version 3.3 (trunk 169314)", isOptimized: true, emissionKind: 0, file: !16, enums: !1, retainedTypes: !1, subprograms: !3, globals: !1)
!1 = !{}
!3 = !{!5}
!5 = !DISubprogram(name: "zzz", linkageName: "_Z3zzzi", line: 1, isLocal: false, isDefinition: true, virtualIndex: 6, flags: DIFlagPrototyped, isOptimized: false, scopeLine: 1, file: !16, scope: !6, type: !7, function: i32 (i32)* @_Z3zzzi, variables: !1)
!6 = !DIFile(filename: "a.cc", directory: "/usr/local/google/llvm_cmake_clang/tmp/debuginfo")
!7 = !DISubroutineType(types: !8)
!8 = !{!9, !9}
!9 = !DIBasicType(tag: DW_TAG_base_type, name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!10 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "p", line: 1, arg: 1, scope: !5, file: !6, type: !9)
!11 = !DILocation(line: 1, scope: !5)
!12 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "r", line: 2, scope: !13, file: !6, type: !9)

; Verify that debug descriptors for argument and local variable will be replaced
; with descriptors that end with OpDeref (encoded as 2).
;   CHECK: ![[ARG_ID]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "p", arg: 1,{{.*}} line: 1
;   CHECK: ![[OPDEREF]] = !DIExpression(DW_OP_deref)
;   CHECK: ![[VAR_ID]] = !DILocalVariable(tag: DW_TAG_auto_variable, name: "r",{{.*}} line: 2
; Verify that there are no more variable descriptors.
;   CHECK-NOT: !DILocalVariable(tag: DW_TAG_arg_variable
;   CHECK-NOT: !DILocalVariable(tag: DW_TAG_auto_variable


!13 = distinct !DILexicalBlock(line: 1, column: 0, file: !16, scope: !5)
!14 = !DILocation(line: 2, scope: !13)
!15 = !DILocation(line: 3, scope: !13)
!16 = !DIFile(filename: "a.cc", directory: "/usr/local/google/llvm_cmake_clang/tmp/debuginfo")
!17 = !{i32 1, !"Debug Info Version", i32 3}
