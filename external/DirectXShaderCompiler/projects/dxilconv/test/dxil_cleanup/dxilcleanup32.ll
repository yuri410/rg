; RUN: %opt-exe %s -S -dxil-cleanup -verify -o %t.ll.converted
; RUN: fc %t.ll.converted %b.ref


%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%dx.types.i8x16 = type { [16 x i8] }

@dx.v32.x0 = internal global [32 x float] undef

define internal void @dx.label.0() {
entry:
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)
  %1 = call float @dx.op.bitcastI32toF32(i32 126, i32 1082130432)
  store float %1, float* getelementptr inbounds ([32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 8), align 4
  %2 = call float @dx.op.bitcastI32toF32(i32 126, i32 1082130432)
  store float %2, float* getelementptr inbounds ([32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 9), align 4
  %3 = call float @dx.op.bitcastI32toF32(i32 126, i32 1082130432)
  store float %3, float* getelementptr inbounds ([32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 10), align 4
  %4 = call float @dx.op.bitcastI32toF32(i32 126, i32 1082130432)
  store float %4, float* getelementptr inbounds ([32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 11), align 4
  ret void
}

define internal void @dx.label.1() {
entry:
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)
  %1 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  store float %1, float* getelementptr inbounds ([32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 8), align 4
  store float %1, float* getelementptr inbounds ([32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 9), align 4
  store float %1, float* getelementptr inbounds ([32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 10), align 4
  store float %1, float* getelementptr inbounds ([32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 11), align 4
  ret void
}

define internal void @dx.label.2() {
entry:
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)
  %1 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 0)
  %2 = extractvalue %dx.types.CBufRet.i32 %1, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %2)
  %3 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %4 = mul i32 %3, 4
  %5 = add i32 %4, 0
  %6 = getelementptr [32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 %5
  %7 = call float @dx.op.bitcastI32toF32(i32 126, i32 0)
  store float %7, float* %6, align 4
  %8 = add i32 %4, 1
  %9 = getelementptr [32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 %8
  %10 = call float @dx.op.bitcastI32toF32(i32 126, i32 0)
  store float %10, float* %9, align 4
  %11 = add i32 %4, 2
  %12 = getelementptr [32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 %11
  %13 = call float @dx.op.bitcastI32toF32(i32 126, i32 0)
  store float %13, float* %12, align 4
  %14 = add i32 %4, 3
  %15 = getelementptr [32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 %14
  %16 = call float @dx.op.bitcastI32toF32(i32 126, i32 0)
  store float %16, float* %15, align 4
  ret void
}

define void @main() {
entry:
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)
  %1 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 0)
  %2 = extractvalue %dx.types.CBufRet.i32 %1, 0
  switch i32 %2, label %switch0.default [
    i32 0, label %switch0.casegroup0
    i32 1, label %switch0.casegroup1
  ]

switch0.casegroup0:                               ; preds = %entry
  call void @dx.label.0()
  br label %switch0.end

switch0.casegroup1:                               ; preds = %entry
  call void @dx.label.1()
  br label %switch0.end

switch0.default:                                  ; preds = %entry
  call void @dx.label.2()
  br label %switch0.end

switch0.end:                                      ; preds = %switch0.default, %switch0.casegroup1, %switch0.casegroup0
  %3 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 0)
  %4 = extractvalue %dx.types.CBufRet.i32 %3, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %4)
  %5 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %6 = mul i32 %5, 4
  %7 = add i32 %6, 0
  %8 = getelementptr [32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 %7
  %9 = load float, float* %8, align 4
  %10 = add i32 %6, 1
  %11 = getelementptr [32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 %10
  %12 = load float, float* %11, align 4
  %13 = add i32 %6, 2
  %14 = getelementptr [32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 %13
  %15 = load float, float* %14, align 4
  %16 = add i32 %6, 3
  %17 = getelementptr [32 x float], [32 x float]* @dx.v32.x0, i32 0, i32 %16
  %18 = load float, float* %17, align 4
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %9)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %12)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %15)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %18)
  ret void
}

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.i32(i32, i32, i32) #1

; Function Attrs: nounwind readonly
declare i32 @dx.op.tempRegLoad.i32(i32, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readnone
declare float @dx.op.bitcastI32toF32(i32, i32) #2

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #2

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone }

!dx.version = !{!0}
!dx.shaderModel = !{!1}
!dx.resources = !{!2}
!dx.entryPoints = !{!5}
!llvm.ident = !{!12}

!0 = !{i32 1, i32 0}
!1 = !{!"ps", i32 6, i32 0}
!2 = !{null, null, !3, null}
!3 = !{!4}
!4 = !{i32 0, %dx.types.i8x16 addrspace(2)* undef, !"CB0", i32 0, i32 0, i32 1, i32 16, null}
!5 = !{void ()* @main, !"main", !6, !2, null}
!6 = !{!7, !10, null}
!7 = !{!8}
!8 = !{i32 0, !"AAA", i8 9, i8 0, !9, i8 2, i32 1, i8 4, i32 0, i8 0, null}
!9 = !{i32 0}
!10 = !{!11}
!11 = !{i32 0, !"SV_Target", i8 9, i8 16, !9, i8 0, i32 1, i8 4, i32 0, i8 0, null}
!12 = !{!"dxbc2dxil 1.0"}
