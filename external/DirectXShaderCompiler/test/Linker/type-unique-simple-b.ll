; RUN: true

; ModuleID = 'bar.cpp'

%struct.Base = type { i32 }

; Function Attrs: nounwind ssp uwtable
define void @_Z1gi(i32 %a) #0 {
entry:
  %a.addr = alloca i32, align 4
  %t = alloca %struct.Base, align 4
  store i32 %a, i32* %a.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %a.addr, metadata !18, metadata !DIExpression()), !dbg !19
  call void @llvm.dbg.declare(metadata %struct.Base* %t, metadata !20, metadata !DIExpression()), !dbg !21
  ret void, !dbg !22
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: ssp uwtable
define i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  call void @_Z1fi(i32 0), !dbg !23
  call void @_Z1gi(i32 1), !dbg !24
  ret i32 0, !dbg !25
}

declare void @_Z1fi(i32) #3

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!17, !26}

!0 = !DICompileUnit(language: DW_LANG_C_plus_plus, producer: "clang version 3.4 (http://llvm.org/git/clang.git c23b1db6268c8e7ce64026d57d1510c1aac200a0) (http://llvm.org/git/llvm.git 09b98fe3978eddefc2145adc1056cf21580ce945)", isOptimized: false, emissionKind: 0, file: !1, enums: !2, retainedTypes: !3, subprograms: !9, globals: !2, imports: !2)
!1 = !DIFile(filename: "bar.cpp", directory: "/Users/mren/c_testing/type_unique_air/simple")
!2 = !{}
!3 = !{!4}
!4 = !DICompositeType(tag: DW_TAG_structure_type, name: "Base", line: 1, size: 32, align: 32, file: !5, elements: !6, identifier: "_ZTS4Base")
!5 = !DIFile(filename: "./a.hpp", directory: "/Users/mren/c_testing/type_unique_air/simple")
!6 = !{!7}
!7 = !DIDerivedType(tag: DW_TAG_member, name: "a", line: 2, size: 32, align: 32, file: !5, scope: !"_ZTS4Base", baseType: !8)
!8 = !DIBasicType(tag: DW_TAG_base_type, name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!9 = !{!10, !14}
!10 = !DISubprogram(name: "g", linkageName: "_Z1gi", line: 4, isLocal: false, isDefinition: true, virtualIndex: 6, flags: DIFlagPrototyped, isOptimized: false, scopeLine: 4, file: !1, scope: !11, type: !12, function: void (i32)* @_Z1gi, variables: !2)
!11 = !DIFile(filename: "bar.cpp", directory: "/Users/mren/c_testing/type_unique_air/simple")
!12 = !DISubroutineType(types: !13)
!13 = !{null, !8}
!14 = !DISubprogram(name: "main", line: 7, isLocal: false, isDefinition: true, virtualIndex: 6, flags: DIFlagPrototyped, isOptimized: false, scopeLine: 7, file: !1, scope: !11, type: !15, function: i32 ()* @main, variables: !2)
!15 = !DISubroutineType(types: !16)
!16 = !{!8}
!17 = !{i32 2, !"Dwarf Version", i32 2}
!18 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "a", line: 4, arg: 1, scope: !10, file: !11, type: !8)
!19 = !DILocation(line: 4, scope: !10)
!20 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "t", line: 5, scope: !10, file: !11, type: !4)
!21 = !DILocation(line: 5, scope: !10)
!22 = !DILocation(line: 6, scope: !10)
!23 = !DILocation(line: 8, scope: !14)
!24 = !DILocation(line: 9, scope: !14)
!25 = !DILocation(line: 10, scope: !14)
!26 = !{i32 1, !"Debug Info Version", i32 3}
