; RUN: llvm-link %s %p/2011-08-18-unique-debug-type2.ll -S -o - | FileCheck %s
; Test to check only one MDNode for "int" after linking.
; CHECK: !DIBasicType(name: "int"
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-apple-macosx10.7.0"

define i32 @foo() nounwind uwtable ssp {
entry:
  ret i32 1, !dbg !10
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!13}

!0 = !DICompileUnit(language: DW_LANG_C99, producer: "clang version 3.0 (trunk 137954)", isOptimized: true, emissionKind: 0, file: !12, enums: !2, retainedTypes: !2, subprograms: !3, globals: !2)
!1 = !{!2}
!2 = !{}
!3 = !{!5}
!5 = !DISubprogram(name: "foo", line: 1, isLocal: false, isDefinition: true, virtualIndex: 6, isOptimized: false, file: !12, scope: !6, type: !7, function: i32 ()* @foo)
!6 = !DIFile(filename: "one.c", directory: "/private/tmp")
!7 = !DISubroutineType(types: !8)
!8 = !{!9}
!9 = !DIBasicType(tag: DW_TAG_base_type, name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!10 = !DILocation(line: 1, column: 13, scope: !11)
!11 = distinct !DILexicalBlock(line: 1, column: 11, file: !12, scope: !5)
!12 = !DIFile(filename: "one.c", directory: "/private/tmp")
!13 = !{i32 1, !"Debug Info Version", i32 3}
