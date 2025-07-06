#include "Common.hlsl"

// --- 입력 리소스 ---
// G-Buffer 텍스처들과 그림자 맵, 총 5개의 텍스처를 입력으로 받습니다.
Texture2D gtxtWorldPos : register(t13); // G-Buffer 0: 월드 좌표 (C++에서 바인딩)
Texture2D gtxtNormal : register(t14); // G-Buffer 1: 노멀
Texture2D gtxtAlbedo : register(t15); // G-Buffer 2: 알베도 (색상)
Texture2D gtxtMaterial : register(t16); // G-Buffer 3: 재질
Texture2D gShadowMap : register(t3); // 그림자 맵 (기존 슬롯 사용)

// 그림자 계수를 계산하는 함수
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

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}


struct VS_OUT
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

// 정점 셰이더: 화면 전체를 덮는 삼각형을 그립니다.
VS_OUT VS(uint vertId : SV_VertexID)
{
    VS_OUT output;
    output.TexC = float2((vertId << 1) & 2, vertId & 2);
    output.PosH = float4(output.TexC * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

// 픽셀 셰이더: G-Buffer를 읽어와 최종 조명을 계산합니다.
float4 PS(VS_OUT pin) : SV_TARGET
{
    // 1. G-Buffer에서 현재 픽셀의 재료 정보들을 읽어옵니다.
    float3 worldPos = gtxtWorldPos.Sample(gssWrap, pin.TexC).rgb;
    float3 normal = gtxtNormal.Sample(gssWrap, pin.TexC).rgb;
    float4 albedo = gtxtAlbedo.Sample(gssWrap, pin.TexC);
    float4 material = gtxtMaterial.Sample(gssWrap, pin.TexC);

    // 2. 저장시 0~1로 변환했던 노멀 벡터를 다시 -1~1 범위로 복원합니다.
    normal = normalize(normal * 2.0f - 1.0f);

    // 3. 그림자 좌표를 계산하고, 그림자 계수를 구합니다.
    //    (이전 Standard 셰이더의 로직과 완전히 동일합니다)
    float4 shadowPosH = mul(float4(worldPos, 1.0f), gmtxShadowTransform);
    float shadowFactor = CalcShadowFactor(shadowPosH);

    // 4. 최종 조명을 계산합니다.
    //    (이전 Standard 셰이더의 로직과 완전히 동일합니다)
    float4 illumination = Lighting(MaterialInfo, worldPos, normal); // gMaterialInfo는 임시값
    float3 totalLight = gLights[0].m_cAmbient.rgb + (shadowFactor * illumination.rgb);

    // 5. 최종 색상을 조합하여 반환합니다.
    float3 finalColor = albedo.rgb * totalLight;
    
    // (이곳에 안개 효과를 추가할 수 있습니다)

    return float4(finalColor, 1.0f);
}