; RUN: opt -basicaa -gvn -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64"

; CHECK-LABEL: @foo(
; CHECK: entry.end_crit_edge:
; CHECK:   %j.phi.trans.insert = sext i32 %x to i64, !dbg [[J_LOC:![0-9]+]]
; CHECK:   %q.phi.trans.insert = getelementptr {{.*}}, !dbg [[Q_LOC:![0-9]+]]
; CHECK:   %n.pre = load i32, i32* %q.phi.trans.insert, !dbg [[N_LOC:![0-9]+]]
; CHECK: then:
; CHECK:   store i32 %z
; CHECK: end:
; CHECK:   %n = phi i32 [ %n.pre, %entry.end_crit_edge ], [ %z, %then ], !dbg [[N_LOC]]
; CHECK:   ret i32 %n

; CHECK-DAG: [[J_LOC]] = !DILocation(line: 45, column: 1, scope: !{{.*}})
; CHECK-DAG: [[Q_LOC]] = !DILocation(line: 46, column: 1, scope: !{{.*}})
; CHECK-DAG: [[N_LOC]] = !DILocation(line: 47, column: 1, scope: !{{.*}})

@G = external global [100 x i32]
define i32 @foo(i32 %x, i32 %z) {
entry:
  %tobool = icmp eq i32 %z, 0, !dbg !7
  br i1 %tobool, label %end, label %then, !dbg !7

then:
  %i = sext i32 %x to i64, !dbg !8
  %p = getelementptr [100 x i32], [100 x i32]* @G, i64 0, i64 %i, !dbg !8
  store i32 %z, i32* %p, !dbg !8
  br label %end, !dbg !8

end:
  %j = sext i32 %x to i64, !dbg !9
  %q = getelementptr [100 x i32], [100 x i32]* @G, i64 0, i64 %j, !dbg !10
  %n = load i32, i32* %q, !dbg !11
  ret i32 %n, !dbg !11
}

!llvm.module.flags = !{!0, !1, !2}
!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = !{i32 1, !"PIC Level", i32 2}

!3 = !{}
!4 = !DISubroutineType(types: !3)
!5 = !DIFile(filename: "a.cc", directory: "/tmp")
!6 = !DISubprogram(name: "foo", scope: !5, file: !5, line: 42, type: !4, isLocal: false, isDefinition: true, scopeLine: 43, flags: DIFlagPrototyped, isOptimized: false, function: i32 (i32, i32)* @foo, variables: !3)
!7 = !DILocation(line: 43, column: 1, scope: !6)
!8 = !DILocation(line: 44, column: 1, scope: !6)
!9 = !DILocation(line: 45, column: 1, scope: !6)
!10 = !DILocation(line: 46, column: 1, scope: !6)
!11 = !DILocation(line: 47, column: 1, scope: !6)
