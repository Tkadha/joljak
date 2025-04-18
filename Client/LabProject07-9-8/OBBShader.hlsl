// OBBShader.hlsl

// --- ��� ���� ---
// OBB �������� ��ȯ ��� (World * View * Projection)
cbuffer cbTransform : register(b0)
{
    matrix gmtxWVP;
};

// --- VS ����� ����ü ---
struct VS_OBB_INPUT
{
    float3 position : POSITION; // OBB ���� ��ġ (���� �Ǵ� ����)
};

struct VS_OBB_OUTPUT
{
    float4 position : SV_POSITION; // Ŭ�� ���� ��ġ
};

// --- Vertex Shader ---
VS_OBB_OUTPUT VSOBB(VS_OBB_INPUT input)
{
    VS_OBB_OUTPUT output;
    // ���� ��ȯ�� �̹� ����� �����̶�� �ٷ� WVP ����
    // ���� �����̶�� ���� ����� ���⼭ ���ϰų�, gmtxWVP ��ü�� ����->Ŭ�� ��ȯ ��ķ� ����
    output.position = mul(float4(input.position, 1.0f), gmtxWVP);
    return output;
}

// --- Pixel Shader ---
float4 PSOBB(VS_OBB_OUTPUT input) : SV_TARGET
{
    // �ܻ� ��� (��: ������)
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}