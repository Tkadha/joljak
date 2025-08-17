//--------------------------------------------------------------------------------------
#define MAX_MATERIALS		16 
#ifndef MAX_LIGHTS
#define MAX_LIGHTS 16
#endif

#ifndef POINT_LIGHT
#define POINT_LIGHT 1
#endif
#ifndef SPOT_LIGHT
#define SPOT_LIGHT 2
#endif
#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT 3
#endif
#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT

struct LIGHT
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular;
	float3					m_vPosition;
	float 					m_fFalloff;
	float3					m_vDirection;
	float 					m_fTheta; //cos(m_fTheta)
	float3					m_vAttenuation;
	float					m_fPhi; //cos(m_fPhi)
	int					    m_bEnable;
	int 					m_nType;
	float					m_fRange;
	float					padding;
};

cbuffer cbLights : register(b4)
{
	LIGHT					gLights[MAX_LIGHTS];
	float4					gcGlobalAmbientLight;
	int						gnLights;
    bool                    gIsDaytime;
};


float4 DirectionalLight(int nIndex, MaterialInfo material, float3 vNormal, float3 vToCamera) // material �Ķ���� �߰�
{
    float3 vToLight = -gLights[nIndex].m_vDirection;
    float fDiffuseFactor = saturate(dot(vToLight, vNormal)); // saturate �߰� (0 �̻�)
    float fSpecularFactor = 0.0f;
    if (fDiffuseFactor > 0.0f) // HLSL������ bool ��� float �񱳰� �Ϲ���
    {
        // material.SpecularColor.a (Power) ���
        if (material.SpecularColor.a != 0.0f)
        {
            // Glossiness�� ���
            float3 vHalf = normalize(vToCamera + vToLight);
            float fPower = material.Glossiness * 100.0f + 1.0f;  
            fSpecularFactor = (fPower > 1.0f) ? pow(saturate(dot(vHalf, vNormal)), fPower) : 0.0f;
            //fSpecularFactor = pow(saturate(dot(vHalf, vNormal)), material.SpecularColor.a); // saturate �߰�
        }
    }

    // �Ķ���ͷ� ���� material ���
    return ((gLights[nIndex].m_cAmbient * material.AmbientColor)
         + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * material.DiffuseColor) // DiffuseColor ���
         + (gLights[nIndex].m_cSpecular * fSpecularFactor * material.SpecularColor));
}

