#include "Common.hlsl" // �������ֽ� ���� ���̴� ���� (Shaders_Common.hlsli)

// --- �ĵ� �ùķ��̼� ��� �ؽ�ó ---
// ��ǻƮ ���̴��� ������ �ĵ��� ���� �Ǵ� ���� ��ġ ������
// float3�� (x_offset, y_offset, z_offset) �Ǵ� (world_x, world_y, world_z)�� �� �� �ֽ��ϴ�.
// ���⼭�� (base_x, simulated_y, base_z) ���·� ���� ���� Y��ǥ�� �ٷ� �����Ѵٰ� �����ϰų�,
// �Ǵ� (offset_x, offset_y, offset_z)�� �����ϰ� VS���� �⺻ ��ġ�� �����شٰ� �����մϴ�.
// d3d12book ���������� ���� �� ������ ���� Y���� �����մϴ�.
Texture2D<float3> gWaveSolution : register(t0); // SRV ���� (��: t0)

cbuffer cbGameObjectInfo : register(b2)
{
    matrix gmtxGameObject; // �ĵ� ��ü�� ���� ��ȯ
    MaterialInfo gMaterialInfo; // �ĵ��� ���� ���� (�� ����)
    uint gnTexturesMask; // �ĵ��� �ؽ�ó ��� ����ũ (�ʿ��)
};

// �ĵ� ǥ�鿡 �˺���/��ָ� ���� ����Ϸ��� ���⿡ �����ϰ� t6~ ���� ���
Texture2D gtxtAlbedoTexture : register(t6); // ��: ���ṫ�� �˺���
Texture2D gtxtNormalTexture : register(t7); // ��: ���ṫ�� ��ָ�
// Texture2D gtxtSpecularTexture : register(t7); // ��� �ʿ信 ����

// --- VS ����� ����ü ---
struct VS_WAVE_INPUT
{
    float3 PosL : POSITION; // ���� XY ��� �׸��� ���� ��ġ (Y�� 0 �Ǵ� ����)
    float2 TexC : TEXCOORD; // �ĵ� �ַ�� �ؽ�ó ���ø� �� ǥ�� �ؽ�ó���� UV
    float3 NormalL : NORMAL; // �⺻ ���� (��: float3(0,1,0)) - ���� ������� ��ü��
    float3 TangentL : TANGENT; // �⺻ ź��Ʈ (��: float3(1,0,0)) - ���� ��� �� �ʿ�
};

struct PS_WAVE_INPUT // VS�� ������� PS�� �Է�
{
    float4 PosH : SV_POSITION; // Ŭ�� ���� ��ġ
    float3 PosW : POSITION; // ���� ���� ��ġ
    float3 NormalW : NORMAL; // ���� ���� ���� (�������� ����)
    float3 TangentW : TANGENT; // ���� ���� ź��Ʈ (�������� ����)
    float2 TexC : TEXCOORD; // UV ��ǥ
    // �ʿ�� float3 BitangentW : BITANGENT; �߰�
};

