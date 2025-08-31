Texture2D gSceneTexture : register(t0);
SamplerState gssWrap : register(s0);

cbuffer cbPostProcess : register(b1)
{
    float gHitEffectAmount; // 피격 효과 강도 (0.0 ~ 1.0)
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
    //  원본 씬의 색상
    float4 sceneColor = gSceneTexture.Sample(gssWrap, input.TexC);

    // 화면 중앙으로부터 거리 계산
    float distFromCenter = length(input.TexC - 0.5f);
    
    // 중앙은 0, 모서리는 1
    float vignette = saturate(distFromCenter * 2.0f);

    // 피격 효과 색상
    float3 hitColor = float3(1.0f, 0.0f, 0.0f);

    // 원본 색상과 피격 색상 섞음
    float blendAmount = vignette * gHitEffectAmount;
    float3 finalColor = lerp(sceneColor.rgb, hitColor, blendAmount);

    return float4(finalColor, sceneColor.a);
}