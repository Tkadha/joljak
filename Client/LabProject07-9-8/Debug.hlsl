Texture2D gDebugTexture : register(t0); // 디버깅할 텍스처를 t0 슬롯에서 받음
SamplerState gssPoint : register(s0); // 가장 기본적인 점 샘플러 사용

struct VS_IN
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
};

struct VS_OUT
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

// 정점 셰이더: 2D 사각형의 정점 위치와 UV를 그대로 전달
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;
    vout.PosH = float4(vin.PosL, 1.0f);
    vout.TexC = vin.TexC;
    return vout;
}

// 픽셀 셰이더: 텍스처를 샘플링해서 화면에 출력
float4 PS(VS_OUT pin) : SV_TARGET
{
    // 그림자 맵은 깊이 값(R 채널)만 가지고 있으므로, R 채널 값을 R,G,B에 모두 넣어 흑백으로 표시
    float depth = gDebugTexture.Sample(gssPoint, pin.TexC).r;
    return float4(depth, depth, depth, 1.0f);
}