float4 PointLight(int nIndex, MaterialInfo material, float3 vPosition, float3 vNormal, float3 vToCamera) // material �Ķ���� �߰�
{
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    // �Ÿ��� ���� ���� ���� ���
    if (fDistance <= gLights[nIndex].m_fRange)
    {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance; // Normalize
        float fDiffuseFactor = saturate(dot(vToLight, vNormal));
        if (fDiffuseFactor > 0.0f)
        {
            if (material.SpecularColor.a != 0.0f)
            {
                float3 vHalf = normalize(vToCamera + vToLight);
                fSpecularFactor = pow(saturate(dot(vHalf, vNormal)), material.SpecularColor.a);
            }
        }
        float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        // �Ÿ��� 0�� ����� �� �ſ� Ŀ���� ���� ���� (������)
        fAttenuationFactor = saturate(fAttenuationFactor);

        // �Ķ���ͷ� ���� material ���
        return (((gLights[nIndex].m_cAmbient * material.AmbientColor)
              + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * material.DiffuseColor)
              + (gLights[nIndex].m_cSpecular * fSpecularFactor * material.SpecularColor)) * fAttenuationFactor);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 SpotLight(int nIndex, MaterialInfo material, float3 vPosition, float3 vNormal, float3 vToCamera) // material �Ķ���� �߰�
{
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    // �Ÿ� �� ���̶���Ʈ ���� ���� ���� ���
    float fDot = dot(-normalize(vToLight), normalize(gLights[nIndex].m_vDirection)); // ���� ����� ��ü->���� ���� ����
    float fMaxDot = gLights[nIndex].m_fPhi; // �ܺ� �� ���� (cos(Phi))

    if (fDistance <= gLights[nIndex].m_fRange && fDot >= fMaxDot) // ���� �� �ܺ� �� ���ο� �ִ��� Ȯ��
    {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance; // Normalize
        float fDiffuseFactor = saturate(dot(vToLight, vNormal));
        if (fDiffuseFactor > 0.0f)
        {
            if (material.SpecularColor.a != 0.0f)
            {
                float3 vHalf = normalize(vToCamera + vToLight);
                fSpecularFactor = pow(saturate(dot(vHalf, vNormal)), material.SpecularColor.a);
            }
        }

        // ���̶���Ʈ ���� (Theta: ���� ��, Phi: �ܺ� ��)
        float fSpotFactor = 1.0f;
        float fMinDot = gLights[nIndex].m_fTheta; // ���� �� ���� (cos(Theta))
        if (fDot < fMinDot) // ���� �ܰ� �ܺ� �� ���̶�� ���� ����
        {
            fSpotFactor = smoothstep(fMaxDot, fMinDot, fDot); // �Ǵ� �ٸ� ���� �Լ� ���
            // fSpotFactor = pow(saturate((fDot - fMaxDot) / (fMinDot - fMaxDot)), gLights[nIndex].m_fFalloff); // ���� �ڵ� ���
        }

        // �Ÿ� ����
        float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        fAttenuationFactor = saturate(fAttenuationFactor);

        // �Ķ���ͷ� ���� material ���
        return (((gLights[nIndex].m_cAmbient * material.AmbientColor)
              + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * material.DiffuseColor)
              + (gLights[nIndex].m_cSpecular * fSpecularFactor * material.SpecularColor)) * fAttenuationFactor * fSpotFactor);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}


// --- ���� Lighting �Լ� ���� ---
// ���� MaterialInfo�� �Ķ���ͷ� ����
float4 Lighting(MaterialInfo material, float3 vPosition, float3 vNormal)
{
    // ī�޶� ��ġ�� ������ ���� cbCameraInfo ���
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vToCamera = normalize(vCameraPosition - vPosition);

    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // �� Ȱ�� ���� ���� ��� (material ����)
    [unroll] // ���� Ǯ�� �õ� (���� ��� ���ɼ�)
    for (int i = 0; i < gnLights; i++)
    {
        if (gLights[i].m_bEnable)
        {
            if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
            {
                cColor += DirectionalLight(i, material, vNormal, vToCamera);
            }
            else if (gLights[i].m_nType == POINT_LIGHT)
            {
                cColor += PointLight(i, material, vPosition, vNormal, vToCamera);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cColor += SpotLight(i, material, vPosition, vNormal, vToCamera);
            }
        }
    }

    // ���� �ں��Ʈ �߰� (material.AmbientColor ���)
    cColor += (gcGlobalAmbientLight * material.AmbientColor);

    // ���İ��� ���� Diffuse�� ���� ��� (����)
    cColor.a = material.DiffuseColor.a;

    return cColor;
    //return float4(0.8f, 0.8f, 0.8f, 1.0f); // �Ǵ� ������ ���� ȸ�� ��ȯ
}



float4 ComputeLight(MaterialInfo material, LIGHT light, float3 posW, float3 normalW)
{
    float4 cIllumination = float4(0.0f, 0.0f, 0.0f, 0.0f);

    if (light.m_nType == DIRECTIONAL_LIGHT) 
    {
        float3 lightVec = -light.m_vDirection;

        float diffuseFactor = saturate(dot(normalW, lightVec));
        cIllumination += light.m_cDiffuse * material.DiffuseColor * diffuseFactor;

        if (diffuseFactor > 0.0f)
        {
            float3 toEye = normalize(gvCameraPosition.xyz - posW);
            float3 halfVec = normalize(lightVec + toEye);
            float specularFactor = pow(saturate(dot(normalW, halfVec)), material.SpecularColor.a * 255.0f);
            cIllumination += light.m_cSpecular * material.SpecularColor * specularFactor;
        }
    }
    else if (light.m_nType == POINT_LIGHT)
    {
        if (!gIsDaytime)
        {
            float3 lightVec = light.m_vPosition - posW;
            float distance = length(lightVec);
        
            if (distance > light.m_fRange)
                return float4(0.0f, 0.0f, 0.0f, 0.0f);
          
            lightVec /= distance; // ����ȭ

        // Ȯ�걤(Diffuse) ���
            float diffuseFactor = saturate(dot(normalW, lightVec));
            cIllumination += light.m_cDiffuse * material.DiffuseColor * diffuseFactor;

        // �ݻ籤(Specular) ���
            if (diffuseFactor > 0.0f)
            {
                float3 toEye = normalize(gvCameraPosition.xyz - posW);
                float3 halfVec = normalize(lightVec + toEye);
                float specularFactor = pow(saturate(dot(normalW, halfVec)), material.SpecularColor.a * 255.0f);
                cIllumination += light.m_cSpecular * material.SpecularColor * specularFactor;
            }

        // �Ÿ� ���� ����
            float attenuation = 1.0f / dot(light.m_vAttenuation, float3(1.0f, distance, distance * distance));
            cIllumination *= attenuation;
        }
    }

    return cIllumination;
}
