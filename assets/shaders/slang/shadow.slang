// Shadow pass shader - depth only

[[vk::binding(0, 0)]]
cbuffer ShadowCB : register(b0)
{
#ifndef TARGET_SPIRV
    float4x4 uLightSpaceMatrix;  // D3D12: light space in UBO
    float4x4 uModel;             // D3D12: model in UBO
    int uUseHeightmapDisplacement;
    float uHeightmapScale;
    float uHeightmapOffset;
    float _heightmapPad0;
    float2 uHeightmapTexelSize;
    float2 _heightmapPad1;
#endif
};

#ifdef TARGET_SPIRV
[[vk::push_constant]]
cbuffer ShadowPush
{
    float4x4 uLightSpaceMatrix;  // Vulkan: light space via push constant (per-cascade)
    float4x4 uModel;             // Vulkan: model via push constant (per-draw)
    int uUseHeightmapDisplacement;
    float uHeightmapScale;
    float uHeightmapOffset;
    float _heightmapPad0;
    float2 uHeightmapTexelSize;
    float2 _heightmapPad1;
};
#endif

[[vk::binding(13, 0)]]Texture2D heightmapTexture : register(t8);

struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

float sampleHeightmap(float2 uv)
{
    float2 texelSize = max(uHeightmapTexelSize, float2(0.000001, 0.000001));
    float2 dimensions = max(round(1.0 / texelSize), float2(1.0, 1.0));
    int2 coord = int2(round(saturate(uv) * (dimensions - 1.0)));
    return heightmapTexture.Load(int3(coord, 0)).r * uHeightmapScale + uHeightmapOffset;
}

float3 applyHeightmapDisplacement(float3 position, float2 uv)
{
    if (uUseHeightmapDisplacement != 0)
        position.y += sampleHeightmap(uv);
    return position;
}

[shader("vertex")]
PSInput vertexMain(VSInput input)
{
    PSInput output;
    float3 localPosition = applyHeightmapDisplacement(input.position, input.texcoord);
    float4 worldPos = mul(uModel, float4(localPosition, 1.0));
    output.position = mul(uLightSpaceMatrix, worldPos);
    return output;
}

[shader("fragment")]
void fragmentMain(PSInput input)
{
    // Depth-only pass - no color output needed
}
