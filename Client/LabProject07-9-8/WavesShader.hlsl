// Shaders.hlsl 파일 맨 아래에 추가

#include "Common.hlsl"

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

struct VS_IN
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD0;
};

struct VS_OUT
{
    float4 PosH : SV_POSITION; // 최종 화면 좌표
    float3 PosW : POSITION; // 월드 좌표
    float3 NormalW : NORMAL; // 월드 공간 법선
    float2 TexC : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1; // 그림자 맵 좌표
};
// 1. 텍스처 리소스 선언
// 컴퓨트 셰이더가 계산한 높낮이 정보가 담긴 텍스처 (t13 레지스터에 바인딩됨)
Texture2D gDisplacementMap : register(t13);
Texture2D gShadowMap : register(t3);
Texture2D gTextures[7] : register(t6);


// 그림자 계수를 계산하는 함수
float CalcShadowFactor(float4 shadowPosH)
{
      // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;
    float sampleRadius = 1.5; // 1.0 ~ 2.5

    const float2 poissonDisk[16] =
    {
        float2(-0.94201624, -0.39906216),
        float2(0.94558609, -0.76890725),
        float2(-0.094184101, -0.92938870),
        float2(0.34495938, 0.29387760),
        float2(-0.91588581, 0.45771432),
        float2(-0.81544232, -0.87912464),
        float2(-0.38277543, 0.27676845),
        float2(0.97484398, 0.75648379),
        float2(0.44323325, -0.97511554),
        float2(0.53742981, -0.47373420),
        float2(-0.26496911, -0.41893023),
        float2(0.79197514, 0.19090188),
        float2(-0.24188840, 0.99706507),
        float2(-0.81409955, 0.91437590),
        float2(0.19984126, 0.78641367),
        float2(0.14383161, -0.14100790)
    };
    
    
    float percentLit = 0.0f;
    
    [unroll]
    for (int i = 0; i < 16; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
            shadowPosH.xy + poissonDisk[i] * dx * sampleRadius, depth).r;
    }

    return percentLit / 16.0f;
}

VS_OUT VS(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    // (1) 변위 맵에서 현재 정점의 높낮이와 법선 정보를 읽어옵니다.
    // SampleLevel의 세 번째 인자 0.0f는 가장 상세한 Mipmap 레벨을 사용하라는 의미입니다.
    float4 displacement = gDisplacementMap.SampleLevel(gssWrap, In.TexC, 0.0f);

    // (2) 읽어온 정보로 평평한 그리드 정점의 위치(Y)와 법선을 실시간으로 수정합니다.
    float3 posL = In.PosL;
    posL.y = displacement.y; // Y좌표(높이)를 텍스처에서 읽어온 y값으로 덮어씁니다.

    // 법선 벡터도 텍스처에서 읽어온 x, z 편미분 값으로 재구성합니다.
    // (컴퓨트 셰이더가 법선을 직접 계산했다면 그 값을 사용해도 됩니다.)
    float3 normalL = normalize(float3(-displacement.x, 1.0f, -displacement.z));
    
    // (3) 수정된 로컬 좌표를 월드 공간으로 변환합니다.
    Out.PosW = mul(float4(posL, 1.0f), gmtxGameObject).xyz;
    Out.NormalW = mul(normalL, (float3x3) gmtxGameObject);
    Out.TexC = In.TexC;
    
    // (4) 최종적으로 클립 공간 좌표로 변환하여 화면에 표시될 위치를 계산합니다.
    Out.PosH = mul(float4(Out.PosW, 1.0f), gmtxView);
    Out.PosH = mul(Out.PosH, gmtxProjection);

    // (5) 그림자 계산을 위해 빛의 시점에서 본 좌표를 계산합니다. (StandardShader와 동일)
    Out.ShadowPosH = mul(float4(Out.PosW, 1.0f), gmtxShadowTransform);
    
    return Out;
}


float4 PS(VS_OUT In) : SV_Target
{
    In.NormalW = normalize(In.NormalW);
    
    float3 toEyeW = normalize(gvCameraPosition - In.PosW);
    
    float shadowFactor = CalcShadowFactor(In.ShadowPosH);
    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    [unroll]
    for (int i = 0; i < gnLights; ++i)
    {
        if (gLights[i].m_bEnable)
        {
            float3 lightVec;
            if (gLights[i].m_nType != 3) 
                lightVec = normalize(gLights[i].m_vPosition - In.PosW);
            else 
                lightVec = -gLights[i].m_vDirection;
            
            float ndotl = max(0.0f, dot(lightVec, In.NormalW));
            diffuse += ndotl * gLights[i].m_cDiffuse;
            
            float3 reflectVec = reflect(-lightVec, In.NormalW);
            float specFactor = pow(max(0.0f, dot(reflectVec, toEyeW)), 32.0f); // 반사광 세기(shininess)
            spec += specFactor * gLights[i].m_cSpecular;
        }
    }
    
    float4 waterColor = float4(0.1f, 0.3f, 0.6f, 0.7f);
    
    float3 totalLight = gcGlobalAmbientLight.rgb + ambient.rgb + shadowFactor * (diffuse.rgb + spec.rgb);
    float3 finalColor = waterColor.rgb * totalLight;
    
    return float4(finalColor, waterColor.a);
}




Texture2D<float3> gPrevSolInput : register(t0);
Texture2D<float3> gCurrSolInput : register(t1);
RWTexture2D<float3> gNextSolOutput : register(u0); 

cbuffer cbWaveSimConstants : register(b0)
{
    float gWaveConstant0;
    float gWaveConstant1;
    float gWaveConstant2;
    float gSimTimeStep;
    
    float2 gDisplacement;
    int gDisturbIndex;
    uint gGridWidth;
    uint gGridHeight;
};

// 스레드 그룹의 크기 정의
[numthreads(16, 16, 1)]
void WaveSimCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // 현재 스레드가 계산할 그리드의 좌표
    uint x = dispatchThreadID.x;
    uint y = dispatchThreadID.y;

    // 경계(가장자리)는 계산하지 않고 0으로 유지합니다.
    if (x > 0 && x < gGridWidth - 1 && y > 0 && y < gGridHeight - 1)
    {
        // 이전 프레임과 현재 프레임의 주변 픽셀 높이 값을 가져옵니다.
        float prevY = gPrevSolInput[dispatchThreadID.xy].y;
        
        float currY_main = gCurrSolInput[dispatchThreadID.xy].y;
        float currY_left = gCurrSolInput[int2(x - 1, y)].y;
        float currY_right = gCurrSolInput[int2(x + 1, y)].y;
        float currY_top = gCurrSolInput[int2(x, y - 1)].y;
        float currY_bottom = gCurrSolInput[int2(x, y + 1)].y;
        
        float nextY = gWaveConstant0 * prevY +
                      gWaveConstant1 * currY_main +
                      gWaveConstant2 * (currY_left + currY_right + currY_top + currY_bottom);

        // 계산된 다음 프레임의 높이(Y) 값을 출력 텍스처 기록
        gNextSolOutput[dispatchThreadID.xy] = float3(gCurrSolInput[dispatchThreadID.xy].xz, nextY);
    }
    else // 경계인 경우
    {
        // 높이를 0으로 고정
        gNextSolOutput[dispatchThreadID.xy] = float3(gCurrSolInput[dispatchThreadID.xy].xz, 0.0f);
    }
}