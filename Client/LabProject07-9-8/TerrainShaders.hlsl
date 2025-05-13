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
    float3 positionW : POSITION0; // ���� ���� ��ġ (�ȼ� ���̴� ���޿�)
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

// --- Vertex Shader ---
VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

   // 1. �� ���� ��ǥ�� ���� �������� ��ȯ
    float4 worldPos_H = mul(float4(input.position, 1.0f), gmtxGameObject);
    output.positionW = worldPos_H.xyz; // �ȼ� ���̴��� ������ ���� ��ġ

    // 2. ���� ���� ��ġ�� �� �� ���� ��ȯ�Ͽ� Ŭ�� ���� ��ġ ���
    float4 viewPos_H = mul(worldPos_H, gmtxView);
    output.position = mul(viewPos_H, gmtxProjection); // SV_POSITION�� Ŭ�� ���� ��ǥ


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
    //float4 cColor = input.color * lerp(cBaseTexColor, cDetailTexColor, 0.5); // ��: ���� ����� ����
    
    float4 cColor = input.color * lerp(cBaseTexColor, cDetailTexColor, 0.5);
    //�Ȱ�
    float distToEye = distance(input.positionW, gvCameraPosition.xyz);
        
    float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);
    
    
    float normalizedDistance = saturate(distToEye / (gFogStart + gFogRange));
    //return float4(normalizedDistance, normalizedDistance, normalizedDistance, 1.0f); // [������ ����� ���]

    
    // --- fogFactor �� ����� ---
    //return float4(fogFactor, fogFactor, fogFactor, 1.0f); 
    
    cColor.rgb = lerp(cColor.rgb, gFogColor.rgb, normalizedDistance);
    
    return cColor;
}