Texture2D gSceneTexture : register(t0);
SamplerState gssWrap : register(s0);

cbuffer cbPostProcess : register(b1)
{
    float gHitEffectAmount; // �ǰ� ȿ�� ���� (0.0 ~ 1.0)
};

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD0;
};

VS_OUTPUT VSPostProcess(float3 pos : POSITION, float2 uv : TEXCOORD)
{
    VS_OUTPUT output;
    output.PosH = float4(pos, 1.0f);
    output.TexC = uv;
    return output;
}

float4 PSPostProcess(VS_OUTPUT input) : SV_TARGET
{
    //  ���� ���� ����
    float4 sceneColor = gSceneTexture.Sample(gssWrap, input.TexC);

    // ȭ�� �߾����κ��� �Ÿ� ���
    float distFromCenter = length(input.TexC - 0.5f);
    
    // �߾��� 0, �𼭸��� 1
    float vignette = saturate(distFromCenter * 2.0f);

    // �ǰ� ȿ�� ����
    float3 hitColor = float3(1.0f, 0.0f, 0.0f);

    // ���� ����� �ǰ� ���� ����
    float blendAmount = vignette * gHitEffectAmount;
    float3 finalColor = lerp(sceneColor.rgb, hitColor, blendAmount);

    return float4(finalColor, sceneColor.a);
}