// Shadow.hlsl

#include "Common.hlsl" // 공통 구조체와 상수를 사용하기 위해 포함합니다.

// --- 상수 버퍼 및 리소스 ---
// C++의 CShadowShader::CreateRootSignature와 슬롯 번호가 일치해야 합니다.
// b0: 빛 시점의 카메라(View, Projection) 행렬 (정점 셰이더 전용)
// b2: 게임 오브젝트 정보 (월드 행렬, 텍스처 사용 여부 마스크)
// t6: Albedo 텍스처 (알파 클리핑용)

cbuffer cbCamera : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
};

cbuffer cbGameObjectInfo : register(b2)
{
    float4x4 gmtxGameObject;
    MaterialInfo gMaterialInfo;
    uint gnTexturesMask; // 텍스처 사용 여부
};

Texture2D gtxtAlbedoTexture : register(t6);


struct VS_IN
{
    float3 PosL : POSITION;
    float2 uv : TEXCOORD; // 텍스처 좌표(UV)를 입력받습니다.
};

struct VS_OUT
{
    float4 PosH : SV_POSITION;
    float2 uv : TEXCOORD; // 픽셀 셰이더로 UV 좌표를 넘겨줍니다.
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