// RUN: %clang_cc1 -fprofile-instr-generate -fcoverage-mapping -dump-coverage-mapping -emit-llvm-only -main-file-name macro-expansion.c %s | FileCheck %s

#define LBRAC {
#define RBRAC }

// CHECK: main:
// CHECK-NEXT: File 0, [[@LINE+1]]:40 -> {{[0-9]+}}:2 = #0
int main(int argc, const char *argv[]) {
  // CHECK-NEXT: File 0, [[@LINE+1]]:7 -> [[@LINE+1]]:12 = #0
  if (!argc) {} // CHECK-NEXT: File 0, [[@LINE]]:14 -> [[@LINE]]:16 = #1

  // CHECK-NEXT: File 0, [[@LINE+3]]:7 -> [[@LINE+3]]:12 = #0
  // CHECK-NEXT: Expansion,File 0, [[@LINE+2]]:14 -> [[@LINE+2]]:19 = #2
  // CHECK-NEXT: File 0, [[@LINE+1]]:19 -> [[@LINE+4]]:8 = #2
  if (!argc) LBRAC
    return 0;
  // CHECK-NEXT: Expansion,File 0, [[@LINE+1]]:3 -> [[@LINE+1]]:8 = #2
  RBRAC

  // CHECK-NEXT: File 0, [[@LINE+4]]:3 -> [[@LINE+15]]:2 = (#0 - #2)
  // CHECK-NEXT: File 0, [[@LINE+3]]:7 -> [[@LINE+3]]:12 = (#0 - #2)
  // CHECK-NEXT: Expansion,File 0, [[@LINE+2]]:14 -> [[@LINE+2]]:19 = #3
  // CHECK-NEXT: File 0, [[@LINE+1]]:19 -> [[@LINE+3]]:4 = #3
  if (!argc) LBRAC
    return 0;
  }

  // CHECK-NEXT: File 0, [[@LINE+3]]:3 -> [[@LINE+7]]:2 = ((#0 - #2) - #3)
  // CHECK-NEXT: File 0, [[@LINE+2]]:7 -> [[@LINE+2]]:12 = ((#0 - #2) - #3)
  // CHECK-NEXT: File 0, [[@LINE+1]]:14 -> [[@LINE+4]]:8 = #4
  if (!argc) {
    return 0;
  // CHECK-NEXT: Expansion,File 0, [[@LINE+1]]:3 -> [[@LINE+1]]:8 = #4
  RBRAC
}

// CHECK-NEXT: File 1, 3:15 -> 3:16 = #2
// CHECK-NEXT: File 2, 4:15 -> 4:16 = #2
// CHECK-NEXT: File 3, 3:15 -> 3:16 = #3
// CHECK-NEXT: File 4, 4:15 -> 4:16 = #4
