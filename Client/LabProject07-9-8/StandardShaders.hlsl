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
    // 1. Albedo 텍스처 샘플링 (또는 기본 Diffuse 색상 사용)
    float4 cAlbedoColor = float4(gMaterialInfo.DiffuseColor.rgb, 1.0f);
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
    {
        cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
        // 텍스처 알파값 사용 또는 Diffuse 알파 사용 결정
        // cAlbedoColor.a = gMaterialInfo.DiffuseColor.a;
    }
    else
    {
        cAlbedoColor.a = gMaterialInfo.DiffuseColor.a; // 텍스처 없으면 재질 알파 사용
    }


    // 2. 노멀 벡터 계산 (노멀맵 적용)
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

    // 3. 조명 계산 (Light.hlsl 함수 호출)
    // Lighting 함수는 gMaterialInfo를 사용하여 최종 조명 색상(Ambient+Diffuse+Specular)을 계산
    float4 cIlluminationColor = Lighting(input.positionW, normalW);

    // 4. 최종 색상 결정
    // Lighting() 결과는 이미 재질 색상이 반영된 조명값이므로, Albedo와 곱하는 것이 아니라
    // Albedo 텍스처 값은 Lighting() 함수 내부에서 gMaterialInfo.DiffuseColor 대신 사용되도록
    // Lighting() 함수를 수정하거나, 여기서 최종 조합 방식을 결정해야 합니다.
    // 현재 Lighting() 함수 구조를 유지한다면, 결과는 이미 조명+재질이므로 그대로 사용하거나,
    // 여기에 Emissive 텍스처/색상을 추가하는 정도가 일반적입니다.
    // float4 cEmissiveColor = float4(0,0,0,0);
    // if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissiveColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
    // cIlluminationColor.rgb += gMaterialInfo.EmissiveColor.rgb + cEmissiveColor.rgb;

    // 알파값은 Albedo 또는 재질의 값을 사용
    // cIlluminationColor.a = cAlbedoColor.a;

    // 기존 코드의 Lerp 유지 시 (임시)
    // return lerp(cAlbedoColor, cIlluminationColor, 0.5f);

    // 예시: 조명 계산 결과에 Emissive 추가 후 반환
    cIlluminationColor.rgb += gMaterialInfo.EmissiveColor.rgb; // 재질 Emissive 더하기
    cIlluminationColor.a = cAlbedoColor.a; // Albedo/재질 알파 사용
    return cIlluminationColor;
}