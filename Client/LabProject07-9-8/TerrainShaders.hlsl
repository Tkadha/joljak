// TerrainShaders.hlsl

#include "Common.hlsl"

// --- ��� ���� ---
cbuffer cbGameObjectInfo : register(b2) // ������ ���� ��ȯ � ���
{
    matrix gmtxGameObject;
    MaterialInfo gMaterialInfo; // ���������� ��� �� �� ���� ����
    uint gnTexturesMask;
    // �е�
};

// --- �ؽ�ó ---
Texture2D gtxtTerrainBaseTexture : register(t1); // ���� �⺻ �ؽ�ó
Texture2D gtxtTerrainDetailTexture : register(t2); // ���� �� �ؽ�ó

// --- VS ����� ����ü ---
struct VS_TERRAIN_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR; // ���� ���� (������ ��)
    float2 uv0 : TEXCOORD0; // Base �ؽ�ó UV
    float2 uv1 : TEXCOORD1; // Detail �ؽ�ó UV
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

    // Ŭ�� ���� ��ȯ
    output.position = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.position = mul(output.position, gmtxView);
    output.position = mul(output.position, gmtxProjection);

    output.color = input.color; // ���� ���� ����
    output.uv0 = input.uv0; // UV ��ǥ ����
    output.uv1 = input.uv1;

    return output;
}

// --- Pixel Shader ---
float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
    // �ؽ�ó ���ø�
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
    float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);

    // ���� ���� (��: ����)
    // float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f)); // �ܼ� ���
    float4 cColor = input.color * lerp(cBaseTexColor, cDetailTexColor, 0.5); // ��: ���� ����� ����

    return cColor;
}