// --- Vertex Shader ---
PS_WAVE_INPUT VSWaveRender(VS_WAVE_INPUT input)
{
    PS_WAVE_INPUT output = (PS_WAVE_INPUT) 0;

    uint width, height;
    gWaveSolution.GetDimensions(width, height);

    // TexC�� ����Ͽ� ���� �ε����� ��ȯ�Ͽ� Load (��Ȯ�� �ȼ� ��)
    // �Ǵ� Sample�� ����� ���� ������, �׸��� �����Ϳ��� Load�� �� ������ �� �ֽ��ϴ�.
    // Load�� ���� ��ǥ�� ����ϹǷ�, TexC�� �����ϸ��ϰ� ���� ����� ������ ��ȯ�ؾ� �� �� �ֽ��ϴ�.
    // ���⼭�� TexC�� �̹� �׸��� ������ ��Ȯ�� ���εȴٰ� �����ϰ�, ������ �����ϸ��մϴ�.
    int s_x = (int) (input.TexC.x * (width - 1)); // width-1 �� �� ��Ȯ�� �� ����
    int s_y = (int) (input.TexC.y * (height - 1)); // height-1

    float3 simulated_pos_data = gWaveSolution.Load(int3(s_x, s_y, 0)); // (local_x, world_y, local_z) ���¶�� ����

    // ���� ���� ��ġ (�ùķ��̼ǵ� Y ��ǥ ����)
    // �Է� PosL.xz�� �׸��� ��ġ, simulated_pos_data.y�� �ùķ��̼ǵ� ����
    float3 finalPosL = float3(input.PosL.x, simulated_pos_data.y, input.PosL.z);
    // ���� simulated_pos_data�� (offset_x, offset_y, offset_z) ���:
    // float3 finalPosL = input.PosL + simulated_pos_data;


    // 2. ���� �������� ��ȯ
    output.PosW = mul(float4(finalPosL, 1.0f), gmtxGameObject).xyz;

    // 3. ���� ���� ���� ��� (���� ��������)
    // �ֺ� �ȼ��� ���� ���̸� �����ͼ� ����մϴ�.
    // gWaveSolution�� ����� ���� ���� Y ��ǥ��� ����.
    // �������� ���� (dx, dz) - Waves Ŭ�������� spatialStep�� ���� ��
    // �� ���� ������۳� �ٸ� ������� ���� �޾ƾ� ��. ���⼭�� ������ �� ���.
    float spatial_step_x = length(mul(float4(1.0f / (width - 1), 0, 0, 0), gmtxGameObject).xyz); // �뷫���� ���� X ����
    float spatial_step_z = length(mul(float4(0, 0, 1.0f / (height - 1), 0), gmtxGameObject).xyz); // �뷫���� ���� Z ����
    // �� ��Ȯ�ϰԴ� C++���� ���� spatialStep�� ������۷� ����

    float hL = gWaveSolution.Load(int3(max(0, s_x - 1), s_y, 0)).y;
    float hR = gWaveSolution.Load(int3(min(width - 1, s_x + 1), s_y, 0)).y;
    float hT = gWaveSolution.Load(int3(s_x, max(0, s_y - 1), 0)).y; // HLSL���� �ؽ�ó ������ V�� ����
    float hB = gWaveSolution.Load(int3(s_x, min(height - 1, s_y + 1), 0)).y;

    // ���� ��ȯ�� ���̶�� �����ϰ� ���� �������� ���� ���
    // (��ġ L, R, T, B�� ���� ��ȯ�ؾ� ������, ���⼭�� ���� ���̸� ���)
    float3 N;
    N.x = hL - hR; // X�� ��ȭ���� ���� ���� ��ȭ (2*spatial_step_x ����)
    N.z = hT - hB; // Z�� ��ȭ���� ���� ���� ��ȭ (2*spatial_step_z ����)
    N.y = 2.0 * spatial_step_x; // �Ǵ� 2.0 * spatial_step_z (������� �� ���)
    // N.y ���� �ֺ� ���� ��ȭ�� ���� ��������� Ŀ�� ������ ����� ������ ����
    output.NormalW = normalize(N);
    
    // ���� simulated_pos_data�� ���� ����(offset)�̶��, ���� ��� �� ���� ��ȯ
    // float3 normalL = normalize(N_local);
    // output.NormalW = normalize(mul(normalL, (float3x3)gmtxGameObject));


    // 4. ���� ���� ź��Ʈ ��� (������ ���� ������� �ʴ´ٸ� ���� ����, �Ǵ� �⺻��)
    // ������ ź��Ʈ (X�� ����)
    output.TangentW = normalize(cross(output.NormalW, float3(0.0, 0.0, 1.0))); // NormalW�� Z�� ����
    if (length(output.TangentW) < 0.001) // NormalW�� Z��� ������ ��� ���
    {
        output.TangentW = normalize(cross(output.NormalW, float3(1.0, 0.0, 0.0)));
    }
    output.TangentW = normalize(mul(input.TangentL, (float3x3) gmtxGameObject)); // �� �����ϰԴ� ���� ź��Ʈ ��ȯ


    // 5. Ŭ�� �������� ��ȯ
    output.PosH = mul(float4(output.PosW, 1.0f), gmtxView);
    output.PosH = mul(output.PosH, gmtxProjection);

    // 6. UV ��ǥ ����
    output.TexC = input.TexC; // �ʿ�� gmtxGameObject�� TexTransform ����

    return output;
}

