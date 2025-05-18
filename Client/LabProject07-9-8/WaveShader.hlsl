#include "Common.hlsl" // 제공해주신 공통 셰이더 파일 (Shaders_Common.hlsli)

// --- 파도 시뮬레이션 결과 텍스처 ---
// 컴퓨트 셰이더가 생성한 파도의 변위 또는 최종 위치 데이터
// float3는 (x_offset, y_offset, z_offset) 또는 (world_x, world_y, world_z)가 될 수 있습니다.
// 여기서는 (base_x, simulated_y, base_z) 형태로 최종 월드 Y좌표를 바로 저장한다고 가정하거나,
// 또는 (offset_x, offset_y, offset_z)를 저장하고 VS에서 기본 위치에 더해준다고 가정합니다.
// d3d12book 예제에서는 보통 각 정점의 최종 Y값을 저장합니다.
Texture2D<float3> gWaveSolution : register(t0); // SRV 슬롯 (예: t0)

cbuffer cbGameObjectInfo : register(b2)
{
    matrix gmtxGameObject; // 파도 객체의 월드 변환
    MaterialInfo gMaterialInfo; // 파도의 재질 정보 (물 재질)
    uint gnTexturesMask; // 파도용 텍스처 사용 마스크 (필요시)
};

// 파도 표면에 알베도/노멀맵 등을 사용하려면 여기에 선언하고 t6~ 슬롯 사용
Texture2D gtxtAlbedoTexture : register(t6); // 예: 물결무늬 알베도
Texture2D gtxtNormalTexture : register(t7); // 예: 물결무늬 노멀맵
// Texture2D gtxtSpecularTexture : register(t7); // 등등 필요에 따라

// --- VS 입출력 구조체 ---
struct VS_WAVE_INPUT
{
    float3 PosL : POSITION; // 로컬 XY 평면 그리드 정점 위치 (Y는 0 또는 무시)
    float2 TexC : TEXCOORD; // 파도 솔루션 텍스처 샘플링 및 표면 텍스처링용 UV
    float3 NormalL : NORMAL; // 기본 법선 (예: float3(0,1,0)) - 동적 계산으로 대체됨
    float3 TangentL : TANGENT; // 기본 탄젠트 (예: float3(1,0,0)) - 동적 계산 시 필요
};

struct PS_WAVE_INPUT // VS의 출력이자 PS의 입력
{
    float4 PosH : SV_POSITION; // 클립 공간 위치
    float3 PosW : POSITION; // 월드 공간 위치
    float3 NormalW : NORMAL; // 월드 공간 법선 (동적으로 계산됨)
    float3 TangentW : TANGENT; // 월드 공간 탄젠트 (동적으로 계산됨)
    float2 TexC : TEXCOORD; // UV 좌표
    // 필요시 float3 BitangentW : BITANGENT; 추가
};

