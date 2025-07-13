#pragma once
#include "stdafx.h"
#include <map>
#include <string>
#include <memory>
#include "Shader.h" // CShader 기본 클래스 포함

class CGameFramework; // 전방 선언

class ShaderManager
{
public:
    ShaderManager(ID3D12Device* pd3dDevice);
    ~ShaderManager();

    // 복사 및 대입을 방지합니다.
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    // 관리하는 모든 리소스를 해제합니다.
    void ReleaseAll();

    // 특정 이름의 루트 시그니처를 가져옵니다. 없으면 생성합니다.
    ID3D12RootSignature* GetRootSignature(const std::string& name);

    // 특정 이름의 셰이더를 가져옵니다. 없으면 생성합니다.
    CShader* GetShader(const std::string& name);

private:
    // 루트 시그니처 생성 함수
    void CreateRootSignatures();

    // 셰이더 생성 함수
    void CreateShaders();

    ID3D12Device* m_pd3dDevice;

    // 셰이더와 루트 시그니처를 이름으로 관리하는 맵
    std::map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> m_mapRootSignatures;
    std::map<std::string, std::unique_ptr<CShader>> m_mapShaders;
};