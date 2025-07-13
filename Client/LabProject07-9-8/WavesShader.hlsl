// Shaders.hlsl ���� �� �Ʒ��� �߰�

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
    
    // ���� ��ǥ���� ���� ��ġ�� ������ ���� �������� ��ȯ
    Out.PosW = mul(float4(In.PosL, 1.0f), gmtxGameObject).xyz;
    Out.NormalW = mul(In.NormalL, (float3x3) gmtxGameObject);
    
    // �ؽ�ó ��ǥ�� �״�� ����
    Out.TexC = In.TexC;
    
    // ���������� ���� ��ǥ�� ��� �������� ����� ���� Ŭ�� ���� ��ǥ�� ��ȯ
    Out.PosH = mul(float4(Out.PosW, 1.0f), gmtxView);
    Out.PosH = mul(Out.PosH, gmtxProjection);
    
    return Out;
}

// Waves Pixel Shader
float4 PS(VS_WAVES_OUT In) : SV_Target
{
    // ���� ���͸� ����ȭ
    In.NormalW = normalize(In.NormalW);
    
    // �ȼ����� ī�޶�(��)�� ���ϴ� ���� ���
    float3 toEyeW = normalize(gvCameraPosition - In.PosW);

    // ���� ����� ���� ���� �ʱ�ȭ
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // ��鿡 �ִ� ��� ���� ���� ���� ��� (StandardShader�� ����)
    // ���⼭�� �����ϰ� �� ���⼺ ����(2�� �ε���)�� ���
    float3 lightVec = -gLights[2].m_vDirection;
    float ndotl = max(0.0f, dot(lightVec, In.NormalW));
    
    diffuse += ndotl * gLights[2].m_cDiffuse;
    
    float3 reflectVec = reflect(-lightVec, In.NormalW);
    float specFactor = pow(max(0.0f, dot(reflectVec, toEyeW)), 32.0f);
    spec += specFactor * gLights[2].m_cSpecular;

    // ���� �⺻ ����� ���� ����
    float4 waterColor = float4(0.1f, 0.3f, 0.6f, 0.7f); // Ǫ����, 70% ������
    
    // ���� ���� = (�� ���� * (���� �ֺ��� + ���ݻ籤)) + ���ݻ籤
    float4 litColor = waterColor * (gcGlobalAmbientLight + diffuse) + spec;
    
    // ���� ���� ���� �⺻ ���� ���� ���
    litColor.a = waterColor.a;

    return litColor;
}