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


float4 DirectionalLight(int nIndex, MaterialInfo material, float3 vNormal, float3 vToCamera) // material 파라미터 추가
{
    float3 vToLight = -gLights[nIndex].m_vDirection;
    float fDiffuseFactor = saturate(dot(vToLight, vNormal)); // saturate 추가 (0 이상)
    float fSpecularFactor = 0.0f;
    if (fDiffuseFactor > 0.0f) // HLSL에서는 bool 대신 float 비교가 일반적
    {
        // material.SpecularColor.a (Power) 사용
        if (material.SpecularColor.a != 0.0f)
        {
            // Glossiness값 사용
            float3 vHalf = normalize(vToCamera + vToLight);
            float fPower = material.Glossiness * 100.0f + 1.0f;  
            fSpecularFactor = (fPower > 1.0f) ? pow(saturate(dot(vHalf, vNormal)), fPower) : 0.0f;
            //fSpecularFactor = pow(saturate(dot(vHalf, vNormal)), material.SpecularColor.a); // saturate 추가
        }
    }

    // 파라미터로 받은 material 사용
    return ((gLights[nIndex].m_cAmbient * material.AmbientColor)
         + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * material.DiffuseColor) // DiffuseColor 사용
         + (gLights[nIndex].m_cSpecular * fSpecularFactor * material.SpecularColor));
}

float4 PointLight(int nIndex, MaterialInfo material, float3 vPosition, float3 vNormal, float3 vToCamera) // material 파라미터 추가
{
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    // 거리가 범위 내일 때만 계산
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
        // 거리가 0에 가까울 때 매우 커지는 것을 방지 (선택적)
        fAttenuationFactor = saturate(fAttenuationFactor);

        // 파라미터로 받은 material 사용
        return (((gLights[nIndex].m_cAmbient * material.AmbientColor)
              + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * material.DiffuseColor)
              + (gLights[nIndex].m_cSpecular * fSpecularFactor * material.SpecularColor)) * fAttenuationFactor);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 SpotLight(int nIndex, MaterialInfo material, float3 vPosition, float3 vNormal, float3 vToCamera) // material 파라미터 추가
{
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    // 거리 및 스팟라이트 각도 내일 때만 계산
    float fDot = dot(-normalize(vToLight), normalize(gLights[nIndex].m_vDirection)); // 조명 방향과 물체->조명 벡터 내적
    float fMaxDot = gLights[nIndex].m_fPhi; // 외부 콘 각도 (cos(Phi))

    if (fDistance <= gLights[nIndex].m_fRange && fDot >= fMaxDot) // 범위 및 외부 콘 내부에 있는지 확인
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

        // 스팟라이트 감쇠 (Theta: 내부 콘, Phi: 외부 콘)
        float fSpotFactor = 1.0f;
        float fMinDot = gLights[nIndex].m_fTheta; // 내부 콘 각도 (cos(Theta))
        if (fDot < fMinDot) // 내부 콘과 외부 콘 사이라면 감쇠 적용
        {
            fSpotFactor = smoothstep(fMaxDot, fMinDot, fDot); // 또는 다른 감쇠 함수 사용
            // fSpotFactor = pow(saturate((fDot - fMaxDot) / (fMinDot - fMaxDot)), gLights[nIndex].m_fFalloff); // 원래 코드 방식
        }

        // 거리 감쇠
        float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        fAttenuationFactor = saturate(fAttenuationFactor);

        // 파라미터로 받은 material 사용
        return (((gLights[nIndex].m_cAmbient * material.AmbientColor)
              + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * material.DiffuseColor)
              + (gLights[nIndex].m_cSpecular * fSpecularFactor * material.SpecularColor)) * fAttenuationFactor * fSpotFactor);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}


// --- 최종 Lighting 함수 수정 ---
// 이제 MaterialInfo를 파라미터로 받음
float4 Lighting(MaterialInfo material, float3 vPosition, float3 vNormal)
{
    // 카메라 위치는 여전히 전역 cbCameraInfo 사용
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vToCamera = normalize(vCameraPosition - vPosition);

    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // 각 활성 조명에 대해 계산 (material 전달)
    [unroll] // 루프 풀기 시도 (성능 향상 가능성)
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

    // 전역 앰비언트 추가 (material.AmbientColor 사용)
    cColor += (gcGlobalAmbientLight * material.AmbientColor);

    // 알파값은 재질 Diffuse의 알파 사용 (예시)
    cColor.a = material.DiffuseColor.a;

    return cColor;
    //return float4(0.8f, 0.8f, 0.8f, 1.0f); // 또는 임의의 밝은 회색 반환
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
          
            lightVec /= distance; // 정규화

        // 확산광(Diffuse) 계산
            float diffuseFactor = saturate(dot(normalW, lightVec));
            cIllumination += light.m_cDiffuse * material.DiffuseColor * diffuseFactor;

        // 반사광(Specular) 계산
            if (diffuseFactor > 0.0f)
            {
                float3 toEye = normalize(gvCameraPosition.xyz - posW);
                float3 halfVec = normalize(lightVec + toEye);
                float specularFactor = pow(saturate(dot(normalW, halfVec)), material.SpecularColor.a * 255.0f);
                cIllumination += light.m_cSpecular * material.SpecularColor * specularFactor;
            }

        // 거리 감쇠 적용
            float attenuation = 1.0f / dot(light.m_vAttenuation, float3(1.0f, distance, distance * distance));
            cIllumination *= attenuation;
        }
    }

    return cIllumination;
}
