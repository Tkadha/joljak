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
    //------------디버깅------------
    
    // 정점 셰이더에서 넘어온 월드 좌표(input.ShadowPosH)를
    // 맵의 최대 크기(예: 5000.0)로 나누어 [0,1] 범위의 색상으로 만듭니다.
    // 이 값을 조절하며 테스트해보세요.
    //float3 worldPosColor = input.ShadowPosH.xyz / 10000.0f;

    //// x, y, z 좌표를 각각 R, G, B 색상에 매핑하여 출력합니다.
    //return float4(worldPosColor.r, worldPosColor.g, worldPosColor.b, 1.0f);
    
    //return float4(input.uv0.x, input.uv0.y, input.uv1.x, 1.0f);
    //------------디버깅------------
    
    // --- 최종 디버깅: 깊이 값 비교 시각화 ---

    // 1. 비교에 사용할 두 개의 깊이 값을 계산합니다.
    //float4 shadowPosH = input.ShadowPosH;
    //shadowPosH.xyz /= shadowPosH.w;
    //float2 shadowUV = float2(0.5f * shadowPosH.x + 0.5f, -0.5f * shadowPosH.y + 0.5f);

    //// [A] 현재 픽셀의 깊이 값 (빛의 시점)
    //float myDepth = shadowPosH.z;

    //// [B] 그림자 맵에 저장된, 가장 가까운 물체의 깊이 값
    //float shadowMapDepth = gShadowMap.SampleLevel(gssWrap, shadowUV, 0).r;

    //// 2. 이 두 값을 직접 화면에 색상으로 출력하여 비교합니다.
    //// R 채널: [A] 현재 픽셀의 깊이
    //// G 채널: [B] 그림자 맵에 저장된 깊이
    //// B 채널: 두 값의 차이 (차이가 클수록 파란색이 강해짐)
    //return float4(myDepth, shadowMapDepth, abs(myDepth - shadowMapDepth) * 10.0, 1.0);

    
    // 텍스처 샘플링
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
    float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);

    // 색상 조합 (예: 블렌딩)
    // float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f)); // 단순 평균
    //float4 cColor = input.color * lerp(cBaseTexColor, cDetailTexColor, 0.5); // 예: 정점 색상과 블렌딩
    
    float4 cTextureColor = lerp(cBaseTexColor, cDetailTexColor, 0.5);

     // 1. 그림자 계수 계산
    float shadowFactor = CalcShadowFactor(input.ShadowPosH);
    
// 최종 조명 계산 (input.color는 미리 계산된 빛, 여기에 그림자를 적용)
    float3 totalLight = float3(0.3f, 0.3f, 0.3f) + (shadowFactor * input.color.rgb);

    // 최종 색상 결정
    float3 finalColor = cTextureColor.rgb * totalLight;
    
    //안개
    //float distToEye = distance(input.positionW, gvCameraPosition.xyz);        
    //float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);    
    //float normalizedDistance = saturate(distToEye / (gFogStart + gFogRange));
    
    float distToEye = distance(input.positionW, gvCameraPosition.xyz);
    float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);
    finalColor = lerp(gFogColor.rgb, finalColor, fogFactor);

    
    return float4(finalColor, cTextureColor.a);
}