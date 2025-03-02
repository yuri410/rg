; RUN: opt -S -deadargelim < %s | FileCheck %s

@.str = private constant [1 x i8] zeroinitializer, align 1 ; <[1 x i8]*> [#uses=1]

define i8* @vfs_addname(i8* %name, i32 %len, i32 %hash, i32 %flags) nounwind ssp {
entry:
  call void @llvm.dbg.value(metadata i8* %name, i64 0, metadata !0, metadata !DIExpression()), !dbg !DILocation(scope: !1)
  call void @llvm.dbg.value(metadata i32 %len, i64 0, metadata !10, metadata !DIExpression()), !dbg !DILocation(scope: !1)
  call void @llvm.dbg.value(metadata i32 %hash, i64 0, metadata !11, metadata !DIExpression()), !dbg !DILocation(scope: !1)
  call void @llvm.dbg.value(metadata i32 %flags, i64 0, metadata !12, metadata !DIExpression()), !dbg !DILocation(scope: !1)
; CHECK:  call fastcc i8* @add_name_internal(i8* %name, i32 %hash) [[NUW:#[0-9]+]], !dbg !{{[0-9]+}}
  %0 = call fastcc i8* @add_name_internal(i8* %name, i32 %len, i32 %hash, i8 zeroext 0, i32 %flags) nounwind, !dbg !13 ; <i8*> [#uses=1]
  ret i8* %0, !dbg !13
}

declare void @llvm.dbg.declare(metadata, metadata, metadata) nounwind readnone

define internal fastcc i8* @add_name_internal(i8* %name, i32 %len, i32 %hash, i8 zeroext %extra, i32 %flags) noinline nounwind ssp {
entry:
  call void @llvm.dbg.value(metadata i8* %name, i64 0, metadata !15, metadata !DIExpression()), !dbg !DILocation(scope: !16)
  call void @llvm.dbg.value(metadata i32 %len, i64 0, metadata !20, metadata !DIExpression()), !dbg !DILocation(scope: !16)
  call void @llvm.dbg.value(metadata i32 %hash, i64 0, metadata !21, metadata !DIExpression()), !dbg !DILocation(scope: !16)
  call void @llvm.dbg.value(metadata i8 %extra, i64 0, metadata !22, metadata !DIExpression()), !dbg !DILocation(scope: !16)
  call void @llvm.dbg.value(metadata i32 %flags, i64 0, metadata !23, metadata !DIExpression()), !dbg !DILocation(scope: !16)
  %0 = icmp eq i32 %hash, 0, !dbg !24             ; <i1> [#uses=1]
  br i1 %0, label %bb, label %bb1, !dbg !24

bb:                                               ; preds = %entry
  br label %bb2, !dbg !26

bb1:                                              ; preds = %entry
  br label %bb2, !dbg !27

bb2:                                              ; preds = %bb1, %bb
  %.0 = phi i8* [ getelementptr inbounds ([1 x i8], [1 x i8]* @.str, i64 0, i64 0), %bb ], [ %name, %bb1 ] ; <i8*> [#uses=1]
  ret i8* %.0, !dbg !27
}

declare void @llvm.dbg.value(metadata, i64, metadata, metadata) nounwind readnone

; CHECK: attributes #0 = { nounwind ssp }
; CHECK: attributes #1 = { nounwind readnone }
; CHECK: attributes #2 = { noinline nounwind ssp }
; CHECK: attributes [[NUW]] = { nounwind }

!llvm.dbg.cu = !{!3}
!llvm.module.flags = !{!30}
!0 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "name", line: 8, arg: 0, scope: !1, file: !2, type: !6)
!1 = !DISubprogram(name: "vfs_addname", linkageName: "vfs_addname", line: 12, isLocal: false, isDefinition: true, virtualIndex: 6, isOptimized: false, file: !28, scope: !2, type: !4)
!2 = !DIFile(filename: "tail.c", directory: "/Users/echeng/LLVM/radars/r7927803/")
!3 = !DICompileUnit(language: DW_LANG_C89, producer: "4.2.1 (Based on Apple Inc. build 5658) (LLVM build 9999)", isOptimized: true, emissionKind: 0, file: !28, enums: !29, retainedTypes: !29)
!4 = !DISubroutineType(types: !5)
!5 = !{!6, !6, !9, !9, !9}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, size: 64, align: 64, file: !28, scope: !2, baseType: !7)
!7 = !DIDerivedType(tag: DW_TAG_const_type, size: 8, align: 8, file: !28, scope: !2, baseType: !8)
!8 = !DIBasicType(tag: DW_TAG_base_type, name: "char", size: 8, align: 8, encoding: DW_ATE_signed_char)
!9 = !DIBasicType(tag: DW_TAG_base_type, name: "unsigned int", size: 32, align: 32, encoding: DW_ATE_unsigned)
!10 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "len", line: 9, arg: 0, scope: !1, file: !2, type: !9)
!11 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "hash", line: 10, arg: 0, scope: !1, file: !2, type: !9)
!12 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "flags", line: 11, arg: 0, scope: !1, file: !2, type: !9)
!13 = !DILocation(line: 13, scope: !14)
!14 = distinct !DILexicalBlock(line: 12, column: 0, file: !28, scope: !1)
!15 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "name", line: 17, arg: 0, scope: !16, file: !2, type: !6)
!16 = !DISubprogram(name: "add_name_internal", linkageName: "add_name_internal", line: 22, isLocal: true, isDefinition: true, virtualIndex: 6, isOptimized: false, file: !28, scope: !2, type: !17)
!17 = !DISubroutineType(types: !18)
!18 = !{!6, !6, !9, !9, !19, !9}
!19 = !DIBasicType(tag: DW_TAG_base_type, name: "unsigned char", size: 8, align: 8, encoding: DW_ATE_unsigned_char)
!20 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "len", line: 18, arg: 0, scope: !16, file: !2, type: !9)
!21 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "hash", line: 19, arg: 0, scope: !16, file: !2, type: !9)
!22 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "extra", line: 20, arg: 0, scope: !16, file: !2, type: !19)
!23 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "flags", line: 21, arg: 0, scope: !16, file: !2, type: !9)
!24 = !DILocation(line: 23, scope: !25)
!25 = distinct !DILexicalBlock(line: 22, column: 0, file: !28, scope: !16)
!26 = !DILocation(line: 24, scope: !25)
!27 = !DILocation(line: 26, scope: !25)
!28 = !DIFile(filename: "tail.c", directory: "/Users/echeng/LLVM/radars/r7927803/")
!29 = !{}
!30 = !{i32 1, !"Debug Info Version", i32 3}
