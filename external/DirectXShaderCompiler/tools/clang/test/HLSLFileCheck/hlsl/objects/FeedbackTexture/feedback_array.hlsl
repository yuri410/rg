// RUN: %dxc -E main -T ps_6_5 %s | FileCheck %s

// Test FeedbackTexture2DArray and its WriteSamplerFeedback methods

FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMipArray;
FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIP_REGION_USED> feebackMipRegionUsedArray;
Texture2DArray<float> texture2DArray;
Texture2DArray<float4> texture2DArray_float4;
SamplerState samp;

float main() : SV_Target
{
    float3 coords2DArray = float3(1, 2, 3);
    float clamp = 4;
    float bias = 0.5F;
    float lod = 6;
    float2 ddx = float2(1.0F / 32, 2.0F / 32);
    float2 ddy = float2(3.0F / 32, 4.0F / 32);

    float idx = 0;  // Make each coord set unique

    // Test every dxil intrinsic
    // CHECK: call void @dx.op.writeSamplerFeedback(
    // CHECK: float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float undef, float 4.000000e+00)
    feedbackMinMipArray.WriteSamplerFeedback(texture2DArray, samp, coords2DArray + (10 * idx++), clamp);
    // CHECK: call void @dx.op.writeSamplerFeedbackBias(
    // CHECK: float 1.100000e+01, float 1.200000e+01, float 1.300000e+01, float undef, float 5.000000e-01, float 4.000000e+00)
    feedbackMinMipArray.WriteSamplerFeedbackBias(texture2DArray, samp, coords2DArray + (10 * idx++), bias, clamp);
    // CHECK: call void @dx.op.writeSamplerFeedbackLevel(
    // CHECK: float 2.100000e+01, float 2.200000e+01, float 2.300000e+01, float undef, float 6.000000e+00)
    feedbackMinMipArray.WriteSamplerFeedbackLevel(texture2DArray, samp, coords2DArray + (10 * idx++), lod);
    // CHECK: call void @dx.op.writeSamplerFeedbackGrad(
    // CHECK: float 3.100000e+01, float 3.200000e+01, float 3.300000e+01, float undef, float 3.125000e-02, float 6.250000e-02, float undef, float 9.375000e-02, float 1.250000e-01, float undef, float 4.000000e+00)
    feedbackMinMipArray.WriteSamplerFeedbackGrad(texture2DArray, samp, coords2DArray + (10 * idx++), ddx, ddy, clamp);

    // Test with undef clamp
    // CHECK: call void @dx.op.writeSamplerFeedback(
    // CHECK: float 4.100000e+01, float 4.200000e+01, float 4.300000e+01, float undef, float undef)
    feedbackMinMipArray.WriteSamplerFeedback(texture2DArray, samp, coords2DArray + (10 * idx++));
    // CHECK: call void @dx.op.writeSamplerFeedbackBias(
    // CHECK: float 5.100000e+01, float 5.200000e+01, float 5.300000e+01, float undef, float 5.000000e-01, float undef)
    feedbackMinMipArray.WriteSamplerFeedbackBias(texture2DArray, samp, coords2DArray + (10 * idx++), bias);
    // CHECK: call void @dx.op.writeSamplerFeedbackGrad(
    // CHECK: float 6.100000e+01, float 6.200000e+01, float 6.300000e+01, float undef, float 3.125000e-02, float 6.250000e-02, float undef, float 9.375000e-02, float 1.250000e-01, float undef, float undef)
    feedbackMinMipArray.WriteSamplerFeedbackGrad(texture2DArray, samp, coords2DArray + (10 * idx++), ddx, ddy);

    // Test on every FeedbackTexture variant
    // CHECK: call void @dx.op.writeSamplerFeedback(
    // CHECK: float 7.100000e+01, float 7.200000e+01, float 7.300000e+01, float undef, float undef)
    feedbackMinMipArray.WriteSamplerFeedback(texture2DArray, samp, coords2DArray + (10 * idx++));
    // CHECK: call void @dx.op.writeSamplerFeedback(
    // CHECK: float 8.100000e+01, float 8.200000e+01, float 8.300000e+01, float undef, float undef)
    feebackMipRegionUsedArray.WriteSamplerFeedback(texture2DArray, samp, coords2DArray + (10 * idx++));

    // Test with overloaded texture type
    // CHECK: call void @dx.op.writeSamplerFeedback(
    // CHECK: float 9.100000e+01, float 9.200000e+01, float 9.300000e+01, float undef, float undef)
    feedbackMinMipArray.WriteSamplerFeedback(texture2DArray_float4, samp, coords2DArray + (10 * idx++));

    // Test max-clamped bias
    // CHECK: call void @dx.op.writeSamplerFeedbackBias(
    // CHECK: float 1.010000e+02, float 1.020000e+02, float 1.030000e+02, float undef, float 0x402FFAE140000000, float undef)
    feedbackMinMipArray.WriteSamplerFeedbackBias(texture2DArray, samp, coords2DArray + (10 * idx++), 27.0);
    // Test min-clamped bias
    // CHECK: call void @dx.op.writeSamplerFeedbackBias(
    // CHECK: float 1.110000e+02, float 1.120000e+02, float 1.130000e+02, float undef, float -1.600000e+01, float undef)
    feedbackMinMipArray.WriteSamplerFeedbackBias(texture2DArray, samp, coords2DArray + (10 * idx++), -27.0);

    return 0;
}
