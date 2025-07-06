// ShaderManager.cpp

#include "stdafx.h" // �ʿ�� ������Ʈ�� stdafx ���
#include "ShaderManager.h"
#include "Shader.h"
#include "OBBShader.h"
#include "TerrainShader.h"
#include "StandardShader.h"
#include "SkinnedShader.h"
#include "SkyBoxShader.h"
#include "ShadowShader.h"
#include "DebugShader.h"
#include "DeferedrLightingShader.h"
// ��: #include "StandardShader.h", "SkinnedShader.h" �� (���� ���ٸ� ���� �ʿ�)

// --- ���� ���÷� ���� ---
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 3> GetStaticSamplers()
{
    const CD3DX12_STATIC_SAMPLER_DESC wrapSampler(
        0, // shaderRegister (s0)
        D3D12_FILTER_ANISOTROPIC,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        0.0f, // mipLODBias
        16, // maxAnisotropy 
        D3D12_COMPARISON_FUNC_LESS_EQUAL
    );

    const CD3DX12_STATIC_SAMPLER_DESC clampSampler(
        1, // shaderRegister (s1)
        D3D12_FILTER_ANISOTROPIC,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        0.0f, // mipLODBias
        16, // maxAnisotropy 
        D3D12_COMPARISON_FUNC_LESS_EQUAL
    );

    const CD3DX12_STATIC_SAMPLER_DESC shadow(
        2, // shaderRegister
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,                               // mipLODBias
        16,                                 // maxAnisotropy
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

    return { wrapSampler, clampSampler, shadow };
}

// --- ������, �Ҹ���, ���� �Լ� ---
ShaderManager::ShaderManager(ID3D12Device* pd3dDevice) : m_pd3dDevice(pd3dDevice)
{
    assert(m_pd3dDevice != nullptr && "ShaderManager requires a valid D3D12 Device!");
}

ShaderManager::~ShaderManager()
{
    ReleaseAll();
}

void ShaderManager::ReleaseAll()
{
    // �����ϴ� CShader ��ü���� ���� ī��Ʈ ����
    for (auto const& [key, pShader] : m_Shaders)
    {
        if (pShader)
        {
            pShader->Release(); // ShaderManager�� ���� �� AddRef �����Ƿ� Release
        }
    }
    m_Shaders.clear();

    // ��Ʈ ������ ComPtr�� �ڵ����� Release ó��
    m_RootSignatures.clear();
}

// --- ��Ʈ ���� ���� ���� ---
ID3D12RootSignature* ShaderManager::GetRootSignature(const std::string& name)
{
    // 1. ĳ�� Ȯ��
    auto it = m_RootSignatures.find(name);
    if (it != m_RootSignatures.end())
    {
        return it->second.Get(); // ĳ�õ� ��Ʈ ���� ��ȯ
    }
    else
    {
        // 2. ĳ�ÿ� ������ ����
        OutputDebugStringA("Creating Root Signature: ");
        OutputDebugStringA(name.c_str());
        OutputDebugStringA("\n");
        ID3D12RootSignature* pNewRootSig = CreateRootSignatureInternal(name);

        // 3. ���� ���� �� ĳ�ÿ� ����
        if (pNewRootSig)
        {
            m_RootSignatures[name] = pNewRootSig; // ComPtr�� ������ ����
            return pNewRootSig;
        }
        else
        {
            OutputDebugStringA("Error: Failed to create Root Signature '");
            OutputDebugStringA(name.c_str());
            OutputDebugStringA("'\n");
            return nullptr; // ���� ����
        }
    }
}

// ��Ʈ ���� ����
ID3D12RootSignature* ShaderManager::CreateRootSignatureInternal(const std::string& name)
{
    if (name == "Standard") return CreateStandardRootSignature();
    if (name == "Skinned") return CreateSkinnedRootSignature();
    if (name == "Terrain") return CreateTerrainRootSignature();
    if (name == "Skybox") return CreateSkyboxRootSignature();
    if (name == "OBB") return CreateOBBRootSignature();
    if (name == "Instancing") return CreateInstancingRootSignature();
    if (name == "Shadow") return CreateShadowRootSignature();
    if (name == "Debug") return CreateDebugRootSignature();
    if (name == "Lighting") return CreateDeferedLightingRootSignature();


    OutputDebugStringA("Error: Unknown root signature name requested!\n");
    return nullptr;
}

// --- �� ��Ʈ ���� ���� �Լ� ���� ---
// (���� �亯���� �����ߴ� CreateStandardRootSignature, CreateSkinnedRootSignature ���� �Լ� ������
//  ���⿡ ��� �Լ� ���·� �ֽ��ϴ�. m_pd3dDevice �� ����ϵ��� �մϴ�.)

ID3D12RootSignature* ShaderManager::CreateStandardRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[2];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 6, 0); // t6-t12: Albedo, Spec, Normal, Metal, Emis, DetailAlb, DetailNrm
    descRangeSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0); // t3: �׸��� �� SRV


    CD3DX12_ROOT_PARAMETER rootParameters[5]; // CBV(b1 Camera), Constants(b2 Object), CBV(b4 Lights), Table(t6-t12 Textures)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL); // 1 Matrix + 4 float4 Material + 1 uint Mask = 16 + 16 + 1 = 33 DWORDS
    rootParameters[2].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL); // b4: Lights (�� �������͸� ����Ѵٰ� ����)
    rootParameters[3].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // �׸��� �� ���̺�

    auto staticSamplers = GetStaticSamplers(); // Wrap(s0), Clamp(s1) ������
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        _countof(rootParameters), // �Ķ���� ����: 5
        rootParameters,
        (UINT)staticSamplers.size(), // ���÷� ����: 3
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}

