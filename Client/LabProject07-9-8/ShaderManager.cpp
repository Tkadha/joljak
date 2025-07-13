// ShaderManager.cpp

#include "stdafx.h" // 필요시 프로젝트의 stdafx 사용
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
// 예: #include "StandardShader.h", "SkinnedShader.h" 등 (아직 없다면 생성 필요)

// --- 정적 샘플러 정의 ---
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

// --- 생성자, 소멸자, 정리 함수 ---
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
    m_mapPipelineStates.clear();
    m_mapRootSignatures.clear();
}


ID3D12PipelineState* ShaderManager::GetPipelineState(const std::string& name)
{
    // 요청받은 이름의 PSO가 맵에 없는지 확인
    if (m_mapPipelineStates.find(name) == m_mapPipelineStates.end())
    {
        // 만약 없다면, CreatePSO 함수를 호출하여 새로 생성
        CreatePSO(name);
    }

    // 맵에서 PSO를 찾아 반환
    return m_mapPipelineStates[name].Get();
}

ID3D12RootSignature* ShaderManager::GetRootSignature(const std::string& name)
{
    // 1. 캐시 확인
    auto it = m_mapRootSignatures.find(name);
    if (it != m_mapRootSignatures.end())
    {
        return it->second.Get(); // 캐시된 루트 서명 반환
    }
    else
    {
        // 2. 캐시에 없으면 생성
        OutputDebugStringA("Creating Root Signature: ");
        OutputDebugStringA(name.c_str());
        OutputDebugStringA("\n");
        ID3D12RootSignature* pNewRootSig = CreateRootSignatureInternal(name);

        // 3. 생성 성공 시 캐시에 저장
        if (pNewRootSig)
        {
            m_mapRootSignatures[name] = pNewRootSig; // ComPtr이 소유권 가짐
            return pNewRootSig;
        }
        else
        {
            OutputDebugStringA("Error: Failed to create Root Signature '");
            OutputDebugStringA(name.c_str());
            OutputDebugStringA("'\n");
            return nullptr; // 생성 실패
        }
    }
}

// 루트 서명 생성
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

// --- 각 루트 서명 생성 함수 구현 ---
// (이전 답변에서 제공했던 CreateStandardRootSignature, CreateSkinnedRootSignature 등의 함수 구현을
//  여기에 멤버 함수 형태로 넣습니다. m_pd3dDevice 를 사용하도록 합니다.)

