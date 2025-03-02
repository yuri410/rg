// Run: %dxc -T cs_6_0 -E main -Zi

// CHECK:      [[file:%\d+]] = OpString
// CHECK-SAME: spirv.debug.opline.precedence.hlsl

void main() {
  int a;
  int b;

//CHECK:      OpLine [[file]] 12 3
//CHECK-NEXT: OpSelectionMerge %switch_merge None
  switch (a) {
  default:
    b = 0;
  case 1:
    b = 1;
    break;
  case 2:
    b = 2;
  }

//CHECK:      OpLine [[file]] 24 23
//CHECK-NEXT: OpLoopMerge %for_merge %for_continue None
  for (int i = 0; i < 4; i++) {
    b += i;
  }
}
