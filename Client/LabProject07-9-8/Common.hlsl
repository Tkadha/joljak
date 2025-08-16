// Shaders_Common.hlsli

#ifndef SHADERS_COMMON_HLSLI
#define SHADERS_COMMON_HLSLI

// --- 공통 구조체 ---

// 재질 정보 (C++의 MaterialInfoCpp 와 일치)
struct MaterialInfo
{
    float4 AmbientColor;
    float4 DiffuseColor; // Albedo 텍스처와 곱해짐
    float4 SpecularColor; // Alpha에 Power 저장 가능
    float4 EmissiveColor;

    float Glossiness;
    float Smoothness;
    float SpecularHighlight;
    float Metallic;
    float GlossyReflection;
    float3 Padding; // 16바이트 정렬용
};

// --- 공통 상수 버퍼 ---

// 카메라 정보 (모든 VS에서 사용될 가능성 높음)
cbuffer cbCameraInfo : register(b1)
{
    matrix gmtxView; // 뷰 행렬
    matrix gmtxProjection; // 투영 행렬
    matrix gmtxShadowTransform;
    matrix gmtxTorchShadowTransform;
    
    float3 gvCameraPosition; // 카메라 월드 위치
    float gCameraPadding; // 패딩 추가
    
    float4 gFogColor; // 안개 색상 (r, g, b, 안개 강도와 무관한 alpha)
    float gFogStart; // 안개가 시작되는 카메라로부터의 거리
    float gFogRange; // 안개가 FogStart부터 완전히 불투명해지기까지의 거리 범위
    float2 gFogPadding; // 패딩 추가
    
};

// 조명 정보 (조명이 필요한 PS에서 사용, Light.hlsl 에서 정의된 구조체 사용 가정)
// 예: #include "Light.hlsl" 이후 정의
// cbuffer cbLights : register(b4)
// {
//     Light gLights[MAX_LIGHTS]; // Light 구조체는 Light.hlsl 에 정의
//     float4 gAmbientLight;
//     int gNumLights;
//     // ...
// };


// --- 공통 함수 (조명 계산 등) ---
// 조명 관련 코드는 별도 파일로 분리하는 것이 좋음
#include "Light.hlsl" // 조명 계산 함수 및 구조체 포함 가정


// --- 공통 Define ---
#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

// --- 공통 샘플러 (Static Sampler로 대체될 수 있음) ---
// 루트 서명에서 정적 샘플러로 정의하는 것이 더 효율적이지만,
// HLSL 코드 내 참조를 위해 선언해 둘 수 있음

SamplerState gssWrap : register(s0); // Wrap 모드 샘플러
SamplerState gssClamp : register(s1); // Clamp 모드 샘플러
SamplerComparisonState gsamShadow : register(s2); // 비교 샘플러

// SamplerComparisonState gssShadow : register(s6); // 그림자 샘플러 등





float CalcShadowFactor(float4 shadowPosH, Texture2D shadowMap)
{
    shadowPosH.xyz /= shadowPosH.w;
    if (saturate(shadowPosH.x) != shadowPosH.x || saturate(shadowPosH.y) != shadowPosH.y)
    {
        return 1.0f; // 빛을 100% 받음
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

// 최종 조명 계산 함수 (모든 셰이더가 이 함수를 공통으로 사용)
float4 Lighting(MaterialInfo material, float3 posW, float3 normalW,
                Texture2D sunShadowMap, Texture2D torchShadowMap)
{
    float4 finalIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // gnLights는 cbLights에 있어야 하지만, 일단 전역으로 가정
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
            
            
            // 계산된 shadowFactor 값을 흑백으로 화면에 바로 출력합니다.
            return float4(shadowFactor, shadowFactor, shadowFactor, 1.0f);
        }
        
        float4 illumination = ComputeLight(material, gLights[i], posW, normalW);
        finalIllumination += shadowFactor * illumination;
    }
    
    return finalIllumination;
}

#endif // SHADERS_COMMON_HLSLI