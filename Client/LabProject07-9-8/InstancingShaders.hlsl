// InstancingShaders.hlsl

#include "Common.hlsl" // ���� ��� (cbCameraInfo ��)
// PSStandard ��� ���� �� ���� ���� �ʿ� (���� ���� �и� ����)
//#include "PBR_PS.hlsli" // ����: PSStandard �� ���� ��� ���� ����

// --- ��� ���� ---
// cbCameraInfo (b1) - Common �� ����
// cbLights (b4) - Common �� ����
// cbGameObjectInfo (b2) �� ��� �� �� (�ν��Ͻ��� ��ȯ ���)

// --- �ؽ�ó (Standard �� ����) ---
Texture2D gtxtAlbedoTexture : register(t6);
// ... (t7-t12) ...

// --- VS ����� ����ü ---
struct VS_INSTANCING_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    // �ν��Ͻ��� ������ (�Է� ���� 1�� ��� ����)
    matrix instanceTransform : INSTANCE_TRANSFORM; // Semantic �̸��� Input Layout�� ��ġ�ؾ� ��
};

// ����� Standard �� �����ϴٰ� ����
struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

// --- Vertex Shader ---
VS_STANDARD_OUTPUT VSInstancing(VS_INSTANCING_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    // �ν��Ͻ��� ���� ��ȯ ����
    matrix worldMatrix = input.instanceTransform;
    output.positionW = mul(float4(input.position, 1.0f), worldMatrix).xyz;
    output.normalW = normalize(mul(input.normal, (float3x3) worldMatrix));
    output.tangentW = normalize(mul(input.tangent, (float3x3) worldMatrix));
    output.bitangentW = normalize(mul(input.bitangent, (float3x3) worldMatrix));

    // Ŭ�� ���� ��ȯ
    output.position = mul(float4(output.positionW, 1.0f), gmtxView);
    output.position = mul(output.position, gmtxProjection);

    output.uv = input.uv;

    return output;
}

// --- Pixel Shader ---
// PSStandard ���� ����
// float4 PSInstancing(VS_STANDARD_OUTPUT input) : SV_TARGET { ... }