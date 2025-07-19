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
    // �ؽ�ó ���ø�
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
    float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);

    // ���� ���� (��: ����)
    // float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f)); // �ܼ� ���
    //float4 cColor = input.color * lerp(cBaseTexColor, cDetailTexColor, 0.5); // ��: ���� ����� ����
    
    float4 cTextureColor = lerp(cBaseTexColor, cDetailTexColor, 0.5);

     // 1. �׸��� ��� ���
    float shadowFactor = 0.15; // �⺻���� �׸��� ����
    if (gIsDaytime) 
    {
        shadowFactor = CalcShadowFactor(input.ShadowPosH);
    }
    
// ���� ���� ��� (input.color�� �̸� ���� ��, ���⿡ �׸��ڸ� ����)
    float3 totalLight = gcGlobalAmbientLight.rgb + (shadowFactor * input.color.rgb);

    // ���� ���� ����
    float3 finalColor = cTextureColor.rgb * totalLight;
    
    //�Ȱ�
    //float distToEye = distance(input.positionW, gvCameraPosition.xyz);        
    //float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);    
    //float normalizedDistance = saturate(distToEye / (gFogStart + gFogRange));
    
    
    if (gIsDaytime)
    {
        float distToEye = distance(input.positionW, gvCameraPosition.xyz);
        float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);
        finalColor = lerp(gFogColor.rgb, finalColor, fogFactor);
    }
    
    return float4(finalColor, cTextureColor.a);
}