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
    float3 gvCameraPosition; // ī�޶� ���� ��ġ
    // �ʿ�� �е�
    float4 gFogColor; // �Ȱ� ���� (r, g, b, �Ȱ� ������ ������ alpha)
    float gFogStart; // �Ȱ��� ���۵Ǵ� ī�޶�κ����� �Ÿ�
    float gFogRange; // �Ȱ��� FogStart���� ������ ����������������� �Ÿ� ����
                             // (��, FogEnd = FogStart + FogRange)
  
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

// SamplerComparisonState gssShadow : register(s6); // �׸��� ���÷� ��

#endif // SHADERS_COMMON_HLSLI