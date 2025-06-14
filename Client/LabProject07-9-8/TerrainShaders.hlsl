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

Texture2D gShadowMap : register(t3);    // 그림자

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

   // 1. 모델 로컬 좌표를 월드 공간으로 변환
    float4 worldPos_H = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.positionW = worldPos_H.xyz; // 픽셀 셰이더로 전달할 월드 위치

    // 2. 월드 공간 위치를 뷰 및 투영 변환하여 클립 공간 위치 계산
    float4 viewPos_H = mul(worldPos_H, gmtxView);
    output.position = mul(viewPos_H, gmtxProjection); // SV_POSITION은 클립 공간 좌표
    
    // 3. 그림자 공간으로 변환
    output.ShadowPosH = mul(worldPos_H, gmtxShadowTransform);

    output.color = input.color; // 정점 색상 전달
    output.uv0 = input.uv0; // UV 좌표 전달
    output.uv1 = input.uv1;

    return output;
}


float CalcShadowFactor(float4 shadowPosH)
{
    shadowPosH.xyz /= shadowPosH.w;
    shadowPosH.x = +0.5f * shadowPosH.x + 0.5f;
    shadowPosH.y = -0.5f * shadowPosH.y + 0.5f;
    float shadowFactor = gShadowMap.SampleCmpLevelZero(gsamShadow, shadowPosH.xy, shadowPosH.z);
    return shadowFactor;
}

// --- Pixel Shader ---
float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
    // 텍스처 샘플링
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
    float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);

    // 색상 조합 (예: 블렌딩)
    // float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f)); // 단순 평균
    //float4 cColor = input.color * lerp(cBaseTexColor, cDetailTexColor, 0.5); // 예: 정점 색상과 블렌딩
    
    float4 cTextureColor = lerp(cBaseTexColor, cDetailTexColor, 0.5);
    
    
     // 1. 그림자 계수 계산
    float shadowFactor = CalcShadowFactor(input.ShadowPosH);

    // 2. 최종 조명 계산 (지형의 정점 색상(input.color)을 기본 조명값으로 사용)
    //    약간의 주변광(Ambient)을 더해주어 그림자가 완전히 새까맣게 되는 것을 방지합니다.
    float3 totalLight = float3(0.3f, 0.3f, 0.3f) + shadowFactor * input.color.rgb;

    // 3. 최종 색상 결정: 텍스처 색상에 최종 조명 색상을 곱합니다.
    float4 cColor;
    cColor.rgb = cTextureColor.rgb * totalLight;
    cColor.a = cTextureColor.a;
    
    //안개
    float distToEye = distance(input.positionW, gvCameraPosition.xyz);        
    float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);    
    float normalizedDistance = saturate(distToEye / (gFogStart + gFogRange));
    //return float4(normalizedDistance, normalizedDistance, normalizedDistance, 1.0f); // [수정된 디버깅 출력]

    
    // --- fogFactor 값 디버깅 ---
    //return float4(fogFactor, fogFactor, fogFactor, 1.0f); 
    
    cColor.rgb = lerp(cColor.rgb, gFogColor.rgb, normalizedDistance);
    
    return cColor;
}