ID3D12RootSignature* ShaderManager::CreateSkinnedRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[2];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 6, 0); // t6-t12
    descRangeSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0); // t3: �׸��� �� SRV

    CD3DX12_ROOT_PARAMETER rootParameters[7]; // Standard + CBV(b7), CBV(b8)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // b1: Camera
    rootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL);     // b2: Object
    rootParameters[2].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL); // b4: Lights
    rootParameters[3].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // t6-t12
    rootParameters[4].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // �׸��� �� ���̺�
    rootParameters[5].InitAsConstantBufferView(7, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b7: Bone Offsets
    rootParameters[6].InitAsConstantBufferView(8, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b8: Bone Transforms

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        _countof(rootParameters), // �Ķ���� ����: 5
        rootParameters,
        (UINT)staticSamplers.size(), // ���÷� ����: 3
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}

ID3D12RootSignature* ShaderManager::CreateTerrainRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[2];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0); // t1, t2
    descRangeSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0); // t3: Shadow Map

    CD3DX12_ROOT_PARAMETER rootParameters[4]; // CBV(b1 Camera), Constants(b2 Object), Table(t1, t2 Textures)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // �� �� ���
    rootParameters[1].InitAsConstants(16, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);     // VS������ �ʿ�
    rootParameters[2].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS������ �ʿ�
    rootParameters[3].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // PS������ �ʿ�

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParameters), rootParameters,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}

ID3D12RootSignature* ShaderManager::CreateSkyboxRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[1];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 13, 0); // t13: SkyCubeTexture

    CD3DX12_ROOT_PARAMETER rootParameters[2]; // CBV(b1 Camera), Table(t13 Texture)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // VS���� View/Proj �ʿ�
    rootParameters[1].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS���� ���ø�

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParameters), rootParameters,
        1, &staticSamplers[1], // Clamp ���÷�(s1)�� ��� ����
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}

ID3D12RootSignature* ShaderManager::CreateOBBRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_ROOT_PARAMETER rootParameters[1]; // CBV(b0 Transform WVP)
    // OBB ���̴� HLSL�� cbTransform:register(b0) ���
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    // ���÷��� �ؽ�ó �ʿ� ����
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParameters), rootParameters,
        0, nullptr, // No static samplers
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}