// --- Pixel Shader ---
float4 PSWaveRender(PS_WAVE_INPUT input) : SV_TARGET
{
    // 1. �˺��� ���� ���� (�ؽ�ó �Ǵ� ���� �⺻��)
    float4 cAlbedoColor = gMaterialInfo.DiffuseColor; // �ĵ� ������ �⺻ Diffuse/Albedo ����
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP) // �ĵ� ǥ�� �ؽ�ó ��� ��
    {
        cAlbedoColor *= gtxtAlbedoTexture.Sample(gssWrap, input.TexC); // �⺻ ���� �ؽ�ó ���ϱ�
    }
    cAlbedoColor.a = saturate(cAlbedoColor.a); // ���İ��� 0~1 ���̷�


    // 2. ���� ���� ���� (VS���� ���� ���� ���� ���)
    float3 normalW = normalize(input.NormalW);

    // (������) ��ָ��� ����Ѵٸ� ���⼭ �߰� ó��
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    {
        float3 tangentW = normalize(input.TangentW);
        float3 bitangentW = normalize(cross(normalW, tangentW)); // ���� �Ǵ� VS���� ����
        float3x3 TBN = float3x3(tangentW, bitangentW, normalW);

        float4 cNormalSample = gtxtNormalTexture.Sample(gssWrap, input.TexC);
        float3 vNormalMap = normalize(cNormalSample.rgb * 2.0f - 1.0f);
        normalW = normalize(mul(vNormalMap, TBN));
    }

    // 3. ���� ��� (Common.hlsl�� Lighting �Լ� ���)
    // cbGameObjectInfo (b2) ���� gMaterialInfo�� ������ ����
    float4 cIlluminationColor = Lighting(gMaterialInfo, input.PosW, normalW);


    // 4. ���� ���� ����
    // Lighting() ����� �̹� ������ Ambient, Diffuse, Specular�� �ݿ��� ���� �����Դϴ�.
    // ���⿡ Albedo �ؽ�ó ������ �����ְų�, Emissive ���� �߰��� �� �ֽ��ϴ�.

    // ���� Lighting() �Լ��� ���� ������ �̹� ���ο��� ���ϹǷ�,
    // cIlluminationColor�� �⺻���� �ϰ�, ���⿡ Albedo �ؽ�ó�� ������ �߰��� �ݿ��� �� �ֽ��ϴ�.
    // (�Ǵ� Lighting �Լ��� ���� ���� ���� ��ȯ�ϵ��� �����ϰ� ���⼭ ������*�����*�ؽ�ó���� ���ϴ� ��ĵ� ����)
    
    // ����: ���� ����� �ؽ�ó �˺��� ������ RGB�� ���� (���Ĵ� ���� ����� ���� ���)
    float4 finalColor = cIlluminationColor * float4(cAlbedoColor.rgb, 1.0f);
    finalColor.a = cAlbedoColor.a; // ���� ���Ĵ� �˺�������


    // Emissive �߰� (�ִٸ�)
    float4 cEmissiveColor = gMaterialInfo.EmissiveColor;
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
    {
        // cEmissiveColor *= gtxtEmissionTexture.Sample(gssWrap, input.TexC); // ���̽ú� �ؽ�ó�� �ִٸ�
    }
    finalColor.rgb += cEmissiveColor.rgb;


    // 5. �Ȱ� ȿ�� ���� (Common.hlsl�� gFogColor �� ���)
    float distToEye = distance(input.PosW, gvCameraPosition.xyz);
    float fogLerp = saturate((distToEye - gFogStart) / gFogRange); // �Ȱ� �� (0: �Ȱ� ����, 1: ���� �Ȱ�)
    finalColor.rgb = lerp(finalColor.rgb, gFogColor.rgb, fogLerp);
    
    // ���� ���İ��� ���ϴ� ��� (��: ���� ����, �Ǵ� ������ ȿ�� �� ����)
    // finalColor.a = gMaterialInfo.DiffuseColor.a; // �Ǵ� ���� ��

    return finalColor;
}