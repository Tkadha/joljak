// SkyboxShaders.hlsl

#include "Common.hlsl" // cbCameraInfo 필요

// --- 텍스처 ---
TextureCube gtxtSkyCubeTexture : register(t13); // 스카이박스 큐브맵

// --- VS 입출력 구조체 ---
struct VS_SKYBOX_INPUT
{
    float3 position : POSITION; // 정점 위치 (보통 정육면체)
};

struct VS_SKYBOX_OUTPUT
{
    float4 position : SV_POSITION; // 클립 공간 위치 (z=w 트릭 적용됨)
    float3 positionL : POSITION; // 로컬 위치 (텍스처 샘플링용)
};

// --- Vertex Shader ---
VS_SKYBOX_OUTPUT VSSkyBox(VS_SKYBOX_INPUT input)
{
    VS_SKYBOX_OUTPUT output;

    // 카메라 이동 제거
    float4x4 matViewNoTranslation = gmtxView;
    matViewNoTranslation._41_42_43 = float3(0.0f, 0.0f, 0.0f);

    // 클립 공간 변환 (월드 변환 없음)
    float4 viewPos = mul(float4(input.position, 1.0f), matViewNoTranslation);
    output.position = mul(viewPos, gmtxProjection);

    // 깊이 값을 최대로 설정 (z=w)
    output.position.z = output.position.w;

    // 텍스처 샘플링용 로컬 위치 전달
    output.positionL = input.position;

    return output;
}

// --- Pixel Shader ---
float4 PSSkyBox(VS_SKYBOX_OUTPUT input) : SV_TARGET
{
    // 큐브맵 샘플링 (gssClamp 샘플러 사용 - register s1)
    float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, normalize(input.positionL));

    return cColor;
}