// --- Vertex Shader ---
PS_WAVE_INPUT VSWaveRender(VS_WAVE_INPUT input)
{
    PS_WAVE_INPUT output = (PS_WAVE_INPUT) 0;

    uint width, height;
    gWaveSolution.GetDimensions(width, height);

    // TexC를 사용하여 정수 인덱스로 변환하여 Load (정확한 픽셀 값)
    // 또는 Sample을 사용할 수도 있지만, 그리드 데이터에는 Load가 더 적합할 수 있습니다.
    // Load는 정수 좌표를 사용하므로, TexC를 스케일링하고 가장 가까운 정수로 변환해야 할 수 있습니다.
    // 여기서는 TexC가 이미 그리드 정점에 정확히 매핑된다고 가정하고, 간단히 스케일링합니다.
    int s_x = (int) (input.TexC.x * (width - 1)); // width-1 이 더 정확할 수 있음
    int s_y = (int) (input.TexC.y * (height - 1)); // height-1

    float3 simulated_pos_data = gWaveSolution.Load(int3(s_x, s_y, 0)); // (local_x, world_y, local_z) 형태라고 가정

    // 최종 로컬 위치 (시뮬레이션된 Y 좌표 적용)
    // 입력 PosL.xz는 그리드 위치, simulated_pos_data.y는 시뮬레이션된 높이
    float3 finalPosL = float3(input.PosL.x, simulated_pos_data.y, input.PosL.z);
    // 만약 simulated_pos_data가 (offset_x, offset_y, offset_z) 라면:
    // float3 finalPosL = input.PosL + simulated_pos_data;


    // 2. 월드 공간으로 변환
    output.PosW = mul(float4(finalPosL, 1.0f), gmtxGameObject).xyz;

    // 3. 동적 법선 벡터 계산 (월드 공간에서)
    // 주변 픽셀의 월드 높이를 가져와서 계산합니다.
    // gWaveSolution에 저장된 값이 월드 Y 좌표라고 가정.
    // 공간상의 간격 (dx, dz) - Waves 클래스에서 spatialStep과 같은 값
    // 이 값은 상수버퍼나 다른 방법으로 전달 받아야 함. 여기서는 임의의 값 사용.
    float spatial_step_x = length(mul(float4(1.0f / (width - 1), 0, 0, 0), gmtxGameObject).xyz); // 대략적인 월드 X 간격
    float spatial_step_z = length(mul(float4(0, 0, 1.0f / (height - 1), 0), gmtxGameObject).xyz); // 대략적인 월드 Z 간격
    // 더 정확하게는 C++에서 계산된 spatialStep을 상수버퍼로 전달

    float hL = gWaveSolution.Load(int3(max(0, s_x - 1), s_y, 0)).y;
    float hR = gWaveSolution.Load(int3(min(width - 1, s_x + 1), s_y, 0)).y;
    float hT = gWaveSolution.Load(int3(s_x, max(0, s_y - 1), 0)).y; // HLSL에서 텍스처 위쪽은 V가 작음
    float hB = gWaveSolution.Load(int3(s_x, min(height - 1, s_y + 1), 0)).y;

    // 월드 변환된 높이라고 가정하고 월드 공간에서 법선 계산
    // (위치 L, R, T, B도 월드 변환해야 하지만, 여기서는 높이 차이만 사용)
    float3 N;
    N.x = hL - hR; // X축 변화량에 따른 높이 변화 (2*spatial_step_x 간격)
    N.z = hT - hB; // Z축 변화량에 따른 높이 변화 (2*spatial_step_z 간격)
    N.y = 2.0 * spatial_step_x; // 또는 2.0 * spatial_step_z (평균적인 값 사용)
    // N.y 값은 주변 높이 변화에 비해 상대적으로 커야 수직에 가까운 법선이 나옴
    output.NormalW = normalize(N);
    
    // 만약 simulated_pos_data가 로컬 변위(offset)이라면, 법선 계산 후 월드 변환
    // float3 normalL = normalize(N_local);
    // output.NormalW = normalize(mul(normalL, (float3x3)gmtxGameObject));


    // 4. 월드 공간 탄젠트 계산 (법선맵 등을 사용하지 않는다면 생략 가능, 또는 기본값)
    // 간단한 탄젠트 (X축 방향)
    output.TangentW = normalize(cross(output.NormalW, float3(0.0, 0.0, 1.0))); // NormalW와 Z축 외적
    if (length(output.TangentW) < 0.001) // NormalW가 Z축과 평행할 경우 대비
    {
        output.TangentW = normalize(cross(output.NormalW, float3(1.0, 0.0, 0.0)));
    }
    output.TangentW = normalize(mul(input.TangentL, (float3x3) gmtxGameObject)); // 더 간단하게는 로컬 탄젠트 변환


    // 5. 클립 공간으로 변환
    output.PosH = mul(float4(output.PosW, 1.0f), gmtxView);
    output.PosH = mul(output.PosH, gmtxProjection);

    // 6. UV 좌표 전달
    output.TexC = input.TexC; // 필요시 gmtxGameObject의 TexTransform 적용

    return output;
}

