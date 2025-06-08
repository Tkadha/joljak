// ShaderManager.cpp

#include "stdafx.h" // 필요시 프로젝트의 stdafx 사용
#include "ShaderManager.h"
#include "Shader.h"
#include "OBBShader.h"
#include "TerrainShader.h"
#include "StandardShader.h"
#include "SkinnedShader.h"
#include "SkyBoxShader.h"
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

    const CD3DX12_STATIC_SAMPLER_DESC shadowSampler(
        2, // shaderRegister
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,                               // mipLODBias
        16,                                 // maxAnisotropy
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);


    return { wrapSampler, clampSampler, shadowSampler };
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
    // 관리하던 CShader 객체들의 참조 카운트 감소
    for (auto const& [key, pShader] : m_Shaders)
    {
        if (pShader)
        {
            pShader->Release(); // ShaderManager가 저장 시 AddRef 했으므로 Release
        }
    }
    m_Shaders.clear();

    // 루트 서명은 ComPtr이 자동으로 Release 처리
    m_RootSignatures.clear();
}

// --- 루트 서명 관리 구현 ---
ID3D12RootSignature* ShaderManager::GetRootSignature(const std::string& name)
{
    // 1. 캐시 확인
    auto it = m_RootSignatures.find(name);
    if (it != m_RootSignatures.end())
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
            m_RootSignatures[name] = pNewRootSig; // ComPtr이 소유권 가짐
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
    if (name == "Instancing") return CreateInstancingRootSignature(); // 필요시

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

    // 디스크립터 레인지가 두 종류 필요합니다. (기존 재질용, 그림자맵 용)
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[2];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 6, 0); // t6-t12: Albedo, Spec, Normal, Metal, Emis, DetailAlb, DetailNrm
    descRangeSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0); // t3: 그림자 맵


    CD3DX12_ROOT_PARAMETER rootParameters[6];
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);      // b0: Pass CBV (View, Proj, ShadowTransform)
    rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);      // b1: Camera
    rootParameters[2].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL);           // b2: Object
    rootParameters[3].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL);      // b4: Lights
    rootParameters[4].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // t6-t12: Material
    rootParameters[5].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // t3: Shadow Map


    auto staticSamplers = GetStaticSamplers(); // Wrap(s0), Clamp(s1) 가져옴
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

ID3D12RootSignature* ShaderManager::CreateSkinnedRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;

    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[2];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 6, 0); // t6-t12: 재질 텍스처
    descRangeSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0); // t3: 그림자 맵 SRV

    // 루트 파라미터 개수를 6개에서 8개로 늘립니다.
    CD3DX12_ROOT_PARAMETER rootParameters[8];
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);      // b0: Pass CBV (View, Proj, ShadowTransform)
    rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);      // b1: Camera
    rootParameters[2].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL);           // b2: Object
    rootParameters[3].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL);      // b4: Lights
    rootParameters[4].InitAsConstantBufferView(7, 0, D3D12_SHADER_VISIBILITY_VERTEX);  // b7: Bone Offsets
    rootParameters[5].InitAsConstantBufferView(8, 0, D3D12_SHADER_VISIBILITY_VERTEX);  // b8: Bone Transforms
    rootParameters[6].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // t6-t12: Material
    rootParameters[7].InitAsDescriptorTable(1, &descRangeSRV[1], D3D12_SHADER_VISIBILITY_PIXEL); // t3: Shadow Map


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

ID3D12RootSignature* ShaderManager::CreateTerrainRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[1];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0); // t1, t2

    CD3DX12_ROOT_PARAMETER rootParameters[3]; // CBV(b1 Camera), Constants(b2 Object), Table(t1, t2 Textures)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // 둘 다 사용
    rootParameters[1].InitAsConstants(16, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);     // VS에서만 필요
    rootParameters[2].InitAsDescriptorTable(1, &descRangeSRV[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요

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

ID3D12RootSignature* ShaderManager::CreateSkyboxRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE descRangeSRV[1];
    descRangeSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0); // t3: SkyCubeTexture

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

ID3D12RootSignature* ShaderManager::CreateShadowMapRootSignature()
{
    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    CD3DX12_DESCRIPTOR_RANGE texTable;
    texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3); // t3 : shadow map

    CD3DX12_ROOT_PARAMETER rootParameters[2]; // CBV(b1 Camera), Table(t3 Texture)
    rootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // VS에서 View/Proj 필요
    rootParameters[1].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL); // PS에서 샘플링

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParameters), rootParameters,
        1, &staticSamplers[2],
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = m_pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
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

// 셰이더 생성 분기 로직
CShader* ShaderManager::CreateShaderInternal(const std::string& name, ID3D12GraphicsCommandList* pd3dCommandList)
{
    CShader* pShader = nullptr;
    ID3D12RootSignature* pRootSig = nullptr;
    std::string rootSignatureName = "";

    // 1. 셰이더 이름에 따라 사용할 클래스와 루트 서명 이름 결정
    if (name == "Standard") {
        pShader = new CStandardShader(); // Standard 셰이더 객체 생성
        rootSignatureName = "Standard";  // Standard 루트 서명 필요
    }
    else if (name == "Skinned") {
        pShader = new CSkinnedAnimationStandardShader();
        rootSignatureName = "Skinned";
    }
    else if (name == "Terrain") {
        pShader = new CTerrainShader();
        rootSignatureName = "Terrain";
    }
    else if (name == "Skybox") {
        pShader = new CSkyBoxShader();
        rootSignatureName = "Skybox";
    }
    else if (name == "OBB") {
        pShader = new COBBShader();
        rootSignatureName = "OBB";
    }
    else if (name == "Instancing") {
        // CInstancingShader 클래스가 있다고 가정 (필요시 생성)
        // pShader = new CInstancingShader();
        rootSignatureName = "Instancing"; // 또는 "Standard" 루트 서명을 사용할 수도 있음
        OutputDebugStringA("Warning: Instancing shader creation not fully implemented in example.\n");
        // return nullptr; // 혹은 적절한 클래스 생성
    }
    else {
        OutputDebugStringA("Error: Unknown shader name requested for creation!\n");
        return nullptr; // 모르는 셰이더 이름
    }

    // 셰이더 객체 생성 실패 시
    if (!pShader) {
        OutputDebugStringA("Error: Failed to instantiate shader object.\n");
        return nullptr;
    }

    // 2. 필요한 루트 서명 가져오기 (실패 시 생성된 셰이더 객체 삭제)
    if (!rootSignatureName.empty()) {
        pRootSig = GetRootSignature(rootSignatureName);
        if (!pRootSig) {
            OutputDebugStringA(("Error: Failed to get Root Signature '" + rootSignatureName + "' for shader '" + name + "'\n").c_str());
            delete pShader; // 루트 서명 없으면 셰이더 생성 불가
            return nullptr;
        }
    }
    else {
        OutputDebugStringA(("Error: Root Signature name not set for shader '" + name + "'\n").c_str());
        delete pShader;
        return nullptr;
    }

    // 3. PSO 생성
    pShader->CreateShader(m_pd3dDevice, pd3dCommandList, pRootSig);


    // 4. PSO 생성 성공 여부 확인
    if (!pShader->GetPipelineState()) {
        OutputDebugStringA(("Error: Failed to create PSO for shader '" + name + "'\n").c_str());
        delete pShader; // PSO 생성 실패 시 객체 삭제
        return nullptr;
    }

    // 5. 셰이더 객체에 루트 서명 포인터 저장
    pShader->SetRootSignature(pRootSig);


    OutputDebugStringA(("Successfully created Shader and PSO: " + name + "\n").c_str());
    return pShader; // 성공
}