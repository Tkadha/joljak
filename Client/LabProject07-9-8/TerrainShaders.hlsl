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
Texture2D gtxtTerrainBaseTexture    : register(t1); // 지형 기본 텍스처
Texture2D gtxtTerrainSplatMap : register(t2);
Texture2D gtxtDirt01 : register(t3);
Texture2D gtxtDirt02 : register(t4);
Texture2D gtxtGrass01 : register(t5);
Texture2D gtxtGrass02 : register(t6);
Texture2D gtxtRock01 : register(t7);
Texture2D gtxtRock02 : register(t8);

Texture2D gShadowMap : register(t9);
Texture2D gTorchShadowMap : register(t10); // 횃불 그림자 맵

// --- VS 입출력 구조체 ---
struct VS_TERRAIN_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR; // 정점 색상 (라이팅 등)
    float2 uv0 : TEXCOORD0; // Base 텍스처 UV
    float2 uv1 : TEXCOORD1; // Detail 텍스처 UV
    float3 normal : NORMAL;
};

struct VS_TERRAIN_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 positionW : POSITION0; // 월드 공간 위치 (픽셀 셰이더 전달용)
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float4 ShadowPosH : TEXCOORD2;
    float3 normalW : NORMAL0;
};

// --- Vertex Shader ---
VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

   // 1. input.position이 이미 월드 좌표
    float4 worldPos_H = float4(input.position, 1.0f);
    output.positionW = worldPos_H.xyz; // 픽셀 셰이더로 전달할 월드 위치

    // 2. 월드 공간 위치를 뷰 및 투영 변환하여 클립 공간 위치 계산
    float4 viewPos_H = mul(worldPos_H, gmtxView);
    output.position = mul(viewPos_H, gmtxProjection); // SV_POSITION은 클립 공간 좌표
    
    // 3. 그림자 공간으로 변환
    output.ShadowPosH = mul(worldPos_H, gmtxShadowTransform);
    
    output.normalW = input.normal;
    
    output.color = input.color;
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
    
    return output;
}


float CalcShadowFactor(float4 shadowPosH)
{
   // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;
    float sampleRadius = 1.5; // 1.0 ~ 2.5

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
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow, 
            shadowPosH.xy + poissonDisk[i] * dx * sampleRadius, depth).r;
    }

    return percentLit / 16.0f;
}

float N21(float2 p)
{
    return frac(sin(dot(p, float2(12.9898, 4.1414))) * 43758.5453);
}

float SimpleNoise(float2 p)
{
    float2 ip = floor(p); // 정수 (격자 칸 ID)
    float2 fp = frac(p); // 소수 (격자 칸 내의 위치)
    
    float a = N21(ip);
    float b = N21(ip + float2(1.0, 0.0));
    float c = N21(ip + float2(0.0, 1.0));
    float d = N21(ip + float2(1.0, 1.0));
    
    fp = fp * fp * (3.0 - 2.0 * fp);
    return lerp(lerp(a, b, fp.x), lerp(c, d, fp.x), fp.y);
}

// --- Pixel Shader ---
float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
    float shadowFactor = 1.0;
    LIGHT torchLight = gLights[0]; // 횃불이 0번 조명이라고 가정

    if (torchLight.m_bEnable)
    {
        float4 torchShadowPosH = mul(float4(input.positionW, 1.0f), gmtxTorchShadowTransform);
        shadowFactor = CalcShadowFactor(torchShadowPosH, gTorchShadowMap);
    }
    
    // 계산된 shadowFactor 값을 흑백으로 화면에 바로 출력합니다.
    return float4(shadowFactor, shadowFactor, shadowFactor, 1.0f);
    
    // 1. 텍스처 색상을 계산합니다 (이전과 동일).
    float4 splatWeights = gtxtTerrainSplatMap.Sample(gssWrap, input.uv0);

    float noiseScale = 0.9f;
    float blendNoise = SimpleNoise(input.positionW.xz * noiseScale);

    float2 tiled_uv1 = input.uv1 * 100.0f;
    float2 tiled_uv2 = (input.uv1 * 57.0f) + 0.3f;

    float4 cDirt1 = gtxtDirt01.Sample(gssWrap, tiled_uv1);
    float4 cDirt2 = gtxtDirt02.Sample(gssWrap, tiled_uv2);
    float4 cGrass1 = gtxtGrass01.Sample(gssWrap, tiled_uv1);
    float4 cGrass2 = gtxtGrass02.Sample(gssWrap, tiled_uv2);
    float4 cRock1 = gtxtRock01.Sample(gssWrap, tiled_uv1);
    float4 cRock2 = gtxtRock02.Sample(gssWrap, tiled_uv2);

    float4 cMixedDirt = lerp(cDirt1, cDirt2, blendNoise);
    float4 cMixedGrass = lerp(cGrass1, cGrass2, blendNoise);
    float4 cMixedRock = lerp(cRock1, cRock2, blendNoise);

    float4 cDetailColor = splatWeights.r * cMixedDirt +
                          splatWeights.g * cMixedGrass +
                          splatWeights.b * cMixedRock;

    float baseWeight = 1.0f - saturate(splatWeights.r + splatWeights.g + splatWeights.b);
    cDetailColor += baseWeight * cMixedGrass;

    float distanceToEye = distance(input.positionW, gvCameraPosition.xyz);
    float baseTexWeight = saturate((distanceToEye - 4000.0f) / 1000.0f); // 4000~5000 거리
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);

    float4 cTextureColor = lerp(cDetailColor, cBaseTexColor, baseTexWeight);

    
    //float shadowFactor = 0.3;
    //if (gIsDaytime)
    //{
    //    shadowFactor = CalcShadowFactor(input.ShadowPosH);
    //}
    
    
    float3 normalW = normalize(input.normalW);
    
    float4 cIlluminationColor = Lighting(gMaterialInfo, input.positionW, normalW, gShadowMap, gTorchShadowMap);

    float3 totalLight = gMaterialInfo.AmbientColor.rgb + cIlluminationColor.rgb;
    float3 finalColor = totalLight * cTextureColor.rgb;
    
   
    // 안개
    if (gIsDaytime)
    {
        float distToEye = distance(input.positionW, gvCameraPosition.xyz);
        float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);
        finalColor = lerp(gFogColor.rgb, finalColor, fogFactor);
    }
    
    return float4(finalColor, cTextureColor.a);
}