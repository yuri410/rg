// Run: %dxc -T ps_6_2 -E main -enable-16bit-types

// CHECK: OpCapability Float16
// CHECK: OpExtInstImport "GLSL.std.450"
void main() {
  float16_t4 a;
  float16_t4 result = atan(a);
}
