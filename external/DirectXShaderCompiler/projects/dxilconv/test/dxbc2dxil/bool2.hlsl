// RUN: %fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float a : A, float b : B) : SV_Target
{
    return a < b ? 0 : 1;
}
