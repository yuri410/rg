// RUN: %dxc -auto-binding-space 13 -exports PSMain,PSMain_Clone1,PSMain_Clone2=PSMain -T lib_6_3 %s | %D3DReflect %s | FileCheck %s

Buffer<int> T0;

Texture2D<float4> T1;

struct Foo { uint u; float f; };
StructuredBuffer<Foo> T2;

[shader("vertex")]
float4 VSMain(int3 coord : COORD) : SV_Position {
  return T1.Load(coord);
}

[shader("pixel")]
float4 PSMain(int idx : INDEX) : SV_Target {
  return T2[T0.Load(idx)].f;
}


// CHECK: ID3D12LibraryReflection:
// CHECK:   D3D12_LIBRARY_DESC:
// CHECK:     FunctionCount: 6
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?PSMain@@YA?AV?$vector@M$03@@H@Z
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?PSMain_Clone1@@YA?AV?$vector@M$03@@H@Z
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?PSMain_Clone2@@YA?AV?$vector@M$03@@H@Z
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: PSMain
// CHECK:       Shader Version: Pixel 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: PSMain_Clone1
// CHECK:       Shader Version: Pixel 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: PSMain_Clone2
// CHECK:       Shader Version: Pixel 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
