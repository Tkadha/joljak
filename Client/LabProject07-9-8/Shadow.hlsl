// cbuffer와 리소스 바인딩 슬롯은 C++ 코드와 일치해야 합니다.
// CScene::Render의 Shadow Pass에서 카메라(b0), 오브젝트(b2)를 바인딩했습니다.

cbuffer cbCamera : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
};

cbuffer cbGameObject : register(b2)
{
    float4x4 gWorld;
    // 이 cbuffer에 다른 데이터가 있어도 여기서는 gWorld만 사용합니다.
};

struct VS_IN
{
    float3 PosL : POSITION;
};

struct VS_OUT
{
    float4 PosH : SV_POSITION;
};

// 정점 셰이더: 위치 변환만 수행
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;
    float4x4 gViewProj = mul(gView, gProj);
    vout.PosH = mul(mul(float4(vin.PosL, 1.0f), gWorld), gViewProj);
    return vout;
}

// 픽셀 셰이더: 아무 작업도 하지 않습니다.
// 깊이 값만 필요하므로 색상 출력은 없습니다.
void PS()
{
}