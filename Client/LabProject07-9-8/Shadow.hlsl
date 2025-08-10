// Shadow.hlsl

#include "Common.hlsl" // ���� ����ü�� ����� ����ϱ� ���� �����մϴ�.

// --- ��� ���� �� ���ҽ� ---
// C++�� CShadowShader::CreateRootSignature�� ���� ��ȣ�� ��ġ�ؾ� �մϴ�.
// b0: �� ������ ī�޶�(View, Projection) ��� (���� ���̴� ����)
// b2: ���� ������Ʈ ���� (���� ���, �ؽ�ó ��� ���� ����ũ)
// t6: Albedo �ؽ�ó (���� Ŭ���ο�)

cbuffer cbCamera : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
};

cbuffer cbGameObjectInfo : register(b2)
{
    float4x4 gmtxGameObject;
    MaterialInfo gMaterialInfo;
    uint gnTexturesMask; // �ؽ�ó ��� ����
};

Texture2D gtxtAlbedoTexture : register(t6);


struct VS_IN
{
    float3 PosL : POSITION;
    float2 uv : TEXCOORD; // �ؽ�ó ��ǥ(UV)�� �Է¹޽��ϴ�.
};

struct VS_OUT
{
    float4 PosH : SV_POSITION;
    float2 uv : TEXCOORD; // �ȼ� ���̴��� UV ��ǥ�� �Ѱ��ݴϴ�.
};


VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;
    
    float4x4 gViewProj = mul(gView, gProj);
    vout.PosH = mul(mul(float4(vin.PosL, 1.0f), gmtxGameObject), gViewProj);
    
    vout.uv = vin.uv;
    
    return vout;
}


void PS(VS_OUT pin)
{
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
    {
        float4 cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, pin.uv);
        
        clip(cAlbedoColor.a - 0.000000000000000001f);
    }
    
}