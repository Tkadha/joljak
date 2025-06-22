// ShaderManager.h

#pragma once

#include "stdafx.h"

// 미리 선언 (순환 참조 방지)
class CShader;

// 헬퍼 함수 선언 (ShaderManager.cpp 에서 정의)
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 3> GetStaticSamplers(); // 예시: Wrap, Clamp 샘플러 2개

class ShaderManager
{
private:
    ID3D12Device* m_pd3dDevice = nullptr; // 리소스 생성에 필요

    // 루트 서명 캐시 (이름 -> 루트 서명 객체)
    std::map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> m_RootSignatures;

    // 셰이더 캐시 (이름 -> 셰이더 객체)
    // CShader가 자체 참조 카운팅(AddRef/Release)을 하므로 raw 포인터 사용
    // ShaderManager는 저장 시 AddRef, 소멸 시 Release 필요
    std::map<std::string, CShader*> m_Shaders;

    // 루트 서명 생성 함수
    ID3D12RootSignature* CreateRootSignatureInternal(const std::string& name);

    ID3D12RootSignature* CreateStandardRootSignature();
    ID3D12RootSignature* CreateSkinnedRootSignature();
    ID3D12RootSignature* CreateTerrainRootSignature();
    ID3D12RootSignature* CreateSkyboxRootSignature();
    ID3D12RootSignature* CreateOBBRootSignature();
    ID3D12RootSignature* CreateInstancingRootSignature();
    ID3D12RootSignature* CreateShadowRootSignature();
    ID3D12RootSignature* CreateDebugRootSignature();

    // 셰이더 생성 함수
    CShader* CreateShaderInternal(const std::string& name, ID3D12GraphicsCommandList* pd3dCommandList);


public:
    ShaderManager(ID3D12Device* pd3dDevice);
    ~ShaderManager(); // 소멸자에서 캐시된 리소스 정리 필요

    // 복사/대입 방지
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    // --- 루트 서명 접근 ---
    // 요청한 이름의 루트 서명을 반환 (없으면 생성 후 캐싱)
    ID3D12RootSignature* GetRootSignature(const std::string& name);

    // --- 셰이더 접근 ---
    // 요청한 이름의 셰이더를 반환 (없으면 생성 후 캐싱 및 PSO 생성)
    // 주의: CShader::CreateShader는 CommandList가 필요함
    CShader* GetShader(const std::string& name, ID3D12GraphicsCommandList* pd3dCommandList);

    // --- 정리 ---
    void ReleaseAll(); // 모든 캐시된 리소스 해제
};