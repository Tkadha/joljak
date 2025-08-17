// SkinnedShaders.hlsl

#include "Common.hlsl"
// Standard ���̴��� ����ü(VS_STANDARD_OUTPUT) �� PS �Լ��� ����Ѵٸ� ����
// #include "StandardShaders.hlsl" // ���� ���Ժ��ٴ� �ʿ��� ����ü/�Լ��� Common�̳� ���� ���Ϸ� �и� ����
// ���⼭�� VS_STANDARD_OUTPUT ����ü ���ǰ� �ʿ��ϴٰ� �����ϰ� �Ʒ��� ����

// --- ��� ���� ---
cbuffer cbGameObjectInfo : register(b2) // Standard�� ����
{
    matrix gmtxGameObject;
    MaterialInfo gMaterialInfo;
    uint gnTexturesMask;
    // �е�
};

cbuffer cbBoneOffsets : register(b7)
{
    matrix gpmtxBoneOffsets[256]; // �ִ� �� ����
};

cbuffer cbBoneTransforms : register(b8)
{
    matrix gpmtxBoneTransforms[256]; // �ִ� �� ����
};

// --- �ؽ�ó (Standard�� ����) ---
Texture2D gShadowMap : register(t3);
Texture2D gTorchShadowMap : register(t4);
Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
// ... (������ t9-t12) ...

// --- VS ����� ����ü ---
struct VS_SKINNED_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    int4 indices : BONEINDEX; // ������ �ִ� �� �ε���
    float4 weights : BONEWEIGHT; // �� ���� ����ġ
};

// Standard ���̴��� ��� ����ü (���⼭�� ���)
struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
    
    float4 ShadowPosH : TEXCOORD1; // �׸��� ��ǥ�� ���� ���� �߰�
};


// --- Vertex Shader ---
#define MAX_VERTEX_INFLUENCES 4 // ������ �ִ� ���� �� ����

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output = (VS_STANDARD_OUTPUT) 0; // �ʱ�ȭ

    matrix skinTransform = (matrix) 0.0f; // ���� ��Ű�� ��ȯ ���

    // �� ���� ���� ���� ��ȯ ��� ��� �� ����ġ ����
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; ++i)
    {
        // �� ������ ���(�޽�->�� ����) * ���� �� ��ȯ ���(�ִϸ��̼� ����� ����)
        matrix boneMatrix = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
        skinTransform += input.weights[i] * boneMatrix;
    }

    // ��Ű�� ��ȯ ���� (���� ����)
    output.positionW = mul(float4(input.position, 1.0f), skinTransform).xyz;
    // ���/ź��Ʈ/����ź��Ʈ�� �����ϰ� ��ȯ (����ȭ �ʿ�)
    output.normalW = normalize(mul(input.normal, (float3x3) skinTransform));
    output.tangentW = normalize(mul(input.tangent, (float3x3) skinTransform));
    output.bitangentW = normalize(mul(input.bitangent, (float3x3) skinTransform));

    // Ŭ�� ���� ��ȯ
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
// Standard ���̴��� PSStandard �Լ��� ����Ѵٰ� ���� (���� ���� ����)
// ���� ��Ű�� ���� PS�� �ʿ��ϴٸ� ���⿡ ����