// --- Pixel Shader ---
float4 PSWaveRender(PS_WAVE_INPUT input) : SV_TARGET
{
    // 1. 알베도 색상 결정 (텍스처 또는 재질 기본색)
    float4 cAlbedoColor = gMaterialInfo.DiffuseColor; // 파도 재질의 기본 Diffuse/Albedo 색상
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP) // 파도 표면 텍스처 사용 시
    {
        cAlbedoColor *= gtxtAlbedoTexture.Sample(gssWrap, input.TexC); // 기본 색상에 텍스처 곱하기
    }
    cAlbedoColor.a = saturate(cAlbedoColor.a); // 알파값은 0~1 사이로


    // 2. 최종 법선 벡터 (VS에서 계산된 월드 법선 사용)
    float3 normalW = normalize(input.NormalW);

    // (선택적) 노멀맵을 사용한다면 여기서 추가 처리
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        float3 tangentW = normalize(input.TangentW);
        float3 bitangentW = normalize(cross(normalW, tangentW)); // 재계산 또는 VS에서 전달
        float3x3 TBN = float3x3(tangentW, bitangentW, normalW);

        float4 cNormalSample = gtxtNormalTexture.Sample(gssWrap, input.TexC);
        float3 vNormalMap = normalize(cNormalSample.rgb * 2.0f - 1.0f);
        normalW = normalize(mul(vNormalMap, TBN));
    }

    // 3. 조명 계산 (Common.hlsl의 Lighting 함수 사용)
    // cbGameObjectInfo (b2) 에서 gMaterialInfo를 가져와 전달
    float4 cIlluminationColor = Lighting(gMaterialInfo, input.PosW, normalW);


    // 4. 최종 색상 조합
    // Lighting() 결과는 이미 재질의 Ambient, Diffuse, Specular가 반영된 조명 색상입니다.
    // 여기에 Albedo 텍스처 색상을 곱해주거나, Emissive 등을 추가할 수 있습니다.

    // 현재 Lighting() 함수는 재질 색상을 이미 내부에서 곱하므로,
    // cIlluminationColor를 기본으로 하고, 여기에 Albedo 텍스처의 영향을 추가로 반영할 수 있습니다.
    // (또는 Lighting 함수가 순수 조명 값만 반환하도록 수정하고 여기서 재질색*조명색*텍스처색을 곱하는 방식도 가능)
    
    // 예시: 조명 결과에 텍스처 알베도 색상의 RGB를 곱함 (알파는 조명 결과의 알파 사용)
    float4 finalColor = cIlluminationColor * float4(cAlbedoColor.rgb, 1.0f);
    finalColor.a = cAlbedoColor.a; // 최종 알파는 알베도에서


    // Emissive 추가 (있다면)
    float4 cEmissiveColor = gMaterialInfo.EmissiveColor;
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
    {
        // cEmissiveColor *= gtxtEmissionTexture.Sample(gssWrap, input.TexC); // 에미시브 텍스처가 있다면
    }
    finalColor.rgb += cEmissiveColor.rgb;


    // 5. 안개 효과 적용 (Common.hlsl의 gFogColor 등 사용)
    float distToEye = distance(input.PosW, gvCameraPosition.xyz);
    float fogLerp = saturate((distToEye - gFogStart) / gFogRange); // 안개 농도 (0: 안개 없음, 1: 완전 안개)
    finalColor.rgb = lerp(finalColor.rgb, gFogColor.rgb, fogLerp);
    
    // 최종 알파값은 원하는 대로 (예: 재질 알파, 또는 반투명 효과 시 조절)
    // finalColor.a = gMaterialInfo.DiffuseColor.a; // 또는 계산된 값

    return finalColor;
}