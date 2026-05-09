// Unlit shader - no lighting calculations

[[vk::binding(0, 0)]]
cbuffer GlobalCB : register(b0)
{
    float4x4 uView;
    float4x4 uProjection;
    float4x4 uLightSpaceMatrices[4];
    float4 uCascadeSplits;
    float3 uLightDir;
    float uCascadeSplit4;
    float3 uLightAmbient;
    int uCascadeCount;
    float3 uLightDiffuse;
    int uDebugCascades;
    float2 uShadowMapTexelSize;
    float2 _shadowPad;
};

[[vk::binding(4, 0)]]
cbuffer PerObjectCB : register(b1)
{
    float4x4 uModel;
    float4x4 uNormalMatrix;
    float3 uColor;
    int uUseTexture;
    float uAlphaCutoff;
#ifdef TARGET_SPIRV
    float uMetallic;
    float uRoughness;
    float _pbrPad1;
    float3 uEmissive;
    float _pbrPad2;
    int uHasMetallicRoughnessMap;
    int uHasNormalMap;
    int uHasOcclusionMap;
    int uHasEmissiveMap;
    int uUseInstanceData;
    uint uInstanceBase;
    float2 _instancePad;
#else
    float3 _objPad;
#endif
};

[[vk::binding(1, 0)]]Texture2D diffuseTexture : register(t0);
[[vk::binding(1, 0)]]SamplerState linearSampler : register(s0);

#ifdef TARGET_SPIRV
struct InstanceData
{
    float4x4 model;
    float4x4 normalMatrix;
};
[[vk::binding(12, 0)]]StructuredBuffer<InstanceData> instanceData : register(t9);
#endif

struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

[shader("vertex")]
#ifdef TARGET_SPIRV
PSInput vertexMain(VSInput input, uint instanceId : SV_InstanceID)
#else
PSInput vertexMain(VSInput input)
#endif
{
    PSInput output;

#ifdef TARGET_SPIRV
    float4x4 model = uModel;
    if (uUseInstanceData != 0)
    {
        model = instanceData[uInstanceBase + instanceId].model;
    }
#else
    float4x4 model = uModel;
#endif

    float4 worldPos = mul(model, float4(input.position, 1.0));
    float4 viewPos = mul(uView, worldPos);
    output.position = mul(uProjection, viewPos);
    output.texcoord = input.texcoord;

    return output;
}

[shader("fragment")]
float4 fragmentMain(PSInput input) : SV_TARGET
{
    float4 color;
    if (uUseTexture != 0)
    {
        color = diffuseTexture.Sample(linearSampler, input.texcoord);
        if (uAlphaCutoff > 0.0 && color.a < uAlphaCutoff)
            discard;
    }
    else
    {
        color = float4(uColor, 1.0);
    }
    return color;
}
