// Shaders.hlsl ���� �� �Ʒ��� �߰�

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
    float4 PosH : SV_POSITION; // ���� ȭ�� ��ǥ
    float3 PosW : POSITION; // ���� ��ǥ
    float3 NormalW : NORMAL; // ���� ���� ����
    float2 TexC : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1; // �׸��� �� ��ǥ
};
// 1. �ؽ�ó ���ҽ� ����
// ��ǻƮ ���̴��� ����� ������ ������ ��� �ؽ�ó (t13 �������Ϳ� ���ε���)
Texture2D gDisplacementMap : register(t13);
Texture2D gShadowMap : register(t3);
Texture2D gTextures[7] : register(t6);


// �׸��� ����� ����ϴ� �Լ�
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
    
    // (1) ���� �ʿ��� ���� ������ �����̿� ���� ������ �о�ɴϴ�.
    // SampleLevel�� �� ��° ���� 0.0f�� ���� ���� Mipmap ������ ����϶�� �ǹ��Դϴ�.
    float4 displacement = gDisplacementMap.SampleLevel(gssWrap, In.TexC, 0.0f);

    // (2) �о�� ������ ������ �׸��� ������ ��ġ(Y)�� ������ �ǽð����� �����մϴ�.
    float3 posL = In.PosL;
    posL.y = displacement.y; // Y��ǥ(����)�� �ؽ�ó���� �о�� y������ ����ϴ�.

    // ���� ���͵� �ؽ�ó���� �о�� x, z ��̺� ������ �籸���մϴ�.
    // (��ǻƮ ���̴��� ������ ���� ����ߴٸ� �� ���� ����ص� �˴ϴ�.)
    float3 normalL = normalize(float3(-displacement.x, 1.0f, -displacement.z));
    
    // (3) ������ ���� ��ǥ�� ���� �������� ��ȯ�մϴ�.
    Out.PosW = mul(float4(posL, 1.0f), gmtxGameObject).xyz;
    Out.NormalW = mul(normalL, (float3x3) gmtxGameObject);
    Out.TexC = In.TexC;
    
    // (4) ���������� Ŭ�� ���� ��ǥ�� ��ȯ�Ͽ� ȭ�鿡 ǥ�õ� ��ġ�� ����մϴ�.
    Out.PosH = mul(float4(Out.PosW, 1.0f), gmtxView);
    Out.PosH = mul(Out.PosH, gmtxProjection);

    // (5) �׸��� ����� ���� ���� �������� �� ��ǥ�� ����մϴ�. (StandardShader�� ����)
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
            float specFactor = pow(max(0.0f, dot(reflectVec, toEyeW)), 32.0f); // �ݻ籤 ����(shininess)
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

// ������ �׷��� ũ�� ����
[numthreads(16, 16, 1)]
void WaveSimCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // ���� �����尡 ����� �׸����� ��ǥ
    uint x = dispatchThreadID.x;
    uint y = dispatchThreadID.y;

    // ���(�����ڸ�)�� ������� �ʰ� 0���� �����մϴ�.
    if (x > 0 && x < gGridWidth - 1 && y > 0 && y < gGridHeight - 1)
    {
        // ���� �����Ӱ� ���� �������� �ֺ� �ȼ� ���� ���� �����ɴϴ�.
        float prevY = gPrevSolInput[dispatchThreadID.xy].y;
        
        float currY_main = gCurrSolInput[dispatchThreadID.xy].y;
        float currY_left = gCurrSolInput[int2(x - 1, y)].y;
        float currY_right = gCurrSolInput[int2(x + 1, y)].y;
        float currY_top = gCurrSolInput[int2(x, y - 1)].y;
        float currY_bottom = gCurrSolInput[int2(x, y + 1)].y;
        
        float nextY = gWaveConstant0 * prevY +
                      gWaveConstant1 * currY_main +
                      gWaveConstant2 * (currY_left + currY_right + currY_top + currY_bottom);

        // ���� ���� �������� ����(Y) ���� ��� �ؽ�ó ���
        gNextSolOutput[dispatchThreadID.xy] = float3(gCurrSolInput[dispatchThreadID.xy].xz, nextY);
    }
    else // ����� ���
    {
        // ���̸� 0���� ����
        gNextSolOutput[dispatchThreadID.xy] = float3(gCurrSolInput[dispatchThreadID.xy].xz, 0.0f);
    }
}