// StandardShader.hlsl

#include "Common.hlsl" // ���� ��� ����

// --- ��� ���� ---

// ���� ������Ʈ ���� (Standard, Skinned, Terrain ��� ���)
cbuffer cbGameObjectInfo : register(b2)
{
    matrix gmtxGameObject; // ���� ��ȯ
    MaterialInfo gMaterialInfo; // ���� ����
    uint gnTexturesMask; // �ؽ�ó ��� ����ũ
    // �ʿ�� �е�
};

// --- �ؽ�ó (Standard, Skinned, Instancing ���� ���) ---
Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

// --- VS ����� ����ü ---
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
    float4 position : SV_POSITION; // Ŭ�� ���� ��ġ
    float3 positionW : POSITION; // ���� ���� ��ġ
    float3 normalW : NORMAL; // ���� ���� ���
    float3 tangentW : TANGENT; // ���� ���� ź��Ʈ
    float3 bitangentW : BITANGENT; // ���� ���� ����ź��Ʈ
    float2 uv : TEXCOORD; // UV ��ǥ
};

// --- Vertex Shader ---
VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    // ���� ���� ��ȯ
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    output.normalW = normalize(mul(input.normal, (float3x3) gmtxGameObject));
    output.tangentW = normalize(mul(input.tangent, (float3x3) gmtxGameObject));
    output.bitangentW = normalize(mul(input.bitangent, (float3x3) gmtxGameObject));

    // Ŭ�� ���� ��ȯ
    output.position = mul(float4(output.positionW, 1.0f), gmtxView);
    output.position = mul(output.position, gmtxProjection);

    output.uv = input.uv;

    return output;
}

// --- Pixel Shader ---
// (PSStandard �Լ��� Skinned, Instancing ������ ���� �� �����Ƿ�,
//  ������ PBR_PS.hlsli ���� ���Ϸ� �и��ϴ� ���� �� ���� �� ����)

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    // ��� ��� �����ϰ� �׳� ������ ���
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

float4 PSStandard2(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    // 1. Albedo �ؽ�ó ���ø� (�Ǵ� �⺻ Diffuse ���� ���)
    float4 cAlbedoColor = float4(gMaterialInfo.DiffuseColor.rgb, 1.0f);
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
    {
        cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
    }
    // ���� �� ���� (�ؽ�ó �Ǵ� ���� �� ���)
    //cAlbedoColor.a = (gnTexturesMask & MATERIAL_ALBEDO_MAP) ? cAlbedoColor.a : gMaterialInfo.DiffuseColor.a;
    
    // ���� ���� 1�� �����Ͽ� ���̵��� ��
    cAlbedoColor.a = 1.0f;

    // 2. ��� ���� ���
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

    
    // --- 3. ���� ��� (������ Lighting �Լ� ȣ��) ---
    // cbGameObjectInfo (b2) ���� gMaterialInfo�� ������ ����
    float4 cIlluminationColor = Lighting(gMaterialInfo, input.positionW, normalW);

    // 4. ���� ���� ����
    // Lighting() ����� �̹� ���� ������ �ݿ��� �����̹Ƿ�,
    // ���⿡ Emissive �Ǵ� �ٸ� ȿ���� �߰��� �� ����

    // ��: Albedo �ؽ�ó ������ ���� ����� Diffuse/Ambient ��ҿ� �����ִ� ��� (���� ��ݿ� �����)
    //     (�̸� ���ؼ��� Lighting �Լ��� Ambient, Diffuse, Specular ��Ҹ� �и� ��ȯ�ؾ� �� - ���� �����δ� �����)

    // ��: ���� Lighting �Լ� ����� �״�� ����ϰ� Emissive�� �߰�
    float4 cEmissiveColor = gMaterialInfo.EmissiveColor; // �⺻ Emissive
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
    {
         // Emissive �ؽ�ó�� ���ø��ϰ� ���� ����� ���� (��: ���ϱ� �Ǵ� ���ϱ�)
        cEmissiveColor *= gtxtEmissionTexture.Sample(gssWrap, input.uv);
    }
    cIlluminationColor.rgb += cEmissiveColor.rgb; // Emissive ���ϱ�
    cIlluminationColor.a = cAlbedoColor.a; // ���� ���� ����

    return cIlluminationColor;
}


float4 PSStandard3(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
    clip(cAlbedoColor.a - 0.0000000000000001f);   // ���� ���� 0.1���� ������ �׸��� �ߴ�
    
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
        float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
        normalW = normalize(mul(vNormal, TBN));
    }
    else
    {
        normalW = normalize(input.normalW);
    }
    float4 cIlluminationColor = Lighting(gMaterialInfo, input.positionW, normalW);
    
    float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
    
    // �Ȱ�
    float distToEye = distance(input.positionW, gvCameraPosition.xyz);

    float fogFactor = saturate((gFogStart + gFogRange - distToEye) / gFogRange);

    float4 litColor = lerp(cColor, cIlluminationColor, 0.5f);
    
    litColor.rgb = lerp(gFogColor.rgb, cColor.rgb, fogFactor);
    
    float normalizedDistance = saturate(distToEye / (gFogStart + gFogRange));
    //return float4(normalizedDistance, normalizedDistance, normalizedDistance, 1.0f); // [������ ����� ���]

    
    // --- fogFactor �� ����� ---
    //return float4(fogFactor, fogFactor, fogFactor, 1.0f); 
    
    cColor.rgb = lerp(cColor.rgb, gFogColor.rgb, normalizedDistance);
    //return float4(fogFactor, fogFactor, fogFactor, 1.0f);
    return (cColor);
}