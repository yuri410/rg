// RUN: %fxc /T gs_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




struct MyStruct
{
    float4 pos : SV_Position;
    float2 a : AAA;
};

struct MyStruct2
{
    uint3 X : XXX;
    float4 p[3] : PPP;
    uint3 Y : YYY;
};

int g1;

[maxvertexcount(12)]
void main(point float4 array[1] : COORD, inout PointStream<MyStruct> OutputStream0, 
                                         inout PointStream<MyStruct2> OutputStream1,
                                         inout PointStream<MyStruct> OutputStream2)
{
  float4 r = array[0];
  MyStruct a = (MyStruct)0;
  MyStruct2 b = (MyStruct2)0;
  a.pos = array[r.x];
  a.a = r.xy;
  b.X = r.xyz;
  b.Y = a.pos.xyz;
  b.p[2] = a.pos * 44;
  if (g1) {
    OutputStream0.Append(a);
    OutputStream0.RestartStrip();
  } else {
    b.X.x = r.w;
    OutputStream1.Append(b);
    OutputStream1.RestartStrip();
  }
  OutputStream1.Append(b);
  OutputStream1.RestartStrip();

  OutputStream2.Append(a);
  OutputStream2.RestartStrip();
}

