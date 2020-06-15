/*// RUN: %testasm %s /allowMinimumPrecision /Fo %t.dxbc*/
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1 INTERNAL
//
//
// Note: shader requires additional functionality:
//       Minimum-precision data types
//
//
// Buffer Definitions:
//
// cbuffer $Globals
// {
//
//   min16float g1[4];                  // Offset:    0 Size:    52
//   min16float g2[8];                  // Offset:   64 Size:   116
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// $Globals                          cbuffer      NA          NA            cb0      1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// A                        0   xyzw        0     NONE  min16f
// B                        0   x           1     NONE     int   x
// C                        0    y          1     NONE     int    y
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_TARGET                0   x           0   TARGET  min16f   x
//
ps_5_0
dcl_globalFlags refactoringAllowed | enableMinimumPrecision
dcl_constantbuffer cb0[12], dynamicIndexed
dcl_input_ps constant v1.x
dcl_input_ps constant v1.y
dcl_output o0.x {min16f}
dcl_temps 1
dcl_indexableTemp x0[4], 4
mov r0.x, v1.x
mov x0[0].x, cb0[r0.x + 4].x
mov r0.y, x0[0].x
mov x0[0].x {min16f}, cb0[r0.x + 0].x {min16f}
mov x0[1].x {min16f}, cb0[r0.x + 4].x {min16f}
mov r0.x, v1.y
add x0[r0.x + 0].x {min16f}, x0[r0.x + 0].x {min16f}, r0.y {min16f}
mov o0.x {min16f}, x0[r0.x + 0].x {min16f}
ret
