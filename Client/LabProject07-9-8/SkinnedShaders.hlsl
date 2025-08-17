// SkinnedShaders.hlsl

#include "Common.hlsl"
// Standard 셰이더의 구조체(VS_STANDARD_OUTPUT) 및 PS 함수를 사용한다면 포함
// #include "StandardShaders.hlsl" // 직접 포함보다는 필요한 구조체/함수만 Common이나 별도 파일로 분리 권장
// 여기서는 VS_STANDARD_OUTPUT 구조체 정의가 필요하다고 가정하고 아래에 복사

// --- 상수 버퍼 ---
cbuffer cbGameObjectInfo : register(b2) // Standard와 동일
{
    matrix gmtxGameObject;
    MaterialInfo gMaterialInfo;
    uint gnTexturesMask;
    // 패딩
};

cbuffer cbBoneOffsets : register(b7)
{
    matrix gpmtxBoneOffsets[256]; // 최대 본 개수
};

cbuffer cbBoneTransforms : register(b8)
{
    matrix gpmtxBoneTransforms[256]; // 최대 본 개수
};

// --- 텍스처 (Standard와 동일) ---
Texture2D gShadowMap : register(t3);
Texture2D gTorchShadowMap : register(t4);
Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
// ... (나머지 t9-t12) ...

// --- VS 입출력 구조체 ---
struct VS_SKINNED_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    int4 indices : BONEINDEX; // 영향을 주는 본 인덱스
    float4 weights : BONEWEIGHT; // 각 본의 가중치
};

// Standard 셰이더의 출력 구조체 (여기서도 사용)
struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
    
    float4 ShadowPosH : TEXCOORD1; // 그림자 좌표를 위한 공간 추가
};


// --- Vertex Shader ---
#define MAX_VERTEX_INFLUENCES 4 // 정점당 최대 영향 본 개수

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output = (VS_STANDARD_OUTPUT) 0; // 초기화

    matrix skinTransform = (matrix) 0.0f; // 최종 스키닝 변환 행렬

    // 각 영향 본에 대해 변환 행렬 계산 및 가중치 적용
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; ++i)
    {
        // 본 오프셋 행렬(메쉬->본 공간) * 최종 본 변환 행렬(애니메이션 적용된 월드)
        matrix boneMatrix = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
        skinTransform += input.weights[i] * boneMatrix;
    }

    // 스키닝 변환 적용 (월드 공간)
    output.positionW = mul(float4(input.position, 1.0f), skinTransform).xyz;
    // 노멀/탄젠트/바이탄젠트도 동일하게 변환 (정규화 필요)
    output.normalW = normalize(mul(input.normal, (float3x3) skinTransform));
    output.tangentW = normalize(mul(input.tangent, (float3x3) skinTransform));
    output.bitangentW = normalize(mul(input.bitangent, (float3x3) skinTransform));

    // 클립 공간 변환
    output.position = mul(float4(output.positionW, 1.0f), gmtxView);
    output.position = mul(output.position, gmtxProjection);

    output.uv = input.uv;
    
    output.ShadowPosH = mul(float4(output.positionW, 1.0f), gmtxShadowTransform);
    
    return output;
}

struct VS_SHADOW_OUTPUT
{
    float4 PosH : SV_POSITION;
};

VS_SHADOW_OUTPUT VSSkinnedAnimationShadow(VS_SKINNED_STANDARD_INPUT input)
{
    VS_SHADOW_OUTPUT output = (VS_SHADOW_OUTPUT) 0;
    
    matrix skinTransform = (matrix) 0.0f;
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; ++i)
    {
        matrix boneMatrix = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
        skinTransform += input.weights[i] * boneMatrix;
    }
    
    float4 skinnedPosW = mul(float4(input.position, 1.0f), skinTransform);
   
    float4x4 gLightViewProj = mul(gmtxView, gmtxProjection);
    
    output.PosH = mul(skinnedPosW, gLightViewProj);

    return output;
}

// --- Pixel Shader ---
// Standard 셰이더의 PSStandard 함수를 사용한다고 가정 (별도 정의 없음)
// 만약 스키닝 전용 PS가 필요하다면 여기에 정의