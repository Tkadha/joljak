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
Texture2D gtxtTerrainGrassTexture : register(t2); // DetailTexture -> GrassTexture
Texture2D gtxtTerrainDirtTexture : register(t3); // 새로 추가
Texture2D gtxtTerrainRockTexture : register(t4); // 새로 추가
Texture2D gShadowMap : register(t5);    // 그림자

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
    float3 positionW : POSITION0; // 월드 공간 위치 (픽셀 셰이더 전달용)
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float4 ShadowPosH : TEXCOORD2;
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

// --- Pixel Shader ---
float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{     
    // 1. 각 디테일 텍스처를 타일링된 UV 좌표(uv1)로 샘플링합니다.
    //    타일링 횟수를 늘려 해상도를 높이고 싶다면 input.uv1에 상수를 곱합니다.
    float2 tiled_uv = input.uv1 * 100.0f; // ★ 텍스처를 100번 반복하여 선명하게 만듭니다.
    float4 cGrassColor = gtxtTerrainGrassTexture.Sample(gssWrap, tiled_uv);
    float4 cDirtColor = gtxtTerrainDirtTexture.Sample(gssWrap, tiled_uv);
    float4 cRockColor = gtxtTerrainRockTexture.Sample(gssWrap, tiled_uv);

    // 2. 지형의 높이(월드 좌표 Y)를 기준으로 텍스처 혼합 비율(Weight)을 계산합니다.
    float height = input.positionW.y;
    
    // 낮은 지역(풀) -> 중간 지역(흙)으로 넘어가는 구간을 부드럽게 만듭니다.
    float dirtWeight = saturate((height - 200.0f) / 100.0f); // 200~300 사이에서 흙이 100%가 됨
    
    // 중간 지역(흙) -> 높은 지역(돌)으로 넘어가는 구간을 부드럽게 만듭니다.
    float rockWeight = saturate((height - 400.0f) / 150.0f); // 400~550 사이에서 돌이 100%가 됨

    // 3. 계산된 혼합 비율에 따라 텍스처 색상을 선형 보간(lerp)합니다.
    float4 cDetailColor = lerp(cGrassColor, cDirtColor, dirtWeight);
    cDetailColor = lerp(cDetailColor, cRockColor, rockWeight);
    
    // 4. 멀리 있는 지형은 저해상도 베이스 텍스처와 자연스럽게 섞어줍니다.
    float distanceToEye = distance(input.positionW, gvCameraPosition.xyz);
    float baseTexWeight = saturate((distanceToEye - 4000.0f) / 1000.0f); // 4000~5000 거리에서 베이스 텍스처와 섞임
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);

    float4 cTextureColor = lerp(cDetailColor, cBaseTexColor, baseTexWeight);

     // 1. 그림자 계수 계산
    float shadowFactor = 0.15; // 기본값은 그림자 없음
    if (gIsDaytime) 
    {
        shadowFactor = CalcShadowFactor(input.ShadowPosH);
    }
    
// 최종 조명 계산 (input.color는 미리 계산된 빛, 여기에 그림자를 적용)
    float3 totalLight = gcGlobalAmbientLight.rgb + (shadowFactor * input.color.rgb);

    // 최종 색상 결정
    float3 finalColor = cTextureColor.rgb * totalLight;
    
    //안개
    //float distToEye = distance(input.positionW, gvCameraPosition.xyz);        
    //float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);    
    //float normalizedDistance = saturate(distToEye / (gFogStart + gFogRange));
    
    
    if (gIsDaytime)
    {
        float distToEye = distance(input.positionW, gvCameraPosition.xyz);
        float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);
        finalColor = lerp(gFogColor.rgb, finalColor, fogFactor);
    }
    
    return float4(finalColor, cTextureColor.a);
}