// InstancingShaders.hlsl

#include "Common.hlsl" // 공통 요소 (cbCameraInfo 등)
// PSStandard 사용 가정 시 관련 정의 필요 (별도 파일 분리 권장)
//#include "PBR_PS.hlsli" // 예시: PSStandard 및 관련 요소 포함 가정

// --- 상수 버퍼 ---
// cbCameraInfo (b1) - Common 에 있음
// cbLights (b4) - Common 에 있음
// cbGameObjectInfo (b2) 는 사용 안 함 (인스턴스별 변환 사용)

// --- 텍스처 (Standard 와 동일) ---
Texture2D gtxtAlbedoTexture : register(t6);
// ... (t7-t12) ...

// --- VS 입출력 구조체 ---
struct VS_INSTANCING_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    // 인스턴스별 데이터 (입력 슬롯 1번 사용 가정)
    matrix instanceTransform : INSTANCE_TRANSFORM; // Semantic 이름은 Input Layout과 일치해야 함
};

// 출력은 Standard 와 동일하다고 가정
struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

// --- Vertex Shader ---
VS_STANDARD_OUTPUT VSInstancing(VS_INSTANCING_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    // 인스턴스별 월드 변환 적용
    matrix worldMatrix = input.instanceTransform;
    output.positionW = mul(float4(input.position, 1.0f), worldMatrix).xyz;
    output.normalW = normalize(mul(input.normal, (float3x3) worldMatrix));
    output.tangentW = normalize(mul(input.tangent, (float3x3) worldMatrix));
    output.bitangentW = normalize(mul(input.bitangent, (float3x3) worldMatrix));

    // 클립 공간 변환
    output.position = mul(float4(output.positionW, 1.0f), gmtxView);
    output.position = mul(output.position, gmtxProjection);

    output.uv = input.uv;

    return output;
}

// --- Pixel Shader ---
// PSStandard 재사용 가정
// float4 PSInstancing(VS_STANDARD_OUTPUT input) : SV_TARGET { ... }