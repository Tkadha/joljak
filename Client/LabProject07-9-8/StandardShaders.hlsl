// StandardShader.hlsl

#include "Common.hlsl" // 공통 요소 포함

// --- 상수 버퍼 ---

// 게임 오브젝트 정보 (Standard, Skinned, Terrain 등에서 사용)
cbuffer cbGameObjectInfo : register(b2)
{
    matrix gmtxGameObject; // 월드 변환
    MaterialInfo gMaterialInfo; // 재질 정보
    uint gnTexturesMask; // 텍스처 사용 마스크
    // 필요시 패딩
};

// --- 텍스처 (Standard, Skinned, Instancing 에서 사용) ---
Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

// --- VS 입출력 구조체 ---
struct VS_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION; // 클립 공간 위치
    float3 positionW : POSITION; // 월드 공간 위치
    float3 normalW : NORMAL; // 월드 공간 노멀
    float3 tangentW : TANGENT; // 월드 공간 탄젠트
    float3 bitangentW : BITANGENT; // 월드 공간 바이탄젠트
    float2 uv : TEXCOORD; // UV 좌표
};

// --- Vertex Shader ---
VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    // 월드 공간 변환
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    output.normalW = normalize(mul(input.normal, (float3x3) gmtxGameObject));
    output.tangentW = normalize(mul(input.tangent, (float3x3) gmtxGameObject));
    output.bitangentW = normalize(mul(input.bitangent, (float3x3) gmtxGameObject));

    // 클립 공간 변환
    output.position = mul(float4(output.positionW, 1.0f), gmtxView);
    output.position = mul(output.position, gmtxProjection);

    output.uv = input.uv;

    return output;
}

// --- Pixel Shader ---
// (PSStandard 함수는 Skinned, Instancing 에서도 사용될 수 있으므로,
//  별도의 PBR_PS.hlsli 같은 파일로 분리하는 것이 더 좋을 수 있음)

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    // 모든 계산 제거하고 그냥 빨간색 출력
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

float4 PSStandard2(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    // 1. Albedo 텍스처 샘플링 (또는 기본 Diffuse 색상 사용)
    float4 cAlbedoColor = float4(gMaterialInfo.DiffuseColor.rgb, 1.0f);
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
    {
        cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
    }
    // 알파 값 설정 (텍스처 또는 재질 값 사용)
    //cAlbedoColor.a = (gnTexturesMask & MATERIAL_ALBEDO_MAP) ? cAlbedoColor.a : gMaterialInfo.DiffuseColor.a;
    
    // 알파 값을 1로 강제하여 보이도록 함
    cAlbedoColor.a = 1.0f;

    // 2. 노멀 벡터 계산
    float3 normalW;
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        float4 cNormalSample = gtxtNormalTexture.Sample(gssWrap, input.uv);
        float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
        float3 vNormalMap = normalize(cNormalSample.rgb * 2.0f - 1.0f);
        normalW = normalize(mul(vNormalMap, TBN));
    }
    else
    {
       normalW = normalize(input.normalW);
    }

    
    // --- 3. 조명 계산 (수정된 Lighting 함수 호출) ---
    // cbGameObjectInfo (b2) 에서 gMaterialInfo를 가져와 전달
    float4 cIlluminationColor = Lighting(gMaterialInfo, input.positionW, normalW);

    // 4. 최종 색상 결정
    // Lighting() 결과가 이미 재질 색상이 반영된 조명값이므로,
    // 여기에 Emissive 또는 다른 효과를 추가할 수 있음

    // 예: Albedo 텍스처 색상을 조명 결과의 Diffuse/Ambient 요소에 곱해주는 방식 (물리 기반에 가까움)
    //     (이를 위해서는 Lighting 함수가 Ambient, Diffuse, Specular 요소를 분리 반환해야 함 - 현재 구조로는 어려움)

    // 예: 현재 Lighting 함수 출력을 그대로 사용하고 Emissive만 추가
    float4 cEmissiveColor = gMaterialInfo.EmissiveColor; // 기본 Emissive
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
    {
         // Emissive 텍스처를 샘플링하고 재질 색상과 조합 (예: 곱하기 또는 더하기)
        cEmissiveColor *= gtxtEmissionTexture.Sample(gssWrap, input.uv);
    }
    cIlluminationColor.rgb += cEmissiveColor.rgb; // Emissive 더하기
    cIlluminationColor.a = cAlbedoColor.a; // 최종 알파 설정

    return cIlluminationColor;
}


float4 PSStandard3(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
    clip(cAlbedoColor.a - 0.0000000000000001f);   // 알파 값이 0.1보다 작으면 그리기 중단
    
    float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
        cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
        cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_METALLIC_MAP)
        cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
    float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
        cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

    float3 normalW;
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
        float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        normalW = normalize(mul(vNormal, TBN));
    }
    else
    {
        normalW = normalize(input.normalW);
    }
    float4 cIlluminationColor = Lighting(gMaterialInfo, input.positionW, normalW);
    
    float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
    
    // 안개
    float distToEye = distance(input.positionW, gvCameraPosition.xyz);

    float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);

    float4 litColor = lerp(cColor, cIlluminationColor, 0.5f);
    
    litColor.rgb = lerp(gFogColor.rgb, cColor.rgb, fogFactor);
    
    float normalizedDistance = saturate(distToEye / (gFogStart + gFogRange));
    //return float4(normalizedDistance, normalizedDistance, normalizedDistance, 1.0f); // [수정된 디버깅 출력]

    
    // --- fogFactor 값 디버깅 ---
    //return float4(fogFactor, fogFactor, fogFactor, 1.0f); 
    
    cColor.rgb = lerp(cColor.rgb, gFogColor.rgb, normalizedDistance);
    //return float4(fogFactor, fogFactor, fogFactor, 1.0f);
    return (cColor);
}