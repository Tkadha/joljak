// SkyboxShaders.hlsl

#include "Common.hlsl" // cbCameraInfo �ʿ�

// --- �ؽ�ó ---
TextureCube gtxtSkyCubeTexture : register(t13); // ��ī�̹ڽ� ť���

// --- VS ����� ����ü ---
struct VS_SKYBOX_INPUT
{
    float3 position : POSITION; // ���� ��ġ (���� ������ü)
};

struct VS_SKYBOX_OUTPUT
{
    float4 position : SV_POSITION; // Ŭ�� ���� ��ġ (z=w Ʈ�� �����)
    float3 positionL : POSITION; // ���� ��ġ (�ؽ�ó ���ø���)
};

// --- Vertex Shader ---
VS_SKYBOX_OUTPUT VSSkyBox(VS_SKYBOX_INPUT input)
{
    VS_SKYBOX_OUTPUT output;

    // ī�޶� �̵� ����
    float4x4 matViewNoTranslation = gmtxView;
    matViewNoTranslation._41_42_43 = float3(0.0f, 0.0f, 0.0f);

    // Ŭ�� ���� ��ȯ (���� ��ȯ ����)
    float4 viewPos = mul(float4(input.position, 1.0f), matViewNoTranslation);
    output.position = mul(viewPos, gmtxProjection);

    // ���� ���� �ִ�� ���� (z=w)
    output.position.z = output.position.w;

    // �ؽ�ó ���ø��� ���� ��ġ ����
    output.positionL = input.position;

    return output;
}

// --- Pixel Shader ---
float4 PSSkyBox(VS_SKYBOX_OUTPUT input) : SV_TARGET
{
    // ť��� ���ø� (gssClamp ���÷� ��� - register s1)
    float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, normalize(input.positionL));

    return cColor;
}