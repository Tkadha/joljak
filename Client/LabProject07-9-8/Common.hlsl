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
    float3 gvCameraPosition; // 카메라 월드 위치
    // 필요시 패딩
    float4 gFogColor; // 안개 색상 (r, g, b, 안개 강도와 무관한 alpha)
    float gFogStart; // 안개가 시작되는 카메라로부터의 거리
    float gFogRange; // 안개가 FogStart부터 완전히 불투명해지기까지의 거리 범위
                             // (즉, FogEnd = FogStart + FogRange)
  
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

// SamplerComparisonState gssShadow : register(s6); // 그림자 샘플러 등

#endif // SHADERS_COMMON_HLSLI