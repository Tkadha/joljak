// OBBShader.hlsl

// --- 상수 버퍼 ---
// OBB 렌더링용 변환 행렬 (World * View * Projection)
cbuffer cbTransform : register(b0)
{
    matrix gmtxWVP;
};

// --- VS 입출력 구조체 ---
struct VS_OBB_INPUT
{
    float3 position : POSITION; // OBB 정점 위치 (로컬 또는 월드)
};

struct VS_OBB_OUTPUT
{
    float4 position : SV_POSITION; // 클립 공간 위치
};

// --- Vertex Shader ---
VS_OBB_OUTPUT VSOBB(VS_OBB_INPUT input)
{
    VS_OBB_OUTPUT output;
    // 월드 변환이 이미 적용된 정점이라면 바로 WVP 곱함
    // 로컬 정점이라면 월드 행렬을 여기서 곱하거나, gmtxWVP 자체를 로컬->클립 변환 행렬로 전달
    output.position = mul(float4(input.position, 1.0f), gmtxWVP);
    return output;
}

// --- Pixel Shader ---
float4 PSOBB(VS_OBB_OUTPUT input) : SV_TARGET
{
    // 단색 출력 (예: 빨간색)
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}