#pragma once
#pragma once
#include "Shader.h" // CShader 기본 클래스 헤더

class CComputeShaderWrapper : public CShader
{
public:
    CComputeShaderWrapper();
    virtual ~CComputeShaderWrapper();

    // CShader의 순수 가상 함수 구현
    virtual std::string GetShaderType() const override { return "ComputeShaderWrapper"; }

    // 그래픽스 PSO 생성 함수 (CShader에서 상속) - 컴퓨트 셰이더는 사용 안 함
    // 비워두거나, 호출 시 에러/경고를 출력하도록 오버라이드 할 수 있습니다.
    virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature) override;

    // 컴퓨트 PSO 생성 함수
    void BuildComputePipelineState(ID3D12Device* pd3dDevice, const std::wstring& shaderFileName, const std::string& entryPoint, ID3D12RootSignature* pd3dRootSignature);
    ID3D12PipelineState* GetComputePSO() const { return m_pd3dComputePipelineState.Get(); }

protected:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pd3dComputePipelineState;
};