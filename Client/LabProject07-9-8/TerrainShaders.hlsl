// TerrainShaders.hlsl

#include "Common.hlsl"

// --- 상수 버퍼 ---
cbuffer cbGameObjectInfo : register(b2) // 지형의 월드 변환 등에 사용
{
    matrix gmtxGameObject;
    MaterialInfo gMaterialInfo; // 지형에서는 사용 안 할 수도 있음
    uint gnTexturesMask;
    // 패딩
};

// --- 텍스처 ---
Texture2D gtxtTerrainBaseTexture : register(t1); // 지형 기본 텍스처
Texture2D gtxtTerrainDetailTexture : register(t2); // 지형 상세 텍스처

// --- VS 입출력 구조체 ---
struct VS_TERRAIN_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR; // 정점 색상 (라이팅 등)
    float2 uv0 : TEXCOORD0; // Base 텍스처 UV
    float2 uv1 : TEXCOORD1; // Detail 텍스처 UV
};

struct VS_TERRAIN_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

// --- Vertex Shader ---
VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

    // 클립 공간 변환
    output.position = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.position = mul(output.position, gmtxView);
    output.position = mul(output.position, gmtxProjection);

    output.color = input.color; // 정점 색상 전달
    output.uv0 = input.uv0; // UV 좌표 전달
    output.uv1 = input.uv1;

    return output;
}

// --- Pixel Shader ---
float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
    // 텍스처 샘플링
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
    float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);

    // 색상 조합 (예: 블렌딩)
    // float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f)); // 단순 평균
    float4 cColor = input.color * lerp(cBaseTexColor, cDetailTexColor, 0.5); // 예: 정점 색상과 블렌딩

    return cColor;
}