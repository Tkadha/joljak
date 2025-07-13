// Shaders.hlsl 파일 맨 아래에 추가

#include "Common.hlsl"
#include "Light.hlsl"

struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
};

cbuffer cbGameObjectInfo : register(b2)
{
    matrix gmtxGameObject : packoffset(c0);
    MATERIAL gMaterial : packoffset(c4);
    uint gnTexturesMask : packoffset(c8);
};

struct VS_WAVES_OUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD0;
};

// Waves Vertex Shader
VS_WAVES_OUT VS(VS_IN In)
{
    VS_WAVES_OUT Out = (VS_WAVES_OUT) 0;
    
    // 로컬 좌표계의 정점 위치와 법선을 월드 공간으로 변환
    Out.PosW = mul(float4(In.PosL, 1.0f), gmtxGameObject).xyz;
    Out.NormalW = mul(In.NormalL, (float3x3) gmtxGameObject);
    
    // 텍스처 좌표는 그대로 전달
    Out.TexC = In.TexC;
    
    // 최종적으로 월드 좌표를 뷰와 프로젝션 행렬을 곱해 클립 공간 좌표로 변환
    Out.PosH = mul(float4(Out.PosW, 1.0f), gmtxView);
    Out.PosH = mul(Out.PosH, gmtxProjection);
    
    return Out;
}

// Waves Pixel Shader
float4 PS(VS_WAVES_OUT In) : SV_Target
{
    // 법선 벡터를 정규화
    In.NormalW = normalize(In.NormalW);
    
    // 픽셀에서 카메라(눈)를 향하는 벡터 계산
    float3 toEyeW = normalize(gvCameraPosition - In.PosW);

    // 조명 계산을 위한 변수 초기화
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // 장면에 있는 모든 빛에 대해 조명 계산 (StandardShader와 유사)
    // 여기서는 간단하게 주 방향성 광원(2번 인덱스)만 계산
    float3 lightVec = -gLights[2].m_vDirection;
    float ndotl = max(0.0f, dot(lightVec, In.NormalW));
    
    diffuse += ndotl * gLights[2].m_cDiffuse;
    
    float3 reflectVec = reflect(-lightVec, In.NormalW);
    float specFactor = pow(max(0.0f, dot(reflectVec, toEyeW)), 32.0f);
    spec += specFactor * gLights[2].m_cSpecular;

    // 물의 기본 색상과 투명도 설정
    float4 waterColor = float4(0.1f, 0.3f, 0.6f, 0.7f); // 푸른색, 70% 불투명도
    
    // 최종 색상 = (물 색상 * (전역 주변광 + 난반사광)) + 정반사광
    float4 litColor = waterColor * (gcGlobalAmbientLight + diffuse) + spec;
    
    // 알파 값은 물의 기본 알파 값을 사용
    litColor.a = waterColor.a;

    return litColor;
}