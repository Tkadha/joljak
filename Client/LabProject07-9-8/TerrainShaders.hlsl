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

Texture2D gShadowMap : register(t3);    // �׸���

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
    float4 ShadowPosH : TEXCOORD2;
};

// --- Vertex Shader ---
VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

   // 1. input.position�� �̹� ���� ��ǥ
    float4 worldPos_H = float4(input.position, 1.0f);
    output.positionW = worldPos_H.xyz; // �ȼ� ���̴��� ������ ���� ��ġ

    // 2. ���� ���� ��ġ�� �� �� ���� ��ȯ�Ͽ� Ŭ�� ���� ��ġ ���
    float4 viewPos_H = mul(worldPos_H, gmtxView);
    output.position = mul(viewPos_H, gmtxProjection); // SV_POSITION�� Ŭ�� ���� ��ǥ
    
    // 3. �׸��� �������� ��ȯ
    output.ShadowPosH = mul(worldPos_H, gmtxShadowTransform);
    
    output.color = input.color;
    output.uv0 = input.uv0;
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
    //------------�����------------
    
    // ���� ���̴����� �Ѿ�� ���� ��ǥ(input.ShadowPosH)��
    // ���� �ִ� ũ��(��: 5000.0)�� ������ [0,1] ������ �������� ����ϴ�.
    // �� ���� �����ϸ� �׽�Ʈ�غ�����.
    //float3 worldPosColor = input.ShadowPosH.xyz / 10000.0f;

    //// x, y, z ��ǥ�� ���� R, G, B ���� �����Ͽ� ����մϴ�.
    //return float4(worldPosColor.r, worldPosColor.g, worldPosColor.b, 1.0f);
    
    //return float4(input.uv0.x, input.uv0.y, input.uv1.x, 1.0f);
    //------------�����------------
    
    
    // �ؽ�ó ���ø�
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
    float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);

    // ���� ���� (��: ����)
    // float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f)); // �ܼ� ���
    //float4 cColor = input.color * lerp(cBaseTexColor, cDetailTexColor, 0.5); // ��: ���� ����� ����
    
    float4 cTextureColor = lerp(cBaseTexColor, cDetailTexColor, 0.5);

     // 1. �׸��� ��� ���
    float shadowFactor = CalcShadowFactor(input.ShadowPosH);
    
// ���� ���� ��� (input.color�� �̸� ���� ��, ���⿡ �׸��ڸ� ����)
    float3 totalLight = float3(0.3f, 0.3f, 0.3f) + (shadowFactor * input.color.rgb);

    // ���� ���� ����
    float3 finalColor = cTextureColor.rgb * totalLight;
    
    //�Ȱ�
    //float distToEye = distance(input.positionW, gvCameraPosition.xyz);        
    //float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);    
    //float normalizedDistance = saturate(distToEye / (gFogStart + gFogRange));
    
    float distToEye = distance(input.positionW, gvCameraPosition.xyz);
    float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);
    finalColor = lerp(gFogColor.rgb, finalColor, fogFactor);

    
    return float4(finalColor, cTextureColor.a);
}