#include "ShaderManager.h"
#include "GameFramework.h"

#include "StandardShader.h"
#include "TerrainShader.h"
#include "SkyboxShader.h"
#include "SkinnedShader.h"
#include "OBBShader.h"
#include "ShadowShader.h"
#include "DebugShader.h"
#include "WavesShader.h"

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
    CreateRootSignatures();
    CreateShaders();
}

ShaderManager::~ShaderManager()
{
    ReleaseAll();
}

void ShaderManager::ReleaseAll()
{
    m_mapRootSignatures.clear();
    m_mapShaders.clear();
}

void ShaderManager::CreateRootSignatures()
{
    m_mapRootSignatures["Standard"] = CStandardShader::CreateRootSignature(m_pd3dDevice);
    m_mapRootSignatures["Skinned"] = CSkinnedAnimationStandardShader::CreateRootSignature(m_pd3dDevice);
    m_mapRootSignatures["Terrain"] = CTerrainShader::CreateRootSignature(m_pd3dDevice);
    m_mapRootSignatures["Skybox"] = CSkyBoxShader::CreateRootSignature(m_pd3dDevice);
    m_mapRootSignatures["OBB"] = COBBShader::CreateRootSignature(m_pd3dDevice);
    m_mapRootSignatures["Shadow"] = CShadowShader::CreateRootSignature(m_pd3dDevice);
    m_mapRootSignatures["Debug"] = CDebugShader::CreateRootSignature(m_pd3dDevice);
    // Waves�� Standard�� ������ �Ķ����
    m_mapRootSignatures["Waves"] = CStandardShader::CreateRootSignature(m_pd3dDevice);
}

void ShaderManager::CreateShaders()
ID3D12RootSignature* ShaderManager::GetRootSignature(const std::string& name)
{
    auto it = m_mapRootSignatures.find(name);
    if (it != m_mapRootSignatures.end())
    {
        return it->second.Get();
    }
// ��Ʈ ���� ����
ID3D12RootSignature* ShaderManager::CreateRootSignatureInternal(const std::string& name)
{
    if (name == "Standard") return CreateStandardRootSignature();
    if (name == "Skinned") return CreateSkinnedRootSignature();
    auto it = m_mapShaders.find(name);
    if (it != m_mapShaders.end())
    {
        if (it->second->GetPipelineState() == nullptr)
        {
            ID3D12RootSignature* pRootSignature = GetRootSignature(it->second->GetShaderType());
            if (pRootSignature)
            {
                it->second->CreateShader(m_pd3dDevice, nullptr, pRootSignature);


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
        pShader = new CStandardShader(); // Standard ���̴� ��ü ����
        rootSignatureName = "Standard";  // Standard ��Ʈ ���� �ʿ�
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
    else if (name == "Shadow") {
        pShader = new CShadowShader();
        rootSignatureName = "Shadow"; // Shadow ���̴��� Shadow ��Ʈ ������ ���
    }
    else if (name == "Debug") {
        pShader = new CDebugShader();
        rootSignatureName = "Debug"; // Shadow ���̴��� Shadow ��Ʈ ������ ���
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

                it->second->SetRootSignature(pRootSignature);
            }
        }
        return it->second.get();
    }
    return nullptr;
}