ID3D12RootSignature* ShaderManager::CreateInstancingRootSignature()
{
    // Instancing ���̴��� ��Ʈ ���� (Standard�� �����ϳ� b2 GameObjectInfo ��� Instance Data ���)
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[1];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 6, 0); // t6-t12

    CD3DX12_ROOT_PARAMETER rootParameters[3]; // CBV(b1 Camera), CBV(b4 Lights), Table(t6-t12 Textures)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[1].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL); // b4: Lights
    rootParameters[2].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParameters), rootParameters,
        1, &staticSamplers[0], // Wrap ���÷�(s0)�� ��� ����
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}

ID3D12RootSignature* ShaderManager::CreateShadowRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[5];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 13, 0); // t13
    descRangeSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 14, 0); // t13
    descRangeSRV[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 15, 0); // t13
    descRangeSRV[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 16, 0); // t13
    descRangeSRV[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0); // t3: Shadow Map

    CD3DX12_ROOT_PARAMETER rootParameters[7]; // CBV(b1 Camera), Constants(b2 Object), Table(t1, t2 Textures)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // �� �� ���
    rootParameters[1].InitAsConstants(16, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);     // VS������ �ʿ�
    rootParameters[2].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS������ �ʿ�
    rootParameters[3].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // PS������ �ʿ�
    rootParameters[4].InitAsDescriptorTable(1, &descRangeSRV[2], D3D12_SHADER_VISIBILITY_PIXEL); // PS������ �ʿ�
    rootParameters[5].InitAsDescriptorTable(1, &descRangeSRV[3], D3D12_SHADER_VISIBILITY_PIXEL); // PS������ �ʿ�
    rootParameters[6].InitAsDescriptorTable(1, &descRangeSRV[4], D3D12_SHADER_VISIBILITY_PIXEL); // PS������ �ʿ�

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParameters), rootParameters,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}

ID3D12RootSignature* ShaderManager::CreateDebugRootSignature()
{

    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;

    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[1];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[1]; 
    rootParameters[0].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS���� ���ø�

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParameters), rootParameters,
        1, &staticSamplers[0],
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}

ID3D12RootSignature* ShaderManager::CreateDeferedLightingRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;

    // --- ���̴��� ���� ���ҽ����� �������� �����մϴ� ---

    // 1. G-Buffer �ؽ�ó�� (4��, t13���� ����)
    CD3DX12_DESCRIPTOR_RANGE descRangeGBuffer;
    descRangeGBuffer.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 13); // 4 SRVs starting at register t13

    // 2. �׸��� �� �ؽ�ó (1��, t3)
    CD3DX12_DESCRIPTOR_RANGE descRangeShadowMap;
    descRangeShadowMap.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3); // 1 SRV at register t3

    // ��Ʈ �Ķ����
    CD3DX12_ROOT_PARAMETER rootParameters[4];

    // �Ķ���� 0: ī�޶� ���� (b0)
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    // �Ķ���� 1: ���� ���� (b4)
    rootParameters[1].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    // �Ķ���� 2: �׸��� �� ���̺� (t3)
    rootParameters[2].InitAsDescriptorTable(1, &descRangeShadowMap, D3D12_SHADER_VISIBILITY_PIXEL);

    // �Ķ���� 3: G-Buffer �ؽ�ó ���̺� (t13, t14, t15, t16)
    rootParameters[3].InitAsDescriptorTable(1, &descRangeGBuffer, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        _countof(rootParameters), // �Ķ���� ����
        rootParameters,
        (UINT)staticSamplers.size(), // ��� ���� ���÷� ���
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | // ���� �н��� VS�� �ܼ��ϹǷ� ���� ���ʿ�
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
    );

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        return nullptr;
    }

    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) {
        return nullptr;
    }

    return pd3dRootSignature;
}


CShader* ShaderManager::GetShader(const std::string& name, ID3D12GraphicsCommandList* pd3dCommandList)
{
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end())
    {
        it->second->AddRef();
        return it->second;
    }
    else
    {
        OutputDebugStringA("Creating Shader: ");
        OutputDebugStringA(name.c_str());
        OutputDebugStringA("\n");
        CShader* pNewShader = CreateShaderInternal(name, pd3dCommandList);
        if (pNewShader)
        {
            pNewShader->AddRef();
            m_Shaders[name] = pNewShader;
            pNewShader->AddRef();
            return pNewShader;
        }
        else
        {
            OutputDebugStringA("Error: Failed to create Shader '");
            OutputDebugStringA(name.c_str());
            OutputDebugStringA("'\n");
            return nullptr;
        }
    }
}

