#include "ShaderManager.h"
#include "GameFramework.h"

#include "StandardShader.h"
#include "TerrainShader.h"
#include "SkyboxShader.h"
//#include "SkinnedShader.h"
#include "OBBShader.h"
#include "ShadowShader.h"
#include "DebugShader.h"
#include "WavesShader.h" 
#include "SkinnedShadowShader.h"

// --- 생성자, 소멸자, 정리 함수 ---
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
    m_mapRootSignatures["Waves"] = CStandardShader::CreateRootSignature(m_pd3dDevice);
    m_mapRootSignatures["Skinned_Shadow"] = CSkinnedAnimationStandardShader::CreateRootSignature(m_pd3dDevice);
}

void ShaderManager::CreateShaders()
{
    m_mapShaders["Standard"] = std::make_unique<CStandardShader>();
    m_mapShaders["Skinned"] = std::make_unique<CSkinnedAnimationStandardShader>();
    m_mapShaders["Terrain"] = std::make_unique<CTerrainShader>();
    m_mapShaders["Skybox"] = std::make_unique<CSkyBoxShader>();
    m_mapShaders["OBB"] = std::make_unique<COBBShader>();
    m_mapShaders["Shadow"] = std::make_unique<CShadowShader>();
    m_mapShaders["Debug"] = std::make_unique<CDebugShader>();
    m_mapShaders["Waves"] = std::make_unique<CWavesShader>();
    m_mapShaders["Skinned_Shadow"] = std::make_unique<CSkinnedShadowShader>();
}

ID3D12RootSignature* ShaderManager::GetRootSignature(const std::string& name)
{
    auto it = m_mapRootSignatures.find(name);
    if (it != m_mapRootSignatures.end())
    {
        return it->second.Get();
    }
    return nullptr;
}

CShader* ShaderManager::GetShader(const std::string& name)
{
    auto it = m_mapShaders.find(name);
    if (it != m_mapShaders.end())
    {
        if (it->second->GetPipelineState() == nullptr)
        {
            ID3D12RootSignature* pRootSignature = GetRootSignature(it->second->GetShaderType());
            if (pRootSignature)
            {
                it->second->CreateShader(m_pd3dDevice, nullptr, pRootSignature);

                it->second->SetRootSignature(pRootSignature);
            }
        }
        return it->second.get();
    }
    return nullptr;
}