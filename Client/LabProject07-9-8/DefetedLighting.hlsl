#include "Common.hlsl"

// --- �Է� ���ҽ� ---
// G-Buffer �ؽ�ó��� �׸��� ��, �� 5���� �ؽ�ó�� �Է����� �޽��ϴ�.
Texture2D gtxtWorldPos : register(t13); // G-Buffer 0: ���� ��ǥ (C++���� ���ε�)
Texture2D gtxtNormal : register(t14); // G-Buffer 1: ���
Texture2D gtxtAlbedo : register(t15); // G-Buffer 2: �˺��� (����)
Texture2D gtxtMaterial : register(t16); // G-Buffer 3: ����
Texture2D gShadowMap : register(t3); // �׸��� �� (���� ���� ���)

// �׸��� ����� ����ϴ� �Լ�
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

// ���� ���̴�: ȭ�� ��ü�� ���� �ﰢ���� �׸��ϴ�.
VS_OUT VS(uint vertId : SV_VertexID)
{
    VS_OUT output;
    output.TexC = float2((vertId << 1) & 2, vertId & 2);
    output.PosH = float4(output.TexC * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

// �ȼ� ���̴�: G-Buffer�� �о�� ���� ������ ����մϴ�.
float4 PS(VS_OUT pin) : SV_TARGET
{
    // 1. G-Buffer���� ���� �ȼ��� ��� �������� �о�ɴϴ�.
    float3 worldPos = gtxtWorldPos.Sample(gssWrap, pin.TexC).rgb;
    float3 normal = gtxtNormal.Sample(gssWrap, pin.TexC).rgb;
    float4 albedo = gtxtAlbedo.Sample(gssWrap, pin.TexC);
    float4 material = gtxtMaterial.Sample(gssWrap, pin.TexC);

    // 2. ����� 0~1�� ��ȯ�ߴ� ��� ���͸� �ٽ� -1~1 ������ �����մϴ�.
    normal = normalize(normal * 2.0f - 1.0f);

    // 3. �׸��� ��ǥ�� ����ϰ�, �׸��� ����� ���մϴ�.
    //    (���� Standard ���̴��� ������ ������ �����մϴ�)
    float4 shadowPosH = mul(float4(worldPos, 1.0f), gmtxShadowTransform);
    float shadowFactor = CalcShadowFactor(shadowPosH);

    // 4. ���� ������ ����մϴ�.
    //    (���� Standard ���̴��� ������ ������ �����մϴ�)
    float4 illumination = Lighting(MaterialInfo, worldPos, normal); // gMaterialInfo�� �ӽð�
    float3 totalLight = gLights[0].m_cAmbient.rgb + (shadowFactor * illumination.rgb);

    // 5. ���� ������ �����Ͽ� ��ȯ�մϴ�.
    float3 finalColor = albedo.rgb * totalLight;
    
    // (�̰��� �Ȱ� ȿ���� �߰��� �� �ֽ��ϴ�)

    return float4(finalColor, 1.0f);
}