// ���̴� ���� �б� ����
CShader* ShaderManager::CreateShaderInternal(const std::string& name, ID3D12GraphicsCommandList* pd3dCommandList)
{
    CShader* pShader = nullptr;
    ID3D12RootSignature* pRootSig = nullptr;
    std::string rootSignatureName = "";

    // 1. ���̴� �̸��� ���� ����� Ŭ������ ��Ʈ ���� �̸� ����
    if (name == "Standard") {
        pShader = new CStandardShader(name); // Standard ���̴� ��ü ����
        rootSignatureName = "Standard";  // Standard ��Ʈ ���� �ʿ�
    }
    else if (name == "Standard_GBuffer") {
        pShader = new CStandardShader(name);
        rootSignatureName = "Standard"; // ��Ʈ ������ ���� ���� ����
    }
    else if (name == "Skinned") {
        pShader = new CSkinnedAnimationStandardShader(name);
        rootSignatureName = "Skinned";
    }
    else if (name == "Terrain") {
        pShader = new CTerrainShader(name);
        rootSignatureName = "Terrain";
    }
    else if (name == "Skybox") {
        pShader = new CSkyBoxShader(name);
        rootSignatureName = "Skybox";
    }
    else if (name == "OBB") {
        pShader = new COBBShader(name);
        rootSignatureName = "OBB";
    }
    else if (name == "Shadow") {
        pShader = new CShadowShader(name);
        rootSignatureName = "Shadow";
    }
    else if (name == "Debug") {
        pShader = new CDebugShader(name);
        rootSignatureName = "Debug"; 
    }
    else if (name == "Lighting") {
        pShader = new CLightingShader(name);
        rootSignatureName = "Lighting"; 
    }


    else if (name == "Instancing") {
        // CInstancingShader Ŭ������ �ִٰ� ���� (�ʿ�� ����)
        // pShader = new CInstancingShader();
        rootSignatureName = "Instancing"; // �Ǵ� "Standard" ��Ʈ ������ ����� ���� ����
        OutputDebugStringA("Warning: Instancing shader creation not fully implemented in example.\n");
        // return nullptr; // Ȥ�� ������ Ŭ���� ����
    }
    else {
        OutputDebugStringA("Error: Unknown shader name requested for creation!\n");
        return nullptr; // �𸣴� ���̴� �̸�
    }

    // ���̴� ��ü ���� ���� ��
    if (!pShader) {
        OutputDebugStringA("Error: Failed to instantiate shader object.\n");
        return nullptr;
    }

    // 2. �ʿ��� ��Ʈ ���� �������� (���� �� ������ ���̴� ��ü ����)
    if (!rootSignatureName.empty()) {
        pRootSig = GetRootSignature(rootSignatureName);
        if (!pRootSig) {
            OutputDebugStringA(("Error: Failed to get Root Signature '" + rootSignatureName + "' for shader '" + name + "'\n").c_str());
            delete pShader; // ��Ʈ ���� ������ ���̴� ���� �Ұ�
            return nullptr;
        }
    }
    else {
        OutputDebugStringA(("Error: Root Signature name not set for shader '" + name + "'\n").c_str());
        delete pShader;
        return nullptr;
    }

    // 3. PSO ����
    pShader->CreateShader(m_pd3dDevice, pd3dCommandList, pRootSig);


    // 4. PSO ���� ���� ���� Ȯ��
    if (!pShader->GetPipelineState()) {
        OutputDebugStringA(("Error: Failed to create PSO for shader '" + name + "'\n").c_str());
        delete pShader; // PSO ���� ���� �� ��ü ����
        return nullptr;
    }

    // 5. ���̴� ��ü�� ��Ʈ ���� ������ ����
    pShader->SetRootSignature(pRootSig);


    OutputDebugStringA(("Successfully created Shader and PSO: " + name + "\n").c_str());
    return pShader; // ����
}