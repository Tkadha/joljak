struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	float3					gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
	uint					gnTexturesMask : packoffset(c8);
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

SamplerState gssWrap : register(s0);

struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float3 normalW;
	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}
	float4 cIllumination = Lighting(input.positionW, normalW);
	return(lerp(cColor, cIllumination, 0.5f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	//output.positionW = float3(0.0f, 0.0f, 0.0f);
	//output.normalW = float3(0.0f, 0.0f, 0.0f);
	//output.tangentW = float3(0.0f, 0.0f, 0.0f);
	//output.bitangentW = float3(0.0f, 0.0f, 0.0f);
	//matrix mtxVertexToBoneWorld;
	//for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	//{
	//	mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	//	output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	//	output.normalW += input.weights[i] * mul(input.normal, (float3x3)mtxVertexToBoneWorld);
	//	output.tangentW += input.weights[i] * mul(input.tangent, (float3x3)mtxVertexToBoneWorld);
	//	output.bitangentW += input.weights[i] * mul(input.bitangent, (float3x3)mtxVertexToBoneWorld);
	//}
	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
//		mtxVertexToBoneWorld += input.weights[i] * gpmtxBoneTransforms[input.indices[i]];
		mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
	}
	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	output.normalW = mul(input.normal, (float3x3)mtxVertexToBoneWorld).xyz;
	output.tangentW = mul(input.tangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)mtxVertexToBoneWorld).xyz;

//	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainBaseTexture : register(t1);
Texture2D gtxtTerrainDetailTexture : register(t2);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
//	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
	float4 cColor = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
    VS_SKYBOX_CUBEMAP_OUTPUT output;

    // 뷰 행렬에서 이동 성분 제거 (4행 1,2,3열을 0으로)
    float4x4 matViewNoTranslation = gmtxView;
    matViewNoTranslation._41_42_43 = float3(0.0f, 0.0f, 0.0f); // 또는 _m30 = _m31 = _m32 = 0.0f;

    // 스카이박스는 보통 월드 변환이 필요 없으므로 gmtxGameObject는 곱하지 않거나 항등 행렬 사용
    // float4 worldPos = float4(input.position, 1.0f); // 월드 변환 필요 시
    float4 viewPos = mul(float4(input.position, 1.0f), matViewNoTranslation);
    output.position = mul(viewPos, gmtxProjection);

    // Z 값을 W와 같게 만들어 깊이 버퍼의 최대 깊이(1.0)에 위치하도록 함
    // 이렇게 하면 깊이 테스트 시 가장 뒤에 그려지게 할 수 있음
    output.position.z = output.position.w;

    output.positionL = input.position; // 샘플링 좌표는 로컬 위치 그대로 사용

    return (output);
}

TextureCube gtxtSkyCubeTexture : register(t13);
SamplerState gssClamp : register(s1);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	return(cColor);
}	


// 인스턴싱
struct VS_INSTANCING_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4x4 instanceTransform : INSTANCE_TRANSFORM; // 인스턴스별 변환 행렬
};

struct VS_INSTANCING_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};

VS_INSTANCING_OUTPUT VSInstancing(VS_INSTANCING_INPUT input)
{
    VS_INSTANCING_OUTPUT output;

    // 인스턴스별 변환 행렬을 적용
    float4x4 worldMatrix = input.instanceTransform;
    output.positionW = mul(float4(input.position, 1.0f), worldMatrix).xyz;
    output.normalW = mul(input.normal, (float3x3) worldMatrix);
    output.tangentW = mul(input.tangent, (float3x3) worldMatrix);
    output.bitangentW = mul(input.bitangent, (float3x3) worldMatrix);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return output;
}

float4 PSInstancing(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
    float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
        cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
        cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_METALLIC_MAP)
        cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
    float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
        cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

    float3 normalW;
    float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
        float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
        normalW = normalize(mul(vNormal, TBN));
    }
    else
    {
        normalW = normalize(input.normalW);
    }
    float4 cIllumination = Lighting(input.positionW, normalW);
    return (lerp(cColor, cIllumination, 0.5f));
}



// ObbShader.hlsl

// 상수 버퍼 (월드, 뷰, 투영 행렬)
cbuffer cbTransform : register(b0)
{
    float4x4 gWorld;
    float4x4 gView;
    float4x4 gProjection;
};

// 버텍스 셰이더 입력
struct VS_OBBINPUT
{
    float3 position : POSITION;
};

// 버텍스 셰이더 출력
struct VS_OBBOUTPUT
{
    float4 position : SV_POSITION;
};

// 버텍스 셰이더
VS_OBBOUTPUT VSOBB(VS_OBBINPUT input)
{
    VS_OBBOUTPUT output;
    float4 pos = float4(input.position, 1.0f);
    pos = mul(pos, gWorld);
    pos = mul(pos, gView);
    pos = mul(pos, gProjection);
    output.position = pos;
    return output;
}

// 픽셀 셰이더
float4 PSOBB(VS_OBBOUTPUT input) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f); // 빨간색 선
}








// Wave
// Shaders.hlsl 파일 맨 아래에 추가

struct VS_IN
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD0;
};

struct VS_WAVES_OUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD0;
};

// Waves Vertex Shader
VS_WAVES_OUT VSWaves(VS_IN In)
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
float4 PSWaves(VS_WAVES_OUT In) : SV_Target
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