// Shaders_Common.hlsli

#ifndef SHADERS_COMMON_HLSLI
#define SHADERS_COMMON_HLSLI

// --- ���� ����ü ---

// ���� ���� (C++�� MaterialInfoCpp �� ��ġ)
struct MaterialInfo
{
    float4 AmbientColor;
    float4 DiffuseColor; // Albedo �ؽ�ó�� ������
    float4 SpecularColor; // Alpha�� Power ���� ����
    float4 EmissiveColor;

    float Glossiness;
    float Smoothness;
    float SpecularHighlight;
    float Metallic;
    float GlossyReflection;
    float3 Padding; // 16����Ʈ ���Ŀ�
};

// --- ���� ��� ���� ---

// ī�޶� ���� (��� VS���� ���� ���ɼ� ����)
cbuffer cbCameraInfo : register(b1)
{
    matrix gmtxView; // �� ���
    matrix gmtxProjection; // ���� ���
    matrix gmtxShadowTransform;
    matrix gmtxTorchShadowTransform;
    
    float3 gvCameraPosition; // ī�޶� ���� ��ġ
    float gCameraPadding; // �е� �߰�
    
    float4 gFogColor; // �Ȱ� ���� (r, g, b, �Ȱ� ������ ������ alpha)
    float gFogStart; // �Ȱ��� ���۵Ǵ� ī�޶�κ����� �Ÿ�
    float gFogRange; // �Ȱ��� FogStart���� ������ ����������������� �Ÿ� ����
    float2 gFogPadding; // �е� �߰�
    
};

// ���� ���� (������ �ʿ��� PS���� ���, Light.hlsl ���� ���ǵ� ����ü ��� ����)
// ��: #include "Light.hlsl" ���� ����
// cbuffer cbLights : register(b4)
// {
//     Light gLights[MAX_LIGHTS]; // Light ����ü�� Light.hlsl �� ����
//     float4 gAmbientLight;
//     int gNumLights;
//     // ...
// };


// --- ���� �Լ� (���� ��� ��) ---
// ���� ���� �ڵ�� ���� ���Ϸ� �и��ϴ� ���� ����
#include "Light.hlsl" // ���� ��� �Լ� �� ����ü ���� ����


// --- ���� Define ---
#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

// --- ���� ���÷� (Static Sampler�� ��ü�� �� ����) ---
// ��Ʈ ������ ���� ���÷��� �����ϴ� ���� �� ȿ����������,
// HLSL �ڵ� �� ������ ���� ������ �� �� ����

SamplerState gssWrap : register(s0); // Wrap ��� ���÷�
SamplerState gssClamp : register(s1); // Clamp ��� ���÷�
SamplerComparisonState gsamShadow : register(s2); // �� ���÷�

// SamplerComparisonState gssShadow : register(s6); // �׸��� ���÷� ��





float CalcShadowFactor(float4 shadowPosH, Texture2D shadowMap)
{
    shadowPosH.xyz /= shadowPosH.w;
    if (saturate(shadowPosH.x) != shadowPosH.x || saturate(shadowPosH.y) != shadowPosH.y)
    {
        return 1.0f; // ���� 100% ����
    }
    float depth = shadowPosH.z;

    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);

    float dx = 1.0f / (float) width;
    float sampleRadius = 1.5;
    
    const float2 poissonDisk[16] =
    {
        float2(-0.94201624, -0.39906216),
        float2(0.94558609, -0.76890725),
        float2(-0.094184101, -0.92938870),
        float2(0.34495938, 0.29387760),
        float2(-0.91588581, 0.45771432),
        float2(-0.81544232, -0.87912464),
        float2(-0.38277543, 0.27676845),
        float2(0.97484398, 0.75648379),
        float2(0.44323325, -0.97511554),
        float2(0.53742981, -0.47373420),
        float2(-0.26496911, -0.41893023),
        float2(0.79197514, 0.19090188),
        float2(-0.24188840, 0.99706507),
        float2(-0.81409955, 0.91437590),
        float2(0.19984126, 0.78641367),
        float2(0.14383161, -0.14100790)
    };

    float percentLit = 0.0f;
    [unroll]
    for (int i = 0; i < 16; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(gsamShadow, shadowPosH.xy + poissonDisk[i] * dx * sampleRadius, depth).r;
    }
    
    return percentLit / 16.0f;
}

// ���� ���� ��� �Լ� (��� ���̴��� �� �Լ��� �������� ���)
float4 Lighting(MaterialInfo material, float3 posW, float3 normalW,
                Texture2D sunShadowMap, Texture2D torchShadowMap)
{
    float4 finalIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // gnLights�� cbLights�� �־�� ������, �ϴ� �������� ����
    for (int i = 0; i < gnLights; ++i)
    {
        if (!gLights[i].m_bEnable)
            continue;

        float shadowFactor = 1.0f;

        if (gLights[i].m_nType == DIRECTIONAL_LIGHT && gIsDaytime)
        {
            float4 shadowPosH = mul(float4(posW, 1.0f), gmtxShadowTransform);
            shadowFactor = CalcShadowFactor(shadowPosH, sunShadowMap);
        }
        else if (gLights[i].m_nType == POINT_LIGHT)
        {
            float4 shadowPosH = mul(float4(posW, 1.0f), gmtxTorchShadowTransform);
            shadowFactor = CalcShadowFactor(shadowPosH, torchShadowMap);
            
            
            // ���� shadowFactor ���� ������� ȭ�鿡 �ٷ� ����մϴ�.
            return float4(shadowFactor, shadowFactor, shadowFactor, 1.0f);
        }
        
        float4 illumination = ComputeLight(material, gLights[i], posW, normalW);
        finalIllumination += shadowFactor * illumination;
    }
    
    return finalIllumination;
}

#endif // SHADERS_COMMON_HLSLI