ID3D12RootSignature* ShaderManager::CreateStandardRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[2];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 6, 0); // t6-t12: Albedo, Spec, Normal, Metal, Emis, DetailAlb, DetailNrm
    descRangeSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0); // t3: 그림자 맵 SRV


    CD3DX12_ROOT_PARAMETER rootParameters[5]; // CBV(b1 Camera), Constants(b2 Object), CBV(b4 Lights), Table(t6-t12 Textures)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL); // 1 Matrix + 4 float4 Material + 1 uint Mask = 16 + 16 + 1 = 33 DWORDS
    rootParameters[2].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL); // b4: Lights (이 레지스터를 사용한다고 가정)
    rootParameters[3].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // 그림자 맵 테이블

    auto staticSamplers = GetStaticSamplers(); // Wrap(s0), Clamp(s1) 가져옴
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        _countof(rootParameters), // 파라미터 개수: 5
        rootParameters,
        (UINT)staticSamplers.size(), // 샘플러 개수: 3
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
    descRangeSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0); // t3: 그림자 맵 SRV

    CD3DX12_ROOT_PARAMETER rootParameters[7]; // Standard + CBV(b7), CBV(b8)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // b1: Camera
    rootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL);     // b2: Object
    rootParameters[2].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL); // b4: Lights
    rootParameters[3].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // t6-t12
    rootParameters[4].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // 그림자 맵 테이블
    rootParameters[5].InitAsConstantBufferView(7, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b7: Bone Offsets
    rootParameters[6].InitAsConstantBufferView(8, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b8: Bone Transforms

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        _countof(rootParameters), // 파라미터 개수: 5
        rootParameters,
        (UINT)staticSamplers.size(), // 샘플러 개수: 3
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
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // 둘 다 사용
    rootParameters[1].InitAsConstants(16, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);     // VS에서만 필요
    rootParameters[2].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요
    rootParameters[3].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요

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
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // VS에서 View/Proj 필요
    rootParameters[1].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서 샘플링

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParameters), rootParameters,
        1, &staticSamplers[1], // Clamp 샘플러(s1)만 사용 가정
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
    // OBB 셰이더 HLSL은 cbTransform:register(b0) 사용
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    // 샘플러나 텍스처 필요 없음
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
    // Instancing 셰이더용 루트 서명 (Standard와 유사하나 b2 GameObjectInfo 대신 Instance Data 사용)
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
        1, &staticSamplers[0], // Wrap 샘플러(s0)만 사용 가정
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
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // 둘 다 사용
    rootParameters[1].InitAsConstants(16, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);     // VS에서만 필요
    rootParameters[2].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요
    rootParameters[3].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요
    rootParameters[4].InitAsDescriptorTable(1, &descRangeSRV[2], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요
    rootParameters[5].InitAsDescriptorTable(1, &descRangeSRV[3], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요
    rootParameters[6].InitAsDescriptorTable(1, &descRangeSRV[4], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요

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
    rootParameters[0].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서 샘플링

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

    // --- 셰이더가 받을 리소스들을 종류별로 정의합니다 ---

    // 1. G-Buffer 텍스처들 (4개, t13부터 시작)
    CD3DX12_DESCRIPTOR_RANGE descRangeGBuffer;
    descRangeGBuffer.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 13); // 4 SRVs starting at register t13

    // 2. 그림자 맵 텍스처 (1개, t3)
    CD3DX12_DESCRIPTOR_RANGE descRangeShadowMap;
    descRangeShadowMap.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3); // 1 SRV at register t3

    // 루트 파라미터
    CD3DX12_ROOT_PARAMETER rootParameters[4];

    // 파라미터 0: 카메라 정보 (b0)
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    // 파라미터 1: 조명 정보 (b4)
    rootParameters[1].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    // 파라미터 2: 그림자 맵 테이블 (t3)
    rootParameters[2].InitAsDescriptorTable(1, &descRangeShadowMap, D3D12_SHADER_VISIBILITY_PIXEL);

    // 파라미터 3: G-Buffer 텍스처 테이블 (t13, t14, t15, t16)
    rootParameters[3].InitAsDescriptorTable(1, &descRangeGBuffer, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        _countof(rootParameters), // 파라미터 개수
        rootParameters,
        (UINT)staticSamplers.size(), // 모든 정적 샘플러 사용
        staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | // 조명 패스는 VS가 단순하므로 접근 불필요
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

void ShaderManager::CreatePSO(const std::string& name)
{
    // 이미 생성된 PSO면 바로 리턴
    if (m_mapPipelineStates.count(name) > 0) return;

    // 이름에 따라 사용할 재료(CShader)와 루트서명을 결정
    std::unique_ptr<CShader> pTempShader = nullptr;
    std::string rootSignatureName;
    D3D12_SHADER_BYTECODE psByteCode;

    if (name == "Standard_GBuffer")
    {
        pTempShader = std::make_unique<CStandardShader>(name);
        rootSignatureName = "Standard";
    }
    else if (name == "Standard")
    {
        pTempShader = std::make_unique<CStandardShader>(name);
        rootSignatureName = "Standard";
    }
    else if (name == "Skinned")
    {
        pTempShader = std::make_unique<CSkinnedAnimationStandardShader>(name);
        rootSignatureName = "Skinned";
    }
    else if (name == "Shadow")
    {
        pTempShader = std::make_unique<CShadowShader>(name);
        rootSignatureName = "Shadow";
    }
    // 다른 셰이더

    if (!pTempShader) return;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = GetRootSignature(rootSignatureName); // 미리 생성된 루트서명 가져오기

    ID3DBlob* vsBlob = nullptr, * psBlob = nullptr;
    psoDesc.VS = pTempShader->CreateVertexShader(&vsBlob);
    psoDesc.PS = pTempShader->CreatePixelShader(&psBlob);

    D3D12_INPUT_LAYOUT_DESC inputLayout = pTempShader->CreateInputLayout();
    psoDesc.InputLayout = inputLayout;

    psoDesc.RasterizerState = pTempShader->CreateRasterizerState();
    psoDesc.BlendState = pTempShader->CreateBlendState();
    psoDesc.DepthStencilState = pTempShader->CreateDepthStencilState();
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    //psoDesc.NumRenderTargets = 1;
    //psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    if (name.find("_GBuffer") != std::string::npos) // 이름에 _GBuffer가 포함되어 있으면
    {
		psoDesc.NumRenderTargets = GBUFFER_COUNT;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
    }
    else if (name == "Shadow")
    {
        psoDesc.NumRenderTargets = 0; // 그림자 패스는 컬러 타겟 없음
        psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    }
    else // 그 외 (포워드 렌더링)
    {
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    // 최종 PSO 생성 및 맵에 저장
    HRESULT hResult = m_pd3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_mapPipelineStates[name]));

    if (psoDesc.InputLayout.pInputElementDescs) delete[] psoDesc.InputLayout.pInputElementDescs;

    if (FAILED(hResult))
    {
        OutputDebugStringA(("\n!!! CRITICAL ERROR: CreateGraphicsPipelineState FAILED for shader: " + name + "\n").c_str());
        OutputDebugStringA("!!! Querying D3D12 InfoQueue for details...\n");

        // 실패 시, InfoQueue에서 상세 오류 내용을 가져와 출력합니다.
        PrintD3D12InfoQueue(m_pd3dDevice);
    }


    if (vsBlob) vsBlob->Release();
    if (psBlob) psBlob->Release();
}